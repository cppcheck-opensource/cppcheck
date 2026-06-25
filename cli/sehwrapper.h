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

#ifndef CPPCHECKEXECUTORSEH_H
#define CPPCHECKEXECUTORSEH_H

#include "config.h" // IWYU pragma: keep

#ifdef USE_WINDOWS_SEH

#include <cstdio>

/**
 * @param f Output file
 */
void set_seh_output(FILE* f);

namespace internal
{
    int filter_seh_exeception(int code, void* ex);
}

/**
 * Signal/SEH handling
 * Has to be clean for using with SEH on windows, i.e. no construction of C++ object instances is allowed!
 */
#define CALL_WITH_SEH_WRAPPER(f) \
    __try {                      \
        return (f);              \
    } __except (internal::filter_seh_exeception(GetExceptionCode(), GetExceptionInformation())) { \
        return -1; \
    }

#endif

#endif // CPPCHECKEXECUTORSEH_H
