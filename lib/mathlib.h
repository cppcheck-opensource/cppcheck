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
#ifndef mathlibH
#define mathlibH
//---------------------------------------------------------------------------

#include "config.h"

#include <cstdint>
#include <string>

#if defined(HAVE_BOOST) && defined(HAVE_BOOST_INT128)
#include <boost/multiprecision/cpp_int.hpp>
#endif

class Token;

/// @addtogroup Core
/// @{

/** @brief simple math functions that uses operands stored in std::string. useful when performing math on tokens. */

class CPPCHECKLIB MathLib {
public:
#if defined(HAVE_BOOST) && defined(HAVE_BOOST_INT128)
    using bigint = boost::multiprecision::int128_t;
    using biguint = boost::multiprecision::uint128_t;
#else
    using bigint = long long;
    using biguint = unsigned long long;
#endif

    /** @brief value class */
    class value {
    private:
        bigint mIntValue{};
        double mDoubleValue{};
        enum class Type : std::uint8_t { INT, LONG, LONGLONG, FLOAT } mType;
        bool mIsUnsigned{};

        void promote(const value &v);

    public:
        /**
         * @throws InternalError thrown on invalid value
         */
        explicit value(const std::string &s);
        std::string str() const;
        bool isInt() const {
            return mType != Type::FLOAT;
        }
        bool isFloat() const {
            return mType == Type::FLOAT;
        }

        double getDoubleValue() const {
            return isFloat() ? mDoubleValue : static_cast<double>(mIntValue);
        }

        /**
         * @throws InternalError thrown on invalid/unhandled calculation or division by zero
         */
        static value calc(char op, const value &v1, const value &v2);
        int compare(const value &v) const;
        value add(int v) const;
        /**
         * @throws InternalError thrown if operand is not an integer
         */
        value shiftLeft(const value &v) const;
        /**
         * @throws InternalError thrown if operand is not an integer
         */
        value shiftRight(const value &v) const;
    };

    static const int bigint_bits;

    /** @brief for conversion of numeric literals - for atoi-like conversions please use strToInt() */
    static bigint toBigNumber(const Token * tok);
    /** @brief for conversion of numeric literals - for atoi-like conversions please use strToInt()
     * @throws InternalError thrown if conversion failed
     */
    static bigint toBigNumber(const std::string & str, const Token *tok = nullptr);
    /** @brief for conversion of numeric literals - for atoi-like conversions please use strToInt() */
    static biguint toBigUNumber(const Token * tok);
    /** @brief for conversion of numeric literals - for atoi-like conversions please use strToInt()
     * @throws InternalError thrown if conversion failed
     */
    static biguint toBigUNumber(const std::string & str, const Token *tok = nullptr);

    template<class T> static std::string toString(T value) = delete;
    /** @brief for conversion of numeric literals */
    static double toDoubleNumber(const Token * tok);
    /** @brief for conversion of numeric literals
     * @throws InternalError thrown if conversion failed
     */
    static double toDoubleNumber(const std::string & str, const Token * tok = nullptr);

    static bool isInt(const std::string & str);
    static bool isFloat(const std::string &str);
    static bool isDecimalFloat(const std::string &str);
    static bool isNegative(const std::string &str);
    static bool isPositive(const std::string &str);
    static bool isDec(const std::string & str);
    static bool isFloatHex(const std::string& str);
    static bool isIntHex(const std::string& str);
    static bool isOct(const std::string& str);
    static bool isBin(const std::string& str);

    static std::string getSuffix(const std::string& value);
    /**
     * Only used in unit tests
     *
     * \param[in] str string
     * \param[in] supportMicrosoftExtensions support Microsoft extension: i64
     *  \return true if str is a non-empty valid integer suffix
     */
    static bool isValidIntegerSuffix(const std::string& str, bool supportMicrosoftExtensions=true);

    static std::string add(const std::string & first, const std::string & second);
    static std::string subtract(const std::string & first, const std::string & second);
    static std::string multiply(const std::string & first, const std::string & second);
    /**
     * @throws InternalError thrown on overflow or division by zero
     */
    static std::string divide(const std::string & first, const std::string & second);
    /**
     * @throws InternalError thrown on division by zero
     */
    static std::string mod(const std::string & first, const std::string & second);
    /**
     * @throws InternalError thrown on unexpected action
     */
    static std::string calculate(const std::string & first, const std::string & second, char action);

    static std::string sin(const std::string & tok);
    static std::string cos(const std::string & tok);
    static std::string tan(const std::string & tok);
    static std::string abs(const std::string & tok);
    static bool isEqual(const std::string & first, const std::string & second);
    static bool isNotEqual(const std::string & first, const std::string & second);
    static bool isGreater(const std::string & first, const std::string & second);
    static bool isGreaterEqual(const std::string & first, const std::string & second);
    static bool isLess(const std::string & first, const std::string & second);
    static bool isLessEqual(const std::string & first, const std::string & second);
    static bool isNullValue(const std::string & str);
    /**
     * Return true if given character is 0,1,2,3,4,5,6 or 7.
     * @param[in] c The character to check
     * @return true if given character is octal digit.
     */
    static bool isOctalDigit(char c);

    static unsigned int encodeMultiChar(const std::string& str);
};

MathLib::value operator+(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator-(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator*(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator/(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator%(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator&(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator|(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator^(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator<<(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator>>(const MathLib::value &v1, const MathLib::value &v2);

template<> CPPCHECKLIB std::string MathLib::toString<MathLib::bigint>(MathLib::bigint value);
template<> CPPCHECKLIB std::string MathLib::toString<MathLib::biguint>(MathLib::biguint value);
template<> CPPCHECKLIB std::string MathLib::toString<double>(double value);

/// @}
//---------------------------------------------------------------------------
#endif // mathlibH
