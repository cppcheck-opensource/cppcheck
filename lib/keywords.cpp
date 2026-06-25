/*
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

#include "keywords.h"

#include "utils.h"

// see https://en.cppreference.com/w/c/keyword

#define C90_KEYWORDS \
    "auto", "break", "case", "char", "const", "continue", "default", \
    "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", \
    "register", "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef", \
    "union", "unsigned", "void", "volatile", "while"

#define C99_KEYWORDS \
    "inline", "restrict", "_Bool", "_Complex", "_Imaginary"

#define C11_KEYWORDS \
    "_Alignas", "_Alignof", "_Atomic", "_Generic", "_Noreturn", "_Static_assert", "_Thread_local"

#define C23_KEYWORDS \
    "alignas", "alignof", "bool", "constexpr", "false", "nullptr", "static_assert", "thread_local", "true", "typeof", "typeof_unqual", \
    "_BitInt", "_Decimal128", "_Decimal32", "_Decimal64"

static const std::unordered_set<std::string> c89_keywords_all = {
    C90_KEYWORDS
};

static const std::unordered_set<std::string> c89_keywords = {
    C90_KEYWORDS
};

static const std::unordered_set<std::string> c99_keywords_all = {
    C90_KEYWORDS, C99_KEYWORDS
};

static const std::unordered_set<std::string> c99_keywords = {
    C99_KEYWORDS
};

static const std::unordered_set<std::string> c11_keywords_all = {
    C90_KEYWORDS, C99_KEYWORDS, C11_KEYWORDS
};

static const std::unordered_set<std::string> c11_keywords = {
    C11_KEYWORDS
};

static const std::unordered_set<std::string> c17_keywords_all = {
    C90_KEYWORDS, C99_KEYWORDS, C11_KEYWORDS
};

static const std::unordered_set<std::string> c17_keywords = {
};

static const std::unordered_set<std::string> c23_keywords_all = {
    C90_KEYWORDS, C99_KEYWORDS, C11_KEYWORDS, C23_KEYWORDS
};

static const std::unordered_set<std::string> c23_keywords = {
    C23_KEYWORDS
};

static const std::unordered_set<std::string> c2y_keywords_all = {
    C90_KEYWORDS, C99_KEYWORDS, C11_KEYWORDS, C23_KEYWORDS
};

static const std::unordered_set<std::string> c2y_keywords = {
};

// see https://en.cppreference.com/w/cpp/keyword

#define CPP03_KEYWORDS \
    "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", \
    "class", "compl", "const", "const_cast", "continue", "default", \
    "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", \
    "float", "for", "friend", "goto", "if", "inline", "int", "long", \
    "mutable", "namespace", "new", "not", "not_eq",  "operator", \
    "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast", \
    "return", "short", "signed", "sizeof", "static", \
    "static_cast", "struct", "switch", "template", "this", "throw", \
    "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", \
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"

#define CPP11_KEYWORDS \
    "alignas", "alignof", "char16_t", "char32_t", "constexpr", "decltype", \
    "noexcept", "nullptr", "static_assert", "thread_local"

#define CPP20_KEYWORDS \
    "char8_t", "concept", "consteval", "constinit", "co_await", \
    "co_return", "co_yield", "requires"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#endif

#define CPP_TMTS_KEYWORDS \
    "atomic_cancel", "atomic_commit", "atomic_noexcept", "synchronized"

#define CPP_REFL_TS_KEYWORDS \
    "reflexpr"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

static const std::unordered_set<std::string> cpp03_keywords_all = {
    CPP03_KEYWORDS
};

static const std::unordered_set<std::string> cpp03_keywords = {
    CPP03_KEYWORDS
};

static const std::unordered_set<std::string> cpp11_keywords_all = {
    CPP03_KEYWORDS, CPP11_KEYWORDS
};

static const std::unordered_set<std::string> cpp11_keywords = {
    CPP11_KEYWORDS
};

static const std::unordered_set<std::string> cpp14_keywords_all = {
    CPP03_KEYWORDS, CPP11_KEYWORDS
};

static const std::unordered_set<std::string> cpp14_keywords = {
};

static const std::unordered_set<std::string> cpp17_keywords_all = {
    CPP03_KEYWORDS, CPP11_KEYWORDS
};

static const std::unordered_set<std::string> cpp17_keywords = {
};

static const std::unordered_set<std::string> cpp20_keywords_all = {
    CPP03_KEYWORDS, CPP11_KEYWORDS, CPP20_KEYWORDS
};

static const std::unordered_set<std::string> cpp20_keywords = {
    CPP20_KEYWORDS
};

static const std::unordered_set<std::string> cpp23_keywords = {
};

static const std::unordered_set<std::string> cpp23_keywords_all = {
    CPP03_KEYWORDS, CPP11_KEYWORDS, CPP20_KEYWORDS
};

static const std::unordered_set<std::string> cpp26_keywords = {
};

static const std::unordered_set<std::string> cpp26_keywords_all = {
    CPP03_KEYWORDS, CPP11_KEYWORDS, CPP20_KEYWORDS
};

const std::unordered_set<std::string>& Keywords::getAll(Standards::cstd_t cStd)
{
    switch (cStd) {
    case Standards::cstd_t::C89:
        return c89_keywords_all;
    case Standards::cstd_t::C99:
        return c99_keywords_all;
    case Standards::cstd_t::C11:
        return c11_keywords_all;
    case Standards::cstd_t::C17:
        return c17_keywords_all;
    case Standards::cstd_t::C23:
        return c23_keywords_all;
    case Standards::cstd_t::C2Y:
        return c2y_keywords_all;
    }
    cppcheck::unreachable();
}

const std::unordered_set<std::string>& Keywords::getAll(Standards::cppstd_t cppStd) {
    switch (cppStd) {
    case Standards::cppstd_t::CPP03:
        return cpp03_keywords_all;
    case Standards::cppstd_t::CPP11:
        return cpp11_keywords_all;
    case Standards::cppstd_t::CPP14:
        return cpp14_keywords_all;
    case Standards::cppstd_t::CPP17:
        return cpp17_keywords_all;
    case Standards::cppstd_t::CPP20:
        return cpp20_keywords_all;
    case Standards::cppstd_t::CPP23:
        return cpp23_keywords_all;
    case Standards::cppstd_t::CPP26:
        return cpp26_keywords_all;
    }
    cppcheck::unreachable();
}

// cppcheck-suppress unusedFunction
const std::unordered_set<std::string>& Keywords::getOnly(Standards::cstd_t cStd)
{
    switch (cStd) {
    case Standards::cstd_t::C89:
        return c89_keywords;
    case Standards::cstd_t::C99:
        return c99_keywords;
    case Standards::cstd_t::C11:
        return c11_keywords;
    case Standards::cstd_t::C17:
        return c17_keywords;
    case Standards::cstd_t::C23:
        return c23_keywords;
    case Standards::cstd_t::C2Y:
        return c2y_keywords;
    }
    cppcheck::unreachable();
}

const std::unordered_set<std::string>& Keywords::getOnly(Standards::cppstd_t cppStd)
{
    switch (cppStd) {
    case Standards::cppstd_t::CPP03:
        return cpp03_keywords;
    case Standards::cppstd_t::CPP11:
        return cpp11_keywords;
    case Standards::cppstd_t::CPP14:
        return cpp14_keywords;
    case Standards::cppstd_t::CPP17:
        return cpp17_keywords;
    case Standards::cppstd_t::CPP20:
        return cpp20_keywords;
    case Standards::cppstd_t::CPP23:
        return cpp23_keywords;
    case Standards::cppstd_t::CPP26:
        return cpp26_keywords;
    }
    cppcheck::unreachable();
}

