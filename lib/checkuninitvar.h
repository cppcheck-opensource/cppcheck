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
#ifndef checkuninitvarH
#define checkuninitvarH
//---------------------------------------------------------------------------

#include "check.h"
#include "checkimpl.h"
#include "config.h"
#include "mathlib.h"
#include "errortypes.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>

class Scope;
class Token;
class Variable;
class ErrorLogger;
class Settings;
class Library;
class Tokenizer;
namespace ValueFlow
{
    class Value;
}

struct VariableValue {
    explicit VariableValue(MathLib::bigint val = 0) : value(val) {}
    MathLib::bigint value;
    bool notEqual{};
};

/// @addtogroup Checks
/// @{


/** @brief Checking for uninitialized variables */

class CPPCHECKLIB CheckUninitVar : public Check {
    friend class TestUninitVar;

public:
    /** @brief This constructor is used when registering the CheckUninitVar */
    CheckUninitVar() : Check("Uninitialized variables") {}

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer &tokenizer, const Settings &settings, const std::string& /*currentConfig*/) const override;

    Check::FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::FileInfo &ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) override;

    void getErrorMessages(ErrorLogger* errorLogger, const Settings& settings) const override;

    std::string classInfo() const override {
        return "Uninitialized variables\n"
               "- using uninitialized local variables\n"
               "- using allocated data before it has been initialized\n";
    }
};

class CPPCHECKLIB CheckUninitVarImpl : public CheckImpl {
public:
    /** @brief This constructor is used when running checks. */
    CheckUninitVarImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    enum Alloc : std::uint8_t { NO_ALLOC, NO_CTOR_CALL, CTOR_CALL, ARRAY };

    bool diag(const Token* tok);
    /** Check for uninitialized variables */
    void check();
    void checkScope(const Scope* scope, const std::set<std::string> &arrayTypeDefs);
    void checkStruct(const Token *tok, const Variable &structvar);
    bool checkScopeForVariable(const Token *tok, const Variable& var, bool* possibleInit, bool* noreturn, Alloc* alloc, const std::string &membervar, std::map<nonneg int, VariableValue>& variableValue);
    const Token* checkExpr(const Token* tok, const Variable& var, Alloc alloc, bool known, bool* bailout = nullptr) const;
    bool checkIfForWhileHead(const Token *startparentheses, const Variable& var, bool suppressErrors, bool isuninit, Alloc alloc, const std::string &membervar);
    bool checkLoopBody(const Token *tok, const Variable& var, Alloc alloc, const std::string &membervar, bool suppressErrors);
    const Token* checkLoopBodyRecursive(const Token *start, const Variable& var, Alloc alloc, const std::string &membervar, bool &bailout, bool &alwaysReturns) const;
    void checkRhs(const Token *tok, const Variable &var, Alloc alloc, nonneg int number_of_if, const std::string &membervar);
    static int isFunctionParUsage(const Token *vartok, const Library &library, bool pointer, Alloc alloc, int indirect = 0);
    int isFunctionParUsage(const Token *vartok, bool pointer, Alloc alloc, int indirect = 0) const;
    bool isMemberVariableAssignment(const Token *tok, const std::string &membervar) const;
    bool isMemberVariableUsage(const Token *tok, bool isPointer, Alloc alloc, const std::string &membervar) const;

    static const Token *isVariableUsage(const Token *vartok, const Library &library, bool pointer, Alloc alloc, int indirect = 0);
    const Token *isVariableUsage(const Token *vartok, bool pointer, Alloc alloc, int indirect = 0) const;

    /** ValueFlow-based checking for uninitialized variables */
    void valueFlowUninit();

    void uninitvarError(const Token* tok, const ValueFlow::Value& v);
    void uninitdataError(const Token *tok, const std::string &varname);
    void uninitvarError(const Token *tok, const std::string &varname, ErrorPath errorPath);
    void uninitvarError(const Token *tok, const std::string &varname) {
        uninitvarError(tok, varname, ErrorPath{});
    }
    void uninitvarError(const Token *tok, const std::string &varname, Alloc alloc) {
        if (alloc == NO_CTOR_CALL || alloc == CTOR_CALL)
            uninitdataError(tok, varname);
        else
            uninitvarError(tok, varname);
    }
    void uninitStructMemberError(const Token *tok, const std::string &membername);

    std::set<const Token*> mUninitDiags;
};
/// @}
//---------------------------------------------------------------------------
#endif // checkuninitvarH
