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

#include <limits>
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
                if (minValue->isImpossible() && minValue->bound == ValueFlow::Value::Bound::Upper &&
                    minValue->intvalue != std::numeric_limits<MathLib::bigint>::max())
                    result.setMinValue(minValue->intvalue + 1, minValue);
                if (minValue->isPossible() && minValue->bound == ValueFlow::Value::Bound::Lower)
                    result.setMinValue(minValue->intvalue, minValue);
                if (!minValue->isImpossible() && (minValue->bound == ValueFlow::Value::Bound::Point || minValue->isKnown()) &&
                    std::count_if(values.begin(), values.end(), predicate) == 1)
                    return Interval::fromInt(minValue->intvalue, minValue);
            }
            const ValueFlow::Value* maxValue = getCompareValue(values, predicate, std::greater<MathLib::bigint>{});
            if (maxValue) {
                if (maxValue->isImpossible() && maxValue->bound == ValueFlow::Value::Bound::Lower &&
                    maxValue->intvalue != std::numeric_limits<MathLib::bigint>::min())
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

static bool addToErrorPath(ValueFlow::Value& value, const std::vector<const ValueFlow::Value*>& refs)
{
    std::unordered_set<const Token*> locations;
    const Token* condition = nullptr;
    MathLib::bigint path = 0;
    nonneg int varId = 0;
    MathLib::bigint varvalue = 0;
    bool conflictingVariable = false;
    for (const ValueFlow::Value* ref : refs) {
        if (ref->path < 0 || (ref->path > 0 && path > 0 && ref->path != path))
            return false;
        if (ref->path > 0)
            path = ref->path;
        if (ref->condition && !condition)
            condition = ref->condition;
        if (ref->varId) {
            if (varId && (varId != ref->varId || varvalue != ref->varvalue))
                conflictingVariable = true;
            else {
                varId = ref->varId;
                varvalue = ref->varvalue;
            }
        }
        value.conditional = value.conditional || ref->conditional;
        value.defaultArg = value.defaultArg || ref->defaultArg;
        value.safe = value.safe || ref->safe;
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
    value.condition = condition;
    value.path = path;
    if (conflictingVariable) {
        value.varId = 0;
        value.varvalue = 0;
    } else {
        value.varId = varId;
        value.varvalue = varvalue;
    }
    return true;
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
    else if (isPossible || value.path > 0)
        value.setPossible();
    else
        value.setKnown();
}

static const ValueFlow::Value* findImpossiblePoint(const ValuePtr<InferModel>& model,
                                                   const std::list<ValueFlow::Value>& values,
                                                   const std::vector<const ValueFlow::Value*>& scalarRefs,
                                                   MathLib::bigint x)
{
    const auto it = std::find_if(values.cbegin(), values.cend(), [&](const ValueFlow::Value& value) {
        if (!model->match(value) || !value.isImpossible() || value.bound != ValueFlow::Value::Bound::Point ||
            value.intvalue != x)
            return false;
        return std::any_of(scalarRefs.cbegin(), scalarRefs.cend(), [&](const ValueFlow::Value* ref) {
            return model->match(*ref) && value.equalValue(*ref);
        });
    });
    return it == values.cend() ? nullptr : &*it;
}

std::vector<ValueFlow::Value> infer(const ValuePtr<InferModel>& model,
                                    const std::string& op,
                                    std::list<ValueFlow::Value> lhsValues,
                                    std::list<ValueFlow::Value> rhsValues)
{
    auto notMatch = [&](const ValueFlow::Value& value) {
        return !model->match(value);
    };
    lhsValues.remove_if(notMatch);
    if (lhsValues.empty())
        return {};
    rhsValues.remove_if(notMatch);
    if (rhsValues.empty())
        return {};

    lhsValues.remove_if([](const ValueFlow::Value& value) {
        return value.path < 0;
    });
    rhsValues.remove_if([](const ValueFlow::Value& value) {
        return value.path < 0;
    });
    if (lhsValues.empty() || rhsValues.empty())
        return {};

    const auto inferIntervals = [&](const std::list<ValueFlow::Value>& lhsValues2,
                                    const std::list<ValueFlow::Value>& rhsValues2) {
        std::vector<ValueFlow::Value> inferred;
        Interval lhs = Interval::fromValues(lhsValues2);
        Interval rhs = Interval::fromValues(rhsValues2);

        const ValueFlow::Value* excludedPoint = nullptr;
        std::vector<const ValueFlow::Value*> excludedRefs;
        if (op == "!=" || op == "==") {
            if (lhs.isScalar()) {
                std::vector<const ValueFlow::Value*> scalarRefs = lhs.getScalarRef();
                excludedPoint = findImpossiblePoint(model, rhsValues2, scalarRefs, lhs.getScalar());
                if (excludedPoint)
                    excludedRefs = std::move(scalarRefs);
            }
            if (!excludedPoint && rhs.isScalar()) {
                std::vector<const ValueFlow::Value*> scalarRefs = rhs.getScalarRef();
                excludedPoint = findImpossiblePoint(model, lhsValues2, scalarRefs, rhs.getScalar());
                if (excludedPoint)
                    excludedRefs = std::move(scalarRefs);
            }
            if (excludedPoint)
                excludedRefs.push_back(excludedPoint);
        }
        if (op == "-") {
            Interval diff = lhs - rhs;
            if (diff.isScalar()) {
                std::vector<const ValueFlow::Value*> refs = diff.getScalarRef();
                ValueFlow::Value value(diff.getScalar());
                if (addToErrorPath(value, refs)) {
                    setValueKind(value, refs);
                    inferred.push_back(std::move(value));
                }
            } else {
                if (!diff.minvalue.empty()) {
                    ValueFlow::Value value(diff.minvalue.front() - 1);
                    value.setImpossible();
                    value.bound = ValueFlow::Value::Bound::Upper;
                    if (addToErrorPath(value, diff.minRef))
                        inferred.push_back(std::move(value));
                }
                if (!diff.maxvalue.empty()) {
                    ValueFlow::Value value(diff.maxvalue.front() + 1);
                    value.setImpossible();
                    value.bound = ValueFlow::Value::Bound::Lower;
                    if (addToErrorPath(value, diff.maxRef))
                        inferred.push_back(std::move(value));
                }
            }
        } else if (excludedPoint) {
            ValueFlow::Value value(op == "!=");
            if (addToErrorPath(value, excludedRefs)) {
                setValueKind(value, excludedRefs);
                inferred.push_back(std::move(value));
            }
        } else if ((op == "!=" || op == "==") && lhs.isScalarOrEmpty() && rhs.isScalarOrEmpty()) {
            if (lhs.isScalar() && rhs.isScalar()) {
                std::vector<const ValueFlow::Value*> refs = Interval::merge(lhs.getScalarRef(), rhs.getScalarRef());
                ValueFlow::Value value(calculate(op, lhs.getScalar(), rhs.getScalar()));
                if (addToErrorPath(value, refs)) {
                    setValueKind(value, refs);
                    inferred.push_back(std::move(value));
                }
            }
        } else {
            std::vector<const ValueFlow::Value*> refs;
            std::vector<bool> r = Interval::compare(op, lhs, rhs, &refs);
            if (!r.empty()) {
                ValueFlow::Value value(static_cast<int>(r.front()));
                if (addToErrorPath(value, refs)) {
                    setValueKind(value, refs);
                    inferred.push_back(std::move(value));
                }
            }
        }
        return inferred;
    };

    std::vector<MathLib::bigint> paths;
    const auto addPath = [&](MathLib::bigint path) {
        if (std::find(paths.cbegin(), paths.cend(), path) == paths.cend())
            paths.push_back(path);
    };
    const auto hasCompatibleValue = [](const std::list<ValueFlow::Value>& values, MathLib::bigint path) {
        return std::any_of(values.cbegin(), values.cend(), [path](const ValueFlow::Value& value) {
            return value.path == 0 || value.path == path;
        });
    };
    if (hasCompatibleValue(lhsValues, 0) && hasCompatibleValue(rhsValues, 0))
        addPath(0);
    for (const ValueFlow::Value& value : lhsValues) {
        if (value.path > 0 && hasCompatibleValue(rhsValues, value.path))
            addPath(value.path);
    }
    for (const ValueFlow::Value& value : rhsValues) {
        if (value.path > 0 && hasCompatibleValue(lhsValues, value.path))
            addPath(value.path);
    }

    std::vector<ValueFlow::Value> result;
    if (paths.size() == 1 && paths.front() == 0)
        return inferIntervals(lhsValues, rhsValues);
    for (MathLib::bigint path : paths) {
        std::list<ValueFlow::Value> lhsCompatible;
        std::list<ValueFlow::Value> rhsCompatible;
        const auto compatible = [path](const ValueFlow::Value& value) {
            return path == 0 ? value.path == 0 : (value.path == 0 || value.path == path);
        };
        std::copy_if(lhsValues.cbegin(), lhsValues.cend(), std::back_inserter(lhsCompatible), compatible);
        std::copy_if(rhsValues.cbegin(), rhsValues.cend(), std::back_inserter(rhsCompatible), compatible);
        std::vector<ValueFlow::Value> inferred = inferIntervals(lhsCompatible, rhsCompatible);
        std::move(inferred.begin(), inferred.end(), std::back_inserter(result));
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

std::vector<MathLib::bigint> getMinValue(const ValuePtr<InferModel>& model, const std::list<ValueFlow::Value>& values, const ValueFlow::Value** minRef)
{
    const Interval interval = Interval::fromValues(values, [&](const ValueFlow::Value& v) {
        return model->match(v);
    });
    if (minRef)
        *minRef = interval.minRef.empty() ? nullptr : interval.minRef.front();
    return interval.minvalue;
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
