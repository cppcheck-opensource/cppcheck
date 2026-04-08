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
#ifndef regexH
#define regexH
//---------------------------------------------------------------------------

#ifdef HAVE_RULES

#include "config.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

class CPPCHECKLIB Regex
{
public:
    virtual ~Regex() = default;

    using MatchFn = std::function<void (int start, int end)>;
    virtual std::string match(const std::string& str, const MatchFn& matchFn) const = 0;

    enum class Engine : std::uint8_t
    {
        Unknown = 0,
#ifdef HAVE_PCRE
        Pcre = 1,
#endif
#ifdef HAVE_PCRE2
        Pcre2 = 2
#endif
    };

    static std::shared_ptr<Regex> create(std::string pattern, Engine engine, std::string& err);
};

#endif // HAVE_RULES

#endif // regexH
