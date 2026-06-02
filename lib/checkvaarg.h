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
#ifndef checkvaargtH
#define checkvaargtH
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
 * @brief Checking for misusage of variable argument lists
 */

class CPPCHECKLIB CheckVaarg : public Check {
public:
    CheckVaarg() : Check("Vaarg") {}

    void runChecks(const Tokenizer &tokenizer, ErrorLogger& errorLogger) override;

private:
    void getErrorMessages(ErrorLogger& errorLogger, const Settings &settings) const override;

    std::string classInfo() const override {
        return "Check for misusage of variable argument lists:\n"
               "- Wrong parameter passed to va_start()\n"
               "- Reference passed to va_start()\n"
               "- Missing va_end()\n"
               "- Using va_list before it is opened\n"
               "- Subsequent calls to va_start/va_copy()\n";
    }
};

class CPPCHECKLIB CheckVaargImpl : public CheckImpl
{
public:
    CheckVaargImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger &errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    void va_start_argument();
    void va_list_usage();

    void wrongParameterTo_va_start_error(const Token *tok, const std::string& paramIsName, const std::string& paramShouldName);
    void referenceAs_va_start_error(const Token *tok, const std::string& paramName);
    void va_end_missingError(const Token *tok, const std::string& varname);
    void va_list_usedBeforeStartedError(const Token *tok, const std::string& varname);
    void va_start_subsequentCallsError(const Token *tok, const std::string& varname);
};

/// @}

//---------------------------------------------------------------------------
#endif // checkvaargtH
