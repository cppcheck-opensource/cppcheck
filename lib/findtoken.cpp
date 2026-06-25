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

#include "findtoken.h"

#include "astutils.h"
#include "token.h"

class Library;

template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
static bool findTokensSkipDeadCodeImpl(const Library& library,
                                       T* start,
                                       const Token* end,
                                       const std::function<bool(const Token*)>& pred,
                                       const std::function<bool(T*)>& found,
                                       const std::function<std::vector<MathLib::bigint>(const Token*)>& evaluate,
                                       bool skipUnevaluated)
{
    for (T* tok = start; precedes(tok, end); tok = tok->next()) {
        if (pred(tok)) {
            if (found(tok))
                return true;
        }
        if (Token::Match(tok, "if|for|while (") && Token::simpleMatch(tok->linkAt(1), ") {")) {
            const Token* condTok = getCondTok(tok);
            if (!condTok)
                continue;
            auto result = evaluate(condTok);
            if (result.empty())
                continue;
            if (internal::findTokensSkipDeadCodeImpl(library, tok->next(), tok->linkAt(1), pred, found, evaluate, skipUnevaluated))
                return true;
            T* thenStart = tok->linkAt(1)->next();
            T* elseStart = nullptr;
            if (Token::simpleMatch(thenStart->link(), "} else {"))
                elseStart = thenStart->link()->tokAt(2);

            auto r = result.front();
            if (r == 0) {
                if (elseStart) {
                    if (internal::findTokensSkipDeadCodeImpl(library, elseStart, elseStart->link(), pred, found, evaluate, skipUnevaluated))
                        return true;
                    if (isReturnScope(elseStart->link(), library))
                        return true;
                    tok = elseStart->link();
                } else {
                    tok = thenStart->link();
                }
            } else {
                if (internal::findTokensSkipDeadCodeImpl(library, thenStart, thenStart->link(), pred, found, evaluate, skipUnevaluated))
                    return true;
                if (isReturnScope(thenStart->link(), library))
                    return true;
                tok = thenStart->link();
            }
        } else if (Token::Match(tok->astParent(), "&&|?|%oror%") && astIsLHS(tok)) {
            auto result = evaluate(tok);
            if (result.empty())
                continue;
            const bool cond = result.front() != 0;
            T* next = nullptr;
            if ((cond && Token::simpleMatch(tok->astParent(), "||")) ||
                (!cond && Token::simpleMatch(tok->astParent(), "&&"))) {
                next = nextAfterAstRightmostLeaf(tok->astParent());
            } else if (Token::simpleMatch(tok->astParent(), "?")) {
                T* colon = tok->astParent()->astOperand2();
                if (!cond) {
                    next = colon;
                } else {
                    if (internal::findTokensSkipDeadCodeImpl(library, tok->astParent()->next(), colon, pred, found, evaluate, skipUnevaluated))
                        return true;
                    next = nextAfterAstRightmostLeaf(colon);
                }
            }
            if (next)
                tok = next;
        } else if (Token::simpleMatch(tok, "} else {")) {
            const Token* condTok = getCondTokFromEnd(tok);
            if (!condTok)
                continue;
            auto result = evaluate(condTok);
            if (result.empty())
                continue;
            if (isReturnScope(tok->link(), library))
                return true;
            auto r = result.front();
            if (r != 0) {
                tok = tok->linkAt(2);
            }
        } else if (Token::simpleMatch(tok, "[") && Token::Match(tok->link(), "] (|{")) {
            T* afterCapture = tok->link()->next();
            if (Token::simpleMatch(afterCapture, "(") && afterCapture->link())
                tok = afterCapture->link()->next();
            else
                tok = afterCapture;
        }
        if (skipUnevaluated && isUnevaluated(tok)) {
            T *next = tok->linkAt(1);
            if (!next)
                continue;
            tok = next;
        }
    }
    return false;
}

namespace internal {
    bool findTokensSkipDeadCodeImpl(const Library& library,
                                    Token* start,
                                    const Token* end,
                                    const std::function<bool(const Token*)>& pred,
                                    const std::function<bool(Token*)>& found,
                                    const std::function<std::vector<MathLib::bigint>(const Token*)>& evaluate,
                                    bool skipUnevaluated)
    {
        return ::findTokensSkipDeadCodeImpl(library, start, end, pred, found, evaluate, skipUnevaluated);
    }

    bool findTokensSkipDeadCodeImpl(const Library& library,
                                    const Token* start,
                                    const Token* end,
                                    const std::function<bool(const Token*)>& pred,
                                    const std::function<bool(const Token*)>& found,
                                    const std::function<std::vector<MathLib::bigint>(const Token*)>& evaluate,
                                    bool skipUnevaluated)
    {
        return ::findTokensSkipDeadCodeImpl(library, start, end, pred, found, evaluate, skipUnevaluated);
    }
}
