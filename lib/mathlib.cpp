/*
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


#include "mathlib.h"
#include "errortypes.h"
#include "token.h"
#include "utils.h"

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <numeric>

#include <simplecpp.h>


const int MathLib::bigint_bits = 64;

MathLib::value::value(const std::string &s)
{
    if (MathLib::isFloat(s)) {
        mType = MathLib::value::Type::FLOAT;
        mDoubleValue = MathLib::toDoubleNumber(s);
        return;
    }

    if (!MathLib::isInt(s))
        throw InternalError(nullptr, "Invalid value: " + s);

    mType = MathLib::value::Type::INT;
    mIntValue = MathLib::toBigNumber(s);

    if (isIntHex(s) && mIntValue < 0)
        mIsUnsigned = true;

    // read suffix
    if (s.size() >= 2U) {
        for (std::size_t i = s.size() - 1U; i > 0U; --i) {
            const char c = s[i];
            if (c == 'u' || c == 'U')
                mIsUnsigned = true;
            else if (c == 'l' || c == 'L') {
                if (mType == MathLib::value::Type::INT)
                    mType = MathLib::value::Type::LONG;
                else if (mType == MathLib::value::Type::LONG)
                    mType = MathLib::value::Type::LONGLONG;
            } else if (i > 2U && c == '4' && s[i-1] == '6' && s[i-2] == 'i')
                mType = MathLib::value::Type::LONGLONG;
        }
    }
}

std::string MathLib::value::str() const
{
    std::ostringstream ostr;
    if (mType == MathLib::value::Type::FLOAT) {
        if (std::isnan(mDoubleValue))
            return "nan.0";
        if (std::isinf(mDoubleValue))
            return (mDoubleValue > 0) ? "inf.0" : "-inf.0";

        ostr.precision(9);
        ostr << std::fixed << mDoubleValue;

        // remove trailing zeros
        std::string ret(ostr.str());
        std::string::size_type pos = ret.size() - 1U;
        while (ret[pos] == '0')
            pos--;
        if (ret[pos] == '.')
            ++pos;

        return ret.substr(0, pos+1);
    }

    if (mIsUnsigned)
        ostr << static_cast<biguint>(mIntValue) << "U";
    else
        ostr << mIntValue;
    if (mType == MathLib::value::Type::LONG)
        ostr << "L";
    else if (mType == MathLib::value::Type::LONGLONG)
        ostr << "LL";
    return ostr.str();
}

void MathLib::value::promote(const MathLib::value &v)
{
    if (isInt() && v.isInt()) {
        if (mType < v.mType) {
            mType = v.mType;
            mIsUnsigned = v.mIsUnsigned;
        } else if (mType == v.mType) {
            mIsUnsigned |= v.mIsUnsigned;
        }
    } else if (!isFloat()) {
        mIsUnsigned = false;
        mDoubleValue = static_cast<double>(mIntValue);
        mType = MathLib::value::Type::FLOAT;
    }
}


MathLib::value MathLib::value::calc(char op, const MathLib::value &v1, const MathLib::value &v2)
{
    value temp(v1);
    temp.promote(v2);
    if (temp.isFloat()) {
        switch (op) {
        case '+':
            temp.mDoubleValue += v2.getDoubleValue();
            break;
        case '-':
            temp.mDoubleValue -= v2.getDoubleValue();
            break;
        case '*':
            temp.mDoubleValue *= v2.getDoubleValue();
            break;
        case '/':
            temp.mDoubleValue /= v2.getDoubleValue();
            break;
        case '%':
        case '&':
        case '|':
        case '^':
            throw InternalError(nullptr, "Invalid calculation");
        default:
            throw InternalError(nullptr, "Unhandled calculation");
        }
    } else if (temp.mIsUnsigned) {
        switch (op) {
        case '+':
            temp.mIntValue += static_cast<unsigned long long>(v2.mIntValue);
            break;
        case '-':
            temp.mIntValue -= static_cast<unsigned long long>(v2.mIntValue);
            break;
        case '*':
            temp.mIntValue *= static_cast<unsigned long long>(v2.mIntValue);
            break;
        case '/':
            if (v2.mIntValue == 0)
                throw InternalError(nullptr, "Internal Error: Division by zero");
            if (v1.mIntValue == std::numeric_limits<bigint>::min() && std::abs(v2.mIntValue)<=1)
                throw InternalError(nullptr, "Internal Error: Division overflow");
            temp.mIntValue /= static_cast<unsigned long long>(v2.mIntValue);
            break;
        case '%':
            if (v2.mIntValue == 0)
                throw InternalError(nullptr, "Internal Error: Division by zero");
            temp.mIntValue %= static_cast<unsigned long long>(v2.mIntValue);
            break;
        case '&':
            temp.mIntValue &= static_cast<unsigned long long>(v2.mIntValue);
            break;
        case '|':
            temp.mIntValue |= static_cast<unsigned long long>(v2.mIntValue);
            break;
        case '^':
            temp.mIntValue ^= static_cast<unsigned long long>(v2.mIntValue);
            break;
        default:
            throw InternalError(nullptr, "Unhandled calculation");
        }
    } else {
        switch (op) {
        case '+':
            temp.mIntValue += v2.mIntValue;
            break;
        case '-':
            temp.mIntValue -= v2.mIntValue;
            break;
        case '*':
            temp.mIntValue *= v2.mIntValue;
            break;
        case '/':
            if (v2.mIntValue == 0)
                throw InternalError(nullptr, "Internal Error: Division by zero");
            if (v1.mIntValue == std::numeric_limits<bigint>::min() && std::abs(v2.mIntValue)<=1)
                throw InternalError(nullptr, "Internal Error: Division overflow");
            temp.mIntValue /= v2.mIntValue;
            break;
        case '%':
            if (v2.mIntValue == 0)
                throw InternalError(nullptr, "Internal Error: Division by zero");
            temp.mIntValue %= v2.mIntValue;
            break;
        case '&':
            temp.mIntValue &= v2.mIntValue;
            break;
        case '|':
            temp.mIntValue |= v2.mIntValue;
            break;
        case '^':
            temp.mIntValue ^= v2.mIntValue;
            break;
        default:
            throw InternalError(nullptr, "Unhandled calculation");
        }
    }
    return temp;
}


int MathLib::value::compare(const MathLib::value &v) const
{
    value temp(*this);
    temp.promote(v);

    if (temp.isFloat()) {
        if (temp.mDoubleValue < v.getDoubleValue())
            return -1;
        if (temp.mDoubleValue > v.getDoubleValue())
            return 1;
        return 0;
    }

    if (temp.mIsUnsigned) {
        if (static_cast<unsigned long long>(mIntValue) < static_cast<unsigned long long>(v.mIntValue))
            return -1;
        if (static_cast<unsigned long long>(mIntValue) > static_cast<unsigned long long>(v.mIntValue))
            return 1;
        return 0;
    }

    if (mIntValue < v.mIntValue)
        return -1;
    if (mIntValue > v.mIntValue)
        return 1;
    return 0;
}

MathLib::value MathLib::value::add(int v) const
{
    MathLib::value temp(*this);
    if (temp.isInt())
        temp.mIntValue += v;
    else
        temp.mDoubleValue += v;
    return temp;
}

MathLib::value MathLib::value::shiftLeft(const MathLib::value &v) const
{
    if (!isInt() || !v.isInt())
        throw InternalError(nullptr, "Shift operand is not integer");
    MathLib::value ret(*this);
    if (v.mIntValue >= MathLib::bigint_bits) {
        return ret;
    }
    ret.mIntValue <<= v.mIntValue;
    return ret;
}

MathLib::value MathLib::value::shiftRight(const MathLib::value &v) const
{
    if (!isInt() || !v.isInt())
        throw InternalError(nullptr, "Shift operand is not integer");
    MathLib::value ret(*this);
    if (v.mIntValue >= MathLib::bigint_bits) {
        return ret;
    }
    ret.mIntValue >>= v.mIntValue;
    return ret;
}

MathLib::biguint MathLib::toBigUNumber(const Token * tok)
{
    return toBigUNumber(tok->str(), tok);
}

// TODO: remove handling of non-literal stuff
MathLib::biguint MathLib::toBigUNumber(const std::string & str, const Token * const tok)
{
    // hexadecimal numbers:
    if (isIntHex(str)) {
        try {
            const biguint ret = std::stoull(str, nullptr, 16);
            return ret;
        } catch (const std::out_of_range& /*e*/) {
            throw InternalError(tok, "Internal Error. MathLib::toBigUNumber: out_of_range: " + str);
        } catch (const std::invalid_argument& /*e*/) {
            throw InternalError(tok, "Internal Error. MathLib::toBigUNumber: invalid_argument: " + str);
        }
    }

    // octal numbers:
    if (isOct(str)) {
        try {
            const biguint ret = std::stoull(str, nullptr, 8);
            return ret;
        } catch (const std::out_of_range& /*e*/) {
            throw InternalError(tok, "Internal Error. MathLib::toBigUNumber: out_of_range: " + str);
        } catch (const std::invalid_argument& /*e*/) {
            throw InternalError(tok, "Internal Error. MathLib::toBigUNumber: invalid_argument: " + str);
        }
    }

    // binary numbers:
    if (isBin(str)) {
        biguint ret = 0;
        for (std::string::size_type i = str[0] == '0'?2:3; i < str.length(); i++) {
            if (str[i] != '1' && str[i] != '0')
                break;
            ret <<= 1;
            if (str[i] == '1')
                ret |= 1;
        }
        if (str[0] == '-')
            ret = -ret;
        return ret;
    }

    if (isFloat(str)) {
        // Things are going to be less precise now: the value can't be represented in the biguint type.
        // Use min/max values as an approximation. See #5843
        // TODO: bail out when we are out of range?
        const double doubleval = toDoubleNumber(str, tok);
        if (doubleval > static_cast<double>(std::numeric_limits<biguint>::max()))
            return std::numeric_limits<biguint>::max();
        // cast to bigint to avoid UBSAN warning about negative double being out-of-range
        return static_cast<biguint>(static_cast<bigint>(doubleval));
    }

    if (isCharLiteral(str)) {
        try {
            return simplecpp::characterLiteralToLL(str);
        } catch (const std::runtime_error& e) {
            throw InternalError(tok, "Internal Error. MathLib::toBigUNumber: characterLiteralToLL(" + str + ") => " + e.what());
        }
    }

    try {
        std::size_t idx = 0;
        const biguint ret = std::stoull(str, &idx, 10);
        if (idx != str.size()) {
            const std::string s = str.substr(idx);
            if (!isValidIntegerSuffix(s, true))
                throw InternalError(tok, "Internal Error. MathLib::toBigUNumber: input was not completely consumed: " + str);
        }
        return ret;
    } catch (const std::out_of_range& /*e*/) {
        throw InternalError(tok, "Internal Error. MathLib::toBigUNumber: out_of_range: " + str);
    } catch (const std::invalid_argument& /*e*/) {
        throw InternalError(tok, "Internal Error. MathLib::toBigUNumber: invalid_argument: " + str);
    }
}

unsigned int MathLib::encodeMultiChar(const std::string& str)
{
    return std::accumulate(str.cbegin(), str.cend(), uint32_t(), [](uint32_t v, char c) {
        return (v << 8) | c;
    });
}

MathLib::bigint MathLib::toBigNumber(const Token * tok)
{
    return toBigNumber(tok->str(), tok);
}

// TODO: remove handling of non-literal stuff
MathLib::bigint MathLib::toBigNumber(const std::string & str, const Token * const tok)
{
    // hexadecimal numbers:
    if (isIntHex(str)) {
        try {
            const biguint ret = std::stoull(str, nullptr, 16);
            return static_cast<bigint>(ret);
        } catch (const std::out_of_range& /*e*/) {
            throw InternalError(tok, "Internal Error. MathLib::toBigNumber: out_of_range: " + str);
        } catch (const std::invalid_argument& /*e*/) {
            throw InternalError(tok, "Internal Error. MathLib::toBigNumber: invalid_argument: " + str);
        }
    }

    // octal numbers:
    if (isOct(str)) {
        try {
            const biguint ret = std::stoull(str, nullptr, 8);
            return ret;
        } catch (const std::out_of_range& /*e*/) {
            throw InternalError(tok, "Internal Error. MathLib::toBigNumber: out_of_range: " + str);
        } catch (const std::invalid_argument& /*e*/) {
            throw InternalError(tok, "Internal Error. MathLib::toBigNumber: invalid_argument: " + str);
        }
    }

    // binary numbers:
    if (isBin(str)) {
        bigint ret = 0;
        for (std::string::size_type i = str[0] == '0'?2:3; i < str.length(); i++) {
            if (str[i] != '1' && str[i] != '0')
                break;
            ret <<= 1;
            if (str[i] == '1')
                ret |= 1;
        }
        if (str[0] == '-')
            ret = -ret;
        return ret;
    }

    if (isFloat(str)) {
        // Things are going to be less precise now: the value can't be represented in the bigint type.
        // Use min/max values as an approximation. See #5843
        // TODO: bail out when we are out of range?
        const double doubleval = toDoubleNumber(str, tok);
        if (doubleval > static_cast<double>(std::numeric_limits<bigint>::max()))
            return std::numeric_limits<bigint>::max();
        if (doubleval < static_cast<double>(std::numeric_limits<bigint>::min()))
            return std::numeric_limits<bigint>::min();
        return static_cast<bigint>(doubleval);
    }

    if (isCharLiteral(str)) {
        try {
            return simplecpp::characterLiteralToLL(str);
        } catch (const std::runtime_error& e) {
            throw InternalError(tok, "Internal Error. MathLib::toBigNumber: characterLiteralToLL(" + str + ") => " + e.what());
        }
    }

    try {
        std::size_t idx = 0;
        const biguint ret = std::stoull(str, &idx, 10);
        if (idx != str.size()) {
            const std::string s = str.substr(idx);
            if (!isValidIntegerSuffix(s, true))
                throw InternalError(tok, "Internal Error. MathLib::toBigNumber: input was not completely consumed: " + str);
        }
        return ret;
    } catch (const std::out_of_range& /*e*/) {
        throw InternalError(tok, "Internal Error. MathLib::toBigNumber: out_of_range: " + str);
    } catch (const std::invalid_argument& /*e*/) {
        throw InternalError(tok, "Internal Error. MathLib::toBigNumber: invalid_argument: " + str);
    }
}

// in-place conversion of (sub)string to double. Requires no heap.
static double myStod(const std::string& str, std::string::const_iterator from, std::string::const_iterator to, int base)
{
    double result = 0.;
    bool positivesign = true;
    std::string::const_iterator it;
    if ('+' == *from) {
        it = from + 1;
    } else if ('-' == *from) {
        it = from + 1;
        positivesign = false;
    } else
        it = from;
    const std::size_t decimalsep = str.find('.', it-str.begin());
    int distance;
    if (std::string::npos == decimalsep) {
        distance = to - it;
    } else if (decimalsep > (to - str.begin()))
        return 0.; // error handling??
    else
        distance = int(decimalsep)-(from - str.begin());
    auto digitval = [&](char c) {
        if ((10 < base) && (c > '9'))
            return 10 + std::tolower(c) - 'a';
        return c - '0';
    };
    for (; it!=to; ++it) {
        if ('.' == *it)
            continue;
        --distance;
        result += digitval(*it)* std::pow(base, distance);
    }
    return positivesign ? result : -result;
}

// Assuming a limited support of built-in hexadecimal floats (see C99, C++17) that is a fall-back implementation.
// Performance has been optimized WRT to heap activity, however the calculation part is not optimized.
static double floatHexToDoubleNumber(const std::string& str)
{
    const std::size_t p = str.find_first_of("pP",3);
    const double factor1 = myStod(str, str.cbegin() + 2, str.cbegin()+p, 16);
    const bool suffix = (str.back() == 'f') || (str.back() == 'F') || (str.back() == 'l') || (str.back() == 'L');
    const double exponent = myStod(str, str.cbegin() + p + 1, suffix ? str.cend()-1:str.cend(), 10);
    const double factor2 = std::pow(2, exponent);
    return factor1 * factor2;
}

double MathLib::toDoubleNumber(const Token * tok)
{
    return toDoubleNumber(tok->str(), tok);
}

double MathLib::toDoubleNumber(const std::string &str, const Token * const tok)
{
    if (isCharLiteral(str)) {
        try {
            return simplecpp::characterLiteralToLL(str);
        } catch (const std::runtime_error& e) {
            throw InternalError(tok, "Internal Error. MathLib::toDoubleNumber: characterLiteralToLL(" + str + ") => " + e.what());
        }
    }
    if (isIntHex(str))
        return static_cast<double>(toBigNumber(str, tok));
#ifdef _LIBCPP_VERSION
    if (isFloat(str)) // Workaround libc++ bug at https://github.com/llvm/llvm-project/issues/18156
        // TODO: handle locale
        // TODO: make sure all characters are being consumed
        return std::strtod(str.c_str(), nullptr);
#endif
    if (isFloatHex(str))
        return floatHexToDoubleNumber(str);
    // otherwise, convert to double
    std::istringstream istr(str);
    istr.imbue(std::locale::classic());
    double ret;
    if (!(istr >> ret))
        throw InternalError(tok, "Internal Error. MathLib::toDoubleNumber: conversion failed: " + str);
    std::string s;
    if (istr >> s) {
        if (isDecimalFloat(str))
            return ret;
        if (!isValidIntegerSuffix(s, true))
            throw InternalError(tok, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: " + str);
    }
    return ret;
}

template<> std::string MathLib::toString<MathLib::bigint>(MathLib::bigint value)
{
#if defined(HAVE_BOOST) && defined(HAVE_BOOST_INT128)
    std::ostringstream result;
    result << value;
    return result.str();
#else
    return std::to_string(value);
#endif
}

template<> std::string MathLib::toString<MathLib::biguint>(MathLib::biguint value)
{
#if defined(HAVE_BOOST) && defined(HAVE_BOOST_INT128)
    std::ostringstream result;
    result << value;
    return result.str();
#else
    return std::to_string(value);
#endif
}

template<> std::string MathLib::toString<double>(double value)
{
    std::ostringstream result;
    result.precision(12);
    result << value;
    std::string s = result.str();
    if (s == "-0")
        s = "0.0";
    else if (s.find_first_of(".e") == std::string::npos)
        s += ".0";
    return s;
}

bool MathLib::isFloat(const std::string &str)
{
    return isDecimalFloat(str) || isFloatHex(str);
}

bool MathLib::isDecimalFloat(const std::string &str)
{
    if (str.empty())
        return false;
    enum class State : std::uint8_t {
        START, BASE_DIGITS1, LEADING_DECIMAL, TRAILING_DECIMAL, BASE_DIGITS2, E, MANTISSA_PLUSMINUS, MANTISSA_DIGITS, SUFFIX_F, SUFFIX_L, SUFFIX_LITERAL_LEADER, SUFFIX_LITERAL
    } state = State::START;
    auto it = str.cbegin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.cend(); ++it) {
        switch (state) {
        case State::START:
            if (*it=='.')
                state = State::LEADING_DECIMAL;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state = State::BASE_DIGITS1;
            else
                return false;
            break;
        case State::LEADING_DECIMAL:
            if (std::isdigit(static_cast<unsigned char>(*it)))
                state = State::BASE_DIGITS2;
            else
                return false;
            break;
        case State::BASE_DIGITS1:
            if (*it=='e' || *it=='E')
                state = State::E;
            else if (*it=='.')
                state = State::TRAILING_DECIMAL;
            else if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            break;
        case State::TRAILING_DECIMAL:
            if (*it=='e' || *it=='E')
                state = State::E;
            else if (*it=='f' || *it=='F')
                state = State::SUFFIX_F;
            else if (*it=='l' || *it=='L')
                state = State::SUFFIX_L;
            else if (*it == '_')
                state = State::SUFFIX_LITERAL_LEADER;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state = State::BASE_DIGITS2;
            else
                return false;
            break;
        case State::BASE_DIGITS2:
            if (*it=='e' || *it=='E')
                state = State::E;
            else if (*it=='f' || *it=='F')
                state = State::SUFFIX_F;
            else if (*it=='l' || *it=='L')
                state = State::SUFFIX_L;
            else if (*it == '_')
                state = State::SUFFIX_LITERAL_LEADER;
            else if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            break;
        case State::E:
            if (*it=='+' || *it=='-')
                state = State::MANTISSA_PLUSMINUS;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state = State::MANTISSA_DIGITS;
            else
                return false;
            break;
        case State::MANTISSA_PLUSMINUS:
            if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            else
                state = State::MANTISSA_DIGITS;
            break;
        case State::MANTISSA_DIGITS:
            if (*it=='f' || *it=='F')
                state = State::SUFFIX_F;
            else if (*it=='l' || *it=='L')
                state = State::SUFFIX_L;
            else if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            break;
        // Ensure at least one post _ char for user defined literals
        case State::SUFFIX_LITERAL:
        case State::SUFFIX_LITERAL_LEADER:
            state = State::SUFFIX_LITERAL;
            break;
        case State::SUFFIX_F:
            return false;
        case State::SUFFIX_L:
            return false;
        }
    }
    return (state==State::BASE_DIGITS2 || state==State::MANTISSA_DIGITS || state==State::TRAILING_DECIMAL || state==State::SUFFIX_F || state==State::SUFFIX_L || (state==State::SUFFIX_LITERAL));
}

bool MathLib::isNegative(const std::string &str)
{
    if (str.empty())
        return false;
    return (str[0] == '-');
}

bool MathLib::isPositive(const std::string &str)
{
    if (str.empty())
        return false;
    return !MathLib::isNegative(str);
}

static bool isValidIntegerSuffixIt(std::string::const_iterator it, std::string::const_iterator end, bool supportMicrosoftExtensions=true)
{
    enum class Status : std::uint8_t { START, SUFFIX_U, SUFFIX_UL, SUFFIX_ULL, SUFFIX_UZ, SUFFIX_L, SUFFIX_LU, SUFFIX_LL, SUFFIX_LLU, SUFFIX_I, SUFFIX_I6, SUFFIX_I64, SUFFIX_UI, SUFFIX_UI6, SUFFIX_UI64, SUFFIX_Z, SUFFIX_LITERAL_LEADER, SUFFIX_LITERAL } state = Status::START;
    for (; it != end; ++it) {
        switch (state) {
        case Status::START:
            if (*it == 'u' || *it == 'U')
                state = Status::SUFFIX_U;
            else if (*it == 'l' || *it == 'L')
                state = Status::SUFFIX_L;
            else if (*it == 'z' || *it == 'Z')
                state = Status::SUFFIX_Z;
            else if (supportMicrosoftExtensions && (*it == 'i' || *it == 'I'))
                state = Status::SUFFIX_I;
            else if (*it == '_')
                state = Status::SUFFIX_LITERAL_LEADER;
            else
                return false;
            break;
        case Status::SUFFIX_U:
            if (*it == 'l' || *it == 'L')
                state = Status::SUFFIX_UL; // UL
            else if (*it == 'z' || *it == 'Z')
                state = Status::SUFFIX_UZ; // UZ
            else if (supportMicrosoftExtensions && (*it == 'i' || *it == 'I'))
                state = Status::SUFFIX_UI;
            else
                return false;
            break;
        case Status::SUFFIX_UL:
            if (*it == 'l' || *it == 'L')
                state = Status::SUFFIX_ULL; // ULL
            else
                return false;
            break;
        case Status::SUFFIX_L:
            if (*it == 'u' || *it == 'U')
                state = Status::SUFFIX_LU; // LU
            else if (*it == 'l' || *it == 'L')
                state = Status::SUFFIX_LL; // LL
            else
                return false;
            break;
        case Status::SUFFIX_LU:
            return false;
        case Status::SUFFIX_LL:
            if (*it == 'u' || *it == 'U')
                state = Status::SUFFIX_LLU; // LLU
            else
                return false;
            break;
        case Status::SUFFIX_I:
            if (*it == '6')
                state = Status::SUFFIX_I6;
            else
                return false;
            break;
        case Status::SUFFIX_I6:
            if (*it == '4')
                state = Status::SUFFIX_I64;
            else
                return false;
            break;
        case Status::SUFFIX_UI:
            if (*it == '6')
                state = Status::SUFFIX_UI6;
            else
                return false;
            break;
        case Status::SUFFIX_UI6:
            if (*it == '4')
                state = Status::SUFFIX_UI64;
            else
                return false;
            break;
        case Status::SUFFIX_Z:
            if (*it == 'u' || *it == 'U')
                state = Status::SUFFIX_UZ;
            else
                return false;
            break;
        // Ensure at least one post _ char for user defined literals
        case Status::SUFFIX_LITERAL:
        case Status::SUFFIX_LITERAL_LEADER:
            state = Status::SUFFIX_LITERAL;
            break;
        default:
            return false;
        }
    }
    return ((state == Status::SUFFIX_U) ||
            (state == Status::SUFFIX_L) ||
            (state == Status::SUFFIX_Z) ||
            (state == Status::SUFFIX_UL) ||
            (state == Status::SUFFIX_UZ) ||
            (state == Status::SUFFIX_LU) ||
            (state == Status::SUFFIX_LL) ||
            (state == Status::SUFFIX_ULL) ||
            (state == Status::SUFFIX_LLU) ||
            (state == Status::SUFFIX_I64) ||
            (state == Status::SUFFIX_UI64) ||
            (state == Status::SUFFIX_LITERAL));
}

bool MathLib::isValidIntegerSuffix(const std::string& str, bool supportMicrosoftExtensions)
{
    return isValidIntegerSuffixIt(str.cbegin(), str.cend(), supportMicrosoftExtensions);
}



/*! \brief Does the string represent an octal number?
 * In case leading or trailing white space is provided, the function
 * returns false.
 * Additional information can be found here:
 * http://gcc.gnu.org/onlinedocs/gcc/Binary-constants.html
 *
 * \param str The string to check. In case the string is empty, the function returns false.
 * \return Return true in case a octal number is provided and false otherwise.
 **/
bool MathLib::isOct(const std::string& str)
{
    enum class Status : std::uint8_t {
        START, OCTAL_PREFIX, DIGITS
    } state = Status::START;
    if (str.empty())
        return false;
    auto it = str.cbegin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.cend(); ++it) {
        switch (state) {
        case Status::START:
            if (*it == '0')
                state = Status::OCTAL_PREFIX;
            else
                return false;
            break;
        case Status::OCTAL_PREFIX:
            if (isOctalDigit(static_cast<unsigned char>(*it)))
                state = Status::DIGITS;
            else
                return false;
            break;
        case Status::DIGITS:
            if (isOctalDigit(static_cast<unsigned char>(*it)))
                state = Status::DIGITS;
            else
                return isValidIntegerSuffixIt(it,str.end());
            break;
        }
    }
    return state == Status::DIGITS;
}

bool MathLib::isIntHex(const std::string& str)
{
    enum class Status : std::uint8_t {
        START, HEX_0, HEX_X, DIGIT
    } state = Status::START;
    if (str.empty())
        return false;
    auto it = str.cbegin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.cend(); ++it) {
        switch (state) {
        case Status::START:
            if (*it == '0')
                state = Status::HEX_0;
            else
                return false;
            break;
        case Status::HEX_0:
            if (*it == 'x' || *it == 'X')
                state = Status::HEX_X;
            else
                return false;
            break;
        case Status::HEX_X:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = Status::DIGIT;
            else
                return false;
            break;
        case Status::DIGIT:
            if (isxdigit(static_cast<unsigned char>(*it)))
                ; //  state = Status::DIGIT;
            else
                return isValidIntegerSuffixIt(it,str.end());
            break;
        }
    }
    return Status::DIGIT == state;
}

bool MathLib::isFloatHex(const std::string& str)
{
    enum class Status : std::uint8_t {
        START, HEX_0, HEX_X, WHOLE_NUMBER_DIGIT, POINT, FRACTION, EXPONENT_P, EXPONENT_SIGN, EXPONENT_DIGITS, EXPONENT_SUFFIX
    } state = Status::START;
    if (str.empty())
        return false;
    auto it = str.cbegin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.cend(); ++it) {
        switch (state) {
        case Status::START:
            if (*it == '0')
                state = Status::HEX_0;
            else
                return false;
            break;
        case Status::HEX_0:
            if (*it == 'x' || *it == 'X')
                state = Status::HEX_X;
            else
                return false;
            break;
        case Status::HEX_X:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = Status::WHOLE_NUMBER_DIGIT;
            else if (*it == '.')
                state = Status::POINT;
            else
                return false;
            break;
        case Status::WHOLE_NUMBER_DIGIT:
            if (isxdigit(static_cast<unsigned char>(*it)))
                ; // state = Status::WHOLE_NUMBER_DIGITS;
            else if (*it=='.')
                state = Status::FRACTION;
            else if (*it=='p' || *it=='P')
                state = Status::EXPONENT_P;
            else
                return false;
            break;
        case Status::POINT:
        case Status::FRACTION:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = Status::FRACTION;
            else if (*it == 'p' || *it == 'P')
                state = Status::EXPONENT_P;
            else
                return false;
            break;
        case Status::EXPONENT_P:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = Status::EXPONENT_DIGITS;
            else if (*it == '+' || *it == '-')
                state = Status::EXPONENT_SIGN;
            else
                return false;
            break;
        case Status::EXPONENT_SIGN:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = Status::EXPONENT_DIGITS;
            else
                return false;
            break;
        case Status::EXPONENT_DIGITS:
            if (isdigit(static_cast<unsigned char>(*it)))
                ; //  state = Status::EXPONENT_DIGITS;
            else if (*it == 'f' || *it == 'F' || *it == 'l' || *it == 'L')
                state = Status::EXPONENT_SUFFIX;
            else
                return false;
            break;
        case Status::EXPONENT_SUFFIX:
            return false;
        }
    }
    return (Status::EXPONENT_DIGITS == state) || (Status::EXPONENT_SUFFIX == state);
}


/*! \brief Does the string represent a binary number?
 * In case leading or trailing white space is provided, the function
 * returns false.
 * Additional information can be found here:
 * http://gcc.gnu.org/onlinedocs/gcc/Binary-constants.html
 *
 * \param str The string to check. In case the string is empty, the function returns false.
 * \return Return true in case a binary number is provided and false otherwise.
 **/
bool MathLib::isBin(const std::string& str)
{
    enum class Status : std::uint8_t {
        START, GNU_BIN_PREFIX_0, GNU_BIN_PREFIX_B, DIGIT
    } state = Status::START;
    if (str.empty())
        return false;
    auto it = str.cbegin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.cend(); ++it) {
        switch (state) {
        case Status::START:
            if (*it == '0')
                state = Status::GNU_BIN_PREFIX_0;
            else
                return false;
            break;
        case Status::GNU_BIN_PREFIX_0:
            if (*it == 'b' || *it == 'B')
                state = Status::GNU_BIN_PREFIX_B;
            else
                return false;
            break;
        case Status::GNU_BIN_PREFIX_B:
            if (*it == '0' || *it == '1')
                state = Status::DIGIT;
            else
                return false;
            break;
        case Status::DIGIT:
            if (*it == '0' || *it == '1')
                ; //  state = Status::DIGIT;
            else
                return isValidIntegerSuffixIt(it,str.end());
            break;
        }
    }
    return state == Status::DIGIT;
}

bool MathLib::isDec(const std::string & str)
{
    enum class Status : std::uint8_t {
        START, DIGIT
    } state = Status::START;
    if (str.empty())
        return false;
    auto it = str.cbegin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.cend(); ++it) {
        switch (state) {
        case Status::START:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = Status::DIGIT;
            else
                return false;
            break;
        case Status::DIGIT:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = Status::DIGIT;
            else
                return isValidIntegerSuffixIt(it,str.end());
            break;
        }
    }
    return state == Status::DIGIT;
}

bool MathLib::isInt(const std::string & str)
{
    return isDec(str) || isIntHex(str) || isOct(str) || isBin(str);
}

std::string MathLib::getSuffix(const std::string& value)
{
    if (value.size() > 3 && value[value.size() - 3] == 'i' && value[value.size() - 2] == '6' && value[value.size() - 1] == '4') {
        if (value[value.size() - 4] == 'u')
            return "ULL";
        return "LL";
    }
    bool isUnsigned = false;
    unsigned int longState = 0;
    for (std::size_t i = 1U; i < value.size(); ++i) {
        const char c = value[value.size() - i];
        if (c == 'u' || c == 'U')
            isUnsigned = true;
        else if (c == 'L' || c == 'l')
            longState++;
        else break;
    }
    if (longState == 0)
        return isUnsigned ? "U" : "";
    if (longState == 1)
        return isUnsigned ? "UL" : "L";
    if (longState == 2)
        return isUnsigned ? "ULL" : "LL";
    return "";
}

static std::string intsuffix(const std::string & first, const std::string & second)
{
    const std::string suffix1 = MathLib::getSuffix(first);
    const std::string suffix2 = MathLib::getSuffix(second);
    if (suffix1 == "ULL" || suffix2 == "ULL")
        return "ULL";
    if (suffix1 == "LL" || suffix2 == "LL")
        return "LL";
    if (suffix1 == "UL" || suffix2 == "UL")
        return "UL";
    if (suffix1 == "L" || suffix2 == "L")
        return "L";
    if (suffix1 == "U" || suffix2 == "U")
        return "U";

    return suffix1.empty() ? suffix2 : suffix1;
}

std::string MathLib::add(const std::string & first, const std::string & second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) + value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return MathLib::toString(toBigNumber(first) + toBigNumber(second)) + intsuffix(first, second);
    }

    double d1 = toDoubleNumber(first);
    double d2 = toDoubleNumber(second);

    int count = 0;
    while (d1 > 100000.0 * d2 && toString(d1+d2)==first && ++count<5)
        d2 *= 10.0;
    while (d2 > 100000.0 * d1 && toString(d1+d2)==second && ++count<5)
        d1 *= 10.0;

    return toString(d1 + d2);
#endif
}

std::string MathLib::subtract(const std::string &first, const std::string &second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) - value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return MathLib::toString(toBigNumber(first) - toBigNumber(second)) + intsuffix(first, second);
    }

    if (first == second)
        return "0.0";

    double d1 = toDoubleNumber(first);
    double d2 = toDoubleNumber(second);

    int count = 0;
    while (d1 > 100000.0 * d2 && toString(d1-d2)==first && ++count<5)
        d2 *= 10.0;
    while (d2 > 100000.0 * d1 && toString(d1-d2)==second && ++count<5)
        d1 *= 10.0;

    return toString(d1 - d2);
#endif
}

std::string MathLib::divide(const std::string &first, const std::string &second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) / value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        const bigint a = toBigNumber(first);
        const bigint b = toBigNumber(second);
        if (b == 0)
            throw InternalError(nullptr, "Internal Error: Division by zero");
        if (a == std::numeric_limits<bigint>::min() && std::abs(b)<=1)
            throw InternalError(nullptr, "Internal Error: Division overflow");
        return MathLib::toString(toBigNumber(first) / b) + intsuffix(first, second);
    }
    if (isNullValue(second)) {
        if (isNullValue(first))
            return "nan.0";
        return isPositive(first) == isPositive(second) ? "inf.0" : "-inf.0";
    }
    return toString(toDoubleNumber(first) / toDoubleNumber(second));
#endif
}

std::string MathLib::multiply(const std::string &first, const std::string &second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) * value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return MathLib::toString(toBigNumber(first) * toBigNumber(second)) + intsuffix(first, second);
    }
    return toString(toDoubleNumber(first) * toDoubleNumber(second));
#endif
}

std::string MathLib::mod(const std::string &first, const std::string &second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) % value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        const bigint b = toBigNumber(second);
        if (b == 0)
            throw InternalError(nullptr, "Internal Error: Division by zero");
        return MathLib::toString(toBigNumber(first) % b) + intsuffix(first, second);
    }
    return toString(std::fmod(toDoubleNumber(first),toDoubleNumber(second)));
#endif
}

std::string MathLib::calculate(const std::string &first, const std::string &second, char action)
{
    switch (action) {
    case '+':
        return MathLib::add(first, second);

    case '-':
        return MathLib::subtract(first, second);

    case '*':
        return MathLib::multiply(first, second);

    case '/':
        return MathLib::divide(first, second);

    case '%':
        return MathLib::mod(first, second);

    case '&':
        return MathLib::toString(MathLib::toBigNumber(first) & MathLib::toBigNumber(second)) + intsuffix(first, second);

    case '|':
        return MathLib::toString(MathLib::toBigNumber(first) | MathLib::toBigNumber(second)) + intsuffix(first, second);

    case '^':
        return MathLib::toString(MathLib::toBigNumber(first) ^ MathLib::toBigNumber(second)) + intsuffix(first, second);

    default:
        throw InternalError(nullptr, std::string("Unexpected action '") + action + "' in MathLib::calculate().");
    }
}

std::string MathLib::sin(const std::string &tok)
{
    return toString(std::sin(toDoubleNumber(tok)));
}


std::string MathLib::cos(const std::string &tok)
{
    return toString(std::cos(toDoubleNumber(tok)));
}

std::string MathLib::tan(const std::string &tok)
{
    return toString(std::tan(toDoubleNumber(tok)));
}


std::string MathLib::abs(const std::string &tok)
{
    if (isNegative(tok))
        return tok.substr(1, tok.length() - 1);
    return tok;
}

bool MathLib::isEqual(const std::string &first, const std::string &second)
{
    // this conversion is needed for formatting
    // e.g. if first=0.1 and second=1.0E-1, the direct comparison of the strings would fail
    return toString(toDoubleNumber(first)) == toString(toDoubleNumber(second));
}

bool MathLib::isNotEqual(const std::string &first, const std::string &second)
{
    return !isEqual(first, second);
}

// cppcheck-suppress unusedFunction
bool MathLib::isGreater(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) > toDoubleNumber(second);
}

// cppcheck-suppress unusedFunction
bool MathLib::isGreaterEqual(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) >= toDoubleNumber(second);
}

// cppcheck-suppress unusedFunction
bool MathLib::isLess(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) < toDoubleNumber(second);
}

bool MathLib::isLessEqual(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) <= toDoubleNumber(second);
}

/*! \brief Does the string represent the numerical value of 0?
 * In case leading or trailing white space is provided, the function
 * returns false.
 * Requirement for this function:
 * - This code is allowed to be slow because of simplicity of the code.
 *
 * \param[in] str The string to check. In case the string is empty, the function returns false.
 * \return Return true in case the string represents a numerical null value.
 **/
bool MathLib::isNullValue(const std::string &str)
{
    if (str.empty() || (!std::isdigit(static_cast<unsigned char>(str[0])) && (str[0] != '.' && str[0] != '-' && str[0] != '+')))
        return false; // Has to be a number

    if (!isInt(str) && !isFloat(str))
        return false;
    const bool isHex = isIntHex(str) || isFloatHex(str);
    for (const char i : str) {
        if (std::isdigit(static_cast<unsigned char>(i)) && i != '0') // May not contain digits other than 0
            return false;
        if (i == 'p' || i == 'P' || (!isHex && (i == 'E' || i == 'e')))
            return true;
        if (isHex && isxdigit(i) && i != '0')
            return false;
    }
    return true;
}

bool MathLib::isOctalDigit(char c)
{
    return (c >= '0' && c <= '7');
}

MathLib::value operator+(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('+',v1,v2);
}

MathLib::value operator-(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('-',v1,v2);
}

MathLib::value operator*(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('*',v1,v2);
}

MathLib::value operator/(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('/',v1,v2);
}

MathLib::value operator%(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('%',v1,v2);
}

MathLib::value operator&(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('&',v1,v2);
}

MathLib::value operator|(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('|',v1,v2);
}

MathLib::value operator^(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('^',v1,v2);
}

MathLib::value operator<<(const MathLib::value &v1, const MathLib::value &v2)
{
    return v1.shiftLeft(v2);
}

MathLib::value operator>>(const MathLib::value &v1, const MathLib::value &v2)
{
    return v1.shiftRight(v2);
}
