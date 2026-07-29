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

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "config.h"

#include <cstddef>
#include <list>
#include <mutex>
#include <string>
#include <unordered_set>

class Settings;
class ErrorLogger;
class ErrorMessage;
struct Suppressions;
struct FileSettings;
class FileWithDetails;
class TimerResults;

/// @addtogroup CLI
/// @{

/**
 * This class will take a list of filenames and settings and check then
 * all files using threads.
 */
class Executor {
public:
    Executor(const std::list<FileWithDetails> &files LIFETIMEBOUND, const std::list<FileSettings>& fileSettings LIFETIMEBOUND, const Settings &settings LIFETIMEBOUND, Suppressions &suppressions LIFETIMEBOUND, ErrorLogger &errorLogger LIFETIMEBOUND, TimerResults* timerResults LIFETIMEBOUND);
    virtual ~Executor() = default;

    Executor(const Executor &) = delete;
    Executor& operator=(const Executor &) = delete;

    virtual unsigned int check() = 0;

    /**
     * Information about how many files have been checked
     *
     * @param fileindex This many files have been checked.
     * @param filecount This many files there are in total.
     * @param sizedone The sum of sizes of the files checked.
     * @param sizetotal The total sizes of the files.
     */
    void reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal);

protected:
    /**
     * @brief Check if message is being suppressed and unique.
     * @param msg the message to check
     * @return true if message is not suppressed and unique
     */
    bool hasToLog(const ErrorMessage &msg);

    const std::list<FileWithDetails> &mFiles;
    const std::list<FileSettings>& mFileSettings;
    const Settings &mSettings;
    Suppressions &mSuppressions;
    ErrorLogger &mErrorLogger;
    TimerResults *mTimerResults;

private:
    std::mutex mErrorListSync;
    // TODO: store hashes instead of the full messages
    std::unordered_set<std::string> mErrorList;
};

/// @}

#endif // EXECUTOR_H
