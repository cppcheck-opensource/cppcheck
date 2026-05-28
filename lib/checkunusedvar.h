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
#ifndef checkunusedvarH
#define checkunusedvarH
//---------------------------------------------------------------------------

#include "check.h"
#include "checkimpl.h"
#include "config.h"

#include <map>
#include <string>

class ErrorLogger;
class Scope;
class Settings;
class Token;
class Type;
class Variables;
class Tokenizer;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB CheckUnusedVar : public Check {
    friend class TestUnusedVar;

public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckUnusedVar() : Check("UnusedVar") {}

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings &settings) const override;

    std::string classInfo() const override {
        return "UnusedVar checks\n"

               // style
               "- unused variable\n"
               "- allocated but unused variable\n"
               "- unread variable\n"
               "- unassigned variable\n"
               "- unused struct member\n";
    }
};

class CPPCHECKLIB CheckUnusedVarImpl : public CheckImpl {
public:
    /** @brief This constructor is used when running checks. */
    CheckUnusedVarImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    /** @brief %Check for unused function variables */
    void checkFunctionVariableUsage_iterateScopes(const Scope* scope, Variables& variables) const;
    void checkFunctionVariableUsage();

    /** @brief %Check that all struct members are used */
    void checkStructMemberUsage();

    bool isEmptyType(const Type* type);

    // Error messages..
    void unusedStructMemberError(const Token *tok, const std::string &structname, const std::string &varname, const std::string& prefix = "struct");
    void unusedVariableError(const Token *tok, const std::string &varname);
    void allocatedButUnusedVariableError(const Token *tok, const std::string &varname);
    void unreadVariableError(const Token *tok, const std::string &varname, bool modified);
    void unassignedVariableError(const Token *tok, const std::string &varname);

    std::map<const Type *,bool> mIsEmptyTypeMap;
};
/// @}
//---------------------------------------------------------------------------
#endif // checkunusedvarH
