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
#ifndef checkleakautovarH
#define checkleakautovarH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "library.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>

class ErrorLogger;
class Settings;
class Token;
class Tokenizer;

class CPPCHECKLIB VarInfo {
public:
    enum AllocStatus : std::int8_t { REALLOC = -3, OWNED = -2, DEALLOC = -1, NOALLOC = 0, ALLOC = 1 };
    struct AllocInfo {
        AllocStatus status;
        /** Allocation type. If it is a positive value then it corresponds to
         * a Library allocation id. A negative value is a builtin
         * checkleakautovar allocation type.
         */
        int type;
        int reallocedFromType = -1;
        const Token * allocTok;
        explicit AllocInfo(int type_ = 0, AllocStatus status_ = NOALLOC, const Token* allocTok_ = nullptr) : status(status_), type(type_), allocTok(allocTok_) {}

        bool managed() const {
            return status < 0;
        }
    };
    enum Usage : std::uint8_t { USED, NORET };
    std::map<int, AllocInfo> alloctype;
    std::map<int, std::pair<const Token*, Usage>> possibleUsage;
    std::set<int> conditionalAlloc;
    std::set<int> referenced;

    void clear() {
        alloctype.clear();
        possibleUsage.clear();
        conditionalAlloc.clear();
        referenced.clear();
    }

    void erase(nonneg int varid) {
        alloctype.erase(varid);
        possibleUsage.erase(varid);
        conditionalAlloc.erase(varid);
        referenced.erase(varid);
    }

    void swap(VarInfo &other) noexcept {
        alloctype.swap(other.alloctype);
        possibleUsage.swap(other.possibleUsage);
        conditionalAlloc.swap(other.conditionalAlloc);
        referenced.swap(other.referenced);
    }

    void reallocToAlloc(nonneg int varid) {
        const AllocInfo& alloc = alloctype[varid];
        if (alloc.reallocedFromType >= 0) {
            const auto it = alloctype.find(alloc.reallocedFromType);
            if (it != alloctype.end() && it->second.status == REALLOC) {
                it->second.status = ALLOC;
            }
        }
    }

    /** set possible usage for all variables */
    void possibleUsageAll(const std::pair<const Token*, Usage>& functionUsage);
};


/// @addtogroup Checks
/// @{

/**
 * @brief Check for leaks
 */

class CPPCHECKLIB CheckLeakAutoVar : public Check {
public:
    /** This constructor is used when registering the CheckLeakAutoVar */
    CheckLeakAutoVar() : Check(myName()) {}

private:
    /** This constructor is used when running checks. */
    CheckLeakAutoVar(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** check for leaks in all scopes */
    void check();

    /** check for leaks in a function scope
     * @throws InternalError thrown if recursion count is exceeded
     */
    bool checkScope(const Token * startToken,
                    VarInfo &varInfo,
                    std::set<int> notzero,
                    nonneg int recursiveCount);

    /** Check token inside expression.
     * @param tok token inside expression.
     * @param varInfo Variable info
     * @return next token to process (if no other checks needed for this token). NULL if other checks could be performed.
     */
    const Token * checkTokenInsideExpression(const Token * tok, VarInfo &varInfo, bool inFuncCall = false);

    /** parse function call */
    void functionCall(const Token *tokName, const Token *tokOpeningPar, VarInfo &varInfo, const VarInfo::AllocInfo& allocation, const Library::AllocFunc* af);

    /** parse changes in allocation status */
    void changeAllocStatus(VarInfo &varInfo, const VarInfo::AllocInfo& allocation, const Token* tok, const Token* arg);

    /** update allocation status if reallocation function */
    void changeAllocStatusIfRealloc(std::map<int, VarInfo::AllocInfo> &alloctype, const Token *fTok, const Token *retTok) const;

    /** return. either "return" or end of variable scope is seen */
    void ret(const Token *tok, VarInfo &varInfo, bool isEndOfScope = false);

    /** if variable is allocated then there is a leak */
    void leakIfAllocated(const Token *vartok, const VarInfo &varInfo);

    void leakError(const Token* tok, const std::string &varname, int type) const;
    void mismatchError(const Token* deallocTok, const Token* allocTok, const std::string &varname) const;
    void deallocUseError(const Token *tok, const std::string &varname) const;
    void deallocReturnError(const Token *tok, const Token *deallocTok, const std::string &varname);
    void doubleFreeError(const Token *tok, const Token *prevFreeTok, const std::string &varname, int type);

    /** message: user configuration is needed to complete analysis */
    void configurationInfo(const Token* tok, const std::pair<const Token*, VarInfo::Usage>& functionUsage);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    static std::string myName() {
        return "Leaks (auto variables)";
    }

    std::string classInfo() const override {
        return "Detect when a auto variable is allocated but not deallocated or deallocated twice.\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkleakautovarH
