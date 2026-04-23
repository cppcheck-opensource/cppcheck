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

#include "vf_analyzers.h"

#include "analyzer.h"
#include "astutils.h"
#include "calculate.h"
#include "config.h"
#include "library.h"
#include "mathlib.h"
#include "programmemory.h"
#include "smallvector.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "utils.h"
#include "vfvalue.h"
#include "valueptr.h"
#include "valueflow.h"

#include "vf_common.h"
#include "vf_settokenvalue.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <list>
#include <set>
#include <type_traits>

static bool isDereferenceOp(const Token* tok)
{
    if (!tok)
        return false;
    if (!tok->astOperand1())
        return false;
    if (tok->str() == "*")
        return true;
    return tok->str() == "." && tok->originalName() == "->";
}

struct ValueFlowAnalyzer : Analyzer {
    const Settings& settings;
    ProgramMemoryState pms;

    explicit ValueFlowAnalyzer(const Settings& s) : settings(s), pms(settings) {}

    virtual const ValueFlow::Value* getValue(const Token* tok) const = 0;
    virtual ValueFlow::Value* getValue(const Token* tok) = 0;

    virtual void makeConditional() = 0;

    virtual void addErrorPath(const Token* tok, const std::string& s) = 0;

    virtual bool match(const Token* tok) const = 0;

    virtual bool internalMatch(const Token* /*tok*/) const {
        return false;
    }

    virtual bool isAlias(const Token* tok, bool& inconclusive) const = 0;

    using ProgramState = ProgramMemory::Map;

    virtual ProgramState getProgramState() const = 0;

    virtual int getIndirect(const Token* tok) const {
        const ValueFlow::Value* value = getValue(tok);
        if (value)
            return value->indirect;
        return 0;
    }

    virtual bool isGlobal() const {
        return false;
    }
    virtual bool dependsOnThis() const {
        return false;
    }
    virtual bool isClassVariable() const {
        return false;
    }

    const Settings& getSettings() const {
        return settings;
    }

    struct ConditionState {
        bool dependent = true;
        bool unknown = true;

        bool isUnknownDependent() const {
            return unknown && dependent;
        }
    };

    ConditionState analyzeCondition(const Token* tok, int depth = 20) const
    {
        if (!tok)
            return {};
        if (depth < 0)
            return {};
        depth--;
        if (analyze(tok, Direction::Forward).isRead()) {
            ConditionState result;
            result.dependent = true;
            result.unknown = false;
            return result;
        }
        if (tok->hasKnownIntValue() || tok->isLiteral()) {
            ConditionState result;
            result.dependent = false;
            result.unknown = false;
            return result;
        }
        if (Token::Match(tok, "%cop%")) {
            if (isLikelyStream(tok->astOperand1())) {
                ConditionState result;
                result.dependent = false;
                return result;
            }
            ConditionState lhs = analyzeCondition(tok->astOperand1(), depth - 1);
            if (lhs.isUnknownDependent() || !tok->astOperand2())
                return lhs;
            ConditionState rhs = analyzeCondition(tok->astOperand2(), depth - 1);
            if (rhs.isUnknownDependent())
                return rhs;
            ConditionState result;
            if (Token::Match(tok, "%comp%"))
                result.dependent = lhs.dependent && rhs.dependent;
            else
                result.dependent = lhs.dependent || rhs.dependent;
            result.unknown = lhs.unknown || rhs.unknown;
            return result;
        }
        if (Token::Match(tok->previous(), "%name% (")) {
            std::vector<const Token*> args = getArguments(tok->previous());
            if (Token::Match(tok->tokAt(-2), ". %name% (")) {
                args.push_back(tok->tokAt(-2)->astOperand1());
            }
            ConditionState result;
            result.dependent = std::any_of(args.cbegin(), args.cend(), [&](const Token* arg) {
                ConditionState cs = analyzeCondition(arg, depth - 1);
                return cs.dependent;
            });
            if (result.dependent) {
                // Check if we can evaluate the function
                if (!evaluate(Evaluate::Integral, tok).empty())
                    result.unknown = false;
            }
            return result;
        }

        std::unordered_map<nonneg int, const Token*> symbols = getSymbols(tok);
        ConditionState result;
        result.dependent = false;
        for (auto&& p : symbols) {
            const Token* arg = p.second;
            ConditionState cs = analyzeCondition(arg, depth - 1);
            result.dependent = cs.dependent;
            if (result.dependent)
                break;
        }
        if (result.dependent) {
            // Check if we can evaluate the token
            if (!evaluate(Evaluate::Integral, tok).empty())
                result.unknown = false;
        }
        return result;
    }

    static ValueFlow::Value::MoveKind isMoveOrForward(const Token* tok)
    {
        if (!tok)
            return ValueFlow::Value::MoveKind::NonMovedVariable;
        const Token* parent = tok->astParent();
        if (!Token::simpleMatch(parent, "("))
            return ValueFlow::Value::MoveKind::NonMovedVariable;
        const Token* ftok = parent->astOperand1();
        if (!ftok)
            return ValueFlow::Value::MoveKind::NonMovedVariable;
        if (Token::simpleMatch(ftok->astOperand1(), "std :: move"))
            return ValueFlow::Value::MoveKind::MovedVariable;
        if (Token::simpleMatch(ftok->astOperand1(), "std :: forward"))
            return ValueFlow::Value::MoveKind::ForwardedVariable;
        // TODO: Check for cast
        return ValueFlow::Value::MoveKind::NonMovedVariable;
    }

    virtual Action isModified(const Token* tok) const {
        const ValueFlow::Value* value = getValue(tok);
        if (value) {
            // Moving a moved value won't change the moved value
            if (value->isMovedValue() && isMoveOrForward(tok) != ValueFlow::Value::MoveKind::NonMovedVariable)
                return Action::Read;
            // Inserting elements to container won't change the lifetime
            if (astIsContainer(tok) && value->isLifetimeValue() &&
                contains({Library::Container::Action::PUSH,
                          Library::Container::Action::INSERT,
                          Library::Container::Action::APPEND,
                          Library::Container::Action::CHANGE_INTERNAL},
                         astContainerAction(tok, getSettings().library)))
                return Action::Read;
        }
        bool inconclusive = false;
        if (isVariableChangedByFunctionCall(tok, getIndirect(tok), getSettings(), &inconclusive))
            return Action::Read | Action::Invalid;
        if (inconclusive)
            return Action::Read | Action::Inconclusive;
        if (isVariableChanged(tok, getIndirect(tok), getSettings())) {
            if (Token::Match(tok->astParent(), "*|[|.|++|--"))
                return Action::Read | Action::Invalid;
            // Check if its assigned to the same value
            if (value && !value->isImpossible() && Token::simpleMatch(tok->astParent(), "=") && astIsLHS(tok) &&
                astIsIntegral(tok->astParent()->astOperand2(), false)) {
                std::vector<MathLib::bigint> result = evaluateInt(tok->astParent()->astOperand2());
                if (!result.empty() && value->equalTo(result.front()))
                    return Action::Idempotent;
            }
            return Action::Invalid;
        }
        return Action::Read;
    }

    virtual Action isAliasModified(const Token* tok, int indirect = -1) const {
        // Lambda function call
        if (Token::Match(tok, "%var% ("))
            // TODO: Check if modified in the lambda function
            return Action::Invalid;
        if (indirect == -1) {
            indirect = 0;
            if (const ValueType* vt = tok->valueType()) {
                indirect = vt->pointer;
                if (vt->type == ValueType::ITERATOR)
                    ++indirect;
                const Token* tok2 = tok;
                while (Token::simpleMatch(tok2->astParent(), "[")) {
                    tok2 = tok2->astParent();
                    --indirect;
                }
                indirect = std::max(indirect, 0);
            }
        }
        for (int i = 0; i <= indirect; ++i)
            if (isVariableChanged(tok, i, getSettings()))
                return Action::Invalid;
        return Action::None;
    }

    virtual Action isThisModified(const Token* tok) const {
        if (isThisChanged(tok, 0, getSettings()))
            return Action::Invalid;
        return Action::None;
    }

    virtual Action isWritable(const Token* tok, Direction d) const {
        const ValueFlow::Value* value = getValue(tok);
        if (!value)
            return Action::None;
        if (!(value->isIntValue() || value->isFloatValue() || value->isSymbolicValue() || value->isLifetimeValue()))
            return Action::None;
        const Token* parent = tok->astParent();
        // Only if its invertible
        if (value->isImpossible() && !Token::Match(parent, "+=|-=|*=|++|--"))
            return Action::None;
        if (value->isLifetimeValue()) {
            if (value->lifetimeKind != ValueFlow::Value::LifetimeKind::Iterator)
                return Action::None;
            if (!Token::Match(parent, "++|--|+="))
                return Action::None;
            return Action::Read | Action::Write;
        }
        if (parent && parent->isAssignmentOp() && astIsLHS(tok)) {
            const Token* rhs = parent->astOperand2();
            std::vector<MathLib::bigint> result = evaluateInt(rhs);
            if (!result.empty()) {
                ValueFlow::Value rhsValue{result.front()};
                Action a;
                if (!evalAssignment(*value, getAssign(parent, d), rhsValue))
                    a = Action::Invalid;
                else
                    a = Action::Write;
                if (parent->str() != "=") {
                    a |= Action::Read | Action::Incremental;
                } else {
                    if (!value->isImpossible() && value->equalValue(rhsValue))
                        a = Action::Idempotent;
                    if (tok->exprId() != 0 &&
                        findAstNode(rhs, [&](const Token* child) {
                        return tok->exprId() == child->exprId();
                    }))
                        a |= Action::Incremental;
                }
                return a;
            }
        }

        // increment/decrement
        if (Token::Match(tok->astParent(), "++|--")) {
            return Action::Read | Action::Write | Action::Incremental;
        }
        return Action::None;
    }

    virtual void writeValue(ValueFlow::Value* value, const Token* tok, Direction d) const {
        if (!value)
            return;
        if (!tok->astParent())
            return;
        // Lifetime value doesn't change
        if (value->isLifetimeValue())
            return;
        if (tok->astParent()->isAssignmentOp()) {
            const Token* rhs = tok->astParent()->astOperand2();
            std::vector<MathLib::bigint> result = evaluateInt(rhs);
            assert(!result.empty());
            ValueFlow::Value rhsValue{result.front()};
            if (evalAssignment(*value, getAssign(tok->astParent(), d), rhsValue)) {
                std::string info("Compound assignment '" + tok->astParent()->str() + "', assigned value is " +
                                 value->infoString());
                if (tok->astParent()->str() == "=")
                    value->errorPath.clear();
                value->errorPath.emplace_back(tok, std::move(info));
            } else {
                assert(false && "Writable value cannot be evaluated");
                // TODO: Don't set to zero
                value->intvalue = 0;
            }
        } else if (tok->astParent()->tokType() == Token::eIncDecOp) {
            bool inc = tok->astParent()->str() == "++";
            const std::string opName(inc ? "incremented" : "decremented");
            if (d == Direction::Reverse)
                inc = !inc;
            value->intvalue += (inc ? 1 : -1);

            /* Truncate value */
            const ValueType *dst = tok->valueType();
            if (dst) {
                const size_t sz = dst->getSizeOf(settings, ValueType::Accuracy::ExactOrZero, ValueType::SizeOf::Pointer);
                if (sz > 0 && sz < sizeof(MathLib::biguint)) {
                    MathLib::bigint newvalue = ValueFlow::truncateIntValue(value->intvalue, sz, dst->sign);

                    /* Handle overflow/underflow for value bounds */
                    if (value->bound != ValueFlow::Value::Bound::Point) {
                        if ((newvalue > value->intvalue && !inc) || (newvalue < value->intvalue && inc))
                            value->invertBound();
                    }

                    value->intvalue = newvalue;
                }

                value->errorPath.emplace_back(tok, tok->str() + " is " + opName + "', new value is " + value->infoString());
            }
        }
    }

    virtual bool useSymbolicValues() const {
        return true;
    }

    virtual void internalUpdate(Token* /*tok*/, const ValueFlow::Value& /*v*/, Direction /*d*/)
    {
        assert(false && "Internal update unimplemented.");
    }

private:
    // Returns Action::Match if its an exact match, return Action::Read if it partially matches the lifetime
    Action analyzeLifetime(const Token* tok) const
    {
        if (!tok)
            return Action::None;
        if (match(tok))
            return Action::Match;
        if (Token::simpleMatch(tok, ".") && analyzeLifetime(tok->astOperand1()) != Action::None)
            return Action::Read;
        if (astIsRHS(tok) && Token::simpleMatch(tok->astParent(), "."))
            return analyzeLifetime(tok->astParent());
        return Action::None;
    }

    std::unordered_map<nonneg int, const Token*> getSymbols(const Token* tok) const
    {
        std::unordered_map<nonneg int, const Token*> result;
        if (!tok)
            return result;
        for (const ValueFlow::Value& v : tok->values()) {
            if (!v.isSymbolicValue())
                continue;
            if (v.isImpossible())
                continue;
            if (!v.tokvalue)
                continue;
            if (v.tokvalue->exprId() == 0)
                continue;
            if (match(v.tokvalue))
                continue;
            result[v.tokvalue->exprId()] = v.tokvalue;
        }
        return result;
    }

    Action isGlobalModified(const Token* tok) const
    {
        if (tok->function()) {
            if (!tok->function()->isConstexpr() && !isConstFunctionCall(tok, getSettings().library))
                return Action::Invalid;
        } else if (getSettings().library.getFunction(tok)) {
            // Assume library function doesn't modify user-global variables
            return Action::None;
        } else if (Token::simpleMatch(tok->astParent(), ".") && astIsContainer(tok->astParent()->astOperand1())) {
            // Assume container member function doesn't modify user-global variables
            return Action::None;
        } else if (tok->tokType() == Token::eType && astIsPrimitive(tok->next())) {
            // Function cast does not modify global variables
            return Action::None;
        } else if (!tok->isKeyword() && Token::Match(tok, "%name% (")) {
            return Action::Invalid;
        }
        return Action::None;
    }

    static const std::string& invertAssign(const std::string& assign)
    {
        static std::unordered_map<std::string, std::string> lookup = {{"=", "="},
            {"+=", "-="},
            {"-=", "+="},
            {"*=", "/="},
            {"/=", "*="},
            {"<<=", ">>="},
            {">>=", "<<="},
            {"^=", "^="}};
        auto it = lookup.find(assign);
        if (it == lookup.end()) {
            static const std::string s_empty_string;
            return s_empty_string;
        }
        return it->second;
    }

    static const std::string& getAssign(const Token* tok, Direction d)
    {
        if (d == Direction::Forward)
            return tok->str();
        return invertAssign(tok->str());
    }

    template<class T, class U>
    static void assignValueIfMutable(T& x, const U& y)
    {
        x = y;
    }

    template<class T, class U>
    static void assignValueIfMutable(const T& /*unused*/, const U& /*unused*/)
    {}

    static std::string removeAssign(const std::string& assign) {
        return std::string{assign.cbegin(), assign.cend() - 1};
    }

    template<class T, class U>
    static T calculateAssign(const std::string& assign, const T& x, const U& y, bool* error = nullptr)
    {
        if (assign.empty() || assign.back() != '=') {
            if (error)
                *error = true;
            return T{};
        }
        if (assign == "=")
            return static_cast<T>(y);
        return calculate<T, T>(removeAssign(assign), x, y, error);
    }

    template<class Value, REQUIRES("Value must ValueFlow::Value", std::is_convertible<Value&, const ValueFlow::Value&> )>
    static bool evalAssignment(Value& lhsValue, const std::string& assign, const ValueFlow::Value& rhsValue)
    {
        bool error = false;
        if (lhsValue.isSymbolicValue() && rhsValue.isIntValue()) {
            if (assign != "+=" && assign != "-=")
                return false;
            assignValueIfMutable(lhsValue.intvalue, calculateAssign(assign, lhsValue.intvalue, rhsValue.intvalue, &error));
        } else if (lhsValue.isIntValue() && rhsValue.isIntValue()) {
            assignValueIfMutable(lhsValue.intvalue, calculateAssign(assign, lhsValue.intvalue, rhsValue.intvalue, &error));
        } else if (lhsValue.isFloatValue() && rhsValue.isIntValue()) {
            assignValueIfMutable(lhsValue.floatValue,
                                 calculateAssign(assign, lhsValue.floatValue, rhsValue.intvalue, &error));
        } else {
            return false;
        }
        return !error;
    }

    const Token* findMatch(const Token* tok) const
    {
        return findAstNode(tok, [&](const Token* child) {
            return match(child);
        });
    }

    bool isSameSymbolicValue(const Token* tok, ValueFlow::Value* value = nullptr) const
    {
        if (!useSymbolicValues())
            return false;
        if (Token::Match(tok, "%assign%"))
            return false;
        const ValueFlow::Value* currValue = getValue(tok);
        if (!currValue)
            return false;
        // If the same symbolic value is already there then skip
        if (currValue->isSymbolicValue() &&
            std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
            return v.isSymbolicValue() && currValue->equalValue(v);
        }))
            return false;
        const bool isPoint = currValue->bound == ValueFlow::Value::Bound::Point && currValue->isIntValue();
        const bool exact = !currValue->isIntValue() || currValue->isImpossible();
        for (const ValueFlow::Value& v : tok->values()) {
            if (!v.isSymbolicValue())
                continue;
            if (currValue->equalValue(v))
                continue;
            const bool toImpossible = v.isImpossible() && currValue->isKnown();
            if (!v.isKnown() && !toImpossible)
                continue;
            if (exact && v.intvalue != 0 && !isPoint)
                continue;
            if (astIsUnsigned(tok) != astIsUnsigned(v.tokvalue))
                continue;
            std::vector<MathLib::bigint> r;
            ValueFlow::Value::Bound bound = currValue->bound;
            if (match(v.tokvalue)) {
                r = {currValue->intvalue};
            } else if (!exact && findMatch(v.tokvalue)) {
                r = evaluate(Evaluate::Integral, v.tokvalue, tok);
                if (bound == ValueFlow::Value::Bound::Point)
                    bound = v.bound;
            }
            if (!r.empty()) {
                if (value) {
                    value->errorPath.insert(value->errorPath.end(), v.errorPath.cbegin(), v.errorPath.cend());
                    value->intvalue = r.front() + v.intvalue;
                    if (toImpossible)
                        value->setImpossible();
                    value->bound = bound;
                }
                return true;
            }
        }
        return false;
    }

    Action analyzeMatch(const Token* tok, Direction d) const {
        const Token* parent = tok->astParent();
        if (d == Direction::Reverse && isGlobal() && !dependsOnThis() && Token::Match(parent, ". %name% (")) {
            Action a = isGlobalModified(parent->next());
            if (a != Action::None)
                return a;
        }
        if ((astIsPointer(tok) || astIsSmartPointer(tok)) &&
            (Token::Match(parent, "*|[") || (parent && parent->originalName() == "->")) && getIndirect(tok) <= 0)
            return Action::Read;

        Action w = isWritable(tok, d);
        if (w != Action::None)
            return w;

        // Check for modifications by function calls
        return isModified(tok);
    }

    Action analyzeToken(const Token* ref, const Token* tok, Direction d, bool inconclusiveRef) const {
        if (!ref)
            return Action::None;
        // If its an inconclusiveRef then ref != tok
        assert(!inconclusiveRef || ref != tok);
        bool inconclusive = false;
        if (match(ref)) {
            if (inconclusiveRef) {
                Action a = isModified(tok);
                if (a.isModified() || a.isInconclusive())
                    return Action::Inconclusive;
            } else {
                return analyzeMatch(tok, d) | Action::Match;
            }
        } else if (isDereferenceOp(ref) && !match(ref->astOperand1())) {
            const Token* lifeTok = nullptr;
            for (const ValueFlow::Value& v:ref->astOperand1()->values()) {
                if (!v.isLocalLifetimeValue())
                    continue;
                if (v.conditional)
                    continue;
                if (lifeTok)
                    return Action::None;
                lifeTok = v.tokvalue;
            }
            if (!lifeTok)
                return Action::None;
            Action la = analyzeLifetime(lifeTok);
            if (la.matches()) {
                Action a = Action::Read;
                if (isModified(tok).isModified())
                    a = Action::Invalid;
                if (Token::Match(tok->astParent(), "%assign%") && astIsLHS(tok))
                    a |= Action::Invalid;
                if (inconclusiveRef && a.isModified())
                    return Action::Inconclusive;
                return a;
            }
            if (la.isRead()) {
                return isAliasModified(tok);
            }
            return Action::None;

        } else if (isAlias(ref, inconclusive)) {
            inconclusive |= inconclusiveRef;
            Action a = isAliasModified(tok);
            if (inconclusive && a.isModified())
                return Action::Inconclusive;
            return a;
        }
        if (isSameSymbolicValue(ref))
            return Action::Read | Action::SymbolicMatch;

        return Action::None;
    }

    Action analyze(const Token* tok, Direction d) const override {
        if (invalid())
            return Action::Invalid;
        // Follow references
        // TODO: avoid copy
        auto refs = tok->refs();
        const bool inconclusiveRefs = refs.size() != 1;
        if (std::none_of(refs.cbegin(), refs.cend(), [&](const ReferenceToken& ref) {
            return tok == ref.token;
        }))
            refs.emplace_back(ReferenceToken{tok, {}});
        for (const ReferenceToken& ref:refs) {
            Action a = analyzeToken(ref.token, tok, d, inconclusiveRefs && ref.token != tok);
            if (internalMatch(ref.token))
                a |= Action::Internal;
            if (a != Action::None)
                return a;
        }
        if (dependsOnThis() && exprDependsOnThis(tok, !isClassVariable()))
            return isThisModified(tok);

        // bailout: global non-const variables
        if (isGlobal() && !dependsOnThis() && Token::Match(tok, "%name% (") && !tok->variable() &&
            !Token::simpleMatch(tok->linkAt(1), ") {")) {
            return isGlobalModified(tok);
        }
        return Action::None;
    }

    template<class F>
    std::vector<MathLib::bigint> evaluateInt(const Token* tok, F getProgramMemory) const
    {
        if (const ValueFlow::Value* v = tok->getKnownValue(ValueFlow::Value::ValueType::INT))
            return {v->intvalue};
        std::vector<MathLib::bigint> result;
        ProgramMemory pm = getProgramMemory();
        if (Token::Match(tok, "&&|%oror%")) {
            if (conditionIsTrue(tok, pm, getSettings()))
                result.push_back(1);
            if (conditionIsFalse(tok, std::move(pm), getSettings()))
                result.push_back(0);
        } else {
            MathLib::bigint out = 0;
            bool error = false;
            execute(tok, pm, &out, &error, getSettings());
            if (!error)
                result.push_back(out);
        }
        return result;
    }

    std::vector<MathLib::bigint> evaluateInt(const Token* tok) const
    {
        return evaluateInt(tok, [&] {
            return ProgramMemory{getProgramState()};
        });
    }

    std::vector<MathLib::bigint> evaluate(Evaluate e, const Token* tok, const Token* ctx = nullptr) const override
    {
        if (e == Evaluate::Integral) {
            return evaluateInt(tok, [&] {
                return pms.get(tok, ctx, getProgramState());
            });
        }
        if (e == Evaluate::ContainerEmpty) {
            const ValueFlow::Value* value = ValueFlow::findValue(tok->values(), settings, [](const ValueFlow::Value& v) {
                return v.isKnown() && v.isContainerSizeValue();
            });
            if (value)
                return {value->intvalue == 0};
            ProgramMemory pm = pms.get(tok, ctx, getProgramState());
            MathLib::bigint out = 0;
            if (pm.getContainerEmptyValue(tok->exprId(), out))
                return {out};
            return {};
        }
        return {};
    }

    void assume(const Token* tok, bool state, unsigned int flags) override {
        // Update program state
        pms.removeModifiedVars(tok);
        pms.addState(tok, getProgramState());
        pms.assume(tok, state, flags & Assume::ContainerEmpty);

        bool isCondBlock = false;
        const Token* parent = tok->astParent();
        if (parent) {
            isCondBlock = Token::Match(parent->previous(), "if|while (");
        }

        if (isCondBlock) {
            const Token* startBlock = parent->link()->next();
            if (Token::simpleMatch(startBlock, ";") && Token::simpleMatch(parent->tokAt(-2), "} while ("))
                startBlock = parent->linkAt(-2);
            const Token* endBlock = startBlock->link();
            if (state) {
                pms.removeModifiedVars(endBlock);
                pms.addState(endBlock->previous(), getProgramState());
            } else {
                if (Token::simpleMatch(endBlock, "} else {"))
                    pms.addState(endBlock->linkAt(2)->previous(), getProgramState());
            }
        }

        if (!(flags & Assume::Quiet)) {
            if (flags & Assume::ContainerEmpty) {
                std::string s = state ? "empty" : "not empty";
                addErrorPath(tok, "Assuming container is " + s);
            } else {
                std::string s = bool_to_string(state);
                addErrorPath(tok, "Assuming condition is " + s);
            }
        }
        if (!(flags & Assume::Absolute))
            makeConditional();
    }

    void updateState(const Token* tok) override
    {
        // Update program state
        pms.removeModifiedVars(tok);
        pms.addState(tok, getProgramState());
    }

    void update(Token* tok, Action a, Direction d) override {
        ValueFlow::Value* value = getValue(tok);
        if (!value)
            return;
        ValueFlow::Value localValue;
        if (a.isSymbolicMatch()) {
            // Make a copy of the value to modify it
            localValue = *value;
            value = &localValue;
            isSameSymbolicValue(tok, &localValue);
        }
        if (a.isInternal())
            internalUpdate(tok, *value, d);
        // Read first when moving forward
        if (d == Direction::Forward && a.isRead())
            setTokenValue(tok, *value, getSettings());
        if (a.isInconclusive())
            (void)lowerToInconclusive();
        if (a.isWrite() && tok->astParent()) {
            writeValue(value, tok, d);
        }
        // Read last when moving in reverse
        if (d == Direction::Reverse && a.isRead())
            setTokenValue(tok, *value, getSettings());
    }

    ValuePtr<Analyzer> reanalyze(Token* /*tok*/, const std::string& /*msg*/) const override {
        return {};
    }
};

static bool bifurcate(const Token* tok, const std::set<nonneg int>& varids, const Settings& settings, int depth = 20);

static bool bifurcateVariableChanged(const Variable* var,
                                     const std::set<nonneg int>& varids,
                                     const Token* start,
                                     const Token* end,
                                     const Settings& settings,
                                     int depth = 20)
{
    bool result = false;
    const Token* tok = start;
    while ((tok = findVariableChanged(
                tok->next(), end, var->isPointer(), var->declarationId(), var->isGlobal(), settings))) {
        if (Token::Match(tok->astParent(), "%assign%")) {
            if (!bifurcate(tok->astParent()->astOperand2(), varids, settings, depth - 1))
                return true;
        } else {
            result = true;
        }
    }
    return result;
}

static bool bifurcate(const Token* tok, const std::set<nonneg int>& varids, const Settings& settings, int depth)
{
    if (depth < 0)
        return false;
    if (!tok)
        return true;
    if (tok->hasKnownIntValue())
        return true;
    if (tok->isConstOp())
        return bifurcate(tok->astOperand1(), varids, settings, depth) && bifurcate(tok->astOperand2(), varids, settings, depth);
    if (tok->varId() != 0) {
        if (varids.count(tok->varId()) > 0)
            return true;
        const Variable* var = tok->variable();
        if (!var)
            return false;
        if (!var->isLocal() && !var->isArgument())
            return false;
        const Token* start = var->declEndToken();
        if (!start)
            return false;
        if (start->strAt(-1) == ")" || start->strAt(-1) == "}")
            return false;
        if (Token::Match(start, "; %varid% =", var->declarationId()))
            start = start->tokAt(2);
        if (var->isConst() || !bifurcateVariableChanged(var, varids, start, tok, settings, depth))
            return var->isArgument() || bifurcate(start->astOperand2(), varids, settings, depth - 1);
        return false;
    }
    return false;
}

// Check if its an alias of the variable or is being aliased to this variable
template<typename V>
static bool isAliasOf(const Variable * var, const Token *tok, nonneg int varid, const V& values, bool* inconclusive = nullptr)
{
    if (tok->varId() == varid)
        return false;
    if (tok->varId() == 0)
        return false;
    if (isAliasOf(tok, varid, inconclusive))
        return true;
    if (var && !var->isPointer())
        return false;
    // Search through non value aliases
    return std::any_of(values.begin(), values.end(), [&](const ValueFlow::Value& val) {
        if (!val.isNonValue())
            return false;
        if (val.isInconclusive())
            return false;
        if (val.isLifetimeValue() && !val.isLocalLifetimeValue())
            return false;
        if (val.isLifetimeValue() && val.lifetimeKind != ValueFlow::Value::LifetimeKind::Address)
            return false;
        if (!Token::Match(val.tokvalue, ".|&|*|%var%"))
            return false;
        return astHasVar(val.tokvalue, tok->varId());
    });
}

struct MultiValueFlowAnalyzer : ValueFlowAnalyzer {
    class SelectValueFromVarIdMapRange {
        using M = std::unordered_map<nonneg int, ValueFlow::Value>;

        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            using value_type = const ValueFlow::Value;
            using pointer = value_type *;
            using reference = value_type &;
            using difference_type = std::ptrdiff_t;

            explicit Iterator(const M::const_iterator & it)
                : mIt(it) {}

            reference operator*() const {
                return mIt->second;
            }

            pointer operator->() const {
                return &mIt->second;
            }

            Iterator &operator++() {
                // cppcheck-suppress postfixOperator - forward iterator needs to perform post-increment
                mIt++;
                return *this;
            }

            friend bool operator==(const Iterator &a, const Iterator &b) {
                return a.mIt == b.mIt;
            }

            friend bool operator!=(const Iterator &a, const Iterator &b) {
                return a.mIt != b.mIt;
            }

        private:
            M::const_iterator mIt;
        };

    public:
        explicit SelectValueFromVarIdMapRange(const M *m)
            : mMap(m) {}

        Iterator begin() const {
            return Iterator(mMap->begin());
        }
        Iterator end() const {
            return Iterator(mMap->end());
        }

    private:
        const M *mMap;
    };

    std::unordered_map<nonneg int, ValueFlow::Value> values;
    std::unordered_map<nonneg int, const Variable*> vars;

    MultiValueFlowAnalyzer(const std::unordered_map<const Variable*, ValueFlow::Value>& args, const Settings& set)
        : ValueFlowAnalyzer(set) {
        for (const auto& p:args) {
            const auto declId = p.first->declarationId();
            if (declId == 0)
                continue; // TODO: should never happen?
            values[declId] = p.second;
            vars[declId] = p.first;
        }
    }

    virtual const std::unordered_map<nonneg int, const Variable*>& getVars() const {
        return vars;
    }

    const ValueFlow::Value* getValue(const Token* tok) const override {
        if (tok->varId() == 0)
            return nullptr;
        auto it = values.find(tok->varId());
        if (it == values.end())
            return nullptr;
        return &it->second;
    }
    ValueFlow::Value* getValue(const Token* tok) override {
        if (tok->varId() == 0)
            return nullptr;
        auto it = values.find(tok->varId());
        if (it == values.end())
            return nullptr;
        return &it->second;
    }

    void makeConditional() override {
        for (auto&& p:values) {
            p.second.conditional = true;
        }
    }

    void addErrorPath(const Token* tok, const std::string& s) override {
        for (auto&& p:values) {
            p.second.errorPath.emplace_back(tok, s);
        }
    }

    bool isAlias(const Token* tok, bool& inconclusive) const override {
        const auto range = SelectValueFromVarIdMapRange(&values);

        for (const auto& p:getVars()) {
            nonneg int const varid = p.first;
            const Variable* var = p.second;
            if (tok->varId() == varid)
                return true;
            if (isAliasOf(var, tok, varid, range, &inconclusive))
                return true;
        }
        return false;
    }

    bool lowerToPossible() override {
        for (auto&& p:values) {
            if (p.second.isImpossible())
                return false;
            p.second.changeKnownToPossible();
        }
        return true;
    }
    bool lowerToInconclusive() override {
        for (auto&& p:values) {
            if (p.second.isImpossible())
                return false;
            p.second.setInconclusive();
        }
        return true;
    }

    bool isConditional() const override {
        for (auto&& p:values) {
            if (p.second.conditional)
                return true;
            if (p.second.condition)
                return !p.second.isImpossible();
        }
        return false;
    }

    bool stopOnCondition(const Token* condTok) const override {
        if (isConditional())
            return true;
        if (!condTok->hasKnownIntValue() && values.count(condTok->varId()) == 0) {
            const auto& values_ = condTok->values();
            return std::any_of(values_.cbegin(), values_.cend(), [](const ValueFlow::Value& v) {
                return v.isSymbolicValue() && Token::Match(v.tokvalue, "%oror%|&&");
            });
        }
        return false;
    }

    bool updateScope(const Token* endBlock, bool /*modified*/) const override {
        const Scope* scope = endBlock->scope();
        if (!scope)
            return false;
        if (scope->type == ScopeType::eLambda) {
            return std::all_of(values.cbegin(), values.cend(), [](const std::pair<nonneg int, ValueFlow::Value>& p) {
                return p.second.isLifetimeValue();
            });
        }
        if (scope->type == ScopeType::eIf || scope->type == ScopeType::eElse || scope->type == ScopeType::eWhile ||
            scope->type == ScopeType::eFor) {
            auto pred = [](const ValueFlow::Value& value) {
                if (value.isKnown())
                    return true;
                if (value.isImpossible())
                    return true;
                if (value.isLifetimeValue())
                    return true;
                return false;
            };
            if (std::all_of(values.cbegin(), values.cend(), std::bind(pred, std::bind(SelectMapValues{}, std::placeholders::_1))))
                return true;
            if (isConditional())
                return false;
            const Token* condTok = getCondTokFromEnd(endBlock);
            std::set<nonneg int> varids;
            std::transform(getVars().cbegin(), getVars().cend(), std::inserter(varids, varids.begin()), SelectMapKeys{});
            return bifurcate(condTok, varids, getSettings());
        }

        return false;
    }

    bool match(const Token* tok) const override {
        return values.count(tok->varId()) > 0;
    }

    ProgramState getProgramState() const override {
        ProgramState ps;
        for (const auto& p : values) {
            const Variable* var = vars.at(p.first);
            if (!var)
                continue;
            assert(var->nameToken());
            ps[var->nameToken()] = p.second;
        }
        return ps;
    }
};

ValuePtr<Analyzer> makeMultiValueFlowAnalyzer(const std::unordered_map<const Variable*, ValueFlow::Value>& args, const Settings& settings)
{
    return MultiValueFlowAnalyzer{args, settings};
}

struct SingleValueFlowAnalyzer : ValueFlowAnalyzer {
    std::unordered_map<nonneg int, const Variable*> varids;
    std::unordered_map<nonneg int, const Variable*> aliases;
    ValueFlow::Value value;

    SingleValueFlowAnalyzer(ValueFlow::Value v, const Settings& s) : ValueFlowAnalyzer(s), value(std::move(v)) {}

    const std::unordered_map<nonneg int, const Variable*>& getVars() const {
        return varids;
    }

    const std::unordered_map<nonneg int, const Variable*>& getAliasedVars() const {
        return aliases;
    }

    const ValueFlow::Value* getValue(const Token* /*tok*/) const override {
        return &value;
    }
    ValueFlow::Value* getValue(const Token* /*tok*/) override {
        return &value;
    }

    void makeConditional() override {
        value.conditional = true;
    }

    bool useSymbolicValues() const override
    {
        if (value.isUninitValue())
            return false;
        if (value.isLifetimeValue())
            return false;
        return true;
    }

    void addErrorPath(const Token* tok, const std::string& s) override {
        value.errorPath.emplace_back(tok, s);
    }

    template<class T>
    struct SingleRange {
        T* x;
        T* begin() const {
            return x;
        }
        T* end() const {
            return x+1;
        }
    };

    template<class T>
    static SingleRange<T> MakeSingleRange(T& x)
    {
        return {&x};
    }

    bool isAlias(const Token* tok, bool& inconclusive) const override {
        if (value.isLifetimeValue())
            return false;
        for (const auto& m: {
            std::ref(getVars()), std::ref(getAliasedVars())
        }) {
            for (const auto& p:m.get()) {
                nonneg int const varid = p.first;
                const Variable* var = p.second;
                if (tok->varId() == varid)
                    return true;
                if (isAliasOf(var, tok, varid, MakeSingleRange(value), &inconclusive))
                    return true;
            }
        }
        return false;
    }

    bool isGlobal() const override {
        const auto& vars = getVars();
        return std::any_of(vars.cbegin(), vars.cend(), [] (const std::pair<nonneg int, const Variable*>& p) {
            const Variable* var = p.second;
            return !var->isLocal() && !var->isArgument() && !var->isConst();
        });
    }

    bool lowerToPossible() override {
        if (value.isImpossible())
            return false;
        value.changeKnownToPossible();
        return true;
    }
    bool lowerToInconclusive() override {
        if (value.isImpossible())
            return false;
        value.setInconclusive();
        return true;
    }

    bool isConditional() const override {
        if (value.conditional)
            return true;
        if (value.condition)
            return !value.isKnown() && !value.isImpossible();
        return false;
    }

    bool stopOnCondition(const Token* condTok) const override
    {
        if (value.isNonValue())
            return false;
        if (value.isImpossible())
            return false;
        if (isConditional() && !value.isKnown())
            return true;
        if (value.isSymbolicValue())
            return false;
        ConditionState cs = analyzeCondition(condTok);
        return cs.isUnknownDependent();
    }

    bool updateScope(const Token* endBlock, bool /*modified*/) const override {
        const Scope* scope = endBlock->scope();
        if (!scope)
            return false;
        if (scope->type == ScopeType::eLambda)
            return value.isLifetimeValue();
        if (scope->type == ScopeType::eIf || scope->type == ScopeType::eElse || scope->type == ScopeType::eWhile ||
            scope->type == ScopeType::eFor) {
            if (value.isKnown() || value.isImpossible())
                return true;
            if (value.isLifetimeValue())
                return true;
            if (isConditional())
                return false;
            const Token* condTok = getCondTokFromEnd(endBlock);
            std::set<nonneg int> varids2;
            std::transform(getVars().cbegin(), getVars().cend(), std::inserter(varids2, varids2.begin()), SelectMapKeys{});
            return bifurcate(condTok, varids2, getSettings());
        }

        return false;
    }

private:
    ValuePtr<Analyzer> reanalyze(Token* tok, const std::string& msg) const override {
        ValueFlow::Value newValue = value;
        newValue.errorPath.emplace_back(tok, msg);
        return makeAnalyzer(tok, std::move(newValue), settings);
    }
};

struct ExpressionAnalyzer : SingleValueFlowAnalyzer {
    const Token* expr;
    bool local = true;
    bool unknown{};
    bool dependOnThis{};
    bool uniqueExprId{};

    ExpressionAnalyzer(const Token* e, ValueFlow::Value val, const Settings& s)
        : SingleValueFlowAnalyzer(std::move(val), s),
        expr(e)
    {

        assert(e && e->exprId() != 0 && "Not a valid expression");
        dependOnThis = exprDependsOnThis(expr);
        setupExprVarIds(expr);
        if (value.isSymbolicValue()) {
            dependOnThis |= exprDependsOnThis(value.tokvalue);
            setupExprVarIds(value.tokvalue);
        }
        uniqueExprId =
            expr->isUniqueExprId() && (Token::Match(expr, "%cop%") || !isVariableChanged(expr, 0, s));
    }

    static bool nonLocal(const Variable* var, bool deref) {
        return !var || (!var->isLocal() && !var->isArgument()) || (deref && var->isArgument() && var->isPointer()) ||
               var->isStatic() || var->isReference() || var->isExtern();
    }

    void setupExprVarIds(const Token* start, int depth = 0) {
        if (depth > settings.vfOptions.maxExprVarIdDepth) {
            // TODO: add bailout message
            return;
        }
        visitAstNodes(start, [&](const Token* tok) {
            const bool top = depth == 0 && tok == start;
            const bool ispointer = astIsPointer(tok) || astIsSmartPointer(tok) || astIsIterator(tok);
            if (!top || !ispointer || value.indirect != 0) {
                for (const ValueFlow::Value& v : tok->values()) {
                    if (!(v.isLocalLifetimeValue() || (ispointer && v.isSymbolicValue() && v.isKnown())))
                        continue;
                    if (!v.tokvalue)
                        continue;
                    if (v.tokvalue == tok)
                        continue;
                    setupExprVarIds(v.tokvalue, depth + 1);
                }
            }
            if (depth == 0 && tok->isIncompleteVar()) {
                // TODO: Treat incomplete var as global, but we need to update
                // the alias variables to just expr ids instead of requiring
                // Variable
                unknown = true;
                return ChildrenToVisit::none;
            }
            if (tok->varId() > 0) {
                varids[tok->varId()] = tok->variable();
                if (!Token::simpleMatch(tok->previous(), ".")) {
                    const Variable* var = tok->variable();
                    if (var && var->isReference() && var->isLocal() && Token::Match(var->nameToken(), "%var% [=(]") &&
                        !isGlobalData(var->nameToken()->next()->astOperand2()))
                        return ChildrenToVisit::none;
                    const bool deref = tok->astParent() &&
                                       (tok->astParent()->isUnaryOp("*") ||
                                        (tok->astParent()->str() == "[" && tok == tok->astParent()->astOperand1()));
                    local &= !nonLocal(tok->variable(), deref);
                }
            }
            return ChildrenToVisit::op1_and_op2;
        });
    }

    virtual bool skipUniqueExprIds() const {
        return true;
    }

    bool invalid() const override {
        if (skipUniqueExprIds() && uniqueExprId)
            return true;
        return unknown;
    }

    ProgramState getProgramState() const override {
        ProgramState ps;
        ps[expr] = value;
        return ps;
    }

    bool match(const Token* tok) const override {
        return tok->exprId() == expr->exprId();
    }

    bool dependsOnThis() const override {
        return dependOnThis;
    }

    bool isGlobal() const override {
        return !local;
    }

    bool isClassVariable() const override
    {
        if (expr->variable()) {
            const Variable* var = expr->variable();
            return !var->isLocal() && !var->isArgument() && !var->isStatic() && !var->isGlobal();
        }
        return expr->varId() > 0;
    }

    Action isAliasModified(const Token* tok, int indirect) const override {
        if (value.isSymbolicValue() && tok->exprId() == value.tokvalue->exprId())
            indirect = 0;
        return SingleValueFlowAnalyzer::isAliasModified(tok, indirect);
    }
};

struct SameExpressionAnalyzer : ExpressionAnalyzer {
    SameExpressionAnalyzer(const Token* e, ValueFlow::Value val, const Settings& s)
        : ExpressionAnalyzer(e, std::move(val), s)
    {}

    bool skipUniqueExprIds() const override {
        return false;
    }

    bool match(const Token* tok) const override
    {
        return isSameExpression(true, expr, tok, getSettings(), true, true);
    }
};

ValuePtr<Analyzer> makeSameExpressionAnalyzer(const Token* e, ValueFlow::Value val, const Settings& s)
{
    return SameExpressionAnalyzer{e, std::move(val), s};
}

struct OppositeExpressionAnalyzer : ExpressionAnalyzer {
    bool isNot{};

    OppositeExpressionAnalyzer(bool pIsNot, const Token* e, ValueFlow::Value val, const Settings& s)
        : ExpressionAnalyzer(e, std::move(val), s), isNot(pIsNot)
    {}

    bool skipUniqueExprIds() const override {
        return false;
    }

    bool match(const Token* tok) const override {
        return isOppositeCond(isNot, expr, tok, getSettings(), true, true);
    }
};

ValuePtr<Analyzer> makeOppositeExpressionAnalyzer(bool pIsNot, const Token* e, ValueFlow::Value val, const Settings& s)
{
    return OppositeExpressionAnalyzer{pIsNot, e, std::move(val), s};
}

struct SubExpressionAnalyzer : ExpressionAnalyzer {
    using PartialReadContainer = std::vector<std::pair<Token *, ValueFlow::Value>>;
    // A shared_ptr is used so partial reads can be captured even after forking
    std::shared_ptr<PartialReadContainer> partialReads;

    SubExpressionAnalyzer(const Token* e, ValueFlow::Value val, const Settings& s)
        : ExpressionAnalyzer(e, std::move(val), s), partialReads(std::make_shared<PartialReadContainer>())
    {}

    SubExpressionAnalyzer(const Token* e, ValueFlow::Value val, const std::shared_ptr<PartialReadContainer>& p, const Settings& s)
        : ExpressionAnalyzer(e, std::move(val), s), partialReads(p)
    {}

    virtual bool submatch(const Token* tok, bool exact = true) const = 0;

    bool isAlias(const Token* tok, bool& inconclusive) const override
    {
        if (tok->exprId() == expr->exprId() && tok->astParent() && submatch(tok->astParent(), false))
            return false;
        return ExpressionAnalyzer::isAlias(tok, inconclusive);
    }

    bool match(const Token* tok) const override
    {
        return tok->astOperand1() && tok->astOperand1()->exprId() == expr->exprId() && submatch(tok);
    }
    bool internalMatch(const Token* tok) const override
    {
        return tok->exprId() == expr->exprId() && !(astIsLHS(tok) && submatch(tok->astParent(), false));
    }
    void internalUpdate(Token* tok, const ValueFlow::Value& v, Direction /*d*/) override
    {
        partialReads->emplace_back(tok, v);
    }

private:
    // No reanalysis for subexpression
    ValuePtr<Analyzer> reanalyze(Token* /*tok*/, const std::string& /*msg*/) const override {
        return {};
    }
};

struct MemberExpressionAnalyzer : SubExpressionAnalyzer {
    std::string varname;

    MemberExpressionAnalyzer(std::string varname, const Token* e, ValueFlow::Value val, const std::shared_ptr<PartialReadContainer>& p, const Settings& s)
        : SubExpressionAnalyzer(e, std::move(val), p, s), varname(std::move(varname))
    {}

    bool submatch(const Token* tok, bool exact) const override
    {
        if (!Token::Match(tok, ". %var%"))
            return false;
        if (!exact)
            return true;
        return tok->strAt(1) == varname;
    }
};

ValuePtr<Analyzer> makeMemberExpressionAnalyzer(std::string varname, const Token* e, ValueFlow::Value val, const std::shared_ptr<PartialReadContainer>& p, const Settings& s)
{
    return MemberExpressionAnalyzer{std::move(varname), e, std::move(val), p, s};
}

struct ContainerExpressionAnalyzer : ExpressionAnalyzer {
    ContainerExpressionAnalyzer(const Token* expr, ValueFlow::Value val, const Settings& s)
        : ExpressionAnalyzer(expr, std::move(val), s)
    {}

    bool match(const Token* tok) const override {
        return tok->exprId() == expr->exprId() || (astIsIterator(tok) && isAliasOf(tok, expr->exprId()));
    }

    Action isWritable(const Token* tok, Direction /*d*/) const override
    {
        if (astIsIterator(tok))
            return Action::None;
        if (!getValue(tok))
            return Action::None;
        if (!tok->valueType())
            return Action::None;
        if (!astIsContainer(tok))
            return Action::None;
        const Token* parent = tok->astParent();
        const Library::Container* container = getLibraryContainer(tok);

        if (container->stdStringLike && Token::simpleMatch(parent, "+=") && astIsLHS(tok) && parent->astOperand2()) {
            const Token* rhs = parent->astOperand2();
            if (rhs->tokType() == Token::eString)
                return Action::Read | Action::Write | Action::Incremental;
            const Library::Container* rhsContainer = getLibraryContainer(rhs);
            if (rhsContainer && rhsContainer->stdStringLike) {
                if (std::any_of(rhs->values().cbegin(), rhs->values().cend(), [&](const ValueFlow::Value &rhsval) {
                    return rhsval.isKnown() && rhsval.isContainerSizeValue();
                }))
                    return Action::Read | Action::Write | Action::Incremental;
            }
        } else if (astIsLHS(tok) && Token::Match(tok->astParent(), ". %name% (")) {
            const Library::Container::Action action = container->getAction(tok->astParent()->strAt(1));
            if (action == Library::Container::Action::PUSH || action == Library::Container::Action::POP || action == Library::Container::Action::APPEND) { // TODO: handle more actions?
                std::vector<const Token*> args = getArguments(tok->tokAt(3));
                bool isVariadic = false;
                if (const Library::Function* libFunc = settings.library.getFunction(tok->tokAt(2))) {
                    const auto& argChecks = libFunc->argumentChecks;
                    isVariadic = argChecks.find(-1) != argChecks.end() && argChecks.at(-1).variadic;
                }
                if (args.size() < 2 || action == Library::Container::Action::APPEND || isVariadic)
                    return Action::Read | Action::Write | Action::Incremental;
            }
        }
        return Action::None;
    }

    void writeValue(ValueFlow::Value* val, const Token* tok, Direction d) const override {
        if (!val)
            return;
        if (!tok->astParent())
            return;
        if (!tok->valueType())
            return;
        if (!astIsContainer(tok))
            return;
        const Token* parent = tok->astParent();
        const Library::Container* container = getLibraryContainer(tok);
        MathLib::bigint n = 0;

        if (container->stdStringLike && Token::simpleMatch(parent, "+=") && parent->astOperand2()) {
            const Token* rhs = parent->astOperand2();
            const Library::Container* rhsContainer = getLibraryContainer(rhs);
            if (rhs->tokType() == Token::eString)
                n = Token::getStrLength(rhs);
            else if (rhsContainer && rhsContainer->stdStringLike) {
                auto it = std::find_if(rhs->values().begin(), rhs->values().end(), [&](const ValueFlow::Value& rhsval) {
                    return rhsval.isKnown() && rhsval.isContainerSizeValue();
                });
                if (it != rhs->values().end())
                    n = it->intvalue;
            }
        } else if (astIsLHS(tok) && Token::Match(tok->astParent(), ". %name% (")) {
            const Library::Container::Action action = container->getAction(tok->astParent()->strAt(1));
            switch (action) {
            case Library::Container::Action::PUSH:
                n = 1;
                break;
            case Library::Container::Action::POP:
                n = -1;
                break;
            case Library::Container::Action::APPEND: {
                std::vector<const Token*> args = getArguments(tok->astParent()->tokAt(2));
                if (args.size() == 1) // TODO: handle overloads
                    n = ValueFlow::valueFlowGetStrLength(tok->astParent()->tokAt(3));
                if (n == 0) // TODO: handle known empty append
                    val->setPossible();
                break;
            }
            default:
                break;
            }
        }
        if (d == Direction::Reverse)
            val->intvalue -= n;
        else
            val->intvalue += n;
    }

    int getIndirect(const Token* tok) const override
    {
        if (tok->valueType()) {
            return tok->valueType()->pointer;
        }
        return ValueFlowAnalyzer::getIndirect(tok);
    }

    Action isModified(const Token* tok) const override {
        // An iterator won't change the container size
        if (astIsIterator(tok))
            return Action::Read;
        if (Token::Match(tok->astParent(), "%assign%") && astIsLHS(tok))
            return Action::Invalid;
        if (isLikelyStreamRead(tok->astParent()))
            return Action::Invalid;
        if (astIsContainer(tok) && ValueFlow::isContainerSizeChanged(tok, getIndirect(tok), getSettings()))
            return Action::Read | Action::Invalid;
        return Action::Read;
    }
};

static const Token* solveExprValue(const Token* expr, ValueFlow::Value& value)
{
    return ValueFlow::solveExprValue(
        expr,
        [](const Token* tok) -> std::vector<MathLib::bigint> {
        if (const ValueFlow::Value* v = tok->getKnownValue(ValueFlow::Value::ValueType::INT))
            return {v->intvalue};
        return {};
    },
        value);
}

ValuePtr<Analyzer> makeAnalyzer(const Token* exprTok, ValueFlow::Value value, const Settings& settings)
{
    if (value.isContainerSizeValue())
        return ContainerExpressionAnalyzer(exprTok, std::move(value), settings);
    const Token* expr = solveExprValue(exprTok, value);
    return ExpressionAnalyzer(expr, std::move(value), settings);
}

ValuePtr<Analyzer> makeReverseAnalyzer(const Token* exprTok, ValueFlow::Value value, const Settings& settings)
{
    if (value.isContainerSizeValue())
        return ContainerExpressionAnalyzer(exprTok, std::move(value), settings);
    return ExpressionAnalyzer(exprTok, std::move(value), settings);
}
