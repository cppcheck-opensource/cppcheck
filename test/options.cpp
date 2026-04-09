// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2025 Cppcheck team.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "options.h"

#include "timer.h"

options::options(int argc, const char* const argv[])
    : mExe(argv[0])
{
    const std::set<std::string> args(argv + 1, argv + argc);
    for (const auto& arg : args) {
        if (arg.empty())
            continue; // empty argument
        if (arg[0] == '-') {
            if (arg == "-q")
                mQuiet = true;
            else if (arg == "-h" || arg == "--help")
                mHelp = true;
            else if (arg == "-n")
                mSummary = false;
            else if (arg == "-d")
                mDryRun = true;
            else if (arg == "-x")
                mExcludeTests = true;
            else if (arg == "-t")
                mTimerResults.reset(new TimerResults);
            else
                mErrors.emplace_back("unknown option '" + arg + "'");
            continue; // command-line switch
        }
        const auto pos = arg.find("::");
        if (pos == std::string::npos) {
            mWhichTests[arg] = {}; // run whole fixture
            continue;
        }
        const std::string fixture = arg.substr(0, pos);
        const auto it = mWhichTests.find(fixture);
        if (it != mWhichTests.cend() && it->second.empty())
            continue; // whole fixture is already included
        const std::string test = arg.substr(pos+2);
        mWhichTests[fixture].emplace(test); // run individual test
    }
}

options::~options()
{
    // TODO: allow more than 5 results to be shown
    // TODO: provide higher resolution in output
    // TODO: disable the metrics
    if (mTimerResults)
        mTimerResults->showResults(ShowTime::TOP5_FILE);
}

bool options::quiet() const
{
    return mQuiet;
}

bool options::help() const
{
    return mHelp;
}

bool options::summary() const
{
    return mSummary;
}

bool options::dry_run() const
{
    return mDryRun;
}

const std::map<std::string, std::set<std::string>>& options::which_tests() const
{
    return mWhichTests;
}

const std::string& options::exe() const
{
    return mExe;
}

bool options::exclude_tests() const
{
    return mExcludeTests;
}

TimerResultsIntf* options::timer_results() const
{
    return mTimerResults.get();
}

const std::vector<std::string>& options::errors() const
{
    return mErrors;
}
