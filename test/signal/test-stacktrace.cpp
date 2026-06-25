/*
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

#include "config.h"

#ifdef USE_UNIX_BACKTRACE_SUPPORT
#include "stacktrace.h"

#include <cstdio>

// static functions are omitted from trace

/*static*/ void my_func_2() // NOLINT(misc-use-internal-linkage)
{
    print_stacktrace(stdout, 0, true, -1, true);
}

/*static*/ void my_func() // NOLINT(misc-use-internal-linkage)
{
    my_func_2();
}
#endif

int main()
{
#ifdef USE_UNIX_BACKTRACE_SUPPORT
    my_func();

    return 0;
#else
    return 1;
#endif
}
