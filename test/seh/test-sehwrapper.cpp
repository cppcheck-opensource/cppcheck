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

#include "config.h" // IWYU pragma: keep

#if defined(USE_WINDOWS_SEH)
#include "sehwrapper.h"

#include <cassert>
#include <cfenv>
#include <cstdlib>
#include <cstring>

#include <windows.h>

static int my_assert()
{
    assert(false);
    return 0;
}

static int my_abort()
{
    abort();
}

static int my_segv()
{
    // cppcheck-suppress nullPointer
    ++*(int*)nullptr;
    return 0;
}

// TODO: how to catch these?
static int my_fpe()
{
    const int fpe_res = std::feraiseexcept(FE_ALL_EXCEPT);
    if (fpe_res != 0)
        return 11;
    return sqrt(-1.0); // invalid operation
}
#endif

int main(int argc, const char * const argv[])
{
#if defined(USE_WINDOWS_SEH)
    if (argc != 2)
        return 1;

    if (strcmp(argv[1], "assert") == 0) {
        _set_abort_behavior(0, _WRITE_ABORT_MSG); // suppress the "Debug Error!" MessageBox
        CALL_WITH_SEH_WRAPPER(my_assert());
    }
    if (strcmp(argv[1], "abort") == 0) {
        _set_abort_behavior(0, _WRITE_ABORT_MSG); // suppress the "Debug Error!" MessageBox
        CALL_WITH_SEH_WRAPPER(my_abort());
    }
    if (strcmp(argv[1], "fpe") == 0) {
        CALL_WITH_SEH_WRAPPER(my_fpe());
    }
    if (strcmp(argv[1], "segv") == 0) {
        CALL_WITH_SEH_WRAPPER(my_segv());
    }

    return 0;
#else
    (void)argc;
    (void)argv;
    return 111;
#endif
}
