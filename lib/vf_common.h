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

namespace ValueFlow
{
    class Value;
}

namespace ValueFlow
{
    bool getMinMaxValues(const ValueType* vt, const Platform& platform, MathLib::bigint& minValue, MathLib::bigint& maxValue);

    MathLib::bigint truncateIntValue(MathLib::bigint value, size_t value_size, ValueType::Sign dst_sign);

    Token * valueFlowSetConstantValue(Token *tok, const Settings &settings);

    Value castValue(Value value, ValueType::Sign sign, nonneg int bit);

    std::string debugString(const Value& v);

    void setSourceLocation(Value& v,
                           SourceLocation ctx,
                           const Token* tok,
                           SourceLocation local = SourceLocation::current());

    MathLib::bigint valueFlowGetStrLength(const Token* tok, const Settings& settings);
}

#endif // vfCommonH
