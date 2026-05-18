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


//---------------------------------------------------------------------------
#ifndef checktypeH
#define checktypeH
//---------------------------------------------------------------------------

#include "check.h"
#include "checkimpl.h"
#include "config.h"

#include <list>
#include <string>

class ErrorLogger;
class Settings;
class Token;
class ValueType;
class Tokenizer;
namespace ValueFlow
{
    class Value;
}

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB CheckType : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckType() : Check("Type") {}

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings &settings) const override;

    std::string classInfo() const override {
        return "Type checks\n"
               "- bitwise shift by too many bits (only enabled when --platform is used)\n"
               "- signed integer overflow (only enabled when --platform is used)\n"
               "- dangerous sign conversion, when signed value can be negative\n"
               "- possible loss of information when assigning int result to long variable\n"
               "- possible loss of information when returning int result as long return value\n"
               "- float conversion overflow\n";
    }
};

class CPPCHECKLIB CheckTypeImpl : public CheckImpl {
public:
    /** @brief This constructor is used when running checks. */
    CheckTypeImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    /** @brief %Check for bitwise shift with too big right operand */
    void checkTooBigBitwiseShift();

    /** @brief %Check for integer overflow */
    void checkIntegerOverflow();

    /** @brief %Check for dangerous sign conversion */
    void checkSignConversion();

    /** @brief %Check for implicit long cast of int result */
    void checkLongCast();

    /** @brief %Check for float to integer overflow */
    void checkFloatToIntegerOverflow();
    void checkFloatToIntegerOverflow(const Token *tok, const ValueType *vtint, const ValueType *vtfloat, const std::list<ValueFlow::Value> &floatValues);

    // Error messages..
    void tooBigBitwiseShiftError(const Token *tok, int lhsbits, const ValueFlow::Value &rhsbits);
    void tooBigSignedBitwiseShiftError(const Token *tok, int lhsbits, const ValueFlow::Value &rhsbits);
    void integerOverflowError(const Token *tok, const ValueFlow::Value &value, bool isOverflow = true);
    void signConversionError(const Token *tok, const ValueFlow::Value *negativeValue, bool constvalue);
    void longCastAssignError(const Token *tok, const ValueType* src = nullptr, const ValueType* tgt = nullptr);
    void longCastReturnError(const Token *tok, const ValueType* src = nullptr, const ValueType* tgt = nullptr);
    void floatToIntegerOverflowError(const Token *tok, const ValueFlow::Value &value);
};
/// @}
//---------------------------------------------------------------------------
#endif // checktypeH
