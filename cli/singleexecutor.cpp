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

#include "singleexecutor.h"

#include "cppcheck.h"
#include "filesettings.h"
#include "settings.h"
#include "timer.h"

#include <cassert>
#include <cstddef>
#include <list>
#include <numeric>

class ErrorLogger;

SingleExecutor::SingleExecutor(CppCheck &cppcheck, const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger, TimerResults* timerResults)
    : Executor(files, fileSettings, settings, suppressions, errorLogger, timerResults)
    , mCppcheck(cppcheck)
{
    assert(mSettings.jobs == 1);
}

// TODO: markup handling is not performed with multiple jobs
unsigned int SingleExecutor::check()
{
    unsigned int result = 0;

    const std::size_t totalfilesize = std::accumulate(mFiles.cbegin(), mFiles.cend(), std::size_t(0), [](std::size_t v, const FileWithDetails& f) {
        return v + f.size();
    });

    std::size_t processedsize = 0;
    unsigned int c = 0;

    for (auto i = mFiles.cbegin(); i != mFiles.cend(); ++i) {
        result += mCppcheck.check(*i);
        processedsize += i->size();
        ++c;
        if (!mSettings.quiet)
            reportStatus(c, mFiles.size(), processedsize, totalfilesize);
        // TODO: call analyseClangTidy()?
    }

    // filesettings
    // check all files of the project
    for (const FileSettings &fs : mFileSettings) {
        result += mCppcheck.check(fs);
        ++c;
        if (!mSettings.quiet)
            reportStatus(c, mFileSettings.size(), c, mFileSettings.size());
    }

    // TODO: show time after the whole program analysis
    // TODO: CppCheckExecutor::check_internal() is also invoking the whole program analysis - is it run twice?
    if (mCppcheck.analyseWholeProgram())
        result++;

    return result;
}
