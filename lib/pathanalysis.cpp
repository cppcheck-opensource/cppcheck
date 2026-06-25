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

#include "pathanalysis.h"

#include "astutils.h"
#include "symboldatabase.h"
#include "token.h"
#include "vfvalue.h"

#include <algorithm>
#include <string>
#include <tuple>

const Scope* PathAnalysis::findOuterScope(const Scope * scope)
{
    if (!scope)
        return nullptr;
    if (scope->isLocal() && scope->type != ScopeType::eSwitch)
        return findOuterScope(scope->nestedIn);
    return scope;
}

static const Token* assignExpr(const Token* tok)
{
    while (tok->astParent() && astIsLHS(tok)) {
        if (Token::Match(tok->astParent(), "%assign%"))
            return tok->astParent();
        tok = tok->astParent();
    }
    return nullptr;
}

std::pair<bool, bool> PathAnalysis::checkCond(const Token * tok, bool& known)
{
    if (const ValueFlow::Value* v = tok->getKnownValue(ValueFlow::Value::ValueType::INT)) {
        known = true;
        return std::make_pair(!!v->intvalue, !v->intvalue);
    }
    auto it = std::find_if(tok->values().cbegin(), tok->values().cend(), [](const ValueFlow::Value& v) {
        return v.isIntValue();
    });
    // If all possible values are the same, then assume all paths have the same value
    if (it != tok->values().cend() && std::all_of(it, tok->values().cend(), [&](const ValueFlow::Value& v) {
        if (v.isIntValue())
            return v.intvalue == it->intvalue;
        return true;
    })) {
        known = false;
        return std::make_pair(!!it->intvalue, !it->intvalue);
    }
    return std::make_pair(true, true);
}

PathAnalysis::Progress PathAnalysis::forwardRecursive(const Token* tok, Info info, const std::function<PathAnalysis::Progress(const Info&)>& f)
{
    if (!tok)
        return Progress::Continue;
    if (tok->astOperand1() && forwardRecursive(tok->astOperand1(), info, f) == Progress::Break)
        return Progress::Break;
    info.tok = tok;
    if (f(info) == Progress::Break)
        return Progress::Break;
    if (tok->astOperand2() && forwardRecursive(tok->astOperand2(), std::move(info), f) == Progress::Break)
        return Progress::Break;
    return Progress::Continue;
}

PathAnalysis::Progress PathAnalysis::forwardRange(const Token* startToken, const Token* endToken, Info info, const std::function<PathAnalysis::Progress(const Info&)>& f) const
{
    for (const Token *tok = startToken; precedes(tok, endToken); tok = tok->next()) {
        if (Token::Match(tok, "asm|goto|break|continue"))
            return Progress::Break;
        if (Token::Match(tok, "return|throw")) {
            forwardRecursive(tok, std::move(info), f);
            return Progress::Break;
            // Evaluate RHS of assignment before LHS
        }
        if (const Token* assignTok = assignExpr(tok)) {
            if (forwardRecursive(assignTok->astOperand2(), info, f) == Progress::Break)
                return Progress::Break;
            if (forwardRecursive(assignTok->astOperand1(), info, f) == Progress::Break)
                return Progress::Break;
            tok = nextAfterAstRightmostLeaf(assignTok);
            if (!tok)
                return Progress::Break;
        } else if (Token::simpleMatch(tok, "}") && Token::simpleMatch(tok->link()->previous(), ") {") && Token::Match(tok->link()->linkAt(-1)->previous(), "if|while|for (")) {
            const Token * blockStart = tok->link()->linkAt(-1)->previous();
            const Token * condTok = getCondTok(blockStart);
            if (!condTok)
                continue;
            info.errorPath.emplace_back(condTok, "Assuming condition is true.");
            // Traverse a loop a second time
            if (Token::Match(blockStart, "for|while (")) {
                const Token* endCond = blockStart->linkAt(1);
                bool traverseLoop = true;
                // Only traverse simple for loops
                if (Token::simpleMatch(blockStart, "for") && !Token::Match(endCond->tokAt(-3), "; ++|--|%var% %var%|++|-- ) {"))
                    traverseLoop = false;
                // Traverse loop a second time
                if (traverseLoop) {
                    // Traverse condition
                    if (forwardRecursive(condTok, info, f) == Progress::Break)
                        return Progress::Break;
                    // TODO: Should we traverse the body: forwardRange(tok->link(), tok, info, f)?
                }
            }
            if (Token::simpleMatch(tok, "} else {")) {
                tok = tok->linkAt(2);
            }
        } else if (Token::Match(tok, "if|while|for (") && Token::simpleMatch(tok->linkAt(1), ") {")) {
            const Token * endCond = tok->linkAt(1);
            const Token * endBlock = endCond->linkAt(1);
            const Token * condTok = getCondTok(tok);
            if (!condTok)
                continue;
            // Traverse condition
            if (forwardRange(tok->next(), tok->linkAt(1), info, f) == Progress::Break)
                return Progress::Break;
            Info i = info;
            i.known = false;
            i.errorPath.emplace_back(condTok, "Assuming condition is true.");

            // Check if condition is true or false
            bool checkThen = false;
            bool checkElse = false;
            std::tie(checkThen, checkElse) = checkCond(condTok, i.known);

            // Traverse then block
            if (checkThen) {
                if (forwardRange(endCond->next(), endBlock, i, f) == Progress::Break)
                    return Progress::Break;
            }
            // Traverse else block
            if (Token::simpleMatch(endBlock, "} else {")) {
                if (checkElse) {
                    i.errorPath.back().second = "Assuming condition is false.";
                    const Progress result = forwardRange(endCond->next(), endBlock, std::move(i), f);
                    if (result == Progress::Break)
                        return Progress::Break;
                }
                tok = endBlock->linkAt(2);
            } else {
                tok = endBlock;
            }
        } else if (Token::simpleMatch(tok, "} else {")) {
            tok = tok->linkAt(2);
        } else {
            info.tok = tok;
            if (f(info) == Progress::Break)
                return Progress::Break;
        }
        // Prevent infinite recursion
        if (tok->next() == mStart)
            break;
    }
    return Progress::Continue;
}

void PathAnalysis::forward(const std::function<Progress(const Info&)>& f) const
{
    const Scope * endScope = findOuterScope(mStart->scope());
    if (!endScope)
        return;
    const Token * endToken = endScope->bodyEnd;
    Info info{mStart, ErrorPath{}, true};
    forwardRange(mStart, endToken, std::move(info), f);
}

bool reaches(const Token * start, const Token * dest, ErrorPath* errorPath)
{
    PathAnalysis::Info info = PathAnalysis{start}.forwardFind([&](const PathAnalysis::Info& i) {
        return (i.tok == dest);
    });
    if (!info.tok)
        return false;
    if (errorPath)
        errorPath->insert(errorPath->end(), info.errorPath.cbegin(), info.errorPath.cend());
    return true;
}
