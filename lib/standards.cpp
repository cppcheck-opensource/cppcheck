/*
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

#include "standards.h"
#include "utils.h"

#include <utility>

#include <simplecpp.h>

static Standards::cstd_t mapC(simplecpp::cstd_t cstd) {
    switch (cstd)
    {
    case simplecpp::C89:
        return Standards::C89;
    case simplecpp::C99:
        return Standards::C99;
    case simplecpp::C11:
        return Standards::C11;
    case simplecpp::C17:
        return Standards::C17;
    case simplecpp::C23:
        return Standards::C23;
    case simplecpp::C2Y:
        return Standards::C2Y;
    case simplecpp::CUnknown:
        return Standards::CLatest;
    }
    cppcheck::unreachable();
}

bool Standards::setC(std::string str)
{
    if (str.empty())
        return false;
    const simplecpp::cstd_t c_new = simplecpp::getCStd(str);
    const bool b = (c_new != simplecpp::CUnknown);
    if (b) {
        c = mapC(c_new);
        stdValueC = std::move(str);
    }
    return b;
}

std::string Standards::getC() const
{
    return getC(c);
}

std::string Standards::getC(cstd_t c_std)
{
    switch (c_std) {
    case C89:
        return "c89";
    case C99:
        return "c99";
    case C11:
        return "c11";
    case C17:
        return "c17";
    case C23:
        return "c23";
    case C2Y:
        return "c2y";
    }
    return "";
}

Standards::cstd_t Standards::getC(const std::string &std)
{
    return mapC(simplecpp::getCStd(std));
}

static Standards::cppstd_t mapCPP(simplecpp::cppstd_t cppstd) {
    switch (cppstd)
    {
    case simplecpp::CPP03:
        return Standards::CPP03;
    case simplecpp::CPP11:
        return Standards::CPP11;
    case simplecpp::CPP14:
        return Standards::CPP14;
    case simplecpp::CPP17:
        return Standards::CPP17;
    case simplecpp::CPP20:
        return Standards::CPP20;
    case simplecpp::CPP23:
        return Standards::CPP23;
    case simplecpp::CPP26:
        return Standards::CPP26;
    case simplecpp::CPPUnknown:
        return Standards::CPPLatest;
    }
    cppcheck::unreachable();
}

bool Standards::setCPP(std::string str)
{
    if (str.empty())
        return false;
    const simplecpp::cppstd_t cpp_new = simplecpp::getCppStd(str);
    const bool b = (cpp_new != simplecpp::CPPUnknown);
    if (b) {
        cpp = mapCPP(cpp_new);
        stdValueCPP = std::move(str);
    }
    return b;
}

std::string Standards::getCPP() const
{
    return getCPP(cpp);
}

std::string Standards::getCPP(cppstd_t std)
{
    switch (std) {
    case CPP03:
        return "c++03";
    case CPP11:
        return "c++11";
    case CPP14:
        return "c++14";
    case CPP17:
        return "c++17";
    case CPP20:
        return "c++20";
    case CPP23:
        return "c++23";
    case CPP26:
        return "c++26";
    }
    return "";
}

Standards::cppstd_t Standards::getCPP(const std::string &std)
{
    return mapCPP(simplecpp::getCppStd(std));
}

bool Standards::setStd(const std::string& str)
{
    return setC(str) || setCPP(str);
}
