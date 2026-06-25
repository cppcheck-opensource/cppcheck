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

#ifndef colorH
#define colorH

#include "config.h"

#include <cstdint>
#include <ostream>
#include <string>

enum class Color : std::uint8_t {
    Reset      = 0,
    Bold       = 1,
    Dim        = 2,
    FgRed      = 31,
    FgGreen    = 32,
    FgBlue     = 34,
    FgMagenta  = 35,
    FgDefault  = 39
};
CPPCHECKLIB std::ostream& operator<<(std::ostream& os, Color c);

CPPCHECKLIB std::string toString(Color c);

extern CPPCHECKLIB bool gDisableColors; // for testing

#endif
