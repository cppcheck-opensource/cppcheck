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

#ifndef smallvectorH
#define smallvectorH

#include <cstddef>

static constexpr std::size_t DefaultSmallVectorSize = 3;

#ifdef HAVE_BOOST
#include <boost/container/small_vector.hpp> // IWYU pragma: export

template<typename T, std::size_t N = DefaultSmallVectorSize>
using SmallVector = boost::container::small_vector<T, N>;
#else
#include <utility>
#include <vector>

template<class T, std::size_t N>
struct TaggedAllocator : std::allocator<T>
{
    template<class ... Ts>
    // NOLINTNEXTLINE(google-explicit-constructor)
    TaggedAllocator(Ts && ... ts)
        : std::allocator<T>(std::forward<Ts>(ts)...)
    {}

    template<class U>
    // cppcheck-suppress noExplicitConstructor
    // NOLINTNEXTLINE(google-explicit-constructor)
    TaggedAllocator(const TaggedAllocator<U, N> /*unused*/) {}

    template<class U>
    struct rebind
    {
        using other = TaggedAllocator<U, N>;
    };
};

template<typename T, std::size_t N = DefaultSmallVectorSize>
class SmallVector : public std::vector<T, TaggedAllocator<T, N>>
{
public:
    template<class ... Ts>
    // NOLINTNEXTLINE(google-explicit-constructor)
    SmallVector(Ts&&... ts)
        : std::vector<T, TaggedAllocator<T, N>>(std::forward<Ts>(ts)...)
    {
        this->reserve(N);
    }
};
#endif

#endif
