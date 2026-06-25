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

#include "infer.h"

#include "calculate.h"
#include "errortypes.h"
#include "token.h"
#include "valueptr.h"
#include "vfvalue.h"

#include <cassert>
#include <algorithm>
#include <functional>
#include <iterator>
#include <unordered_set>
#include <utility>

template<class Predicate, class Compare>
static const ValueFlow::Value* getCompareValue(const std::list<ValueFlow::Value>& values, Predicate pred, Compare compare)
{
    const ValueFlow::Value* result = nullptr;
    for (const ValueFlow::Value& value : values) {
        if (!pred(value))
            continue;
        if (result)
            result = &std::min(value, *result, [compare](const ValueFlow::Value& x, const ValueFlow::Value& y) {
                return compare(x.intvalue, y.intvalue);
            });
        else
            result = &value;
    }
    return result;
}

namespace {
    struct Interval {
        std::vector<MathLib::bigint> minvalue, maxvalue;
        std::vector<const ValueFlow::Value*> minRef, maxRef;

        void setMinValue(MathLib::bigint x, const ValueFlow::Value* ref = nullptr)
        {
            minvalue = {x};
            if (ref)
                minRef = {ref};
        }

        void setMaxValue(MathLib::bigint x, const ValueFlow::Value* ref = nullptr)
        {
            maxvalue = {x};
            if (ref)
                maxRef = {ref};
        }

        bool isLessThan(MathLib::bigint x, std::vector<const ValueFlow::Value*>* ref = nullptr) const
        {
            if (!this->maxvalue.empty() && this->maxvalue.front() < x) {
                if (ref)
                    *ref = maxRef;
                return true;
            }
            return false;
        }

        bool isGreaterThan(MathLib::bigint x, std::vector<const ValueFlow::Value*>* ref = nullptr) const
        {
            if (!this->minvalue.empty() && this->minvalue.front() > x) {
                if (ref)
                    *ref = minRef;
                return true;
            }
            return false;
        }

        bool isScalar() const {
            return minvalue.size() == 1 && minvalue == maxvalue;
        }

        bool empty() const {
            return minvalue.empty() && maxvalue.empty();
        }

        bool isScalarOrEmpty() const {
            return empty() || isScalar();
        }

        MathLib::bigint getScalar() const
        {
            assert(isScalar());
            return minvalue.front();
        }

        std::vector<const ValueFlow::Value*> getScalarRef() const
        {
            assert(isScalar());
            if (minRef != maxRef)
                return merge(minRef, maxRef);
            return minRef;
        }

        static Interval fromInt(MathLib::bigint x, const ValueFlow::Value* ref = nullptr)
        {
            Interval result;
            result.setMinValue(x, ref);
            result.setMaxValue(x, ref);
            return result;
        }

        template<class Predicate>
        static Interval fromValues(const std::list<ValueFlow::Value>& values, Predicate predicate)
        {
            Interval result;
            const ValueFlow::Value* minValue = getCompareValue(values, predicate, std::less<MathLib::bigint>{});
            if (minValue) {
                if (minValue->isImpossible() && minValue->bound == ValueFlow::Value::Bound::Upper)
                    result.setMinValue(minValue->intvalue + 1, minValue);
                if (minValue->isPossible() && minValue->bound == ValueFlow::Value::Bound::Lower)
                    result.setMinValue(minValue->intvalue, minValue);
                if (!minValue->isImpossible() && (minValue->bound == ValueFlow::Value::Bound::Point || minValue->isKnown()) &&
                    std::count_if(values.begin(), values.end(), predicate) == 1)
                    return Interval::fromInt(minValue->intvalue, minValue);
            }
            const ValueFlow::Value* maxValue = getCompareValue(values, predicate, std::greater<MathLib::bigint>{});
            if (maxValue) {
                if (maxValue->isImpossible() && maxValue->bound == ValueFlow::Value::Bound::Lower)
                    result.setMaxValue(maxValue->intvalue - 1, maxValue);
                if (maxValue->isPossible() && maxValue->bound == ValueFlow::Value::Bound::Upper)
                    result.setMaxValue(maxValue->intvalue, maxValue);
                assert(!maxValue->isKnown());
            }
            return result;
        }

        static Interval fromValues(const std::list<ValueFlow::Value>& values)
        {
            return Interval::fromValues(values, [](const ValueFlow::Value&) {
                return true;
            });
        }

        template<class F>
        static std::vector<MathLib::bigint> apply(const std::vector<MathLib::bigint>& x,
                                                  const std::vector<MathLib::bigint>& y,
                                                  F f)
        {
            if (x.empty())
                return {};
            if (y.empty())
                return {};
            return {f(x.front(), y.front())};
        }

        static std::vector<const ValueFlow::Value*> merge(std::vector<const ValueFlow::Value*> x,
                                                          const std::vector<const ValueFlow::Value*>& y)
        {
            x.insert(x.end(), y.cbegin(), y.cend());
            return x;
        }

        friend Interval operator-(const Interval& lhs, const Interval& rhs)
        {
            Interval result;
            result.minvalue = Interval::apply(lhs.minvalue, rhs.maxvalue, std::minus<MathLib::bigint>{});
            result.maxvalue = Interval::apply(lhs.maxvalue, rhs.minvalue, std::minus<MathLib::bigint>{});
            if (!result.minvalue.empty())
                result.minRef = merge(lhs.minRef, rhs.maxRef);
            if (!result.maxvalue.empty())
                result.maxRef = merge(lhs.maxRef, rhs.minRef);
            return result;
        }

        static std::vector<int> equal(const Interval& lhs,
                                      const Interval& rhs,
                                      std::vector<const ValueFlow::Value*>* ref = nullptr)
        {
            if (!lhs.isScalar())
                return {};
            if (!rhs.isScalar())
                return {};
            if (ref)
                *ref = merge(lhs.getScalarRef(), rhs.getScalarRef());
            return {lhs.minvalue == rhs.minvalue};
        }

        static std::vector<int> compare(const Interval& lhs,
                                        const Interval& rhs,
                                        std::vector<const ValueFlow::Value*>* ref = nullptr)
        {
            Interval diff = lhs - rhs;
            if (diff.isGreaterThan(0, ref))
                return {1};
            if (diff.isLessThan(0, ref))
                return {-1};
            std::vector<int> eq = Interval::equal(lhs, rhs, ref);
            if (!eq.empty()) {
                if (eq.front() == 0)
                    return {1, -1};
                return {0};
            }
            if (diff.isGreaterThan(-1, ref))
                return {0, 1};
            if (diff.isLessThan(1, ref))
                return {0, -1};
            return {};
        }

        static std::vector<bool> compare(const std::string& op,
                                         const Interval& lhs,
                                         const Interval& rhs,
                                         std::vector<const ValueFlow::Value*>* ref = nullptr)
        {
            std::vector<int> r = compare(lhs, rhs, ref);
            if (r.empty())
                return {};
            bool b = calculate(op, r.front(), 0);
            if (std::all_of(r.cbegin() + 1, r.cend(), [&](int i) {
                return b == calculate(op, i, 0);
            }))
                return {b};
            return {};
        }
    };
}

static void addToErrorPath(ValueFlow::Value& value, const std::vector<const ValueFlow::Value*>& refs)
{
    std::unordered_set<const Token*> locations;
    for (const ValueFlow::Value* ref : refs) {
        if (ref->condition && !value.condition)
            value.condition = ref->condition;
        std::copy_if(ref->errorPath.cbegin(),
                     ref->errorPath.cend(),
                     std::back_inserter(value.errorPath),
                     [&](const ErrorPathItem& e) {
            return locations.insert(e.first).second;
        });
        std::copy_if(ref->debugPath.cbegin(),
                     ref->debugPath.cend(),
                     std::back_inserter(value.debugPath),
                     [&](const ErrorPathItem& e) {
            return locations.insert(e.first).second;
        });
    }
}

static void setValueKind(ValueFlow::Value& value, const std::vector<const ValueFlow::Value*>& refs)
{
    bool isPossible = false;
    bool isInconclusive = false;
    for (const ValueFlow::Value* ref : refs) {
        if (ref->isPossible())
            isPossible = true;
        if (ref->isInconclusive())
            isInconclusive = true;
    }
    if (isInconclusive)
        value.setInconclusive();
    else if (isPossible)
        value.setPossible();
    else
        value.setKnown();
}

static bool inferNotEqual(const std::list<ValueFlow::Value>& values, MathLib::bigint x)
{
    return std::any_of(values.cbegin(), values.cend(), [&](const ValueFlow::Value& value) {
        return value.isImpossible() && value.intvalue == x;
    });
}

std::vector<ValueFlow::Value> infer(const ValuePtr<InferModel>& model,
                                    const std::string& op,
                                    std::list<ValueFlow::Value> lhsValues,
                                    std::list<ValueFlow::Value> rhsValues)
{
    std::vector<ValueFlow::Value> result;
    auto notMatch = [&](const ValueFlow::Value& value) {
        return !model->match(value);
    };
    lhsValues.remove_if(notMatch);
    if (lhsValues.empty())
        return result;
    rhsValues.remove_if(notMatch);
    if (rhsValues.empty())
        return result;

    Interval lhs = Interval::fromValues(lhsValues);
    Interval rhs = Interval::fromValues(rhsValues);

    if (op == "-") {
        Interval diff = lhs - rhs;
        if (diff.isScalar()) {
            std::vector<const ValueFlow::Value*> refs = diff.getScalarRef();
            ValueFlow::Value value(diff.getScalar());
            addToErrorPath(value, refs);
            setValueKind(value, refs);
            result.push_back(std::move(value));
        } else {
            if (!diff.minvalue.empty()) {
                ValueFlow::Value value(diff.minvalue.front() - 1);
                value.setImpossible();
                value.bound = ValueFlow::Value::Bound::Upper;
                addToErrorPath(value, diff.minRef);
                result.push_back(std::move(value));
            }
            if (!diff.maxvalue.empty()) {
                ValueFlow::Value value(diff.maxvalue.front() + 1);
                value.setImpossible();
                value.bound = ValueFlow::Value::Bound::Lower;
                addToErrorPath(value, diff.maxRef);
                result.push_back(std::move(value));
            }
        }
    } else if ((op == "!=" || op == "==") && lhs.isScalarOrEmpty() && rhs.isScalarOrEmpty()) {
        if (lhs.isScalar() && rhs.isScalar()) {
            std::vector<const ValueFlow::Value*> refs = Interval::merge(lhs.getScalarRef(), rhs.getScalarRef());
            ValueFlow::Value value(calculate(op, lhs.getScalar(), rhs.getScalar()));
            addToErrorPath(value, refs);
            setValueKind(value, refs);
            result.push_back(std::move(value));
        } else {
            std::vector<const ValueFlow::Value*> refs;
            if (lhs.isScalar() && inferNotEqual(rhsValues, lhs.getScalar()))
                refs = lhs.getScalarRef();
            else if (rhs.isScalar() && inferNotEqual(lhsValues, rhs.getScalar()))
                refs = rhs.getScalarRef();
            if (!refs.empty()) {
                ValueFlow::Value value(op == "!=");
                addToErrorPath(value, refs);
                setValueKind(value, refs);
                result.push_back(std::move(value));
            }
        }
    } else {
        std::vector<const ValueFlow::Value*> refs;
        std::vector<bool> r = Interval::compare(op, lhs, rhs, &refs);
        if (!r.empty()) {
            ValueFlow::Value value(static_cast<int>(r.front()));
            addToErrorPath(value, refs);
            setValueKind(value, refs);
            result.push_back(std::move(value));
        }
    }

    return result;
}

std::vector<ValueFlow::Value> infer(const ValuePtr<InferModel>& model,
                                    const std::string& op,
                                    MathLib::bigint lhs,
                                    std::list<ValueFlow::Value> rhsValues)
{
    return infer(model, op, {model->yield(lhs)}, std::move(rhsValues));
}

std::vector<ValueFlow::Value> infer(const ValuePtr<InferModel>& model,
                                    const std::string& op,
                                    std::list<ValueFlow::Value> lhsValues,
                                    MathLib::bigint rhs)
{
    return infer(model, op, std::move(lhsValues), {model->yield(rhs)});
}

std::vector<MathLib::bigint> getMinValue(const ValuePtr<InferModel>& model, const std::list<ValueFlow::Value>& values)
{
    return Interval::fromValues(values, [&](const ValueFlow::Value& v) {
        return model->match(v);
    }).minvalue;
}
std::vector<MathLib::bigint> getMaxValue(const ValuePtr<InferModel>& model, const std::list<ValueFlow::Value>& values)
{
    return Interval::fromValues(values, [&](const ValueFlow::Value& v) {
        return model->match(v);
    }).maxvalue;
}

namespace {
    struct IntegralInferModel : InferModel {
        bool match(const ValueFlow::Value& value) const override {
            return value.isIntValue();
        }
        ValueFlow::Value yield(MathLib::bigint value) const override
        {
            ValueFlow::Value result(value);
            result.valueType = ValueFlow::Value::ValueType::INT;
            result.setKnown();
            return result;
        }
    };
}

ValuePtr<InferModel> makeIntegralInferModel()
{
    return IntegralInferModel{};
}

ValueFlow::Value inferCondition(const std::string& op, const Token* varTok, MathLib::bigint val)
{
    if (!varTok)
        return ValueFlow::Value{};
    if (varTok->hasKnownIntValue())
        return ValueFlow::Value{};
    std::vector<ValueFlow::Value> r = infer(makeIntegralInferModel(), op, varTok->values(), val);
    if (r.size() == 1 && r.front().isKnown())
        return r.front();
    return ValueFlow::Value{};
}
