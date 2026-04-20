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

#include "executor.h"

#include "color.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "settings.h"
#include "suppressions.h"

#include <cassert>
#include <sstream>
#include <utility>

struct FileSettings;

Executor::Executor(const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger, TimerResults* timerResults)
    : mFiles(files), mFileSettings(fileSettings), mSettings(settings), mSuppressions(suppressions), mErrorLogger(errorLogger), mTimerResults(timerResults)
{
    // the two inputs may only be used exclusively
    assert(!(!files.empty() && !fileSettings.empty()));
}

// TODO: this logic is duplicated in CppCheck::reportErr()
bool Executor::hasToLog(const ErrorMessage &msg)
{
    if (msg.severity == Severity::internal)
        return true;

    if (!mSuppressions.nomsg.isSuppressed(msg, {}))
    {
        // TODO: there should be no need for verbose and default messages here
        std::string errmsg = msg.toString(mSettings.verbose, mSettings.templateFormat, mSettings.templateLocation);
        if (errmsg.empty())
            return false;

        if (mSettings.emitDuplicates)
            return true;

        std::lock_guard<std::mutex> lg(mErrorListSync);
        if (mErrorList.emplace(std::move(errmsg)).second) {
            return true;
        }
    }
    return false;
}

void Executor::reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal)
{
    if (filecount > 1) {
        std::ostringstream oss;
        const unsigned long percentDone = (sizetotal > 0) ? (100 * sizedone) / sizetotal : (100 * fileindex / filecount);
        oss << fileindex << '/' << filecount
            << " files checked " << percentDone
            << "% done";
        mErrorLogger.reportOut(oss.str(), Color::FgBlue);
    }
}

