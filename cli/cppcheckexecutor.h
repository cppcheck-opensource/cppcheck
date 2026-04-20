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

#ifndef CPPCHECKEXECUTOR_H
#define CPPCHECKEXECUTOR_H

#include "filesettings.h"

#include <list>
#include <string>
#include <vector>

class Settings;
class ErrorLogger;
class SuppressionList;
struct Suppressions;

/**
 * This class works as an example of how CppCheck can be used in external
 * programs without very little knowledge of the internal parts of the
 * program itself. If you wish to use cppcheck e.g. as a part of IDE,
 * just rewrite this class for your needs and possibly use other methods
 * from CppCheck class instead the ones used here.
 */
class CppCheckExecutor {
public:
    /**
     * Constructor
     */
    CppCheckExecutor() = default;
    CppCheckExecutor(const CppCheckExecutor &) = delete;
    CppCheckExecutor& operator=(const CppCheckExecutor&) = delete;

    /**
     * Starts the checking.
     *
     * @param argc from main()
     * @param argv from main()
     * @return EXIT_FAILURE if arguments are invalid or no input files
     *         were found.
     *         If errors are found and --error-exitcode is used,
     *         given value is returned instead of default 0.
     *         If no errors are found, 0 is returned.
     */
    int check(int argc, const char* const argv[]);

protected:

    /**
     * Execute a shell command and read the output from it. Returns exitcode of the executed command,.
     */
    static int executeCommand(std::string exe, std::vector<std::string> args, std::string redirect, std::string &output_);

    static bool reportUnmatchedSuppressions(const Settings &settings, const SuppressionList& suppressions, const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, ErrorLogger& errorLogger);

private:
    /**
     * Wrapper around check_internal
     *   - installs optional platform dependent signal handling
     *
     * @param settings the settings
     * @param supprs the suppressions
     **/
    int check_wrapper(const Settings& settings, Suppressions& supprs);

    /**
     * Starts the checking.
     *
     * @param settings the settings
     * @param supprs the suppressions
     * @return EXIT_FAILURE if arguments are invalid or no input files
     *         were found.
     *         If errors are found and --error-exitcode is used,
     *         given value is returned instead of default 0.
     *         If no errors are found, 0 is returned.
     */
    int check_internal(const Settings& settings, Suppressions& supprs) const;

    /**
     * Filename associated with size of file
     */
    std::list<FileWithDetails> mFiles;

    std::list<FileSettings> mFileSettings;
};

#endif // CPPCHECKEXECUTOR_H
