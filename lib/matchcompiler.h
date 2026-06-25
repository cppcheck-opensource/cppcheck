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

//---------------------------------------------------------------------------
#ifndef matchcompilerH
#define matchcompilerH

#include <string>

namespace MatchCompiler {

    template<unsigned int n>
    class ConstString {
    public:
        using StringRef = const char (&)[n];
        explicit ConstString(StringRef s)
            : _s(s) {}

        operator StringRef() const {
            return _s;
        }

    private:
        StringRef _s;
    };

    template<unsigned int n>
    inline bool equalN(const char s1[], const char s2[])
    {
        return (*s1 == *s2) && equalN<n-1>(s1+1, s2+1);
    }

    template<>
    inline bool equalN<0>(const char /*s1*/[], const char /*s2*/[])
    {
        return true;
    }

    template<unsigned int n>
    inline bool operator==(const std::string & s1, ConstString<n> const & s2)
    {
        return equalN<n>(s1.c_str(), s2);
    }

    template<unsigned int n>
    inline bool operator!=(const std::string & s1, ConstString<n> const & s2)
    {
        return !operator==(s1,s2);
    }

    template<unsigned int n>
    inline ConstString<n> makeConstString(const char (&s)[n])
    {
        return ConstString<n>(s);
    }
}

#endif // matchcompilerH

