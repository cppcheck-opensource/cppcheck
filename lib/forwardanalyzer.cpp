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

#include "forwardanalyzer.h"

#include "analyzer.h"
#include "astutils.h"
#include "config.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueptr.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <functional>
#include <list>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
    struct ForwardTraversal {
        enum class Progress : std::uint8_t { Continue, Break, Skip };
        ForwardTraversal(const ValuePtr<Analyzer>& analyzer, const TokenList& tokenList, ErrorLogger& errorLogger, const Settings& settings)
            : analyzer(analyzer), tokenList(tokenList), errorLogger(errorLogger), settings(settings)
        {}
        ValuePtr<Analyzer> analyzer;
        const TokenList& tokenList;
        ErrorLogger& errorLogger;
        const Settings& settings;
        Analyzer::Action actions;
        bool analyzeOnly{};
        bool analyzeTerminate{};
        Analyzer::Terminate terminate = Analyzer::Terminate::None;
        std::vector<Token*> loopEnds;
        int branchCount = 0;
        // Nested condition-fork depth on this lineage (copied by fork()); bounds the fan-out.
        int forkDepth = 0;
        // Total forks of the traversal (shared via the fork() copy); backstop past the depth bound.
        std::shared_ptr<int> forkBudget = std::make_shared<int>(0);

        Progress Break(Analyzer::Terminate t = Analyzer::Terminate::None) {
            if ((!analyzeOnly || analyzeTerminate) && t != Analyzer::Terminate::None)
                terminate = t;
            return Progress::Break;
        }

        struct Branch {
            explicit Branch(Token* tok = nullptr) : endBlock(tok) {}
            Token* endBlock = nullptr;
            Analyzer::Action action = Analyzer::Action::None;
            bool check = false;
            bool escape = false;
            bool escapeUnknown = false;
            bool isEscape() const {
                return escape || escapeUnknown;
            }
            bool isConclusiveEscape() const {
                return escape && !escapeUnknown;
            }
            bool isModified() const {
                return action.isModified() && !isConclusiveEscape();
            }
            bool isInconclusive() const {
                return action.isInconclusive() && !isConclusiveEscape();
            }
            bool isDead() const {
                return action.isModified() || action.isInconclusive() || isEscape();
            }
            bool hasGoto() const {
                return endBlock ? ForwardTraversal::hasGoto(endBlock) : false;
            }
        };

        bool stopUpdates() {
            analyzeOnly = true;
            return actions.isModified();
        }

        bool stopOnCondition(const Token* condTok) const
        {
            if (analyzer->isConditional() && findAstNode(condTok, [](const Token* tok) {
                return tok->isIncompleteVar();
            }))
                return true;
            return analyzer->stopOnCondition(condTok);
        }

        std::pair<bool, bool> evalCond(const Token* tok, const Token* ctx = nullptr) const {
            if (!tok)
                return std::make_pair(false, false);
            std::vector<MathLib::bigint> result = analyzer->evaluate(tok, ctx);
            // TODO: We should convert to bool
            const bool checkThen = std::any_of(result.cbegin(), result.cend(), [](MathLib::bigint x) {
                return x != 0;
            });
            const bool checkElse = std::any_of(result.cbegin(), result.cend(), [](MathLib::bigint x) {
                return x == 0;
            });
            return std::make_pair(checkThen, checkElse);
        }

        bool isConditionTrue(const Token* tok, const Token* ctx = nullptr) const {
            return evalCond(tok, ctx).first;
        }

        template<class T, class F, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
        Progress traverseTok(T* tok, const F &f, bool traverseUnknown, T** out = nullptr) {
            if (Token::Match(tok, "asm|goto"))
                return Break(Analyzer::Terminate::Bail);
            if (Token::Match(tok, "setjmp|longjmp (")) {
                // Traverse the parameters of the function before escaping
                traverseRecursive(tok->next()->astOperand2(), f, traverseUnknown);
                return Break(Analyzer::Terminate::Bail);
            }
            if (Token::simpleMatch(tok, "continue")) {
                if (loopEnds.empty())
                    return Break(Analyzer::Terminate::Escape);
                // If we are in a loop then jump to the end
                if (out)
                    *out = loopEnds.back();
            } else if (Token::Match(tok, "return|throw")) {
                traverseRecursive(tok->astOperand2(), f, traverseUnknown);
                traverseRecursive(tok->astOperand1(), f, traverseUnknown);
                return Break(Analyzer::Terminate::Escape);
            } else if (Token::Match(tok, "%name% (") && isEscapeFunction(tok, settings.library)) {
                // Traverse the parameters of the function before escaping
                traverseRecursive(tok->next()->astOperand2(), f, traverseUnknown);
                return Break(Analyzer::Terminate::Escape);
            } else if (isUnevaluated(tok->previous())) {
                if (out)
                    *out = tok->link();
                return Progress::Skip;
            } else if (tok->astOperand1() && tok->astOperand2() && Token::Match(tok, "?|&&|%oror%")) {
                if (traverseConditional(tok, f, traverseUnknown) == Progress::Break)
                    return Break();
                if (out)
                    *out = nextAfterAstRightmostLeaf(tok);
                return Progress::Skip;
                // Skip lambdas
            } else if (T* lambdaEndToken = findLambdaEndToken(tok)) {
                if (checkScope(lambdaEndToken).isModified())
                    return Break(Analyzer::Terminate::Bail);
                if (out)
                    *out = lambdaEndToken->next();
                // Skip class scope
            } else if (tok->str() == "{" && tok->scope() && tok->scope()->isClassOrStruct()) {
                if (out)
                    *out = tok->link();
            } else {
                if (f(tok) == Progress::Break)
                    return Break();
            }
            return Progress::Continue;
        }

        template<class T, class F, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
        Progress traverseRecursive(T* tok, const F &f, bool traverseUnknown, unsigned int recursion=0) {
            if (!tok)
                return Progress::Continue;
            if (recursion > 10000)
                return Progress::Skip;
            T* firstOp = tok->astOperand1();
            T* secondOp = tok->astOperand2();
            // Evaluate:
            //     1. RHS of assignment before LHS
            //     2. Unary op before operand
            //     3. Function arguments before function call
            if (tok->isAssignmentOp() || !secondOp || isFunctionCall(tok))
                std::swap(firstOp, secondOp);
            if (firstOp && traverseRecursive(firstOp, f, traverseUnknown, recursion+1) == Progress::Break)
                return Break();
            const Progress p = tok->isAssignmentOp() ? Progress::Continue : traverseTok(tok, f, traverseUnknown);
            if (p == Progress::Break)
                return Break();
            if (p == Progress::Continue && secondOp && traverseRecursive(secondOp, f, traverseUnknown, recursion+1) == Progress::Break)
                return Break();
            if (tok->isAssignmentOp() && traverseTok(tok, f, traverseUnknown) == Progress::Break)
                return Break();
            return Progress::Continue;
        }

        template<class T, class F, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
        Progress traverseConditional(T* tok, F f, bool traverseUnknown) {
            Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Forward);
            if (action.isNone() && Token::Match(tok, "?|&&|%oror%") && tok->astOperand1() && tok->astOperand2()) {
                const T* condTok = tok->astOperand1();
                T* childTok = tok->astOperand2();
                bool checkThen, checkElse;
                std::tie(checkThen, checkElse) = evalCond(condTok);
                if (!checkThen && !checkElse) {
                    if (!traverseUnknown && stopOnCondition(condTok) && tok->str() != "?" && stopUpdates()) {
                        return Progress::Continue;
                    }
                    checkThen = true;
                    checkElse = true;
                }
                if (childTok->str() == ":") {
                    if (checkThen && traverseRecursive(childTok->astOperand1(), f, traverseUnknown) == Progress::Break)
                        return Break();
                    if (checkElse && traverseRecursive(childTok->astOperand2(), f, traverseUnknown) == Progress::Break)
                        return Break();
                } else {
                    if (!checkThen && tok->str() == "&&")
                        return Progress::Continue;
                    if (!checkElse && tok->str() == "||")
                        return Progress::Continue;
                    if (traverseRecursive(childTok, f, traverseUnknown) == Progress::Break)
                        return Break();
                }
            } else {
                return f(tok, action);
            }
            return Progress::Continue;
        }

        Progress update(Token* tok) {
            Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Forward);
            return update(tok, action);
        }

        Progress update(Token* tok, Analyzer::Action action)
        {
            actions |= action;
            if (!action.isNone() && !analyzeOnly)
                analyzer->update(tok, action, Analyzer::Direction::Forward);
            if (action.isInconclusive() && !analyzer->lowerToInconclusive())
                return Break(Analyzer::Terminate::Inconclusive);
            if (action.isInvalid())
                return Break(Analyzer::Terminate::Modified);
            if (action.isWrite() && !action.isRead())
                // Analysis of this write will continue separately
                return Break(Analyzer::Terminate::Modified);
            return Progress::Continue;
        }

        struct AsUpdate {
            ForwardTraversal* self = nullptr;

            explicit AsUpdate(ForwardTraversal* self) : self(self) {}

            template<class... Ts>
            Progress operator()(Ts... xs) const
            {
                assert(self);
                return self->update(xs ...);
            }
        };

        Progress updateTok(Token* tok, Token** out = nullptr) {
            return traverseTok(tok, AsUpdate{this}, false, out);
        }

        Progress updateRecursive(Token* tok) {
            return traverseRecursive(tok, AsUpdate{this}, false);
        }

        struct AsAnalyze {
            ForwardTraversal* self = nullptr;
            Analyzer::Action* result = nullptr;

            AsAnalyze(ForwardTraversal* self, Analyzer::Action* result) : self(self), result(result) {}

            Progress operator()(const Token* tok) const
            {
                assert(self);
                assert(result);
                return (*this)(tok, self->analyzer->analyze(tok, Analyzer::Direction::Forward));
            }

            Progress operator()(const Token* /*unused*/, Analyzer::Action action) const
            {
                assert(self);
                assert(result);
                *result = action;
                if (result->isModified() || result->isInconclusive())
                    return self->Break();
                return Progress::Continue;
            }
        };

        Analyzer::Action analyzeRecursive(const Token* start) {
            Analyzer::Action result = Analyzer::Action::None;
            traverseRecursive(start, AsAnalyze{this, &result}, true);
            return result;
        }

        Analyzer::Action analyzeRange(const Token* start, const Token* end) const {
            Analyzer::Action result = Analyzer::Action::None;
            for (const Token* tok = start; tok && tok != end; tok = tok->next()) {
                Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Forward);
                if (action.isModified() || action.isInconclusive())
                    return action;
                result |= action;
            }
            return result;
        }

        ForwardTraversal fork(bool analyze = false) const {
            ForwardTraversal ft = *this;
            if (analyze) {
                ft.analyzeOnly = true;
                ft.analyzeTerminate = true;
            }
            ft.actions = Analyzer::Action::None;
            return ft;
        }

        std::vector<ForwardTraversal> tryForkScope(Token* endBlock, bool isModified = false) const {
            if (analyzer->updateScope(endBlock, isModified)) {
                ForwardTraversal ft = fork();
                return {std::move(ft)};
            }
            return std::vector<ForwardTraversal> {};
        }

        std::vector<ForwardTraversal> tryForkUpdateScope(Token* endBlock, bool isModified = false) const {
            std::vector<ForwardTraversal> result = tryForkScope(endBlock, isModified);
            for (ForwardTraversal& ft : result)
                ft.updateScope(endBlock);
            return result;
        }

        static bool hasGoto(const Token* endBlock) {
            return Token::findsimplematch(endBlock->link(), "goto", endBlock);
        }

        static bool hasJump(const Token* endBlock) {
            return Token::findmatch(endBlock->link(), "goto|break", endBlock);
        }

        bool isEscapeScope(const Token* endBlock, bool& unknown) const {
            const Token* ftok = nullptr;
            const bool r = isReturnScope(endBlock, settings.library, &ftok);
            if (!r && ftok)
                unknown = true;
            return r;
        }

        Analyzer::Action analyzeScope(const Token* endBlock) const {
            return analyzeRange(endBlock->link(), endBlock);
        }

        Analyzer::Action checkScope(Token* endBlock) const {
            Analyzer::Action a = analyzeScope(endBlock);
            tryForkUpdateScope(endBlock, a.isModified());
            return a;
        }

        Analyzer::Action checkScope(const Token* endBlock) const {
            Analyzer::Action a = analyzeScope(endBlock);
            return a;
        }

        Progress updateBranch(Branch& branch, int depth)
        {
            // Save and reset actions
            Analyzer::Action prevActions = actions;
            actions = Analyzer::Action::None;
            Progress p = updateRange(branch.endBlock->link(), branch.endBlock, depth);
            branch.action |= actions;
            // Restore actions
            actions |= prevActions;

            if (terminate == Analyzer::Terminate::Escape) {
                branch.escape = true;
                // The traversal followed an escaping path, but if the scope does not structurally
                // always escape then another path (e.g. a modified fork) falls through, so the escape
                // is only conditional - keep isModified() meaningful by not treating it as conclusive.
                bool structuralUnknown = false;
                const bool structuralEscape = isEscapeScope(branch.endBlock, structuralUnknown);
                branch.escapeUnknown = !structuralEscape || structuralUnknown;
                // The traversal stopped at the escape, so the rest of the scope was not walked; a
                // fall-through path could still modify the value there - include the whole scope's
                // actions so isModified() sees it.
                if (branch.escapeUnknown)
                    branch.action |= analyzeScope(branch.endBlock);
            } else {
                // Detect an escape the traversal did not flag (e.g. an unknown noreturn call);
                // escapeUnknown reports a possible (unknown) escape.
                branch.escape = isEscapeScope(branch.endBlock, branch.escapeUnknown);
                if (terminate != Analyzer::Terminate::None && terminate != Analyzer::Terminate::Modified) {
                    branch.action |= analyzeScope(branch.endBlock);
                }
            }

            return p;
        }

        bool reentersLoop(Token* endBlock, const Token* condTok, const Token* stepTok) const {
            if (!condTok)
                return true;
            if (Token::simpleMatch(condTok, ":"))
                return true;
            bool stepChangesCond = false;
            if (stepTok) {
                std::pair<const Token*, const Token*> exprToks = stepTok->findExpressionStartEndTokens();
                if (exprToks.first != nullptr && exprToks.second != nullptr)
                    stepChangesCond |=
                        findExpressionChanged(condTok, exprToks.first, exprToks.second->next(), settings) != nullptr;
            }
            const bool bodyChangesCond = findExpressionChanged(condTok, endBlock->link(), endBlock, settings);
            // Check for mutation in the condition
            const bool condChanged =
                nullptr != findAstNode(condTok, [&](const Token* tok) {
                return isVariableChanged(tok, 0, settings);
            });
            const bool changed = stepChangesCond || bodyChangesCond || condChanged;
            if (!changed)
                return true;
            ForwardTraversal ft = fork(true);
            ft.updateScope(endBlock);
            return ft.isConditionTrue(condTok) && bodyChangesCond;
        }

        Progress updateInnerLoop(Token* endBlock, Token* stepTok, Token* condTok) {
            loopEnds.push_back(endBlock);
            OnExit oe{[&] {
                    loopEnds.pop_back();
                }};
            if (endBlock && updateScope(endBlock) == Progress::Break)
                return Break();
            if (stepTok && updateRecursive(stepTok) == Progress::Break)
                return Break();
            if (condTok && !Token::simpleMatch(condTok, ":") && updateRecursive(condTok) == Progress::Break)
                return Break();
            return Progress::Continue;
        }

        Progress updateLoop(const Token* endToken,
                            Token* endBlock,
                            Token* condTok,
                            Token* initTok = nullptr,
                            Token* stepTok = nullptr,
                            bool exit = false) {
            if (initTok && updateRecursive(initTok) == Progress::Break)
                return Break();
            const bool isDoWhile = precedes(endBlock, condTok);
            bool checkThen = true;
            bool checkElse = false;
            if (condTok && !Token::simpleMatch(condTok, ":"))
                std::tie(checkThen, checkElse) = evalCond(condTok, isDoWhile ? endBlock->previous() : nullptr);
            // exiting a do while(false)
            if (checkElse && exit) {
                if (hasJump(endBlock)) {
                    if (!analyzer->lowerToPossible())
                        return Break(Analyzer::Terminate::Bail);
                    if (analyzer->isConditional() && stopUpdates())
                        return Break(Analyzer::Terminate::Conditional);
                }
                return Progress::Continue;
            }
            Analyzer::Action bodyAnalysis = analyzeScope(endBlock);
            Analyzer::Action allAnalysis = bodyAnalysis;
            Analyzer::Action condAnalysis;
            if (condTok) {
                condAnalysis = analyzeRecursive(condTok);
                allAnalysis |= condAnalysis;
            }
            if (stepTok)
                allAnalysis |= analyzeRecursive(stepTok);
            actions |= allAnalysis;
            // do while(false) is not really a loop
            if (checkElse && isDoWhile &&
                (condTok->hasKnownIntValue() ||
                 (!bodyAnalysis.isModified() && !condAnalysis.isModified() && condAnalysis.isRead()))) {
                if (updateScope(endBlock) == Progress::Break)
                    return Break();
                return updateRecursive(condTok);
            }
            if (allAnalysis.isInconclusive()) {
                if (!analyzer->lowerToInconclusive())
                    return Break(Analyzer::Terminate::Bail);
            } else if (allAnalysis.isModified() || (exit && allAnalysis.isIdempotent())) {
                if (!analyzer->lowerToPossible())
                    return Break(Analyzer::Terminate::Bail);
            }

            if (condTok && !Token::simpleMatch(condTok, ":")) {
                if (!isDoWhile || (!bodyAnalysis.isModified() && !bodyAnalysis.isIdempotent()))
                    if (updateRecursive(condTok) == Progress::Break)
                        return Break();
            }
            if (!checkThen && !checkElse && !isDoWhile && stopOnCondition(condTok) && stopUpdates())
                return Break(Analyzer::Terminate::Conditional);
            // condition is false, we don't enter the loop
            if (checkElse && !isDoWhile)
                return Progress::Continue;
            if (checkThen || isDoWhile) {
                // Since we are re-entering the loop then assume the condition is true to update the state
                if (exit)
                    analyzer->assume(condTok, true, Analyzer::Assume::Quiet | Analyzer::Assume::Absolute);
                if (updateInnerLoop(endBlock, stepTok, condTok) == Progress::Break)
                    return Break();
                // If loop re-enters then it could be modified again
                if (allAnalysis.isModified() && reentersLoop(endBlock, condTok, stepTok))
                    return Break(Analyzer::Terminate::Bail);
                if (allAnalysis.isIncremental())
                    return Break(Analyzer::Terminate::Bail);
            } else if (allAnalysis.isModified()) {
                std::vector<ForwardTraversal> ftv = tryForkScope(endBlock, allAnalysis.isModified());
                bool forkContinue = true;
                for (ForwardTraversal& ft : ftv) {
                    if (condTok)
                        ft.analyzer->assume(condTok, false, Analyzer::Assume::Quiet);
                    if (ft.updateInnerLoop(endBlock, stepTok, condTok) == Progress::Break)
                        forkContinue = false;
                }

                // TODO: Don't bail on missing condition
                if (!condTok)
                    return Break(Analyzer::Terminate::Bail);
                if (analyzer->isConditional() && stopUpdates())
                    return Break(Analyzer::Terminate::Conditional);
                analyzer->assume(condTok, false);
                if (forkContinue) {
                    for (ForwardTraversal& ft : ftv) {
                        if (!ft.actions.isIncremental())
                            ft.updateRange(endBlock, endToken);
                    }
                }
                if (allAnalysis.isIncremental())
                    return Break(Analyzer::Terminate::Bail);
            } else {
                if (updateInnerLoop(endBlock, stepTok, condTok) == Progress::Break)
                    return Progress::Break;
                if (allAnalysis.isIncremental())
                    return Break(Analyzer::Terminate::Bail);
            }
            return Progress::Continue;
        }

        Progress updateLoopExit(const Token* endToken,
                                Token* endBlock,
                                Token* condTok,
                                Token* initTok = nullptr,
                                Token* stepTok = nullptr) {
            return updateLoop(endToken, endBlock, condTok, initTok, stepTok, true);
        }

        Progress updateScope(Token* endBlock, int depth = 20)
        {
            if (!endBlock)
                return Break();
            assert(endBlock->link());
            Token* ctx = endBlock->link()->previous();
            if (Token::simpleMatch(ctx, ")"))
                ctx = ctx->link()->previous();
            if (ctx)
                analyzer->updateState(ctx);
            return updateRange(endBlock->link(), endBlock, depth);
        }

        /**
         * @throws InternalError thrown on cyclic analysis
         */
        Progress updateRange(Token* start, const Token* end, int depth = 20) {
            if (depth < 0)
                return Break(Analyzer::Terminate::Bail);
            std::size_t i = 0;
            for (Token* tok = start; precedes(tok, end); tok = tok->next()) {
                Token* next = nullptr;
                if (tok->index() <= i)
                    throw InternalError(tok, "Cyclic forward analysis.");
                i = tok->index();

                if (tok->link()) {
                    // Skip casts..
                    if (tok->str() == "(" && !tok->astOperand2() && tok->isCast()) {
                        tok = tok->link();
                        continue;
                    }
                    // Skip template arguments..
                    if (tok->str() == "<") {
                        tok = tok->link();
                        continue;
                    }
                }

                // Evaluate RHS of assignment before LHS
                if (Token* assignTok = assignExpr(tok)) {
                    if (updateRecursive(assignTok) == Progress::Break)
                        return Break();
                    tok = nextAfterAstRightmostLeaf(assignTok);
                    if (!tok)
                        return Break();
                } else if (Token::simpleMatch(tok, ") {") && Token::Match(tok->link()->previous(), "for|while (") &&
                           !Token::simpleMatch(tok->link()->astOperand2(), ":")) {
                    // In the middle of a loop structure so bail
                    return Break(Analyzer::Terminate::Bail);
                } else if (tok->str() == ";" && tok->astParent()) {
                    Token* top = tok->astTop();
                    if (Token::Match(top->previous(), "for|while (") && Token::simpleMatch(top->link(), ") {")) {
                        Token* endCond = top->link();
                        Token* endBlock = endCond->linkAt(1);
                        Token* condTok = getCondTok(top);
                        Token* stepTok = getStepTok(top);
                        // The semicolon should belong to the initTok otherwise something went wrong, so just bail
                        if (tok->astOperand2() != condTok && !Token::simpleMatch(tok->astOperand2(), ";"))
                            return Break(Analyzer::Terminate::Bail);
                        if (updateLoop(end, endBlock, condTok, nullptr, stepTok) == Progress::Break)
                            return Break();
                    }
                } else if (tok->str() == "break") {
                    const Token *scopeEndToken = findNextTokenFromBreak(tok);
                    if (!scopeEndToken)
                        return Break();
                    tok = skipTo(tok, scopeEndToken, end);
                    if (!precedes(tok, end))
                        return Break(Analyzer::Terminate::Escape);
                    if (!analyzer->lowerToPossible())
                        return Break(Analyzer::Terminate::Bail);
                    // TODO: Don't break, instead move to the outer scope
                    if (!tok)
                        return Break();
                } else if (!tok->variable() && (Token::Match(tok, "%name% :") || tok->str() == "case")) {
                    if (!analyzer->lowerToPossible())
                        return Break(Analyzer::Terminate::Bail);
                } else if (tok->link() && tok->str() == "}" && tok == tok->scope()->bodyEnd) { // might be an init list
                    const Scope* scope = tok->scope();
                    if (contains({ScopeType::eDo, ScopeType::eFor, ScopeType::eWhile, ScopeType::eIf, ScopeType::eElse, ScopeType::eSwitch}, scope->type)) {
                        const bool inElse = scope->type == ScopeType::eElse;
                        const bool inDoWhile = scope->type == ScopeType::eDo;
                        const bool inLoop = contains({ScopeType::eDo, ScopeType::eFor, ScopeType::eWhile}, scope->type);
                        const bool hasElse = Token::simpleMatch(tok, "} else {");
                        Token* condTok = getCondTokFromEnd(tok);
                        if (!condTok)
                            return Break();
                        // When the 'else' branch escapes (e.g. returns), control can only continue
                        // here via the 'then' branch, so the value established there is still
                        // definite - keep it known instead of lowering to possible.
                        bool elseEscape = false;
                        if (!inLoop && !inElse && hasElse) {
                            bool unknownEscape = false;
                            elseEscape = isEscapeScope(tok->linkAt(2), unknownEscape);
                        }
                        if (!condTok->hasKnownIntValue() || inLoop) {
                            if (!elseEscape && !analyzer->lowerToPossible())
                                return Break(Analyzer::Terminate::Bail);
                        } else if (condTok->getKnownIntValue() == inElse) {
                            return Break();
                        }
                        // Handle loop
                        if (inLoop) {
                            Token* stepTok = getStepTokFromEnd(tok);
                            bool checkThen, checkElse;
                            std::tie(checkThen, checkElse) = evalCond(condTok);
                            if (stepTok && !checkElse) {
                                if (updateRecursive(stepTok) == Progress::Break)
                                    return Break();
                                if (updateRecursive(condTok) == Progress::Break)
                                    return Break();
                                // Reevaluate condition
                                std::tie(checkThen, checkElse) = evalCond(condTok);
                            }
                            if (!checkElse) {
                                if (updateLoopExit(end, tok, condTok, nullptr, stepTok) == Progress::Break)
                                    return Break();
                            }
                        }
                        analyzer->assume(condTok, !inElse, Analyzer::Assume::Quiet);
                        assert(!inDoWhile || Token::simpleMatch(tok, "} while ("));
                        if (hasElse || inDoWhile)
                            tok = tok->linkAt(2);
                    } else if (contains({ScopeType::eTry, ScopeType::eCatch}, scope->type)) {
                        if (!analyzer->lowerToPossible())
                            return Break(Analyzer::Terminate::Bail);
                    } else if (scope->type == ScopeType::eLambda) {
                        return Break();
                    }
                } else if (tok->isControlFlowKeyword() && Token::Match(tok, "if|while|for (") &&
                           Token::simpleMatch(tok->linkAt(1), ") {")) {
                    if ((settings.vfOptions.maxForwardBranches > 0) && (++branchCount > settings.vfOptions.maxForwardBranches)) {
                        // TODO: should be logged on function-level instead of file-level
                        if (settings.severity.isEnabled(Severity::information)) {
                            reportError(Severity::information, "normalCheckLevelMaxBranches", "Limiting analysis of branches. Use --check-level=exhaustive to analyze all branches.");
                        }
                        return Break(Analyzer::Terminate::Bail);
                    }
                    Token* endCond = tok->linkAt(1);
                    Token* endBlock = endCond->linkAt(1);
                    Token* condTok = getCondTok(tok);
                    Token* initTok = getInitTok(tok);
                    if (initTok && updateRecursive(initTok) == Progress::Break)
                        return Break();
                    if (Token::Match(tok, "for|while (")) {
                        // For-range loop
                        if (Token::simpleMatch(condTok, ":")) {
                            Token* conTok = condTok->astOperand2();
                            if (conTok && updateRecursive(conTok) == Progress::Break)
                                return Break();
                            bool isEmpty = false;
                            std::vector<MathLib::bigint> result =
                                analyzer->evaluate(Analyzer::Evaluate::ContainerEmpty, conTok);
                            if (result.empty())
                                analyzer->assume(conTok, false, Analyzer::Assume::ContainerEmpty);
                            else
                                isEmpty = result.front() != 0;
                            if (!isEmpty && updateLoop(end, endBlock, condTok) == Progress::Break)
                                return Break();
                        } else {
                            Token* stepTok = getStepTok(tok);
                            // Dont pass initTok since it was already evaluated
                            if (updateLoop(end, endBlock, condTok, nullptr, stepTok) == Progress::Break)
                                return Break();
                        }
                        tok = endBlock;
                    } else {
                        // Traverse condition
                        if (updateRecursive(condTok) == Progress::Break)
                            return Break();
                        Branch thenBranch{endBlock};
                        Branch elseBranch{endBlock->tokAt(2) ? endBlock->linkAt(2) : nullptr};
                        // Check if condition is true or false
                        std::tie(thenBranch.check, elseBranch.check) = evalCond(condTok);
                        if (!thenBranch.check && !elseBranch.check && stopOnCondition(condTok) && stopUpdates())
                            return Break(Analyzer::Terminate::Conditional);
                        const bool hasElse = Token::simpleMatch(endBlock, "} else {");
                        tok = hasElse ? endBlock->linkAt(2) : endBlock;
                        if (thenBranch.check) {
                            // The condition is only "known" because of an earlier assumption, so the
                            // skipped else block could still modify the value -> lower to possible
                            if (!condTok->hasKnownIntValue() && hasElse &&
                                analyzeScope(elseBranch.endBlock).isModified() && !analyzer->lowerToPossible())
                                return Break(Analyzer::Terminate::Bail);
                            if (updateScope(thenBranch.endBlock, depth - 1) == Progress::Break)
                                return Break();
                        } else if (elseBranch.check) {
                            // Likewise the skipped then block could still modify the value
                            if (!condTok->hasKnownIntValue() && analyzeScope(thenBranch.endBlock).isModified() &&
                                !analyzer->lowerToPossible())
                                return Break(Analyzer::Terminate::Bail);
                            if (elseBranch.endBlock && updateScope(elseBranch.endBlock, depth - 1) == Progress::Break)
                                return Break();
                        } else {
                            const bool conditional = stopOnCondition(condTok);
                            // The value only flows into the then-branch when the condition can split
                            // it; for an opaque or correlated condition (e.g. 'if (f(x))') it does
                            // not, so fork in analyze-only mode: the branch's effect is still tracked
                            // but nothing is reported in it.
                            ForwardTraversal ft = fork(!analyzer->updateScope(thenBranch.endBlock, false));
                            // The branch is traversed below, so don't record its boundary state here.
                            ft.analyzer->assume(condTok, true, Analyzer::Assume::Pending);
                            Progress pThen = ft.updateBranch(thenBranch, depth - 1);
                            // Merge the fork's actions so a modification in the then-branch bubbles up
                            // to the enclosing branch's isModified().
                            actions |= thenBranch.action;

                            // Commit the condition as false on the main path only when the then-branch
                            // is dead. The else block, if any, is traversed separately (Pending); with
                            // no else the false path continues past the closing brace, so record the
                            // assumed state there (None).
                            if (thenBranch.isDead())
                                analyzer->assume(condTok,
                                                 false,
                                                 hasElse ? Analyzer::Assume::Pending : Analyzer::Assume::None);
                            // The else block is traversed on the main path. If it kills the value
                            // (modified) the main path stops, but the then-fork may still carry the
                            // value forward, so defer the break until after the fork continues.
                            Progress pElse = Progress::Continue;
                            if (hasElse)
                                pElse = updateBranch(elseBranch, depth - 1);
                            if (thenBranch.isDead() || elseBranch.isDead()) {
                                if (conditional && stopUpdates())
                                    pElse = Break(Analyzer::Terminate::Conditional);
                            }
                            if (thenBranch.isModified() || elseBranch.isModified()) {
                                if (!ft.analyzer->lowerToPossible())
                                    pThen = Progress::Break;
                                if (pElse != Progress::Break && !analyzer->lowerToPossible())
                                    pElse = Break(Analyzer::Terminate::Bail);
                            }
                            if (thenBranch.isInconclusive() || elseBranch.isInconclusive()) {
                                if (!ft.analyzer->lowerToInconclusive())
                                    pThen = Progress::Break;
                                if (pElse != Progress::Break && !analyzer->lowerToInconclusive())
                                    pElse = Break(Analyzer::Terminate::Bail);
                            }
                            if (thenBranch.hasGoto() || elseBranch.hasGoto()) {
                                return Break(Analyzer::Terminate::Bail);
                            }
                            // Carry the then-fork forward, unless a limit is hit - then only the linear main
                            // path continues (no bail). forkDepth bounds nesting, forkBudget total. <0 = off.
                            assert(forkBudget != nullptr);
                            const int forkDepthLimit = settings.vfOptions.maxForwardConditionForkDepth;
                            const int forkBudgetLimit = settings.vfOptions.maxForwardConditionForks;
                            const bool depthOk = forkDepthLimit < 0 || forkDepth < forkDepthLimit;
                            const bool budgetOk = forkBudgetLimit < 0 || *forkBudget < forkBudgetLimit;
                            if (pThen != Progress::Break && !thenBranch.isEscape() && depthOk && budgetOk) {
                                ++(*forkBudget);
                                ++ft.forkDepth;
                                ft.updateRange(thenBranch.endBlock, end, depth - 1);
                            }
                            if (pElse == Progress::Break)
                                return Break();
                        }
                    }
                } else if (Token::simpleMatch(tok, "try {")) {
                    Token* endBlock = tok->linkAt(1);
                    ForwardTraversal tryTraversal = fork();
                    tryTraversal.updateScope(endBlock, depth - 1);
                    bool bail = tryTraversal.actions.isModified();
                    if (bail) {
                        actions = tryTraversal.actions;
                        terminate = tryTraversal.terminate;
                        return Break();
                    }

                    while (Token::simpleMatch(endBlock, "} catch (")) {
                        Token* endCatch = endBlock->linkAt(2);
                        if (!Token::simpleMatch(endCatch, ") {"))
                            return Break();
                        endBlock = endCatch->linkAt(1);
                        ForwardTraversal ft = fork();
                        ft.updateScope(endBlock, depth - 1);
                        bail |= ft.terminate != Analyzer::Terminate::None || ft.actions.isModified();
                    }
                    if (bail)
                        return Break();
                    tok = endBlock;
                } else if (Token::simpleMatch(tok, "do {")) {
                    Token* endBlock = tok->linkAt(1);
                    Token* condTok = Token::simpleMatch(endBlock, "} while (") ? endBlock->tokAt(2)->astOperand2() : nullptr;
                    if (updateLoop(end, endBlock, condTok) == Progress::Break)
                        return Break();
                    if (condTok)
                        tok = endBlock->linkAt(2)->next();
                    else
                        tok = endBlock;
                } else if (Token::Match(tok, "assert|ASSERT (")) {
                    const Token* condTok = tok->next()->astOperand2();
                    bool checkThen, checkElse;
                    std::tie(checkThen, checkElse) = evalCond(condTok);
                    if (checkElse)
                        return Break();
                    if (!checkThen)
                        analyzer->assume(condTok, true, Analyzer::Assume::Quiet | Analyzer::Assume::Absolute);
                } else if (Token::simpleMatch(tok, "switch (")) {
                    if (updateRecursive(tok->next()->astOperand2()) == Progress::Break)
                        return Break();
                    actions |= Analyzer::Action::Write; // bailout for switch scope
                    return Break();
                } else if (Token* callTok = callExpr(tok)) {
                    // TODO: Dont traverse tokens a second time
                    if (start != callTok && tok != callTok && (tok->str() != "." || tok != callTok->astOperand1()) && updateRecursive(callTok->astOperand1()) == Progress::Break)
                        return Break();
                    // Since the call could be an unknown macro, traverse the tokens as a range instead of recursively
                    if (!Token::simpleMatch(callTok, "( )") &&
                        updateRange(callTok->next(), callTok->link(), depth - 1) == Progress::Break)
                        return Break();
                    if (updateTok(callTok) == Progress::Break)
                        return Break();
                    tok = callTok->link();
                    if (!tok)
                        return Break();
                } else {
                    if (updateTok(tok, &next) == Progress::Break)
                        return Break();
                    if (next) {
                        if (precedes(next, end))
                            tok = next->previous();
                        else
                            return Progress::Continue;
                    }
                }
                // Prevent infinite recursion
                if (tok->next() == start)
                    break;
            }
            return Progress::Continue;
        }

        void reportError(Severity severity, const std::string& id, const std::string& msg) {
            ErrorMessage::FileLocation loc(tokenList.getSourceFilePath(), 0, 0);
            const ErrorMessage errmsg({std::move(loc)}, tokenList.getSourceFilePath(), severity, msg, id, Certainty::normal);
            errorLogger.reportErr(errmsg);
        }

        static bool isFunctionCall(const Token* tok)
        {
            if (!Token::simpleMatch(tok, "("))
                return false;
            if (tok->isCast())
                return false;
            if (!tok->isBinaryOp())
                return false;
            if (Token::simpleMatch(tok->link(), ") {"))
                return false;
            if (isUnevaluated(tok->previous()))
                return false;
            return Token::Match(tok->previous(), "%name%|)|]|>");
        }

        static Token* assignExpr(Token* tok) {
            while (tok->astParent() && astIsLHS(tok)) {
                if (tok->astParent()->isAssignmentOp())
                    return tok->astParent();
                tok = tok->astParent();
            }
            return nullptr;
        }

        static Token* callExpr(Token* tok)
        {
            while (tok->astParent() && astIsLHS(tok)) {
                if (!Token::Match(tok, "%name%|::|<|."))
                    break;
                if (Token::simpleMatch(tok, "<") && !tok->link())
                    break;
                tok = tok->astParent();
            }
            if (isFunctionCall(tok))
                return tok;
            return nullptr;
        }

        static Token* skipTo(Token* tok, const Token* dest, const Token* end = nullptr) {
            if (end && dest->index() > end->index())
                return nullptr;
            const int i = dest->index() - tok->index();
            if (i > 0)
                return tok->tokAt(dest->index() - tok->index());
            return nullptr;
        }

        static Token* getStepTokFromEnd(Token* tok) {
            if (!Token::simpleMatch(tok, "}"))
                return nullptr;
            Token* end = tok->link()->previous();
            if (!Token::simpleMatch(end, ")"))
                return nullptr;
            return getStepTok(end->link());
        }
    };
}

Analyzer::Result valueFlowGenericForward(Token* start, const Token* end, const ValuePtr<Analyzer>& a, const TokenList& tokenList, ErrorLogger& errorLogger, const Settings& settings)
{
    if (a->invalid())
        return Analyzer::Result{Analyzer::Action::None, Analyzer::Terminate::Bail};
    ForwardTraversal ft{a, tokenList, errorLogger, settings};
    if (start)
        ft.analyzer->updateState(start);
    ft.updateRange(start, end);
    return Analyzer::Result{ ft.actions, ft.terminate };
}

Analyzer::Result valueFlowGenericForward(Token* start, const ValuePtr<Analyzer>& a, const TokenList& tokenList, ErrorLogger& errorLogger, const Settings& settings)
{
    if (Settings::terminated())
        throw TerminateException();
    if (a->invalid())
        return Analyzer::Result{Analyzer::Action::None, Analyzer::Terminate::Bail};
    ForwardTraversal ft{a, tokenList, errorLogger, settings};
    (void)ft.updateRecursive(start);
    return Analyzer::Result{ ft.actions, ft.terminate };
}
