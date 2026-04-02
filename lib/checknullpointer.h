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
#ifndef checknullpointerH
#define checknullpointerH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <list>
#include <set>
#include <string>

class ErrorLogger;
class Library;
class Settings;
class Token;
class Tokenizer;

namespace ValueFlow
{
    class Value;
}

/// @addtogroup Checks
/// @{


/** @brief check for null pointer dereferencing */

class CPPCHECKLIB CheckNullPointer : public Check {
    friend class TestNullPointer;

public:
    /** @brief This constructor is used when registering the CheckNullPointer */
    CheckNullPointer() : Check(myName()) {}

    /**
     * Is there a pointer dereference? Everything that should result in
     * a nullpointer dereference error message will result in a true
     * return value. If it's unknown if the pointer is dereferenced false
     * is returned.
     * @param tok token for the pointer
     * @param unknown it is not known if there is a pointer dereference (could be reported as a debug message)
     * @return true => there is a dereference
     */
    bool isPointerDeRef(const Token *tok, bool &unknown) const;

    static bool isPointerDeRef(const Token *tok, bool &unknown, const Settings &settings, bool checkNullArg = true);

private:
    /**
     * @brief parse a function call and extract information about variable usage
     * @param tok first token
     * @param library --library files data
     * @param checkNullArg perform isnullargbad check for each argument?
     * @return list of variables that the function reads / writes.
     */
    static std::list<const Token*> parseFunctionCall(const Token &tok, const Library &library, bool checkNullArg = true);

    /** @brief This constructor is used when running checks. */
    CheckNullPointer(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** @brief possible null pointer dereference */
    void nullPointer();

    /** @brief dereferencing null constant (after Tokenizer::simplifyKnownVariables) */
    void nullConstantDereference();

    void nullPointerError(const Token *tok);
    void nullPointerError(const Token *tok, const std::string &varname, const ValueFlow::Value* value, bool inconclusive);

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer &tokenizer, const Settings &settings, const std::string& /*currentConfig*/) const override;

    Check::FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::FileInfo &ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) override;

    /** Get error messages. Used by --errorlist */
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    /** Name of check */
    static std::string myName() {
        return "Null pointer";
    }

    /** class info in WIKI format. Used by --doc */
    std::string classInfo() const override {
        return "Null pointers\n"
               "- null pointer dereferencing\n"
               "- undefined null pointer arithmetic\n";
    }

    /**
     * @brief Does one part of the check for nullPointer().
     * Dereferencing a pointer and then checking if it's NULL..
     */
    void nullPointerByDeRefAndCheck();

    /** undefined null pointer arithmetic */
    void arithmetic();
    void pointerArithmeticError(const Token* tok, const ValueFlow::Value *value, bool inconclusive);
    void redundantConditionWarning(const Token* tok, const ValueFlow::Value *value, const Token *condition, bool inconclusive);

    bool diag(const Token* tok) {
        return !mDiag.emplace(tok).second;
    }

    std::set<const Token*> mDiag;
};
/// @}
//---------------------------------------------------------------------------
#endif // checknullpointerH
