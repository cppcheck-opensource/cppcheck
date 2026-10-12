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
#include "vfvalue.h"

#include <cstddef>
#include <limits>
#include <string>
#include <utility>
#include <vector>

class Token;
class Settings;
class Platform;
class Library;

namespace ValueFlow
{
    /** A conversion result: valid when the value was converted, else the conversion failed. */
    struct ConvertedValue {
        ConvertedValue() = default;
        explicit ConvertedValue(Value v) : valid(true), value(std::move(v)) {}

        bool valid = false;
        Value value;
    };

    ValueType::Sign resolveCharSign(const ValueType& type, const Platform& platform);
    /** The bit width of an integral type; 0 when its size is unknown. */
    std::size_t getTypeBits(const ValueType& type, const Settings& settings);
    /** 2^bits - 1, saturating at the carrier width. Requires bits > 0. */
    MathLib::biguint getUnsignedMaximum(std::size_t bits);
    MathLib::biguint convertToUnsigned(MathLib::bigint value, std::size_t bits);
    // Whether a comparison of a signed integral operand with an unsigned integral constant is
    // performed in the unsigned domain, i.e. the operand is converted to unsigned first.
    CPPCHECKLIB bool comparesSignedOperandAsUnsigned(const Token* constantTok, const Token* operandTok, const Settings& settings);
    // Read an unsigned constant in the type of the signed integral expression it is compared with.
    // Returns the value unchanged unless the comparison converts the operand to unsigned and the
    // constant reads differently in the operand's type, e.g. 0xFFFFFFFFu -> -1 for an 'int'.
    CPPCHECKLIB MathLib::bigint castConstantToOperandType(const Token* constantTok,
                                                          const Token* operandTok,
                                                          MathLib::bigint value,
                                                          const Settings& settings);
    CPPCHECKLIB bool getMinMaxValues(const ValueType* vt, const Platform& platform, MathLib::bigint& minValue, MathLib::bigint& maxValue);
    CPPCHECKLIB bool isValueRangePreservingConversion(const ValueType* source,
                                                      const ValueType* destination,
                                                      const Settings& settings,
                                                      const Value& value);
    // Collect the excluded bounds of a source expression when they together describe an interval that the
    // destination represents exactly. Each of them survives the conversion unchanged.
    CPPCHECKLIB std::vector<Value> getFittedSourceValues(const Token* sourceTok,
                                                         const Value& value,
                                                         const Token* useTok,
                                                         const ValueType* destination,
                                                         const Settings& settings);
    // Transfer an integer range fact through a conversion when its image is soundly representable.
    CPPCHECKLIB ConvertedValue convertValueRange(Value value,
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

    CPPCHECKLIB ConvertedValue castValue(Value value, ValueType::Sign sign, nonneg int bit);

    std::string debugString(const Value& v);

    void setSourceLocation(Value& v,
                           SourceLocation ctx,
                           const Token* tok,
                           SourceLocation local = SourceLocation::current());

    MathLib::bigint valueFlowGetStrLength(const Token* tok, const Library& library);
}

#endif // vfCommonH
