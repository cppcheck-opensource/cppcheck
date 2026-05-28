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


//---------------------------------------------------------------------------
#ifndef checkpostfixoperatorH
#define checkpostfixoperatorH
//---------------------------------------------------------------------------

#include "check.h"
#include "checkimpl.h"
#include "config.h"

#include <string>

class ErrorLogger;
class Settings;
class Token;
class Tokenizer;

/// @addtogroup Checks
/// @{

/**
 * @brief Using postfix operators ++ or -- rather than postfix operator.
 */

class CPPCHECKLIB CheckPostfixOperator : public Check {
    friend class TestPostfixOperator;

public:
    /** This constructor is used when registering the CheckPostfixOperator */
    CheckPostfixOperator() : Check("Using postfix operators") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings &settings) const override;
    std::string classInfo() const override {
        return "Warn if using postfix operators ++ or -- rather than prefix operator\n";
    }
};

class CPPCHECKLIB CheckPostfixOperatorImpl : public CheckImpl {
public:
    /** This constructor is used when running checks. */
    CheckPostfixOperatorImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    /** Check postfix operators */
    void postfixOperator();

    /** Report Error */
    void postfixOperatorError(const Token *tok);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkpostfixoperatorH
