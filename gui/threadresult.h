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


#ifndef THREADRESULT_H
#define THREADRESULT_H

#include "color.h"
#include "errorlogger.h"
#include "filesettings.h"
#include "checkthread.h"

#include <cstddef>
#include <list>
#include <mutex>
#include <string>

#include <QObject>
#include <QString>
#include <QStringList>

class ErrorItem;
class ImportProject;

/// @addtogroup GUI
/// @{

/**
 * @brief Threads use this class to obtain new files to process and to publish results
 *
 */
class ThreadResult : public QObject, public ErrorLogger {
    Q_OBJECT
public:
    ThreadResult() = default;

    /**
     * @brief Get next unprocessed file
     */
    void getNextFile(const FileWithDetails*& file);

    void getNextFileSettings(const FileSettings*& fs);

    /**
     * @brief Set list of files to check
     * @param files List of files to check
     */
    void setFiles(std::list<FileWithDetails> files);

    void setProject(const ImportProject &prj);

    /**
     * @brief Clear files to check
     *
     */
    void clearFiles();

    /**
     * @brief Get the number of files to check
     *
     */
    int getFileCount() const;

    /**
     * ErrorLogger methods
     */
    void reportOut(const std::string &outmsg, Color /*c*/ = Color::Reset) override;
    void reportErr(const ErrorMessage &msg) override;
    void reportMetric(const std::string &metric) override
    {
        (void) metric;
    }
    // NOLINTNEXTLINE(readability-avoid-const-params-in-decls) - false positive this is an overload
    void reportProgress(const std::string &filename, const char stage[], const std::size_t value) final;

public slots:

    /**
     * @brief Slot threads use to signal this class that it finish checking a file
     * @param details Details about what file finished being checked and by what thread
     */
    void finishCheck(CheckThread::Details details);

signals:
    /**
     * @brief Files checked progress
     * @param value Current progress (0 - PROGRESS_MAX)
     * @param description Description of the current stage (example: "13/45 files checked")
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void filesCheckedProgress(int value, const QString& description);

    /**
     * @brief Signal of a new error
     *
     * @param item Error data
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void error(const ErrorItem &item);

    /**
     * @brief Signal of a new log message
     *
     * @param logline Log line
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void log(const QString &logline);

    /**
     * @brief Signal of a debug error
     *
     * @param item Error data
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void debugError(const ErrorItem &item);

    void progress(QString filename, QString stage, std::size_t value);

protected:

    /**
     * @brief Mutex
     *
     */
    mutable std::mutex mutex;

    /**
     * @brief List of files to check
     *
     */
    std::list<FileWithDetails> mFiles;
    std::list<FileWithDetails>::const_iterator mItNextFile{mFiles.cbegin()};

    std::list<FileSettings> mFileSettings;
    std::list<FileSettings>::const_iterator mItNextFileSettings{mFileSettings.cbegin()};

    /** @brief Total file size */
    quint64 mTotalFileSize{};

    /** @brief File size of checked files */
    quint64 mCheckedFileSize{};

    /**
     * @brief Current number of files checked
     *
     */
    unsigned long mFilesChecked{};

    /**
     * @brief Total number of files
     *
     */
    unsigned long mTotalFiles{};
};
/// @}
#endif // THREADRESULT_H
