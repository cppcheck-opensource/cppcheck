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
#ifndef check64bitH
#define check64bitH
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
 * @brief Check for 64-bit portability issues
 */

class CPPCHECKLIB Check64BitPortability : public Check {
    friend class Test64BitPortability;

public:
    /** This constructor is used when registering the Check64BitPortability */
    Check64BitPortability() : Check("64-bit portability") {}

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger& errorLogger) override;

    void getErrorMessages(ErrorLogger& errorLogger, const Settings &settings) const override;

    std::string classInfo() const override {
        return "Check if there is 64-bit portability issues:\n"
               "- assign address to/from int/long\n"
               "- casting address from/to integer when returning from function\n";
    }
};

class CPPCHECKLIB Check64BitPortabilityImpl : public CheckImpl {
public:
    /** This constructor is used when running checks. */
    Check64BitPortabilityImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger &errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    /** Check for pointer assignment */
    void pointerassignment();

    void assignmentAddressToIntegerError(const Token *tok);
    void assignmentIntegerToAddressError(const Token *tok);
    void returnIntegerError(const Token *tok);
    void returnPointerError(const Token *tok);
};
/// @}
//---------------------------------------------------------------------------
#endif // check64bitH
