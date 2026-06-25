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
#ifndef findtokenH
#define findtokenH
//---------------------------------------------------------------------------

#include <functional>
#include <type_traits>
#include <vector>

#include "config.h"
#include "mathlib.h"
#include "token.h"

class Library;

inline std::vector<MathLib::bigint> evaluateKnownValues(const Token* tok)
{
    if (!tok->hasKnownIntValue())
        return {};
    return {tok->getKnownIntValue()};
}

template<class T, class Predicate, class Found, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
void findTokensImpl(T* start, const Token* end, const Predicate& pred, Found found)
{
    for (T* tok = start; precedes(tok, end); tok = tok->next()) {
        if (pred(tok)) {
            if (found(tok))
                break;
        }
    }
}

template<class T, class Predicate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
std::vector<T*> findTokens(T* start, const Token* end, const Predicate& pred)
{
    std::vector<T*> result;
    findTokensImpl(start, end, pred, [&](T* tok) {
        result.push_back(tok);
        return false;
    });
    return result;
}

template<class T, class Predicate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
T* findToken(T* start, const Token* end, const Predicate& pred)
{
    T* result = nullptr;
    findTokensImpl(start, end, pred, [&](T* tok) {
        result = tok;
        return true;
    });
    return result;
}

namespace internal {
    bool findTokensSkipDeadCodeImpl(const Library &library,
                                    Token *start,
                                    const Token *end,
                                    const std::function<bool(const Token *)> &pred,
                                    const std::function<bool(Token *)>& found,
                                    const std::function<std::vector<MathLib::bigint>(const Token *)> &evaluate,
                                    bool skipUnevaluated);

    bool findTokensSkipDeadCodeImpl(const Library &library,
                                    const Token *start,
                                    const Token *end,
                                    const std::function<bool(const Token *)> &pred,
                                    const std::function<bool(const Token *)>& found,
                                    const std::function<std::vector<MathLib::bigint>(const Token *)> &evaluate,
                                    bool skipUnevaluated);
}

template<class T, class Predicate, class Evaluate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
std::vector<T*> findTokensSkipDeadCode(const Library& library,
                                       T* start,
                                       const Token* end,
                                       const Predicate& pred,
                                       const Evaluate& evaluate)
{
    std::vector<T*> result;
    (void)internal::findTokensSkipDeadCodeImpl(
        library,
        start,
        end,
        pred,
        [&](T* tok) {
        result.push_back(tok);
        return false;
    },
        evaluate,
        false);
    return result;
}

template<class T, class Predicate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
std::vector<T*> findTokensSkipDeadCode(const Library& library, T* start, const Token* end, const Predicate& pred)
{
    return findTokensSkipDeadCode(library, start, end, pred, &evaluateKnownValues);
}

template<class T, class Predicate, class Evaluate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
std::vector<T*> findTokensSkipDeadAndUnevaluatedCode(const Library& library,
                                                     T* start,
                                                     const Token* end,
                                                     const Predicate& pred,
                                                     const Evaluate& evaluate)
{
    std::vector<T*> result;
    (void)internal::findTokensSkipDeadCodeImpl(
        library,
        start,
        end,
        pred,
        [&](T* tok) {
        result.push_back(tok);
        return false;
    },
        evaluate,
        true);
    return result;
}

template<class T, class Predicate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
std::vector<T*> findTokensSkipDeadAndUnevaluatedCode(const Library& library, T* start, const Token* end, const Predicate& pred)
{
    return findTokensSkipDeadAndUnevaluatedCode(library, start, end, pred, &evaluateKnownValues);
}


template<class T, class Predicate, class Evaluate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
T* findTokenSkipDeadCode(const Library& library, T* start, const Token* end, const Predicate& pred, const Evaluate& evaluate)
{
    T* result = nullptr;
    (void)internal::findTokensSkipDeadCodeImpl(
        library,
        start,
        end,
        pred,
        [&](T* tok) {
        result = tok;
        return true;
    },
        evaluate,
        false);
    return result;
}

template<class T, class Predicate, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
T* findTokenSkipDeadCode(const Library& library, T* start, const Token* end, const Predicate& pred)
{
    return findTokenSkipDeadCode(library, start, end, pred, &evaluateKnownValues);
}

#endif // findtokenH
