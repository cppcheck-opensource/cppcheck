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

#ifndef ruleH
#define ruleH

#ifdef HAVE_RULES

#include "errortypes.h"

#include <memory>
#include <string>

class Regex;

/** Rule */
struct Rule
{
    std::string tokenlist = "normal"; // use normal tokenlist
    std::string pattern;
    std::string id = "rule"; // default id
    std::string summary;
    Severity severity = Severity::style; // default severity
    std::shared_ptr<Regex> regex;
};
#endif // HAVE_RULES

#endif // ruleH
