// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2024 Cppcheck team.
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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <map>
#include <set>
#include <string>
#include <vector>

/**
 * @brief Class to parse command-line parameters for ./testrunner .
 * Has getters for available switches and parameters.
 * See test/testoptions.cpp for sample usage.
 */
class options {
public:
    /** Call from main() to populate object */
    options(int argc, const char* const argv[]);
    /** Don't print the name of each method being tested. */
    bool quiet() const;
    /** Print help. */
    bool help() const;
    /** Print summary. */
    bool summary() const;
    /** Perform dry run. */
    bool dry_run() const;
    /** Exclude provided lists of tests. */
    bool exclude_tests() const;
    /** Which tests should be run. */
    const std::map<std::string, std::set<std::string>>& which_tests() const;

    /** Errors encountered during option processing. */
    const std::vector<std::string>& errors() const;

    const std::string& exe() const;

    options() = delete;
    options(const options&) = delete;
    options& operator =(const options&) = delete;

private:
    std::map<std::string, std::set<std::string>> mWhichTests;
    std::vector<std::string> mErrors;
    bool mQuiet{};
    bool mHelp{};
    bool mSummary{true};
    bool mDryRun{};
    bool mExcludeTests{};
    std::string mExe;
};

#endif
