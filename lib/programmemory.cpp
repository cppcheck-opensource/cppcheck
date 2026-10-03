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
#include "vf_common.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <limits>
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

// Does the value carry its range in intvalue?
static bool isRangeValue(const ValueFlow::Value& value)
{
    return value.isIntValue() || value.isContainerSizeValue() || value.isBufferSizeValue() || value.isIteratorValue();
}

// A constraint that is a lower bound: the values up to the bound are impossible
static bool isLowerBound(const ValueFlow::Value& value)
{
    return value.isImpossible() && isRangeValue(value) && value.bound == ValueFlow::Value::Bound::Upper;
}

// A constraint that is an upper bound: the values from the bound on are impossible
static bool isUpperBound(const ValueFlow::Value& value)
{
    return value.isImpossible() && isRangeValue(value) && value.bound == ValueFlow::Value::Bound::Lower;
}

// An impossible value of the expression, without a bound
static bool isImpossiblePoint(const ValueFlow::Value& value)
{
    return value.isImpossible() && value.bound == ValueFlow::Value::Bound::Point;
}

// Is the value (a range when it is impossible with a bound) known to be nonzero?
static bool isTrue(const ValueFlow::Value& v)
{
    if (v.isUninitValue())
        return false;
    if (v.isImpossible()) {
        if (v.bound == ValueFlow::Value::Bound::Point)
            return v.intvalue == 0;
        // An impossible range excludes zero when it lies on one side of it
        return v.isLowerEdge() ? v.rangeEdge() > 0 : v.rangeEdge() < 0;
    }
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

// Does the value satisfy the constraint of the same type?
static bool satisfies(const ValueFlow::Value& value, const ValueFlow::Value& constraint)
{
    if (isImpossiblePoint(constraint))
        return !value.equalValue(constraint);
    if (isLowerBound(constraint))
        return value.intvalue >= constraint.rangeEdge();
    if (isUpperBound(constraint))
        return value.intvalue <= constraint.rangeEdge();
    return false;
}

static bool sameValue(const ValueFlow::Value& x, const ValueFlow::Value& y)
{
    return x == y && x.bound == y.bound;
}

// Is the value already recorded: as the value of the expression, as one of its constraints, or as
// a constraint that the recorded value satisfies?
static bool isRecorded(const ProgramMemory::Values& values, const ValueFlow::Value& value)
{
    if (values.empty() || values.front().valueType != value.valueType)
        return false;
    if (!value.isImpossible())
        return values.size() == 1 && sameValue(values.front(), value);
    if (!values.front().isImpossible())
        return satisfies(values.front(), value);
    return std::any_of(values.cbegin(), values.cend(), [&](const ValueFlow::Value& v) {
        return sameValue(v, value);
    });
}

void ProgramMemory::setValue(const Token* expr, const ValueFlow::Value& value) {
    if (!expr)
        return;

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
        record(expr, value);
    if (subexpr)
        record(subexpr, subvalue);
}

void ProgramMemory::record(const Token* expr, const ValueFlow::Value& value)
{
    const Values* existing = getValues(expr->exprId());
    if (existing && isRecorded(*existing, value))
        return;
    copyOnWrite();
    Values& values = (*mValues)[expr];
    // A value of the expression, a first value, a value of another type or a constraint that the
    // recorded value violates replaces what is recorded. A constraint joins the recorded constraints,
    // merged the way Token::addValue() merges values: weaker bounds are dropped and an impossible
    // value next to a bound moves the bound past it.
    if (!value.isImpossible() || values.empty() || values.front().valueType != value.valueType ||
        !values.front().isImpossible()) {
        values.assign(1, value);
    } else {
        if (values.size() >= 10U)
            return;
        values.push_back(value);
        Token::removeContradictions(values);
    }
}

void ProgramMemory::setValues(const Token* expr, const Values& values)
{
    for (const ValueFlow::Value& value : values)
        setValue(expr, value);
}

const ValueFlow::Value* ProgramMemory::getValue(nonneg int exprid, bool impossible) const
{
    const Values* values = getValues(exprid);
    if (!values || values->size() != 1)
        return nullptr;
    const ValueFlow::Value& value = values->front();
    if (!impossible && value.isImpossible())
        return nullptr;
    return &value;
}

const ProgramMemory::Values* ProgramMemory::getValues(nonneg int exprid) const
{
    const auto it = find(exprid);
    if (it == mValues->cend())
        return nullptr;
    return &it->second;
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

// Is the container empty according to its recorded size values? Unknown if they do not decide it.
static ValueFlow::Value containerEmptyValue(const ProgramMemory::Values& values)
{
    for (const ValueFlow::Value& value : values) {
        if (!value.isContainerSizeValue())
            continue;
        if (!value.isImpossible())
            return ValueFlow::Value{value.intvalue == 0};
        if (isUpperBound(value) && value.rangeEdge() <= 0)
            return ValueFlow::Value{1};
        if (isTrue(value))
            return ValueFlow::Value{0};
    }
    return ValueFlow::Value::unknown();
}

bool ProgramMemory::getContainerEmptyValue(nonneg int exprid, MathLib::bigint& result) const
{
    const Values* values = getValues(exprid);
    if (!values)
        return false;
    const ValueFlow::Value isEmpty = containerEmptyValue(*values);
    if (isEmpty.isUninitValue())
        return false;
    result = isEmpty.intvalue;
    return true;
}

void ProgramMemory::setContainerSizeValue(const Token* expr, MathLib::bigint value, bool equal)
{
    ValueFlow::Value v(value);
    v.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
    if (!equal)
        v.valueKind = ValueFlow::Value::ValueKind::Impossible;
    setValue(expr, v);
}

void ProgramMemory::setUnknown(const Token* expr) {
    copyOnWrite();

    (*mValues)[expr].assign(1, ValueFlow::Value::unknown());
}

bool ProgramMemory::hasValue(nonneg int exprid) const
{
    const auto it = find(exprid);
    return it != mValues->cend();
}

const ProgramMemory::Values& ProgramMemory::at(nonneg int exprid) const {
    const auto it = find(exprid);
    if (it == mValues->cend()) {
        throw std::out_of_range("ProgramMemory::at");
    }
    return it->second;
}

ProgramMemory::Values& ProgramMemory::at(nonneg int exprid) {
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

// Is the expression recorded as modified with an unknown value?
static bool isUnknown(const ProgramMemory::Values& values)
{
    return !values.empty() && values.front().isUninitValue();
}

// NOLINTNEXTLINE(performance-unnecessary-value-param) - technically correct but we are moving the given values
void ProgramMemory::replace(ProgramMemory pm, bool skipUnknown)
{
    if (pm.empty())
        return;

    copyOnWrite();

    // The values can be moved out of the given memory only when no other memory shares them
    const bool owned = pm.mValues.use_count() == 1;
    for (auto&& p : (*pm.mValues)) {
        if (skipUnknown) {
            auto it = mValues->find(p.first);
            if (it != mValues->end() && isUnknown(it->second))
                continue;
        }
        if (owned)
            (*mValues)[p.first] = std::move(p.second);
        else
            (*mValues)[p.first] = p.second;
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

static ValueFlow::Value execute(const Token* expr,
                                ProgramMemory& pm,
                                const Settings& settings,
                                const ProgramMemory::Map& vars = {});

// All values of the expression: the constraints of a range, or the single result of execute()
static ProgramMemory::Values executeValues(const Token* expr,
                                           ProgramMemory& pm,
                                           const Settings& settings,
                                           const ProgramMemory::Map& vars = {});

static bool evaluateCondition(MathLib::bigint r,
                              const Token* condition,
                              ProgramMemory& pm,
                              const Settings& settings,
                              const ProgramMemory::Map& vars = {})
{
    if (!condition)
        return false;
    MathLib::bigint result = 0;
    bool error = false;
    execute(condition, pm, &result, &error, settings, vars);
    return !error && result == r;
}

bool conditionIsFalse(const Token* condition, ProgramMemory pm, const Settings& settings, const ProgramMemory::Map& vars)
{
    return evaluateCondition(0, condition, pm, settings, vars);
}

bool conditionIsTrue(const Token* condition, ProgramMemory pm, const Settings& settings, const ProgramMemory::Map& vars)
{
    return evaluateCondition(1, condition, pm, settings, vars);
}

static bool frontIs(const std::vector<MathLib::bigint>& v, bool i)
{
    if (v.empty())
        return false;
    if (v.front())
        return i;
    return !i;
}

static bool isTrueOrFalse(const ValueFlow::Value& v, bool b)
{
    if (b)
        return isTrue(v);
    return isFalse(v);
}

// Is the expression with these values known to be nonzero? It is when its value is, or when one of
// its constraints excludes zero.
static bool isTrue(const ProgramMemory::Values& values)
{
    return std::any_of(values.cbegin(), values.cend(), [](const ValueFlow::Value& v) {
        return isTrue(v);
    });
}

static bool isFalse(const ProgramMemory::Values& values)
{
    return values.size() == 1 && isFalse(values.front());
}

static bool isTrueOrFalse(const ProgramMemory::Values& values, bool b)
{
    return b ? isTrue(values) : isFalse(values);
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

// findChanged: optional cached findExpressionChanged (see ProgramMemoryState::FindChangedFn).
static void programMemoryParseCondition(ProgramMemory& pm,
                                        const Token* tok,
                                        const Token* endTok,
                                        const Settings& settings,
                                        bool then,
                                        const ProgramMemoryState::FindChangedFn& findChanged = {})
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
    // Use the cached closure if given, else compute directly.
    auto changed = [&](const Token* e, const Token* s, const Token* en) -> const Token* {
        return findChanged ? findChanged(e, s, en) : findExpressionChanged(e, s, en, settings);
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
        if (endTok && changed(vartok, tok->next(), endTok))
            return;
        const bool impossible = (tok->str() == "==" && !then) || (tok->str() == "!=" && then);
        ValueFlow::Value& v = then ? truevalue : falsevalue;
        // A value with a bound is a range: record it as the range of impossible values so that it
        // constrains the expression instead of standing in for its value.
        if (impossible || v.bound != ValueFlow::Value::Bound::Point)
            v = asImpossible(std::move(v));
        pm.setValue(vartok, v);
        const Token* containerTok = settings.library.getContainerFromYield(vartok, Library::Container::Yield::SIZE);
        if (containerTok) {
            v.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
            pm.setValue(containerTok, v);
        }
    } else if (Token::simpleMatch(tok, "!")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, !then, findChanged);
    } else if (then && Token::simpleMatch(tok, "&&")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then, findChanged);
        programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then, findChanged);
    } else if (!then && Token::simpleMatch(tok, "||")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then, findChanged);
        programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then, findChanged);
    } else if (Token::Match(tok, "&&|%oror%")) {
        std::vector<MathLib::bigint> lhs = eval(tok->astOperand1());
        std::vector<MathLib::bigint> rhs = eval(tok->astOperand2());
        if (lhs.empty() || rhs.empty()) {
            if (frontIs(lhs, !then))
                programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then, findChanged);
            else if (frontIs(rhs, !then))
                programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then, findChanged);
            else
                pm.setIntValue(tok, 0, then);
        }
    } else if (tok && tok->exprId() > 0) {
        if (endTok && changed(tok, tok->next(), endTok))
            return;
        pm.setIntValue(tok, 0, then);
        const Token* containerTok = settings.library.getContainerFromYield(tok, Library::Container::Yield::EMPTY);
        if (containerTok)
            pm.setContainerSizeValue(containerTok, 0, then);
    }
}

static void fillProgramMemoryFromConditions(ProgramMemory& pm,
                                            const Scope* scope,
                                            const Token* endTok,
                                            const Settings& settings,
                                            const ProgramMemoryState::FindChangedFn& findChanged)
{
    if (!scope)
        return;
    if (!scope->isLocal())
        return;
    assert(scope != scope->nestedIn);
    fillProgramMemoryFromConditions(pm, scope->nestedIn, endTok, settings, findChanged);
    if (scope->type == ScopeType::eIf || scope->type == ScopeType::eWhile || scope->type == ScopeType::eElse || scope->type == ScopeType::eFor) {
        const Token* condTok = getCondTokFromEnd(scope->bodyEnd);
        if (!condTok)
            return;
        MathLib::bigint result = 0;
        bool error = false;
        execute(condTok, pm, &result, &error, settings);
        if (error)
            programMemoryParseCondition(pm, condTok, endTok, settings, scope->type != ScopeType::eElse, findChanged);
    }
}

static void fillProgramMemoryFromConditions(ProgramMemory& pm,
                                            const Token* tok,
                                            const Settings& settings,
                                            const ProgramMemoryState::FindChangedFn& findChanged = {})
{
    fillProgramMemoryFromConditions(pm, tok->scope(), tok, settings, findChanged);
}

static void fillProgramMemoryFromAssignments(ProgramMemory& pm, const Token* tok, const Settings& settings, const ProgramMemory& state, const ProgramMemory::Map& vars)
{
    int indentlevel = 0;
    for (const Token *tok2 = tok; tok2; tok2 = tok2->previous()) {
        if ((Token::simpleMatch(tok2, "=") || Token::Match(tok2->previous(), "%var% (|{")) && tok2->astOperand1() &&
            tok2->astOperand2()) {
            const Token* vartok = tok2->astOperand1();
            if (!pm.hasValue(vartok->exprId())) {
                const Token* valuetok = tok2->astOperand2();
                ProgramMemory local = state;
                // Tracked values are substituted by execute() when the expression is evaluated.
                const ProgramMemory::Values values = executeValues(valuetok, local, settings, vars);
                if (values.empty())
                    pm.setUnknown(vartok);
                else
                    pm.setValues(vartok, values);
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

ProgramMemoryState::ProgramMemoryState(const Settings& s) : settings(s), changedCache(std::make_shared<ChangedCache>())
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
    for (const auto& p:vars)
        pm.setValues(p.first.tok, p.second);
}

void ProgramMemoryState::addState(const Token* tok, const ProgramMemory::Map& vars)
{
    ProgramMemory local = state;
    addVars(local, vars);
    fillProgramMemoryFromConditions(local, tok, settings, getCachedFindExpressionChanged(/*skipDeadCode*/ false));
    ProgramMemory pm;
    fillProgramMemoryFromAssignments(pm, tok, settings, local, vars);
    local.replace(std::move(pm));
    addVars(local, vars);
    replace(std::move(local), tok);
}

void ProgramMemoryState::assume(const Token* tok, bool b, bool isEmpty, const Token* origin)
{
    ProgramMemory pm = state;
    if (isEmpty)
        pm.setContainerSizeValue(tok, 0, b);
    else
        programMemoryParseCondition(pm, tok, nullptr, settings, b);
    if (!origin) {
        origin = tok;
        const Token* top = tok->astTop();
        if (Token::Match(top->previous(), "for|while|if (") && !Token::simpleMatch(tok->astParent(), "?")) {
            origin = top->link()->next();
            if (!b && origin->link()) {
                origin = origin->link();
            }
        }
    }
    replace(std::move(pm), origin);
}

ProgramMemoryState::FindChangedFn ProgramMemoryState::getCachedFindExpressionChanged(bool skipDeadCode) const
{
    // Structural findExpressionChanged() is pure, so memoize it in changedCache (never invalidated).
    // skipDeadCode adds the dead-code walk; it evaluates guards against a fixed state snapshot (so every
    // variable follows the same path) and memoizes those evals in evalCache for the closure's lifetime.
    using EvalCache = std::map<const Token*, std::vector<MathLib::bigint>>;
    const std::shared_ptr<ChangedCache> cache = changedCache;
    const Settings* const sp = &settings;
    ProgramMemory snapshot = state;
    const std::shared_ptr<EvalCache> evalCache = skipDeadCode ? std::make_shared<EvalCache>() : nullptr;
    return [cache, sp, snapshot, skipDeadCode, evalCache](const Token* expr,
                                                          const Token* start,
                                                          const Token* end) -> const Token* {
        const auto key = std::make_tuple(expr, start, end);
        const auto it = cache->find(key);
        const Token* modified = (it != cache->end())
                                    ? it->second
                                    : cache->emplace(key, findExpressionChanged(expr, start, end, *sp)).first->second;
        if (!skipDeadCode || !modified)
            return modified;
        auto eval = [&](const Token* cond) -> std::vector<MathLib::bigint> {
            const auto cit = evalCache->find(cond);
            if (cit != evalCache->end())
                return cit->second;
            ProgramMemory pm2 = snapshot;
            const auto result = execute(cond, pm2, *sp);
            std::vector<MathLib::bigint> r;
            if (isTrue(result))
                r = {1};
            else if (isFalse(result))
                r = {0};
            return evalCache->emplace(cond, std::move(r)).first->second;
        };
        return findExpressionChangedSkipDeadCode(expr, start, end, *sp, eval);
    };
}

void ProgramMemoryState::removeModifiedVars(const Token* tok)
{
    const auto findChanged = getCachedFindExpressionChanged(/*skipDeadCode*/ true);
    state.erase_if([&](const ExprIdToken& e) {
        const Token* start = origins[e.getExpressionId()];
        const Token* expr = e.tok;
        const bool changed = !expr || findChanged(expr, start, tok);
        if (changed)
            origins.erase(e.getExpressionId());
        return changed;
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
    fillProgramMemoryFromAssignments(programMemory, tok, settings, state, {{expr, {value}}});
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

static bool isBounded(const ValueFlow::Value& value)
{
    return value.bound != ValueFlow::Value::Bound::Point;
}

static bool multiplyOverflows(MathLib::bigint x, MathLib::bigint y)
{
    if (x == 0 || y == 0)
        return false;
    if (ValueFlow::isSaturated(x) || ValueFlow::isSaturated(y))
        return true;
    return std::abs(x) > std::numeric_limits<MathLib::bigint>::max() / std::abs(y);
}

// The operations that keep the order of the values of a range
static bool isMonotone(const std::string& op)
{
    return contains({"+", "-", "*", "/", "<<", ">>"}, op);
}

// The range of "x <op> k" (or "k <op> x") when the values of x up to (or from) the bound are
// impossible: the end of the range is transformed; an operation that reverses the order of the
// values turns the range around.
static ValueFlow::Value applyToRange(const std::string& op, const ValueFlow::Value& range, MathLib::bigint k, bool rangeIsLhs)
{
    const MathLib::bigint edge = range.rangeEdge();
    if (ValueFlow::isSaturated(edge) || ValueFlow::isSaturated(k))
        return ValueFlow::Value::unknown();
    bool increasing = true;
    MathLib::bigint result = 0;
    if (op == "+") {
        result = edge + k;
    } else if (op == "-") {
        increasing = rangeIsLhs;
        result = rangeIsLhs ? edge - k : k - edge;
    } else if (op == "*") {
        if (k == 0 || multiplyOverflows(edge, k))
            return ValueFlow::Value::unknown();
        increasing = k > 0;
        result = edge * k;
    } else if (op == "/") {
        if (k == 0 || !rangeIsLhs)
            return ValueFlow::Value::unknown();
        // Truncation towards zero keeps the order of the values
        increasing = k > 0;
        result = edge / k;
    } else {
        // Shifts: calculate() rejects a negative or too large shift and a negative value
        bool error = false;
        result = calculate(op, edge, k, &error);
        if (!rangeIsLhs || error || (op == "<<" && (result >> k) != edge))
            return ValueFlow::Value::unknown();
    }
    ValueFlow::Value scaled = range;
    scaled.setRangeEdge(result, range.isLowerEdge() == increasing);
    return scaled;
}

static ValueFlow::Value evaluate(const Token* op, const ValueFlow::Value& lhs, const ValueFlow::Value& rhs, bool removeAssign = false)
{
    const std::string opStr = removeAssign ? op->str().substr(0, op->str().size() - 1) : op->str();
    ValueFlow::Value result;
    if (lhs.isImpossible() && rhs.isImpossible())
        return ValueFlow::Value::unknown();
    // An impossible range and an int: the range of the result, for the operations that keep the order
    const bool rangeIsLhs = lhs.isImpossible() && isBounded(lhs);
    const ValueFlow::Value& range = rangeIsLhs ? lhs : rhs;
    const ValueFlow::Value& k = rangeIsLhs ? rhs : lhs;
    if (range.isImpossible() && isBounded(range) && range.isIntValue() && !k.isImpossible() && !isBounded(k) &&
        k.isIntValue() && isMonotone(opStr))
        return applyToRange(opStr, range, k.intvalue, rangeIsLhs);
    if (lhs.isImpossible() || rhs.isImpossible()) {
        // The image of an impossible value is impossible only for an injective operation
        if (contains({"%", "/", "&", "|", ">>"}, opStr))
            return ValueFlow::Value::unknown();
        const ValueFlow::Value& factor = lhs.isImpossible() ? rhs : lhs;
        if (opStr == "*" && factor.equalTo(0))
            return ValueFlow::Value::unknown();
        result.setImpossible();
    }
    if (isNumericValue(lhs) && isNumericValue(rhs)) {
        if (lhs.isFloatValue() || rhs.isFloatValue()) {
            result.valueType = op->isArithmeticalOp() ? ValueFlow::Value::ValueType::FLOAT : ValueFlow::Value::ValueType::INT;
            bool error = false;
            result.floatValue = calculate(opStr, asFloat(lhs), asFloat(rhs), &error);
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
    const bool compareOp = op->isComparisonOp();
    // Comparison must be the same type
    if (compareOp && lhs.valueType != rhs.valueType)
        return ValueFlow::Value::unknown();
    // Only add, subtract, and compare for non-integers
    if (!compareOp && !contains({"+", "-"}, opStr) && !lhs.isIntValue() && !rhs.isIntValue())
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
    result.intvalue = calculate(opStr, lhs.intvalue, rhs.intvalue, &error);
    if (error)
        return ValueFlow::Value::unknown();
    if (result.isImpossible() && opStr == "!=") {
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
        using Values = ProgramMemory::Values;

        ProgramMemory* pm;
        const Settings& settings;
        // Values tracked by the forward/reverse analysis. A tracked value is the authoritative
        // current value of its expression and takes precedence over the program memory.
        const ProgramMemory::Map* vars = nullptr;
        int fdepth = 4;
        int depth = 10;

        Executor(ProgramMemory* pm, const Settings& settings) : pm(pm), settings(settings)
        {
            assert(pm != nullptr);
        }

        // The tracked values for this expression, if there are any
        const Values* getTrackedValues(const Token* expr) const
        {
            if (!vars || expr->exprId() == 0)
                return nullptr;
            const auto it = vars->find(ExprIdToken::create(expr->exprId()));
            return it == vars->end() ? nullptr : &it->second;
        }

        // Does the expression read a tracked value? If so, any value cached for it may be stale
        // (the tracked value may have changed since), so it must be re-evaluated, not served cached.
        bool dependsOnTrackedValue(const Token* expr) const
        {
            if (!vars || vars->empty())
                return false;
            return findAstNode(expr, [&](const Token* tok) {
                return getTrackedValues(tok) != nullptr;
            }) != nullptr;
        }

        static ValueFlow::Value unknown() {
            return ValueFlow::Value::unknown();
        }

        // The one value to read for an expression: its value, or the first of its constraints (every
        // one of them holds for the expression)
        static ValueFlow::Value representative(const Values& values)
        {
            return values.empty() ? unknown() : values.front();
        }

        // The values of an expression that has the one value; none when it is unknown
        static Values single(ValueFlow::Value value)
        {
            Values values;
            if (!value.isUninitValue())
                values.push_back(std::move(value));
            return values;
        }

        // The value of the expression when it has exactly one, and not constraints
        static const ValueFlow::Value* getSingleValue(const Values& values)
        {
            if (values.size() != 1 || values.front().isImpossible())
                return nullptr;
            return &values.front();
        }

        // The value of a condition: constraints that exclude zero are true, a value stands for itself
        static ValueFlow::Value conditionValue(const Values& values)
        {
            if (!values.empty() && values.front().isImpossible() && isTrue(values))
                return ValueFlow::Value{1};
            return representative(values);
        }

        // The values recorded for the expression, when it is read from the program memory: it has no
        // known value and does not depend on a tracked value
        const Values* getStoredValues(const Token* expr) const
        {
            if (expr->exprId() == 0)
                return nullptr;
            const Values* stored = pm->getValues(expr->exprId());
            if (!stored || expr->hasKnownIntValue() || dependsOnTrackedValue(expr))
                return nullptr;
            return stored;
        }

        std::unordered_map<nonneg int, ValueFlow::Value> executeAll(const std::vector<const Token*>& toks,
                                                                    const bool* b = nullptr) const
        {
            std::unordered_map<nonneg int, ValueFlow::Value> result;
            auto state = *this;
            for (const Token* tok : toks) {
                const Values r = state.execute(tok);
                if (r.empty())
                    continue;
                const bool brk = b && isTrueOrFalse(r, *b);
                result.emplace(tok->exprId(), conditionValue(r));
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
            std::sort(conditions.begin(), conditions.end(), &TokenExprIdCompare);
            conditions.erase(std::unique(conditions.begin(), conditions.end(), &TokenExprIdCompare), conditions.end());
            return !conditions.empty() && conditions.front()->exprId() != 0;
        }

        ValueFlow::Value executeMultiCondition(bool b, const Token* expr)
        {
            if (const ValueFlow::Value* v = pm->getValue(expr->exprId(), /*impossible*/ true)) {
                if (v->isIntValue())
                    return *v;
            }

            // Evaluate recursively if there are no exprids
            if ((expr->astOperand1() && expr->astOperand1()->exprId() == 0) ||
                (expr->astOperand2() && expr->astOperand2()->exprId() == 0)) {
                const Values lhs = execute(expr->astOperand1());
                if (isTrueOrFalse(lhs, b))
                    return conditionValue(lhs);
                const Values rhs = execute(expr->astOperand2());
                if (isTrueOrFalse(rhs, b))
                    return conditionValue(rhs);
                if (isTrueOrFalse(lhs, !b) && isTrueOrFalse(rhs, !b))
                    return conditionValue(lhs);
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
                if (p.second.size() != 1)
                    continue;
                const ValueFlow::Value& value = p.second.front();

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

        // Get the size values of the container. If the container itself is not tracked in the
        // program memory then check if it is symbolically equal to a container whose size is tracked.
        Values executeContainerSizes(const Token* containerTok)
        {
            Values sizes = execute(containerTok);
            sizes.remove_if([](const ValueFlow::Value& v) {
                return !v.isContainerSizeValue();
            });
            if (!sizes.empty())
                return sizes;
            for (const ValueFlow::Value& value : containerTok->values()) {
                if (!value.isSymbolicValue())
                    continue;
                if (value.isImpossible())
                    continue;
                if (value.intvalue != 0)
                    continue;
                if (!value.tokvalue)
                    continue;
                if (value.tokvalue->exprId() == 0)
                    continue;
                const ValueFlow::Value* sizeValue = pm->getValue(value.tokvalue->exprId());
                if (sizeValue && sizeValue->isContainerSizeValue()) {
                    sizes.push_back(*sizeValue);
                    break;
                }
            }
            return sizes;
        }

        // The size values of the container, as ints
        Values executeSizeYield(const Token* containerTok)
        {
            Values sizes = executeContainerSizes(containerTok);
            for (ValueFlow::Value& v : sizes)
                v.valueType = ValueFlow::Value::ValueType::INT;
            return sizes;
        }

        // The values of the expression: its one value, or the constraints it is known to satisfy
        Values executeImpl(const Token* expr)
        {
            const ValueFlow::Value* value = nullptr;
            if (expr->hasKnownIntValue() && !expr->isAssignmentOp() && expr->str() != ",")
                return single(*expr->getKnownValue(ValueFlow::Value::ValueType::INT));
            if ((value = expr->getKnownValue(ValueFlow::Value::ValueType::FLOAT)) ||
                (value = expr->getKnownValue(ValueFlow::Value::ValueType::TOK)) ||
                (value = expr->getKnownValue(ValueFlow::Value::ValueType::ITERATOR_START)) ||
                (value = expr->getKnownValue(ValueFlow::Value::ValueType::ITERATOR_END)) ||
                (value = expr->getKnownValue(ValueFlow::Value::ValueType::CONTAINER_SIZE))) {
                return single(*value);
            }
            if (expr->isNumber()) {
                if (MathLib::isFloat(expr->str()))
                    return {};
                MathLib::bigint i = MathLib::toBigNumber(expr);
                if (i < 0 && astIsUnsigned(expr))
                    return {};
                return single(ValueFlow::Value{i});
            }
            if (expr->isBoolean())
                return single(ValueFlow::Value{expr->str() == "true"});
            if (const Token* containerTok = settings.library.getContainerFromYield(expr, Library::Container::Yield::SIZE)) {
                return executeSizeYield(containerTok);
            }
            if (const Token* containerTok = settings.library.getContainerFromYield(expr, Library::Container::Yield::EMPTY)) {
                const ValueFlow::Value v = containerEmptyValue(executeContainerSizes(containerTok));
                if (!v.isUninitValue())
                    return single(v);
            } else if (expr->isAssignmentOp() && expr->astOperand1() && expr->astOperand2() &&
                       expr->astOperand1()->exprId() > 0) {
                Values rhs = execute(expr->astOperand2());
                if (rhs.empty())
                    return {};
                if (expr->str() != "=") {
                    if (!pm->hasValue(expr->astOperand1()->exprId()))
                        return {};
                    // Constraints of the right hand side cannot be combined with the values
                    const ValueFlow::Value& delta = rhs.front();
                    Values& lhs = pm->at(expr->astOperand1()->exprId());
                    for (ValueFlow::Value& v : lhs) {
                        const ValueFlow::Value r = evaluate(expr, v, delta, /*removeAssign*/ true);
                        if (r.isUninitValue()) {
                            lhs.assign(1, unknown());
                            return {};
                        }
                        if (v.isIntValue())
                            ValueFlow::Value::visitValue(r, std::bind(assign{}, std::ref(v.intvalue), std::placeholders::_1));
                        else if (v.isFloatValue())
                            ValueFlow::Value::visitValue(r, std::bind(assign{}, std::ref(v.floatValue), std::placeholders::_1));
                        else
                            return {};
                        // The operation may have turned the range around or dissolved it
                        v.bound = r.bound;
                    }
                    return lhs;
                }
                pm->setValues(expr->astOperand1(), rhs);
                return rhs;
            } else if (expr->str() == "&&" && expr->astOperand1() && expr->astOperand2()) {
                return single(executeMultiCondition(false, expr));
            } else if (expr->str() == "||" && expr->astOperand1() && expr->astOperand2()) {
                return single(executeMultiCondition(true, expr));
            } else if (expr->str() == "," && expr->astOperand1() && expr->astOperand2()) {
                execute(expr->astOperand1());
                return execute(expr->astOperand2());
            } else if (expr->tokType() == Token::eIncDecOp && expr->astOperand1() && expr->astOperand1()->exprId() != 0) {
                if (!pm->hasValue(expr->astOperand1()->exprId()))
                    return {};
                Values& lhs = pm->at(expr->astOperand1()->exprId());
                // The values of an expression all have the same type
                if (!lhs.front().isIntValue())
                    return {};
                // An unsigned value wraps around when it is decremented and may be zero
                if (expr->str() == "--" && astIsUnsigned(expr->astOperand1()) && !isTrue(lhs)) {
                    lhs.assign(1, unknown());
                    return {};
                }

                // Shift every value of the variable; bounds and impossible values move along
                for (ValueFlow::Value& v : lhs) {
                    if (expr->str() == "++")
                        v.intvalue++;
                    else
                        v.intvalue--;
                }
                return lhs;
            } else if (expr->str() == "[" && expr->astOperand1() && expr->astOperand2()) {
                const Token* tokvalue = nullptr;
                if (!pm->getTokValue(expr->astOperand1()->exprId(), tokvalue)) {
                    auto tokvalue_it = std::find_if(expr->astOperand1()->values().cbegin(),
                                                    expr->astOperand1()->values().cend(),
                                                    std::mem_fn(&ValueFlow::Value::isTokValue));
                    if (tokvalue_it == expr->astOperand1()->values().cend() || !tokvalue_it->isKnown()) {
                        return {};
                    }
                    tokvalue = tokvalue_it->tokvalue;
                }
                if (!tokvalue || !tokvalue->isLiteral()) {
                    return {};
                }
                const std::string strValue = tokvalue->strValue();
                const Values rhs = execute(expr->astOperand2());
                const ValueFlow::Value* index = getSingleValue(rhs);
                if (!index || !index->isIntValue())
                    return {};
                if (index->intvalue >= 0 && index->intvalue < strValue.size())
                    return single(ValueFlow::Value{strValue[index->intvalue]});
                if (index->intvalue == strValue.size())
                    return single(ValueFlow::Value{});
            } else if (Token::Match(expr, "%cop%") && expr->astOperand1() && expr->astOperand2()) {
                Values lhsValues = execute(expr->astOperand1());
                if (lhsValues.empty())
                    return {};
                Values rhsValues = execute(expr->astOperand2());
                if (rhsValues.empty())
                    return {};
                // The values of an operand are either one value or all constraints
                const bool lhsConstraints = lhsValues.front().isImpossible();
                const bool rhsConstraints = rhsValues.front().isImpossible();
                if (!expr->isComparisonOp() && lhsConstraints != rhsConstraints) {
                    // Apply the operation to each constraint; a constraint it cannot transform is dropped
                    const Values& constraints = lhsConstraints ? lhsValues : rhsValues;
                    const ValueFlow::Value& other = lhsConstraints ? rhsValues.front() : lhsValues.front();
                    Values result;
                    for (const ValueFlow::Value& constraint : constraints) {
                        ValueFlow::Value r = lhsConstraints ? evaluate(expr, constraint, other) : evaluate(expr, other, constraint);
                        if (!r.isUninitValue())
                            result.push_back(std::move(r));
                    }
                    return result;
                }
                ValueFlow::Value lhs = lhsValues.front();
                ValueFlow::Value rhs = rhsValues.front();
                // Compare ranges: an operand with constraints is compared as the interval they describe
                if (expr->isComparisonOp() && (lhsConstraints || rhsConstraints)) {
                    std::vector<ValueFlow::Value> result =
                        infer(makeIntegralInferModel(), expr->str(), std::move(lhsValues), std::move(rhsValues));
                    if (!result.empty())
                        return single(std::move(result.front()));
                }
                ValueFlow::Value r = evaluate(expr, lhs, rhs);
                if (expr->isComparisonOp() && (r.isUninitValue() || r.isImpossible())) {
                    if (rhs.isIntValue() && !expr->astOperand1()->values().empty()) {
                        std::vector<ValueFlow::Value> result = infer(makeIntegralInferModel(),
                                                                     expr->str(),
                                                                     expr->astOperand1()->values(),
                                                                     {std::move(rhs)});
                        if (!result.empty() && result.front().isKnown())
                            return single(std::move(result.front()));
                    }
                    if (lhs.isIntValue() && !expr->astOperand2()->values().empty()) {
                        std::vector<ValueFlow::Value> result = infer(makeIntegralInferModel(),
                                                                     expr->str(),
                                                                     {std::move(lhs)},
                                                                     expr->astOperand2()->values());
                        if (!result.empty() && result.front().isKnown())
                            return single(std::move(result.front()));
                    }
                    return {};
                }
                return single(std::move(r));
            }
            // Unary ops
            else if (Token::Match(expr, "!|+|-") && expr->astOperand1() && !expr->astOperand2()) {
                Values lhs = execute(expr->astOperand1());
                if (lhs.empty() || !lhs.front().isIntValue())
                    return {};
                if (expr->str() == "!") {
                    ValueFlow::Value result = lhs.front();
                    if (isTrue(lhs)) {
                        result.intvalue = 0;
                    } else if (isFalse(lhs)) {
                        result.intvalue = 1;
                    } else {
                        return {};
                    }
                    result.setPossible();
                    result.bound = ValueFlow::Value::Bound::Point;
                    return single(std::move(result));
                }
                if (expr->str() == "-") {
                    for (ValueFlow::Value& v : lhs) {
                        v.intvalue = -v.intvalue;
                        v.invertBound();
                    }
                }
                return lhs;
            } else if (expr->str() == "?" && expr->astOperand1() && expr->astOperand2()) {
                const Values cond = execute(expr->astOperand1());
                if (cond.empty() || !cond.front().isIntValue())
                    return {};
                const Token* child = expr->astOperand2();
                if (isFalse(cond))
                    return execute(child->astOperand2());
                if (isTrue(cond))
                    return execute(child->astOperand1());

                return {};
            } else if (expr->str() == "(" && expr->isCast()) {
                if (expr->astOperand2()) {
                    if (expr->astOperand1()->str() != "dynamic_cast")
                        return execute(expr->astOperand2());
                    return {};
                }
                return execute(expr->astOperand1());
            }
            // Return the tracked values and write them back when they differ, so later reads see the
            // same values (as fillProgramMemoryFromAssignments used to do).
            if (const Values* tracked = getTrackedValues(expr)) {
                const Values* stored = pm->getValues(expr->exprId());
                if (!stored || *stored != *tracked)
                    pm->setValues(expr, *tracked);
                return *tracked;
            }
            if (const Values* stored = getStoredValues(expr)) {
                // An impossible value that excludes zero makes the expression true as a bool
                if (std::any_of(stored->cbegin(), stored->cend(), [](const ValueFlow::Value& v) {
                    return v.isImpossible() && v.isIntValue() && isTrue(v);
                }) && isUsedAsBool(expr, settings)) {
                    ValueFlow::Value result{1};
                    result.setKnown();
                    return single(std::move(result));
                }
                return *stored;
            }

            if (Token::Match(expr->previous(), ">|%name% {|(")) {
                const Token* ftok = expr->previous();
                const Function* f = ftok->function();
                Values result;
                if (expr->str() == "(") {
                    const std::vector<const Token*> tokArgs = getArguments(expr);
                    std::vector<Values> args;
                    args.reserve(tokArgs.size());
                    std::transform(tokArgs.cbegin(), tokArgs.cend(), std::back_inserter(args), [&](const Token* tok) {
                        return execute(tok);
                    });
                    if (f) {
                        if (fdepth >= 0 && !f->isImplicitlyVirtual()) {
                            ProgramMemory functionState;
                            for (std::size_t i = 0; i < args.size(); ++i) {
                                const Variable* const arg = f->getArgumentVar(i);
                                if (!arg)
                                    return {};
                                functionState.setValues(arg->nameToken(), args[i]);
                            }
                            Executor ex = *this;
                            ex.pm = &functionState;
                            ex.fdepth--;
                            const std::vector<ValueFlow::Value> returned = ex.execute(f->functionScope);
                            std::copy_if(returned.cbegin(), returned.cend(), std::back_inserter(result), [](const ValueFlow::Value& v) {
                                return !v.isUninitValue();
                            });
                            // TODO: Track values changed by reference
                        }
                    } else {
                        if (BuiltinLibraryFunction lf = getBuiltinLibraryFunction(ftok->str())) {
                            // The builtin functions compute with values, not with constraints
                            std::vector<ValueFlow::Value> argValues;
                            argValues.reserve(args.size());
                            for (const Values& a : args) {
                                const ValueFlow::Value* v = getSingleValue(a);
                                if (!v)
                                    return {};
                                argValues.push_back(*v);
                            }
                            return single(lf(argValues));
                        }
                        const std::string& returnValue = settings.library.returnValue(ftok);
                        if (!returnValue.empty()) {
                            std::unordered_map<nonneg int, ValueFlow::Value> arg_map;
                            int argn = 0;
                            for (const Values& a : args) {
                                if (!a.empty())
                                    arg_map[argn] = a.front();
                                argn++;
                            }
                            return single(evaluateLibraryFunction(arg_map, returnValue, settings, ftok->isCpp()));
                        }
                    }
                }
                // Check if function modifies argument
                visitAstNodes(expr->astOperand2(), [&](const Token* child) {
                    const Values* values = child->exprId() > 0 ? pm->getValues(child->exprId()) : nullptr;
                    if (values) {
                        // The values of an expression all have the same type
                        const ValueFlow::Value& v = values->front();
                        if (v.valueType == ValueFlow::Value::ValueType::CONTAINER_SIZE) {
                            if (ValueFlow::isContainerSizeChanged(child, v.indirect, settings))
                                pm->setUnknown(child);
                        } else if (v.valueType != ValueFlow::Value::ValueType::UNINIT) {
                            if (isVariableChanged(child, v.indirect, settings))
                                pm->setUnknown(child);
                        }
                    }
                    return ChildrenToVisit::op1_and_op2;
                });
                return result;
            }

            return {};
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

        // The values of the expression. When it does not evaluate to a value, the program memory and
        // the values of the token may still constrain it.
        Values execute(const Token* expr)
        {
            depth--;
            OnExit onExit{[&] {
                    depth++;
                }};
            if (depth < 0 || !expr)
                return {};
            Values values = executeImpl(expr);
            if (!values.empty() && !values.front().isImpossible())
                return values;
            if (expr->exprId() > 0) {
                if (const Values* stored = pm->getValues(expr->exprId())) {
                    if (!stored->front().isImpossible())
                        return *stored;
                    if (values.empty())
                        values = *stored;
                }
            }
            // Find symbolic values
            for (const ValueFlow::Value& value : expr->values()) {
                if (!value.isSymbolicValue())
                    continue;
                if (!value.isKnown())
                    continue;
                const Values* stored = pm->getValues(value.tokvalue->exprId());
                if (!stored || (!stored->front().isIntValue() && value.intvalue != 0))
                    continue;
                ValueFlow::Value v2 = stored->front();
                v2.intvalue += value.intvalue;
                return single(std::move(v2));
            }
            if (!values.empty() && values.front().isIntValue())
                return values;
            if (const ValueFlow::Value* impossible = getImpossibleValue(expr))
                return single(*impossible);
            return values;
        }

        std::vector<ValueFlow::Value> execute(const Scope* scope)
        {
            if (!scope)
                return {unknown()};
            if (!scope->bodyStart)
                return {unknown()};
            for (const Token* tok = scope->bodyStart->next(); precedes(tok, scope->bodyEnd); tok = tok->next()) {
                const Token* top = tok->astTop();

                if (Token::simpleMatch(top, "return") && top->astOperand1()) {
                    const Values values = execute(top->astOperand1());
                    if (values.empty())
                        return {unknown()};
                    return std::vector<ValueFlow::Value>(values.cbegin(), values.cend());
                }

                if (Token::Match(top, "%op%")) {
                    if (execute(top).empty())
                        return {unknown()};
                    const Token* next = nextAfterAstRightmostLeaf(top);
                    if (!next)
                        return {unknown()};
                    tok = next;
                } else if (Token::simpleMatch(top->previous(), "if (")) {
                    const Token* condTok = top->astOperand2();
                    const Values cond = execute(condTok);
                    if (cond.empty() || !cond.front().isIntValue())
                        return {unknown()};
                    const Token* thenStart = top->link()->next();
                    const Token* next = thenStart->link();
                    const Token* elseStart = nullptr;
                    if (Token::simpleMatch(thenStart->link(), "} else {")) {
                        elseStart = thenStart->link()->tokAt(2);
                        next = elseStart->link();
                    }
                    std::vector<ValueFlow::Value> result;
                    if (isTrue(cond)) {
                        result = execute(thenStart->scope());
                    } else if (isFalse(cond)) {
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

static ValueFlow::Value execute(const Token* expr,
                                ProgramMemory& pm,
                                const Settings& settings,
                                const ProgramMemory::Map& vars)
{
    Executor ex{&pm, settings};
    ex.vars = &vars;
    return Executor::representative(ex.execute(expr));
}

static ProgramMemory::Values executeValues(const Token* expr,
                                           ProgramMemory& pm,
                                           const Settings& settings,
                                           const ProgramMemory::Map& vars)
{
    Executor ex{&pm, settings};
    ex.vars = &vars;
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
        const std::string code = "return " + returnValue + ";\n";
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
             const Settings& settings,
             const ProgramMemory::Map& vars)
{
    ValueFlow::Value v = execute(expr, programMemory, settings, vars);
    if (!v.isIntValue() || v.isImpossible()) {
        if (error)
            *error = true;
    } else if (result)
        *result = v.intvalue;
}
