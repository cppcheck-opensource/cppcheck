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


#ifndef THREADHANDLER_H
#define THREADHANDLER_H

#include "settings.h"
#include "suppressions.h"
#include "threadresult.h"

#include <cstddef>
#include <list>
#include <memory>
#include <set>
#include <string>

#include <QDateTime>
#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

class ResultsView;
class CheckThread;
class QSettings;
class ImportProject;
class ErrorItem;
class FileWithDetails;

/// @addtogroup GUI
/// @{


/**
 * @brief This class handles creating threadresult and starting threads
 *
 */
class ThreadHandler : public QObject {
    Q_OBJECT
public:
    explicit ThreadHandler(QObject *parent = nullptr);
    ~ThreadHandler() override;

    /**
     * @brief Initialize the threads (connect all signals to resultsview's slots)
     *
     * @param view View to show error results
     */
    void initialize(const ResultsView *view);

    /**
     * @brief Load settings
     * @param settings QSettings to load settings from
     */
    void loadSettings(const QSettings &settings);

    /**
     * @brief Save settings
     * @param settings QSettings to save settings to
     */
    void saveSettings(QSettings &settings) const;

    void setAddonsAndTools(const QStringList &addonsAndTools) {
        mAddonsAndTools = addonsAndTools;
    }

    void setSuppressions(const QList<SuppressionList::Suppression> &s) {
        mSuppressionsUI = s;
    }

    void setClangIncludePaths(const QStringList &s) {
        mClangIncludePaths = s;
    }

    /**
     * @brief Clear all files from cppcheck
     *
     */
    void clearFiles();

    /**
     * @brief Set files to check
     *
     * @param files files to check
     */
    void setFiles(std::list<FileWithDetails> files);

    /**
     * @brief Set project to check
     *
     * @param prj project to check
     */
    void setProject(const ImportProject &prj);

    /**
     * @brief Start the threads to check the files
     *
     * @param settings Settings for checking
     * @param supprs Suppressions for checking
     */
    void check(const Settings &settings, const std::shared_ptr<Suppressions>& supprs);

    /**
     * @brief Set files to check
     *
     * @param all true if all files, false if modified files
     */
    void setCheckFiles(bool all);

    /**
     * @brief Set selected files to check
     *
     * @param files list of files to be checked
     */
    void setCheckFiles(std::list<FileWithDetails> files);

    /**
     * @brief Is checking running?
     *
     * @return true if check is running, false otherwise.
     */
    bool isChecking() const;

    /**
     * @brief Have we checked files already?
     *
     * @return true check has been previously run and recheck can be done
     */
    bool hasPreviousFiles() const;

    /**
     * @brief Return count of files we checked last time.
     *
     * @return count of files that were checked last time.
     */
    int getPreviousFilesCount() const;

    /**
     * @brief Return the time elapsed while scanning the previous time.
     *
     * @return the time elapsed in milliseconds.
     */
    int getPreviousScanDuration() const;

    /**
     * @brief Get files that should be rechecked because they have been
     * changed.
     */
    std::list<FileWithDetails> getReCheckFiles(bool all) const;

    /**
     * @brief Get start time of last check
     *
     * @return start time of last check
     */
    QDateTime getCheckStartTime() const;

    /**
     * @brief Set start time of check
     *
     * @param checkStartTime saved start time of the last check
     */
    void setCheckStartTime(QDateTime checkStartTime);

    /**
     * @brief Emit the threadDetailsUpdated signal
     */
    void emitThreadDetailsUpdated();

signals:
    /**
     * @brief Signal that all threads are done
     *
     */
    void done();

    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void log(const QString &msg);

    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void debugError(const ErrorItem &item);

    /**
     * @brief Emitted when thread details are updated.
     */
    void threadDetailsUpdated(QMap<int, CheckThread::Details> threadDetails);

public slots:

    /**
     * @brief Slot to stop all threads
     *
     */
    void stop();

    /**
     * @brief Slot threads use to signal this class that it started checking a file
     * @param details Details about what file is being checked and by what thread
     */
    void startCheck(CheckThread::Details details);

    /**
     * @brief Slot threads use to signal this class that it finish checking a file
     * @param details Details about what file finished being checked and by what thread
     */
    void finishCheck(CheckThread::Details details);

signals:
    void progress(QString filename, QString stage, std::size_t value);

protected slots:
    /**
     * @brief Slot that a single thread is done
     *
     */
    void threadDone();
protected:
    /**
     * @brief List of files checked last time (used when rechecking)
     *
     */
    std::list<FileWithDetails> mLastFiles;

    /** @brief date and time when current checking started */
    QDateTime mCheckStartTime;

    /**
     * @brief when was the files checked the last time (used when rechecking)
     */
    QDateTime mLastCheckTime;

    /**
     * @brief Timer used for measuring scan duration
     *
     */
    QElapsedTimer mTimer;

    /**
     * @brief The previous scan duration in milliseconds.
     *
     */
    int mScanDuration{};

    /**
     * @brief Create checker threads
     * @param count The number of threads to spawn
     */
    void createThreads(int count);

    /**
     * @brief Function to delete all threads
     *
     */
    void removeThreads();

    /*
     * @brief Apply check settings to a checker thread
     * @param thread The thread to setup
     */
    void setupCheckThread(CheckThread &thread) const;

    /**
     * @brief Thread results are stored here
     *
     */
    ThreadResult mResults;

    /**
     * @brief List of threads currently in use
     *
     */
    QList<CheckThread *> mThreads;

    /**
     * @brief The amount of threads currently running
     *
     */
    int mRunningThreadCount{};

    /**
     * @brief A whole program check is queued by check()
     */
    bool mAnalyseWholeProgram{};
    std::string mCtuInfo;

    QStringList mAddonsAndTools;
    QList<SuppressionList::Suppression> mSuppressionsUI;
    QStringList mClangIncludePaths;

    /// @{
    /**
     * @brief Settings specific to the current analysis
     */
    QStringList mCheckAddonsAndTools;
    Settings mCheckSettings;
    std::shared_ptr<Suppressions> mCheckSuppressions;
    /// @}

    /**
     * @brief Details about currently running threads
     */
    QMap<int, CheckThread::Details> mThreadDetails;

private:

    /**
     * @brief Check if a file needs to be rechecked. Recursively checks
     * included headers. Used by GetReCheckFiles()
     */
    bool needsReCheck(const QString &filename, std::set<QString> &modified, std::set<QString> &unmodified) const;
};
/// @}
#endif // THREADHANDLER_H
