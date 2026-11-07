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
#include <string>

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
    CPPCHECKLIB bool getMinMaxValues(const ValueType* vt, const Platform& platform, MathLib::bigint& minValue, MathLib::bigint& maxValue);
    CPPCHECKLIB bool isValueRangePreservingConversion(const ValueType* source,
                                                      const ValueType* destination,
                                                      const Settings& settings,
                                                      const Value* range = nullptr);
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
