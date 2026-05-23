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
#ifndef platformH
#define platformH
//---------------------------------------------------------------------------

#include "config.h"
#include "mathlib.h"
#include "standards.h"
#include "utils.h"

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/// @addtogroup Core
/// @{

namespace tinyxml2 {
    class XMLDocument;
}

/**
 * @brief Platform settings
 */
class CPPCHECKLIB Platform {
private:
    static long long min_value(std::uint8_t bit) {
        assert(bit > 0);
        if (bit >= 64)
            return LLONG_MIN;
        return -(1LL << (bit-1));
    }

    static long long max_value(std::uint8_t bit) {
        assert(bit > 0);
        if (bit >= 64)
            return (~0ULL) >> 1;
        return (1LL << (bit-1)) - 1LL;
    }

    static unsigned long long max_value_unsigned(std::uint8_t bit) {
        assert(bit > 0);
        if (bit >= 64)
            return ~0ULL;
        return (1ULL << bit) - 1ULL;
    }

    void calculateBitMembers() {
        short_bit = char_bit * sizeof_short;
        int_bit = char_bit * sizeof_int;
        long_bit = char_bit * sizeof_long;
        long_long_bit = char_bit * sizeof_long_long;
        float_bit = char_bit * sizeof_float;
        double_bit = char_bit * sizeof_double;
        long_double_bit = char_bit * sizeof_long_double;
    }

    /** provides list of defines specified by the limit.h/climits includes */
    std::string getLimitsDefines(bool c99) const;

protected:
    /** load platform from xml document, primarily for testing */
    bool loadFromXmlDocument(const tinyxml2::XMLDocument *doc);
public:
    Platform();

    bool isIntValue(MathLib::bigint value) const {
        return value >= min_value(int_bit) && value <= max_value(int_bit);
    }

    bool isIntValue(MathLib::biguint value) const {
        const unsigned long long intMax = max_value(int_bit);
        return value <= intMax;
    }

    bool isLongValue(MathLib::bigint value) const {
        return value >= min_value(long_bit) && value <= max_value(long_bit);
    }

    bool isLongValue(MathLib::biguint value) const {
        const MathLib::biguint longMax = max_value(long_bit);
        return value <= longMax;
    }

    bool isLongLongValue(MathLib::biguint value) const {
        const MathLib::biguint longLongMax = max_value(long_long_bit);
        return value <= longLongMax;
    }

    std::uint8_t char_bit{};        /// bits in char
    std::uint8_t short_bit{};       /// bits in short
    std::uint8_t int_bit{};         /// bits in int
    std::uint8_t long_bit{};        /// bits in long
    std::uint8_t long_long_bit{};   /// bits in long long
    std::uint8_t float_bit{};       /// bits in float
    std::uint8_t double_bit{};      /// bits in double
    std::uint8_t long_double_bit{}; /// bits in long double

    /** size of standard types */
    std::size_t sizeof_bool{};
    std::size_t sizeof_short{};
    std::size_t sizeof_int{};
    std::size_t sizeof_long{};
    std::size_t sizeof_long_long{};
    std::size_t sizeof_float{};
    std::size_t sizeof_double{};
    std::size_t sizeof_long_double{};
    std::size_t sizeof_wchar_t{};
    std::size_t sizeof_size_t{};
    std::size_t sizeof_pointer{};

    char defaultSign{};  // unsigned:'u', signed:'s', unknown:'\0'

    bool windows{false}; // indicates if the platform is Windows

    enum Type : std::uint8_t {
        Unspecified, // No platform specified
        Native, // whatever system this code was compiled on
        Win32A,
        Win32W,
        Win64,
        Unix32,
        Unix64,
        File
    };

    /** platform type */
    Type type{};

    /** set the platform type for predefined platforms - deprecated use set(const std::string&, std::string&) instead */
    bool set(Type t);

    /** set the platform type */
    bool set(const std::string& platformstr, std::string& errstr, const std::vector<std::string>& paths = {}, bool debug = false);

    /**
     * load platform file
     * @param paths the additional paths to look into
     * @param filename platform filename
     * @param debug log verbose information about the lookup
     * @return returns true if file was loaded successfully
     */
    bool loadFromFile(const std::vector<std::string>& paths, const std::string &filename, bool debug = false);


    /**
     * @brief Returns true if platform type is Windows
     * @return true if Windows platform type.
     */
    bool isWindows() const {
        return windows;
    }

    const char *toString() const {
        return toString(type);
    }

    static const char *toString(Type pt) {
        switch (pt) {
        case Type::Unspecified:
            return "unspecified";
        case Type::Native:
            return "native";
        case Type::Win32A:
            return "win32A";
        case Type::Win32W:
            return "win32W";
        case Type::Win64:
            return "win64";
        case Type::Unix32:
            return "unix32";
        case Type::Unix64:
            return "unix64";
        case Type::File:
            return "platformFile";
        }
        cppcheck::unreachable();
    }

    long long unsignedCharMax() const {
        return max_value(char_bit + 1);
    }

    long long signedCharMax() const {
        return max_value(char_bit);
    }

    long long signedCharMin() const {
        return min_value(char_bit);
    }

    /** provides list of defines specified by the limit.h/climits includes */
    std::string getLimitsDefines(Standards::cstd_t cstd) const;
    /** provides list of defines specified by the limit.h/climits includes */
    std::string getLimitsDefines(Standards::cppstd_t cppstd) const;
};

/// @}
//---------------------------------------------------------------------------
#endif // platformH
