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

#ifndef addonInfoH
#define addonInfoH

#include "config.h"

#include <string>
#include <map>

struct CPPCHECKLIB AddonInfo {
    std::string name;
    std::string scriptFile; // addon script
    std::string executable; // addon executable
    std::string args;       // special extra arguments
    std::string python;     // script interpreter
    bool ctu = false;
    std::string runScript;
    std::map<std::string, std::string> checkers; // checker name and its requirement

    std::string getAddonInfo(const std::string &fileName, const std::string &exename, bool debug = false);
};

#endif // addonInfoH
