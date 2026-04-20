/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2026 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "checkthread.h"

#include "analyzerinfo.h"
#include "common.h"
#include "cppcheck.h"
#include "erroritem.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "filesettings.h"
#include "settings.h"
#include "standards.h"
#include "threadresult.h"
#include "utils.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <QByteArray>
#include <QChar>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QTextStream>
#include <QVariant>

static QString unquote(QString s) {
    if (s.startsWith("\""))
        s = s.mid(1, s.size() - 2);
    return s;
}

// NOLINTNEXTLINE(performance-unnecessary-value-param) - used as callback so we need to preserve the signature
int CheckThread::executeCommand(std::string exe, std::vector<std::string> args, std::string redirect, std::string &output) // cppcheck-suppress passedByValueCallback
{
    output.clear();

    QStringList args2;
    for (const std::string &arg: args)
        args2 << QString::fromStdString(arg);

    QProcess process;

    QString e = unquote(QString::fromStdString(exe));

    if (e.toLower().replace("\\", "/").endsWith("/python.exe") && !args.empty()) {
        const QString path = e.left(e.size()-11);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PYTHONPATH", path + "/Lib/site-packages");
        env.insert("PYTHONHOME", path);
        process.setProcessEnvironment(env);

        const QString pythonScript = unquote(args2[0]);
        if (pythonScript.endsWith(".py")) {
            const QString path2 = pythonScript.left(QString(pythonScript).replace('\\', '/').lastIndexOf("/"));
            process.setWorkingDirectory(path2);
        }
    }

    process.start(e, args2);
    while (!Settings::terminated() && !process.waitForFinished(1000)) {
        if (process.state() == QProcess::ProcessState::NotRunning)
            break;
    }

    if (redirect == "2>&1") {
        QString s1 = process.readAllStandardOutput();
        QString s2 = process.readAllStandardError();
        output = (s1 + "\n" + s2).toStdString();
    } else
        output = process.readAllStandardOutput().toStdString();

    if (startsWith(redirect, "2> ")) {
        std::ofstream fout(redirect.substr(3));
        fout << process.readAllStandardError().toStdString();
    }
    return process.exitCode();
}


CheckThread::CheckThread(ThreadResult &result, int threadIndex)
    : mResult(result)
    , mThreadIndex(threadIndex)
{}

void CheckThread::setSettings(const Settings &settings, std::shared_ptr<Suppressions> supprs)
{
    mFiles.clear();
    mSettings = settings; // this is a copy
    mSuppressions = std::move(supprs);
}

void CheckThread::analyseWholeProgram(const std::list<FileWithDetails> &files, const std::string& ctuInfo)
{
    mFiles = files;
    mAnalyseWholeProgram = true;
    mCtuInfo = ctuInfo;
    start();
}

void CheckThread::run()
{
    mState = Running;

    CppCheck cppcheck(mSettings, *mSuppressions, mResult, nullptr, true, executeCommand);

    if (!mFiles.empty() || mAnalyseWholeProgram) {
        mAnalyseWholeProgram = false;
        std::string ctuInfo;
        ctuInfo.swap(mCtuInfo);
        qDebug() << "Whole program analysis";
        cppcheck.analyseWholeProgram(mSettings.buildDir, mFiles, {}, ctuInfo);
        mFiles.clear();
        emit done();
        return;
    }

    // TODO: apply enforcedLanguage
    const FileWithDetails* file = nullptr;
    mResult.getNextFile(file);
    while (file && mState == Running) {
        const std::string& fname = file->spath();
        qDebug() << "Checking file" << QString::fromStdString(fname);

        const Details details{ mThreadIndex, QString::fromStdString(fname), QTime::currentTime(), };
        emit startCheck(details);

        cppcheck.check(*file);
        runAddonsAndTools(mSettings, nullptr, QString::fromStdString(fname));

        emit finishCheck(details);

        if (mState == Running)
            mResult.getNextFile(file);
    }

    const FileSettings* fileSettings = nullptr;
    mResult.getNextFileSettings(fileSettings);
    while (fileSettings && mState == Running) {
        const std::string& fname = fileSettings->filename();
        qDebug() << "Checking file" << QString::fromStdString(fname);

        const Details details{ mThreadIndex, QString::fromStdString(fname), QTime::currentTime(), };
        emit startCheck(details);

        cppcheck.check(*fileSettings);
        runAddonsAndTools(mSettings, fileSettings, QString::fromStdString(fname));

        emit finishCheck(details);


        if (mState == Running)
            mResult.getNextFileSettings(fileSettings);
    }

    if (mState == Running)
        mState = Ready;
    else
        mState = Stopped;

    emit done();
}

void CheckThread::runAddonsAndTools(const Settings& settings, const FileSettings *fileSettings, const QString &fileName)
{
    for (const QString& addon : utils::as_const(mAddonsAndTools)) {
        if (addon == CLANG_ANALYZER || addon == CLANG_TIDY) {
            if (!fileSettings)
                continue;

            if (!fileSettings->cfg.empty() && !startsWith(fileSettings->cfg,"Debug"))
                continue;

            QStringList args;
            for (auto incIt = fileSettings->includePaths.cbegin(); incIt != fileSettings->includePaths.cend(); ++incIt)
                args << ("-I" + QString::fromStdString(*incIt));
            for (auto i = fileSettings->systemIncludePaths.cbegin(); i != fileSettings->systemIncludePaths.cend(); ++i)
                args << "-isystem" << QString::fromStdString(*i);
            for (const QString& def : QString::fromStdString(fileSettings->defines).split(";")) {
                args << ("-D" + def);
            }
            for (const std::string& U : fileSettings->undefs) {
                args << QString::fromStdString("-U" + U);
            }

            const QString clangPath = CheckThread::clangTidyCmd();
            if (!clangPath.isEmpty()) {
                QDir dir(clangPath + "/../lib/clang");
                for (const QString& ver : dir.entryList()) {
                    QString includePath = dir.absolutePath() + '/' + ver + "/include";
                    if (ver[0] != '.' && QDir(includePath).exists()) {
                        args << "-isystem" << includePath;
                        break;
                    }
                }
            }

#ifdef Q_OS_WIN
            // To create compile_commands.json in windows see:
            // https://bitsmaker.gitlab.io/post/clang-tidy-from-vs2015/

            for (QString includePath : mClangIncludePaths) {
                if (!includePath.isEmpty()) {
                    includePath.replace("\\", "/");
                    args << "-isystem" << includePath.trimmed();
                }
            }

            args << "-U__STDC__" << "-fno-ms-compatibility";
#endif

            if (!fileSettings->standard.empty())
                args << ("-std=" + QString::fromStdString(fileSettings->standard));
            else {
                // TODO: pass C or C++ standard based on file type
                const std::string std = settings.standards.getCPP();
                if (!std.empty()) {
                    args << ("-std=" + QString::fromStdString(std));
                }
            }

            QString analyzerInfoFile;

            const std::string &buildDir = settings.buildDir;
            if (!buildDir.empty()) {
                analyzerInfoFile = QString::fromStdString(AnalyzerInformation::getAnalyzerInfoFile(buildDir, fileSettings->sfilename(), fileSettings->cfg, fileSettings->file.fsFileId()));

                QStringList args2(args);
                args2.insert(0,"-E");
                args2 << fileName;
                QProcess process;
                process.start(clangCmd(),args2);
                process.waitForFinished();
                const QByteArray &ba = process.readAllStandardOutput();
                const quint16 chksum = qChecksum(QByteArrayView(ba));

                QFile f1(analyzerInfoFile + '.' + addon + "-E");
                if (f1.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in1(&f1);
                    const quint16 oldchksum = in1.readAll().toInt();
                    if (oldchksum == chksum) {
                        QFile f2(analyzerInfoFile + '.' + addon + "-results");
                        if (f2.open(QIODevice::ReadOnly | QIODevice::Text)) {
                            QTextStream in2(&f2);
                            parseClangErrors(addon, fileName, in2.readAll());
                            continue;
                        }
                    }
                    f1.close();
                }
                (void)f1.open(QIODevice::WriteOnly | QIODevice::Text); // TODO: check result
                QTextStream out1(&f1);
                out1 << chksum;

                QFile::remove(analyzerInfoFile + '.' + addon + "-results");
            }

            if (addon == CLANG_ANALYZER) {
                /*
                   // Using clang
                   args.insert(0,"--analyze");
                   args.insert(1, "-Xanalyzer");
                   args.insert(2, "-analyzer-output=text");
                   args << fileName;
                 */
                // Using clang-tidy
                args.insert(0,"-checks=-*,clang-analyzer-*");
                args.insert(1, fileName);
                args.insert(2, "--");
            } else {
                args.insert(0,"-checks=*,-clang-analyzer-*,-llvm*");
                args.insert(1, fileName);
                args.insert(2, "--");
            }

            {
                const QString cmd(clangTidyCmd());
                QString debug(cmd.contains(" ") ? ('\"' + cmd + '\"') : cmd);
                for (const QString& arg : utils::as_const(args)) {
                    if (arg.contains(" "))
                        debug += " \"" + arg + '\"';
                    else
                        debug += ' ' + arg;
                }
                qDebug() << debug;

                if (!analyzerInfoFile.isEmpty()) {
                    QFile f(analyzerInfoFile + '.' + addon + "-cmd");
                    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&f);
                        out << debug;
                    }
                }
            }

            QProcess process;
            process.start(clangTidyCmd(), args);
            process.waitForFinished(600*1000);
            const QString errout(process.readAllStandardOutput() + "\n\n\n" + process.readAllStandardError());
            if (!analyzerInfoFile.isEmpty()) {
                QFile f(analyzerInfoFile + '.' + addon + "-results");
                if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&f);
                    out << errout;
                }
            }

            parseClangErrors(addon, fileName, errout);
        }
    }
}

void CheckThread::stop()
{
    mState = Stopping;
    Settings::terminate();
}

void CheckThread::parseClangErrors(const QString &tool, const QString &file0, QString err)
{
    QList<ErrorItem> errorItems;
    ErrorItem errorItem;
    static const QRegularExpression r1("^(.+):([0-9]+):([0-9]+): (note|warning|error|fatal error): (.*)$");
    static const QRegularExpression r2("^(.*)\\[([a-zA-Z0-9\\-_\\.]+)\\]$");
    QTextStream in(&err, QIODevice::ReadOnly);
    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.startsWith("Assertion failed:")) {
            ErrorItem e;
            e.errorPath.append(QErrorPathItem());
            e.errorPath.last().file   = file0;
            e.errorPath.last().line   = 1;
            e.errorPath.last().column = 1;
            e.errorId = tool + "-internal-error";
            e.file0 = file0;
            e.message = line;
            e.severity = Severity::information;
            errorItems.append(e);
            continue;
        }

        const QRegularExpressionMatch r1MatchRes = r1.match(line);
        if (!r1MatchRes.hasMatch())
            continue;
        if (r1MatchRes.captured(4) != "note") {
            errorItems.append(errorItem);
            errorItem = ErrorItem();
            errorItem.file0 = r1MatchRes.captured(1);
        }

        errorItem.errorPath.append(QErrorPathItem());
        errorItem.errorPath.last().file = r1MatchRes.captured(1);
        errorItem.errorPath.last().line = r1MatchRes.captured(2).toInt();
        errorItem.errorPath.last().column = r1MatchRes.captured(3).toInt();
        if (r1MatchRes.captured(4) == "warning")
            errorItem.severity = Severity::warning;
        else if (r1MatchRes.captured(4) == "error" || r1MatchRes.captured(4) == "fatal error")
            errorItem.severity = Severity::error;

        QString message,id;
        const QRegularExpressionMatch r2MatchRes = r2.match(r1MatchRes.captured(5));
        if (r2MatchRes.hasMatch()) {
            message = r2MatchRes.captured(1);
            const QString id1(r2MatchRes.captured(2));
            if (id1.startsWith("clang"))
                id = id1;
            else
                id = tool + '-' + r2MatchRes.captured(2);
            if (tool == CLANG_TIDY) {
                if (id1.startsWith("performance"))
                    errorItem.severity = Severity::performance;
                else if (id1.startsWith("portability"))
                    errorItem.severity = Severity::portability;
                else if (id1.startsWith("misc") && !id1.contains("unused"))
                    errorItem.severity = Severity::warning;
                else
                    errorItem.severity = Severity::style;
            }
        } else {
            message = r1MatchRes.captured(5);
            id = CLANG_ANALYZER;
        }

        if (errorItem.errorPath.size() == 1) {
            errorItem.message = message;
            errorItem.errorId = id;
        }

        errorItem.errorPath.last().info = message;
    }
    errorItems.append(errorItem);

    for (const ErrorItem &e : errorItems) {
        if (e.errorPath.isEmpty())
            continue;
        SuppressionList::ErrorMessage errorMessage;
        errorMessage.setFileName(e.errorPath.back().file.toStdString());
        errorMessage.lineNumber = e.errorPath.back().line;
        errorMessage.errorId = e.errorId.toStdString();
        errorMessage.symbolNames = e.symbolNames.toStdString();

        if (isSuppressed(errorMessage))
            continue;

        std::list<ErrorMessage::FileLocation> callstack;
        std::transform(e.errorPath.cbegin(), e.errorPath.cend(), std::back_inserter(callstack), [](const QErrorPathItem& path) {
            return ErrorMessage::FileLocation(path.file.toStdString(), path.info.toStdString(), path.line, path.column);
        });
        const std::string f0 = file0.toStdString();
        const std::string msg = e.message.toStdString();
        const std::string id = e.errorId.toStdString();
        ErrorMessage errmsg(std::move(callstack), f0, e.severity, msg, id, Certainty::normal);
        mResult.reportErr(errmsg);
    }
}

bool CheckThread::isSuppressed(const SuppressionList::ErrorMessage &errorMessage) const
{
    return std::any_of(mSuppressionsUi.cbegin(), mSuppressionsUi.cend(), [&](const SuppressionList::Suppression& s) -> bool {
        return s.isSuppressed(errorMessage) == SuppressionList::Suppression::Result::Matched;
    });
}

QString CheckThread::clangCmd()
{
    QString path = QSettings().value(SETTINGS_CLANG_PATH,QString()).toString();
    if (!path.isEmpty())
        path += '/';
    path += "clang";
#ifdef Q_OS_WIN
    path += ".exe";
#endif

    QProcess process;
    process.start(path, QStringList() << "--version");
    process.waitForFinished();
    if (process.exitCode() == 0)
        return path;

#ifdef Q_OS_WIN
    // Try to autodetect clang
    if (QFileInfo("C:/Program Files/LLVM/bin/clang.exe").exists())
        return "C:/Program Files/LLVM/bin/clang.exe";
#endif

    return QString();
}

QString CheckThread::clangTidyCmd()
{
    QString path = QSettings().value(SETTINGS_CLANG_PATH,QString()).toString();
    if (!path.isEmpty())
        path += '/';
    path += "clang-tidy";
#ifdef Q_OS_WIN
    path += ".exe";
#endif

    QProcess process;
    process.start(path, QStringList() << "--version");
    process.waitForFinished();
    if (process.exitCode() == 0)
        return path;

#ifdef Q_OS_WIN
    // Try to autodetect clang-tidy
    if (QFileInfo("C:/Program Files/LLVM/bin/clang-tidy.exe").exists())
        return "C:/Program Files/LLVM/bin/clang-tidy.exe";
#endif

    return QString();
}

