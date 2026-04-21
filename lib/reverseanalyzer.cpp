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

#include "reverseanalyzer.h"

#include "analyzer.h"
#include "astutils.h"
#include "errortypes.h"
#include "forwardanalyzer.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueptr.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {
    struct ReverseTraversal {
        ReverseTraversal(const ValuePtr<Analyzer>& analyzer, const TokenList& tokenlist, ErrorLogger& errorLogger, const Settings& settings)
            : analyzer(analyzer), tokenlist(tokenlist), errorLogger(errorLogger), settings(settings)
        {}
        ValuePtr<Analyzer> analyzer;
        const TokenList& tokenlist;
        ErrorLogger& errorLogger;
        const Settings& settings;

        std::pair<bool, bool> evalCond(const Token* tok) const {
            std::vector<MathLib::bigint> result = analyzer->evaluate(tok);
            // TODO: We should convert to bool
            const bool checkThen = std::any_of(result.cbegin(), result.cend(), [](MathLib::bigint x) {
                return x == 1;
            });
            const bool checkElse = std::any_of(result.cbegin(), result.cend(), [](MathLib::bigint x) {
                return x == 0;
            });
            return std::make_pair(checkThen, checkElse);
        }

        bool update(Token* tok) {
            Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Reverse);
            if (action.isInconclusive() && !analyzer->lowerToInconclusive())
                return false;
            if (action.isInvalid())
                return false;
            if (!action.isNone())
                analyzer->update(tok, action, Analyzer::Direction::Reverse);
            return true;
        }

        static Token* getParentFunction(Token* tok)
        {
            if (!tok)
                return nullptr;
            if (!tok->astParent())
                return nullptr;
            int argn = -1;
            if (Token* ftok = getTokenArgumentFunction(tok, argn)) {
                while (!Token::Match(ftok, "(|{")) {
                    if (!ftok)
                        return nullptr;
                    if (ftok->index() >= tok->index())
                        return nullptr;
                    if (!ftok->link() || ftok->str() == ")")
                        ftok = ftok->next();
                    else
                        ftok = ftok->link()->next();
                }
                if (ftok == tok)
                    return nullptr;
                return ftok;
            }
            return nullptr;
        }

        static Token* getTopFunction(Token* tok)
        {
            if (!tok)
                return nullptr;
            if (!tok->astParent())
                return tok;
            Token* parent = tok;
            Token* top = tok;
            while ((parent = getParentFunction(parent)))
                top = parent;
            return top;
        }

        static Token* jumpToStart(Token* tok)
        {
            if (Token::simpleMatch(tok->tokAt(-2), "} else {"))
                tok = tok->linkAt(-2);
            if (Token::simpleMatch(tok->previous(), ") {")) {
                if (Token::simpleMatch(tok->linkAt(-1)->astOperand2(), ";"))
                    return tok->linkAt(-1)->astOperand2();
                return tok->linkAt(-1);
            }
            if (Token::simpleMatch(tok->previous(), "do {"))
                return tok->previous();
            return tok;
        }

        bool updateRecursive(Token* start) {
            bool continueB = true;
            visitAstNodes(start, [&](Token* tok) {
                const Token* parent = tok->astParent();
                while (Token::simpleMatch(parent, ":"))
                    parent = parent->astParent();
                if (isUnevaluated(tok) || isDeadCode(tok, parent))
                    return ChildrenToVisit::none;
                continueB &= update(tok);
                if (continueB)
                    return ChildrenToVisit::op1_and_op2;
                return ChildrenToVisit::done;
            });
            return continueB;
        }

        Analyzer::Action analyzeRecursive(const Token* start) const {
            Analyzer::Action result = Analyzer::Action::None;
            visitAstNodes(start, [&](const Token* tok) {
                result |= analyzer->analyze(tok, Analyzer::Direction::Reverse);
                if (result.isModified())
                    return ChildrenToVisit::done;
                return ChildrenToVisit::op1_and_op2;
            });
            return result;
        }

        Analyzer::Action analyzeRange(const Token* start, const Token* end) const {
            Analyzer::Action result = Analyzer::Action::None;
            for (const Token* tok = start; tok && tok != end; tok = tok->next()) {
                Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Reverse);
                if (action.isModified())
                    return action;
                result |= action;
            }
            return result;
        }

        Token* isDeadCode(Token* tok, const Token* end = nullptr) const {
            int opSide = 0;
            for (; tok && tok->astParent(); tok = tok->astParent()) {
                if (tok == end)
                    break;
                Token* parent = tok->astParent();
                if (Token::simpleMatch(parent, ":")) {
                    if (astIsLHS(tok))
                        opSide = 1;
                    else if (astIsRHS(tok))
                        opSide = 2;
                    else
                        opSide = 0;
                }
                if (tok != parent->astOperand2())
                    continue;
                if (Token::simpleMatch(parent, ":"))
                    parent = parent->astParent();
                if (!Token::Match(parent, "%oror%|&&|?"))
                    continue;
                const Token* condTok = parent->astOperand1();
                if (!condTok)
                    continue;
                bool checkThen, checkElse;
                std::tie(checkThen, checkElse) = evalCond(condTok);

                if (parent->str() == "?") {
                    if (checkElse && opSide == 1)
                        return parent;
                    if (checkThen && opSide == 2)
                        return parent;
                }
                if (!checkThen && parent->str() == "&&")
                    return parent;
                if (!checkElse && parent->str() == "||")
                    return parent;
            }
            return nullptr;
        }

        /**
         * @throws InternalError thrown on cyclic analysis
         */
        void traverse(Token* start, const Token* end = nullptr) {
            if (start == end)
                return;
            std::size_t i = start->index();
            for (Token* tok = start->previous(); succeeds(tok, end); tok = tok->previous()) {
                if (tok->index() >= i)
                    throw InternalError(tok, "Cyclic reverse analysis.");
                i = tok->index();
                if (tok == start || (tok->str() == "{" && (tok->scope()->type == ScopeType::eFunction ||
                                                           tok->scope()->type == ScopeType::eLambda))) {
                    const Function* f = tok->scope()->function;
                    if (f && f->isConstructor()) {
                        if (const Token* initList = f->constructorMemberInitialization())
                            traverse(tok->previous(), tok->tokAt(initList->index() - tok->index()));
                    }
                    break;
                }
                if (Token::Match(tok, "return|break|continue"))
                    break;
                if (Token::Match(tok, "%name% :"))
                    break;
                if (Token::simpleMatch(tok, ":"))
                    continue;
                // Evaluate LHS of assignment before RHS
                if (Token* assignTok = assignExpr(tok)) {
                    // If assignTok has broken ast then stop
                    if (!assignTok->astOperand1() || !assignTok->astOperand2())
                        break;
                    Token* assignTop = assignTok;
                    bool continueB = true;
                    while (assignTop->isAssignmentOp()) {
                        if (!Token::Match(assignTop->astOperand1(), "%assign%")) {
                            continueB &= updateRecursive(assignTop->astOperand1());
                        }
                        if (!assignTop->astParent() || Token::simpleMatch(assignTop->astParent(), ";"))
                            break;
                        assignTop = assignTop->astParent();
                    }
                    // Is assignment in dead code
                    if (Token* parent = isDeadCode(assignTok)) {
                        tok = parent;
                        continue;
                    }
                    // Simple assign
                    if (assignTok->str() == "=" && (assignTok->astParent() == assignTop || assignTok == assignTop)) {
                        Analyzer::Action rhsAction =
                            analyzer->analyze(assignTok->astOperand2(), Analyzer::Direction::Reverse);
                        Analyzer::Action lhsAction =
                            analyzer->analyze(assignTok->astOperand1(), Analyzer::Direction::Reverse);
                        // Assignment from
                        if (rhsAction.isRead() && !lhsAction.isInvalid() && assignTok->astOperand1()->exprId() > 0) {
                            const std::string info = "Assignment from '" + assignTok->expressionString() + "'";
                            ValuePtr<Analyzer> a = analyzer->reanalyze(assignTok->astOperand1(), info);
                            if (a) {
                                valueFlowGenericForward(nextAfterAstRightmostLeaf(assignTok->astOperand2()),
                                                        assignTok->astOperand2()->scope()->bodyEnd,
                                                        a,
                                                        tokenlist,
                                                        errorLogger,
                                                        settings);
                            }
                            // Assignment to
                        } else if (lhsAction.matches() && !assignTok->astOperand2()->hasKnownIntValue() &&
                                   assignTok->astOperand2()->exprId() > 0 &&
                                   isConstExpression(assignTok->astOperand2(), settings.library)) {
                            const std::string info = "Assignment to '" + assignTok->expressionString() + "'";
                            ValuePtr<Analyzer> a = analyzer->reanalyze(assignTok->astOperand2(), info);
                            if (a) {
                                valueFlowGenericForward(nextAfterAstRightmostLeaf(assignTok->astOperand2()),
                                                        assignTok->astOperand2()->scope()->bodyEnd,
                                                        a,
                                                        tokenlist,
                                                        errorLogger,
                                                        settings);
                                valueFlowGenericReverse(assignTok->astOperand1()->previous(), end, a, tokenlist, errorLogger, settings);
                            }
                        }
                    }
                    if (!continueB)
                        break;
                    if (!updateRecursive(assignTop->astOperand2()))
                        break;
                    tok = previousBeforeAstLeftmostLeaf(assignTop)->next();
                    continue;
                }
                if (tok->str() == ")" && !isUnevaluated(tok)) {
                    if (Token* top = getTopFunction(tok->link())) {
                        if (!updateRecursive(top))
                            break;
                        Token* next = previousBeforeAstLeftmostLeaf(top);
                        if (next && precedes(next, tok))
                            tok = next->next();
                    }
                    continue;
                }
                if (tok->str() == "}") {
                    Token* condTok = getCondTokFromEnd(tok);
                    if (!condTok)
                        break;
                    Analyzer::Action condAction = analyzeRecursive(condTok);
                    if (condAction.isModified())
                        break;
                    const bool inLoop = Token::Match(condTok->astTop()->previous(), "for|while (");
                    // Evaluate condition of for and while loops first
                    if (inLoop) {
                        if (Token::findmatch(tok->link(), "goto|break", tok))
                            break;
                        valueFlowGenericForward(condTok, analyzer, tokenlist, errorLogger, settings);
                    }
                    Token* thenEnd;
                    const bool hasElse = Token::simpleMatch(tok->link()->tokAt(-2), "} else {");
                    if (hasElse) {
                        thenEnd = tok->link()->tokAt(-2);
                    } else {
                        thenEnd = tok;
                    }

                    Analyzer::Action thenAction = analyzeRange(thenEnd->link(), thenEnd);
                    Analyzer::Action elseAction = Analyzer::Action::None;
                    if (hasElse) {
                        elseAction = analyzeRange(tok->link(), tok);
                    }
                    if (thenAction.isModified() && inLoop)
                        break;
                    if (thenAction.isModified() && !elseAction.isModified())
                        analyzer->assume(condTok, hasElse);
                    else if (elseAction.isModified() && !thenAction.isModified())
                        analyzer->assume(condTok, !hasElse);
                    // Bail if one of the branches are read to avoid FPs due to over constraints
                    else if (thenAction.isIdempotent() || elseAction.isIdempotent() || thenAction.isRead() ||
                             elseAction.isRead())
                        break;
                    if (thenAction.isInvalid() || elseAction.isInvalid())
                        break;

                    if (!thenAction.isModified() && !elseAction.isModified())
                        valueFlowGenericForward(condTok, analyzer, tokenlist, errorLogger, settings);
                    else if (condAction.isRead())
                        break;
                    // If the condition modifies the variable then bail
                    if (condAction.isModified())
                        break;
                    tok = jumpToStart(tok->link());
                    continue;
                }
                if (tok->str() == "{") {
                    if (tok->previous() &&
                        (Token::simpleMatch(tok->previous(), "do") ||
                         (tok->strAt(-1) == ")" && Token::Match(tok->linkAt(-1)->previous(), "for|while (")))) {
                        Analyzer::Action action = analyzeRange(tok, tok->link());
                        if (action.isModified())
                            break;
                    }
                    Token* condTok = getCondTokFromEnd(tok->link());
                    if (condTok) {
                        Analyzer::Result r = valueFlowGenericForward(condTok, analyzer, tokenlist, errorLogger, settings);
                        if (r.action.isModified())
                            break;
                    }
                    tok = jumpToStart(tok);
                    continue;
                }
                if (Token* next = isUnevaluated(tok)) {
                    tok = next;
                    continue;
                }
                if (Token* parent = isDeadCode(tok)) {
                    tok = parent;
                    continue;
                }
                if (tok->str() == "case") {
                    const Scope* scope = tok->scope();
                    while (scope && scope->type != ScopeType::eSwitch)
                        scope = scope->nestedIn;
                    if (!scope || scope->type != ScopeType::eSwitch)
                        break;
                    tok = tok->tokAt(scope->bodyStart->index() - tok->index() - 1);
                    continue;
                }
                if (!update(tok))
                    break;
            }
        }

        static Token* assignExpr(Token* tok) {
            if (Token::Match(tok, ")|}"))
                tok = tok->link();
            while (tok->astParent() && (astIsRHS(tok) || !tok->astParent()->isBinaryOp())) {
                if (tok->astParent()->isAssignmentOp())
                    return tok->astParent();
                tok = tok->astParent();
            }
            return nullptr;
        }

        static Token* isUnevaluated(Token* tok) {
            if (Token::Match(tok, ")|>") && tok->link()) {
                Token* start = tok->link();
                if (::isUnevaluated(start->previous()))
                    return start->previous();
                if (Token::simpleMatch(start, "<"))
                    return start;
            }
            return nullptr;
        }
    };
}

void valueFlowGenericReverse(Token* start, const Token* end, const ValuePtr<Analyzer>& a, const TokenList& tokenlist, ErrorLogger& errorLogger, const Settings& settings)
{
    if (a->invalid())
        return;
    ReverseTraversal rt{a, tokenlist, errorLogger, settings};
    rt.traverse(start, end);
}
