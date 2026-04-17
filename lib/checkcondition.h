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
#ifndef checkconditionH
#define checkconditionH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "mathlib.h"
#include "errortypes.h"

#include <set>
#include <string>

class Settings;
class Token;
class ErrorLogger;
class ValueType;
class Tokenizer;

namespace ValueFlow {
    class Value;
}

/// @addtogroup Checks
/// @{

/**
 * @brief Check for condition mismatches
 */

class CPPCHECKLIB CheckCondition : public Check {
public:
    /** This constructor is used when registering the CheckAssignIf */
    CheckCondition() : Check(myName()) {}

private:
    /** This constructor is used when running checks. */
    CheckCondition(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** mismatching assignment / comparison */
    void assignIf();

    /** parse scopes recursively */
    bool assignIfParseScope(const Token * assignTok,
                            const Token * startTok,
                            nonneg int varid,
                            bool islocal,
                            char bitop,
                            MathLib::bigint num);

    /** check bitmask using | instead of & */
    void checkBadBitmaskCheck();

    /** mismatching lhs and rhs in comparison */
    void comparison();

    void duplicateCondition();

    /** match 'if' and 'else if' conditions */
    void multiCondition();

    /**
     * multiconditions #2
     * - Opposite inner conditions => always false
     * - Same/Overlapping inner condition => always true
     * - same condition after early exit => always false
     **/
    void multiCondition2();

    /** @brief %Check for testing for mutual exclusion over ||*/
    void checkIncorrectLogicOperator();

    /** @brief %Check for suspicious usage of modulo (e.g. "if(var % 4 == 4)") */
    void checkModuloAlwaysTrueFalse();

    /** @brief Suspicious condition (assignment+comparison) */
    void clarifyCondition();

    /** @brief Condition is always true/false */
    void alwaysTrueFalse();

    /** @brief %Check for invalid test for overflow 'x+100 < x' */
    void checkInvalidTestForOverflow();

    /** @brief Check if pointer addition result is NULL '(ptr + 1) == NULL' */
    void checkPointerAdditionResultNotNull();

    void checkDuplicateConditionalAssign();

    /** @brief Assignment in condition */
    void checkAssignmentInCondition();

    // The conditions that have been diagnosed
    std::set<const Token*> mCondDiags;
    bool diag(const Token* tok, bool insert=true);
    /** @brief Mark both token as diagnosed */
    bool diag(const Token* tok1, const Token* tok2);
    bool isAliased(const std::set<int> &vars) const;
    bool isOverlappingCond(const Token * cond1, const Token * cond2, bool pure) const;
    void assignIfError(const Token *tok1, const Token *tok2, const std::string &condition, bool result);
    void mismatchingBitAndError(const Token *tok1, MathLib::bigint num1, const Token *tok2, MathLib::bigint num2);
    void badBitmaskCheckError(const Token *tok, bool isNoOp = false);
    void comparisonError(const Token *tok,
                         const std::string &bitop,
                         MathLib::bigint value1,
                         const std::string &op,
                         MathLib::bigint value2,
                         bool result);
    void duplicateConditionError(const Token *tok1, const Token *tok2, ErrorPath errorPath);
    void overlappingElseIfConditionError(const Token *tok, nonneg int line1);
    void overlappingInnerConditionError(const Token *tok1, const Token *tok2, ErrorPath errorPath);
    void oppositeElseIfConditionError(const Token *ifCond, const Token *elseIfCond, ErrorPath errorPath);

    void oppositeInnerConditionError(const Token *tok1, const Token* tok2, ErrorPath errorPath);

    void identicalInnerConditionError(const Token *tok1, const Token* tok2, ErrorPath errorPath);

    void identicalConditionAfterEarlyExitError(const Token *cond1, const Token *cond2, ErrorPath errorPath);

    void incorrectLogicOperatorError(const Token *tok, const std::string &condition, bool always, bool inconclusive, ErrorPath errors);
    void redundantConditionError(const Token *tok, const std::string &text, bool inconclusive);

    void moduloAlwaysTrueFalseError(const Token* tok, const std::string& maxVal);

    void clarifyConditionError(const Token *tok, bool assign, bool boolop);

    void alwaysTrueFalseError(const Token* tok, const Token* condition, const ValueFlow::Value* value);

    void invalidTestForOverflow(const Token* tok, const ValueType *valueType, const std::string &replace);
    void pointerAdditionResultNotNullError(const Token *tok, const Token *calc);

    void duplicateConditionalAssignError(const Token *condTok, const Token* assignTok, bool isRedundant = false);

    void assignmentInCondition(const Token *eq);

    void checkCompareValueOutOfTypeRange();
    void compareValueOutOfTypeRangeError(const Token *comparison, const std::string &type, MathLib::bigint value, bool result);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    static std::string myName() {
        return "Condition";
    }

    std::string classInfo() const override {
        return "Match conditions with assignments and other conditions:\n"
               "- Mismatching assignment and comparison => comparison is always true/false\n"
               "- Mismatching lhs and rhs in comparison => comparison is always true/false\n"
               "- Detect usage of | where & should be used\n"
               "- Duplicate condition and assignment\n"
               "- Detect matching 'if' and 'else if' conditions\n"
               "- Mismatching bitand (a &= 0xf0; a &= 1; => a = 0)\n"
               "- Opposite inner condition is always false\n"
               "- Identical condition after early exit is always false\n"
               "- Condition that is always true/false\n"
               "- Mutual exclusion over || always evaluating to true\n"
               "- Comparisons of modulo results that are always true/false.\n"
               "- Known variable values => condition is always true/false\n"
               "- Invalid test for overflow. Some mainstream compilers remove such overflow tests when optimising code.\n"
               "- Suspicious assignment of container/iterator in condition => condition is always true.\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkconditionH
