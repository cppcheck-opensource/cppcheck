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

#ifndef inferH
#define inferH

#include "config.h"
#include "mathlib.h"

#include <list>
#include <string>
#include <vector>

class Token;
template<class T> class ValuePtr;

namespace ValueFlow
{
    class Value;
}

struct InferModel {
    virtual bool match(const ValueFlow::Value& value) const = 0;
    virtual ValueFlow::Value yield(MathLib::bigint value) const = 0;
    virtual ~InferModel() = default;
    InferModel(const InferModel&) = default;
protected:
    InferModel() = default;
};

std::vector<ValueFlow::Value> infer(const ValuePtr<InferModel>& model,
                                    const std::string& op,
                                    std::list<ValueFlow::Value> lhsValues,
                                    std::list<ValueFlow::Value> rhsValues);

std::vector<ValueFlow::Value> infer(const ValuePtr<InferModel>& model,
                                    const std::string& op,
                                    MathLib::bigint lhs,
                                    std::list<ValueFlow::Value> rhsValues);

std::vector<ValueFlow::Value> infer(const ValuePtr<InferModel>& model,
                                    const std::string& op,
                                    std::list<ValueFlow::Value> lhsValues,
                                    MathLib::bigint rhs);

CPPCHECKLIB std::vector<MathLib::bigint> getMinValue(const ValuePtr<InferModel>& model, const std::list<ValueFlow::Value>& values);
std::vector<MathLib::bigint> getMaxValue(const ValuePtr<InferModel>& model, const std::list<ValueFlow::Value>& values);

ValuePtr<InferModel> makeIntegralInferModel();

ValueFlow::Value inferCondition(const std::string& op, const Token* varTok, MathLib::bigint val);

#endif
