/* -*- C++ -*-
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


#ifndef CHECKTHREAD_H
#define CHECKTHREAD_H

#include "filesettings.h"
#include "settings.h"
#include "suppressions.h"

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTime>

class ThreadResult;

/// @addtogroup GUI
/// @{

/**
 * @brief Thread to run cppcheck
 *
 */
class CheckThread : public QThread {
    Q_OBJECT
public:
    struct Details {
        int threadIndex;
        QString file;
        QTime startTime;
    };

public:
    CheckThread(ThreadResult &result, int threadIndex);

    /**
     * @brief Set settings for cppcheck
     *
     * @param settings settings for cppcheck
     * @param supprs suppressions for cppcheck
     */
    void setSettings(const Settings &settings, std::shared_ptr<Suppressions> supprs);

    /**
     * @brief Run whole program analysis
     * @param files    All files
     * @param ctuInfo  Ctu info for addons
     */
    void analyseWholeProgram(const std::list<FileWithDetails> &files, const std::string& ctuInfo);

    void setAddonsAndTools(const QStringList &addonsAndTools) {
        mAddonsAndTools = addonsAndTools;
    }

    void setClangIncludePaths(const QStringList &s) {
        mClangIncludePaths = s;
    }

    void setSuppressions(const QList<SuppressionList::Suppression> &s) {
        mSuppressionsUi = s;
    }

    void stop();

    /**
     * Determine command to run clang
     * \return Command to run clang, empty if it is not found
     */
    static QString clangCmd();

    /**
     * Determine command to run clang-tidy
     * \return Command to run clang-tidy, empty if it is not found
     */
    static QString clangTidyCmd();

    static int executeCommand(std::string exe, std::vector<std::string> args, std::string redirect, std::string &output);

signals:

    /**
     * @brief cpp checking is done
     *
     */
    void done();

    void startCheck(CheckThread::Details details);
    void finishCheck(CheckThread::Details details);
protected:

    /**
     * @brief States for the check thread.
     * Whole purpose of these states is to allow stopping of the checking. When
     * stopping we say for the thread (Stopping) that stop when current check
     * has been completed. Thread must be stopped cleanly, just terminating thread
     * likely causes unpredictable side-effects.
     */
    enum State : std::uint8_t {
        Running, /**< The thread is checking. */
        Stopping, /**< The thread will stop after current work. */
        Stopped, /**< The thread has been stopped. */
        Ready, /**< The thread is ready. */
    };

    /**
     * @brief Thread's current execution state. Can be changed from outside
     */
    std::atomic<State> mState{Ready};

    ThreadResult &mResult;
    int mThreadIndex{};

    Settings mSettings;
    std::shared_ptr<Suppressions> mSuppressions;

private:
    /**
     * @brief method that is run in a thread
     *
     */
    void run() override;

    void runAddonsAndTools(const Settings& settings, const FileSettings *fileSettings, const QString &fileName);

    void parseClangErrors(const QString &tool, const QString &file0, QString err);

    bool isSuppressed(const SuppressionList::ErrorMessage &errorMessage) const;

    std::list<FileWithDetails> mFiles;
    bool mAnalyseWholeProgram{};
    std::string mCtuInfo;
    QStringList mAddonsAndTools;
    QStringList mClangIncludePaths;
    QList<SuppressionList::Suppression> mSuppressionsUi;
};
Q_DECLARE_METATYPE(CheckThread::Details);
/// @}
#endif // CHECKTHREAD_H
