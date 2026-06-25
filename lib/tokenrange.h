/* -*- C++ -*-
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

//---------------------------------------------------------------------------
#ifndef tokenrangeH
#define tokenrangeH
//---------------------------------------------------------------------------

#include "config.h"

#include <cstddef>
#include <iterator>
#include <type_traits>

class Token;

template<typename T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
class TokenRangeBase {
    T* mFront;
    T* mBack;

public:
    TokenRangeBase(T* front, T* back) : mFront(front), mBack(back) {}

    struct TokenIterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = T*;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = T*;

        T* mt;
        TokenIterator() : mt(nullptr) {}
        explicit TokenIterator(T* t) : mt(t) {}
        TokenIterator& operator++() {
            mt = mt->next();
            return *this;
        }
        bool operator==(const TokenIterator& b) const {
            return mt == b.mt;
        }
        bool operator!=(const TokenIterator& b) const {
            return mt != b.mt;
        }
        T* operator*() const {
            return mt;
        }
    };

    TokenIterator begin() const {
        return TokenIterator(mFront);
    }
    TokenIterator end() const {
        return TokenIterator(mBack);
    }
};

class TokenRange : public TokenRangeBase<Token> {
public:
    TokenRange(Token* front, Token* back) : TokenRangeBase<Token>(front, back) {}
};

class ConstTokenRange : public TokenRangeBase<const Token> {
public:
    ConstTokenRange(const Token* front, const Token* back) : TokenRangeBase<const Token>(front, back) {}
};

#endif // tokenrangeH
