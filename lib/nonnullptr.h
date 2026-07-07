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

//---------------------------------------------------------------------------
#ifndef nonnullptrH
#define nonnullptrH
//---------------------------------------------------------------------------

/**
 * Pointer wrapper that can never be null, similar to gsl::not_null.
 *
 * It is constructed from a reference and converts implicitly back to one
 * (like std::reference_wrapper), so it can replace a reference data member
 * without changing the constructor or call sites that pass the member to
 * functions taking T&. Unlike a reference member it is copyable and
 * assignable; assignment rebinds it.
 */
template<class T>
class NonNullPtr {
public:
    using pointer = T*;
    using element_type = T;

    // cppcheck-suppress noExplicitConstructor
    // NOLINTNEXTLINE(google-explicit-constructor)
    NonNullPtr(T& r) : mPtr(&r) {}
    NonNullPtr(T&& r) = delete;

    template<class U>
    // cppcheck-suppress noExplicitConstructor
    // NOLINTNEXTLINE(google-explicit-constructor)
    NonNullPtr(const NonNullPtr<U>& other) : mPtr(other.get()) {}

    T* get() const noexcept {
        return mPtr;
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator T*() const noexcept {
        return mPtr;
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator T&() const noexcept {
        return *mPtr;
    }

    T& operator*() const noexcept {
        return *mPtr;
    }

    T* operator->() const noexcept {
        return mPtr;
    }

private:
    T* mPtr;
};

#endif
