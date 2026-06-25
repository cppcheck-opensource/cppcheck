/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

//---------------------------------------------------------------------------
#ifndef standardsH
#define standardsH
//---------------------------------------------------------------------------

#include "config.h"

#include <cstdint>
#include <string>

/// @addtogroup Core
/// @{


/**
 * @brief This is just a container for standards settings.
 * This struct contains all possible standards that cppcheck recognize.
 */
struct CPPCHECKLIB Standards {
    enum Language : std::uint8_t { None, C, CPP };

    /** C code standard */
    enum cstd_t : std::uint8_t { C89, C99, C11, C17, C23, C2Y, CLatest = C2Y } c = CLatest;

    /** C++ code standard */
    enum cppstd_t : std::uint8_t { CPP03, CPP11, CPP14, CPP17, CPP20, CPP23, CPP26, CPPLatest = CPP26 } cpp = CPPLatest;

    /** --std value given on command line */
    std::string stdValueC;

    /** --std value given on command line */
    std::string stdValueCPP;

    bool setC(std::string str);
    std::string getC() const;
    static std::string getC(cstd_t c_std);
    static cstd_t getC(const std::string &std);
    bool setCPP(std::string str);
    std::string getCPP() const;
    static std::string getCPP(cppstd_t std);
    static cppstd_t getCPP(const std::string &std);
    bool setStd(const std::string& str);
};

/// @}
//---------------------------------------------------------------------------
#endif // standardsH
