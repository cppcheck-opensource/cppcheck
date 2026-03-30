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

#include "threadhandler.h"

#include "checkthread.h"
#include "common.h"
#include "filesettings.h"
#include "resultsview.h"
#include "settings.h"
#include "utils.h"

#include <algorithm>
#include <string>
#include <unordered_set>
#include <utility>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QSettings>
#include <QTextStream>
#include <QVariant>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#include <QtLogging>
#endif

ThreadHandler::ThreadHandler(QObject *parent) :
    QObject(parent)
{}

ThreadHandler::~ThreadHandler()
{
    removeThreads();
}

void ThreadHandler::clearFiles()
{
    mLastFiles.clear();
    mResults.clearFiles();
    mAnalyseWholeProgram = false;
    mCtuInfo.clear();
    mAddonsAndTools.clear();
    mSuppressionsUI.clear();
}

void ThreadHandler::setFiles(std::list<FileWithDetails> files)
{
    mLastFiles = files;
    mResults.setFiles(std::move(files));
}

void ThreadHandler::setProject(const ImportProject &prj)
{
    mResults.setProject(prj);
    mLastFiles.clear();
}

void ThreadHandler::setCheckFiles(bool all)
{
    if (mRunningThreadCount == 0) {
        mResults.setFiles(getReCheckFiles(all));
    }
}

void ThreadHandler::setCheckFiles(std::list<FileWithDetails> files)
{
    if (mRunningThreadCount == 0) {
        mResults.setFiles(std::move(files));
    }
}

void ThreadHandler::setupCheckThread(CheckThread &thread) const
{
    thread.setAddonsAndTools(mCheckAddonsAndTools);
    thread.setSuppressions(mSuppressionsUI);
    thread.setClangIncludePaths(mClangIncludePaths);
    thread.setSettings(mCheckSettings, mCheckSuppressions);
}

void ThreadHandler::check(const Settings &settings, const std::shared_ptr<Suppressions>& supprs)
{
    if (mResults.getFileCount() == 0 || mRunningThreadCount > 0 || settings.jobs == 0) {
        qDebug() << "Can't start checking if there's no files to check or if check is in progress.";
        emit done();
        return;
    }

    mCheckSettings = settings;
    mCheckSuppressions = supprs;

    createThreads(mCheckSettings.jobs);

    mRunningThreadCount = mThreads.size();
    mRunningThreadCount = std::min(mResults.getFileCount(), mRunningThreadCount);

    mCheckAddonsAndTools = mAddonsAndTools;
    for (const std::string& addon: mCheckSettings.addons) {
        QString s = QString::fromStdString(addon);
        if (!mCheckAddonsAndTools.contains(s))
            mCheckAddonsAndTools << s;
    }

    mCtuInfo.clear();

    for (int i = 0; i < mRunningThreadCount; i++) {
        setupCheckThread(*mThreads[i]);
        mThreads[i]->start();
    }

    // Date and time when checking starts..
    mCheckStartTime = QDateTime::currentDateTime();

    mAnalyseWholeProgram = true;

    mTimer.start();
}

bool ThreadHandler::isChecking() const
{
    return mRunningThreadCount > 0 || mAnalyseWholeProgram;
}

void ThreadHandler::createThreads(const int count)
{
    if (mRunningThreadCount > 0 || count <= 0) {
        return;
    }

    //Remove unused old threads
    removeThreads();
    //Create new threads
    for (int i = mThreads.size(); i < count; i++) {
        mThreads << new CheckThread(mResults, i + 1);
        connect(mThreads.last(), &CheckThread::done,
                this, &ThreadHandler::threadDone, Qt::QueuedConnection);
        connect(mThreads.last(), &CheckThread::finishCheck,
                &mResults, &ThreadResult::finishCheck, Qt::QueuedConnection);
        connect(mThreads.last(), &CheckThread::startCheck,
                this, &ThreadHandler::startCheck, Qt::QueuedConnection);
        connect(mThreads.last(), &CheckThread::finishCheck,
                this, &ThreadHandler::finishCheck, Qt::QueuedConnection);
    }
}


void ThreadHandler::removeThreads()
{
    for (CheckThread* thread : utils::as_const(mThreads)) {
        if (thread->isRunning()) {
            thread->stop();
            thread->wait();
        }
        disconnect(thread, &CheckThread::done,
                   this, &ThreadHandler::threadDone);
        disconnect(thread, &CheckThread::finishCheck,
                   &mResults, &ThreadResult::finishCheck);
        disconnect(mThreads.last(), &CheckThread::startCheck,
                   this, &ThreadHandler::startCheck);
        disconnect(mThreads.last(), &CheckThread::finishCheck,
                   this, &ThreadHandler::finishCheck);
        delete thread;
    }

    mThreads.clear();
}

void ThreadHandler::threadDone()
{
    mRunningThreadCount--;

    // TODO: also run with projects?
    if (mRunningThreadCount == 0 && mAnalyseWholeProgram) {
        createThreads(1);
        mRunningThreadCount = 1;
        setupCheckThread(*mThreads[0]);
        mThreads[0]->analyseWholeProgram(mLastFiles, mCtuInfo);
        mAnalyseWholeProgram = false;
        mCtuInfo.clear();
        return;
    }

    if (mRunningThreadCount == 0) {
        emit done();

        mScanDuration = mTimer.elapsed();

        // Set date/time used by the recheck
        if (!mCheckStartTime.isNull()) {
            mLastCheckTime = mCheckStartTime;
            mCheckStartTime = QDateTime();
        }

        mCheckAddonsAndTools.clear();
        mCheckSuppressions.reset();
    }
}

void ThreadHandler::stop()
{
    mCheckStartTime = QDateTime();
    mAnalyseWholeProgram = false;
    mCtuInfo.clear();
    for (CheckThread* thread : utils::as_const(mThreads)) {
        thread->stop();
    }
}

void ThreadHandler::initialize(const ResultsView *view)
{
    connect(&mResults, &ThreadResult::filesCheckedProgress,
            view, &ResultsView::filesCheckedProgress);

    connect(&mResults, &ThreadResult::error,
            view, &ResultsView::error);

    connect(&mResults, &ThreadResult::log,
            this, &ThreadHandler::log);

    connect(&mResults, &ThreadResult::debugError,
            this, &ThreadHandler::debugError);

    connect(&mResults, &ThreadResult::progress,
            this, &ThreadHandler::progress);
}

void ThreadHandler::loadSettings(const QSettings &settings)
{
    createThreads(settings.value(SETTINGS_CHECK_THREADS, 1).toInt());
}

void ThreadHandler::saveSettings(QSettings &settings) const
{
    settings.setValue(SETTINGS_CHECK_THREADS, mThreads.size());
}

bool ThreadHandler::hasPreviousFiles() const
{
    return !mLastFiles.empty();
}

int ThreadHandler::getPreviousFilesCount() const
{
    return mLastFiles.size();
}

int ThreadHandler::getPreviousScanDuration() const
{
    return mScanDuration;
}

std::list<FileWithDetails> ThreadHandler::getReCheckFiles(bool all) const
{
    if (mLastCheckTime.isNull() || all)
        return mLastFiles;

    std::set<QString> modified;
    std::set<QString> unmodified;

    std::list<FileWithDetails> files;
    std::copy_if(mLastFiles.cbegin(), mLastFiles.cend(), std::back_inserter(files), [&](const FileWithDetails &f) {
        return needsReCheck(QString::fromStdString(f.path()), modified, unmodified);
    });
    return files;
}

bool ThreadHandler::needsReCheck(const QString &filename, std::set<QString> &modified, std::set<QString> &unmodified) const
{
    if (modified.find(filename) != modified.end())
        return true;

    if (unmodified.find(filename) != unmodified.end())
        return false;

    if (QFileInfo(filename).lastModified() > mLastCheckTime) {
        return true;
    }

    // Parse included files recursively
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    // prevent recursion..
    unmodified.insert(filename);

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("#include \"")) {
            line.remove(0,10);
            const int i = line.indexOf("\"");
            if (i > 0) {
                line.remove(i,line.length());
                line = QFileInfo(filename).absolutePath() + "/" + line;
                if (needsReCheck(line, modified, unmodified)) {
                    modified.insert(std::move(line));
                    return true;
                }
            }
        }
    }

    return false;
}

QDateTime ThreadHandler::getCheckStartTime() const
{
    return mCheckStartTime;
}

void ThreadHandler::setCheckStartTime(QDateTime checkStartTime)
{
    mCheckStartTime = std::move(checkStartTime);
}

// cppcheck-suppress passedByValueCallback
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ThreadHandler::startCheck(CheckThread::Details details)
{
    mThreadDetails[details.threadIndex] = details;
    emitThreadDetailsUpdated();
}

// cppcheck-suppress passedByValueCallback
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ThreadHandler::finishCheck(CheckThread::Details details)
{
    mThreadDetails.remove(details.threadIndex);
    emitThreadDetailsUpdated();
}

void ThreadHandler::emitThreadDetailsUpdated()
{
    emit threadDetailsUpdated(mThreadDetails);
}
