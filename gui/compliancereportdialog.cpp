/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "compliancereportdialog.h"

#include "ui_compliancereportdialog.h"

#include "errortypes.h"
#include "filelist.h"
#include "filesettings.h"
#include "importproject.h"
#include "projectfile.h"
#include "utils.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <string>
#include <utility>
#include <vector>

#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QIODevice>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>

static void addHeaders(const QString& file1, QSet<QString> &allFiles) {
    if (allFiles.contains(file1))
        return;
    QFile file(file1);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    allFiles << file1;
    const QRegularExpression re("^#include[ ]*\"([^\">]+)\".*");
    QTextStream in(&file);
    QString line = in.readLine();
    while (!in.atEnd()) {
        if (line.startsWith("#include")) {
            const QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                QString hfile = match.captured(1);
                if (file1.contains("/"))
                    hfile = file1.mid(0,file1.lastIndexOf("/") + 1) + hfile;
                addHeaders(hfile, allFiles);
            }
        }
        line = in.readLine();
    }
}

static std::vector<std::string> toStdStringList(const QStringList& from) {
    std::vector<std::string> ret;
    std::transform(from.cbegin(), from.cend(), std::back_inserter(ret), [](const QString& e) {
        return e.toStdString();
    });
    return ret;
}

ComplianceReportDialog::ComplianceReportDialog(ProjectFile* projectFile, QString resultsFile, QString checkersReport)
    : QDialog(nullptr),
    mUI(new Ui::ComplianceReportDialog),
    mProjectFile(projectFile),
    mResultsFile(std::move(resultsFile)),
    mCheckersReport(std::move(checkersReport))
{
    mUI->setupUi(this);
    mUI->mEditProjectName->setText(projectFile->getProjectName());
    connect(mUI->buttonBox, &QDialogButtonBox::clicked, this, &ComplianceReportDialog::buttonClicked);
    mUI->mCodingStandard->clear();
    if (!projectFile->getCodingStandards().contains("misra-c-2023") && projectFile->getAddons().contains("misra"))
        mUI->mCodingStandard->addItem("Misra C 2012");
    for (QString std: projectFile->getCodingStandards()) {
        std[0] = std[0].toUpper();
        std = std.replace("-", " ").replace(" c ", " C ").replace(" cpp ", " C++ ").replace(" c++ ", " C++ ");
        mUI->mCodingStandard->addItem(std);
    }
}

ComplianceReportDialog::~ComplianceReportDialog()
{
    delete mUI;
}

void ComplianceReportDialog::buttonClicked(QAbstractButton* button)
{
    switch (mUI->buttonBox->standardButton(button)) {
    case QDialogButtonBox::StandardButton::Save:
        save();
        break;
    case QDialogButtonBox::StandardButton::Close:
        close();
        break;
    default:
        break;
    }
}

void ComplianceReportDialog::save()
{
    const QString std(mUI->mCodingStandard->currentText().toLower().replace(" ", "-"));

    const QString outFile = QFileDialog::getSaveFileName(this,
                                                         tr("Compliance report"),
                                                         QDir::homePath() + "/" + std + "-compliance-report.html",
                                                         tr("HTML files (*.html)"));
    if (outFile.isEmpty())
        return;

    close();

    const QString& projectName = mUI->mEditProjectName->text();
    const QString& projectVersion = mUI->mEditProjectVersion->text();
    const bool files = mUI->mCheckFiles->isChecked();

    if (projectName != mProjectFile->getProjectName()) {
        mProjectFile->setProjectName(projectName);
        mProjectFile->write();
    }

    QTemporaryFile tempCheckersReport;
    if (tempCheckersReport.open()) {
        QTextStream out(&tempCheckersReport);
        out << mCheckersReport << "\n";
        tempCheckersReport.close();
    }

    QTemporaryFile tempFiles;
    if (files && tempFiles.open()) {
        QTextStream out(&tempFiles);
        FileList fileList;
        fileList.addPathList(mProjectFile->getCheckPaths());
        if (!mProjectFile->getImportProject().isEmpty()) {
            QFileInfo inf(mProjectFile->getFilename());

            QString prjfile;
            if (QFileInfo(mProjectFile->getImportProject()).isAbsolute())
                prjfile = mProjectFile->getImportProject();
            else
                prjfile = inf.canonicalPath() + '/' + mProjectFile->getImportProject();

            ImportProject p;
            try {
                p.import(prjfile.toStdString());
            } catch (InternalError &e) {
                QMessageBox msg(QMessageBox::Critical,
                                tr("Save compliance report"),
                                tr("Failed to import '%1' (%2), can not show files in compliance report").arg(prjfile).arg(QString::fromStdString(e.errorMessage)),
                                QMessageBox::Ok,
                                this);
                msg.exec();
                return;
            }

            p.ignorePaths(toStdStringList(mProjectFile->getExcludedPaths()));

            QDir dir(inf.absoluteDir());
            for (const FileSettings& fs: p.fileSettings)
                fileList.addFile(dir.relativeFilePath(QString::fromStdString(fs.filename())));
        }

        QSet<QString> allFiles;
        for (const QString &sourcefile: fileList.getFileList())
            addHeaders(sourcefile, allFiles);
        for (const QString& fileName: utils::as_const(allFiles)) {
            QFile f(fileName);
            if (f.open(QFile::ReadOnly)) {
                QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
                if (hash.addData(&f)) {
                    for (auto b: hash.result())
                        out << QString::number(static_cast<unsigned char>(b),16);
                    out << " " << fileName << "\n";
                }
            }
        }
        tempFiles.close();
    }

    QStringList suppressions;
    for (const auto& suppression: mProjectFile->getSuppressions()) {
        if (!suppression.errorId.empty())
            suppressions.append(QString::fromStdString(suppression.errorId));
    }

    QStringList args{"--project-name=" + projectName,
                     "--project-version=" + projectVersion,
                     "--output-file=" + outFile,
                     "--checkers-report=" + tempCheckersReport.fileName()};
    if (!suppressions.isEmpty())
        args << "--suppressions=" + suppressions.join(",");

    args << ("--" + std);

    if (files)
        args << "--files=" + tempFiles.fileName();
    args << mResultsFile;

    const QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).canonicalPath();

    QProcess process;
#ifdef Q_OS_WIN
    process.start(appPath + "/compliance-report.exe", args);
#else
    process.start(appPath + "/compliance-report", args);
#endif
    process.waitForFinished();
    const QString output = process.readAll();
    if (!output.isEmpty()) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Save compliance report"),
                        output,
                        QMessageBox::Ok,
                        this);
        msg.exec();
    }
}
