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
#ifndef checkfunctionsH
#define checkfunctionsH
//---------------------------------------------------------------------------

#include "check.h"
#include "checkimpl.h"
#include "config.h"

#include <string>

class Token;
class ErrorLogger;
class Tokenizer;
class Settings;

namespace ValueFlow {
    class Value;
}  // namespace ValueFlow


/// @addtogroup Checks
/// @{

/**
 * @brief Check for bad function usage
 */

class CPPCHECKLIB CheckFunctions : public Check {
public:
    /** This constructor is used when registering the CheckFunctions */
    CheckFunctions() : Check("Check function usage") {}

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings &settings) const override;

    std::string classInfo() const override {
        return "Check function usage:\n"
               "- missing 'return' in non-void function\n"
               "- return value of certain functions not used\n"
               "- invalid input values for functions\n"
               "- Warn if a function is called whose usage is discouraged\n"
               "- memset() third argument is zero\n"
               "- memset() with a value out of range as the 2nd parameter\n"
               "- memset() with a float as the 2nd parameter\n"
               "- copy elision optimization for returning value affected by std::move\n"
               "- use memcpy()/memset() instead of for loop\n";
    }
};

class CPPCHECKLIB CheckFunctionsImpl : public CheckImpl {
public:
    /** This constructor is used when running checks. */
    CheckFunctionsImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    /** Check for functions that should not be used */
    void checkProhibitedFunctions();

    /**
     * @brief Invalid function usage (invalid input value / overlapping data)
     *
     * %Check that given function parameters are valid according to the standard
     * - wrong radix given for strtol/strtoul
     * - overlapping data when using sprintf/snprintf
     * - wrong input value according to library
     */
    void invalidFunctionUsage();

    /** @brief %Check for ignored return values. */
    void checkIgnoredReturnValue();

    /** @brief %Check for parameters given to math function that do not make sense*/
    void checkMathFunctions();

    /** @brief %Check for filling zero bytes with memset() */
    void memsetZeroBytes();

    /** @brief %Check for invalid 2nd parameter of memset() */
    void memsetInvalid2ndParam();

    /** @brief %Check for copy elision by RVO|NRVO */
    void returnLocalStdMove();

    void useStandardLibrary();

    /** @brief --check-library: warn for unconfigured function calls */
    void checkLibraryMatchFunctions();

    /** @brief %Check for missing "return" */
    void checkMissingReturn();

    void invalidFunctionArgError(const Token *tok, const std::string &functionName, int argnr, const ValueFlow::Value *invalidValue, const std::string &validstr);
    void invalidFunctionArgBoolError(const Token *tok, const std::string &functionName, int argnr);
    void invalidFunctionArgStrError(const Token *tok, const std::string &functionName, nonneg int argnr);
    void ignoredReturnValueError(const Token* tok, const std::string& function);
    void ignoredReturnErrorCode(const Token* tok, const std::string& function);
    void mathfunctionCallWarning(const Token *tok, nonneg int numParam = 1);
    void mathfunctionCallWarning(const Token *tok, const std::string& oldexp, const std::string& newexp);
    void memsetZeroBytesError(const Token *tok);
    void memsetFloatError(const Token *tok, const std::string &var_value);
    void memsetValueOutOfRangeError(const Token *tok, const std::string &value);
    void missingReturnError(const Token *tok);
    void copyElisionError(const Token *tok);
    void useStandardLibraryError(const Token *tok, const std::string& expected);
    void functionCalledError(const Token* tok, Severity severity, const std::string& prefix, const std::string& msg);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkfunctionsH
