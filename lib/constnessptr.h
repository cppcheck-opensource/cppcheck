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
#ifndef constnessPtrH
#define constnessPtrH
//---------------------------------------------------------------------------

#include "config.h"

// as std::optional behaves similarly so we could use that instead of if we ever move to C++17.
// it is not a simple drop-in as our "operator bool()" indicates if the pointer is non-null
// whereas std::optional indicates if a value is set.
//
// This is similar to std::experimental::propagate_const
// see https://en.cppreference.com/w/cpp/experimental/propagate_const
template<typename T>
class constness_ptr
{
public:
    explicit constness_ptr(T* p)
        : mPtr(p)
    {}

    T* get() NOEXCEPT {
        return mPtr;
    }

    const T* get() const NOEXCEPT {
        return mPtr;
    }

    T* operator->() NOEXCEPT {
        return mPtr;
    }

    const T* operator->() const NOEXCEPT {
        return mPtr;
    }

    T& operator*() NOEXCEPT {
        return *mPtr;
    }

    const T& operator*() const NOEXCEPT {
        return *mPtr;
    }

    explicit operator bool() const NOEXCEPT {
        return mPtr != nullptr;
    }

private:
    T* mPtr;
};

#endif // constnessPtrH
