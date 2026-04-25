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
#ifndef utilsH
#define utilsH
//---------------------------------------------------------------------------

#include "config.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

struct SelectMapKeys {
    template<class Pair>
    // NOLINTNEXTLINE(readability-const-return-type) - false positive
    typename Pair::first_type operator()(const Pair& p) const {
        return p.first;
    }
};

struct SelectMapValues {
    template<class Pair>
    typename Pair::second_type operator()(const Pair& p) const {
        return p.second;
    }
};

struct OnExit {
    std::function<void()> f;

    ~OnExit() {
        f();
    }
};

template<class Range, class T>
bool contains(const Range& r, const T& x)
{
    return std::find(r.cbegin(), r.cend(), x) != r.cend();
}

template<class T>
bool contains(const std::initializer_list<T>& r, const T& x)
{
    return std::find(r.begin(), r.end(), x) != r.end();
}

template<class T, class U>
bool contains(const std::initializer_list<T>& r, const U& x)
{
    return std::find(r.begin(), r.end(), x) != r.end();
}

template<class T, class ... Ts>
inline std::array<T, sizeof...(Ts) + 1> makeArray(T x, Ts... xs)
{
    return {std::move(x), std::move(xs)...};
}

// Enum hash for C++11. This is not needed in C++14
struct EnumClassHash {
    template<typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

inline bool startsWith(const std::string& str, const char start[], std::size_t startlen)
{
    return str.compare(0, startlen, start) == 0;
}

template<std::size_t N>
bool startsWith(const std::string& str, const char (&start)[N])
{
    return startsWith(str, start, N - 1);
}

inline bool startsWith(const std::string& str, const std::string& start)
{
    return startsWith(str, start.c_str(), start.length());
}

inline bool endsWith(const std::string &str, char c)
{
    return !str.empty() && str.back() == c;
}

inline bool endsWith(const std::string &str, const char end[], std::size_t endlen)
{
    return (str.size() >= endlen) && (str.compare(str.size()-endlen, endlen, end)==0);
}

template<std::size_t N>
bool endsWith(const std::string& str, const char (&end)[N])
{
    return endsWith(str, end, N - 1);
}

inline bool endsWith(const std::string& str, const std::string& end)
{
    return endsWith(str, end.c_str(), end.length());
}

inline static bool isPrefixStringCharLiteral(const std::string &str, char q, const std::string& p)
{
    // str must be at least the prefix plus the start and end quote
    if (str.length() < p.length() + 2)
        return false;

    // check for end quote
    if (!endsWith(str, q))
        return false;

    // check for start quote
    if (str[p.size()] != q)
        return false;

    // check for prefix
    if (str.compare(0, p.size(), p) != 0)
        return false;

    return true;
}

inline static bool isStringCharLiteral(const std::string &str, char q)
{
    // early out to avoid the loop
    if (!endsWith(str, q))
        return false;

    static const std::array<std::string, 5> suffixes{"", "u8", "u", "U", "L"};
    return std::any_of(suffixes.cbegin(), suffixes.cend(), [&](const std::string& p) {
        return isPrefixStringCharLiteral(str, q, p);
    });
}

inline static bool isStringLiteral(const std::string &str)
{
    return isStringCharLiteral(str, '"');
}

inline static bool isCharLiteral(const std::string &str)
{
    return isStringCharLiteral(str, '\'');
}

inline static std::string getStringCharLiteral(const std::string &str, char q)
{
    const std::size_t quotePos = str.find(q);
    return str.substr(quotePos + 1U, str.size() - quotePos - 2U);
}

inline static std::string getStringLiteral(const std::string &str)
{
    if (isStringLiteral(str))
        return getStringCharLiteral(str, '"');
    return "";
}

inline static std::string getCharLiteral(const std::string &str)
{
    if (isCharLiteral(str))
        return getStringCharLiteral(str, '\'');
    return "";
}

inline static const char *getOrdinalText(int i)
{
    if (i == 1)
        return "st";
    if (i == 2)
        return "nd";
    if (i == 3)
        return "rd";
    return "th";
}

CPPCHECKLIB int caseInsensitiveStringCompare(const std::string& lhs, const std::string& rhs);

CPPCHECKLIB bool isValidGlobPattern(const std::string& pattern);

CPPCHECKLIB bool matchglob(const std::string& pattern, const std::string& name, bool caseInsensitive = false);

CPPCHECKLIB void strTolower(std::string& str);

template<typename T, typename std::enable_if<std::is_signed<T>::value, bool>::type=true>
bool strToInt(const std::string& str, T &num, std::string* err = nullptr)
{
    long long tmp;
    try {
        std::size_t idx = 0;
        tmp = std::stoll(str, &idx);
        if (idx != str.size()) {
            if (err)
                *err = "not an integer (pos)";
            return false;
        }
    } catch (const std::out_of_range&) {
        if (err)
            *err = "out of range (stoll)";
        return false;
    } catch (const std::invalid_argument &) {
        if (err)
            *err = "not an integer (invalid_argument)";
        return false;
    }
    if (str.front() == '-' && std::numeric_limits<T>::min() == 0) {
        if (err)
            *err = "needs to be positive";
        return false;
    }
    if (str.front() != '+' && str.front() != '-' && isdigit(str.front()) == 0) {
        if (err)
            *err = "not an integer";
        return false;
    }
    if (str.size() > 1 && str.front() == '0') {
        if (err)
            *err = "not an integer";
        return false;
    }
    if (tmp < std::numeric_limits<T>::min() || tmp > std::numeric_limits<T>::max()) {
        if (err)
            *err = "out of range (limits)";
        return false;
    }
    num = static_cast<T>(tmp);
    return true;
}

template<typename T, typename std::enable_if<std::is_unsigned<T>::value, bool>::type=true>
bool strToInt(const std::string& str, T &num, std::string* err = nullptr)
{
    unsigned long long tmp;
    try {
        std::size_t idx = 0;
        tmp = std::stoull(str, &idx);
        if (idx != str.size()) {
            if (err)
                *err = "not an integer (pos)";
            return false;
        }
    } catch (const std::out_of_range&) {
        if (err)
            *err = "out of range (stoull)";
        return false;
    } catch (const std::invalid_argument &) {
        if (err)
            *err = "not an integer (invalid_argument)";
        return false;
    }
    if (str.front() == '-') {
        if (err)
            *err = "needs to be positive";
        return false;
    }
    if (str.front() != '+' && isdigit(str.front()) == 0) {
        if (err)
            *err = "not an integer";
        return false;
    }
    if (str.size() > 1 && str.front() == '0') {
        if (err)
            *err = "not an integer";
        return false;
    }
    if (tmp > std::numeric_limits<T>::max()) {
        if (err)
            *err = "out of range (limits)";
        return false;
    }
    num = tmp;
    return true;
}

/**
 * @throws std::runtime_error thrown if conversion failed
 */
template<typename T>
T strToInt(const std::string& str)
{
    T tmp = 0;
    std::string err;
    if (!strToInt(str, tmp, &err))
        throw std::runtime_error("converting '" + str + "' to integer failed - " + err);
    return tmp;
}

/**
 *  Simple helper function:
 * \return size of array
 * */
template<typename T, int size>
std::size_t getArrayLength(const T (& /*unused*/)[size])
{
    return size;
}

/**
 * @brief get id string. i.e. for dump files
 * it will be a hexadecimal output.
 */
static inline std::string id_string_i(std::uintptr_t l)
{
    if (!l)
        return "0";

    static constexpr int ptr_size = sizeof(void*);

    // two characters of each byte / contains terminating \0
    static constexpr int buf_size = (ptr_size * 2) + 1;

    char buf[buf_size];

    // needs to be signed so we don't underflow in padding loop
    int idx = buf_size - 1;
    buf[idx] = '\0';

    while (l != 0)
    {
        char c;
        const std::uintptr_t temp = l % 16; // get the remainder
        if (temp < 10) {
            // 0-9
            c = '0' + temp;
        }
        else {
            // a-f
            c = 'a' + (temp - 10);
        }
        buf[--idx] = c; // store in reverse order
        l = l / 16;
    }

    return &buf[idx];
}

static inline std::string id_string(const void* p)
{
    return id_string_i(reinterpret_cast<std::uintptr_t>(p));
}

static inline const char* bool_to_string(bool b)
{
    return b ? "true" : "false";
}

/**
 * Remove heading and trailing whitespaces from the input parameter.
 * If string is all spaces/tabs, return empty string.
 * @param s The string to trim.
 * @param t The characters to trim.
 */
CPPCHECKLIB std::string trim(const std::string& s, const std::string& t = " \t");

/**
 * Replace all occurrences of searchFor with replaceWith in the
 * given source.
 * @param source The string to modify
 * @param searchFor What should be searched for
 * @param replaceWith What will replace the found item
 */
CPPCHECKLIB void findAndReplace(std::string &source, const std::string &searchFor, const std::string &replaceWith);

/**
 * Replace all escape sequences in the given string.
 * @param source The string that contains escape sequences
 */
CPPCHECKLIB std::string replaceEscapeSequences(const std::string &source);

namespace cppcheck
{
    NORETURN inline void unreachable()
    {
#if defined(__GNUC__)
        __builtin_unreachable();
#elif defined(_MSC_VER)
        __assume(false);
#else
#error "no unreachable implementation"
#endif
    }
}

template<typename T>
static inline T* default_if_null(T* p, T* def)
{
    return p ? p : def;
}

template<typename T>
static inline T* empty_if_null(T* p)
{
    return default_if_null(p, "");
}

/**
 * Split string by given separator.
 * @param str The string to split
 * @param sep The separator
 * @return The list of separate strings (including empty ones). The whole input string if no separator found.
 */
CPPCHECKLIB std::vector<std::string> splitString(const std::string& str, char sep);

namespace utils {
    /**
     * Drop-in replacement for C++17's std::as_const
     * @param t  The function forms the lvalue reference to const type of this argument.
     */
    template<class T>
    constexpr typename std::add_const<T>::type & as_const(T& t) noexcept
    {
        static_assert(!std::is_const<T>::value, "object is already const");
        static_assert(!std::is_pointer<T>::value, "object is a pointer");
        // NOLINTNEXTLINE(bugprone-return-const-ref-from-parameter) - potential false positive
        return t;
    }

    // Thread-unsafe memoization
    template<class F, class R=decltype(std::declval<F>()())>
    static inline std::function<R()> memoize(F f)
    {
        bool init = false;
        R result{};
        return [=]() mutable -> R {
            if (init)
                return result;
            result = f();
            init = true;
            return result;
        };
    }
}

#endif
