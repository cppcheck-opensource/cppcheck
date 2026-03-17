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

#include "threadexecutor.h"

#ifdef HAS_THREADING_MODEL_THREAD

#include "config.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "filesettings.h"
#include "settings.h"
#include "suppressions.h"
#include "timer.h"

#include <cassert>
#include <cstdlib>
#include <future>
#include <iostream>
#include <list>
#include <numeric>
#include <mutex>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

ThreadExecutor::ThreadExecutor(const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger, TimerResults* timerResults, CppCheck::ExecuteCmdFn executeCommand)
    : Executor(files, fileSettings, settings, suppressions, errorLogger, timerResults)
    , mExecuteCommand(std::move(executeCommand))
{
    assert(mSettings.jobs > 1);
}

class SyncLogForwarder : public ErrorLogger
{
public:
    explicit SyncLogForwarder(ThreadExecutor &threadExecutor, ErrorLogger &errorLogger)
        : mThreadExecutor(threadExecutor), mErrorLogger(errorLogger) {}

    void reportOut(const std::string &outmsg, Color c) override
    {
        std::lock_guard<std::mutex> lg(mReportSync);

        mErrorLogger.reportOut(outmsg, c);
    }

    void reportErr(const ErrorMessage &msg) override {
        if (!mThreadExecutor.hasToLog(msg))
            return;

        std::lock_guard<std::mutex> lg(mReportSync);
        mErrorLogger.reportErr(msg);
    }

    void reportMetric(const std::string &metric) override {
        std::lock_guard<std::mutex> lg(mReportSync);
        mErrorLogger.reportMetric(metric);
    }

    void reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal) {
        std::lock_guard<std::mutex> lg(mReportSync);
        mThreadExecutor.reportStatus(fileindex, filecount, sizedone, sizetotal);
    }

private:
    std::mutex mReportSync;
    ThreadExecutor &mThreadExecutor;
    ErrorLogger &mErrorLogger;
};

class ThreadData
{
public:
    ThreadData(ThreadExecutor &threadExecutor, ErrorLogger &errorLogger, TimerResults *timerResults, const Settings &settings, Suppressions& supprs, const std::list<FileWithDetails> &files, const std::list<FileSettings> &fileSettings, CppCheck::ExecuteCmdFn executeCommand)
        : mFiles(files), mFileSettings(fileSettings), mTimerResults(timerResults), mSettings(settings), mSuppressions(supprs), mExecuteCommand(std::move(executeCommand)), mLogForwarder(threadExecutor, errorLogger)
    {
        mItNextFile = mFiles.begin();
        mItNextFileSettings = mFileSettings.begin();

        mTotalFiles = mFiles.size() + mFileSettings.size();
        mTotalFileSize = std::accumulate(mFiles.cbegin(), mFiles.cend(), std::size_t(0), [](std::size_t v, const FileWithDetails& p) {
            return v + p.size();
        });
    }

    bool next(const FileWithDetails *&file, const FileSettings *&fs, std::size_t &fileSize) {
        std::lock_guard<std::mutex> l(mFileSync);
        if (mItNextFile != mFiles.end()) {
            file = &(*mItNextFile);
            fs = nullptr;
            fileSize = mItNextFile->size();
            ++mItNextFile;
            return true;
        }
        if (mItNextFileSettings != mFileSettings.end()) {
            file = nullptr;
            fs = &(*mItNextFileSettings);
            fileSize = 0;
            ++mItNextFileSettings;
            return true;
        }

        return false;
    }

    unsigned int check(const FileWithDetails *file, const FileSettings *fs) {
        CppCheck fileChecker(mSettings, mSuppressions, mLogForwarder, mTimerResults, false, mExecuteCommand);

        unsigned int result;
        if (fs) {
            // file settings..
            result = fileChecker.check(*fs);
        } else {
            // Read file from a file
            result = fileChecker.check(*file);
        }
        for (const auto& suppr : mSuppressions.nomsg.getSuppressions()) {
            // need to transfer all inline suppressions because these are used later on
            if (suppr.isInline) {
                const std::string err = mSuppressions.nomsg.addSuppression(suppr);
                if (!err.empty()) {
                    // TODO: only update state if it doesn't exist - otherwise propagate error
                    mSuppressions.nomsg.updateSuppressionState(suppr); // TODO: check result
                }
                continue;
            }

            // propagate state of global suppressions
            if (!suppr.isLocal()) {
                mSuppressions.nomsg.updateSuppressionState(suppr); // TODO: check result
                continue;
            }
        }
        return result;
    }

    void status(std::size_t fileSize) {
        std::lock_guard<std::mutex> l(mFileSync);
        mProcessedSize += fileSize;
        mProcessedFiles++;
        if (!mSettings.quiet)
            mLogForwarder.reportStatus(mProcessedFiles, mTotalFiles, mProcessedSize, mTotalFileSize);
    }

private:
    const std::list<FileWithDetails> &mFiles;
    std::list<FileWithDetails>::const_iterator mItNextFile;
    const std::list<FileSettings> &mFileSettings;
    std::list<FileSettings>::const_iterator mItNextFileSettings;

    std::size_t mProcessedFiles{};
    std::size_t mTotalFiles{};
    std::size_t mProcessedSize{};
    std::size_t mTotalFileSize{};

    std::mutex mFileSync;
    TimerResults *mTimerResults;
    const Settings &mSettings;
    Suppressions &mSuppressions;
    CppCheck::ExecuteCmdFn mExecuteCommand;

    SyncLogForwarder mLogForwarder;
};

static unsigned int STDCALL threadProc(ThreadData *data)
{
    unsigned int result = 0;

    const FileWithDetails *file;
    const FileSettings *fs;
    std::size_t fileSize;

    while (data->next(file, fs, fileSize)) {
        result += data->check(file, fs);

        data->status(fileSize);
    }

    return result;
}

unsigned int ThreadExecutor::check()
{
    std::vector<std::future<unsigned int>> threadFutures;
    threadFutures.reserve(mSettings.jobs);

    ThreadData data(*this, mErrorLogger, mTimerResults, mSettings, mSuppressions, mFiles, mFileSettings, mExecuteCommand);

    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        try {
            threadFutures.emplace_back(std::async(std::launch::async, &threadProc, &data));
        }
        catch (const std::system_error &e) {
            std::cerr << "#### ThreadExecutor::check exception :" << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    unsigned int result = std::accumulate(threadFutures.begin(), threadFutures.end(), 0U, [](unsigned int v, std::future<unsigned int>& f) {
        return v + f.get();
    });

    return result;
}

#endif // HAS_THREADING_MODEL_THREAD
