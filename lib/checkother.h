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
#ifndef checkotherH
#define checkotherH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "errortypes.h"

#include <set>
#include <string>
#include <vector>

namespace ValueFlow {
    class Value;
}

class Settings;
class Token;
class Function;
class Variable;
class ErrorLogger;
class Tokenizer;
struct UnionMember;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB CheckOther : public Check {
    friend class TestCharVar;
    friend class TestIncompleteStatement;
    friend class TestOther;

public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckOther() : Check(myName()) {}

    /** Is expression a comparison that checks if a nonzero (unsigned/pointer) expression is less than zero? */
    static bool comparisonNonZeroExpressionLessThanZero(const Token *tok, const ValueFlow::Value *&zeroValue, const Token *&nonZeroExpr, bool suppress = false);

    /** Is expression a comparison that checks if a nonzero (unsigned/pointer) expression is positive? */
    static bool testIfNonZeroExpressionIsPositive(const Token *tok, const ValueFlow::Value *&zeroValue, const Token *&nonZeroExpr);

private:
    /** @brief This constructor is used when running checks. */
    CheckOther(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}


    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** @brief Clarify calculation for ".. a * b ? .." */
    void clarifyCalculation();

    /** @brief Suspicious statement like '*A++;' */
    void clarifyStatement();

    /** @brief Are there C-style pointer casts in a c++ file? */
    void warningOldStylePointerCast();

    /** @brief Dangerous type cast */
    void warningDangerousTypeCast();

    /** @brief Casting non-hexadecimal integer literal to pointer */
    void warningIntToPointerCast();

    void suspiciousFloatingPointCast();

    /** @brief Check for pointer casts to a type with an incompatible binary data representation */
    void invalidPointerCast();

    /** @brief %Check scope of variables */
    void checkVariableScope();
    bool checkInnerScope(const Token *tok, const Variable* var, bool& used) const;

    /** @brief %Check for comma separated statements in return */
    void checkCommaSeparatedReturn();

    /** @brief %Check for function parameters that should be passed by reference */
    void checkPassByReference();

    void checkConstVariable();
    void checkConstPointer();

    /** @brief Using char variable as array index / as operand in bit operation */
    void checkCharVariable();

    /** @brief Incomplete statement. A statement that only contains a constant or variable */
    void checkIncompleteStatement();

    /** @brief %Check zero division*/
    void checkZeroDivision();

    /** @brief Check for NaN (not-a-number) in an arithmetic expression */
    void checkNanInArithmeticExpression();

    /** @brief copying to memory or assigning to a variable twice */
    void checkRedundantAssignment();

    /** @brief %Check for redundant bitwise operation in switch statement*/
    void redundantBitwiseOperationInSwitchError();

    /** @brief %Check for code like 'case A||B:'*/
    void checkSuspiciousCaseInSwitch();

    /** @brief %Check for objects that are destroyed immediately */
    void checkMisusedScopedObject();

    /** @brief %Check for suspicious code where if and else branch are the same (e.g "if (a) b = true; else b = true;") */
    void checkDuplicateBranch();

    /** @brief %Check for suspicious code with the same expression on both sides of operator (e.g "if (a && a)") */
    void checkDuplicateExpression();

    /** @brief %Check for code that gets never executed, such as duplicate break statements */
    void checkUnreachableCode();

    /** @brief %Check for testing sign of unsigned variable */
    void checkSignOfUnsignedVariable();

    /** @brief %Check for suspicious use of semicolon */
    void checkSuspiciousSemicolon();

    /** @brief %Check for free() operations on invalid memory locations */
    void checkInvalidFree();
    void invalidFreeError(const Token *tok, const std::string &allocation, bool inconclusive);

    /** @brief %Check for code creating redundant copies */
    void checkRedundantCopy();

    /** @brief %Check for bitwise shift with negative right operand */
    void checkNegativeBitwiseShift();

    /** @brief %Check for buffers that are filled incompletely with memset and similar functions */
    void checkIncompleteArrayFill();

    /** @brief %Check that variadic function calls don't use NULL. If NULL is \#defined as 0 and the function expects a pointer, the behaviour is undefined. */
    void checkVarFuncNullUB();

    /** @brief %Check to avoid casting a return value to unsigned char and then back to integer type.  */
    void checkCastIntToCharAndBack();

    /** @brief %Check for using of comparison functions evaluating always to true or false. */
    void checkComparisonFunctionIsAlwaysTrueOrFalse();

    /** @brief %Check for redundant pointer operations */
    void checkRedundantPointerOp();

    /** @brief %Check for race condition with non-interlocked access after InterlockedDecrement() */
    void checkInterlockedDecrement();

    /** @brief %Check for unused labels */
    void checkUnusedLabel();

    /** @brief %Check for expression that depends on order of evaluation of side effects */
    void checkEvaluationOrder();

    /** @brief %Check for access of moved or forwarded variable */
    void checkAccessOfMovedVariable();

    /** @brief %Check if function declaration and definition argument names different */
    void checkFuncArgNamesDifferent();

    /** @brief %Check for shadow variables. Less noisy than gcc/clang -Wshadow. */
    void checkShadowVariables();

    void checkKnownArgument();

    void checkKnownPointerToBool();

    void checkComparePointers();

    void checkModuloOfOne();

    /**
     * @throws InternalError thrown if largest union member could not be found
     */
    void checkUnionZeroInit();

    void checkOverlappingWrite();
    void overlappingWriteUnion(const Token *tok);
    void overlappingWriteFunction(const Token *tok, const std::string& funcname);

    // Error messages..
    void checkComparisonFunctionIsAlwaysTrueOrFalseError(const Token* tok, const std::string &functionName, const std::string &varName, bool result);
    void checkCastIntToCharAndBackError(const Token *tok, const std::string &strFunctionName);
    void clarifyCalculationError(const Token *tok, const std::string &op);
    void clarifyStatementError(const Token* tok);
    void cstyleCastError(const Token *tok, bool isPtr = true);
    void dangerousTypeCastError(const Token *tok, bool isPtr);
    void intToPointerCastError(const Token *tok, const std::string& format);
    void suspiciousFloatingPointCastError(const Token *tok);
    void invalidPointerCastError(const Token* tok, const std::string& from, const std::string& to, bool inconclusive, bool toIsInt);
    void passedByValueError(const Variable* var, bool inconclusive, bool isRangeBasedFor = false);
    void constVariableError(const Variable *var, const Function *function);
    void constStatementError(const Token *tok, const std::string &type, bool inconclusive);
    void signedCharArrayIndexError(const Token *tok);
    void unknownSignCharArrayIndexError(const Token *tok);
    void charBitOpError(const Token *tok);
    void variableScopeError(const Token *tok, const std::string &varname);
    void zerodivError(const Token *tok, const ValueFlow::Value *value);
    void nanInArithmeticExpressionError(const Token *tok);
    void redundantAssignmentError(const Token *tok1, const Token* tok2, const std::string& var, bool inconclusive);
    void redundantInitializationError(const Token *tok1, const Token* tok2, const std::string& var, bool inconclusive);
    void redundantAssignmentInSwitchError(const Token *tok1, const Token *tok2, const std::string &var);
    void redundantAssignmentSameValueError(const Token* tok, const ValueFlow::Value* val, const std::string& var);
    void redundantCopyError(const Token *tok1, const Token* tok2, const std::string& var);
    void redundantBitwiseOperationInSwitchError(const Token *tok, const std::string &varname);
    void suspiciousCaseInSwitchError(const Token* tok, const std::string& operatorString);
    void selfAssignmentError(const Token *tok, const std::string &varname);
    void misusedScopeObjectError(const Token *tok, const std::string &varname, bool isAssignment = false);
    void duplicateBranchError(const Token *tok1, const Token *tok2, ErrorPath errors);
    void duplicateAssignExpressionError(const Token *tok1, const Token *tok2, bool inconclusive);
    void oppositeExpressionError(const Token *opTok, ErrorPath errors);
    void duplicateExpressionError(const Token *tok1, const Token *tok2, const Token *opTok, ErrorPath errors, bool hasMultipleExpr = false);
    void duplicateValueTernaryError(const Token *tok);
    void duplicateExpressionTernaryError(const Token *tok, ErrorPath errors);
    void duplicateBreakError(const Token *tok, bool inconclusive);
    void unreachableCodeError(const Token* tok, const Token* noreturn, bool inconclusive);
    void redundantContinueError(const Token* tok);
    void unsignedLessThanZeroError(const Token *tok, const ValueFlow::Value *v, const std::string &varname);
    void pointerLessThanZeroError(const Token *tok, const ValueFlow::Value *v);
    void unsignedPositiveError(const Token *tok, const ValueFlow::Value *v, const std::string &varname);
    void pointerPositiveError(const Token *tok, const ValueFlow::Value *v);
    void suspiciousSemicolonError(const Token *tok);
    void negativeBitwiseShiftError(const Token *tok, int op);
    void redundantCopyError(const Token *tok, const std::string &varname);
    void incompleteArrayFillError(const Token* tok, const std::string& buffer, const std::string& function, bool boolean);
    void varFuncNullUBError(const Token *tok);
    void commaSeparatedReturnError(const Token *tok);
    void redundantPointerOpError(const Token* tok, const std::string& varname, bool inconclusive, bool addressOfDeref);
    void raceAfterInterlockedDecrementError(const Token* tok);
    void unusedLabelError(const Token* tok, bool inSwitch, bool hasIfdef);
    void unknownEvaluationOrder(const Token* tok, bool isUnspecifiedBehavior = false);
    void accessMovedError(const Token *tok, const std::string &varname, const ValueFlow::Value *value, bool inconclusive);
    void funcArgNamesDifferent(const std::string & functionName, nonneg int index, const Token* declaration, const Token* definition);
    void funcArgOrderDifferent(const std::string & functionName, const Token * declaration, const Token * definition, const std::vector<const Token*> & declarations, const std::vector<const Token*> & definitions);
    void shadowError(const Token *shadows, const std::string &shadowsType, const Token *shadowed, const std::string &shadowedType);
    void knownArgumentError(const Token *tok, const Token *ftok, const ValueFlow::Value *value, const std::string &varexpr, bool isVariableExpressionHidden);
    void knownPointerToBoolError(const Token* tok, const ValueFlow::Value* value);
    void comparePointersError(const Token *tok, const ValueFlow::Value *v1, const ValueFlow::Value *v2);
    void checkModuloOfOneError(const Token *tok);
    void unionZeroInitError(const Token *tok, const UnionMember& largestMember);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    static std::string myName() {
        return "Other";
    }

    std::string classInfo() const override {
        return "Other checks\n"

               // error
               "- division with zero\n"
               "- scoped object destroyed immediately after construction\n"
               "- assignment in an assert statement\n"
               "- free() or delete of an invalid memory location\n"
               "- bitwise operation with negative right operand\n"
               "- cast the return values of getc(),fgetc() and getchar() to character and compare it to EOF\n"
               "- race condition with non-interlocked access after InterlockedDecrement() call\n"
               "- expression 'x = x++;' depends on order of evaluation of side effects\n"
               "- overlapping write of union\n"

               // warning
               "- either division by zero or useless condition\n"
               "- access of moved or forwarded variable.\n"
               "- potentially dangerous C style type cast of pointer/reference to object.\n"

               // performance
               "- redundant data copying for const variable\n"
               "- subsequent assignment or copying to a variable or buffer\n"
               "- passing parameter by value\n"

               // portability
               "- Passing NULL pointer to function with variable number of arguments leads to UB.\n"
               "- Casting non-zero integer literal in decimal or octal format to pointer.\n"
               "- Incorrect zero initialization of unions can lead to access of uninitialized memory.\n"

               // style
               "- C-style pointer cast in C++ code\n"
               "- casting between incompatible pointer types\n"
               "- [Incomplete statement](IncompleteStatement)\n"
               "- [check how signed char variables are used](CharVar)\n"
               "- variable scope can be limited\n"
               "- unusual pointer arithmetic. For example: \"abc\" + 'd'\n"
               "- redundant assignment, increment, or bitwise operation in a switch statement\n"
               "- redundant strcpy in a switch statement\n"
               "- Suspicious case labels in switch()\n"
               "- assignment of a variable to itself\n"
               "- Comparison of values leading always to true or false\n"
               "- Clarify calculation with parentheses\n"
               "- suspicious comparison of '\\0' with a char\\* variable\n"
               "- duplicate break statement\n"
               "- unreachable code\n"
               "- testing if unsigned variable is negative/positive\n"
               "- Suspicious use of ; at the end of 'if/for/while' statement.\n"
               "- Array filled incompletely using memset/memcpy/memmove.\n"
               "- NaN (not a number) value used in arithmetic expression.\n"
               "- comma in return statement (the comma can easily be misread as a semicolon).\n"
               "- prefer erfc, expm1 or log1p to avoid loss of precision.\n"
               "- identical code in both branches of if/else or ternary operator.\n"
               "- redundant pointer operation on pointer like &\\*some_ptr.\n"
               "- find unused 'goto' labels.\n"
               "- function declaration and definition argument names different.\n"
               "- function declaration and definition argument order different.\n"
               "- shadow variable.\n"
               "- variable can be declared const.\n"
               "- calculating modulo of one.\n"
               "- known function argument, suspicious calculation.\n";
    }

    bool diag(const Token* tok) {
        return !mRedundantAssignmentDiag.emplace(tok).second;
    }
    std::set<const Token*> mRedundantAssignmentDiag;
};
/// @}
//---------------------------------------------------------------------------
#endif // checkotherH
