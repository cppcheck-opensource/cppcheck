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

#include "programmemory.h"

#include "astutils.h"
#include "calculate.h"
#include "infer.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h"
#include "valueptr.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

ExprIdToken::ExprIdToken(const Token* tok)
    : tok(tok)
{
    assert(tok);
    exprid = tok->exprId();
}

ExprIdToken::ExprIdToken(nonneg int exprId) : exprid(exprId) {}

nonneg int ExprIdToken::getExpressionId() const {
    return exprid;
}

std::size_t ExprIdToken::Hash::operator()(ExprIdToken etok) const
{
    return std::hash<nonneg int>()(etok.getExpressionId());
}

void ProgramMemory::setValue(const Token* expr, const ValueFlow::Value& value) {
    if (!expr)
        return;

    copyOnWrite();

    ValueFlow::Value subvalue = value;
    const Token* subexpr = solveExprValue(
        expr,
        [&](const Token* tok) -> std::vector<MathLib::bigint> {
        if (const ValueFlow::Value* v = tok->getKnownValue(ValueFlow::Value::ValueType::INT))
            return {v->intvalue};
        MathLib::bigint result = 0;
        if (getIntValue(tok->exprId(), result))
            return {result};
        return {};
    },
        subvalue);
    if (expr != subexpr)
        (*mValues)[expr] = value;
    if (subexpr)
        (*mValues)[subexpr] = std::move(subvalue);
}

const ValueFlow::Value* ProgramMemory::getValue(nonneg int exprid, bool impossible) const
{
    const auto it = find(exprid);
    const bool found = it != mValues->cend() && (impossible || !it->second.isImpossible());
    if (found)
        return &it->second;
    return nullptr;
}

bool ProgramMemory::getIntValue(nonneg int exprid, MathLib::bigint& result) const
{
    const ValueFlow::Value* value = getValue(exprid);
    if (value && value->isIntValue()) {
        result = value->intvalue;
        return true;
    }
    return false;
}

void ProgramMemory::setIntValue(const Token* expr, MathLib::bigint value, bool impossible)
{
    ValueFlow::Value v(value);
    if (impossible)
        v.setImpossible();
    setValue(expr, v);
}

bool ProgramMemory::getTokValue(nonneg int exprid, const Token*& result) const
{
    const ValueFlow::Value* value = getValue(exprid);
    if (value && value->isTokValue()) {
        result = value->tokvalue;
        return true;
    }
    return false;
}

// cppcheck-suppress unusedFunction
bool ProgramMemory::getContainerSizeValue(nonneg int exprid, MathLib::bigint& result) const
{
    const ValueFlow::Value* value = getValue(exprid);
    if (value && value->isContainerSizeValue()) {
        result = value->intvalue;
        return true;
    }
    return false;
}
bool ProgramMemory::getContainerEmptyValue(nonneg int exprid, MathLib::bigint& result) const
{
    const ValueFlow::Value* value = getValue(exprid, true);
    if (value && value->isContainerSizeValue()) {
        if (value->isImpossible() && value->intvalue == 0) {
            result = false;
            return true;
        }
        if (!value->isImpossible()) {
            result = (value->intvalue == 0);
            return true;
        }
    }
    return false;
}

void ProgramMemory::setContainerSizeValue(const Token* expr, MathLib::bigint value, bool isEqual)
{
    ValueFlow::Value v(value);
    v.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
    if (!isEqual)
        v.valueKind = ValueFlow::Value::ValueKind::Impossible;
    setValue(expr, v);
}

void ProgramMemory::setUnknown(const Token* expr) {
    copyOnWrite();

    (*mValues)[expr].valueType = ValueFlow::Value::ValueType::UNINIT;
}

bool ProgramMemory::hasValue(nonneg int exprid) const
{
    const auto it = find(exprid);
    return it != mValues->cend();
}

const ValueFlow::Value& ProgramMemory::at(nonneg int exprid) const {
    const auto it = find(exprid);
    if (it == mValues->cend()) {
        throw std::out_of_range("ProgramMemory::at");
    }
    return it->second;
}

ValueFlow::Value& ProgramMemory::at(nonneg int exprid) {
    copyOnWrite();

    const auto it = find(exprid);
    if (it == mValues->end()) {
        throw std::out_of_range("ProgramMemory::at");
    }
    return it->second;
}

void ProgramMemory::erase_if(const std::function<bool(const ExprIdToken&)>& pred)
{
    if (mValues->empty())
        return;

    // TODO: how to delay until we actually modify?
    copyOnWrite();

    for (auto it = mValues->begin(); it != mValues->end();) {
        if (pred(it->first))
            it = mValues->erase(it);
        else
            ++it;
    }
}

void ProgramMemory::swap(ProgramMemory &pm) noexcept
{
    mValues.swap(pm.mValues);
}

void ProgramMemory::clear()
{
    if (mValues->empty())
        return;

    copyOnWrite();

    mValues->clear();
}

bool ProgramMemory::empty() const
{
    return mValues->empty();
}

// NOLINTNEXTLINE(performance-unnecessary-value-param) - technically correct but we are moving the given values
void ProgramMemory::replace(ProgramMemory pm, bool skipUnknown)
{
    if (pm.empty())
        return;

    copyOnWrite();

    for (auto&& p : (*pm.mValues)) {
        if (skipUnknown) {
            auto it = mValues->find(p.first);
            if (it != mValues->end() && it->second.isUninitValue())
                continue;
        }
        (*mValues)[p.first] = std::move(p.second);
    }
}

void ProgramMemory::copyOnWrite()
{
    if (mValues.use_count() == 1)
        return;

    mValues = std::make_shared<Map>(*mValues);
}

ProgramMemory::Map::const_iterator ProgramMemory::find(nonneg int exprid) const
{
    const auto& cvalues = utils::as_const(*mValues);
    return cvalues.find(ExprIdToken::create(exprid));
}

ProgramMemory::Map::iterator ProgramMemory::find(nonneg int exprid)
{
    return mValues->find(ExprIdToken::create(exprid));
}

static ValueFlow::Value execute(const Token* expr, ProgramMemory& pm, const Settings& settings);

static bool evaluateCondition(MathLib::bigint r, const Token* condition, ProgramMemory& pm, const Settings& settings)
{
    if (!condition)
        return false;
    MathLib::bigint result = 0;
    bool error = false;
    execute(condition, pm, &result, &error, settings);
    return !error && result == r;
}

bool conditionIsFalse(const Token* condition, ProgramMemory pm, const Settings& settings)
{
    return evaluateCondition(0, condition, pm, settings);
}

bool conditionIsTrue(const Token* condition, ProgramMemory pm, const Settings& settings)
{
    return evaluateCondition(1, condition, pm, settings);
}

static bool frontIs(const std::vector<MathLib::bigint>& v, bool i)
{
    if (v.empty())
        return false;
    if (v.front())
        return i;
    return !i;
}

static bool isTrue(const ValueFlow::Value& v)
{
    if (v.isUninitValue())
        return false;
    if (v.isImpossible())
        return v.intvalue == 0;
    return v.intvalue != 0;
}

static bool isFalse(const ValueFlow::Value& v)
{
    if (v.isUninitValue())
        return false;
    if (v.isImpossible())
        return false;
    return v.intvalue == 0;
}

static bool isTrueOrFalse(const ValueFlow::Value& v, bool b)
{
    if (b)
        return isTrue(v);
    return isFalse(v);
}

// If the scope is a non-range for loop
static bool isBasicForLoop(const Token* tok)
{
    if (!tok)
        return false;
    if (Token::simpleMatch(tok, "}"))
        return isBasicForLoop(tok->link());
    if (!Token::simpleMatch(tok->previous(), ") {"))
        return false;
    const Token* start = tok->linkAt(-1);
    if (!start)
        return false;
    if (!Token::simpleMatch(start->previous(), "for ("))
        return false;
    if (!Token::simpleMatch(start->astOperand2(), ";"))
        return false;
    return true;
}

static void programMemoryParseCondition(ProgramMemory& pm, const Token* tok, const Token* endTok, const Settings& settings, bool then)
{
    auto eval = [&](const Token* t) -> std::vector<MathLib::bigint> {
        if (!t)
            return std::vector<MathLib::bigint>{};
        if (const ValueFlow::Value* v = t->getKnownValue(ValueFlow::Value::ValueType::INT))
            return {v->intvalue};
        MathLib::bigint result = 0;
        bool error = false;
        execute(t, pm, &result, &error, settings);
        if (!error)
            return {result};
        return std::vector<MathLib::bigint>{};
    };
    if (Token::Match(tok, "==|>=|<=|<|>|!=")) {
        ValueFlow::Value truevalue;
        ValueFlow::Value falsevalue;
        const Token* vartok = parseCompareInt(tok, truevalue, falsevalue, eval);
        if (!vartok)
            return;
        if (vartok->exprId() == 0)
            return;
        if (!truevalue.isIntValue())
            return;
        if (endTok && findExpressionChanged(vartok, tok->next(), endTok, settings))
            return;
        const bool impossible = (tok->str() == "==" && !then) || (tok->str() == "!=" && then);
        const ValueFlow::Value& v = then ? truevalue : falsevalue;
        pm.setValue(vartok, impossible ? asImpossible(v) : v);
        const Token* containerTok = settings.library.getContainerFromYield(vartok, Library::Container::Yield::SIZE);
        if (containerTok)
            pm.setContainerSizeValue(containerTok, v.intvalue, !impossible);
    } else if (Token::simpleMatch(tok, "!")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, !then);
    } else if (then && Token::simpleMatch(tok, "&&")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then);
        programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then);
    } else if (!then && Token::simpleMatch(tok, "||")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then);
        programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then);
    } else if (Token::Match(tok, "&&|%oror%")) {
        std::vector<MathLib::bigint> lhs = eval(tok->astOperand1());
        std::vector<MathLib::bigint> rhs = eval(tok->astOperand2());
        if (lhs.empty() || rhs.empty()) {
            if (frontIs(lhs, !then))
                programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then);
            else if (frontIs(rhs, !then))
                programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then);
            else
                pm.setIntValue(tok, 0, then);
        }
    } else if (tok && tok->exprId() > 0) {
        if (endTok && findExpressionChanged(tok, tok->next(), endTok, settings))
            return;
        pm.setIntValue(tok, 0, then);
        const Token* containerTok = settings.library.getContainerFromYield(tok, Library::Container::Yield::EMPTY);
        if (containerTok)
            pm.setContainerSizeValue(containerTok, 0, then);
    }
}

static void fillProgramMemoryFromConditions(ProgramMemory& pm, const Scope* scope, const Token* endTok, const Settings& settings)
{
    if (!scope)
        return;
    if (!scope->isLocal())
        return;
    assert(scope != scope->nestedIn);
    fillProgramMemoryFromConditions(pm, scope->nestedIn, endTok, settings);
    if (scope->type == ScopeType::eIf || scope->type == ScopeType::eWhile || scope->type == ScopeType::eElse || scope->type == ScopeType::eFor) {
        const Token* condTok = getCondTokFromEnd(scope->bodyEnd);
        if (!condTok)
            return;
        MathLib::bigint result = 0;
        bool error = false;
        execute(condTok, pm, &result, &error, settings);
        if (error)
            programMemoryParseCondition(pm, condTok, endTok, settings, scope->type != ScopeType::eElse);
    }
}

static void fillProgramMemoryFromConditions(ProgramMemory& pm, const Token* tok, const Settings& settings)
{
    fillProgramMemoryFromConditions(pm, tok->scope(), tok, settings);
}

static void fillProgramMemoryFromAssignments(ProgramMemory& pm, const Token* tok, const Settings& settings, const ProgramMemory& state, const ProgramMemory::Map& vars)
{
    int indentlevel = 0;
    for (const Token *tok2 = tok; tok2; tok2 = tok2->previous()) {
        if ((Token::simpleMatch(tok2, "=") || Token::Match(tok2->previous(), "%var% (|{")) && tok2->astOperand1() &&
            tok2->astOperand2()) {
            bool setvar = false;
            const Token* vartok = tok2->astOperand1();
            for (const auto& p:vars) {
                if (p.first.getExpressionId() != vartok->exprId())
                    continue;
                if (vartok == tok)
                    continue;
                pm.setValue(vartok, p.second);
                setvar = true;
            }
            if (!setvar) {
                if (!pm.hasValue(vartok->exprId())) {
                    const Token* valuetok = tok2->astOperand2();
                    ProgramMemory local = state;
                    pm.setValue(vartok, execute(valuetok, local, settings));
                }
            }
        } else if (Token::simpleMatch(tok2, ")") && tok2->link() &&
                   Token::Match(tok2->link()->previous(), "assert|ASSERT ( !!)")) {
            const Token* cond = tok2->link()->astOperand2();
            if (!conditionIsTrue(cond, state, settings)) {
                // TODO: change to assert when we can propagate the assert, for now just bail
                if (conditionIsFalse(cond, state, settings))
                    return;
                programMemoryParseCondition(pm, cond, nullptr, settings, true);
            }
            tok2 = tok2->link()->previous();
        } else if (tok2->exprId() > 0 && Token::Match(tok2, ".|(|[|*|%var%") && !pm.hasValue(tok2->exprId()) &&
                   isVariableChanged(tok2, 0, settings)) {
            pm.setUnknown(tok2);
        }

        if (tok2->str() == "{") {
            if (indentlevel <= 0) {
                const Token* cond = getCondTokFromEnd(tok2->link());
                // Keep progressing with anonymous/do scopes and always true branches
                if (!Token::Match(tok2->previous(), "do|; {") && !conditionIsTrue(cond, state, settings) &&
                    (cond || !isBasicForLoop(tok2)))
                    break;
            } else
                --indentlevel;
            if (Token::simpleMatch(tok2->previous(), "else {"))
                tok2 = tok2->linkAt(-2)->previous();
        }
        if (tok2->str() == "}" && !Token::Match(tok2->link()->previous(), "%var% {")) {
            const Token *cond = getCondTokFromEnd(tok2);
            const bool inElse = Token::simpleMatch(tok2->link()->previous(), "else {");
            if (cond) {
                if (conditionIsFalse(cond, state, settings)) {
                    if (inElse) {
                        ++indentlevel;
                        continue;
                    }
                } else if (conditionIsTrue(cond, state, settings)) {
                    if (inElse)
                        tok2 = tok2->link()->tokAt(-2);
                    ++indentlevel;
                    continue;
                }
            }
            break;
        }
    }
}

static void removeModifiedVars(ProgramMemory& pm, const Token* tok, const Token* origin, const Settings& settings)
{
    pm.erase_if([&](const ExprIdToken& e) {
        return isVariableChanged(origin, tok, e.getExpressionId(), false, settings);
    });
}

static ProgramMemory getInitialProgramState(const Token* tok,
                                            const Token* origin,
                                            const Settings& settings,
                                            const ProgramMemory::Map& vars = ProgramMemory::Map {})
{
    ProgramMemory pm;
    if (origin) {
        fillProgramMemoryFromConditions(pm, origin, settings);
        const ProgramMemory state = pm;
        fillProgramMemoryFromAssignments(pm, tok, settings, state, vars);
        removeModifiedVars(pm, tok, origin, settings);
    }
    return pm;
}

ProgramMemoryState::ProgramMemoryState(const Settings& s) : settings(s)
{}

void ProgramMemoryState::replace(ProgramMemory pm, const Token* origin)
{
    if (origin)
        for (const auto& p : pm)
            origins[p.first.getExpressionId()] = origin;
    state.replace(std::move(pm), /*skipUnknown*/ true);
}

static void addVars(ProgramMemory& pm, const ProgramMemory::Map& vars)
{
    for (const auto& p:vars) {
        const ValueFlow::Value &value = p.second;
        pm.setValue(p.first.tok, value);
    }
}

void ProgramMemoryState::addState(const Token* tok, const ProgramMemory::Map& vars)
{
    ProgramMemory local = state;
    addVars(local, vars);
    fillProgramMemoryFromConditions(local, tok, settings);
    ProgramMemory pm;
    fillProgramMemoryFromAssignments(pm, tok, settings, local, vars);
    local.replace(std::move(pm));
    addVars(local, vars);
    replace(std::move(local), tok);
}

void ProgramMemoryState::assume(const Token* tok, bool b, bool isEmpty)
{
    ProgramMemory pm = state;
    if (isEmpty)
        pm.setContainerSizeValue(tok, 0, b);
    else
        programMemoryParseCondition(pm, tok, nullptr, settings, b);
    const Token* origin = tok;
    const Token* top = tok->astTop();
    if (Token::Match(top->previous(), "for|while|if (") && !Token::simpleMatch(tok->astParent(), "?")) {
        origin = top->link()->next();
        if (!b && origin->link()) {
            origin = origin->link();
        }
    }
    replace(std::move(pm), origin);
}

void ProgramMemoryState::removeModifiedVars(const Token* tok)
{
    const ProgramMemory& pm = state;
    auto eval = [&](const Token* cond) -> std::vector<MathLib::bigint> {
        ProgramMemory pm2 = pm;
        auto result = execute(cond, pm2, settings);
        if (isTrue(result))
            return {1};
        if (isFalse(result))
            return {0};
        return {};
    };
    state.erase_if([&](const ExprIdToken& e) {
        const Token* start = origins[e.getExpressionId()];
        const Token* expr = e.tok;
        if (!expr || findExpressionChangedSkipDeadCode(expr, start, tok, settings, eval)) {
            origins.erase(e.getExpressionId());
            return true;
        }
        return false;
    });
}

ProgramMemory ProgramMemoryState::get(const Token* tok, const Token* ctx, const ProgramMemory::Map& vars) const
{
    ProgramMemoryState local = *this;
    if (ctx)
        local.addState(ctx, vars);
    const Token* start = previousBeforeAstLeftmostLeaf(tok);
    if (!start)
        start = tok;

    if (!ctx || precedes(start, ctx)) {
        local.removeModifiedVars(start);
        local.addState(start, vars);
    } else {
        local.removeModifiedVars(ctx);
    }
    return local.state;
}

ProgramMemory getProgramMemory(const Token* tok, const Token* expr, const ValueFlow::Value& value, const Settings& settings)
{
    ProgramMemory programMemory;
    programMemory.replace(getInitialProgramState(tok, value.tokvalue, settings));
    programMemory.replace(getInitialProgramState(tok, value.condition, settings));
    fillProgramMemoryFromConditions(programMemory, tok, settings);
    programMemory.setValue(expr, value);
    const ProgramMemory state = programMemory;
    fillProgramMemoryFromAssignments(programMemory, tok, settings, state, {{expr, value}});
    return programMemory;
}

static bool isNumericValue(const ValueFlow::Value& value) {
    return value.isIntValue() || value.isFloatValue();
}

static double asFloat(const ValueFlow::Value& value)
{
    return value.isFloatValue() ? value.floatValue : static_cast<double>(value.intvalue);
}

static MathLib::bigint asInt(const ValueFlow::Value& value)
{
    return value.isFloatValue() ? static_cast<MathLib::bigint>(value.floatValue) : value.intvalue;
}

static std::string removeAssign(const std::string& assign) {
    return std::string{assign.cbegin(), assign.cend() - 1};
}

namespace {
    struct assign {
        template<class T, class U>
        void operator()(T& x, const U& y) const
        {
            x = static_cast<T>(y);
        }
    };
}

static bool isIntegralValue(const ValueFlow::Value& value)
{
    return value.isIntValue() || value.isIteratorValue() || value.isSymbolicValue();
}

static ValueFlow::Value evaluate(const std::string& op, const ValueFlow::Value& lhs, const ValueFlow::Value& rhs)
{
    ValueFlow::Value result;
    if (lhs.isImpossible() && rhs.isImpossible())
        return ValueFlow::Value::unknown();
    if (lhs.isImpossible() || rhs.isImpossible()) {
        // noninvertible
        if (contains({"%", "/", "&", "|"}, op))
            return ValueFlow::Value::unknown();
        result.setImpossible();
    }
    if (isNumericValue(lhs) && isNumericValue(rhs)) {
        if (lhs.isFloatValue() || rhs.isFloatValue()) {
            result.valueType = ValueFlow::Value::ValueType::FLOAT;
            bool error = false;
            result.floatValue = calculate(op, asFloat(lhs), asFloat(rhs), &error);
            if (error)
                return ValueFlow::Value::unknown();
            return result;
        }
    }
    // Must be integral types
    if (!isIntegralValue(lhs) && !isIntegralValue(rhs))
        return ValueFlow::Value::unknown();
    // If not the same type then one must be int
    if (lhs.valueType != rhs.valueType && !lhs.isIntValue() && !rhs.isIntValue())
        return ValueFlow::Value::unknown();
    const bool compareOp = contains({"==", "!=", "<", ">", ">=", "<="}, op);
    // Comparison must be the same type
    if (compareOp && lhs.valueType != rhs.valueType)
        return ValueFlow::Value::unknown();
    // Only add, subtract, and compare for non-integers
    if (!compareOp && !contains({"+", "-"}, op) && !lhs.isIntValue() && !rhs.isIntValue())
        return ValueFlow::Value::unknown();
    // Both can't be iterators for non-compare
    if (!compareOp && lhs.isIteratorValue() && rhs.isIteratorValue())
        return ValueFlow::Value::unknown();
    // Symbolic values must be in the same ring
    if (lhs.isSymbolicValue() && rhs.isSymbolicValue() && lhs.tokvalue != rhs.tokvalue)
        return ValueFlow::Value::unknown();
    if (!lhs.isIntValue() && !compareOp) {
        result.valueType = lhs.valueType;
        result.tokvalue = lhs.tokvalue;
    } else if (!rhs.isIntValue() && !compareOp) {
        result.valueType = rhs.valueType;
        result.tokvalue = rhs.tokvalue;
    } else {
        result.valueType = ValueFlow::Value::ValueType::INT;
    }
    bool error = false;
    result.intvalue = calculate(op, lhs.intvalue, rhs.intvalue, &error);
    if (error)
        return ValueFlow::Value::unknown();
    if (result.isImpossible() && op == "!=") {
        if (isTrue(result)) {
            result.intvalue = 1;
        } else if (isFalse(result)) {
            result.intvalue = 0;
        } else {
            return ValueFlow::Value::unknown();
        }
        result.setPossible();
        result.bound = ValueFlow::Value::Bound::Point;
    }
    return result;
}

using BuiltinLibraryFunction = std::function<ValueFlow::Value (const std::vector<ValueFlow::Value>&)>;
static std::unordered_map<std::string, BuiltinLibraryFunction> createBuiltinLibraryFunctions()
{
    std::unordered_map<std::string, BuiltinLibraryFunction> functions;
    functions["strlen"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!(v_ref.isTokValue() && v_ref.tokvalue->tokType() == Token::eString))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.valueType = ValueFlow::Value::ValueType::INT;
        v.intvalue = Token::getStrLength(v.tokvalue);
        v.tokvalue = nullptr;
        return v;
    };
    functions["strcmp"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& lhs = args[0];
        if (!(lhs.isTokValue() && lhs.tokvalue->tokType() == Token::eString))
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& rhs = args[1];
        if (!(rhs.isTokValue() && rhs.tokvalue->tokType() == Token::eString))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v(getStringLiteral(lhs.tokvalue->str()).compare(getStringLiteral(rhs.tokvalue->str())));
        ValueFlow::combineValueProperties(lhs, rhs, v);
        return v;
    };
    functions["strncmp"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 3)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& lhs = args[0];
        if (!(lhs.isTokValue() && lhs.tokvalue->tokType() == Token::eString))
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& rhs = args[1];
        if (!(rhs.isTokValue() && rhs.tokvalue->tokType() == Token::eString))
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& len = args[2];
        if (!len.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v(getStringLiteral(lhs.tokvalue->str())
                           .compare(0, len.intvalue, getStringLiteral(rhs.tokvalue->str()), 0, len.intvalue));
        ValueFlow::combineValueProperties(lhs, rhs, v);
        return v;
    };
    functions["sin"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::sin(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["lgamma"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::lgamma(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["cos"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::cos(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["tan"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::tan(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["asin"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::asin(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["acos"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::acos(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["atan"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::atan(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["atan2"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::atan2(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["remainder"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::remainder(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["nextafter"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::nextafter(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["nexttoward"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::nexttoward(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["hypot"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::hypot(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["fdim"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::fdim(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["fmax"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::fmax(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["fmin"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::fmin(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["fmod"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::fmod(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["pow"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::pow(asFloat(args[0]), asFloat(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["scalbln"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::scalbln(asFloat(args[0]), asInt(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["ldexp"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 2 || !std::all_of(args.cbegin(), args.cend(), [](const ValueFlow::Value& v) {
            return v.isFloatValue() || v.isIntValue();
        }))
            return ValueFlow::Value::unknown();
        ValueFlow::Value v;
        combineValueProperties(args[0], args[1], v);
        v.floatValue = std::ldexp(asFloat(args[0]), asInt(args[1]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["ilogb"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.intvalue = std::ilogb(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::INT;
        return v;
    };
    functions["erf"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::erf(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["erfc"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::erfc(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["floor"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::floor(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["sqrt"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::sqrt(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["cbrt"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::cbrt(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["ceil"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::ceil(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["exp"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::exp(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["exp2"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::exp2(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["expm1"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::expm1(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["fabs"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::fabs(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["log"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::log(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["log10"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::log10(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["log1p"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::log1p(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["log2"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::log2(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["logb"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::logb(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["nearbyint"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::nearbyint(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["sinh"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::sinh(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["cosh"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::cosh(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["tanh"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::tanh(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["asinh"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::asinh(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["acosh"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::acosh(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["atanh"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::atanh(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["round"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::round(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["tgamma"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::tgamma(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    functions["trunc"] = [](const std::vector<ValueFlow::Value>& args) {
        if (args.size() != 1)
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& v_ref = args[0];
        if (!v_ref.isFloatValue() && !v_ref.isIntValue())
            return ValueFlow::Value::unknown();
        ValueFlow::Value v = v_ref;
        v.floatValue = std::trunc(asFloat(args[0]));
        v.valueType = ValueFlow::Value::ValueType::FLOAT;
        return v;
    };
    return functions;
}

static BuiltinLibraryFunction getBuiltinLibraryFunction(const std::string& name)
{
    static const std::unordered_map<std::string, BuiltinLibraryFunction> functions = createBuiltinLibraryFunctions();
    auto it = functions.find(name);
    if (it == functions.end())
        return nullptr;
    return it->second;
}
static bool TokenExprIdCompare(const Token* tok1, const Token* tok2) {
    return tok1->exprId() < tok2->exprId();
}
static bool TokenExprIdEqual(const Token* tok1, const Token* tok2) {
    return tok1->exprId() == tok2->exprId();
}

static std::vector<const Token*> setDifference(const std::vector<const Token*>& v1, const std::vector<const Token*>& v2)
{
    std::vector<const Token*> result;
    std::set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), std::back_inserter(result), &TokenExprIdCompare);
    return result;
}

static bool evalSameCondition(const ProgramMemory& state,
                              const Token* storedValue,
                              const Token* cond,
                              const Settings& settings)
{
    assert(!conditionIsTrue(cond, state, settings));
    ProgramMemory pm = state;
    programMemoryParseCondition(pm, storedValue, nullptr, settings, true);
    if (pm == state)
        return false;
    return conditionIsTrue(cond, std::move(pm), settings);
}

static void pruneConditions(std::vector<const Token*>& conds,
                            bool b,
                            const std::unordered_map<nonneg int, ValueFlow::Value>& state)
{
    conds.erase(std::remove_if(conds.begin(),
                               conds.end(),
                               [&](const Token* cond) {
        if (cond->exprId() == 0)
            return false;
        auto it = state.find(cond->exprId());
        if (it == state.end())
            return false;
        const ValueFlow::Value& v = it->second;
        return isTrueOrFalse(v, !b);
    }),
                conds.end());
}

namespace {
    struct Executor {
        ProgramMemory* pm;
        const Settings& settings;
        int fdepth = 4;
        int depth = 10;

        Executor(ProgramMemory* pm, const Settings& settings) : pm(pm), settings(settings)
        {
            assert(pm != nullptr);
        }

        static ValueFlow::Value unknown() {
            return ValueFlow::Value::unknown();
        }

        std::unordered_map<nonneg int, ValueFlow::Value> executeAll(const std::vector<const Token*>& toks,
                                                                    const bool* b = nullptr) const
        {
            std::unordered_map<nonneg int, ValueFlow::Value> result;
            auto state = *this;
            for (const Token* tok : toks) {
                ValueFlow::Value r = state.execute(tok);
                if (r.isUninitValue())
                    continue;
                const bool brk = b && isTrueOrFalse(r, *b);
                result.emplace(tok->exprId(), std::move(r));
                // Short-circuit evaluation
                if (brk)
                    break;
            }
            return result;
        }

        static std::vector<const Token*> flattenConditions(const Token* tok)
        {
            return astFlatten(tok, tok->str().c_str());
        }
        static bool sortConditions(std::vector<const Token*>& conditions)
        {
            if (std::any_of(conditions.begin(), conditions.end(), [](const Token* child) {
                return Token::Match(child, "&&|%oror%");
            }))
                return false;
            // NOLINTNEXTLINE(bugprone-nondeterministic-pointer-iteration-order) - FP with libc++ only
            std::sort(conditions.begin(), conditions.end(), &TokenExprIdCompare);
            conditions.erase(std::unique(conditions.begin(), conditions.end(), &TokenExprIdCompare), conditions.end());
            return !conditions.empty() && conditions.front()->exprId() != 0;
        }

        ValueFlow::Value executeMultiCondition(bool b, const Token* expr)
        {
            if (pm->hasValue(expr->exprId())) {
                const ValueFlow::Value& v = utils::as_const(*pm).at(expr->exprId());
                if (v.isIntValue())
                    return v;
            }

            // Evaluate recursively if there are no exprids
            if ((expr->astOperand1() && expr->astOperand1()->exprId() == 0) ||
                (expr->astOperand2() && expr->astOperand2()->exprId() == 0)) {
                ValueFlow::Value lhs = execute(expr->astOperand1());
                if (isTrueOrFalse(lhs, b))
                    return lhs;
                ValueFlow::Value rhs = execute(expr->astOperand2());
                if (isTrueOrFalse(rhs, b))
                    return rhs;
                if (isTrueOrFalse(lhs, !b) && isTrueOrFalse(rhs, !b))
                    return lhs;
                return unknown();
            }

            nonneg int n = astCount(expr, expr->str().c_str());
            if (n > 50)
                return unknown();
            std::vector<const Token*> conditions1 = flattenConditions(expr);
            if (conditions1.empty())
                return unknown();
            std::unordered_map<nonneg int, ValueFlow::Value> condValues = executeAll(conditions1, &b);
            bool allNegated = true;
            ValueFlow::Value negatedValue = unknown();
            for (const auto& p : condValues) {
                const ValueFlow::Value& v = p.second;
                if (isTrueOrFalse(v, b))
                    return v;
                allNegated &= isTrueOrFalse(v, !b);
                if (allNegated && negatedValue.isUninitValue())
                    negatedValue = v;
            }
            if (condValues.size() == conditions1.size() && allNegated)
                return negatedValue;
            if (n > 4)
                return unknown();
            if (!sortConditions(conditions1))
                return unknown();

            for (const auto& p : *pm) {
                const Token* tok = p.first.tok;
                if (!tok)
                    continue;
                const ValueFlow::Value& value = p.second;

                if (tok->str() == expr->str() && !astHasExpr(tok, expr->exprId())) {
                    // TODO: Handle when it is greater
                    if (n != astCount(tok, expr->str().c_str()))
                        continue;
                    std::vector<const Token*> conditions2 = flattenConditions(tok);
                    if (!sortConditions(conditions2))
                        return unknown();
                    if (conditions1.size() == conditions2.size() &&
                        std::equal(conditions1.begin(), conditions1.end(), conditions2.begin(), &TokenExprIdEqual))
                        return value;
                    std::vector<const Token*> diffConditions1 = setDifference(conditions1, conditions2);
                    pruneConditions(diffConditions1, !b, condValues);
                    if (diffConditions1.size() == conditions1.size())
                        continue;
                    std::vector<const Token*> diffConditions2 = setDifference(conditions2, conditions1);
                    pruneConditions(diffConditions2, !b, executeAll(diffConditions2));
                    if (diffConditions1.size() != diffConditions2.size())
                        continue;
                    for (const Token* cond1 : diffConditions1) {
                        auto it = std::find_if(diffConditions2.begin(), diffConditions2.end(), [&](const Token* cond2) {
                            return evalSameCondition(*pm, cond2, cond1, settings);
                        });
                        if (it == diffConditions2.end())
                            break;
                        diffConditions2.erase(it);
                    }
                    if (diffConditions2.empty())
                        return value;
                }
            }
            return unknown();
        }

        ValueFlow::Value executeImpl(const Token* expr)
        {
            const ValueFlow::Value* value = nullptr;
            if (!expr)
                return unknown();
            if (expr->hasKnownIntValue() && !expr->isAssignmentOp() && expr->str() != ",")
                return *expr->getKnownValue(ValueFlow::Value::ValueType::INT);
            if ((value = expr->getKnownValue(ValueFlow::Value::ValueType::FLOAT)) ||
                (value = expr->getKnownValue(ValueFlow::Value::ValueType::TOK)) ||
                (value = expr->getKnownValue(ValueFlow::Value::ValueType::ITERATOR_START)) ||
                (value = expr->getKnownValue(ValueFlow::Value::ValueType::ITERATOR_END)) ||
                (value = expr->getKnownValue(ValueFlow::Value::ValueType::CONTAINER_SIZE))) {
                return *value;
            }
            if (expr->isNumber()) {
                if (MathLib::isFloat(expr->str()))
                    return unknown();
                MathLib::bigint i = MathLib::toBigNumber(expr);
                if (i < 0 && astIsUnsigned(expr))
                    return unknown();
                return ValueFlow::Value{i};
            }
            if (expr->isBoolean())
                return ValueFlow::Value{expr->str() == "true"};
            if (Token::Match(expr->tokAt(-2), ". %name% (") && astIsContainer(expr->tokAt(-2)->astOperand1())) {
                const Token* containerTok = expr->tokAt(-2)->astOperand1();
                const Library::Container::Yield yield = containerTok->valueType()->container->getYield(expr->strAt(-1));
                if (yield == Library::Container::Yield::SIZE) {
                    ValueFlow::Value v = execute(containerTok);
                    if (!v.isContainerSizeValue())
                        return unknown();
                    v.valueType = ValueFlow::Value::ValueType::INT;
                    return v;
                }
                if (yield == Library::Container::Yield::EMPTY) {
                    ValueFlow::Value v = execute(containerTok);
                    if (!v.isContainerSizeValue())
                        return unknown();
                    if (v.isImpossible() && v.intvalue == 0)
                        return ValueFlow::Value{0};
                    if (!v.isImpossible())
                        return ValueFlow::Value{v.intvalue == 0};
                }
            } else if (expr->isAssignmentOp() && expr->astOperand1() && expr->astOperand2() &&
                       expr->astOperand1()->exprId() > 0) {
                ValueFlow::Value rhs = execute(expr->astOperand2());
                if (rhs.isUninitValue())
                    return unknown();
                if (expr->str() != "=") {
                    if (!pm->hasValue(expr->astOperand1()->exprId()))
                        return unknown();
                    ValueFlow::Value& lhs = pm->at(expr->astOperand1()->exprId());
                    rhs = evaluate(removeAssign(expr->str()), lhs, rhs);
                    if (lhs.isIntValue())
                        ValueFlow::Value::visitValue(rhs, std::bind(assign{}, std::ref(lhs.intvalue), std::placeholders::_1));
                    else if (lhs.isFloatValue())
                        ValueFlow::Value::visitValue(rhs,
                                                     std::bind(assign{}, std::ref(lhs.floatValue), std::placeholders::_1));
                    else
                        return unknown();
                    return lhs;
                }
                pm->setValue(expr->astOperand1(), rhs);
                return rhs;
            } else if (expr->str() == "&&" && expr->astOperand1() && expr->astOperand2()) {
                return executeMultiCondition(false, expr);
            } else if (expr->str() == "||" && expr->astOperand1() && expr->astOperand2()) {
                return executeMultiCondition(true, expr);
            } else if (expr->str() == "," && expr->astOperand1() && expr->astOperand2()) {
                execute(expr->astOperand1());
                return execute(expr->astOperand2());
            } else if (expr->tokType() == Token::eIncDecOp && expr->astOperand1() && expr->astOperand1()->exprId() != 0) {
                if (!pm->hasValue(expr->astOperand1()->exprId()))
                    return ValueFlow::Value::unknown();
                ValueFlow::Value& lhs = pm->at(expr->astOperand1()->exprId());
                if (!lhs.isIntValue())
                    return unknown();
                // overflow
                if (!lhs.isImpossible() && lhs.intvalue == 0 && expr->str() == "--" && astIsUnsigned(expr->astOperand1()))
                    return unknown();

                if (expr->str() == "++")
                    lhs.intvalue++;
                else
                    lhs.intvalue--;
                return lhs;
            } else if (expr->str() == "[" && expr->astOperand1() && expr->astOperand2()) {
                const Token* tokvalue = nullptr;
                if (!pm->getTokValue(expr->astOperand1()->exprId(), tokvalue)) {
                    auto tokvalue_it = std::find_if(expr->astOperand1()->values().cbegin(),
                                                    expr->astOperand1()->values().cend(),
                                                    std::mem_fn(&ValueFlow::Value::isTokValue));
                    if (tokvalue_it == expr->astOperand1()->values().cend() || !tokvalue_it->isKnown()) {
                        return unknown();
                    }
                    tokvalue = tokvalue_it->tokvalue;
                }
                if (!tokvalue || !tokvalue->isLiteral()) {
                    return unknown();
                }
                const std::string strValue = tokvalue->strValue();
                ValueFlow::Value rhs = execute(expr->astOperand2());
                if (!rhs.isIntValue())
                    return unknown();
                const MathLib::bigint index = rhs.intvalue;
                if (index >= 0 && index < strValue.size())
                    return ValueFlow::Value{strValue[static_cast<std::size_t>(index)]};
                if (index == strValue.size())
                    return ValueFlow::Value{};
            } else if (Token::Match(expr, "%cop%") && expr->astOperand1() && expr->astOperand2()) {
                ValueFlow::Value lhs = execute(expr->astOperand1());
                if (lhs.isUninitValue())
                    return unknown();
                ValueFlow::Value rhs = execute(expr->astOperand2());
                if (rhs.isUninitValue())
                    return unknown();
                ValueFlow::Value r = evaluate(expr->str(), lhs, rhs);
                if (expr->isComparisonOp() && (r.isUninitValue() || r.isImpossible())) {
                    if (rhs.isIntValue() && !expr->astOperand1()->values().empty()) {
                        std::vector<ValueFlow::Value> result = infer(makeIntegralInferModel(),
                                                                     expr->str(),
                                                                     expr->astOperand1()->values(),
                                                                     {std::move(rhs)});
                        if (!result.empty() && result.front().isKnown())
                            return std::move(result.front());
                    }
                    if (lhs.isIntValue() && !expr->astOperand2()->values().empty()) {
                        std::vector<ValueFlow::Value> result = infer(makeIntegralInferModel(),
                                                                     expr->str(),
                                                                     {std::move(lhs)},
                                                                     expr->astOperand2()->values());
                        if (!result.empty() && result.front().isKnown())
                            return std::move(result.front());
                    }
                    return unknown();
                }
                return r;
            }
            // Unary ops
            else if (Token::Match(expr, "!|+|-") && expr->astOperand1() && !expr->astOperand2()) {
                ValueFlow::Value lhs = execute(expr->astOperand1());
                if (!lhs.isIntValue())
                    return unknown();
                if (expr->str() == "!") {
                    if (isTrue(lhs)) {
                        lhs.intvalue = 0;
                    } else if (isFalse(lhs)) {
                        lhs.intvalue = 1;
                    } else {
                        return unknown();
                    }
                    lhs.setPossible();
                    lhs.bound = ValueFlow::Value::Bound::Point;
                }
                if (expr->str() == "-")
                    lhs.intvalue = -lhs.intvalue;
                return lhs;
            } else if (expr->str() == "?" && expr->astOperand1() && expr->astOperand2()) {
                ValueFlow::Value cond = execute(expr->astOperand1());
                if (!cond.isIntValue())
                    return unknown();
                const Token* child = expr->astOperand2();
                if (isFalse(cond))
                    return execute(child->astOperand2());
                if (isTrue(cond))
                    return execute(child->astOperand1());

                return unknown();
            } else if (expr->str() == "(" && expr->isCast()) {
                if (expr->astOperand2()) {
                    if (expr->astOperand1()->str() != "dynamic_cast")
                        return execute(expr->astOperand2());
                    return unknown();
                }
                return execute(expr->astOperand1());
            }
            if (expr->exprId() > 0 && pm->hasValue(expr->exprId())) {
                ValueFlow::Value result = utils::as_const(*pm).at(expr->exprId());
                if (result.isImpossible() && result.isIntValue() && result.intvalue == 0 && isUsedAsBool(expr, settings)) {
                    result.intvalue = !result.intvalue;
                    result.setKnown();
                }
                return result;
            }

            if (Token::Match(expr->previous(), ">|%name% {|(")) {
                const Token* ftok = expr->previous();
                const Function* f = ftok->function();
                ValueFlow::Value result = unknown();
                if (expr->str() == "(") {
                    std::vector<const Token*> tokArgs = getArguments(expr);
                    std::vector<ValueFlow::Value> args(tokArgs.size());
                    std::transform(
                        tokArgs.cbegin(), tokArgs.cend(), args.begin(), [&](const Token* tok) {
                        return execute(tok);
                    });
                    if (f) {
                        if (fdepth >= 0 && !f->isImplicitlyVirtual()) {
                            ProgramMemory functionState;
                            for (std::size_t i = 0; i < args.size(); ++i) {
                                const Variable* const arg = f->getArgumentVar(i);
                                if (!arg)
                                    return unknown();
                                functionState.setValue(arg->nameToken(), args[i]);
                            }
                            Executor ex = *this;
                            ex.pm = &functionState;
                            ex.fdepth--;
                            auto r = ex.execute(f->functionScope);
                            if (!r.empty())
                                result = std::move(r.front());
                            // TODO: Track values changed by reference
                        }
                    } else {
                        BuiltinLibraryFunction lf = getBuiltinLibraryFunction(ftok->str());
                        if (lf)
                            return lf(args);
                        const std::string& returnValue = settings.library.returnValue(ftok);
                        if (!returnValue.empty()) {
                            std::unordered_map<nonneg int, ValueFlow::Value> arg_map;
                            int argn = 0;
                            for (const ValueFlow::Value& v : args) {
                                if (!v.isUninitValue())
                                    arg_map[argn] = v;
                                argn++;
                            }
                            return evaluateLibraryFunction(arg_map, returnValue, settings, ftok->isCpp());
                        }
                    }
                }
                // Check if function modifies argument
                visitAstNodes(expr->astOperand2(), [&](const Token* child) {
                    if (child->exprId() > 0 && pm->hasValue(child->exprId())) {
                        ValueFlow::Value& v = pm->at(child->exprId());
                        if (v.valueType == ValueFlow::Value::ValueType::CONTAINER_SIZE) {
                            if (ValueFlow::isContainerSizeChanged(child, v.indirect, settings))
                                v = unknown();
                        } else if (v.valueType != ValueFlow::Value::ValueType::UNINIT) {
                            if (isVariableChanged(child, v.indirect, settings))
                                v = unknown();
                        }
                    }
                    return ChildrenToVisit::op1_and_op2;
                });
                return result;
            }

            return unknown();
        }
        static const ValueFlow::Value* getImpossibleValue(const Token* tok)
        {
            if (!tok)
                return nullptr;
            std::vector<const ValueFlow::Value*> values;
            for (const ValueFlow::Value& v : tok->values()) {
                if (!v.isImpossible())
                    continue;
                if (v.isContainerSizeValue() || v.isIntValue()) {
                    values.push_back(std::addressof(v));
                }
            }
            auto it =
                std::max_element(values.begin(), values.end(), [](const ValueFlow::Value* x, const ValueFlow::Value* y) {
                return x->intvalue < y->intvalue;
            });
            if (it == values.end())
                return nullptr;
            return *it;
        }

        static bool updateValue(ValueFlow::Value& v, ValueFlow::Value x)
        {
            const bool returnValue = !x.isUninitValue() && !x.isImpossible();
            if (v.isUninitValue() || returnValue)
                v = std::move(x);
            return returnValue;
        }

        ValueFlow::Value execute(const Token* expr)
        {
            depth--;
            OnExit onExit{[&] {
                    depth++;
                }};
            if (depth < 0)
                return unknown();
            ValueFlow::Value v = unknown();
            if (updateValue(v, executeImpl(expr)))
                return v;
            if (!expr)
                return v;
            if (expr->exprId() > 0 && pm->hasValue(expr->exprId())) {
                if (updateValue(v, utils::as_const(*pm).at(expr->exprId())))
                    return v;
            }
            // Find symbolic values
            for (const ValueFlow::Value& value : expr->values()) {
                if (!value.isSymbolicValue())
                    continue;
                if (!value.isKnown())
                    continue;
                if (value.tokvalue->exprId() > 0 && !pm->hasValue(value.tokvalue->exprId()))
                    continue;
                const ValueFlow::Value& v_ref = utils::as_const(*pm).at(value.tokvalue->exprId());
                if (!v_ref.isIntValue() && value.intvalue != 0)
                    continue;
                ValueFlow::Value v2 = v_ref;
                v2.intvalue += value.intvalue;
                return v2;
            }
            if (v.isImpossible() && v.isIntValue())
                return v;
            if (const ValueFlow::Value* value = getImpossibleValue(expr))
                return *value;
            return v;
        }

        std::vector<ValueFlow::Value> execute(const Scope* scope)
        {
            if (!scope)
                return {unknown()};
            if (!scope->bodyStart)
                return {unknown()};
            for (const Token* tok = scope->bodyStart->next(); precedes(tok, scope->bodyEnd); tok = tok->next()) {
                const Token* top = tok->astTop();

                if (Token::simpleMatch(top, "return") && top->astOperand1())
                    return {execute(top->astOperand1())};

                if (Token::Match(top, "%op%")) {
                    if (execute(top).isUninitValue())
                        return {unknown()};
                    const Token* next = nextAfterAstRightmostLeaf(top);
                    if (!next)
                        return {unknown()};
                    tok = next;
                } else if (Token::simpleMatch(top->previous(), "if (")) {
                    const Token* condTok = top->astOperand2();
                    ValueFlow::Value v = execute(condTok);
                    if (!v.isIntValue())
                        return {unknown()};
                    const Token* thenStart = top->link()->next();
                    const Token* next = thenStart->link();
                    const Token* elseStart = nullptr;
                    if (Token::simpleMatch(thenStart->link(), "} else {")) {
                        elseStart = thenStart->link()->tokAt(2);
                        next = elseStart->link();
                    }
                    std::vector<ValueFlow::Value> result;
                    if (isTrue(v)) {
                        result = execute(thenStart->scope());
                    } else if (isFalse(v)) {
                        if (elseStart)
                            result = execute(elseStart->scope());
                    } else {
                        return {unknown()};
                    }
                    if (!result.empty())
                        return result;
                    tok = next;
                } else {
                    return {unknown()};
                }
            }
            return {};
        }
    };
}     // namespace

static ValueFlow::Value execute(const Token* expr, ProgramMemory& pm, const Settings& settings)
{
    Executor ex{&pm, settings};
    return ex.execute(expr);
}

std::vector<ValueFlow::Value> execute(const Scope* scope, ProgramMemory& pm, const Settings& settings)
{
    Executor ex{&pm, settings};
    return ex.execute(scope);
}

static std::shared_ptr<Token> createTokenFromExpression(const std::string& returnValue,
                                                        const Settings& settings,
                                                        bool cpp,
                                                        std::unordered_map<nonneg int, const Token*>& lookupVarId)
{
    std::shared_ptr<TokenList> tokenList = std::make_shared<TokenList>(settings, cpp ? Standards::Language::CPP : Standards::Language::C);
    {
        const std::string code = "return " + returnValue + ";";
        if (!tokenList->createTokensFromBuffer(code.data(), code.size()))
            return nullptr;
    }

    // TODO: put in a helper?
    // combine operators, set links, etc..
    std::stack<Token*> lpar;
    for (Token* tok2 = tokenList->front(); tok2; tok2 = tok2->next()) {
        if (Token::Match(tok2, "[!<>=] =")) {
            tok2->str(tok2->str() + "=");
            tok2->deleteNext();
        } else if (tok2->str() == "(")
            lpar.push(tok2);
        else if (tok2->str() == ")") {
            if (lpar.empty())
                return nullptr;
            Token::createMutualLinks(lpar.top(), tok2);
            lpar.pop();
        }
    }
    if (!lpar.empty())
        return nullptr;

    // set varids
    for (Token* tok2 = tokenList->front(); tok2; tok2 = tok2->next()) {
        if (!startsWith(tok2->str(), "arg"))
            continue;
        nonneg int const id = strToInt<nonneg int>(tok2->str().c_str() + 3);
        tok2->varId(id);
        lookupVarId[id] = tok2;
    }

    // Evaluate expression
    tokenList->createAst();
    Token* expr = tokenList->front()->astOperand1();
    ValueFlow::valueFlowConstantFoldAST(expr, settings);
    return {tokenList, expr};
}

ValueFlow::Value evaluateLibraryFunction(const std::unordered_map<nonneg int, ValueFlow::Value>& args,
                                         const std::string& returnValue,
                                         const Settings& settings,
                                         bool cpp)
{
    thread_local static std::unordered_map<std::string,
                                           std::function<ValueFlow::Value(const std::unordered_map<nonneg int, ValueFlow::Value>&, const Settings&)>>
    functions = {};
    if (functions.count(returnValue) == 0) {

        std::unordered_map<nonneg int, const Token*> lookupVarId;
        std::shared_ptr<Token> expr = createTokenFromExpression(returnValue, settings, cpp, lookupVarId);

        functions[returnValue] =
            [lookupVarId, expr](const std::unordered_map<nonneg int, ValueFlow::Value>& xargs, const Settings& settings) {
            if (!expr)
                return ValueFlow::Value::unknown();
            ProgramMemory pm{};
            for (const auto& p : xargs) {
                auto it = lookupVarId.find(p.first);
                if (it != lookupVarId.end())
                    pm.setValue(it->second, p.second);
            }
            return execute(expr.get(), pm, settings);
        };
    }
    return functions.at(returnValue)(args, settings);
}

void execute(const Token* expr,
             ProgramMemory& programMemory,
             MathLib::bigint* result,
             bool* error,
             const Settings& settings)
{
    ValueFlow::Value v = execute(expr, programMemory, settings);
    if (!v.isIntValue() || v.isImpossible()) {
        if (error)
            *error = true;
    } else if (result)
        *result = v.intvalue;
}
