/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#ifndef CMD_LINE_LOGGER_H
#define CMD_LINE_LOGGER_H

#include <string>

class CmdLineLogger
{
public:
    virtual ~CmdLineLogger() = default;

    /** print a regular message */
    virtual void printMessage(const std::string &message) = 0;
    /** print an error message */
    virtual void printError(const std::string &message) = 0;
    /** print to the output */
    virtual void printRaw(const std::string &message) = 0;
};

#endif // CMD_LINE_LOGGER_H
