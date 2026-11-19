/* -*- C++ -*-
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

#ifndef vfCommonH
#define vfCommonH

#include "config.h"
#include "mathlib.h"
#include "sourcelocation.h"
#include "symboldatabase.h"

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

class Token;
class Settings;
class Platform;
class Library;

namespace ValueFlow
{
    class Value;
}

namespace ValueFlow
{
    ValueType::Sign resolveCharSign(const ValueType& type, const Platform& platform);
    // Read an unsigned constant in the type of the signed integral expression it is compared with.
    // Such a comparison converts the expression to unsigned, so the constant's raw value can be
    // outside that type: 0xFFFFFFFFu compared with an 'int' stands for -1. Returns whether the value
    // had to be changed, i.e. whether the raw value was not representable in the operand's type.
    // Whether a comparison of a signed integral operand with an unsigned integral constant is
    // performed in the unsigned domain, i.e. the operand is converted to unsigned first.
    CPPCHECKLIB bool comparesSignedOperandAsUnsigned(const Token* constantTok, const Token* operandTok, const Settings& settings);
    CPPCHECKLIB bool castConstantToOperandType(const Token* constantTok,
                                               const Token* operandTok,
                                               MathLib::bigint& value,
                                               const Settings& settings);
    CPPCHECKLIB bool getMinMaxValues(const ValueType* vt, const Platform& platform, MathLib::bigint& minValue, MathLib::bigint& maxValue);
    CPPCHECKLIB bool isValueRangePreservingConversion(const ValueType* source,
                                                      const ValueType* destination,
                                                      const Settings& settings,
                                                      const Value* range = nullptr);
    // Collect the excluded bounds of a source expression when they together describe an interval that the
    // destination represents exactly. Each of them survives the conversion unchanged.
    CPPCHECKLIB std::vector<Value> getFittedSourceValues(const Token* sourceTok,
                                                         const Value& value,
                                                         const Token* useTok,
                                                         const ValueType* destination,
                                                         const Settings& settings);
    // Transfer an integer range fact through a conversion when its image is soundly representable.
    CPPCHECKLIB bool convertValueRange(Value& value,
                                       const ValueType* source,
                                       const ValueType* destination,
                                       const Settings& settings,
                                       const Token* sourceTok,
                                       const Token* useTok);

    bool isUnsignedArithmeticResult(const Token* tok);
    bool isUnsignedArithmeticNoWrap(const Token* tok, const Settings& settings, bool requireGlobalContext);

    MathLib::bigint truncateIntValue(MathLib::bigint value, size_t value_size, const ValueType* dst, const Platform& platform);

    /** Is the value at a limit of its type, standing for any value beyond? */
    inline bool isSaturated(MathLib::bigint value)
    {
        return value == std::numeric_limits<MathLib::bigint>::max() || value == std::numeric_limits<MathLib::bigint>::min();
    }

    Token * valueFlowSetConstantValue(Token *tok, const Settings &settings);

    CPPCHECKLIB bool castValue(Value& value, ValueType::Sign sign, nonneg int bit);

    std::string debugString(const Value& v);

    void setSourceLocation(Value& v,
                           SourceLocation ctx,
                           const Token* tok,
                           SourceLocation local = SourceLocation::current());

    MathLib::bigint valueFlowGetStrLength(const Token* tok, const Library& library);
}

#endif // vfCommonH
