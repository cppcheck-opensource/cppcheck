/*
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

#include "checkother.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"

#include <cstddef>
#include <sstream>
#include <string>

static std::string unionZeroInitMessage(int lno, int cno, const std::string &varName, const std::string &largestMemberName)
{
    std::stringstream ss;
    ss << "[test.cpp:" << lno << ":" << cno << "]: (portability) ";
    ss << "Zero initializing union '" << varName << "' ";
    ss << "does not guarantee its complete storage to be zero initialized as its largest member is not declared as the first member. ";
    ss << "Consider making " << largestMemberName << " the first member or favor memset(). [UnionZeroInit]";
    ss << std::endl;
    return ss.str();
}

class TestOther : public TestFixture {
public:
    TestOther() : TestFixture("TestOther") {}

private:
    const Settings settings0 = settingsBuilder().library("std.cfg").build();
    /*const*/ Settings settings1 = settingsBuilder().library("std.cfg").severity(Severity::style).severity(Severity::warning).severity(Severity::portability).severity(Severity::performance).build();
    const Settings settings2 = settingsBuilder(settings1).certainty(Certainty::inconclusive).build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(emptyBrackets);

        TEST_CASE(zeroDiv1);
        TEST_CASE(zeroDiv2);
        TEST_CASE(zeroDiv3);
        TEST_CASE(zeroDiv4);
        TEST_CASE(zeroDiv5);
        TEST_CASE(zeroDiv6);
        TEST_CASE(zeroDiv7);  // #4930
        TEST_CASE(zeroDiv8);
        TEST_CASE(zeroDiv9);
        TEST_CASE(zeroDiv10);
        TEST_CASE(zeroDiv11);
        TEST_CASE(zeroDiv12);
        TEST_CASE(zeroDiv13);
        TEST_CASE(zeroDiv14); // #1169
        TEST_CASE(zeroDiv15); // #8319
        TEST_CASE(zeroDiv16); // #11158
        TEST_CASE(zeroDiv17); // #9931
        TEST_CASE(zeroDiv18);
        TEST_CASE(zeroDiv19);
        TEST_CASE(zeroDiv20); // #11175
        TEST_CASE(zeroDiv21);
        TEST_CASE(zeroDiv22);

        TEST_CASE(zeroDivCond); // division by zero / useless condition

        TEST_CASE(nanInArithmeticExpression);

        TEST_CASE(varScope1);
        TEST_CASE(varScope2);
        TEST_CASE(varScope3);
        TEST_CASE(varScope4);
        TEST_CASE(varScope5);
        TEST_CASE(varScope6);
        TEST_CASE(varScope7);
        TEST_CASE(varScope8);
        TEST_CASE(varScope9);       // classes may have extra side-effects
        TEST_CASE(varScope10);      // Undefined macro FOR
        TEST_CASE(varScope11);      // #2475 - struct initialization is not inner scope
        TEST_CASE(varScope12);
        TEST_CASE(varScope13);      // variable usage in inner loop
        TEST_CASE(varScope14);
        TEST_CASE(varScope15);      // #4573 if-else-if
        TEST_CASE(varScope16);
        TEST_CASE(varScope17);
        TEST_CASE(varScope18);
        TEST_CASE(varScope20);      // Ticket #5103
        TEST_CASE(varScope21);      // Ticket #5382
        TEST_CASE(varScope22);      // Ticket #5684
        TEST_CASE(varScope23);      // Ticket #6154
        TEST_CASE(varScope24);      // pointer / reference
        TEST_CASE(varScope25);      // time_t
        TEST_CASE(varScope26);      // range for loop, map
        TEST_CASE(varScope27);      // #7733 - #if
        TEST_CASE(varScope28);      // #10527
        TEST_CASE(varScope29);      // #10888
        TEST_CASE(varScope30);      // #8541
        TEST_CASE(varScope31);      // #11099
        TEST_CASE(varScope32);      // #11441
        TEST_CASE(varScope33);
        TEST_CASE(varScope34);
        TEST_CASE(varScope35);
        TEST_CASE(varScope36);      // #12158
        TEST_CASE(varScope37);      // #12158
        TEST_CASE(varScope38);
        TEST_CASE(varScope39);
        TEST_CASE(varScope40);
        TEST_CASE(varScope41);      // #11845
        TEST_CASE(varScope42);
        TEST_CASE(varScope43);
        TEST_CASE(varScope44);
        TEST_CASE(varScope45);
        TEST_CASE(varScope46);

        TEST_CASE(oldStylePointerCast);
        TEST_CASE(intToPointerCast);
        TEST_CASE(invalidPointerCast);

        TEST_CASE(passedByValue);
        TEST_CASE(passedByValue_nonConst);
        TEST_CASE(passedByValue_externC);

        TEST_CASE(constVariable);
        TEST_CASE(constParameterCallback);
        TEST_CASE(constPointer);
        TEST_CASE(constArray);

        TEST_CASE(switchRedundantAssignmentTest);
        TEST_CASE(switchRedundantOperationTest);
        TEST_CASE(switchRedundantBitwiseOperationTest);
        TEST_CASE(unreachableCode);
        TEST_CASE(redundantContinue);

        TEST_CASE(suspiciousCase);
        TEST_CASE(suspiciousEqualityComparison);
        TEST_CASE(suspiciousUnaryPlusMinus); // #8004
        TEST_CASE(suspiciousFloatingPointCast);

        TEST_CASE(selfAssignment);
        TEST_CASE(trac1132);
        TEST_CASE(testMisusedScopeObjectDoesNotPickFunction1);
        TEST_CASE(testMisusedScopeObjectDoesNotPickFunction2);
        TEST_CASE(testMisusedScopeObjectPicksClass);
        TEST_CASE(testMisusedScopeObjectPicksStruct);
        TEST_CASE(testMisusedScopeObjectDoesNotPickIf);
        TEST_CASE(testMisusedScopeObjectDoesNotPickConstructorDeclaration);
        TEST_CASE(testMisusedScopeObjectDoesNotPickFunctor);
        TEST_CASE(testMisusedScopeObjectDoesNotPickLocalClassConstructors);
        TEST_CASE(testMisusedScopeObjectDoesNotPickUsedObject);
        TEST_CASE(testMisusedScopeObjectDoesNotPickPureC);
        TEST_CASE(testMisusedScopeObjectDoesNotPickNestedClass);
        TEST_CASE(testMisusedScopeObjectInConstructor);
        TEST_CASE(testMisusedScopeObjectStandardType);
        TEST_CASE(testMisusedScopeObjectNamespace);
        TEST_CASE(testMisusedScopeObjectAssignment); // #11371
        TEST_CASE(trac2071);
        TEST_CASE(trac2084);
        TEST_CASE(trac3693);

        TEST_CASE(clarifyCalculation);
        TEST_CASE(clarifyStatement);

        TEST_CASE(duplicateBranch);
        TEST_CASE(duplicateBranch1); // tests extracted by http://www.viva64.com/en/b/0149/ ( Comparison between PVS-Studio and cppcheck ): Errors detected in Quake 3: Arena by PVS-Studio: Fragment 2
        TEST_CASE(duplicateBranch2); // empty macro
        TEST_CASE(duplicateBranch3);
        TEST_CASE(duplicateBranch4);
        TEST_CASE(duplicateBranch5); // make sure the Token attributes are compared
        TEST_CASE(duplicateBranch6);
        TEST_CASE(duplicateExpression1);
        TEST_CASE(duplicateExpression2); // ticket #2730
        TEST_CASE(duplicateExpression3); // ticket #3317
        TEST_CASE(duplicateExpression4); // ticket #3354 (++)
        TEST_CASE(duplicateExpression5); // ticket #3749 (macros with same values)
        TEST_CASE(duplicateExpression6); // ticket #4639
        TEST_CASE(duplicateExpression7);
        TEST_CASE(duplicateExpression8);
        TEST_CASE(duplicateExpression9); // #9320
        TEST_CASE(duplicateExpression10); // #9485
        TEST_CASE(duplicateExpression11); // #8916 (function call)
        TEST_CASE(duplicateExpression12); // #10026
        TEST_CASE(duplicateExpression13); // #7899
        TEST_CASE(duplicateExpression14); // #9871
        TEST_CASE(duplicateExpression15); // #10650
        TEST_CASE(duplicateExpression16); // #10569
        TEST_CASE(duplicateExpression17); // #12036
        TEST_CASE(duplicateExpression18);
        TEST_CASE(duplicateExpression19);
        TEST_CASE(duplicateExpression20);
        TEST_CASE(duplicateExpression21);
        TEST_CASE(duplicateExpressionLoop);
        TEST_CASE(duplicateValueTernary);
        TEST_CASE(duplicateValueTernarySizeof); // #13773
        TEST_CASE(duplicateExpressionTernary); // #6391
        TEST_CASE(duplicateExpressionTemplate); // #6930
        TEST_CASE(duplicateExpressionCompareWithZero);
        TEST_CASE(oppositeExpression);
        TEST_CASE(duplicateVarExpression);
        TEST_CASE(duplicateVarExpressionUnique);
        TEST_CASE(duplicateVarExpressionAssign);
        TEST_CASE(duplicateVarExpressionCrash);
        TEST_CASE(multiConditionSameExpression);

        TEST_CASE(checkSignOfUnsignedVariable);
        TEST_CASE(checkSignOfPointer);

        TEST_CASE(checkSuspiciousSemicolon1);
        TEST_CASE(checkSuspiciousSemicolon2);
        TEST_CASE(checkSuspiciousSemicolon3);
        TEST_CASE(checkSuspiciousComparison);

        TEST_CASE(checkInvalidFree);

        TEST_CASE(checkRedundantCopy);

        TEST_CASE(checkNegativeShift);

        TEST_CASE(incompleteArrayFill);

        TEST_CASE(redundantVarAssignment);
        TEST_CASE(redundantVarAssignment_trivial);
        TEST_CASE(redundantVarAssignment_struct);
        TEST_CASE(redundantVarAssignment_union);
        TEST_CASE(redundantVarAssignment_7133);
        TEST_CASE(redundantVarAssignment_stackoverflow);
        TEST_CASE(redundantVarAssignment_lambda);
        TEST_CASE(redundantVarAssignment_loop);
        TEST_CASE(redundantVarAssignment_after_switch);
        TEST_CASE(redundantVarAssignment_pointer);
        TEST_CASE(redundantVarAssignment_pointer_parameter);
        TEST_CASE(redundantVarAssignment_array);
        TEST_CASE(redundantVarAssignment_switch_break);
        TEST_CASE(redundantInitialization);
        //TEST_CASE(redundantMemWrite); // FIXME: temporary hack
        TEST_CASE(redundantAssignmentSameValue);

        TEST_CASE(varFuncNullUB);

        TEST_CASE(checkCastIntToCharAndBack); // ticket #160

        TEST_CASE(checkCommaSeparatedReturn);
        TEST_CASE(checkPassByReference);

        TEST_CASE(checkComparisonFunctionIsAlwaysTrueOrFalse);

        TEST_CASE(integerOverflow); // #5895

        TEST_CASE(redundantPointerOp);
        TEST_CASE(test_isSameExpression);
        TEST_CASE(raceAfterInterlockedDecrement);

        TEST_CASE(testUnusedLabel);
        TEST_CASE(testUnusedLabelConfiguration);
        TEST_CASE(testUnusedLabelSwitchConfiguration);
        TEST_CASE(testUnusedLabelPremiumMisra);

        TEST_CASE(testEvaluationOrder);
        TEST_CASE(testEvaluationOrderSelfAssignment);
        TEST_CASE(testEvaluationOrderMacro);
        TEST_CASE(testEvaluationOrderSequencePointsFunctionCall);
        TEST_CASE(testEvaluationOrderSequencePointsComma);
        TEST_CASE(testEvaluationOrderSizeof);

        TEST_CASE(testUnsignedLessThanZero);

        TEST_CASE(doubleMove1);
        TEST_CASE(doubleMoveMemberInitialization1);
        TEST_CASE(doubleMoveMemberInitialization2);
        TEST_CASE(doubleMoveMemberInitialization3); // #9974
        TEST_CASE(doubleMoveMemberInitialization4);
        TEST_CASE(moveAndAssign1);
        TEST_CASE(moveAndAssign2);
        TEST_CASE(moveAssignMoveAssign);
        TEST_CASE(moveAndReset1);
        TEST_CASE(moveAndReset2);
        TEST_CASE(moveResetMoveReset);
        TEST_CASE(moveAndFunctionParameter);
        TEST_CASE(moveAndFunctionParameterReference);
        TEST_CASE(moveAndFunctionParameterConstReference);
        TEST_CASE(moveAndFunctionParameterUnknown);
        TEST_CASE(moveAndReturn);
        TEST_CASE(moveAndClear);
        TEST_CASE(movedPointer);
        TEST_CASE(moveAndAddressOf);
        TEST_CASE(partiallyMoved);
        TEST_CASE(moveAndLambda);
        TEST_CASE(moveInLoop);
        TEST_CASE(moveCallback);
        TEST_CASE(moveClassVariable);
        TEST_CASE(forwardAndUsed);
        TEST_CASE(moveAndReference);
        TEST_CASE(moveForRange);
        TEST_CASE(moveTernary);
        TEST_CASE(movePointerAlias);
        TEST_CASE(moveOutparam);
        TEST_CASE(moveTryEmplace);

        TEST_CASE(funcArgNamesDifferent);
        TEST_CASE(funcArgOrderDifferent);
        TEST_CASE(cpp11FunctionArgInit); // #7846 - "void foo(int declaration = {}) {"

        TEST_CASE(shadowVariables);
        TEST_CASE(knownArgument);
        TEST_CASE(knownArgumentHiddenVariableExpression);
        TEST_CASE(knownArgumentTernaryOperator);
        TEST_CASE(checkComparePointers);

        TEST_CASE(unusedVariableValueTemplate); // #8994

        TEST_CASE(moduloOfOne);

        TEST_CASE(sameExpressionPointers);

        TEST_CASE(checkOverlappingWrite);

        TEST_CASE(constVariableArrayMember); // #10371

        TEST_CASE(knownPointerToBool);
        TEST_CASE(iterateByValue);

        TEST_CASE(knownConditionFloating);
        TEST_CASE(knownConditionPrefixed);

        TEST_CASE(unionZeroInitBasic);
        TEST_CASE(unionZeroInitArrayMember);
        TEST_CASE(unionZeroInitStructMember);
        TEST_CASE(unionZeroInitUnknownType);
        TEST_CASE(unionZeroInitBitfields);
    }

    struct CheckOptions
    {
        bool cpp = true;
        bool inconclusive = true;
        bool verbose = false;
        Settings* settings = nullptr; // TODO: split from this
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& opt = make_default_obj{}) {
        // TODO: do not modify object passed into
        Settings* settings;
        if (!opt.settings) {
            settings = &settings1;
        }
        else {
            settings = opt.settings;
        }
        settings->certainty.setEnabled(Certainty::inconclusive, opt.inconclusive);
        settings->verbose = opt.verbose;

        // Tokenize..
        SimpleTokenizer tokenizer(*settings, *this, opt.cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check..
        runChecks<CheckOther>(tokenizer, this);
    }

    struct CheckPOptions
    {
        bool cpp = true;
    };

#define checkP(...) checkP_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkP_(const char* file, int line, const char (&code)[size], const CheckPOptions& options = make_default_obj()) {
        SimpleTokenizer2 tokenizer(settings2, *this, code, options.cpp ? "test.cpp" : "test.c");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check..
        runChecks<CheckOther>(tokenizer, this);
    }

    template<size_t size>
    void checkInterlockedDecrement(const char (&code)[size]) {
        /*const*/ Settings settings = settingsBuilder().platform(Platform::Type::Win32A).build();

        check(code, dinit(CheckOptions, $.inconclusive = false, $.settings = &settings));
    }

    void emptyBrackets() {
        check("{\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    void zeroDiv1() { // floating point division by zero => no error
        check("void foo() {\n"
              "    cout << 1. / 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    cout << 42 / (double)0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    cout << 42 / (float)0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    cout << 42 / (int)0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv2() {
        check("void foo()\n"
              "{\n"
              "    int sum = 0;\n"
              "    for(int i = 0; i < n; i ++)\n"
              "    {\n"
              "        sum += i;\n"
              "    }\n"
              "    cout<<b/sum;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void zeroDiv3() {
        check("int foo(int i) {\n"
              "    return i / 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (error) Division by zero. [zerodiv]\n", errout_str());

        check("int foo(int i) {\n"
              "    return i % 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (error) Division by zero. [zerodiv]\n", errout_str());

        check("void foo(int& i) {\n"
              "    i /= 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (error) Division by zero. [zerodiv]\n", errout_str());

        check("void foo(int& i) {\n"
              "    i %= 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (error) Division by zero. [zerodiv]\n", errout_str());

        check("uint8_t foo(uint8_t i) {\n"
              "    return i / 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv4() {
        check("void f()\n"
              "{\n"
              "   long a = b / 0x6;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0x0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(long b)\n"
              "{\n"
              "   long a = b / 0x0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Division by zero. [zerodiv]\n", errout_str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0L;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(long b)\n"
              "{\n"
              "   long a = b / 0L;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Division by zero. [zerodiv]\n", errout_str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0ul;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(long b)\n"
              "{\n"
              "   long a = b / 0ul;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Division by zero. [zerodiv]\n", errout_str());

        // Don't warn about floating points (gcc doesn't warn either)
        // and floating points are handled differently than integers.
        check("void f()\n"
              "{\n"
              "   long a = b / 0.0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "   long a = b / 0.5;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void zeroDiv5() {
        check("void f()\n"
              "{ { {\n"
              "   long a = b / 0;\n"
              "} } }");
        ASSERT_EQUALS("", errout_str());
        check("void f(long b)\n"
              "{ { {\n"
              "   long a = b / 0;\n"
              "} } }");
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv6() {
        check("void f()\n"
              "{ { {\n"
              "   int a = b % 0;\n"
              "} } }");
        ASSERT_EQUALS("", errout_str());
        check("void f(int b)\n"
              "{ { {\n"
              "   int a = b % 0;\n"
              "} } }");
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv7() {
        // unknown types for x and y --> do not warn
        check("void f() {\n"
              "  int a = x/2*3/0;\n"
              "  int b = y/2*3%0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(int x, int y) {\n"
              "  int a = x/2*3/0;\n"
              "  int b = y/2*3%0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (error) Division by zero. [zerodiv]\n"
                      "[test.cpp:3:16]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv8() {
        // #5584 - FP when function is unknown
        check("void f() {\n"
              "  int a = 0;\n"
              "  do_something(a);\n"
              "  return 4 / a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error, inconclusive) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv9() {
        // #6403 FP zerodiv - inside protecting if-clause
        check("void foo() {\n"
              "  double fStepHelp = 0;\n"
              "   if( (rOuterValue >>= fStepHelp) ) {\n"
              "     if( fStepHelp != 0.0) {\n"
              "       double fStepMain = 1;\n"
              "       sal_Int32 nIntervalCount = static_cast< sal_Int32 >(fStepMain / fStepHelp);\n"
              "    }\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void zeroDiv10() {
        // #5402 false positive: (error) Division by zero -- with boost::format
        check("int main() {\n"
              "  std::cout\n"
              "    << boost::format(\" %d :: %s <> %s\") % 0 % \"a\" % \"b\"\n"
              "    << std::endl;\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void zeroDiv11() {
        check("void f(int a) {\n"
              "  int res = (a+2)/0;\n"
              "  int res = (a*2)/0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (error) Division by zero. [zerodiv]\n"
                      "[test.cpp:3:18]: (error) Division by zero. [zerodiv]\n", errout_str());
        check("void f() {\n"
              "  int res = (a+2)/0;\n"
              "  int res = (a*2)/0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void zeroDiv12() {
        // #8141
        check("intmax_t f() {\n"
              "  return 1 / imaxabs(0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (error) Division by zero. [zerodiv]\n", errout_str());
    }
    void zeroDiv13() {
        // #7324
        check("int f () {\n"
              "    int dividend = 10;\n"
              "        int divisor = 1;\n"
              "    dividend = dividend / (--divisor);\n"
              "    return dividend;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:25]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv14() {
        check("void f() {\n" // #1169
              "    double dx = 1.;\n"
              "    int ix = 1;\n"
              "    int i = 1;\n"
              "    std::cout << ix / (i >> 1) << std::endl;\n"
              "    std::cout << dx / (i >> 1) << std::endl;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:21]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv15() { // #8319
        check("int f(int i) { return i - 1; }\n"
              "int f() {\n"
              "    const int d = 1;\n"
              "    const int r = 1 / f(d);\n"
              "    return r;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:21]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    // #11158
    void zeroDiv16()
    {
        check("int f(int i) {\n"
              "    int number = 10, a = 0;\n"
              "    for (int count = 0; count < 2; count++) {\n"
              "        a += (i / number) % 10;\n"
              "        number = number / 10;\n"
              "    }\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int i) {\n"
              "    int number = 10, a = 0;\n"
              "    for (int count = 0; count < 2; count++) {\n"
              "        int x = number / 10;\n"
              "        a += (i / number) % 10;\n"
              "        number = x;\n"
              "    }\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void zeroDiv17() { // #9931
        check("int f(int len) {\n"
              "    int sz = sizeof(void*[255]) / 255;\n"
              "    int x = len % sz;\n"
              "    return x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void zeroDiv18()
    {
        check("int f(int x, int y) {\n"
              "    if (x == y) {}\n"
              "    return 1 / (x-y);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:11] -> [test.cpp:3:14]: (warning) Either the condition 'x==y' is redundant or there is division by zero at line 3. [zerodivcond]\n",
            errout_str());
    }

    void zeroDiv19()
    {
        check("void f() {\n" // #2456
              "    for (int i = 0;;)\n"
              "        int j = 10 / i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:20]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv20()
    {
        check("uint16_t f(void)\n" // #11175
              "{\n"
              "    uint16_t x = 0xFFFFU;\n" // UINT16_MAX=0xFFFF
              "    return 42/(++x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:14]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv21()
    {
        check("int f(int n) {\n"
              "    return 1 / ((1 / n) - 1);\n"
              "}\n"
              "int g() {\n"
              "    return f(1);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:14]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDiv22()
    {
        check("int main() {\n"
              "    return _Generic((0), int: (42) / (0), default: 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:36]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void zeroDivCond() {
        check("void f(unsigned int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x > 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:2:14]: (warning) Either the condition 'x>0' is redundant or there is division by zero at line 2. [zerodivcond]\n", errout_str());

        check("void f(unsigned int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x >= 1) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:2:14]: (warning) Either the condition 'x>=1' is redundant or there is division by zero at line 2. [zerodivcond]\n", errout_str());

        check("void f(int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x == 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:2:14]: (warning) Either the condition 'x==0' is redundant or there is division by zero at line 2. [zerodivcond]\n", errout_str());

        check("void f(unsigned int x) {\n"
              "  int y = 17 / x;\n"
              "  if (x != 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:2:14]: (warning) Either the condition 'x!=0' is redundant or there is division by zero at line 2. [zerodivcond]\n", errout_str());

        // function call
        check("void f1(int x, int y) { c=x/y; }\n"
              "void f2(unsigned int y) {\n"
              "    f1(123,y);\n"
              "    if (y>0){}\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:10] -> [test.cpp:1:28]: (warning) Either the condition 'y>0' is redundant or there is division by zero at line 1. [zerodivcond]\n",
            errout_str());

        // avoid false positives when variable is changed after division
        check("void f() {\n"
              "  unsigned int x = do_something();\n"
              "  int y = 17 / x;\n"
              "  x = some+calculation;\n"
              "  if (x != 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        {
            // function is called that might modify global variable
            check("void do_something();\n"
                  "int x;\n"
                  "void f() {\n"
                  "  int y = 17 / x;\n"
                  "  do_something();\n"
                  "  if (x != 0) {}\n"
                  "}");
            ASSERT_EQUALS("", errout_str());

            // function is called. but don't care, variable is local
            check("void do_something();\n"
                  "void f() {\n"
                  "  int x = some + calculation;\n"
                  "  int y = 17 / x;\n"
                  "  do_something();\n"
                  "  if (x != 0) {}\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:6:9] -> [test.cpp:4:14]: (warning) Either the condition 'x!=0' is redundant or there is division by zero at line 4. [zerodivcond]\n", errout_str());
        }

        check("void do_something(int value);\n"
              "void f(int x) {\n"
              "  int y = 17 / x;\n"
              "  do_something(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int x;\n"
              "void f() {\n"
              "  int y = 17 / x;\n"
              "  while (y || x == 0) { x--; }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket 5033 segmentation fault (valid code) in CheckOther::checkZeroDivisionOrUselessCondition
        check("void f() {\n"
              "double* p1= new double[1];\n"
              "double* p2= new double[1];\n"
              "double* p3= new double[1];\n"
              "double* pp[3] = {p1,p2,p3};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5105 - FP
        check("int f(int a, int b) {\n"
              "  int r = a / b;\n"
              "  if (func(b)) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Unknown types for b and c --> do not warn
        check("int f(int d) {\n"
              "  int r = (a?b:c) / d;\n"
              "  if (d == 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int a) {\n"
              "  int r = a ? 1 / a : 0;\n"
              "  if (a == 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int a) {\n"
              "  int r = (a == 0) ? 0 : 1 / a;\n"
              "  if (a == 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int g();\n"
              "void f(int b) {\n"
              "  int x = g();\n"
              "  if (x == 0) {}\n"
              "  else if (x > 0) {}\n"
              "  else\n"
              "    a = b / -x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int x;\n"
              "};\n"
              "int f(A* a) {\n"
              "    if (a->x == 0) \n"
              "        a->x = 1;\n"
              "    return 1/a->x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10049
        check("int f(int argc) {\n"
              "    int quotient, remainder;\n"
              "    remainder = argc % 2;\n"
              "    argc = 2;\n"
              "    quotient = argc;\n"
              "    if (quotient != 0) \n"
              "        return quotient;\n"
              "    return remainder;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11315
        checkP("#define STATIC_ASSERT(c) \\\n"
               "do { enum { sa = 1/(int)(!!(c)) }; } while (0)\n"
               "void f() {\n"
               "    STATIC_ASSERT(sizeof(int) == sizeof(FOO));\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11505
        check("void f(uint16_t num, uint8_t radix) {\n"
              "    int c = num % radix;\n"
              "    num /= radix;\n"
              "    if (!num) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nanInArithmeticExpression() {
        check("void f()\n"
              "{\n"
              "   double x = 3.0 / 0.0 + 1.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (style) Using NaN/Inf in a computation. [nanInArithmeticExpression]\n", errout_str());

        check("void f()\n"
              "{\n"
              "   double x = 3.0 / 0.0 - 1.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (style) Using NaN/Inf in a computation. [nanInArithmeticExpression]\n", errout_str());

        check("void f()\n"
              "{\n"
              "   double x = 1.0 + 3.0 / 0.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:25]: (style) Using NaN/Inf in a computation. [nanInArithmeticExpression]\n", errout_str());

        check("void f()\n"
              "{\n"
              "   double x = 1.0 - 3.0 / 0.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:25]: (style) Using NaN/Inf in a computation. [nanInArithmeticExpression]\n", errout_str());

        check("void f()\n"
              "{\n"
              "   double x = 3.0 / 0.0;\n"
              "   printf(\"%f\", x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

    }

    void varScope1() {
        check("unsigned short foo()\n"
              "{\n"
              "    test_client CClient;\n"
              "    try\n"
              "    {\n"
              "        if (CClient.Open())\n"
              "        {\n"
              "            return 0;\n"
              "        }\n"
              "    }\n"
              "    catch (...)\n"
              "    {\n"
              "        return 2;\n"
              "    }\n"
              "\n"
              "    try\n"
              "    {\n"
              "        CClient.Close();\n"
              "    }\n"
              "    catch (...)\n"
              "    {\n"
              "        return 2;\n"
              "    }\n"
              "\n"
              "    return 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope2() {
        check("int foo()\n"
              "{\n"
              "    Error e;\n"
              "    e.SetValue(12);\n"
              "    throw e;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope3() {
        check("void foo()\n"
              "{\n"
              "    int i;\n"
              "    int *p = 0;\n"
              "    if (abc)\n"
              "    {\n"
              "        p = &i;\n"
              "    }\n"
              "    *p = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope4() {
        check("void foo()\n"
              "{\n"
              "    int i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope5() {
        check("void f(int x)\n"
              "{\n"
              "    int i = 0;\n"
              "    if (x) {\n"
              "        for ( ; i < 10; ++i) ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) The scope of the variable 'i' can be reduced. [variableScope]\n", errout_str());

        check("void f(int x) {\n"
              "    const unsigned char i = 0;\n"
              "    if (x) {\n"
              "        for ( ; i < 10; ++i) ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x)\n"
              "{\n"
              "    int i = 0;\n"
              "    if (x) {b()}\n"
              "    else {\n"
              "        for ( ; i < 10; ++i) ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) The scope of the variable 'i' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope6() {
        check("void f(int x)\n"
              "{\n"
              "    int i = x;\n"
              "    if (a) {\n"
              "        x++;\n"
              "    }\n"
              "    if (b) {\n"
              "        c(i);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #5398
              "    bool success = false;\n"
              "    int notReducable(someClass.getX(&success));\n"
              "    if (success) {\n"
              "        foo(notReducable);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(Test &test) {\n"
              "  int& x = test.getData();\n"
              "  if (test.process())\n"
              "    x = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "int foo = 0;\n"
              "std::vector<int> vec(10);\n"
              "BOOST_FOREACH(int& i, vec)\n"
              "{\n"
              " foo += 1;\n"
              " if(foo == 10)\n"
              " {\n"
              "  return 0;\n"
              " }\n"
              "}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int &x)\n"
              "{\n"
              "  int n = 1;\n"
              "  do\n"
              "  {\n"
              "    ++n;\n"
              "    ++x;\n"
              "  } while (x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope7() {
        check("void f(int x)\n"
              "{\n"
              "    int y = 0;\n"
              "    b(y);\n"
              "    if (x) {\n"
              "        y++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope8() {
        check("void test() {\n"
              "    float edgeResistance=1;\n"
              "    std::vector<int> edges;\n"
              "    BOOST_FOREACH(int edge, edges) {\n"
              "        edgeResistance = (edge+1) / 2.0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) The scope of the variable 'edgeResistance' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope9() {
        // classes may have extra side effects
        check("class fred {\n"
              "public:\n"
              "    fred();\n"
              "    void x();\n"
              "};\n"
              "void test(int a) {\n"
              "    fred f;\n"
              "    if (a == 2) {\n"
              "        f.x();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class fred {\n" // #2062
              "public:\n"
              "    void x();\n"
              "};\n"
              "void test(int a) {\n"
              "    fred f;\n"
              "    if (a == 2) {\n"
              "        f.x();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:10]: (style) The scope of the variable 'f' can be reduced. [variableScope]\n", errout_str());

        check("struct S { int a, b; };\n"
              "bool f() {\n"
              "    S s{};\n"
              "    {\n"
              "        bool b = s.a && a.b;\n"
              "        return b;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) The scope of the variable 's' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope10() {
        check("int f()\n"
              "{\n"
              "    int x = 0;\n"
              "    FOR {\n"
              "        foo(x++);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope11() {
        check("int f() {\n"
              "    int x = 0;\n"
              "    AB ab = { x, 0 };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    int x = 0;\n"
              "    if (a == 0) { ++x; }\n"
              "    AB ab = { x, 0 };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    int x = 0;\n"
              "    if (a == 0) { ++x; }\n"
              "    if (a == 1) { AB ab = { x, 0 }; }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope12() {
        check("void f(int x) {\n"
              "    int i[5];\n"
              "    int* j = y;\n"
              "    if (x)\n"
              "        foo(i);\n"
              "    foo(j);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) The scope of the variable 'i' can be reduced. [variableScope]\n", errout_str());

        check("void f(int x) {\n"
              "    int i[5];\n"
              "    int* j;\n"
              "    if (x)\n"
              "        j = i;\n"
              "    foo(j);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    const bool b = true;\n"
              "    x++;\n"
              "    if (x == 5)\n"
              "        foo(b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    const bool b = x;\n"
              "    x++;\n"
              "    if (x == 5)\n"
              "        foo(b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope13() {
        // #2770
        check("void f() {\n"
              "    int i = 0;\n"
              "    forever {\n"
              "        if (i++ == 42) { break; }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope14() {
        // #3941
        check("void f() {\n"
              "    const int i( foo());\n"
              "    if(a) {\n"
              "        for ( ; i < 10; ++i) ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope15() {
        // #4573
        check("void f() {\n"
              "    int a,b,c;\n"
              "    if (a);\n"
              "    else if(b);\n"
              "    else if(c);\n"
              "    else;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
    }

    void varScope16() {
        check("void f() {\n"
              "    int a = 0;\n"
              "    while((++a) < 56) {\n"
              "        foo();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    do {\n"
              "        foo();\n"
              "    } while((++a) < 56);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    do {\n"
              "        a = 64;\n"
              "        foo(a);\n"
              "    } while((++a) < 56);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    do {\n"
              "        a = 64;\n"
              "        foo(a);\n"
              "    } while(z());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) The scope of the variable 'a' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope17() {
        check("void f() {\n"
              "    int x;\n"
              "    if (a) {\n"
              "        x = stuff(x);\n"
              "        morestuff(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) The scope of the variable 'x' can be reduced. [variableScope]\n", errout_str());

        check("void f() {\n"
              "    int x;\n"
              "    if (a) {\n"
              "        x = stuff(x);\n"
              "        morestuff(x);\n"
              "    }\n"
              "    if (b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) The scope of the variable 'x' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope18() {
        check("void f() {\n"
              "    short x;\n"
              "\n"
              "    switch (ab) {\n"
              "        case A:\n"
              "            break;\n"
              "        case B:\n"
              "        default:\n"
              "            break;\n"
              "    }\n"
              "\n"
              "    if (c) {\n"
              "        x = foo();\n"
              "        do_something(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) The scope of the variable 'x' can be reduced. [variableScope]\n", errout_str());

        check("void f() {\n"
              "    short x;\n"
              "\n"
              "    switch (ab) {\n"
              "        case A:\n"
              "            x = 10;\n"
              "            break;\n"
              "        case B:\n"
              "        default:\n"
              "            break;\n"
              "    }\n"
              "\n"
              "    if (c) {\n"
              "        x = foo();\n"
              "        do_something(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    short x;\n"
              "\n"
              "    switch (ab) {\n"
              "        case A:\n"
              "            if(c)\n"
              "                do_something(x);\n"
              "            break;\n"
              "        case B:\n"
              "        default:\n"
              "            break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) The scope of the variable 'x' can be reduced. [variableScope]\n", errout_str());

        check("void f() {\n"
              "    short x;\n"
              "\n"
              "    switch (ab) {\n"
              "        case A:\n"
              "            if(c)\n"
              "                do_something(x);\n"
              "            break;\n"
              "        case B:\n"
              "        default:\n"
              "            if(d)\n"
              "                do_something(x);\n"
              "            break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope20() { // Ticket #5103 - constant variable only used in inner scope
        check("int f(int a) {\n"
              "  const int x = 234;\n"
              "  int b = a;\n"
              "  if (b > 32) b = x;\n"
              "  return b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope21() { // Ticket #5382 - initializing two-dimensional array
        check("int test() {\n"
              "    int test_value = 3;\n"
              "    int test_array[1][1] = { { test_value } };\n"
              "    return sizeof(test_array);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope22() { // Ticket #5684 - "The scope of the variable 'p' can be reduced" - But it can not.
        check("void foo() {\n"
              "   int* p( 42 );\n"
              "   int i = 0;\n"
              "   while ( i != 100 ) {\n"
              "      *p = i;\n"
              "      ++p;\n"
              "      ++i;\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        // try to avoid an obvious false negative after applying the fix for the example above:
        check("void foo() {\n"
              "   int* p( 42 );\n"
              "   int i = 0;\n"
              "   int dummy = 0;\n"
              "   while ( i != 100 ) {\n"
              "      p = & dummy;\n"
              "      *p = i;\n"
              "      ++p;\n"
              "      ++i;\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) The scope of the variable 'p' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope23() { // #6154: Don't suggest to reduce scope if inner scope is a lambda
        check("int main() {\n"
              "   size_t myCounter = 0;\n"
              "   Test myTest([&](size_t aX){\n"
              "       std::cout << myCounter += aX << std::endl;\n"
              "   });\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope24() {
        check("void f(Foo x) {\n"
              "   Foo &r = x;\n"
              "   if (cond) {\n"
              "       r.dostuff();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) The scope of the variable 'r' can be reduced. [variableScope]\n", errout_str());

        check("void f(Foo x) {\n"
              "   Foo foo = x;\n"
              "   if (cond) {\n"
              "       foo.dostuff();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope25() {
        check("void f() {\n"
              "    time_t currtime;\n"
              "    if (a) {\n"
              "        currtime = time(&dummy);\n"
              "        if (currtime > t) {}\n"
              "    }\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:2:12]: (style) The scope of the variable 'currtime' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope26() {
        check("void f(const std::map<int,int> &m) {\n"
              "  for (auto it : m) {\n"
              "     if (cond1) {\n"
              "       int& key = it.first;\n"
              "       if (cond2) { dostuff(key); }\n"
              "     }\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope27() {
        checkP("void f() {\n"
               "  int x = 0;\n"
               "#ifdef X\n"
               "#endif\n"
               "  if (id == ABC) { return x; }\n"
               "}");
        ASSERT_EQUALS("", errout_str());

        checkP("void f() {\n"
               "#ifdef X\n"
               "#endif\n"
               "  int x = 0;\n"
               "  if (id == ABC) { return x; }\n"
               "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) The scope of the variable 'x' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope28() {
        check("void f() {\n" // #10527
              "    int i{};\n"
              "    if (double d = g(i); d == 1.0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope29() { // #10888
        check("enum E { E0 };\n"
              "struct S { int i; };\n"
              "void f(int b) {\n"
              "    enum E e;\n"
              "    struct S s;\n"
              "    if (b) {\n"
              "        e = E0;\n"
              "        s.i = 0;\n"
              "        g(e, s);\n"
              "    }\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:4:12]: (style) The scope of the variable 'e' can be reduced. [variableScope]\n"
                      "[test.c:5:14]: (style) The scope of the variable 's' can be reduced. [variableScope]\n",
                      errout_str());

        check("void f(bool b) {\n"
              "    std::string s;\n"
              "    if (b) {\n"
              "        s = \"abc\";\n"
              "        g(s);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) The scope of the variable 's' can be reduced. [variableScope]\n", errout_str());

        check("auto foo(std::vector<int>& vec, bool flag) {\n"
              "    std::vector<int> dummy;\n"
              "    std::vector<int>::iterator iter;\n"
              "    if (flag)\n"
              "        iter = vec.begin();\n"
              "    else {\n"
              "        dummy.push_back(42);\n"
              "        iter = dummy.begin();\n"
              "    }\n"
              "    return *iter;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:28]: (style) Parameter 'vec' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("auto& foo(std::vector<int>& vec, bool flag) {\n"
              "    std::vector<int> dummy;\n"
              "    std::vector<int>::iterator iter;\n"
              "    if (flag)\n"
              "        iter = vec.begin();\n"
              "    else {\n"
              "        dummy.push_back(42);\n"
              "        iter = dummy.begin();\n"
              "    }\n"
              "    return *iter;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope30() { // #8541
        check("bool f(std::vector<int>& v, int i) {\n"
              "    int n = 0;\n"
              "    bool b = false;\n"
              "    std::for_each(v.begin(), v.end(), [&](int j) {\n"
              "        if (j == i) {\n"
              "            ++n;\n"
              "            if (n > 5)\n"
              "                b = true;\n"
              "        }\n"
              "    });\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope31() { // #11099
        check("bool g(std::vector<int>&);\n"
              "void h(std::vector<int>);\n"
              "void f0(std::vector<int> v) {\n"
              "    std::vector<int> w{ v };\n"
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f1(std::vector<int> v) {\n"
              "    std::vector<int> w{ v.begin(), v.end() };\n"
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f2(std::vector<int> v) {\n"
              "    std::vector<int> w{ 10, 0, std::allocator<int>() };\n" // FN
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f3(std::vector<int> v) {\n"
              "    std::vector<int> w{ 10, 0 };\n" // warn
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f4(std::vector<int> v) {\n"
              "    std::vector<int> w{ 10 };\n" // warn
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f5(std::vector<int> v) {\n"
              "    std::vector<int> w(v);\n"
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f6(std::vector<int> v) {\n"
              "    std::vector<int> w(v.begin(), v.end());\n"
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f7(std::vector<int> v) {\n"
              "    std::vector<int> w(10, 0, std::allocator<int>);\n" // FN
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f8(std::vector<int> v) {\n"
              "    std::vector<int> w(10, 0);\n" // warn
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f9(std::vector<int> v) {\n"
              "    std::vector<int> w(10);\n" // warn
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n"
              "void f10(std::vector<int> v) {\n"
              "    std::vector<int> w{};\n" // warn
              "    bool b = g(v);\n"
              "    if (b)\n"
              "        h(w);\n"
              "    h(v);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:25:22]: (style) The scope of the variable 'w' can be reduced. [variableScope]\n"
                      "[test.cpp:32:22]: (style) The scope of the variable 'w' can be reduced. [variableScope]\n"
                      "[test.cpp:60:22]: (style) The scope of the variable 'w' can be reduced. [variableScope]\n"
                      "[test.cpp:67:22]: (style) The scope of the variable 'w' can be reduced. [variableScope]\n"
                      "[test.cpp:74:22]: (style) The scope of the variable 'w' can be reduced. [variableScope]\n",
                      errout_str());
    }

    void varScope32() { // #11441
        check("template <class F>\n"
              "std::vector<int> g(F, const std::vector<int>&);\n"
              "void f(const std::vector<int>&v) {\n"
              "    std::vector<int> w;\n"
              "    for (auto x : v)\n"
              "        w = g([&]() { x; }, w);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:23]: (warning) Unused variable value 'x' [constStatement]\n", errout_str());
    }

    void varScope33() { // #11131
        check("struct S {\n"
              "    const std::string& getStr() const;\n"
              "    void mutate();\n"
              "    bool getB() const;\n"
              "};\n"
              "void g(S& s) {\n"
              "    std::string str = s.getStr();\n"
              "    s.mutate();\n"
              "    if (s.getB()) {\n"
              "        if (str == \"abc\") {}\n"
              "    }\n"
              "}\n"
              "void g(char* s, bool b) {\n"
              "    int i = strlen(s);\n"
              "    s[0] = '\\0';\n"
              "    if (b) {\n"
              "        if (i == 5) {}\n"
              "    }\n"
              "}\n"
              "void f(const S& s) {\n"
              "    std::string str = s.getStr();\n"
              "    std::string str2{ s.getStr() };\n"
              "    std::string str3(s.getStr());\n"
              "    if (s.getB()) {\n"
              "        if (str == \"abc\") {}\n"
              "        if (str2 == \"abc\") {}\n"
              "        if (str3 == \"abc\") {}\n"
              "    }\n"
              "}\n"
              "void f(const char* s, bool b) {\n"
              "    int i = strlen(s);\n"
              "    if (b) {\n"
              "        if (i == 5) {}\n"
              "    }\n"
              "}\n"
              "void f(int j, bool b) {\n"
              "    int k = j;\n"
              "    if (b) {\n"
              "        if (k == 5) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:21:17]: (style) The scope of the variable 'str' can be reduced. [variableScope]\n"
                      "[test.cpp:22:17]: (style) The scope of the variable 'str2' can be reduced. [variableScope]\n"
                      "[test.cpp:23:17]: (style) The scope of the variable 'str3' can be reduced. [variableScope]\n"
                      "[test.cpp:31:9]: (style) The scope of the variable 'i' can be reduced. [variableScope]\n"
                      "[test.cpp:37:9]: (style) The scope of the variable 'k' can be reduced. [variableScope]\n",
                      errout_str());
    }

    void varScope34() { // #11742
        check("void f() {\n"
              "    bool b = false;\n"
              "    int i = 1;\n"
              "    for (int k = 0; k < 20; ++k) {\n"
              "        b = !b;\n"
              "        if (b)\n"
              "            i++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope35() { // #11845
        check("void f(int err, const char* src) {\n"
              "    const char* msg = \"Success\";\n"
              "    char buf[42];\n"
              "    if (err != 0)\n"
              "        msg = strcpy(buf, src);\n"
              "    printf(\"%d: %s\\n\", err, msg);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("char* g(char* dst, const char* src);\n"
              "void f(int err, const char* src) {\n"
              "    const char* msg = \"Success\";\n"
              "    char buf[42];\n"
              "    if (err != 0)\n"
              "        msg = g(buf, src);\n"
              "    printf(\"%d: %s\\n\", err, msg);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("char* g(char* dst, const char* src);\n"
              "void f(int err, const char* src) {\n"
              "    const char* msg = \"Success\";\n"
              "    char buf[42];\n"
              "    if (err != 0)\n"
              "        g(buf, src);\n"
              "    printf(\"%d: %s\\n\", err, msg);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) The scope of the variable 'buf' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope36() {
        // #12158
        check("void f( uint32_t value ) {\n"
              "    uint32_t i = 0U;\n"
              "    if ( value > 100U ) { }\n"
              "    else if( value > 50U ) { }\n"
              "    else{\n"
              "         for( i = 0U; i < 5U; i++ ) {}\n"
              "    }\n"
              "}\n", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:2:14]: (style) The scope of the variable 'i' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope37() {
        // #12158
        check("void f( uint32_t value ) {\n"
              "    uint32_t i = 0U;\n"
              "    if ( value > 100U ) { }\n"
              "    else {\n"
              "        if( value > 50U ) { }\n"
              "        else{\n"
              "            for( i = 0U; i < 5U; i++ ) {}\n"
              "        }\n"
              "    }\n"
              "}\n", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:2:14]: (style) The scope of the variable 'i' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope38() {
        checkP("bool dostuff();\n" // #12519
               "#define DOSTUFF(c) if (c < 5) { if (c) b = dostuff(); }\n"
               "#define DOSTUFFEX(c) { bool b = false; DOSTUFF(c); }\n"
               "void f(int a) {\n"
               "    DOSTUFFEX(a);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope39() {
        check("struct S {\n" // #12405
              "    void f(const std::string& s) const;\n"
              "    const int* g(std::string&) const;\n"
              "};\n"
              "void h(int);\n"
              "void S::f(const std::string& s) const {\n"
              "    std::string n = s;\n"
              "    const int* a = g(n);\n"
              "    if (n == \"abc\") {\n"
              "        h(a[0]);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void varScope40() {
        checkP("#define NUM (-999.9)\n" // #8862
               "double f(int i) {\n"
               "    double a = NUM;\n"
               "    double b = -NUM;\n"
               "    double c = -1.0 * NUM;\n"
               "    if (i == 1) {\n"
               "        return a;\n"
               "    }\n"
               "    if (i == 2) {\n"
               "        return b;\n"
               "    }\n"
               "    if (i == 3) {\n"
               "        return c;\n"
               "    }\n"
               "    return 0.0;\n"
               "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) The scope of the variable 'a' can be reduced. [variableScope]\n"
                      "[test.cpp:4:12]: (style) The scope of the variable 'b' can be reduced. [variableScope]\n"
                      "[test.cpp:5:12]: (style) The scope of the variable 'c' can be reduced. [variableScope]\n",
                      errout_str());

        check("struct S { int a; };\n" // #12618
              "int f(const S* s, int i) {\n"
              "    int x = s->a;\n"
              "    const int b[] = { 1, 2, 3 };\n"
              "    int y = b[1];\n"
              "    if (i)\n"
              "        return x + y;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) The scope of the variable 'x' can be reduced. [variableScope]\n"
                      "[test.cpp:5:9]: (style) The scope of the variable 'y' can be reduced. [variableScope]\n",
                      errout_str());
    }

    void varScope41() {    // #11845
        check("void get_errmsg(const char **msg, char *buf, size_t bufsiz, int err);\n"
              "void test(int err)\n"
              "{\n"
              "    const char *msg = \"Success\";\n"
              "    char buf[42];\n"
              "    if (err != 0)\n"
              "        get_errmsg(&msg, buf, sizeof(buf), err);\n"
              "    printf(\"%d: %s\\n\", err, msg);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void get_errmsg(char *buf, size_t bufsiz, int err);\n"
              "void test(int err)\n"
              "{\n"
              "    const char *msg = \"Success\";\n"
              "    char buf[42];\n"
              "    if (err != 0)\n"
              "        get_errmsg(buf, sizeof(buf), err);\n"
              "    printf(\"%d: %s\\n\", err, msg);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:10]: (style) The scope of the variable 'buf' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope42() {
        check("void f(const char **, char *);\n"
              "void g(int e) {\n"
              "    const char *msg = \"Something\";\n"
              "    char buf[42];\n"
              "    if (e != 0)\n"
              "        f(&msg, buf);\n"
              "    printf(\"result: %s\\n\", msg);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(char *, char *);\n"
              "void g(int e) {\n"
              "    char msg [42] = \"Something\";\n"
              "    char buf[42];\n"
              "    if (e != 0)\n"
              "        f(msg, buf);\n"
              "    printf(\"result: %s\\n\", msg);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const char *, char *);\n"
              "void g(int e) {\n"
              "    const char *msg = \"Something\";\n"
              "    char buf[42];\n"
              "    if (e != 0)\n"
              "        f(msg, buf);\n"
              "    printf(\"result: %s\\n\", msg);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) The scope of the variable 'buf' can be reduced. [variableScope]\n", errout_str());

        check("void f(int **, char *);\n"
              "void g(int e) {\n"
              "    int *msg = calloc(0, sizeof(*msg));\n"
              "    char buf[42];\n"
              "    if (e != 0)\n"
              "        f(&msg, buf);\n"
              "    printf(\"result: %d\\n\", *msg);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) The scope of the variable 'buf' can be reduced. [variableScope]\n", errout_str());

        check("void f(const char *&, const char *&);\n"
              "void g(int e) {\n"
              "    const char *msg = \"Something\";\n"
              "    char *buf = malloc(42);\n"
              "    if (e != 0)\n"
              "        f(msg, buf);\n"
              "    printf(\"result: %d\\n\", msg);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(const char* format, ...);\n"
              "void f(bool b) {\n"
              "    const char* s = \"abc\";\n"
              "    if (b)\n"
              "       g(\"%d %s\", 1, s);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) The scope of the variable 's' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope43() {
        check("struct S { int a, b; };\n" // #13838
              "int f(S s) {\n"
              "    auto& [x, y] = s;\n"
              "    if (x < 5) {\n"
              "        return y;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int a; };\n"
              "int f(S s) {\n"
              "    auto& [x] = s;\n"
              "    if (y) {\n"
              "        return x;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) The scope of the variable 'x' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope44() { // #14496
        check("char* f() {\n"
              "    char* p = nullptr;\n"
              "    {\n"
              "        p = strdup(\"abc\");\n"
              "        if (p) {\n"
              "            return p;\n"
              "        }\n"
              "    }\n"
              "    return nullptr;\n"
              "}\n"
              "char* g() {\n"
              "    char* q = NULL;\n"
              "    {\n"
              "        q = strdup(\"abc\");\n"
              "        if (q) {\n"
              "            return q;\n"
              "        }\n"
              "    }\n"
              "    return nullptr;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) The scope of the variable 'p' can be reduced. [variableScope]\n"
                      "[test.cpp:12:11]: (style) The scope of the variable 'q' can be reduced. [variableScope]\n",
                      errout_str());
    }

    void varScope45() {
        check("void g(int x, int y) {\n" // #14497
              "    int a = x, b = y;\n"
              "    if (a) {}\n"
              "    else {\n"
              "        if (b) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) The scope of the variable 'b' can be reduced. [variableScope]\n", errout_str());
    }

    void varScope46() {
        check("void f() {\n" // #7091
              "    int y1;\n"
              "    for (int i = 0; i < 3; ++i) {\n"
              "        for(int j = 0; j < 3; ++j) {\n"
              "                y1 = 2 * 1;\n"
              "                y1 += 1;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) The scope of the variable 'y1' can be reduced. [variableScope]\n",
                      errout_str());

        check("bool f() {\n"
              "bool b = false;\n"
              "do {\n"
              "    switch (g()) {\n"
              "        case 0:\n"
              "            b = true;\n"
              "            break;\n"
              "        case 1:\n"
              "            return b;\n"
              "            break;\n"
              "        default:\n"
              "            break;\n"
              "    }\n"
              "}\n"
              "while (true);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int y1 = 0;\n"
              "    for (int i = 0; i < 3; ++i) {\n"
              "        for(int j = 0; j < 3; ++j) {\n"
              "                y1 = y1 + 1;\n"
              "                dostuff(y1);\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int& r) {\n" // #14566
              "    int i = 0;\n"
              "    while (g()) {\n"
              "        {\n"
              "            if (g()) {\n"
              "                i = 0;"
              "                std::swap(i, r);\n"
              "            }\n"
              "        }\n"
              "    }\n"
              "    use(i);"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

#define checkOldStylePointerCast(...) checkOldStylePointerCast_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkOldStylePointerCast_(const char* file, int line, const char (&code)[size], Standards::cppstd_t std = Standards::CPPLatest) {

        const Settings settings = settingsBuilder().severity(Severity::warning).severity(Severity::style).cpp(std).build();

        // Tokenize..
        SimpleTokenizer tokenizerCpp(settings, *this);
        ASSERT_LOC(tokenizerCpp.tokenize(code), file, line);

        CheckOther checkOtherCpp(&tokenizerCpp, &settings, this);
        checkOtherCpp.warningOldStylePointerCast();
        checkOtherCpp.warningDangerousTypeCast();
    }

    void oldStylePointerCast() {
        checkOldStylePointerCast("class Base{};\n"
                                 "class Derived: public Base {};\n"
                                 "void foo(Base* base)\n"
                                 "{\n"
                                 "    Derived * d = (Derived *) base;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:5:19]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        checkOldStylePointerCast("class Base{};\n"
                                 "class Derived: public Base {};\n"
                                 "void foo(Derived* derived)\n"
                                 "{\n"
                                 "    Base * b = (Base *) derived;\n" // <- cast from derived to base is safe => cstyleCast
                                 "}");
        ASSERT_EQUALS("[test.cpp:5:16]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        checkOldStylePointerCast("void foo(Base* base)\n"
                                 "{\n"
                                 "    Derived * d = (Derived *) base;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        checkOldStylePointerCast("class Base{};\n"
                                 "class Derived: public Base {};\n"
                                 "void foo(Base* base)\n"
                                 "{\n"
                                 "    Derived * d = (const Derived *) base;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:5:19]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        checkOldStylePointerCast("class Base{};\n"
                                 "class Derived: public Base {};\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Derived * d = (const Derived *) ( new Base() );\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:5:19]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        checkOldStylePointerCast("class Base{};\n"
                                 "class Derived: public Base {};\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Derived * d = (const Derived *) new Base();\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:5:19]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        checkOldStylePointerCast("class Base{};\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (Base *) new short[10];\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (volatile Base *) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (volatile Base * const) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const volatile Base *) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const volatile Base * const) derived;\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) ( new Derived() );\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) new Derived();\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) new short[10];\n"
                                 "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        checkOldStylePointerCast("class B;\n"
                                 "class A\n"
                                 "{\n"
                                 "  virtual void abc(B *) const = 0;\n"
                                 "}");
        ASSERT_EQUALS("", errout_str());

        checkOldStylePointerCast("class B;\n"
                                 "class A\n"
                                 "{\n"
                                 "  virtual void abc(const B *) const = 0;\n"
                                 "}");
        ASSERT_EQUALS("", errout_str());

        // #3630
        checkOldStylePointerCast("class SomeType{};\n"
                                 "class X : public Base {\n"
                                 "    X() : Base((SomeType*)7) {}\n" // <- intToPointerCast
                                 "};");
        ASSERT_EQUALS("", errout_str());

        checkOldStylePointerCast("class SomeType{};\n"
                                 "class X : public Base {\n"
                                 "    X() : Base((SomeType*)0x7000) {}\n" // <- it's common in embedded code to cast address
                                 "};");
        ASSERT_EQUALS("", errout_str());

        checkOldStylePointerCast("class SomeType;\n"
                                 "class X : public Base {\n"
                                 "    X() : Base((SomeType*)var) {}\n"
                                 "};");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        checkOldStylePointerCast("class SomeType;\n"
                                 "class X : public Base {\n"
                                 "    X() : Base((SomeType*)0) {}\n"
                                 "};");
        ASSERT_EQUALS("", errout_str());

        // #5560
        checkOldStylePointerCast("class C;\n"
                                 "\n"
                                 "class B\n"
                                 "{ virtual G* createGui(S*, C*) const = 0; };\n"
                                 "\n"
                                 "class MS : public M\n"
                                 "{ virtual void addController(C*) override {} };", Standards::CPP03);
        ASSERT_EQUALS("", errout_str());

        // #6164
        checkOldStylePointerCast("class Base {};\n"
                                 "class Derived: public Base {};\n"
                                 "void testCC() {\n"
                                 "  std::vector<Base*> v;\n"
                                 "  v.push_back((Base*)new Derived);\n"
                                 "}");
        // FIXME write a dangerousTypeCast warning instead
        ASSERT_EQUALS("[test.cpp:5:15]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        // #7709
        checkOldStylePointerCast("typedef struct S S;\n"
                                 "typedef struct S SS;\n"
                                 "typedef class C C;\n"
                                 "typedef long LONG;\n"
                                 "typedef long* LONGP;\n"
                                 "struct T {};\n"
                                 "typedef struct T TT;\n"
                                 "typedef struct T2 {} TT2;\n"
                                 "void f(int* i) {\n"
                                 "    S* s = (S*)i;\n"
                                 "    SS* ss = (SS*)i;\n"
                                 "    struct S2* s2 = (struct S2*)i;\n"
                                 "    C* c = (C*)i;\n"
                                 "    class C2* c2 = (class C2*)i;\n"
                                 "    long* l = (long*)i;\n"
                                 "    LONG* l2 = (LONG*)i;\n"
                                 "    LONGP l3 = (LONGP)i;\n"
                                 "    TT* tt = (TT*)i;\n"
                                 "    TT2* tt2 = (TT2*)i;\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:10:12]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:11:14]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:12:21]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:13:12]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:14:20]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:15:15]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:16:16]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:17:16]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:18:14]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:19:16]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n",
                      errout_str());

        // #8649
        checkOldStylePointerCast("struct S {};\n"
                                 "void g(S*& s);\n"
                                 "void f(uintptr_t i) {\n"
                                 "    g((S*&)i);\n"
                                 "    S*& r = (S*&)i;\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) C-style pointer casting [cstyleCast]\n"
                      "[test.cpp:5:13]: (style) C-style pointer casting [cstyleCast]\n",
                      errout_str());

        checkOldStylePointerCast("struct S {};\n"
                                 "void g(S*& s);\n"
                                 "void f(uint8_t i) {\n"
                                 "    g((S*&)i);\n"
                                 "    S*& r = (S*&)i;\n"
                                 "}\n");
        // TODO: these conversions are dangerous, but it's a different issue not covered by cstyleCast. A separate checker can be added which is executed for both C and C++ code.
        // clang says: 1.cpp:5:18: warning: cast to 'unsigned char *' from smaller integer type 'uint8_t' (aka 'unsigned char') [-Wint-to-pointer-cast]
        ASSERT_EQUALS("[test.cpp:4:7]: (style) C-style pointer casting [cstyleCast]\n"
                      "[test.cpp:5:13]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        // #10823
        checkOldStylePointerCast("void f(void* p) {\n"
                                 "    auto h = reinterpret_cast<void (STDAPICALLTYPE*)(int)>(p);\n"
                                 "}\n");
        ASSERT_EQUALS("", errout_str());

        // #5210
        checkOldStylePointerCast("class Base {};\n"
                                 "class Derived: public Base {};\n"
                                 "void f(Base** b1, Base*** b2) {\n"
                                 "    Derived** p1 = (Derived**)b1;\n"
                                 "    Derived*** p2 = (Derived***)b2;\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4:20]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:5:21]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n",
                      errout_str());

        // #12446
        checkOldStylePointerCast("namespace N { struct S {}; }\n"
                                 "union U {\n"
                                 "    int i;\n"
                                 "    char c[4];\n"
                                 "};\n"
                                 "void f(void* p) {\n"
                                 "    auto ps = (N::S*)p;\n"
                                 "    auto pu = (union U*)p;\n"
                                 "    auto pv = (std::vector<int>*)(p);\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:7:15]: (style) C-style pointer casting [cstyleCast]\n"
                      "[test.cpp:8:15]: (style) C-style pointer casting [cstyleCast]\n"
                      "[test.cpp:9:15]: (style) C-style pointer casting [cstyleCast]\n", errout_str());

        // #12447
        checkOldStylePointerCast("class Base {};\n"
                                 "class Derived: public Base {};\n"
                                 "void f(const Base& base) {\n"
                                 "  d = (const Derived&)base;\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4:7]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        // #11430
        checkOldStylePointerCast("struct B {\n"
                                 "    float* data() const;\n"
                                 "};\n"
                                 "namespace N {\n"
                                 "    bool f(float* v);\n"
                                 "}\n"
                                 "bool g(B& b) {\n"
                                 "    using float_ptr = float*;\n"
                                 "    return N::f(float_ptr(b.data()));\n" // <- the cast is safe
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:9:17]: (style) C-style pointer casting [cstyleCast]\n", errout_str());
    }

#define checkIntToPointerCast(...) checkIntToPointerCast_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkIntToPointerCast_(const char* file, int line, const char (&code)[size]) {

        const Settings settings = settingsBuilder().severity(Severity::portability).build();

        // Tokenize..
        SimpleTokenizer tokenizerCpp(settings, *this);
        ASSERT_LOC(tokenizerCpp.tokenize(code), file, line);

        CheckOther checkOtherCpp(&tokenizerCpp, &settings, this);
        checkOtherCpp.warningIntToPointerCast();
    }

    void intToPointerCast() {
        // #3630
        checkIntToPointerCast("uint8_t* ptr = (uint8_t*)7;");
        ASSERT_EQUALS("[test.cpp:1:16]: (portability) Casting non-zero decimal integer literal to pointer. [intToPointerCast]\n", errout_str());

        checkIntToPointerCast("void* ptr = (void*)7;");
        ASSERT_EQUALS("[test.cpp:1:13]: (portability) Casting non-zero decimal integer literal to pointer. [intToPointerCast]\n", errout_str());

        checkIntToPointerCast("uint8_t* ptr = (uint8_t*)0;");
        ASSERT_EQUALS("", errout_str());

        checkIntToPointerCast("uint8_t* ptr = (uint8_t*)0x7000;"); // <- it's common in embedded code to cast address
        ASSERT_EQUALS("", errout_str());

        checkIntToPointerCast("struct S { int i; };\n" // #13886, don't crash
                              "int f() { return sizeof(((struct S*)0)->i); }");
        ASSERT_EQUALS("", errout_str());
    }

    struct CheckInvalidPointerCastOptions
    {
        bool portability = true;
        bool inconclusive = false;
    };

#define checkInvalidPointerCast(...) checkInvalidPointerCast_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkInvalidPointerCast_(const char* file, int line, const char (&code)[size], const CheckInvalidPointerCastOptions& opt = make_default_obj{}) {
        /*const*/ Settings settings = settingsBuilder().severity(Severity::warning).severity(Severity::portability, opt.portability).certainty(Certainty::inconclusive, opt.inconclusive).build();
        settings.platform.defaultSign = 's';

        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        CheckOther checkOtherCpp(&tokenizer, &settings, this);
        checkOtherCpp.invalidPointerCast();
    }


    void invalidPointerCast() {
        checkInvalidPointerCast("void test() {\n"
                                "    float *f = new float[10];\n"
                                "    delete [] (double*)f;\n"
                                "    delete [] (long double const*)(new float[10]);\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (portability) Casting between float * and double * which have an incompatible binary data representation. [invalidPointerCast]\n"
                      "[test.cpp:4:15]: (portability) Casting between float * and const long double * which have an incompatible binary data representation. [invalidPointerCast]\n", errout_str());

        checkInvalidPointerCast("void test(const float* f) {\n"
                                "    double *d = (double*)f;\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (portability) Casting between const float * and double * which have an incompatible binary data representation. [invalidPointerCast]\n", errout_str());

        checkInvalidPointerCast("void test(double* d1) {\n"
                                "    long double *ld = (long double*)d1;\n"
                                "    double *d2 = (double*)ld;\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2:23]: (portability) Casting between double * and long double * which have an incompatible binary data representation. [invalidPointerCast]\n"
                      "[test.cpp:3:18]: (portability) Casting between long double * and double * which have an incompatible binary data representation. [invalidPointerCast]\n", errout_str());

        checkInvalidPointerCast("char* test(int* i) {\n"
                                "    long double *d = (long double*)(i);\n"
                                "    double *d = (double*)(i);\n"
                                "    float *f = reinterpret_cast<float*>(i);\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (portability) Casting between signed int * and long double * which have an incompatible binary data representation. [invalidPointerCast]\n"
                      "[test.cpp:3:17]: (portability) Casting between signed int * and double * which have an incompatible binary data representation. [invalidPointerCast]\n"
                      "[test.cpp:4:16]: (portability) Casting between signed int * and float * which have an incompatible binary data representation. [invalidPointerCast]\n", errout_str());

        checkInvalidPointerCast("float* test(unsigned int* i) {\n"
                                "    return (float*)i;\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (portability) Casting between unsigned int * and float * which have an incompatible binary data representation. [invalidPointerCast]\n", errout_str());

        checkInvalidPointerCast("float* test(unsigned int* i) {\n"
                                "    return (float*)i[0];\n"
                                "}");
        ASSERT_EQUALS("", errout_str());

        checkInvalidPointerCast("float* test(double& d) {\n"
                                "    return (float*)&d;\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (portability) Casting between double * and float * which have an incompatible binary data representation. [invalidPointerCast]\n", errout_str());

        checkInvalidPointerCast("void test(float* data) {\n"
                                "    f.write((char*)data,sizeof(float));\n"
                                "}");
        ASSERT_EQUALS("", errout_str());

        checkInvalidPointerCast("void test(float* data) {\n"
                                "    f.write((char*)data,sizeof(float));\n"
                                "}", dinit(CheckInvalidPointerCastOptions, $.inconclusive = true)); // #3639
        ASSERT_EQUALS("[test.cpp:2:13]: (portability, inconclusive) Casting from float * to char * is not portable due to different binary data representations on different platforms. [invalidPointerCast]\n", errout_str());


        checkInvalidPointerCast("long long* test(float* f) {\n"
                                "    return (long long*)f;\n"
                                "}", dinit(CheckInvalidPointerCastOptions, $.portability = false));
        ASSERT_EQUALS("", errout_str());

        checkInvalidPointerCast("long long* test(float* f, char* c) {\n"
                                "    foo((long long*)f);\n"
                                "    return reinterpret_cast<long long*>(c);\n"
                                "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (portability) Casting from float * to signed long long * is not portable due to different binary data representations on different platforms. [invalidPointerCast]\n", errout_str());

        checkInvalidPointerCast("Q_DECLARE_METATYPE(int*)"); // #4135 - don't crash
    }


    void passedByValue() {
        check("void f(const std::string str) {}");
        ASSERT_EQUALS("[test.cpp:1:26]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(std::unique_ptr<std::string> ptr) {}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::shared_ptr<std::string> ptr) {}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::function<F> ptr) {}");
        ASSERT_EQUALS("", errout_str());

        {
            check("void f(const std::pair<int,int> x) {}");
            ASSERT_EQUALS("", errout_str());

            check("void f(const std::pair<std::string,std::string> x) {}");
            TODO_ASSERT_EQUALS("error", "", errout_str());
        }

        check("void f(const std::string::size_type x) {}");
        ASSERT_EQUALS("", errout_str());

        check("class Foo;\nvoid f(const Foo foo) {}"); // Unknown class
        ASSERT_EQUALS("[test.cpp:2:18]: (performance, inconclusive) Function parameter 'foo' should be passed by const reference. [passedByValue]\n", errout_str());

        check("class Foo { std::vector<int> v; };\nvoid f(const Foo foo) {}"); // Large class (STL member)
        ASSERT_EQUALS("[test.cpp:2:18]: (performance) Function parameter 'foo' should be passed by const reference. [passedByValue]\n", errout_str());

        check("class Foo { int i; };\nvoid f(const Foo foo) {}"); // Small class
        ASSERT_EQUALS("", errout_str());

        check("class Foo { int i[6]; };\nvoid f(const Foo foo) {}"); // Large class (array)
        ASSERT_EQUALS("[test.cpp:2:18]: (performance) Function parameter 'foo' should be passed by const reference. [passedByValue]\n", errout_str());

        check("class Foo { std::string* s; };\nvoid f(const Foo foo) {}"); // Small class (pointer)
        ASSERT_EQUALS("", errout_str());

        check("class Foo { static std::string s; };\nvoid f(const Foo foo) {}"); // Small class (static member)
        ASSERT_EQUALS("", errout_str());

        check("class X { std::string s; }; class Foo : X { };\nvoid f(const Foo foo) {}"); // Large class (inherited)
        ASSERT_EQUALS("[test.cpp:2:18]: (performance) Function parameter 'foo' should be passed by const reference. [passedByValue]\n", errout_str());

        check("class X { std::string s; }; class Foo { X x; };\nvoid f(const Foo foo) {}"); // Large class (inherited)
        ASSERT_EQUALS("[test.cpp:2:18]: (performance) Function parameter 'foo' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(const std::string &str) {}");
        ASSERT_EQUALS("", errout_str());

        // The idiomatic way of passing a std::string_view is by value
        check("void f(const std::string_view str) {}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string_view str) {}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::string_view &str) {}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::vector<int> v) {}");
        ASSERT_EQUALS("[test.cpp:1:31]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(const std::vector<std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1:39]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(const std::vector<std::string>::size_type s) {}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::vector<int> &v) {}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::map<int,int> &v) {}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::map<int,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1:32]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(const std::map<std::string,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1:48]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(const std::map<int,std::string> v) {}");
        ASSERT_EQUALS("[test.cpp:1:40]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(const std::map<std::string,int> v) {}");
        ASSERT_EQUALS("[test.cpp:1:40]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(const std::streamoff pos) {}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::initializer_list<int> i) {}");
        ASSERT_EQUALS("", errout_str());

        // #5824
        check("void log(const std::string& file, int line, const std::string& function, const std::string str, ...) {}");
        ASSERT_EQUALS("", errout_str());

        // #5534
        check("struct float3 { };\n"
              "typedef float3 vec;\n"
              "class Plane {\n"
              "    vec Refract(vec &vec) const;\n"
              "    bool IntersectLinePlane(const vec &planeNormal);\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class X {\n"
              "    virtual void func(const std::string str) {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:2:41]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("enum X;\n"
              "void foo(X x1){}\n");
        ASSERT_EQUALS("", errout_str());

        check("enum X { a, b, c };\n"
              "void foo(X x2){}\n");
        ASSERT_EQUALS("", errout_str());

        check("enum X { a, b, c };\n"
              "enum X;"
              "void foo(X x3){}\n");
        ASSERT_EQUALS("", errout_str());

        check("enum X;\n"
              "enum X { a, b, c };"
              "void foo(X x4){}\n");
        ASSERT_EQUALS("", errout_str());

        check("union U {\n"
              "    char* pc;\n"
              "    short* ps;\n"
              "    int* pi;\n"
              "};\n"
              "void f(U u) {}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { char A[8][8]; };\n"
              "void f(S s) {}\n");
        ASSERT_EQUALS("[test.cpp:2:10]: (performance) Function parameter 's' should be passed by const reference. [passedByValue]\n", errout_str());

        check("union U {\n" // don't crash
              "    int a;\n"
              "    decltype(nullptr) b;\n"
              "};\n"
              "int* f(U u) { return u.b; }\n");
        ASSERT_EQUALS("", errout_str());

        check("struct B { virtual int f(std::string s) = 0; };\n" // #11432
              "struct D1 : B {\n"
              "  int f(std::string s) override { s += 'a'; return s.size(); }\n"
              "}\n"
              "struct D2 : B {\n"
              "  int f(std::string s) override { return s.size(); }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int x(int);\n"
              "void f(std::vector<int> v, int& j) {\n"
              "    for (int i : v)\n"
              "        j = i;\n"
              "}\n"
              "void fn(std::vector<int> v) {\n"
              "    for (int& i : v)\n"
              "        i = x(i);\n"
              "}\n"
              "void g(std::vector<int> v, int& j) {\n"
              "    for (int i = 0; i < v.size(); ++i)\n"
              "        j = v[i];\n"
              "}\n"
              "void gn(std::vector<int> v) {\n"
              "    for (int i = 0; i < v.size(); ++i)\n"
              "        v[i] = x(i);\n"
              "}\n"
              "void h(std::vector<std::vector<int>> v, int& j) {\n"
              "    for (int i = 0; i < v.size(); ++i)\n"
              "        j = v[i][0];\n"
              "}\n"
              "void hn(std::vector<std::vector<int>> v) {\n"
              "    for (int i = 0; i < v.size(); ++i)\n"
              "        v[i][0] = x(i);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:25]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n"
                      "[test.cpp:10:25]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n"
                      "[test.cpp:18:38]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n",
                      errout_str());

        check("struct S {\n" // #11995
              "    explicit S(std::string s) noexcept;\n"
              "    std::string m;\n"
              "};\n"
              "S::S(std::string s) noexcept : m(std::move(s)) {}\n"
              "struct T {\n"
              "    explicit S(std::string s) noexcept(true);\n"
              "    std::string m;\n"
              "};\n"
              "T::T(std::string s) noexcept(true) : m(std::move(s)) {}\n");
        ASSERT_EQUALS("", errout_str());

        check("namespace N {\n" // #12086
              "    void g(int);\n"
              "}\n"
              "void f(std::vector<int> v) {\n"
              "    N::g(v[0]);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:25]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(const std::string& s, std::string t) {\n" // #12083
              "    const std::string& v = !s.empty() ? s : t;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:42]: (performance) Function parameter 't' should be passed by const reference. [passedByValue]\n", errout_str());

        /*const*/ Settings settingsUnix64 = settingsBuilder(settings0).platform(Platform::Type::Unix64).build();
        check("struct S {\n" // #12138
              "    union {\n"
              "        int a = 0;\n"
              "        int x;\n"
              "    };\n"
              "    union {\n"
              "        int b = 0;\n"
              "        int y;\n"
              "    };\n"
              "    union {\n"
              "        int c = 0;\n"
              "        int z;\n"
              "    };\n"
              "};\n"
              "void f(S s) {\n"
              "    if (s.x > s.y) {}\n"
              "}\n", dinit(CheckOptions, $.settings = &settingsUnix64));
        ASSERT_EQUALS("", errout_str());

        check("struct S { std::list<int> l; };\n" // #12147
              "class C { public: std::list<int> l; };\n"
              "bool f(S s) {\n"
              "    return s.l.empty();\n"
              "}\n"
              "bool f(C c) {\n"
              "    return c.l.empty();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10]: (performance) Function parameter 's' should be passed by const reference. [passedByValue]\n"
                      "[test.cpp:6:10]: (performance) Function parameter 'c' should be passed by const reference. [passedByValue]\n",
                      errout_str());

        check("struct S { std::list<int> a[1][1]; };\n"
              "bool f(S s) {\n"
              "    return s.a[0][0].empty();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:10]: (performance) Function parameter 's' should be passed by const reference. [passedByValue]\n",
                      errout_str());

        check("struct S {\n"
              "    enum class E : std::uint8_t { E0 };\n"
              "    static void f(S::E e) {\n"
              "        if (e == S::E::E0) {}\n"
              "    }\n"
              "    char a[20];\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::vector<int> v[2]);\n" // #13052
              "void g(const std::vector<int> v[2]);\n"
              "void g(const std::vector<int> v[2]) {}\n"
              "int h(const std::array<std::vector<int>, 2> a) { return a[0][0]; }\n");
        ASSERT_EQUALS("[test.cpp:4:45]: (performance) Function parameter 'a' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(const std::array<int, 10> a[]) {}\n"); // #13524
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #10733
              "    std::vector<int> v;\n"
              "    using F = void (*)();\n"
              "    void func(F f);\n"
              "};\n"
              "void S::func(S::F f) {}\n");
        ASSERT_EQUALS("", errout_str());

        /*const*/ Settings settingsWin64 = settingsBuilder().platform(Platform::Type::Win64).build();
        check("using ui64 = unsigned __int64;\n"
              "ui64 Test(ui64 one, ui64 two) { return one + two; }\n",
              dinit(CheckOptions, $.settings = &settingsWin64));
        ASSERT_EQUALS("", errout_str());
    }

    void passedByValue_nonConst() {
        check("void f(std::string str) {}");
        ASSERT_EQUALS("[test.cpp:1:20]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(std::string str) {\n"
              "    return str + x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:20]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(std::string str) {\n"
              "    std::cout << str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:20]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(std::string str) {\n"
              "    std::cin >> str;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string str) {\n"
              "    std::string s2 = str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (performance, inconclusive) Use const reference for 's2' to avoid unnecessary data copying. [redundantCopyLocalConst]\n"
                      "[test.cpp:1:20]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n",
                      errout_str());

        check("void f(std::string str) {\n"
              "    std::string& s2 = str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:20]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n"
                      "[test.cpp:2:18]: (style) Variable 's2' can be declared as reference to const [constVariableReference]\n",
                      errout_str());

        check("void f(std::string str) {\n"
              "    const std::string& s2 = str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:20]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void f(std::string str) {\n"
              "    str = \"\";\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string str) {\n"
              "    foo(str);\n" // It could be that foo takes str as non-const-reference
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const std::string& str);\n"
              "void f(std::string str) {\n"
              "    foo(str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void foo(std::string str);\n"
              "void f(std::string str) {\n"
              "    foo(str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("void foo(std::string& str);\n"
              "void f(std::string str) {\n"
              "    foo(str);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(std::string* str);\n"
              "void f(std::string str) {\n"
              "    foo(&str);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int& i1, const std::string& str, int& i2);\n"
              "void f(std::string str) {\n"
              "    foo((a+b)*c, str, x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("std::string f(std::string str) {\n"
              "    str += x;\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class X {\n"
              "    std::string s;\n"
              "    void func() const;\n"
              "};\n"
              "Y f(X x) {\n"
              "    x.func();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:7]: (performance) Function parameter 'x' should be passed by const reference. [passedByValue]\n", errout_str());

        check("class X {\n"
              "    void func();\n"
              "};\n"
              "Y f(X x) {\n"
              "    x.func();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class X {\n"
              "    void func(std::string str) {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:2:27]: (performance) Function parameter 'str' should be passed by const reference. [passedByValue]\n", errout_str());

        check("class X {\n"
              "    virtual void func(std::string str) {}\n" // Do not warn about virtual functions, if 'str' is not declared as const
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class X {\n"
              "    char a[1024];\n"
              "};\n"
              "class Y : X {\n"
              "    char b;\n"
              "};\n"
              "void f(Y y) {\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:10]: (performance) Function parameter 'y' should be passed by const reference. [passedByValue]\n", errout_str());

        check("class X {\n"
              "    void* a;\n"
              "    void* b;\n"
              "};\n"
              "class Y {\n"
              "    void* a;\n"
              "    void* b;\n"
              "    char c;\n"
              "};\n"
              "void f(X x, Y y) {\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:15]: (performance) Function parameter 'y' should be passed by const reference. [passedByValue]\n", errout_str());

        {
            // 8-byte data should be passed by const reference on 32-bit platform but not on 64-bit platform
            const char code[] = "class X {\n"
                                "    uint64_t a;\n"
                                "    uint64_t b;\n"
                                "};\n"
                                "void f(X x) {}";

            /*const*/ Settings s32 = settingsBuilder(settings1).platform(Platform::Type::Unix32).build();
            check(code, dinit(CheckOptions, $.settings = &s32));
            ASSERT_EQUALS("[test.cpp:5:10]: (performance) Function parameter 'x' should be passed by const reference. [passedByValue]\n", errout_str());

            /*const*/ Settings s64 = settingsBuilder(settings1).platform(Platform::Type::Unix64).build();
            check(code, dinit(CheckOptions, $.settings = &s64));
            ASSERT_EQUALS("", errout_str());
        }

        check("Writer* getWriter();\n"
              "\n"
              "void foo(Buffer& buffer) {\n"
              "    getWriter()->operator<<(buffer);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void passedByValue_externC() {
        check("struct X { int a[5]; }; void f(X v) { }");
        ASSERT_EQUALS("[test.cpp:1:34]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n", errout_str());

        check("extern \"C\" { struct X { int a[5]; }; void f(X v) { } }");
        ASSERT_EQUALS("", errout_str());

        check("struct X { int a[5]; }; extern \"C\" void f(X v) { }");
        ASSERT_EQUALS("", errout_str());

        check("struct X { int a[5]; }; void f(const X v);");
        ASSERT_EQUALS("[test.cpp:1:40]: (performance) Function parameter 'v' should be passed by const reference. [passedByValue]\n", errout_str());

        check("extern \"C\" { struct X { int a[5]; }; void f(const X v); }");
        ASSERT_EQUALS("", errout_str());

        check("struct X { int a[5]; }; extern \"C\" void f(const X v) { }");
        ASSERT_EQUALS("", errout_str());
    }

    void constVariable() {
        check("int f(std::vector<int> x) {\n"
              "    int& i = x[0];\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:24]: (performance) Function parameter 'x' should be passed by const reference. [passedByValue]\n"
                      "[test.cpp:2:10]: (style) Variable 'i' can be declared as reference to const [constVariableReference]\n",
                      errout_str());

        check("int f(std::vector<int>& x) {\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:25]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("int f(std::vector<int> x) {\n"
              "    const int& i = x[0];\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:24]: (performance) Function parameter 'x' should be passed by const reference. [passedByValue]\n", errout_str());

        check("int f(std::vector<int> x) {\n"
              "    static int& i = x[0];\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:24]: (performance) Function parameter 'x' should be passed by const reference. [passedByValue]\n", errout_str());

        check("int f(std::vector<int> x) {\n"
              "    int& i = x[0];\n"
              "    i++;\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int& f(std::vector<int>& x) {\n"
              "    x.push_back(1);\n"
              "    int& i = x[0];\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(const std::vector<int>& x) {\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int& f(std::vector<int>& x) {\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("const int& f(std::vector<int>& x) {\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:32]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("int f(std::vector<int>& x) {\n"
              "    x[0]++;\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int a; };\n"
              "A f(std::vector<A>& x) {\n"
              "    x[0].a = 1;\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int a(); };\n"
              "A f(std::vector<A>& x) {\n"
              "    x[0].a();\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int g(int& x);\n"
              "int f(std::vector<int>& x) {\n"
              "    g(x[0]);\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("template<class T>\n"
              "T f(T& x) {\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("template<class T>\n"
              "T f(T&& x) {\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("template<class T>\n"
              "T f(T& x) {\n"
              "    return x[0];\n"
              "}\n"
              "void h() { std::vector<int> v; h(v); }");
        ASSERT_EQUALS("", errout_str());

        check("int f(int& x) {\n"
              "    return std::move(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::ostream& os) {\n"
              "    os << \"Hello\";\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int*);\n"
              "void f(int& x) {\n"
              "    g(&x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { A(int*); };\n"
              "A f(int& x) {\n"
              "    return A(&x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { A(int*); };\n"
              "A f(int& x) {\n"
              "    return A{&x};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int& x, int& y) {\n"
              "    y++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:13]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("struct A {\n"
              "    explicit A(int& y) : x(&y) {}\n"
              "    int * x = nullptr;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    std::vector<int> v;\n"
              "    void swap(A& a) {\n"
              "        v.swap(a.v);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    template<class T>\n"
              "    void f();\n"
              "    template<class T>\n"
              "    void f() const;\n"
              "};\n"
              "void g(A& a) {\n"
              "    a.f<int>();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int>& v) {\n"
              "    for(auto&& x:v)\n"
              "        x = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int>& v) {\n"
              "    for(auto x:v)\n"
              "        x = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:26]: (style) Parameter 'v' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("void f(std::vector<int>& v) {\n"
              "    for(auto& x:v) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Variable 'x' can be declared as reference to const [constVariableReference]\n",
                      errout_str());

        check("void f(std::vector<int>& v) {\n" // #10980
              "    for (int& i : v)\n"
              "        if (i == 0) {}\n"
              "    for (const int& i : v)\n"
              "        if (i == 0) {}\n"
              "    for (auto& i : v)\n"
              "        if (i == 0) {}\n"
              "    for (const auto& i : v)\n"
              "        if (i == 0) {}\n"
              "    v.clear();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Variable 'i' can be declared as reference to const [constVariableReference]\n"
                      "[test.cpp:6:16]: (style) Variable 'i' can be declared as reference to const [constVariableReference]\n",
                      errout_str());

        check("void f(std::vector<int>& v) {\n"
              "    for(const auto& x:v) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:26]: (style) Parameter 'v' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("void f(int& i) {\n"
              "    int& j = i;\n"
              "    j++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int>& v) {\n"
              "    int& i = v[0];\n"
              "    i++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::map<unsigned int, std::map<std::string, unsigned int> >& m, unsigned int i) {\n"
              "    std::map<std::string, unsigned int>& members = m[i];\n"
              "    members.clear();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int& x;\n"
              "    A(int& y) : x(y)\n"
              "    {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    A(int& x);\n"
              "};\n"
              "struct B : A {\n"
              "    B(int& x) : A(x)\n"
              "    {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b, int& x, int& y) {\n"
              "  auto& z = x;\n"
              "  auto& w = b ? y : z;\n"
              "  w = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "  int i;\n"
              "};\n"
              "int& f(S& s) {\n"
              "  return s.i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int* f(std::list<int>& x, unsigned int y) {\n"
              "    for (int& m : x) {\n"
              "        if (m == y)\n"
              "            return &m;\n"
              "    }\n"
              "    return nullptr;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int& f(std::list<int>& x, int& y) {\n"
              "    for (int& m : x) {\n"
              "        if (m == y)\n"
              "            return m;\n"
              "    }\n"
              "    return y;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool from_string(int& t, const std::string& s) {\n"
              "    std::istringstream iss(s);\n"
              "    return !(iss >> t).fail();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9710
        check("class a {\n"
              "    void operator()(int& i) const {\n"
              "        i++;\n"
              "    }\n"
              "};\n"
              "void f(int& i) {\n"
              "    a()(i);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("class a {\n"
              "    void operator()(int& i) const {\n"
              "        i++;\n"
              "    }\n"
              "};\n"
              "void f(int& i) {\n"
              "    a x;\n"
              "    x(i);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("class a {\n"
              "    void operator()(const int& i) const;\n"
              "};\n"
              "void f(int& i) {\n"
              "    a x;\n"
              "    x(i);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Parameter 'i' can be declared as reference to const [constParameterReference]\n", errout_str());

        //cast or assignment to a non-const reference should prevent the warning
        check("struct T { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());
        check("struct T : public U  { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const T& z = x;\n" // Make sure we find all assignments
              "    T& y = x;\n"
              "    y.mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());
        check("struct T : public U  { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U& y = x\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());
        check("struct T : public U  { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    U& y = x;\n"
              "    y.mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());
        check("struct T : public U  { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    my<fancy>::type& y = x;\n" // we don't know if y is const or not
              "    y.mutate();\n"             // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());
        check("struct T : public U  { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U& y = static_cast<const U&>(x);\n"
              "    y.mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());
        check("struct T : public U  { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    U& y = static_cast<U&>(x);\n"
              "    y.mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U& y = dynamic_cast<const U&>(x)\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U& y = dynamic_cast<U const &>(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U& y = dynamic_cast<U & const>(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    U& y = dynamic_cast<U&>(x);\n"
              "    y.mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U& y = dynamic_cast<typename const U&>(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    U& y = dynamic_cast<typename U&>(x);\n"
              "    y.mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    U* y = dynamic_cast<U*>(&x);\n"
              "    y->mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U * y = dynamic_cast<const U *>(&x);\n"
              "    y->mutate();\n" // to avoid warnings that y can be const
              "}");
        TODO_ASSERT_EQUALS("can be const", errout_str(), ""); //Currently taking the address is treated as a non-const operation when it should depend on what we do with it
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    U const * y = dynamic_cast<U const *>(&x);\n"
              "    y->mutate();\n" // to avoid warnings that y can be const
              "}");
        TODO_ASSERT_EQUALS("can be const", errout_str(), ""); //Currently taking the address is treated as a non-const operation when it should depend on what we do with it
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U const * const * const * const y = dynamic_cast<const U const * const * const * const>(&x);\n"
              "    y->mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U const * const * const * const y = dynamic_cast<const U const * const * const * const>(&x);\n"
              "    y->mutate();\n" // to avoid warnings that y can be const
              "}");
        TODO_ASSERT_EQUALS("can be const", errout_str(), ""); //Currently taking the address is treated as a non-const operation when it should depend on what we do with it
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U const * const *  * const y = dynamic_cast<const U const * const *  * const>(&x);\n"
              "    y->mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    my::fancy<typename type const *> const * const  * const y = dynamic_cast<my::fancy<typename type const *> const * const  * const>(&x);\n"
              "    y->mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U& y = (const U&)(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:2:11]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n",
                      errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    U& y = (U&)(x);\n"
              "    y.mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    const U& y = (typename const U&)(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:2:11]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n",
                      errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    U& y = (typename U&)(x);\n"
              "    y.mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());
        check("struct T : public U { void dostuff() const {}};\n"
              "void a(T& x) {\n"
              "    x.dostuff();\n"
              "    U* y = (U*)(&x);\n"
              "    y->mutate();\n" // to avoid warnings that y can be const
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        check("struct C { void f() const; };\n" // #9875 - crash
              "\n"
              "void foo(C& x) {\n"
              "   x.f();\n"
              "   foo( static_cast<U2>(0) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Parameter 'x' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("class a {\n"
              "    void foo(const int& i) const;\n"
              "    void operator()(int& i) const;\n"
              "};\n"
              "void f(int& i) {\n"
              "    a()(i);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("class a {\n"
              "    void operator()(const int& i) const;\n"
              "};\n"
              "void f(int& i) {\n"
              "    a()(i);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Parameter 'i' can be declared as reference to const [constParameterReference]\n", errout_str());

        // #9767
        check("void fct1(MyClass& object) {\n"
              "   fct2([&](void){}, object);\n"
              "}\n"
              "bool fct2(std::function<void()> lambdaExpression, MyClass& object) {\n"
              "   object.modify();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9778
        check("struct A {};\n"
              "struct B : A {};\n"
              "B& f(A& x) {\n"
              "    return static_cast<B&>(x);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10002
        check("using A = int*;\n"
              "void f(const A& x) {\n"
              "    ++(*x);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10086
        check("struct V {\n"
              "    V& get(typename std::vector<V>::size_type i) {\n"
              "        std::vector<V>& arr = v;\n"
              "        return arr[i];\n"
              "    }\n"
              "    std::vector<V> v;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #14136
        check("void f(int& x) { (void)x; }\n");
        ASSERT_EQUALS("", errout_str());

        check("void e();\n"
              "void g(void);\n"
              "void h(void);\n"
              "void ah(void);\n"
              "void ai(void);\n"
              "void j(void);\n"
              "void e(void);\n"
              "void k(void);\n"
              "void l(void);\n"
              "void m(void);\n"
              "void n(void);\n"
              "void o(void);\n"
              "void q(void);\n"
              "void r(void);\n"
              "void t(void);\n"
              "void u(void);\n"
              "void v(void);\n"
              "void w(void);\n"
              "void z(void);\n"
              "void aj(void);\n"
              "void am(void);\n"
              "void g(void);\n"
              "void h(void);\n"
              "void ah(void);\n"
              "void an(void);\n"
              "void e(void);\n"
              "void k(void);\n"
              "void ao(wchar_t *d);\n"
              "void ah(void);\n"
              "void e(void);\n"
              "void an(void);\n"
              "void e(void);\n"
              "void k(void);\n"
              "void g(void);\n"
              "void ah(void);\n"
              "void an(void);\n"
              "void e(void);\n"
              "void e(void);\n"
              "void e(void);\n"
              "void k(void);\n"
              "void g(void);\n"
              "void ah(void);\n"
              "void an(void);\n"
              "void e(void);\n"
              "void e(void);\n"
              "void k(void);\n"
              "void g(void);\n"
              "void h(void);\n"
              "void ah(void);\n"
              "void an(void);\n"
              "void e(void);\n"
              "void k(void);\n"
              "void e(void);\n"
              "void g(void);\n"
              "void ah(void);\n"
              "void k(void);\n"
              "void an(void);\n"
              "void e(void);\n"
              "void e(void);\n"
              "void e(void);\n"
              "void k(void);\n"
              "void g(void);\n"
              "void h(void);\n"
              "void ah(void);\n"
              "void k(void);\n"
              "void an(void);\n"
              "void k(void);\n"
              "void e(void);\n"
              "void g(void);\n"
              "void ah(void);\n"
              "void e(void);\n"
              "void k(void);\n"
              "void g(void);\n"
              "void h(void);\n"
              "void ah(void);\n"
              "void an(void);\n"
              "void an(void);\n"
              "void k(void);\n"
              "void e(void);\n"
              "void e(void);\n"
              "void e(void);\n"
              "void g(void);\n"
              "void k(void);\n"
              "void g(void);\n"
              "void h(void);\n"
              "void ah(void);\n"
              "void an(void);\n"
              "void k(void);\n"
              "void k(void);\n"
              "void e(void);\n"
              "void g(void);\n"
              "void g(void);\n"
              "void ah(void);\n"
              "void an(void);\n"
              "void e(void);\n"
              "void k(void);\n"
              "void e(void);\n"
              "void ap(wchar_t *c, int d);\n"
              "void ah(void);\n"
              "void an(void);\n"
              "void g(void);\n"
              "void h(void);\n"
              "void ah(void);\n"
              "void aq(char *b, size_t d, char *c, int a);\n"
              "void ar(char *b, size_t d, char *c, va_list a);\n"
              "void k(void);\n"
              "void g(void);\n"
              "void g(void);\n"
              "void h(void);\n"
              "void ah(void);\n"
              "void an(void);\n"
              "void k(void);\n"
              "void k(void);\n"
              "void e(void);\n"
              "void g(void);\n"
              "void g(void);\n"
              "void as(std::string s);\n"
              "void at(std::ifstream &f);\n"
              "void au(std::istream &f);\n"
              "void av(std::string &aa, std::wstring &ab);\n"
              "void aw(bool b, double x, double y);\n"
              "void ax(int i);\n"
              "void ay(std::string c, std::wstring a);\n"
              "void az(const std::locale &ac);\n"
              "void an();\n"
              "void ba(std::ifstream &f);\n"
              "void bb(std::istream &f) {\n"
              "f.read(NULL, 0);\n"
              "}\n"
              "void h(void) {\n"
              "struct tm *tm = 0;\n"
              "(void)std::asctime(tm);\n"
              "(void)std::asctime(0);\n"
              "}\n"
              "void bc(size_t ae) {\n"
              "wchar_t *ad = 0, *af = 0;\n"
              "struct tm *ag = 0;\n"
              "(void)std::wcsftime(ad, ae, af, ag);\n"
              "(void)std::wcsftime(0, ae, 0, 0);\n"
              "}\n"
              "void k(void) {}\n"
              "void bd(void);\n"
              "void be(void);\n"
              "void bf(int b);\n"
              "void e(void);\n"
              "void e(void);\n"
              "void bg(wchar_t *p);\n"
              "void bh(const std::list<int> &ak, const std::list<int> &al);\n"
              "void ah();\n"
              "void an();\n"
              "void h();");
        ASSERT_EQUALS("[test.cpp:131:12]: (style) Variable 'tm' can be declared as pointer to const [constVariablePointer]\n"
                      "[test.cpp:136:19]: (style) Variable 'af' can be declared as pointer to const [constVariablePointer]\n"
                      "[test.cpp:137:12]: (style) Variable 'ag' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("class C\n"
              "{\n"
              "public:\n"
              "  explicit C(int&);\n"
              "};\n"
              "\n"
              "class D\n"
              "{\n"
              "public:\n"
              "  explicit D(int& i);\n"
              "\n"
              "private:\n"
              "  C c;\n"
              "};\n"
              "\n"
              "D::D(int& i)\n"
              "  : c(i)\n"
              "{\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class C\n"
              "{\n"
              "public:\n"
              "  explicit C(int&);\n"
              "};\n"
              "\n"
              "class D\n"
              "{\n"
              "public:\n"
              "  explicit D(int& i) noexcept;\n"
              "\n"
              "private:\n"
              "  C c;\n"
              "};\n"
              "\n"
              "D::D(int& i) noexcept\n"
              "  : c(i)\n"
              "{}");
        ASSERT_EQUALS("", errout_str());

        check("class C\n"
              "{\n"
              "public:\n"
              "  explicit C(const int&);\n"
              "};\n"
              "\n"
              "class D\n"
              "{\n"
              "public:\n"
              "  explicit D(int& i);\n"
              "\n"
              "private:\n"
              "  C c;\n"
              "};\n"
              "\n"
              "D::D(int& i)\n"
              "  : c(i)\n"
              "{\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:16]: (style) Parameter 'i' can be declared as reference to const\n", "", errout_str());

        check("class C\n"
              "{\n"
              "public:\n"
              "  explicit C(int);\n"
              "};\n"
              "\n"
              "class D\n"
              "{\n"
              "public:\n"
              "  explicit D(int& i);\n"
              "\n"
              "private:\n"
              "  C c;\n"
              "};\n"
              "\n"
              "D::D(int& i)\n"
              "  : c(i)\n"
              "{\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:16]: (style) Parameter 'i' can be declared as reference to const\n", "", errout_str());

        check("class C\n"
              "{\n"
              "public:\n"
              "  explicit C(int, int);\n"
              "};\n"
              "\n"
              "class D\n"
              "{\n"
              "public:\n"
              "  explicit D(int& i);\n"
              "\n"
              "private:\n"
              "  C c;\n"
              "};\n"
              "\n"
              "D::D(int& i)\n"
              "  : c(0, i)\n"
              "{\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:16]: (style) Parameter 'i' can be declared as reference to const\n", "", errout_str());

        check("void f(std::map<int, std::vector<int>> &map) {\n" // #10266
              "  for (auto &[slave, panels] : map)\n"
              "    panels.erase(it);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S { void f(); int i; };\n"
              "void call_f(S& s) { (s.*(&S::f))(); }\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int a[1]; };\n"
              "void f(S& s) { int* p = s.a; *p = 0; }\n");
        ASSERT_EQUALS("", errout_str());

        check("struct Foo {\n" // #9910
              "    int* p{};\n"
              "    int* get() { return p; }\n"
              "    const int* get() const { return p; }\n"
              "};\n"
              "struct Bar {\n"
              "    int j{};\n"
              "    void f(Foo& foo) const { int* q = foo.get(); *q = j; }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #10679
              "    void g(long L, const C*& PC) const;\n"
              "    void g(long L, C*& PC);\n"
              "};\n"
              "void f(S& s) {\n"
              "    C* PC{};\n"
              "    s.g(0, PC);\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #10785
        check("template <class T, class C>\n"
              "struct d {\n"
              "    T& g(C& c, T C::*f) { return c.*f; }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::map<int, int>& m) {\n"
              "    std::cout << m[0] << std::endl;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<std::map<int, int>>& v) {\n" // #11607
              "    for (auto& m : v)\n"
              "        std::cout << m[0];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n" // #11473
              "void f(std::vector<std::vector<S>>&m, int*& p) {\n"
              "    auto& a = m[0];\n"
              "    for (auto& s : a) {\n"
              "        p = &s.i;\n"
              "        return;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int& g(int* p, int& r) {\n" // #11625
              "    if (p)\n"
              "        return *p;\n"
              "    return r;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("template <typename T> void f(std::vector<T*>& d, const std::vector<T*>& s) {\n" // #11632
              "    for (const auto& e : s) {\n"
              "        T* newE = new T(*e);\n"
              "        d.push_back(newE);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11682
        check("struct b {\n"
              "    void mutate();\n"
              "};\n"
              "struct c {\n"
              "    const b& get() const;\n"
              "    b get();\n"
              "};\n"
              "struct d {\n"
              "    void f(c& e) const {\n"
              "        e.get().mutate();\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct B { virtual void f() const {} };\n" // #11528
              "struct D : B {};\n"
              "void g(B* b) {\n"
              "    D* d = dynamic_cast<D*>(b);\n"
              "    if (d)\n"
              "        d->f();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:8]: (style) Variable 'd' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("void g(const int*);\n"
              "void f(const std::vector<int*>&v) {\n"
              "    for (int* i : v)\n"
              "        g(i);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:15]: (style) Variable 'i' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("struct A {\n" // #11225
              "    A();\n"
              "    virtual ~A();\n"
              "};\n"
              "struct B : A {};\n"
              "void f(A* a) {\n"
              "    const B* b = dynamic_cast<const B*>(a);\n"
              "}\n"
              "void g(A* a) {\n"
              "    const B* b = (const B*)a;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:10:18]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n"
                      "[test.cpp:6:11]: (style) Parameter 'a' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:9:11]: (style) Parameter 'a' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void g(int*);\n"
              "void f(std::vector<int>& v) {\n"
              "    g(v.data());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(const int*);\n"
              "void f(std::vector<int>& v) {\n"
              "    g(v.data());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:26]: (style) Parameter 'v' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("struct a {\n"
              "    template <class T>\n"
              "    void mutate();\n"
              "};\n"
              "struct b {};\n"
              "template <class T>\n"
              "void f(a& x) {\n"
              "    x.mutate<T>();\n"
              "}\n"
              "template <class T>\n"
              "void f(const b&)\n"
              "{}\n"
              "void g(a& c) { f<int>(c); }\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    template <typename T>\n"
              "    T* g() {\n"
              "        return reinterpret_cast<T*>(m);\n"
              "    }\n"
              "    template <typename T>\n"
              "    const T* g() const {\n"
              "        return reinterpret_cast<const T*>(m);\n"
              "    }\n"
              "    char* m;\n"
              "};\n"
              "void f(S& s) {\n"
              "    const int* p = s.g<int>();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int x; };\n" // #11818
              "std::istream& f(std::istream& is, S& s) {\n"
              "    return is >> s.x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool f(std::string& s1, std::string& s2) {\n" // #12203
              "    return &s1 == &s2;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:21]: (style) Parameter 's1' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:1:38]: (style) Parameter 's2' can be declared as reference to const [constParameterReference]\n",
                      errout_str());

        check("void f(int& r) {\n" // #12214
              "    (void)(true);\n"
              "    if (r) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:13]: (style) Parameter 'r' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("struct S { void f(int&); };\n" // #12216
              "void g(S& s, int& r, void (S::* p2m)(int&)) {\n"
              "    (s.*p2m)(r);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    void f(int& r) { p = &r; }\n"
              "    int* p;\n"
              "};\n"
              "void g(std::vector<int>& v1, std::vector<int*>& v2) {\n"
              "    std::transform(v1.begin(), v1.end(), v2.begin(), [](auto& x) { return &x; });\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("class T;\n" // #11869
              "class E {\n"
              "public:\n"
              "    class F {\n"
              "    public:\n"
              "        explicit F(const T* t);\n"
              "    };\n"
              "};\n"
              "void f(T& t) {\n"
              "    std::list<E::F> c(1, E::F(&t));\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9:11]: (style) Parameter 't' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("struct T;\n"
              "struct U {\n"
              "    struct V { explicit V(const T* p); };\n"
              "};\n"
              "void g(U::V v);\n"
              "void f(T& t) {\n"
              "    g(U::V(&t));\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:11]: (style) Parameter 't' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("void f1(std::vector<int>& v) {\n" // #11207
              "    auto it = v.cbegin();\n"
              "    while (it != v.cend()) {\n"
              "        if (*it > 12) {}\n"
              "        ++it;\n"
              "    }\n"
              "}\n"
              "void f2(std::vector<int>& v) {\n"
              "    auto it = v.begin();\n"
              "    while (it != v.end()) {\n"
              "        if (*it > 12) {}\n"
              "        ++it;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:27]: (style) Parameter 'v' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:8:27]: (style) Parameter 'v' can be declared as reference to const [constParameterReference]\n",
                      errout_str());

        check("void cb(const std::string&);\n" // #12349, #12350, #12351
              "void f(std::string& s) {\n"
              "    const std::string& str(s);\n"
              "    cb(str);\n"
              "}\n"
              "void g(std::string& s) {\n"
              "    const std::string& str{ s };\n"
              "    cb(str);\n"
              "}\n"
              "void h(std::string* s) {\n"
              "    const std::string& str(*s);\n"
              "    cb(str);\n"
              "}\n"
              "void k(std::string* s) {\n"
              "    const std::string& str = *s;\n"
              "    cb(str);\n"
              "}\n"
              "void m(std::string& s) {\n"
              "    const std::string str(s);\n"
              "    cb(str);\n"
              "}\n"
              "void n(std::string* s) {\n"
              "    const std::string& str(*s);\n"
              "    cb(str);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:21]: (style) Parameter 's' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:6:21]: (style) Parameter 's' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:18:21]: (style) Parameter 's' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:10:21]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:14:21]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:22:21]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n"
              "    S(std::string& r);\n"
              "};\n"
              "void f(std::string& str) {\n"
              "    const S& s(str);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n" // #10052
              "    int& operator()(int);\n"
              "};\n"
              "void f(std::vector<C>& c) {\n"
              "    c[0](5) = 12;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int& t) {\n" // #11713
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:12]: (style) Parameter 't' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("void f(std::list<std::string>& v) {\n" // #12202
              "    v.remove_if([](std::string& s) {\n"
              "        return true;\n"
              "    });\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:33]: (style) Parameter 's' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("struct S {\n" // #12762
              "    std::vector<int> m;\n"
              "    void f();\n"
              "};\n"
              "void S::f() {\n"
              "    std::vector<int>& r = m;\n"
              "    g(r[0] * 2);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:23]: (style) Variable 'r' can be declared as reference to const [constVariableReference]\n", errout_str());

        check("std::iostream& get();\n" // #12940
              "std::iostream& Fun() {\n"
              "    auto lam = []() -> std::iostream& {\n"
              "        std::iostream& ios = get();\n"
              "        return ios;\n"
              "    };\n"
              "    return lam();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int x[3]; };\n" // #13226
              "void g(int a, int* b);\n"
              "void f(int a, S& s) {\n"
              "    return g(a, s.x);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { std::vector<int> v; };\n" // #13317
              "struct T { S s; };\n"
              "int f(S& s) {\n"
              "    for (std::vector<int>::const_iterator it = s.v.cbegin(); it != s.v.cend(); ++it) {}\n"
              "    return *s.v.cbegin();\n"
              "}\n"
              "int f(T& t) {\n"
              "    return *t.s.v.cbegin();\n"
              "}\n"
              "int f(std::vector<int>& v) {\n"
              "    return *v.cbegin();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Parameter 's' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:7:10]: (style) Parameter 't' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:10:25]: (style) Parameter 'v' can be declared as reference to const [constParameterReference]\n",
                      errout_str());

        check("void push(V& v) { v.push_back({ .x = 1 }); }"); // #14010
        ASSERT_EQUALS("", errout_str());

        check("size_t* f(std::array<uint8_t, 128>& a) { return reinterpret_cast<size_t*>(a.data()); }\n"); // #14074
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n" // #14231
              "void* f(S& s, int& v) {\n"
              "    v = s.i;\n"
              "    return (void*)&s;\n"
              "}\n"); // don't crash
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n" // #14251
              "struct T { std::optional<S> s; };\n"
              "void f(T& t) {\n"
              "    t.s->i = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct B {};\n" // #13877
              "struct D : B { int i; };\n"
              "void f(B& b) {\n"
              "    static_cast<D&>(b).i = 0;\n"
              "}\n"
              "void g(B& b) {\n"
              "    std::cin >> static_cast<D&>(b).i;\n"
              "}\n"
              "int h(B& b) {\n"
              "    return static_cast<const D&>(b).i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9:10]: (style) Parameter 'b' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("void f(int *p) {\n" // #14409
              "    int*& pp{ p };\n"
              "    if (*pp) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void constParameterCallback() {
        check("int callback(std::vector<int>& x) { return x[0]; }\n"
              "void f() { dostuff(callback); }");
        ASSERT_EQUALS("[test.cpp:2:20] -> [test.cpp:1:32]: (style) Parameter 'x' can be declared as reference to const. However it seems that 'callback' is a callback function, if 'x' is declared with const you might also need to cast function pointer(s). [constParameterCallback]\n", errout_str());

        // #9906
        check("class EventEngine : public IEventEngine {\n"
              "public:\n"
              "    EventEngine();\n"
              "\n"
              "private:\n"
              "    void signalEvent(ev::sig& signal, int revents);\n"
              "};\n"
              "\n"
              "EventEngine::EventEngine() {\n"
              "    mSigWatcher.set<EventEngine, &EventEngine::signalEvent>(this);\n"
              "}\n"
              "\n"
              "void EventEngine::signalEvent(ev::sig& signal, int revents) {\n"
              "    switch (signal.signum) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:48] -> [test.cpp:13:40]: (style) Parameter 'signal' can be declared as reference to const. However it seems that 'signalEvent' is a callback function, if 'signal' is declared with const you might also need to cast function pointer(s). [constParameterCallback]\n", errout_str());

        check("void f(int* p) {}\n" // 12843
              "void g(std::map<void(*)(int*), int>&m) {\n"
              "    m[&f] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:1:13]: (style) Parameter 'p' can be declared as pointer to const. "
                      "However it seems that 'f' is a callback function, if 'p' is declared with const you might also need to cast function pointer(s). [constParameterCallback]\n",
                      errout_str());

        check("struct S { explicit S(std::function<void(std::string)>); };\n" // #13338
              "void cb(std::string s) {\n"
              "    (void)s.empty();\n"
              "}\n"
              "void f() {\n"
              "    S s2{ cb };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:11] -> [test.cpp:2:21]: (performance) Function parameter 's' should be passed by const reference. However it seems that 'cb' is a callback function. [passedByValueCallback]\n", errout_str());
    }

    void constPointer() {
        check("void foo(int *p) { return *p; }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { x = *p; }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { int &ref = *p; ref = 12; }");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int *p) { x = *p + 10; }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { return p[10]; }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { int &ref = p[0]; ref = 12; }");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int *p) { x[*p] = 12; }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { if (p) {} }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { if (p || x) {} }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { if (p == 0) {} }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { if (!p) {} }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { if (*p > 123) {} }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { return *p + 1; }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(int *p) { return *p > 1; }");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(const int* c) { if (c == 0) {}; }");
        ASSERT_EQUALS("", errout_str());

        check("struct a { void b(); };\n"
              "struct c {\n"
              "    a* d;\n"
              "    a& g() { return *d; }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct a { void b(); };\n"
              "struct c { a* d; };\n"
              "void e(c);\n");
        ASSERT_EQUALS("", errout_str());

        check("struct V {\n"
              "    V& get(typename std::vector<V>::size_type i, std::vector<V>* arr) {\n"
              "        return arr->at(i);\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {};\n"
              "struct B : A {};\n"
              "B* f(A* x) {\n"
              "    return static_cast<B*>(x);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(std::vector<int>* x) {\n"
              "    int& i = (*x)[0];\n"
              "    i++;\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int a; };\n"
              "A f(std::vector<A>* x) {\n"
              "    x->front().a = 1;\n"
              "    return x->front();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int>* v) {\n"
              "    for(auto&& x:*v)\n"
              "        x = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int* x;\n"
              "    A(int* y) : x(y)\n"
              "    {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b, int* x, int* y) {\n"
              "  int* z = x;\n"
              "  int* w = b ? y : z;\n"
              "  *w = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b, int* x, int* y) {\n"
              "  int& z = *x;\n"
              "  int& w = b ? *y : z;\n"
              "  w = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class Base { virtual void dostuff(int *p) = 0; };\n" // #10397
              "class Derived: public Base { int x; void dostuff(int *p) override { x = *p; } };");
        ASSERT_EQUALS("", errout_str());

        check("struct Data { char buf[128]; };\n" // #10483
              "void encrypt(Data& data) {\n"
              "    const char a[] = \"asfasd\";\n"
              "    memcpy(data.buf, &a, sizeof(a));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #10547
        check("void foo(std::istream &istr) {\n"
              "  unsigned char x[2];\n"
              "  istr >> x[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #10744
        check("S& f() {\n"
              "    static S* p = new S();\n"
              "    return *p;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10471
        check("void f(std::array<int, 1> const& i) {\n"
              "    if (i[0] == 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10466
        check("typedef void* HWND;\n"
              "void f(const std::vector<HWND>&v) {\n"
              "    for (const auto* h : v)\n"
              "        if (h) {}\n"
              "    for (const auto& h : v)\n"
              "        if (h) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:5:22]: (style) Variable 'h' can be declared as pointer to const [constVariableReference]\n",
            errout_str());

        check("void f(const std::vector<int*>& v) {\n"
              "    for (const auto& p : v)\n"
              "        if (p == nullptr) {}\n"
              "    for (const auto* p : v)\n"
              "        if (p == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:22]: (style) Variable 'p' can be declared as pointer to const [constVariableReference]\n",
            errout_str());

        check("void f(std::vector<int*>& v) {\n"
              "    for (const auto& p : v)\n"
              "        if (p == nullptr) {}\n"
              "    for (const auto* p : v)\n"
              "        if (p == nullptr) {}\n"
              "    for (const int* const& p : v)\n"
              "        if (p == nullptr) {}\n"
              "    for (const int* p : v)\n"
              "        if (p == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:27]: (style) Parameter 'v' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:2:22]: (style) Variable 'p' can be declared as pointer to const [constVariableReference]\n",
                      errout_str());

        check("void f(std::vector<const int*>& v) {\n"
              "    for (const auto& p : v)\n"
              "        if (p == nullptr) {}\n"
              "    for (const auto* p : v)\n"
              "        if (p == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:33]: (style) Parameter 'v' can be declared as reference to const [constParameterReference]\n", errout_str());

        check("void f(const std::vector<const int*>& v) {\n"
              "    for (const auto& p : v)\n"
              "        if (p == nullptr) {}\n"
              "    for (const auto* p : v)\n"
              "        if (p == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const int* const p) {\n"
              "    if (p == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(int*);\n"
              "void f(int* const* pp) {\n"
              "    int* p = pp[0];\n"
              "    g(p);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("template <typename T>\n"
              "struct S {\n"
              "    static bool f(const T& t) { return t != nullptr; }\n"
              "};\n"
              "S<int*> s;\n");
        ASSERT_EQUALS("", errout_str());

        check("typedef void* HWND;\n" // #11084
              "void f(const HWND h) {\n"
              "    if (h == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("using HWND = void*;\n"
              "void f(const HWND h) {\n"
              "    if (h == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("typedef int A;\n"
              "void f(A* x) {\n"
              "    if (x == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'x' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("using A = int;\n"
              "void f(A* x) {\n"
              "    if (x == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'x' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("struct S { void v(); };\n" // #11095
              "void f(S* s) {\n"
              "    (s - 1)->v();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int*>& v) {\n" // #11085
              "    for (int* p : v) {\n"
              "        if (p) {}\n"
              "    }\n"
              "    for (auto* p : v) {\n"
              "        if (p) {}\n"
              "    }\n"
              "    v.clear();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Variable 'p' can be declared as pointer to const [constVariablePointer]\n"
                      "[test.cpp:5:16]: (style) Variable 'p' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("void f() {\n"
              "    char a[1][1];\n"
              "    char* b[1];\n"
              "    b[0] = a[0];\n"
              "    **b = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("ptrdiff_t f(int *p0, int *p1) {\n" // #11148
              "    return p0 - p1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:18]: (style) Parameter 'p0' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:1:27]: (style) Parameter 'p1' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void f() {\n"
              "    std::array<int, 1> a{}, b{};\n"
              "    const std::array<int, 1>& r = a;\n"
              "    if (r == b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {};\n" // #11599
              "void g(S);\n"
              "void h(const S&);\n"
              "void h(int, int, const S&);\n"
              "void i(S&);\n"
              "void j(const S*);\n"
              "void j(int, int, const S*);\n"
              "void f1(S* s) {\n"
              "    g(*s);\n"
              "}\n"
              "void f2(S* s) {\n"
              "    h(*s);\n"
              "}\n"
              "void f3(S* s) {\n"
              "    h(1, 2, *s);\n"
              "}\n"
              "void f4(S* s) {\n"
              "    i(*s);\n"
              "}\n"
              "void f5(S& s) {\n"
              "    j(&s);\n"
              "}\n"
              "void f6(S& s) {\n"
              "    j(1, 2, &s);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:20:12]: (style) Parameter 's' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:23:12]: (style) Parameter 's' can be declared as reference to const [constParameterReference]\n"
                      "[test.cpp:8:12]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:11:12]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:14:12]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void g(int, const int*);\n"
              "void h(const int*);\n"
              "void f(int* p) {\n"
              "    g(1, p);\n"
              "    h(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void f(int, const int*);\n"
              "void f(int i, int* p) {\n"
              "    f(i, const_cast<const int*>(p));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int a; };\n"
              "void f(std::vector<S>& v, int b) {\n"
              "    size_t n = v.size();\n"
              "    for (size_t i = 0; i < n; i++) {\n"
              "        S& s = v[i];\n"
              "        if (!(b & s.a))\n"
              "            continue;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:12]: (style) Variable 's' can be declared as reference to const [constVariableReference]\n", errout_str()); // don't crash

        check("void f(int& i) {\n"
              "    new (&i) int();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str()); // don't crash

        check("void f(int& i) {\n"
              "    int& r = i;\n"
              "    if (!&r) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Variable 'r' can be declared as reference to const [constVariableReference]\n", errout_str()); // don't crash

        check("class C;\n" // #11646
              "void g(const C* const p);\n"
              "void f(C* c) {\n"
              "    g(c);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Parameter 'c' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("typedef void (*cb_t)(int*);\n" // #11674
              "void cb(int* p) {\n"
              "    if (*p) {}\n"
              "}\n"
              "void g(cb_t);\n"
              "void f() {\n"
              "    g(cb);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:7] -> [test.cpp:2:14]: (style) Parameter 'p' can be declared as pointer to const. "
                      "However it seems that 'cb' is a callback function, if 'p' is declared with const you might also need to cast function pointer(s). [constParameterCallback]\n",
                      errout_str());

        check("typedef void (*cb_t)(int*);\n"
              "void cb(int* p) {\n"
              "    if (*p) {}\n"
              "}\n"
              "void g(cb_t);\n"
              "void f() {\n"
              "    g(::cb);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:9] -> [test.cpp:2:14]: (style) Parameter 'p' can be declared as pointer to const. "
                      "However it seems that 'cb' is a callback function, if 'p' is declared with const you might also need to cast function pointer(s). [constParameterCallback]\n",
                      errout_str());

        check("void f1(std::vector<int>* p) {\n" // #11681
              "    if (p->empty()) {}\n" // warn
              "}\n"
              "void f2(std::vector<int>* p) {\n"
              "    p->resize(0);\n"
              "}\n"
              "struct S {\n"
              "    void h1() const;\n"
              "    void h2();\n"
              "    int i;\n"
              "};\n"
              "void k(int&);\n"
              "void g1(S* s) {\n"
              "    s->h1();\n" // warn
              "}\n"
              "void g1(S* s) {\n"
              "    s->h2();\n"
              "}\n"
              "void g1(S* s) {\n"
              "    if (s->i) {}\n" // warn
              "}\n"
              "void g2(S* s) {\n"
              "    s->i = 0;\n"
              "}\n"
              "void g3(S* s) {\n"
              "    k(s->i);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:27]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:13:12]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:19:12]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n" // #11573
              "    const char* g() const {\n"
              "        return m;\n"
              "    }\n"
              "    const char* m;\n"
              "};\n"
              "struct T { std::vector<S*> v; };\n"
              "void f(T* t, const char* n) {\n"
              "    for (const auto* p : t->v)\n"
              "        if (strcmp(p->g(), n) == 0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:11]: (style) Parameter 't' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void f(int*& p, int* q) {\n"
              "    p = q;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int a[1]; };\n"
              "void f(S* s) {\n"
              "    if (s->a[0]) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("size_t f(char* p) {\n" // #11842
              "    return strlen(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:16]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void f(int* p) {\n" // #11862
              "    long long j = *(p++);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:13]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void f(void *p, size_t nmemb, size_t size, int (*cmp)(const void *, const void *)) {\n"
              "    qsort(p, nmemb, size, cmp);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(bool *r, std::size_t *b) {\n" // #12129
              "    if (*r && *b >= 5) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:14]: (style) Parameter 'r' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:1:30]: (style) Parameter 'b' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void f(int i) {\n" // #12185
              "    void* p = &i;\n"
              "    std::cout << p << '\\n';\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Variable 'p' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("struct S { const T* t; };\n" // #12206
              "void f(S* s) {\n"
              "    if (s->t.i) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void f(char *a1, char *a2) {\n" // #12252
              "    char* b = new char[strlen(a1) + strlen(a2) + 2];\n"
              "    sprintf(b, \"%s_%s\", a1, a2);\n"
              "    delete[] b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:14]: (style) Parameter 'a1' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:1:24]: (style) Parameter 'a2' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("int f(int* p) {\n" // #11713
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:12]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void f(int *src, int* dst) {\n" // #12518
              "    *dst++ = (int)*src++;\n"
              "    *dst++ = static_cast<int>(*src++);\n"
              "    *dst = (int)*src;\n"
              "}\n"
              "void g(int* dst) {\n"
              "    (int&)*dst = 5;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:13]: (style) Parameter 'src' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {};\n"
              "void f(T* t) {\n"
              "    S* s = (S*)t->p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) C-style pointer casting [cstyleCast]\n"
                      "[test.cpp:3:8]: (style) Variable 's' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str()); // don't crash

        check("struct S { int i; };\n" // #12205
              "void f(S* s) {\n"
              "    (void)s->i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void f(int* a, int* b, int i) {\n" // #13072
              "    a[b[i]] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:21]: (style) Parameter 'b' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("int f(int* a, int* b, int i) {\n" // #13085
              "    a[*(b + i)] = 0;\n"
              "    return *(b + i);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:20]: (style) Parameter 'b' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S { int a; };\n" // #13286
              "void f(struct S* s) {\n"
              "    if ((--s)->a >= 0) {}\n"
              "}\n"
              "void g(struct S* s) {\n"
              "    --s;\n"
              "    if (s->a >= 0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:18]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:5:18]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct T;\n"
              "void use(const T*);\n"
              "void f(T* tok0) {\n"
              "    T *tok1 = tok0;\n"
              "    const T *tok2 = tok1;\n"
              "    use(tok2);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:8]: (style) Variable 'tok1' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("struct S { S* next; };\n" // #14119
              "void f(S* s) {\n"
              "    for (S* p = s->next; p != nullptr; p = p->next) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Variable 'p' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("void f(int* p) {\n"
              "    for (int* q = p; q;)\n"
              "        break;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Variable 'q' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("void g(const int*);\n" // #14148
              "void f() {\n"
              "    int a[] = {1, 2, 3};\n"
              "    for (int* p = a; *p != 3; p++) {\n"
              "        g(p);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:15]: (style) Variable 'p' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("uintptr_t f(int* p) {\n"
              "    return (uintptr_t)p;\n"
              "}\n"
              "uintptr_t g(int* p) {\n"
              "    return static_cast<uintptr_t>(p);\n"
              "}\n"
              "U h(int* p) {\n"
              "    return (U)p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:18]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:4:18]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("using fp_t = int (*)(int*);\n" // #14510
              "fp_t g_fp;\n"
              "struct S { fp_t m_fp; };\n"
              "void g(fp_t);\n"
              "S f(S* s) {\n"
              "    g_fp = [](int* p) { return *p; };\n"
              "    s->m_fp = [](int* p) { return *p; };\n"
              "    g([](int* p) { return *p; });\n"
              "    auto x = [](int* p) { return *p; };\n"
              "    g(x);\n"
              "    return { [](int* p) { return *p; } };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int i = 0;\n"
              "    auto x = [&]() { int* p = &i; if (*p) {} };\n"
              "    x();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:27]: (style) Variable 'p' can be declared as pointer to const [constVariablePointer]\n", errout_str());

        check("int f() {\n"
              "    int i = 0;\n"
              "    return [](int* p) { return *p; }(&i);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:20]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("struct S {\n" // #14571
              "    char* c;\n"
              "};\n"
              "struct T {\n"
              "    S s;\n"
              "};\n"
              "void f(std::string* p, T& t) {\n"
              "    S& r = t.s;\n"
              "    strcpy(r.c, p->c_str());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:21]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("struct S {\n" // #14559
              "    int gc() const;\n"
              "    int gnc();\n"
              "};\n"
              "int f1(S* s) {\n"
              "    return h(s ? s->gc() : 1);\n"
              "}\n"
              "int f2(S* s) {\n"
              "    return h(s ? s->gnc() : 1);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:11]: (style) Parameter 's' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("using IntPtr = int *;\n"
              "int* foo(IntPtr bar) {\n"
              "  return bar = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void constArray() {
        check("void f(std::array<int, 2>& a) {\n"
              "    if (a[0]) {}\n"
              "}\n"
              "void g(std::array<int, 2>& a) {\n"
              "    a.fill(0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:28]: (style) Parameter 'a' can be declared as const array [constParameterReference]\n", errout_str());

        check("int f() {\n"
              "    static int i[1] = {};\n"
              "    return i[0];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Variable 'i' can be declared as const array [constVariable]\n", errout_str());

        check("int f() {\n"
              "    static int i[] = { 0 };\n"
              "    int j = i[0] + 1;\n"
              "    return j;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Variable 'i' can be declared as const array [constVariable]\n", errout_str());

        check("void f(int i) {\n"
              "    const char *tmp;\n"
              "    char* a[] = { \"a\", \"aa\" };\n"
              "    static char* b[] = { \"b\", \"bb\" };\n"
              "    tmp = a[i];\n"
              "    printf(\"%s\", tmp);\n"
              "    tmp = b[i];\n"
              "    printf(\"%s\", tmp);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Variable 'a' can be declared as const array [constVariable]\n"
                      "[test.cpp:4:18]: (style) Variable 'b' can be declared as const array [constVariable]\n",
                      errout_str());

        check("int f(int i, int j) {\n" // #13069
              "    int a[3][4] = {\n"
              "        { 2,  2, -1, -1 },\n"
              "        { 2, -1,  2, -1 },\n"
              "        { 2, -1, -1,  2 },\n"
              "    };\n"
              "    return a[j][i];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Variable 'a' can be declared as const array [constVariable]\n",
                      errout_str());

        check("void f(int n, int v[42]) {\n" // #12796
              "    int j = 0;\n"
              "    for (int i = 0; i < n; ++i) {\n"
              "        j += 1;\n"
              "        if (j == 1) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:19]: (style) Parameter 'v' can be declared as const array [constParameter]\n",
                      errout_str());
    }

    void switchRedundantAssignmentTest() {
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:11] -> [test.cpp:9:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y = 2;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:11] -> [test.cpp:11:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "    case 3:\n"
              "        if (x)\n"
              "        {\n"
              "            y = 3;\n"
              "        }\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y = 2;\n"
              "        if (z)\n"
              "            printf(\"%d\", y);\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    int x = a;\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        x = 2;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "        break;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    while(xyz()) {\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y = 2;\n"
              "            continue;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "        bar(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    while(xyz()) {\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y = 2;\n"
              "            throw e;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "        bar(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "        printf(\"%d\", y);\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "        bar();\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:11] -> [test.cpp:10:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());

        check("void bar() {}\n" // bar isn't noreturn
              "void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "        bar();\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:11] -> [test.cpp:11:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strcpy(str, \"a'\");\n"
              "    case 3:\n"
              "      strcpy(str, \"b'\");\n"
              "    }\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        TODO_ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:8]: (style) Buffer 'str' is being written before its old content has been used. 'break;' missing?\n",
                           "",
                           errout_str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strncpy(str, \"a'\");\n"
              "    case 3:\n"
              "      strncpy(str, \"b'\");\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:8]: (style) Buffer 'str' is being written before its old content has been used. 'break;' missing?\n",
                           "",
                           errout_str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    int z = 0;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strcpy(str, \"a'\");\n"
              "      z++;\n"
              "    case 3:\n"
              "      strcpy(str, \"b'\");\n"
              "      z++;\n"
              "    }\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        TODO_ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:10]: (style) Buffer 'str' is being written before its old content has been used. 'break;' missing?\n",
                           "",
                           errout_str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strcpy(str, \"a'\");\n"
              "      break;\n"
              "    case 3:\n"
              "      strcpy(str, \"b'\");\n"
              "      break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a) {\n"
              "    char str[10];\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      strcpy(str, \"a'\");\n"
              "      printf(str);\n"
              "    case 3:\n"
              "      strcpy(str, \"b'\");\n"
              "    }\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // Ticket #5158 "segmentation fault (valid code)"
        check("typedef struct ct_data_s {\n"
              "    union {\n"
              "        char freq;\n"
              "    } fc;\n"
              "} ct_data;\n"
              "typedef struct internal_state {\n"
              "    struct ct_data_s dyn_ltree[10];\n"
              "} deflate_state;\n"
              "void f(deflate_state *s) {\n"
              "    s->dyn_ltree[0].fc.freq++;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // Ticket #6132 "crash: daca: kvirc CheckOther::checkRedundantAssignment()"
        check("void HttpFileTransfer :: transferTerminated ( bool bSuccess ) {\n"
              "if ( m_szCompletionCallback . isNull ( ) ) {\n"
              "KVS_TRIGGER_EVENT ( KviEvent_OnHTTPGetTerminated , out ? out : ( g_pApp . activeConsole ( ) ) , & vParams )\n"
              "} else {\n"
              "KviKvsScript :: run ( m_szCompletionCallback , out ? out : ( g_pApp . activeConsole ( ) ) , & vParams ) ;\n"
              "}\n"
              "}\n", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int x;\n"
              "  switch (state) {\n"
              "  case 1: x = 3; goto a;\n"
              "  case 1: x = 6; goto a;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void switchRedundantOperationTest() {
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        ++y;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:9] -> [test.cpp:9:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        ++y;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:9] -> [test.cpp:11:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        (void)y;\n"
              "    case 3:\n"
              "        ++y;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        ++y;\n"
              "    case 3:\n"
              "        ++y;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        --y;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:9] -> [test.cpp:9:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        --y;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:9] -> [test.cpp:11:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        (void)y;\n"
              "    case 3:\n"
              "        --y;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        --y;\n"
              "    case 3:\n"
              "        --y;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:10] -> [test.cpp:9:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y++;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:10] -> [test.cpp:11:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "    case 3:\n"
              "        y++;\n"
              "    }\n"
              "    bar(y);\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "    case 3:\n"
              "        y++;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y--;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:10] -> [test.cpp:9:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y--;\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:10] -> [test.cpp:11:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y = 2;\n"
              "    case 3:\n"
              "        y--;\n"
              "    }\n"
              "    bar(y);\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y--;\n"
              "    case 3:\n"
              "        y--;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "    case 3:\n"
              "        if (x)\n"
              "        {\n"
              "            y = 3;\n"
              "        }\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "      {\n"
              "        y++;\n"
              "        if (y)\n"
              "            printf(\"%d\", y);\n"
              "      }\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int x = a;\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        x++;\n"
              "    case 3:\n"
              "        y++;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "        break;\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    while(xyz()) {\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y++;\n"
              "            continue;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "        bar(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    while(xyz()) {\n"
              "        switch (x)\n"
              "        {\n"
              "        case 2:\n"
              "            y++;\n"
              "            throw e;\n"
              "        case 3:\n"
              "            y = 3;\n"
              "        }\n"
              "        bar(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "        printf(\"%d\", y);\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (x)\n"
              "    {\n"
              "    case 2:\n"
              "        y++;\n"
              "        bar();\n"
              "    case 3:\n"
              "        y = 3;\n"
              "    }\n"
              "    bar(y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:10] -> [test.cpp:10:11]: (style) Variable 'y' is reassigned a value before the old one has been used. 'break;' missing? [redundantAssignInSwitch]\n", errout_str());

        check("bool f() {\n"
              "    bool ret = false;\n"
              "    switch (switchCond) {\n"
              "    case 1:\n"
              "        ret = true;\n"
              "        break;\n"
              "    case 31:\n"
              "        ret = true;\n"
              "        break;\n"
              "    case 54:\n"
              "        ret = true;\n"
              "        break;\n"
              "    };\n"
              "    ret = true;\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13] -> [test.cpp:14:9]: (style) Variable 'ret' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:8:13] -> [test.cpp:14:9]: (style) Variable 'ret' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:11:13] -> [test.cpp:14:9]: (style) Variable 'ret' is reassigned a value before the old one has been used. [redundantAssignment]\n",
                      errout_str());
    }

    void switchRedundantBitwiseOperationTest() {
        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:9]: (style) Redundant bitwise operation on 'y' in 'switch' statement. 'break;' missing? [redundantBitwiseOperationInSwitch]\n", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y = y | 3;\n"
              "    case 3:\n"
              "        y = y | 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:9]: (style) Redundant bitwise operation on 'y' in 'switch' statement. 'break;' missing? [redundantBitwiseOperationInSwitch]\n", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "    default:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:9]: (style) Redundant bitwise operation on 'y' in 'switch' statement. 'break;' missing? [redundantBitwiseOperationInSwitch]\n", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "    default:\n"
              "        if (z)\n"
              "            y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= z;\n"
              "        z++;\n"
              "    default:\n"
              "        y |= z;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "        bar(y);\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "        y = 4;\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:11] -> [test.cpp:8:11]: (style) Variable 'y' is reassigned a value before the old one has been used. [redundantAssignment]\n", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y &= 3;\n"
              "    case 3:\n"
              "        y &= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:9]: (style) Redundant bitwise operation on 'y' in 'switch' statement. 'break;' missing? [redundantBitwiseOperationInSwitch]\n", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y ^= 3;\n"
              "    case 3:\n"
              "        y ^= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 2;\n"
              "    case 3:\n"
              "        y |= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y &= 2;\n"
              "    case 3:\n"
              "        y &= 3;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "    int y = 1;\n"
              "    switch (a)\n"
              "    {\n"
              "    case 2:\n"
              "        y |= 2;\n"
              "    case 3:\n"
              "        y &= 2;\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void unreachableCode() {
        check("void foo(int a) {\n"
              "    while(1) {\n"
              "        if (a++ >= 100) {\n"
              "            break;\n"
              "            continue;\n"
              "        }\n"
              "    }\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:5:13]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("int foo(int a) {\n"
              "    return 0;\n"
              "    return(a-1);\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("int foo(int a) {\n"
              "  A:"
              "    return(0);\n"
              "    goto A;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"exit\">\n"
                                   "    <noreturn>true</noreturn>\n"
                                   "    <arg nr=\"1\"/>\n"
                                   "  </function>\n"
                                   "</def>";
        /*const*/ Settings settings = settingsBuilder().libraryxml(xmldata).severity(Severity::style).build();

        check("void foo() {\n"
              "    exit(0);\n"
              "    break;\n"
              "}", dinit(CheckOptions, $.inconclusive = false, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("class NeonSession {\n"
              "    void exit();\n"
              "};\n"
              "void NeonSession::exit()\n"
              "{\n"
              "    SAL_INFO(\"ucb.ucp.webdav\", \"neon commands cannot be aborted\");\n"
              "}", dinit(CheckOptions, $.inconclusive = false, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        check("void NeonSession::exit()\n"
              "{\n"
              "    SAL_INFO(\"ucb.ucp.webdav\", \"neon commands cannot be aborted\");\n"
              "}", dinit(CheckOptions, $.inconclusive = false, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        check("void foo() { xResAccess->exit(); }", dinit(CheckOptions, $.inconclusive = false, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "        switch(a) {\n"
              "          case 0:\n"
              "            printf(\"case 0\");\n"
              "            break;\n"
              "            break;\n"
              "          case 1:\n"
              "            c++;\n"
              "            break;\n"
              "         }\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:7:13]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "        switch(a) {\n"
              "          case 0:\n"
              "            printf(\"case 0\");\n"
              "            break;\n"
              "          case 1:\n"
              "            c++;\n"
              "            break;\n"
              "         }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "        while(true) {\n"
              "          if (a++ >= 100) {\n"
              "            break;\n"
              "            break;\n"
              "          }\n"
              "       }\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:6:13]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "        while(true) {\n"
              "          if (a++ >= 100) {\n"
              "            continue;\n"
              "            continue;\n"
              "          }\n"
              "          a+=2;\n"
              "       }\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:6:13]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("void foo(int a)\n"
              "{\n"
              "        while(true) {\n"
              "          if (a++ >= 100) {\n"
              "            continue;\n"
              "          }\n"
              "          a+=2;\n"
              "       }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int foo() {\n"
              "    throw 0;\n"
              "    return 1;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("void foo() {\n"
              "    throw 0;\n"
              "    return;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("int foo() {\n"
              "    throw = 0;\n"
              "    return 1;\n"
              "}", dinit(CheckOptions, $.cpp = false, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("int foo() {\n"
              "    return 0;\n"
              "    return 1;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("int foo() {\n"
              "    return 0;\n"
              "    foo();\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Statements following 'return' will never be executed. [unreachableCode]\n", errout_str());

        check("int foo(int unused) {\n"
              "    return 0;\n"
              "    (void)unused;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("int foo(int unused1, int unused2) {\n"
              "    return 0;\n"
              "    (void)unused1;\n"
              "    (void)unused2;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("int foo(int unused1, int unused2) {\n"
              "    return 0;\n"
              "    (void)unused1;\n"
              "    (void)unused2;\n"
              "    foo();\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:5:5]: (style) Statements following 'return' will never be executed. [unreachableCode]\n", errout_str());

        check("int foo() {\n"
              "    if(bar)\n"
              "        return 0;\n"
              "    return 124;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int foo() {\n"
              "    while(bar) {\n"
              "        return 0;\n"
              "        return 0;\n"
              "        return 0;\n"
              "        return 0;\n"
              "    }\n"
              "    return 124;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:4:9]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        check("void foo() {\n"
              "    while(bar) {\n"
              "        return;\n"
              "        break;\n"
              "    }\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:4:9]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        // #5707
        check("extern int i,j;\n"
              "int foo() {\n"
              "    switch(i) {\n"
              "        default: j=1; break;\n"
              "    }\n"
              "    return 0;\n"
              "    j=2;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:7:5]: (style) Statements following 'return' will never be executed. [unreachableCode]\n", errout_str());

        check("int foo() {\n"
              "    return 0;\n"
              "  label:\n"
              "    throw 0;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:3:3]: (style) Label 'label' is not used. [unusedLabel]\n", errout_str());

        check("struct A {\n"
              "    virtual void foo  (P & Val) throw ();\n"
              "    virtual void foo1 (P & Val) throw ();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int foo() {\n"
              "    goto label;\n"
              "    while (true) {\n"
              "     bar();\n"
              "     label:\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str()); // #3457

        check("int foo() {\n"
              "    goto label;\n"
              "    do {\n"
              "     bar();\n"
              "     label:\n"
              "    } while (true);\n"
              "}");
        ASSERT_EQUALS("", errout_str()); // #3457

        check("int foo() {\n"
              "    goto label;\n"
              "    for (;;) {\n"
              "     bar();\n"
              "     label:\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str()); // #3457

        // #3383. TODO: Use preprocessor
        check("int foo() {\n"
              "\n" // #ifdef A
              "    return 0;\n"
              "\n" // #endif
              "    return 1;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
        check("int foo() {\n"
              "\n" // #ifdef A
              "    return 0;\n"
              "\n" // #endif
              "    return 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5]: (style, inconclusive) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n", errout_str());

        // #4711 lambda functions
        check("int f() {\n"
              "    return g([](int x){(void)x+1; return x;});\n"
              "}",
              dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // #4756
        check("template <>\n"
              "inline uint16_t htobe(uint16_t value) {\n"
              "     return ( __extension__ ({\n"
              "         register unsigned short int __v, __x = (unsigned short int) (value);\n"
              "         if (__builtin_constant_p (__x))\n"
              "             __v = ((unsigned short int) ((((__x) >> 8) & 0xff) | (((__x) & 0xff) << 8)));\n"
              "         else\n"
              "             __asm__ (\"rorw $8, %w0\" : \"=r\" (__v) : \"0\" (__x) : \"cc\");\n"
              "         (void)__v;\n"
              "     }));\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // #6008
        check("static std::function< int ( int, int ) > GetFunctor() {\n"
              "    return [](int a_, int b_) -> int {\n"
              "        int sum = a_ + b_;\n"
              "        return sum;\n"
              "    };\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // #5789
        check("struct per_state_info {\n"
              "    uint64_t enter, exit;\n"
              "    uint64_t events;\n"
              "    per_state_info() : enter(0), exit(0), events(0) {}\n"
              "};", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // #6664
        check("void foo() {\n"
              "    (beat < 100) ? (void)0 : exit(0);\n"
              "    bar();\n"
              "}", dinit(CheckOptions, $.inconclusive = false, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    (beat < 100) ? exit(0) : (void)0;\n"
              "    bar();\n"
              "}", dinit(CheckOptions, $.inconclusive = false, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        // #8261
        // TODO Do not throw AST validation exception
        TODO_ASSERT_THROW(check("void foo() {\n"
                                "    (beat < 100) ? (void)0 : throw(0);\n"
                                "    bar();\n"
                                "}", dinit(CheckOptions, $.inconclusive = false, $.settings = &settings)), InternalError);
        //ASSERT_EQUALS("", errout_str());

        check("int foo() {\n"
              "    exit(0);\n"
              "    return 1;\n" // <- clarify for tools that function does not continue..
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    enum : uint8_t { A, B } var = A;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        checkP("#define INB(x) __extension__ ({ u_int tmp = (x); inb(tmp); })\n" // #4739
               "static unsigned char cmos_hal_read(unsigned index) {\n"
               "    unsigned short port_0, port_1;\n"
               "    assert(!verify_cmos_byte_index(index));\n"
               "    if (index < 128) {\n"
               "      port_0 = 0x70;\n"
               "      port_1 = 0x71;\n"
               "    }\n"
               "    else {\n"
               "      port_0 = 0x72;\n"
               "      port_1 = 0x73;\n"
               "    }\n"
               "    OUTB(index, port_0);\n"
               "    return INB(port_1);\n"
               "}\n", dinit(CheckPOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("[[noreturn]] void n();\n"
              "void f() {\n"
              "    n();\n"
              "    g();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:5]: (style) Statements following noreturn function 'n()' will never be executed. [unreachableCode]\n", errout_str());

        check("void f() {\n"
              "    exit(1);\n"
              "    g();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Statements following noreturn function 'exit()' will never be executed. [unreachableCode]\n", errout_str());

        check("void f() {\n"
              "    do {\n"
              "        break;\n"
              "        g();\n"
              "    } while (0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:9]: (style) Statements following 'break' will never be executed. [unreachableCode]\n", errout_str());

        check("void f() {\n" // #12244
              "    {\n"
              "        std::cout << \"x\";\n"
              "        return;\n"
              "    }\n"
              "    std::cout << \"y\";\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:5]: (style) Statements following 'return' will never be executed. [unreachableCode]\n", errout_str());

        check("void f() {\n"
              "    {\n"
              "        std::cout << \"x\";\n"
              "        exit(1);\n"
              "    }\n"
              "    std::cout << \"y\";\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:5]: (style) Statements following noreturn function 'exit()' will never be executed. [unreachableCode]\n", errout_str());

        check("int f() {\n" // #13475
              "    { return 0; };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int i) {\n" // #13478
              "    int x = 0;\n"
              "    switch (i) {\n"
              "        { case 0: x = 5; break; }\n"
              "        { case 1: x = 7; break; }\n"
              "    }\n"
              "    return x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int c) {\n"
              "    switch (c) {\n"
              "    case '\\n':\n"
              "    { return 1; };\n"
              "    default:\n"
              "    { return c; };\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int main(int argc, char *argv[]) {\n" // #11
              "    switch (argc) {\n"
              "        case 0: {\n"
              "            return 1;\n"
              "        }\n"
              "        break;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:9]: (style) Consecutive return, break, continue, goto or throw statements are unnecessary. [duplicateBreak]\n"
                      "[test.cpp:1:26]: (style) Parameter 'argv' can be declared as const array [constParameter]\n",
                      errout_str());

        check("int f(int i) {\n" // #13491
              "    switch (i) {\n"
              "    case 0:\n"
              "        return 0;\n"
              "        int j;\n"
              "    case 1:\n"
              "    case 2:\n"
              "        j = 5;\n"
              "        return j + i;\n"
              "    }\n"
              "    return 3;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int i) {\n"
              "    switch (i) {\n"
              "    {\n"
              "    case 0:\n"
              "        return 0;\n"
              "    }\n"
              "    {\n"
              "        int j;\n"
              "    case 1:\n"
              "    case 2:\n"
              "        j = 5;\n"
              "        return j + i;\n"
              "    }\n"
              "    }\n"
              "    return 3;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int i) {\n"
              "    switch (i) {\n"
              "    case 0:\n"
              "        return 0;\n"
              "        int a[1];\n"
              "    case 1:\n"
              "    case 2:\n"
              "        a[0] = 5;\n"
              "        return a[0] + i;\n"
              "    }\n"
              "    return 3;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int i) {\n"
              "    switch (i) {\n"
              "    case 0:\n"
              "        return 0;\n"
              "        int j;\n"
              "        dostuff();\n"
              "    case 1:\n"
              "    case 2:\n"
              "        j = 5;\n"
              "        return j + i;\n"
              "    }\n"
              "    return 3;\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:6:5]: (style) Statements following 'return' will never be executed. [unreachableCode]\n", "", errout_str());

        check("int f() {\n" // #13472
              "    int var;\n"
              "    auto int ret();\n"
              "    int ret() {\n"
              "        return var;\n"
              "    }\n"
              "    var = 42;\n"
              "    return ret();\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #13516
              "    io_uring_for_each_cqe(&ring, head, cqe) {\n"
              "        if (cqe->res == -EOPNOTSUPP)\n"
              "            printf(\"error\");\n"
              "        goto ok;\n"
              "    }\n"
              "    usleep(10000);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantContinue() {
        check("void f() {\n" // #11195
              "    for (int i = 0; i < 10; ++i) {\n"
              "        printf(\"i = %d\\n\", i);\n"
              "        continue;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:9]: (style) 'continue' is redundant since it is the last statement in a loop. [redundantContinue]\n", errout_str());

        check("void f() {\n"
              "    int i = 0;"
              "    do {\n"
              "        ++i;\n"
              "        printf(\"i = %d\\n\", i);\n"
              "        continue;\n"
              "    } while (i < 10);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:9]: (style) 'continue' is redundant since it is the last statement in a loop. [redundantContinue]\n", errout_str());

        check("int f() {\n" // #13475
              "    { return 0; };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int i) {\n" // #13478
              "    int x = 0;\n"
              "    switch (i) {\n"
              "        { case 0: x = 5; break; }\n"
              "        { case 1: x = 7; break; }\n"
              "    }\n"
              "    return x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int x, int y) {\n" // #13544
              "    switch (x) {\n"
              "    case 1: {\n"
              "        return y != 0;\n"
              "        int z = y + 5;\n"
              "        return z != 7;\n"
              "    }\n"
              "    }\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:9]: (style) Statements following 'return' will never be executed. [unreachableCode]\n", errout_str());
    }


    void suspiciousCase() {
        check("void foo() {\n"
              "    switch(a) {\n"
              "        case A&&B:\n"
              "            foo();\n"
              "        case (A||B):\n"
              "            foo();\n"
              "        case A||B:\n"
              "            foo();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (warning, inconclusive) Found suspicious case label in switch(). Operator '&&' probably doesn't work as intended. [suspiciousCase]\n"
                      "[test.cpp:5:16]: (warning, inconclusive) Found suspicious case label in switch(). Operator '||' probably doesn't work as intended. [suspiciousCase]\n"
                      "[test.cpp:7:15]: (warning, inconclusive) Found suspicious case label in switch(). Operator '||' probably doesn't work as intended. [suspiciousCase]\n", errout_str());

        check("void foo() {\n"
              "    switch(a) {\n"
              "        case 1:\n"
              "            a=A&&B;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // TODO Do not throw AST validation exception
        TODO_ASSERT_THROW(check("void foo() {\n"
                                "    switch(a) {\n"
                                "        case A&&B?B:A:\n"
                                "            foo();\n"
                                "    }\n"
                                "}"), InternalError);
        //ASSERT_EQUALS("", errout_str());
    }

    void suspiciousEqualityComparison() {
        check("void foo(int c) {\n"
              "    if (x) c == 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead? [constStatement]\n", errout_str());

        check("void foo(const int* c) {\n"
              "    if (x) *c == 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead? [constStatement]\n", errout_str());


        check("void foo(int c) {\n"
              "    if (c == 1) {\n"
              "        c = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int c) {\n"
              "    c == 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead? [constStatement]\n", errout_str());

        check("void foo(int c) {\n"
              "    for (int i = 0; i == 10; i ++) {\n"
              "        a ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int c) {\n"
              "    for (i == 0; i < 10; i ++) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead? [constStatement]\n", errout_str());

        check("void foo(int c) {\n"
              "    for (i == 1; i < 10; i ++) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead? [constStatement]\n", errout_str());

        check("void foo(int c) {\n"
              "    for (i == 2; i < 10; i ++) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead? [constStatement]\n", errout_str());

        check("void foo(int c) {\n"
              "    for (int i = 0; i < 10; i == c) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:31]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead? [constStatement]\n", errout_str());

        check("void foo(int c) {\n"
              "    for (; running == 1;) {\n"
              "        c ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int c) {\n"
              "    printf(\"%i\", ({x==0;}));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int arg) {\n"
              "    printf(\"%i\", ({int x = do_something(); x == 0;}));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int x) {\n"
              "    printf(\"%i\", ({x == 0; x > 0 ? 10 : 20}));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (warning, inconclusive) Found suspicious equality comparison. Did you intend to assign a value instead? [constStatement]\n", errout_str());

        check("void foo(int x) {\n"
              "    for (const Token* end = tok->link(); tok != end; tok = (tok == end) ? end : tok->next()) {\n"
              "        x++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int x) {\n"
              "    for (int i = (x == 0) ? 0 : 5; i < 10; i ++) {\n"
              "        x++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int x) {\n"
              "    for (int i = 0; i < 10; i += (x == 5) ? 1 : 2) {\n"
              "        x++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void suspiciousUnaryPlusMinus() { // #8004
        check("int g() { return 1; }\n"
              "void f() {\n"
              "    +g();\n"
              "    -g();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning, inconclusive) Found suspicious operator '+', result is not used. [constStatement]\n"
                      "[test.cpp:4:5]: (warning, inconclusive) Found suspicious operator '-', result is not used. [constStatement]\n",
                      errout_str());

        check("void f(int i) {\n"
              "    +i;\n"
              "    -i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '+', result is not used. [constStatement]\n"
                      "[test.cpp:3:5]: (warning, inconclusive) Found suspicious operator '-', result is not used. [constStatement]\n",
                      errout_str());
    }

    void suspiciousFloatingPointCast() {
        check("double f(double a, double b, float c) {\n"
              "    return a + (float)b + c;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Floating-point cast causes loss of precision. [suspiciousFloatingPointCast]\n", errout_str());

        check("double f(double a, double b, float c) {\n"
              "    return a + static_cast<float>(b) + c;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:34]: (style) Floating-point cast causes loss of precision. [suspiciousFloatingPointCast]\n", errout_str());

        check("long double f(long double a, long double b, float c) {\n"
              "    return a + (double)b + c;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Floating-point cast causes loss of precision. [suspiciousFloatingPointCast]\n", errout_str());

        check("void g(int, double);\n"
              "void h(double);\n"
              "void f(double d) {\n"
              "    g(1, (float)d);\n"
              "    h((float)d);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Floating-point cast causes loss of precision. [suspiciousFloatingPointCast]\n"
                      "[test.cpp:5:7]: (style) Floating-point cast causes loss of precision. [suspiciousFloatingPointCast]\n",
                      errout_str());
    }

    void selfAssignment() {
        check("void foo()\n"
              "{\n"
              "    int x = 1;\n"
              "    x = x;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Redundant assignment of 'x' to itself. [selfAssignment]\n", errout_str());

        check("void foo()\n"
              "{\n"
              "    int x = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Redundant assignment of 'x' to itself. [selfAssignment]\n", errout_str());

        check("struct A { int b; };\n"
              "void foo(A* a1, A* a2) {\n"
              "    a1->b = a1->b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Redundant assignment of 'a1->b' to itself. [selfAssignment]\n", errout_str());

        check("int x;\n"
              "void f()\n"
              "{\n"
              "    x = x = 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Redundant assignment of 'x' to itself. [selfAssignment]\n", errout_str());

        // #4073 (segmentation fault)
        check("void Foo::myFunc( int a )\n"
              "{\n"
              "    if (a == 42)\n"
              "    a = a;\n"
              "}");

        check("void foo()\n"
              "{\n"
              "    int x = 1;\n"
              "    x = x + 1;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "        int *x = getx();\n"
              "        *x = x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    BAR *x = getx();\n"
              "    x = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) Redundant assignment of 'x' to itself. [selfAssignment]\n", errout_str());

        // #2502 - non-primitive type -> there might be some side effects
        check("void foo()\n"
              "{\n"
              "    Fred fred; fred = fred;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    x = (x == 0);"
              "    func(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    x = (x != 0);"
              "    func(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #3001 - false positive
        check("void foo(int x) {\n"
              "    x = x ? x : 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3800 - false negative when variable is extern
        check("extern int i;\n"
              "void f() {\n"
              "    i = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) Redundant assignment of 'i' to itself. [selfAssignment]\n", errout_str());

        // #4291 - id for variables accessed through 'this'
        check("class Foo {\n"
              "    int var;\n"
              "    void func();\n"
              "};\n"
              "void Foo::func() {\n"
              "    this->var = var;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:15]: (style) Redundant assignment of 'this->var' to itself. [selfAssignment]\n", errout_str());

        check("class Foo {\n"
              "    int var;\n"
              "    void func(int var);\n"
              "};\n"
              "void Foo::func(int var) {\n"
              "    this->var = var;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6406 - designated initializer doing bogus self assignment
        check("struct callbacks {\n"
              "    void (*s)(void);\n"
              "};\n"
              "void something(void) {}\n"
              "void f() {\n"
              "    struct callbacks ops = { .s = ops.s };\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (style) Redundant assignment of 'something' to itself.\n", "", errout_str());

        check("class V\n"
              "{\n"
              "public:\n"
              "    V()\n"
              "    {\n"
              "        x = y = z = 0.0;\n"
              "    }\n"
              "    V( double x, const double y_, const double &z_)\n"
              "    {\n"
              "        x = x; y = y; z = z;\n"
              "    }\n"
              "    double x, y, z;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:10:11]: (style) Redundant assignment of 'x' to itself. [selfAssignment]\n"
                      "[test.cpp:10:18]: (style) Redundant assignment of 'y' to itself. [selfAssignment]\n"
                      "[test.cpp:10:25]: (style) Redundant assignment of 'z' to itself. [selfAssignment]\n", errout_str());

        check("void f(int i) { i = !!i; }");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    int x = 1;\n"
              "    int &ref = x;\n"
              "    ref = x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:9]: (style) Redundant assignment of 'ref' to itself. [selfAssignment]\n", errout_str());

        check("class Foo {\n" // #9850
              "    int i{};\n"
              "    void modify();\n"
              "    void method() {\n"
              "        Foo copy = *this;\n"
              "        modify();\n"
              "        *this = copy;\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #11383
              "    void f() {\n"
              "        int x = 42;"
              "        auto l2 = [i = i, x, y = 0]() { return i + x + y; };\n"
              "    }\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #10337
              "    int b[2] = { 1, 2 };\n"
              "    int idx = 0;\n"
              "    int& i = b[idx];\n"
              "    idx++;\n"
              "    i = b[idx];\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(int*);\n" // #12390
              "void f() {\n"
              "    int o = s.i;\n"
              "    g(&s.i);\n"
              "    s.i = o;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int N;\n" // #14234
              "void f() { ::N = N; }\n");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Redundant assignment of '::N' to itself. [selfAssignment]\n", errout_str());
    }

    void trac1132() {
        check("class Lock\n"
              "{\n"
              "public:\n"
              "    Lock(int i)\n"
              "    {\n"
              "        std::cout << \"Lock \" << i << std::endl;\n"
              "    }\n"
              "    ~Lock()\n"
              "    {\n"
              "        std::cout << \"~Lock\" << std::endl;\n"
              "    }\n"
              "};\n"
              "int main()\n"
              "{\n"
              "    Lock(123);\n"
              "    std::cout << \"hello\" << std::endl;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:15:5]: (style) Instance of 'Lock' object is destroyed immediately. [unusedScopedObject]\n", errout_str());
    }

    void trac3693() {
        check("struct A{\n"
              "  enum {\n"
              "    b = 300\n"
              "  };\n"
              "};\n"
              "const int DFLT_TIMEOUT = A::b % 1000000 ;\n", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectDoesNotPickFunction1() {
        check("int main ( )\n"
              "{\n"
              "    CouldBeFunction ( 123 ) ;\n"
              "    return 0 ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectDoesNotPickFunction2() {
        check("struct error {\n"
              "    error() {}\n"
              "};\n"
              "\n"
              "class parser {\n"
              "public:\n"
              "    void error() const {}\n"
              "\n"
              "    void foo() const {\n"
              "        error();\n"
              "        do_something();\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectPicksClass() {
        check("class NotAFunction ;\n"
              "int function ( )\n"
              "{\n"
              "    NotAFunction ( 123 );\n"
              "    return 0 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (style) Instance of 'NotAFunction' object is destroyed immediately. [unusedScopedObject]\n", errout_str());
    }

    void testMisusedScopeObjectPicksStruct() {
        check("struct NotAClass;\n"
              "bool func ( )\n"
              "{\n"
              "    NotAClass ( 123 ) ;\n"
              "    return true ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (style) Instance of 'NotAClass' object is destroyed immediately. [unusedScopedObject]\n", errout_str());
    }

    void testMisusedScopeObjectDoesNotPickIf() {
        check("bool func( int a , int b , int c )\n"
              "{\n"
              "    if ( a > b ) return c == a ;\n"
              "    return b == a ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectDoesNotPickConstructorDeclaration() {
        check("class Something : public SomethingElse\n"
              "{\n"
              "public:\n"
              "~Something ( ) ;\n"
              "Something ( ) ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectDoesNotPickFunctor() {
        check("class IncrementFunctor\n"
              "{\n"
              "public:\n"
              "    void operator()(int &i)\n"
              "    {\n"
              "        ++i;\n"
              "    }\n"
              "};\n"
              "\n"
              "int main()\n"
              "{\n"
              "    int a = 1;\n"
              "    IncrementFunctor()(a);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectDoesNotPickLocalClassConstructors() {
        check("void f() {\n"
              "    class Foo {\n"
              "        Foo() { }\n"
              "        Foo(int a) { }\n"
              "        Foo(int a, int b) { }\n"
              "    };\n"
              "    Foo();\n"
              "    do_something();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:5]: (style) Instance of 'Foo' object is destroyed immediately. [unusedScopedObject]\n", errout_str());
    }

    void testMisusedScopeObjectDoesNotPickUsedObject() {
        check("struct Foo {\n"
              "    void bar() {\n"
              "    }\n"
              "};\n"
              "\n"
              "void fn() {\n"
              "    Foo().bar();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectDoesNotPickPureC() {
        // Ticket #2352
        const char code[] = "struct cb_watch_bool {\n"
                            "    int a;\n"
                            "};\n"
                            "\n"
                            "void f()\n"
                            "{\n"
                            "    cb_watch_bool();\n"
                            "    do_something();\n"
                            "}\n";

        check(code);
        ASSERT_EQUALS("[test.cpp:7:5]: (style) Instance of 'cb_watch_bool' object is destroyed immediately. [unusedScopedObject]\n", errout_str());

        check(code, dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        // Ticket #2639
        check("struct stat { int a; int b; };\n"
              "void stat(const char *fn, struct stat *);\n"
              "\n"
              "void foo() {\n"
              "    stat(\"file.txt\", &st);\n"
              "    do_something();\n"
              "}");
        ASSERT_EQUALS("",errout_str());

        check("struct AMethodObject {\n" // #4336
              "    AMethodObject(double, double, double);\n"
              "};\n"
              "struct S {\n"
              "    static void A(double a1, double a2, double a3);\n"
              "};\n"
              "void S::A(double const a1, double const a2, double const a3) {\n"
              "    AMethodObject(a1, a2, a3);\n"
              "}\n");
        ASSERT_EQUALS("",errout_str());
    }

    void testMisusedScopeObjectDoesNotPickNestedClass() {
        const char code[] = "class ios_base {\n"
                            "public:\n"
                            "  class Init {\n"
                            "  public:\n"
                            "  };\n"
                            "};\n"
                            "class foo {\n"
                            "public:\n"
                            "  foo();\n"
                            "  void Init(int);\n"
                            "};\n"
                            "foo::foo() {\n"
                            "  Init(0);\n"
                            "  do_something();\n"
                            "}\n";

        check(code);
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectInConstructor() {
        const char code[] = "class Foo {\n"
                            "public:\n"
                            "  Foo(char x) {\n"
                            "    Foo(x, 0);\n"
                            "    do_something();\n"
                            "  }\n"
                            "  Foo(char x, int y) { }\n"
                            "};\n";
        check(code);
        ASSERT_EQUALS("[test.cpp:4:5]: (style) Instance of 'Foo' object is destroyed immediately. [unusedScopedObject]\n", errout_str());
    }

    void testMisusedScopeObjectStandardType() {
        check("int g();\n"
              "void f(int i) {\n"
              "    int();\n"
              "    int(0);\n"
              "    int( g() );\n" // don't warn
              "    int{};\n"
              "    int{ 0 };\n"
              "    int{ i };\n"
              "    int{ g() };\n" // don't warn
              "    g();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Instance of 'int' object is destroyed immediately. [unusedScopedObject]\n"
                      "[test.cpp:4:5]: (style) Instance of 'int' object is destroyed immediately. [unusedScopedObject]\n"
                      "[test.cpp:6:5]: (style) Instance of 'int' object is destroyed immediately. [unusedScopedObject]\n"
                      "[test.cpp:7:5]: (style) Instance of 'int' object is destroyed immediately. [unusedScopedObject]\n"
                      "[test.cpp:8:5]: (style) Instance of 'int' object is destroyed immediately. [unusedScopedObject]\n",
                      errout_str());

        check("void f(int j) {\n"
              "    for (; bool(j); ) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g() {\n"
              "    float (f);\n"
              "    float (*p);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int i) {\n"
              "    void();\n"
              "    return i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectNamespace() {
        check("namespace M {\n" // #4779
              "    namespace N {\n"
              "        struct S {};\n"
              "    }\n"
              "}\n"
              "int f() {\n"
              "    M::N::S();\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:11]: (style) Instance of 'M::N::S' object is destroyed immediately. [unusedScopedObject]\n", errout_str());

        check("void f() {\n" // #10057
              "    std::string(\"abc\");\n"
              "    std::string{ \"abc\" };\n"
              "    std::pair<int, int>(1, 2);\n"
              "    (void)0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Instance of 'std::string' object is destroyed immediately. [unusedScopedObject]\n"
                      "[test.cpp:3:10]: (style) Instance of 'std::string' object is destroyed immediately. [unusedScopedObject]\n"
                      "[test.cpp:4:10]: (style) Instance of 'std::pair' object is destroyed immediately. [unusedScopedObject]\n",
                      errout_str());

        check("struct S {\n" // #10083
              "    void f() {\n"
              "        std::lock_guard<std::mutex>(m);\n"
              "    }\n"
              "    void g() {\n"
              "        std::scoped_lock<std::mutex>(m);\n"
              "    }\n"
              "    void h() {\n"
              "        std::scoped_lock(m);\n"
              "    }\n"
              "    std::mutex m;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:14]: (style) Instance of 'std::lock_guard' object is destroyed immediately. [unusedScopedObject]\n"
                      "[test.cpp:6:14]: (style) Instance of 'std::scoped_lock' object is destroyed immediately. [unusedScopedObject]\n"
                      "[test.cpp:9:14]: (style) Instance of 'std::scoped_lock' object is destroyed immediately. [unusedScopedObject]\n",
                      errout_str());

        check("struct S { int i; };\n"
              "namespace {\n"
              "    S s() { return ::S{42}; }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void testMisusedScopeObjectAssignment() { // #11371
        check("struct S;\n"
              "S f();\n"
              "S& g();\n"
              "S&& h();\n"
              "S* i();\n"
              "void t0() { f() = {}; }\n"
              "void t1() { g() = {}; }\n"
              "void t2() { h() = {}; }\n"
              "void t3() { *i() = {}; }\n");
        ASSERT_EQUALS("[test.cpp:6:19]: (style) Instance of 'S' object is destroyed immediately, assignment has no effect. [unusedScopedObject]\n", errout_str());
    }

    void trac2084() {
        check("void f()\n"
              "{\n"
              "    struct sigaction sa;\n"
              "\n"
              "    { sigaction(SIGHUP, &sa, 0); };\n"
              "    { sigaction(SIGINT, &sa, 0); };\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void trac2071() {
        check("void f() {\n"
              "    struct AB {\n"
              "        AB(int a) { }\n"
              "    };\n"
              "\n"
              "    const AB ab[3] = { AB(0), AB(1), AB(2) };\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void clarifyCalculation() {
        check("int f(char c) {\n"
              "    return 10 * (c == 0) ? 1 : 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:26]: (style) Clarify calculation precedence for '*' and '?'. [clarifyCalculation]\n", errout_str());

        check("void f(char c) {\n"
              "    printf(\"%i\", 10 * (c == 0) ? 1 : 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:32]: (style) Clarify calculation precedence for '*' and '?'. [clarifyCalculation]\n", errout_str());

        check("void f() {\n"
              "    return (2*a)?b:c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char c) {\n"
              "    printf(\"%i\", a + b ? 1 : 2);\n"
              "}",dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:2:24]: (style) Clarify calculation precedence for '+' and '?'. [clarifyCalculation]\n", errout_str());

        check("void f() {\n"
              "    std::cout << x << y ? 2 : 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25]: (style) Clarify calculation precedence for '<<' and '?'. [clarifyCalculation]\n", errout_str());

        check("void f() {\n"
              "    int ab = a - b ? 2 : 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (style) Clarify calculation precedence for '-' and '?'. [clarifyCalculation]\n", errout_str());

        check("void f() {\n"
              "    int ab = a | b ? 2 : 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (style) Clarify calculation precedence for '|' and '?'. [clarifyCalculation]\n", errout_str());

        // ticket #195
        check("int f(int x, int y) {\n"
              "    return x >> ! y ? 8 : 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:21]: (style) Clarify calculation precedence for '>>' and '?'. [clarifyCalculation]\n", errout_str());

        check("int f() {\n"
              "   return shift < sizeof(int64_t)*8 ? 1 : 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() { a = *p ? 1 : 2; }");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) { const char *p = x & 1 ? \"1\" : \"0\"; }");
        ASSERT_EQUALS("", errout_str());

        check("void foo() { x = a % b ? \"1\" : \"0\"; }");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) { return x & 1 ? '1' : '0'; }");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) { return x & 16 ? 1 : 0; }");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) { return x % 16 ? 1 : 0; }");
        ASSERT_EQUALS("", errout_str());

        check("enum {X,Y}; void f(int x) { return x & Y ? 1 : 0; }");
        ASSERT_EQUALS("", errout_str());
    }

    void clarifyStatement() {
        check("char* f(char* c) {\n"
              "    *c++;\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '*', result is not used. [constStatement]\n"
            "[test.cpp:2:7]: (warning) In expression like '*A++' the result of '*' is unused. Did you intend to write '(*A)++;'? [clarifyStatement]\n",
            errout_str());

        check("char* f(char** c) {\n"
              "    *c[5]--;\n"
              "    return *c;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '*', result is not used. [constStatement]\n"
            "[test.cpp:2:10]: (warning) In expression like '*A++' the result of '*' is unused. Did you intend to write '(*A)++;'? [clarifyStatement]\n",
            errout_str());

        check("void f(Foo f) {\n"
              "    *f.a++;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '*', result is not used. [constStatement]\n"
            "[test.cpp:2:9]: (warning) In expression like '*A++' the result of '*' is unused. Did you intend to write '(*A)++;'? [clarifyStatement]\n",
            errout_str());

        check("void f(Foo f) {\n"
              "    *f.a[5].v[3]++;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '*', result is not used. [constStatement]\n"
            "[test.cpp:2:17]: (warning) In expression like '*A++' the result of '*' is unused. Did you intend to write '(*A)++;'? [clarifyStatement]\n",
            errout_str());

        check("void f(Foo f) {\n"
              "    *f.a(1, 5).v[x + y]++;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '*', result is not used. [constStatement]\n"
            "[test.cpp:2:24]: (warning) In expression like '*A++' the result of '*' is unused. Did you intend to write '(*A)++;'? [clarifyStatement]\n",
            errout_str());

        check("char* f(char* c) {\n"
              "    (*c)++;\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char* c) {\n"
              "    bar(*c++);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("char*** f(char*** c) {\n"
              "    ***c++;\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '*', result is not used. [constStatement]\n"
            "[test.cpp:2:9]: (warning) In expression like '*A++' the result of '*' is unused. Did you intend to write '(*A)++;'? [clarifyStatement]\n",
            errout_str());

        check("char** f(char*** c) {\n"
              "    **c[5]--;\n"
              "    return **c;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '*', result is not used. [constStatement]\n"
            "[test.cpp:2:11]: (warning) In expression like '*A++' the result of '*' is unused. Did you intend to write '(*A)++;'? [clarifyStatement]\n",
            errout_str());

        check("char*** f(char*** c) {\n"
              "    (***c)++;\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const int*** p) {\n" // #10923
              "    delete[] **p;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void *f(char** c) {\n"
              "    bar(**c++);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void *f(char* p) {\n"
              "    for (p = path; *p++;) ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::array<std::array<double,3>,3> array;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::vector<int*>& v) {\n" // #12088
              "    for (auto it = v.begin(); it != v.end(); delete *it++);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateBranch() {
        check("void f(int a, int &b) {\n"
              "    if (a)\n"
              "        b = 1;\n"
              "    else\n"
              "        b = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5] -> [test.cpp:2:5]: (style, inconclusive) Found duplicate branches for 'if' and 'else'. [duplicateBranch]\n", errout_str());

        check("void f(int a, int &b) {\n"
              "    if (a) {\n"
              "        if (a == 1)\n"
              "            b = 2;\n"
              "        else\n"
              "            b = 2;\n"
              "    } else\n"
              "        b = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:3:9]: (style, inconclusive) Found duplicate branches for 'if' and 'else'. [duplicateBranch]\n", errout_str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1)\n"
              "        b = 1;\n"
              "    else {\n"
              "        if (a)\n"
              "            b = 2;\n"
              "        else\n"
              "            b = 2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:9] -> [test.cpp:5:9]: (style, inconclusive) Found duplicate branches for 'if' and 'else'. [duplicateBranch]\n", errout_str());

        check("int f(int signed, unsigned char value) {\n"
              "    int ret;\n"
              "    if (signed)\n"
              "        ret = (signed char)value;\n"  // cast must be kept so the simplifications and verification is skipped
              "    else\n"
              "        ret = (unsigned char)value;\n"
              "    return ret;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    if (b)\n"
              "        __asm__(\"mov ax, bx\");\n"
              "    else\n"
              "        __asm__(\"mov bx, bx\");\n"
              "}");
        ASSERT_EQUALS("", errout_str()); // #3407

        check("void f() {\n"
              "    if (b)\n"
              "        __asm__(\"mov ax, bx\");\n"
              "    else\n"
              "        __asm__(\"mov ax, bx\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5] -> [test.cpp:2:5]: (style, inconclusive) Found duplicate branches for 'if' and 'else'. [duplicateBranch]\n", errout_str());
    }

    void duplicateBranch1() {

        // tests inspired by http://www.viva64.com/en/b/0149/ ( Comparison between PVS-Studio and cppcheck )
        // Errors detected in Quake 3: Arena by PVS-Studio: Fragment 2
        check("void f()\n"
              "{\n"
              "  if (front < 0)\n"
              "    frac = front/(front-back);\n"
              "  else\n"
              "    frac = front/(front-back);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:3] -> [test.cpp:3:3]: (style, inconclusive) Found duplicate branches for 'if' and 'else'. [duplicateBranch]\n", errout_str());

        check("void f()\n"
              "{\n"
              "  if (front < 0)\n"
              "  { frac = front/(front-back);}\n"
              "  else\n"
              "    frac = front/((front-back));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:3] -> [test.cpp:3:3]: (style, inconclusive) Found duplicate branches for 'if' and 'else'. [duplicateBranch]\n", errout_str());

        // No message about empty branches (#5354)
        check("void f()\n"
              "{\n"
              "  if (front < 0)\n"
              "  {}\n"
              "  else\n"
              "  {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateBranch2() {
        checkP("#define DOSTUFF1 ;\n"
               "#define DOSTUFF2 ;\n"
               "void f(int x) {\n" // #4329
               "  if (x)\n"
               "    DOSTUFF1\n"
               "  else\n"
               "    DOSTUFF2\n"
               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateBranch3() {
        check("void f(bool b, int i) {\n"
              "    int j = i;\n"
              "    if (b) {\n"
              "        x = i;\n"
              "    } else {\n"
              "        x = j;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:5:7] -> [test.cpp:3:5]: (style, inconclusive) Found duplicate branches for 'if' and 'else'. [duplicateBranch]\n"
                      "[test.cpp:2:9]: (style) The scope of the variable 'j' can be reduced. [variableScope]\n",
                      errout_str());

        check("void f(bool b, int i) {\n"
              "    int j = i;\n"
              "    i++;\n"
              "    if (b) {\n"
              "        x = i;\n"
              "    } else {\n"
              "        x = j;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateBranch4() {
        check("void* f(bool b) {\n"
              "    if (b) {\n"
              "        return new A::Y(true);\n"
              "    } else {\n"
              "        return new A::Z(true);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateBranch5() {
        check("void f(bool b) {\n"
              "    int j;\n"
              "    if (b) {\n"
              "        unsigned int i = 0;\n"
              "        j = i;\n"
              "    } else {\n"
              "        unsigned int i = 0;\n"
              "        j = i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:7] -> [test.cpp:3:5]: (style, inconclusive) Found duplicate branches for 'if' and 'else'. [duplicateBranch]\n", errout_str());

        check("void f(bool b) {\n"
              "    int j;\n"
              "    if (b) {\n"
              "        unsigned int i = 0;\n"
              "        j = i;\n"
              "    } else {\n"
              "        unsigned int i = 0;\n"
              "        j = 1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n"
              "    int j;\n"
              "    if (b) {\n"
              "        unsigned int i = 0;\n"
              "    } else {\n"
              "        int i = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n"
              "    int j;\n"
              "    if (b) {\n"
              "        unsigned int i = 0;\n"
              "        j = i;\n"
              "    } else {\n"
              "        int i = 0;\n"
              "        j = i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateBranch6() {
        check("void f(bool b) {\n"
              "    if (b) {\n"
              "    } else {\n"
              "        int i = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n"
              "    if (b) {\n"
              "        int i = 0;\n"
              "    } else {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression1() {
        check("void foo(int a) {\n"
              "    if (a == a) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Same expression on both sides of '=='. [duplicateExpression]\n", errout_str());

        check("void fun(int b) {\n"
              "    return  a && a ||\n"
              "            b == b &&\n"
              "            d > d &&\n"
              "            e < e &&\n"
              "            f ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Same expression on both sides of '&&'. [duplicateExpression]\n"
                      "[test.cpp:3:15]: (style) Same expression on both sides of '=='. [duplicateExpression]\n"
                      "[test.cpp:4:15]: (style) Same expression on both sides of '>'. [duplicateExpression]\n"
                      "[test.cpp:5:15]: (style) Same expression on both sides of '<'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    return a && a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (style) Same expression on both sides of '&&'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    a = b && b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Same expression on both sides of '&&'. [duplicateExpression]\n", errout_str());

        check("void foo(int b) {\n"
              "    f(a,b == b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Same expression on both sides of '=='. [duplicateExpression]\n", errout_str());

        check("void foo(int b) {\n"
              "    f(b == b, a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Same expression on both sides of '=='. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if (x!=2 || x!=2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (style) Same expression on both sides of '||'. [duplicateExpression]\n", errout_str());

        check("void foo(int a, int b) {\n"
              "    if ((a < b) && (b > a)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Same expression on both sides of '&&' because 'a<b' and 'b>a' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void foo(int a, int b) {\n"
              "    if ((a <= b) && (b >= a)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (style) Same expression on both sides of '&&' because 'a<=b' and 'b>=a' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"
              "    if (x!=2 || y!=3 || x!=2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (style) Same expression 'x!=2' found multiple times in chain of '||' operators. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if (x!=2 && (x=y) && x!=2) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    if (a && b || a && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Same expression on both sides of '||'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if (a && b || b && c) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    if (a && b | b && c) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Same expression on both sides of '|'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if ((a + b) | (a + b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Same expression on both sides of '|'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if ((a | b) & (a | b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Same expression on both sides of '&'. [duplicateExpression]\n", errout_str());

        check("void foo(int a, int b) {\n"
              "    if ((a | b) == (a | b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Same expression on both sides of '=='. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if (a1[a2[c & 0xff] & 0xff]) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void d(const char f, int o, int v)\n"
              "{\n"
              "     if (((f=='R') && (o == 1) && ((v < 2) || (v > 99))) ||\n"
              "         ((f=='R') && (o == 2) && ((v < 2) || (v > 99))) ||\n"
              "         ((f=='T') && (o == 2) && ((v < 200) || (v > 9999)))) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int x) { return x+x; }");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) { while (x+=x) ; }");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    if (a && b && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Same expression on both sides of '&&'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if (a || b || b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Same expression on both sides of '||'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if (a / 1000 / 1000) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int foo(int i) {\n"
              "    return i/i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Same expression on both sides of '/'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if (a << 1 << 1) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() { return !!y; }"); // No FP
        ASSERT_EQUALS("", errout_str());

        // make sure there are not "same expression" fp when there are different casts
        check("void f(long x) { if ((int32_t)x == (int64_t)x) {} }",
              dinit(CheckOptions, $.inconclusive = false)
              );
        ASSERT_EQUALS("", errout_str());

        // make sure there are not "same expression" fp when there are different ({}) expressions
        check("void f(long x) { if (({ 1+2; }) == ({3+4;})) {} }");
        ASSERT_EQUALS("", errout_str());

        // #5535: Reference named like its type
        check("void foo() { UMSConfig& UMSConfig = GetUMSConfiguration(); }");
        ASSERT_EQUALS("[test.cpp:1:25]: (style) Variable 'UMSConfig' can be declared as reference to const [constVariableReference]\n", errout_str());

        // #3868 - false positive (same expression on both sides of |)
        check("void f(int x) {\n"
              "    a = x ? A | B | C\n"
              "          : A | B;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const Bar &bar) {\n"
              "    bool a = bar.isSet() && bar->isSet();\n"
              "    bool b = bar.isSet() && bar.isSet();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:26]: (style) Same expression on both sides of '&&'. [duplicateExpression]\n", errout_str());


        check("void foo(int a, int b) {\n"
              "    if ((b + a) | (a + b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Same expression on both sides of '|' because 'b+a' and 'a+b' represent the same value. [duplicateExpression]\n", errout_str());

        check("void foo(const std::string& a, const std::string& b) {\n"
              "  return a.find(b+\"&\") || a.find(\"&\"+b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a, int b) {\n"
              "    if ((b > a) | (a > b)) {}\n" // > is not commutative
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(double a, double b) {\n"
              "    if ((b + a) > (a + b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) The comparison 'b+a > a+b' is always false because 'b+a' and 'a+b' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x == 1) && (x == 0x00000001))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (style) Same expression on both sides of '&&' because 'x==1' and 'x==0x00000001' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "    enum { Four = 4 };\n"
              "    if (Four == 4) {}"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (style) The comparison 'Four == 4' is always true. [knownConditionTrueFalse]\n",
                      errout_str());

        check("void f() {\n"
              "    enum { Four = 4 };\n"
              "    static_assert(Four == 4, \"\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    enum { Four = 4 };\n"
              "    _Static_assert(Four == 4, \"\");\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    enum { Four = 4 };\n"
              "    static_assert(4 == Four, \"\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    enum { FourInEnumOne = 4 };\n"
              "    enum { FourInEnumTwo = 4 };\n"
              "    if (FourInEnumOne == FourInEnumTwo) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:23]: (style) The comparison 'FourInEnumOne == FourInEnumTwo' is always true because 'FourInEnumOne' and 'FourInEnumTwo' represent the same value. [knownConditionTrueFalse]\n",
                      errout_str());

        check("void f() {\n"
              "    enum { FourInEnumOne = 4 };\n"
              "    enum { FourInEnumTwo = 4 };\n"
              "    static_assert(FourInEnumOne == FourInEnumTwo, \"\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a, int b) {\n"
              "    if (sizeof(a) == sizeof(a)) { }\n"
              "    if (sizeof(a) == sizeof(b)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:19]: (style) Same expression on both sides of '=='. [duplicateExpression]\n", errout_str());

        check("float bar(int) __attribute__((pure));\n"
              "char foo(int) __attribute__((pure));\n"
              "int test(int a, int b) {\n"
              "    if (bar(a) == bar(a)) { }\n"
              "    if (unknown(a) == unknown(a)) { }\n"
              "    if (foo(a) == foo(a)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:16]: (style) Same expression on both sides of '=='. [duplicateExpression]\n", errout_str());
    }

    void duplicateExpression2() { // check if float is NaN or Inf
        check("int f(long double ldbl, double dbl, float flt) {\n" // ticket #2730
              "    if (ldbl != ldbl) have_nan = 1;\n"
              "    if (!(dbl == dbl)) have_nan = 1;\n"
              "    if (flt != flt) have_nan = 1;\n"
              "    return have_nan;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("float f(float x) { return x-x; }"); // ticket #4485 (Inf)
        ASSERT_EQUALS("", errout_str());

        check("float f(float x) { return (X double)x == (X double)x; }", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("struct X { float f; };\n"
              "float f(struct X x) { return x.f == x.f; }");
        ASSERT_EQUALS("", errout_str());

        check("struct X { int i; };\n"
              "int f(struct X x) { return x.i == x.i; }");
        ASSERT_EQUALS("[test.cpp:2:32]: (style) Same expression on both sides of '=='. [duplicateExpression]\n", errout_str());

        // #5284 - when type is unknown, assume it's float
        check("int f() { return x==x; }");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression3() {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"mystrcmp\">\n"
                                   "    <pure/>\n"
                                   "    <arg nr=\"1\"/>\n"
                                   "    <arg nr=\"2\"/>\n"
                                   "  </function>\n"
                                   "</def>";
        /*const*/ Settings settings = settingsBuilder().libraryxml(xmldata).severity(Severity::style).build();

        check("void foo() {\n"
              "    if (x() || x()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "  void foo() const;\n"
              "  bool bar() const;\n"
              "};\n"
              "void A::foo() const {\n"
              "    if (bar() && bar()) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:15]: (style) Same expression on both sides of '&&'. [duplicateExpression]\n", errout_str());

        check("struct A {\n"
              "  void foo();\n"
              "  bool bar();\n"
              "  bool bar() const;\n"
              "};\n"
              "void A::foo() {\n"
              "    if (bar() && bar()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class B {\n"
              "    void bar(int i);\n"
              "};\n"
              "class A {\n"
              "    void bar(int i) const;\n"
              "};\n"
              "void foo() {\n"
              "    B b;\n"
              "    A a;\n"
              "    if (b.bar(1) && b.bar(1)) {}\n"
              "    if (a.bar(1) && a.bar(1)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11:18]: (style) Same expression on both sides of '&&'. [duplicateExpression]\n", errout_str());

        check("class D { void strcmp(); };\n"
              "void foo() {\n"
              "    D d;\n"
              "    if (d.strcmp() && d.strcmp()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    if ((mystrcmp(a, b) == 0) || (mystrcmp(a, b) == 0)) {}\n"
              "}", dinit(CheckOptions, $.inconclusive = false, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:31]: (style) Same expression on both sides of '||'. [duplicateExpression]\n", errout_str());

        check("void GetValue() { return rand(); }\n"
              "void foo() {\n"
              "    if ((GetValue() == 0) || (GetValue() == 0)) { dostuff(); }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void __attribute__((const)) GetValue() { return X; }\n"
              "void foo() {\n"
              "    if ((GetValue() == 0) || (GetValue() == 0)) { dostuff(); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:27]: (style) Same expression on both sides of '||'. [duplicateExpression]\n", errout_str());

        check("void GetValue() __attribute__((const));\n"
              "void GetValue() { return X; }\n"
              "void foo() {\n"
              "    if ((GetValue() == 0) || (GetValue() == 0)) { dostuff(); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:27]: (style) Same expression on both sides of '||'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if (str == \"(\" || str == \"(\") {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (style) Same expression on both sides of '||'. [duplicateExpression]\n", errout_str());

        check("void foo() {\n"
              "    if (bar(a) && !strcmp(a, b) && bar(a) && !strcmp(a, b)) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5334
        check("void f(C *src) {\n"
              "    if (x<A*>(src) || x<B*>(src))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(A *src) {\n"
              "    if (dynamic_cast<B*>(src) || dynamic_cast<B*>(src)) {}\n"
              "}\n", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:2:31]: (style) Same expression on both sides of '||'. [duplicateExpression]\n", errout_str());

        // #5819
        check("Vector func(Vector vec1) {\n"
              "    return fabs(vec1 & vec1 & vec1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("Vector func(int vec1) {\n"
              "    return fabs(vec1 & vec1 & vec1);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:22]: (style) Same expression on both sides of '&'. [duplicateExpression]\n"
            "[test.cpp:2:29]: (style) Same expression on both sides of '&'. [duplicateExpression]\n",
            errout_str());

    }

    void duplicateExpression4() {
        check("void foo() {\n"
              "    if (*a++ != b || *a++ != b) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    if (*a-- != b || *a-- != b) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // assignment
        check("void f() {\n"
              "  while (*(a+=2)==*(b+=2) && *(a+=2)==*(b+=2)) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression5() {  // #3749 - macros with same values
        check("void f() {\n"
              "    if ($a == $a) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        checkP("#define X 1\n"
               "#define Y 1\n"
               "void f() {\n"
               "    if (X == X) {}\n"
               "    if (X == Y) {}\n"
               "}\n");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Same expression on both sides of '=='. [duplicateExpression]\n", errout_str());

        checkP("#define X 1\n"
               "#define Y X\n"
               "void f() {\n"
               "    if (X == Y) {}\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression6() {  // #4639
        check("float IsNan(float value) { return !(value == value); }\n"
              "double IsNan(double value) { return !(value == value); }\n"
              "long double IsNan(long double value) { return !(value == value); }");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression7() {
        check("void f() {\n"
              "    const int i = sizeof(int);\n"
              "    if ( i != sizeof (int)){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25] -> [test.cpp:3:12]: (style) The comparison 'i != sizeof(int)' is always false because 'i' and 'sizeof(int)' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "    const int i = sizeof(int);\n"
              "    if ( sizeof (int) != i){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25] -> [test.cpp:3:23]: (style) The comparison 'sizeof(int) != i' is always false because 'sizeof(int)' and 'i' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f(int a = 1) { if ( a != 1){}}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a = 1;\n"
              "    if ( a != 1){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:3:12]: (style) The comparison 'a != 1' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "    int a = 1;\n"
              "    int b = 1;\n"
              "    if ( a != b){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:3:13] -> [test.cpp:4:12]: (style) The comparison 'a != b' is always false because 'a' and 'b' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "    int a = 1;\n"
              "    int b = a;\n"
              "    if ( a != b){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13] -> [test.cpp:4:12]: (style) The comparison 'a != b' is always false because 'a' and 'b' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void use(int);\n"
              "void f() {\n"
              "    int a = 1;\n"
              "    int b = 1;\n"
              "    use(b);\n"
              "    if ( a != 1){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13] -> [test.cpp:6:12]: (style) The comparison 'a != 1' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("void use(int);\n"
              "void f() {\n"
              "    int a = 1;\n"
              "    use(a);\n"
              "    a = 2;\n"
              "    if ( a != 1){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void use(int);\n"
              "void f() {\n"
              "    int a = 2;\n"
              "    use(a);\n"
              "    a = 1;\n"
              "    if ( a != 1){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("const int a = 1;\n"
              "void f() {\n"
              "    if ( a != 1){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:15] -> [test.cpp:3:12]: (style) The comparison 'a != 1' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("int a = 1;\n"
              "    void f() {\n"
              "    if ( a != 1){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    static const int a = 1;\n"
              "    if ( a != 1){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:26] -> [test.cpp:3:12]: (style) The comparison 'a != 1' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "    static int a = 1;\n"
              "    if ( a != 1){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a = 1;\n"
              "    if ( a != 1){\n"
              "        a++;\n"
              "    }}");
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:3:12]: (style) The comparison 'a != 1' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("void f(int b) {\n"
              "    int a = 1;\n"
              "    while (b) {\n"
              "        if ( a != 1){}\n"
              "        a++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(bool a, bool b) {\n"
              "    const bool c = a;\n"
              "    return a && b && c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20] -> [test.cpp:3:19]: (style) Same expression 'a' found multiple times in chain of '&&' operators because 'a' and 'c' represent the same value. [knownConditionTrueFalse]\n",
                      errout_str());

        // 6906
        check("void f(const bool b) {\n"
              "   const bool b1 = !b;\n"
              "   if(!b && b1){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20] -> [test.cpp:3:10]: (style) Same expression on both sides of '&&' because '!b' and 'b1' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        // 7284
        check("void f(void) {\n"
              "   if (a || !!a) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Same expression on both sides of '||' because 'a' and '!!a' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        // 8205
        check("void f(int x) {\n"
              "   int Diag = 0;\n"
              "   switch (x) {\n"
              "   case 12:\n"
              "       if (Diag==0) {}\n"
              "       break;\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15] -> [test.cpp:5:16]: (style) The comparison 'Diag == 0' is always true. [knownConditionTrueFalse]\n", errout_str());

        // #9744
        check("void f(const std::vector<int>& ints) {\n"
              "    int i = 0;\n"
              "    for (int p = 0; i < ints.size(); ++i) {\n"
              "        if (p == 0) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:18] -> [test.cpp:4:15]: (style) The comparison 'p == 0' is always true. [knownConditionTrueFalse]\n", errout_str());

        // #11820
        check("unsigned f(unsigned x) {\n"
              "    return x - !!x;\n"
              "}\n"
              "unsigned g(unsigned x) {\n"
              "    return !!x - x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression8() {
        check("void f() {\n"
              "    int a = 1;\n"
              "    int b = a;\n"
              "    a = 2;\n"
              "    if ( b != a){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int * a, int i) { int b = a[i]; a[i] = 2; if ( b != a[i]){}}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int * a, int i) { int b = *a; *a = 2; if ( b != *a){}}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int f() const; };\n"
              "A g();\n"
              "void foo() {\n"
              "    for (A x = A();;) {\n"
              "        const int a = x.f();\n"
              "        x = g();\n"
              "        if (x.f() == a) break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int i);\n"
              "struct A {\n"
              "    enum E { B, C };\n"
              "    bool f(E);\n"
              "};\n"
              "void foo() {\n"
              "    A a;\n"
              "    const bool x = a.f(A::B);\n"
              "    const bool y = a.f(A::C);\n"
              "    if(!x && !y) return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    const bool x = a.f(A::B);\n"
              "    const bool y = a.f(A::C);\n"
              "    if (!x && !y) return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool * const b);\n"
              "void foo() {\n"
              "    bool x = true;\n"
              "    bool y = true;\n"
              "    f(&x);\n"
              "    if (!x && !y) return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    const int a = {};\n"
              "    if(a == 1) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("volatile const int var = 42;\n"
              "void f() { if(var == 42) {} }");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    struct b c;\n"
              "    c.a = &a;\n"
              "    g(&c);\n"
              "    if (a == 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression9() {
        // #9320
        check("void f() {\n"
              "  uint16_t x = 1000;\n"
              "  uint8_t y = x;\n"
              "  if (x != y) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression10() {
        // #9485
        check("int f() {\n"
              "   const int a = 1;\n"
              "   const int b = a-1;\n"
              "   const int c = a+1;\n"
              "   return c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression11() {
        check("class Fred {\n"
              "public:\n"
              "    double getScale() const { return m_range * m_zoom; }\n"
              "    void setZoom(double z) { m_zoom = z; }\n"
              "    void dostuff(int x);\n"
              "private:\n"
              "    double m_zoom;\n"
              "    double m_range;\n"
              "};\n"
              "\n"
              "void Fred::dostuff(int x) {\n"
              "    if (x == 43) {\n"
              "        double old_scale = getScale();\n"
              "        setZoom(m_zoom + 1);\n"
              "        double scale_ratio = getScale() / old_scale;\n" // <- FP
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression12() { //#10026
        check("int f(const std::vector<int> &buffer, const uint8_t index)\n"
              "{\n"
              "        int var = buffer[index - 1];\n"
              "        return buffer[index - 1] - var;\n"  // <<
              "}");
        ASSERT_EQUALS("[test.cpp:3:25] -> [test.cpp:4:34]: (style) Same expression on both sides of '-' because 'buffer[index-1]' and 'var' represent the same value. [duplicateExpression]\n", errout_str());
    }

    void duplicateExpression13() { //#7899
        check("void f() {\n"
              "    if (sizeof(long) == sizeof(long long)) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression14() { //#9871
        check("int f() {\n"
              "    int k = 7;\n"
              "    int* f = &k;\n"
              "    int* g = &k;\n"
              "    return (f + 4 != g + 4);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:14] -> [test.cpp:4:14] -> [test.cpp:5:19]: (style) The comparison 'f+4 != g+4' is always false because 'f+4' and 'g+4' represent the same value. [knownConditionTrueFalse]\n", errout_str());
    }

    void duplicateExpression15() { //#10650
        check("bool f() {\n"
              "    const int i = int(0);\n"
              "    return i == 0;\n"
              "}\n"
              "bool g() {\n"
              "    const int i = int{ 0 };\n"
              "    return i == 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:22] -> [test.cpp:3:14]: (style) The comparison 'i == 0' is always true. [knownConditionTrueFalse]\n"
                      "[test.cpp:6:22] -> [test.cpp:7:14]: (style) The comparison 'i == 0' is always true. [knownConditionTrueFalse]\n",
                      errout_str());
    }

    void duplicateExpression16() {
        check("void f(const std::string& a) {\n" //#10569
              "    if ((a == \"x\") ||\n"
              "        (a == \"42\") ||\n"
              "        (a == \"y\") ||\n"
              "        (a == \"42\")) {}\n"
              "}\n"
              "void g(const std::string& a) {\n"
              "    if ((a == \"42\") ||\n"
              "        (a == \"x\") ||\n"
              "        (a == \"42\") ||\n"
              "        (a == \"y\")) {}\n"
              "}\n"
              "void h(const std::string& a) {\n"
              "    if ((a == \"42\") ||\n"
              "        (a == \"x\") ||\n"
              "        (a == \"y\") ||\n"
              "        (a == \"42\")) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:1:28] -> [test.cpp:4:20]: (style) Same expression 'a==\"42\"' found multiple times in chain of '||' operators. [duplicateExpression]\n"
                      "[test.cpp:7:28] -> [test.cpp:9:20]: (style) Same expression 'a==\"42\"' found multiple times in chain of '||' operators. [duplicateExpression]\n"
                      "[test.cpp:13:28] -> [test.cpp:16:20]: (style) Same expression 'a==\"42\"' found multiple times in chain of '||' operators. [duplicateExpression]\n",
                      errout_str());

        check("void f(const char* s) {\n" // #6371
              "    if (*s == '\x0F') {\n"
              "        if (!s[1] || !s[2] || !s[1])\n"
              "            break;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:28]: (style) Same expression '!s[1]' found multiple times in chain of '||' operators. [duplicateExpression]\n", errout_str());
    }

    void duplicateExpression17() {
        check("enum { E0 };\n" // #12036
              "void f() {\n"
              "    if (0 > E0) {}\n"
              "    if (E0 > 0) {}\n"
              "    if (E0 == 0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) The comparison '0 > E0' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:4:12]: (style) The comparison 'E0 > 0' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:5:12]: (style) The comparison 'E0 == 0' is always true. [knownConditionTrueFalse]\n",
                      errout_str());

        check("struct S {\n" // #12040, #12044
              "    static const int I = 0;\n"
              "    enum { E0 };\n"
              "    enum F { F0 };\n"
              "    void f() {\n"
              "        if (0 > I) {}\n"
              "        if (0 > S::I) {}\n"
              "        if (0 > E0) {}\n"
              "        if (0 > S::E0) {}\n"
              "    }\n"
              "};\n"
              "void g() {\n"
              "    if (0 > S::I) {}\n"
              "    if (0 > S::E0) {}\n"
              "    if (0 > S::F::F0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:26] -> [test.cpp:6:15]: (style) The comparison '0 > I' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:2:26] -> [test.cpp:7:15]: (style) The comparison '0 > S::I' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:8:15]: (style) The comparison '0 > E0' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:9:15]: (style) The comparison '0 > S::E0' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:2:26] -> [test.cpp:13:11]: (style) The comparison '0 > S::I' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:14:11]: (style) The comparison '0 > S::E0' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:15:11]: (style) The comparison '0 > S::F::F0' is always false. [knownConditionTrueFalse]\n",
                      errout_str());

        check("template<typename T, typename U>\n" // #12122
              "void f() {\n"
              "    static_assert(std::is_same<T, U>::value || std::is_integral<T>::value);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression18() {
        checkP("#if defined(ABC)\n" // #13218
               "#define MACRO1 (0x1)\n"
               "#else\n"
               "#define MACRO1 (0)\n"
               "#endif\n"
               "#if defined(XYZ)\n"
               "#define MACRO2 (0x2)\n"
               "#else\n"
               "#define MACRO2 (0)\n"
               "#endif\n"
               "#define MACRO_ALL (MACRO1 | MACRO2)\n"
               "void f() {\n"
               "    if (MACRO_ALL == 0) {}\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression19() {
        checkP("const int i = 0;\n"
               "void f() {\n"
               "    assert(i == 0);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression20() {
        check("enum { N = 1 };\n" // #14202
              "int f() { return N - 1; }");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpression21() {
        check("struct S { int i; };\n" // #12795
              "struct T {\n"
              "    std::map<std::string, S*> m;\n"
              "    S* get(const std::string& s) { return m[s]; }\n"
              "    void modify() { for (const auto& e : m) e.second->i = 0; }\n"
              "};\n"
              "void f(T& t) {\n"
              "    const S* p = t.get(\"abc\");\n"
              "    const int o = p->i;\n"
              "    t.modify();\n"
              "    if (p->i == o) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n"
              "    struct T {\n"
              "    std::vector<S*> m;\n"
              "    void modify() { for (auto e : m) e->i = 0; }\n"
              "};\n"
              "void f(T& t) {\n"
              "    const S* p = t.m[0];\n"
              "    const int o = p->i;\n"
              "    t.modify();\n"
              "    if (p->i == o) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpressionLoop() {
        check("void f() {\n"
              "    int a = 1;\n"
              "    while ( a != 1){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:3:15]: (style) The comparison 'a != 1' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("void f() { int a = 1; while ( a != 1){ a++; }}");
        ASSERT_EQUALS("", errout_str());

        check("void f() { int a = 1; for ( int i=0; i < 3 && a != 1; i++){ a++; }}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int b) { int a = 1; while (b) { if ( a != 1){} b++; } a++; }");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    for(int i = 0; i < 10;) {\n"
              "        if( i != 0 ) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17] -> [test.cpp:3:15]: (style) The comparison 'i != 0' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "    for(int i = 0; i < 10;) {\n"
              "        if( i != 0 ) {}\n"
              "        i++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    for(int i = 0; i < 10;) {\n"
              "        if( i != 0 ) { i++; }\n"
              "        i++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    for(int i = 0; i < 10;) {\n"
              "        if( i != 0 ) { i++; }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int i = 0;\n"
              "    while(i < 10) {\n"
              "        if( i != 0 ) {}\n"
              "        i++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int b) {\n"
              "    while (b) {\n"
              "        int a = 1;\n"
              "        if ( a != 1){}\n"
              "        b++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17] -> [test.cpp:4:16]: (style) The comparison 'a != 1' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("struct T {\n" // #11083
              "    std::string m;\n"
              "    const std::string & str() const { return m; }\n"
              "    T* next();\n"
              "};\n"
              "void f(T* t) {\n"
              "    const std::string& s = t->str();\n"
              "    while (t && t->str() == s)\n"
              "        t = t->next();\n"
              "    do {\n"
              "        t = t->next();\n"
              "    } while (t && t->str() == s);\n"
              "    for (; t && t->str() == s; t = t->next());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string &out, const std::vector<std::string> &list) {\n" // #13669
              "    for (int i = 0, size = list.size(); i < size; i++) {\n"
              "        out += list[i];\n"
              "        if (size > 0 && i < (size - 2))\n"
              "            out += \",\";\n"
              "        else if (i == (size - 1))\n"
              "            out += \".\";\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::vector<int>& v) {\n" // #14193
              "    for (const int& r1 : v) {\n"
              "        for (const int& r2 : v) {\n"
              "            if (&r1 == &r2) {}\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpressionTernary() { // #6391
        check("void f() {\n"
              "    return A ? x : x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (style) Same expression in both branches of ternary operator. [duplicateExpressionTernary]\n", errout_str());

        check("int f(bool b, int a) {\n"
              "    const int c = a;\n"
              "    return b ? a : c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:19] -> [test.cpp:3:18]: (style) Same expression in both branches of ternary operator. [duplicateExpressionTernary]\n", errout_str());

        check("void f() {\n"
              "    return A ? x : z;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(unsigned char c) {\n"
              "  x = y ? (signed char)c : (unsigned char)c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string stringMerge(std::string const& x, std::string const& y) {\n" // #7938
              "    return ((x > y) ? (y + x) : (x + y));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6426
        {
            const char code[] = "void foo(bool flag) {\n"
                                "  bar( (flag) ? ~0u : ~0ul);\n"
                                "}";
            /*const*/ Settings settings = settings1;
            settings.platform.sizeof_int = 4;
            settings.platform.int_bit = 32;

            settings.platform.sizeof_long = 4;
            settings.platform.long_bit = 32;
            check(code, dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("[test.cpp:2:21]: (style) Same value in both branches of ternary operator. [duplicateValueTernary]\n", errout_str());

            settings.platform.sizeof_long = 8;
            settings.platform.long_bit = 64;
            check(code, dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("", errout_str());
        }
    }

    void duplicateValueTernary() {
        check("void f() {\n"
              "    if( a ? (b ? false:false): false ) ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:23]: (style) Same expression in both branches of ternary operator. [duplicateExpressionTernary]\n", errout_str());

        check("int f1(int a) {return (a == 1) ? (int)1 : 1; }");
        ASSERT_EQUALS("[test.cpp:1:41]: (style) Same value in both branches of ternary operator. [duplicateValueTernary]\n", errout_str());

        check("int f2(int a) {return (a == 1) ? (int)1 : (int)1; }");
        ASSERT_EQUALS("[test.cpp:1:41]: (style) Same expression in both branches of ternary operator. [duplicateExpressionTernary]\n", errout_str());

        check("int f3(int a) {return (a == 1) ? 1 : (int)1; }");
        ASSERT_EQUALS("[test.cpp:1:36]: (style) Same value in both branches of ternary operator. [duplicateValueTernary]\n", errout_str());

        check("int f4(int a) {return (a == 1) ? 1 : 1; }");
        ASSERT_EQUALS("[test.cpp:1:36]: (style) Same expression in both branches of ternary operator. [duplicateExpressionTernary]\n", errout_str());

        check("int f5(int a) {return (a == (int)1) ? (int)1 : 1; }");
        ASSERT_EQUALS("[test.cpp:1:46]: (style) Same value in both branches of ternary operator. [duplicateValueTernary]\n", errout_str());

        check("int f6(int a) {return (a == (int)1) ? (int)1 : (int)1; }");
        ASSERT_EQUALS("[test.cpp:1:46]: (style) Same expression in both branches of ternary operator. [duplicateExpressionTernary]\n", errout_str());

        check("int f7(int a) {return (a == (int)1) ? 1 : (int)1; }");
        ASSERT_EQUALS("[test.cpp:1:41]: (style) Same value in both branches of ternary operator. [duplicateValueTernary]\n", errout_str());

        check("int f8(int a) {return (a == (int)1) ? 1 : 1; }");
        ASSERT_EQUALS("[test.cpp:1:41]: (style) Same expression in both branches of ternary operator. [duplicateExpressionTernary]\n", errout_str());

        check("struct Foo {\n"
              "  std::vector<int> bar{1,2,3};\n"
              "  std::vector<int> baz{4,5,6};\n"
              "};\n"
              "void f() {\n"
              "  Foo foo;\n"
              "  it = true ? foo.bar.begin() : foo.baz.begin();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n"
              "  std::vector<int> bar{1,2,3};\n"
              "  std::vector<int> baz{4,5,6};\n"
              "  std::vector<int> v = b ? bar : baz;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool q) {\n" // #9570
              "    static int a = 0;\n"
              "    static int b = 0;\n"
              "    int& x = q ? a : b;\n"
              "    ++x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int a, b; };\n" // #10107
              "S f(bool x, S s) {\n"
              "    (x) ? f.a = 42 : f.b = 42;\n"
              "    return f;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("float f(float x) {\n" // # 11368
              "    return (x >= 0.0) ? 0.0 : -0.0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {};\n" // # 14300
              "struct B {};\n"
              "void f(bool x) {\n"
              "    A* a = new A();\n"
              "    B* b = new B();\n"
              "    auto p = x ? static_cast<void*>(a) : static_cast<void*>(b);\n"
              "    (void)p;\n"
              "    delete a;\n"
              "    delete b;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateValueTernarySizeof() { // #13773
        check("int f() { return x ? sizeof(uint32_t) : sizeof(unsigned int); }");
        ASSERT_EQUALS("", errout_str());

        check("int f() { return x ? sizeof(uint32_t) : sizeof(uint32_t); }");
        ASSERT_EQUALS("[test.cpp:1:39]: (style) Same expression in both branches of ternary operator. [duplicateExpressionTernary]\n", errout_str());
    }

    void duplicateExpressionTemplate() {
        check("template <int I> void f() {\n" // #6930
              "    if (I >= 0 && I < 3) {}\n"
              "}\n"
              "\n"
              "static auto a = f<0>();");
        ASSERT_EQUALS("", errout_str());

        check("template<typename T>\n" // #7754
              "void f() {\n"
              "    if (std::is_same_v<T, char> || std::is_same_v<T, unsigned char>) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("typedef long long int64_t;"
              "template<typename T>\n"
              "void f() {\n"
              "    if (std::is_same_v<T, long> || std::is_same_v<T, int64_t>) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        checkP("#define int32_t int"
               "template<typename T>\n"
               "void f() {\n"
               "    if (std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {}\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkP("#define F(v) (v) != 0\n" // #12392
               "template<class T>\n"
               "void f() {\n"
               "    if (F(0)) {}\n"
               "}\n"
               "void g() {\n"
               "    f<int>();\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateExpressionCompareWithZero() {
        check("void f(const int* x, bool b) {\n"
              "    if ((x && b) || (x != 0 && b)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:18]: (style) Same expression on both sides of '||' because 'x&&b' and 'x!=0&&b' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f(const int* x, bool b) {\n"
              "    if ((x != 0 && b) || (x && b)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:23]: (style) Same expression on both sides of '||' because 'x!=0&&b' and 'x&&b' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f(const int* x, bool b) {\n"
              "    if ((x && b) || (b && x != 0)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:18]: (style) Same expression on both sides of '||' because 'x&&b' and 'b&&x!=0' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f(const int* x, bool b) {\n"
              "    if ((!x && b) || (x == 0 && b)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:19]: (style) Same expression on both sides of '||' because '!x&&b' and 'x==0&&b' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f(const int* x, bool b) {\n"
              "    if ((x == 0 && b) || (!x && b)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:23]: (style) Same expression on both sides of '||' because 'x==0&&b' and '!x&&b' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f(const int* x, bool b) {\n"
              "    if ((!x && b) || (b && x == 0)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:19]: (style) Same expression on both sides of '||' because '!x&&b' and 'b&&x==0' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("struct A {\n"
              "    int* getX() const;\n"
              "    bool getB() const;\n"
              "    void f() {\n"
              "        if ((getX() && getB()) || (getX() != 0 && getB())) {}\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:5:32]: (style) Same expression on both sides of '||' because 'getX()&&getB()' and 'getX()!=0&&getB()' represent the same value. [knownConditionTrueFalse]\n", errout_str());

        check("void f(const int* x, bool b) {\n"
              "    if ((x && b) || (x == 0 && b)) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const int* x, bool b) {\n"
              "    if ((!x && b) || (x != 0 && b)) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void oppositeExpression() {
        check("void f(bool a) { if(a && !a) {} }");
        ASSERT_EQUALS("[test.cpp:1:23]: (style) Opposite expression on both sides of '&&'. [oppositeExpression]\n", errout_str());

        check("void f(bool a) { if(a != !a) {} }");
        ASSERT_EQUALS("[test.cpp:1:23]: (style) Opposite expression on both sides of '!='. [oppositeExpression]\n", errout_str());

        check("void f(bool a) { if( a == !(a) ) {}}");
        ASSERT_EQUALS("[test.cpp:1:24]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n", errout_str());

        check("void f(bool a) { if( a != !(a) ) {}}");
        ASSERT_EQUALS("[test.cpp:1:24]: (style) Opposite expression on both sides of '!='. [oppositeExpression]\n", errout_str());

        check("void f(bool a) { if( !(a) == a ) {}}");
        ASSERT_EQUALS("[test.cpp:1:27]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n", errout_str());

        check("void f(bool a) { if( !(a) != a ) {}}");
        ASSERT_EQUALS("[test.cpp:1:27]: (style) Opposite expression on both sides of '!='. [oppositeExpression]\n", errout_str());

        check("void f(bool a) { if( !(!a) == !(a) ) {}}");
        ASSERT_EQUALS("[test.cpp:1:28]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n", errout_str());

        check("void f(bool a) { if( !(!a) != !(a) ) {}}");
        ASSERT_EQUALS("[test.cpp:1:28]: (style) Opposite expression on both sides of '!='. [oppositeExpression]\n", errout_str());

        check("void f1(bool a) {\n"
              "    const bool b = a;\n"
              "    if( a == !(b) ) {}\n"
              "    if( b == !(a) ) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20] -> [test.cpp:3:11]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n"
                      "[test.cpp:2:20] -> [test.cpp:4:11]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n", errout_str());

        check("void f2(const bool *a) {\n"
              "    const bool b = *a;\n"
              "    if( *a == !(b) ) {}\n"
              "    if( b == !(*a) ) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20] -> [test.cpp:3:12]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n"
                      "[test.cpp:2:20] -> [test.cpp:4:11]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n", errout_str());

        check("void f(bool a) { a = !a; }");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a) { if( a < -a ) {}}");
        ASSERT_EQUALS("[test.cpp:1:23]: (style) Opposite expression on both sides of '<'. [oppositeExpression]\n", errout_str());

        check("void f(int a) { a -= -a; }");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a) { a = a / (-a); }");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int i){ return !((i - 1) & i); }");
        ASSERT_EQUALS("", errout_str());

        check("bool f(unsigned i){ return (x > 0) && (x & (x-1)) == 0; }");
        ASSERT_EQUALS("", errout_str());

        check("void A::f(bool a, bool c)\n"
              "{\n"
              "    const bool b = a;\n"
              "    if(c) { a = false; }\n"
              "    if(b && !a) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool c) {\n"
              "    const bool b = a;\n"
              "    if(c) { a = false; }\n"
              "    if(b && !a) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    bool x = a;\n"
              "    dostuff();\n"
              "    if (x && a) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  const bool b = g();\n"
              "  if (!b && g()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const bool *a) {\n"
              "    const bool b = a[42];\n"
              "    if( b == !(a[42]) ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:21] -> [test.cpp:3:11]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n", errout_str());

        check("void f(const bool *a) {\n"
              "    const bool b = a[42];\n"
              "    if( a[42] == !(b) ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:21] -> [test.cpp:3:15]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n", errout_str());

        check("void f(const bool *a) {\n"
              "    const bool b = *a;\n"
              "    if( b == !(*a) ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:20] -> [test.cpp:3:11]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n", errout_str());

        check("void f(const bool *a) {\n"
              "    const bool b = *a;\n"
              "    if( *a == !(b) ) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:20] -> [test.cpp:3:12]: (style) Opposite expression on both sides of '=='. [oppositeExpression]\n", errout_str());

        check("void f(uint16_t u) {\n" // #9342
              "    if (u != (u & -u))\n"
              "        return false;\n"
              "    if (u != (-u & u))\n"
              "        return false;\n"
              "    return true;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateVarExpression() {
        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int i = f();\n"
              "    int j = f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9]: (style) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("struct Foo { int f() const; int g() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9]: (style) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("struct Foo { int f() const; int g() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    Foo f2 = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:9] -> [test.cpp:5:9]: (style) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int i = 1 + f();\n"
              "    int j = 1 + f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9]: (style) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int i = f() + 1;\n"
              "    int j = 1 + f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int x = f();\n"
              "    int i = x + 1;\n"
              "    int j = f() + 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() __attribute__((pure));\n"
              "int g() __attribute__((pure));\n"
              "void test() {\n"
              "    int i = f() + f();\n"
              "    int j = f() + f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9]: (style) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("int f(int) __attribute__((pure));\n"
              "int g(int) __attribute__((pure));\n"
              "void test() {\n"
              "    int i = f(0);\n"
              "    int j = f(0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9]: (style) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("int f(int) __attribute__((pure));\n"
              "int g(int) __attribute__((pure));\n"
              "void test() {\n"
              "    const int x = 0;\n"
              "    int i = f(0);\n"
              "    int j = f(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test(const int * p, const int * q) {\n"
              "    int i = *p;\n"
              "    int j = *p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:2:9]: (style) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("struct A { int x; int y; };"
              "void test(A a) {\n"
              "    int i = a.x;\n"
              "    int j = a.x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:2:9]: (style) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("void test() {\n"
              "    int i = 0;\n"
              "    int j = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test() {\n"
              "    int i = -1;\n"
              "    int j = -1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int);\n"
              "void test() {\n"
              "    int i = f(0);\n"
              "    int j = f(1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f();\n"
              "int g();\n"
              "void test() {\n"
              "    int i = f() || f();\n"
              "    int j = f() && f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Foo {};\n"
              "void test() {\n"
              "    Foo i = Foo();\n"
              "    Foo j = Foo();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Foo {};\n"
              "void test() {\n"
              "    Foo i = Foo{};\n"
              "    Foo j = Foo{};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Foo { int f() const; float g() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("struct Foo { int f(); int g(); };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test() {\n"
              "    int i = f();\n"
              "    int j = f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test(int x) {\n"
              "    int i = ++x;\n"
              "    int j = ++x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test(int x) {\n"
              "    int i = x++;\n"
              "    int j = x++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test(int x) {\n"
              "    int i = --x;\n"
              "    int j = --x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test(int x) {\n"
              "    int i = x--;\n"
              "    int j = x--;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test(int x) {\n"
              "    int i = x + 1;\n"
              "    int j = 1 + x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateVarExpressionUnique() {
        check("struct SW { int first; };\n"
              "void foo(SW* x) {\n"
              "    int start = x->first;\n"
              "    int end   = x->first;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:9]: (style, inconclusive) Same expression used in consecutive assignments of 'start' and 'end'. [duplicateAssignExpression]\n"
                      "[test.cpp:2:14]: (style) Parameter 'x' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct SW { int first; };\n"
              "void foo(SW* x, int i, int j) {\n"
              "    int start = x->first;\n"
              "    int end   = x->first;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:9]: (style, inconclusive) Same expression used in consecutive assignments of 'start' and 'end'. [duplicateAssignExpression]\n"
                      "[test.cpp:2:14]: (style) Parameter 'x' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct Foo { int f() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("void test(const int * p) {\n"
              "    int i = *p;\n"
              "    int j = *p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:2:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("struct Foo { int f() const; int g(int) const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("struct Foo { int f() const; };\n"
              "void test() {\n"
              "    Foo f = Foo{};\n"
              "    int i = f.f();\n"
              "    int j = f.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());
    }

    void duplicateVarExpressionAssign() {
        check("struct A { int x; int y; };"
              "void use(int);\n"
              "void test(A a) {\n"
              "    int i = a.x;\n"
              "    int j = a.x;\n"
              "    use(i);\n"
              "    i = j;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("struct A { int x; int y; };"
              "void use(int);\n"
              "void test(A a) {\n"
              "    int i = a.x;\n"
              "    int j = a.x;\n"
              "    use(j);\n"
              "    j = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n", errout_str());

        check("struct A { int x; int y; };"
              "void use(int);\n"
              "void test(A a) {\n"
              "    int i = a.x;\n"
              "    int j = a.x;\n"
              "    use(j);\n"
              "    if (i == j) {}\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:9] -> [test.cpp:3:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n",
            errout_str());

        check("struct A { int x; int y; };"
              "void use(int);\n"
              "void test(A a) {\n"
              "    int i = a.x;\n"
              "    int j = a.x;\n"
              "    use(j);\n"
              "    if (i == a.x) {}\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:9] -> [test.cpp:3:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n",
            errout_str());

        check("struct A { int x; int y; };"
              "void use(int);\n"
              "void test(A a) {\n"
              "    int i = a.x;\n"
              "    int j = a.x;\n"
              "    use(i);\n"
              "    if (j == a.x) {}\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:9] -> [test.cpp:3:9]: (style, inconclusive) Same expression used in consecutive assignments of 'i' and 'j'. [duplicateAssignExpression]\n",
            errout_str());

        // Issue #8612
        check("struct P\n"
              "{\n"
              "    void func();\n"
              "    bool operator==(const P&) const;\n"
              "};\n"
              "struct X\n"
              "{\n"
              "    P first;\n"
              "    P second;\n"
              "};\n"
              "bool bar();\n"
              "void baz(const P&);\n"
              "void foo(const X& x)\n"
              "{\n"
              "    P current = x.first;\n"
              "    P previous = x.first;\n"
              "    while (true)\n"
              "    {\n"
              "        baz(current);\n"
              "        if (bar() && previous == current)\n"
              "        {\n"
              "            current.func();\n"
              "        }\n"
              "        previous = current;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:16:7] -> [test.cpp:15:7]: (style, inconclusive) Same expression used in consecutive assignments of 'current' and 'previous'. [duplicateAssignExpression]\n", errout_str());
    }

    void duplicateVarExpressionCrash() {
        // Issue #8624
        check("struct  X {\n"
              "    X();\n"
              "    int f() const;\n"
              "};\n"
              "void run() {\n"
              "        X x;\n"
              "        int a = x.f();\n"
              "        int b = x.f();\n"
              "        (void)a;\n"
              "        (void)b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:13] -> [test.cpp:7:13]: (style, inconclusive) Same expression used in consecutive assignments of 'a' and 'b'. [duplicateAssignExpression]\n", errout_str());

        // Issue #8712
        check("void f() {\n"
              "  unsigned char d;\n"
              "  d = d % 5;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("template <typename T>\n"
              "T f() {\n"
              "  T x = T();\n"
              "}\n"
              "int &a = f<int&>();");
        ASSERT_EQUALS("", errout_str());

        // Issue #8713
        check("class A {\n"
              "  int64_t B = 32768;\n"
              "  P<uint8_t> m = MakeP<uint8_t>(B);\n"
              "};\n"
              "void f() {\n"
              "  uint32_t a = 42;\n"
              "  uint32_t b = uint32_t(A ::B / 1024);\n"
              "  int32_t c = int32_t(a / b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Issue #8709
        check("a b;\n"
              "void c() {\n"
              "  switch (d) { case b:; }\n"
              "  double e(b);\n"
              "  if(e <= 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #10718
        // Should probably not be inconclusive
        check("struct a {\n"
              "  int b() const;\n"
              "  auto c() -> decltype(0) {\n"
              "    a d;\n"
              "    int e = d.b(), f = d.b();\n"
              "    return e + f;\n"
              "  }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:5:20] -> [test.cpp:5:9]: (style, inconclusive) Same expression used in consecutive assignments of 'e' and 'f'. [duplicateAssignExpression]\n", errout_str());
    }

    void multiConditionSameExpression() {
        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) continue;\n"
              "  if ((val > 0)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:3:11]: (style) The comparison 'val < 0' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:2:13] -> [test.cpp:4:12]: (style) The comparison 'val > 0' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  int *p = &val;n"
              "  val = 1;\n"
              "  if (*p < 0) continue;\n"
              "  if ((*p > 0)) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:8]: (style) Variable 'p' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  int *p = &val;\n"
              "  if (*p < 0) continue;\n"
              "  if ((*p > 0)) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (style) The comparison '*p < 0' is always false.\n"
                           "[test.cpp:2] -> [test.cpp:4]: (style) The comparison '*p > 0' is always false.\n",
                           "[test.cpp:3:8]: (style) Variable 'p' can be declared as pointer to const [constVariablePointer]\n",
                           errout_str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) {\n"
              "    if ((val > 0)) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:3:11]: (style) The comparison 'val < 0' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:2:13] -> [test.cpp:4:14]: (style) The comparison 'val > 0' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) {\n"
              "    if ((val < 0)) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:3:11]: (style) The comparison 'val < 0' is always false. [knownConditionTrueFalse]\n"
                      "[test.cpp:2:13] -> [test.cpp:4:14]: (style) The comparison 'val < 0' is always false. [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "  int activate = 0;\n"
              "  int foo = 0;\n"
              "  if (activate) {}\n"
              "  else if (foo) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkSignOfUnsignedVariable() {
        check("void foo() {\n"
              "  for(unsigned char i = 10; i >= 0; i--) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:31]: (style) Unsigned expression 'i' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        check("void foo(bool b) {\n"
              "  for(unsigned int i = 10; b || i >= 0; i--) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:35]: (style) Unsigned expression 'i' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        {
            const char code[] = "void foo(unsigned int x) {\n"
                                "  if (x < 0) {}\n"
                                "}";
            check(code, dinit(CheckOptions, $.inconclusive = false));
            ASSERT_EQUALS("[test.cpp:2:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());
            check(code, dinit(CheckOptions, $.inconclusive = false, $.verbose = true));
            ASSERT_EQUALS("[test.cpp:2:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());
        }

        check("void foo(unsigned int x) {\n"
              "  if (x < 0u) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());

        check("void foo(int x) {\n"
              "  if (x < 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        {
            const char code[] = "void foo(unsigned x) {\n"
                                "  int y = 0;\n"
                                "  if (x < y) {}\n"
                                "}";
            check(code, dinit(CheckOptions, $.inconclusive = false));
            ASSERT_EQUALS("[test.cpp:3:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());
            check(code, dinit(CheckOptions, $.inconclusive = false, $.verbose = true));
            ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());
        }
        check("void foo(unsigned x) {\n"
              "  int y = 0;\n"
              "  if (b)\n"
              "    y = 1;\n"
              "  if (x < y) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x) {\n"
              "  if (0 > x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());

        check("void foo(unsigned int x) {\n"
              "  if (0UL > x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());

        check("void foo(int x) {\n"
              "  if (0 > x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x) {\n"
              "  if (x >= 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Unsigned expression 'x' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        check("void foo(unsigned int x, unsigned y) {\n"
              "  if (x - y >= 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Unsigned expression 'x-y' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        check("void foo(unsigned int x) {\n"
              "  if (x >= 0ull) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Unsigned expression 'x' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        check("void foo(int x) {\n"
              "  if (x >= 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x) {\n"
              "  if (0 <= x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Unsigned expression 'x' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        check("void foo(unsigned int x) {\n"
              "  if (0ll <= x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Unsigned expression 'x' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        check("void foo(int x) {\n"
              "  if (0 <= x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x, bool y) {\n"
              "  if (x < 0 && y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());

        check("void foo(int x, bool y) {\n"
              "  if (x < 0 && y) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x, bool y) {\n"
              "  if (0 > x && y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());

        check("void foo(int x, bool y) {\n"
              "  if (0 > x && y) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x, bool y) {\n"
              "  if (x >= 0 && y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Unsigned expression 'x' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        check("void foo(int x, bool y) {\n"
              "  if (x >= 0 && y) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        check("void foo(unsigned int x, bool y) {\n"
              "  if (y && x < 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());

        check("void foo(int x, bool y) {\n"
              "  if (y && x < 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x, bool y) {\n"
              "  if (y && 0 > x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());

        check("void foo(int x, bool y) {\n"
              "  if (y && 0 > x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x, bool y) {\n"
              "  if (y && x >= 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (style) Unsigned expression 'x' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        check("void foo(int x, bool y) {\n"
              "  if (y && x >= 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        check("void foo(unsigned int x, bool y) {\n"
              "  if (x < 0 || y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());

        check("void foo(int x, bool y) {\n"
              "  if (x < 0 || y) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x, bool y) {\n"
              "  if (0 > x || y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());

        check("void foo(int x, bool y) {\n"
              "  if (0 > x || y) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(unsigned int x, bool y) {\n"
              "  if (x >= 0 || y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Unsigned expression 'x' can't be negative so it is unnecessary to test it. [unsignedPositive]\n", errout_str());

        check("void foo(int x, bool y) {\n"
              "  if (x >= 0 || y) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3233 - FP when template is used (template parameter is numeric constant)
        {
            const char code[] = "template<int n> void foo(unsigned int x) {\n"
                                "  if (x <= n);\n"
                                "}\n"
                                "foo<0>();";
            check(code, dinit(CheckOptions, $.inconclusive = false));
            ASSERT_EQUALS("", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            check("template<int n> void foo(unsigned int x) {\n"
                  "if (x <= 0);\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:2:7]: (style) Checking if unsigned expression 'x' is less than zero. [unsignedLessThanZero]\n", errout_str());
        }

        // #8836
        check("uint32_t value = 0xFUL;\n"
              "void f() {\n"
              "  if (value < 0u)\n"
              "  {\n"
              "    value = 0u;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Checking if unsigned expression 'value' is less than zero. [unsignedLessThanZero]\n", errout_str());

        // #9040
        /*const*/ Settings settingsWin64 = settingsBuilder().platform(Platform::Type::Win64).build();
        check("using BOOL = unsigned;\n"
              "int i;\n"
              "bool f() {\n"
              "    return i >= 0;\n"
              "}\n", dinit(CheckOptions, $.settings = &settingsWin64));
        ASSERT_EQUALS("", errout_str());

        // #10612
        check("void f(void) {\n"
              "   const uint32_t x = 0;\n"
              "   constexpr const auto y = 0xFFFFU;\n"
              "   if (y < x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Checking if unsigned expression 'y' is less than zero. [unsignedLessThanZero]\n", errout_str());

        // #12387
        check("template<typename T>\n"
              "void f(T t) {\n"
              "    if constexpr (std::numeric_limits<T>::is_signed) {\n"
              "        if (t < 0) {}\n"
              "    }\n"
              "}\n"
              "void g() {\n"
              "    f<uint32_t>(0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #13734
        check("void f() {\n"
              "    uint8_t a[N + 1];\n"
              "    for (unsigned p = 0; p < (sizeof(a) / sizeof((a)[0])); ++p) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const unsigned char u) {\n"
              "    if (u >  0) {}\n"
              "    if (u <  0) {}\n"
              "    if (u >= 0) {}\n"
              "    if (u <= 0) {}\n"
              "    if (0 <  u) {}\n"
              "    if (0 >  u) {}\n"
              "    if (0 <= u) {}\n"
              "    if (0 >= u) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Checking if unsigned expression 'u' is less than zero. [unsignedLessThanZero]\n"
                      "[test.cpp:4:11]: (style) Unsigned expression 'u' can't be negative so it is unnecessary to test it. [unsignedPositive]\n"
                      "[test.cpp:5:11]: (style) Checking if unsigned expression 'u' is less than zero. [unsignedLessThanZero]\n"
                      "[test.cpp:7:11]: (style) Checking if unsigned expression 'u' is less than zero. [unsignedLessThanZero]\n"
                      "[test.cpp:8:11]: (style) Unsigned expression 'u' can't be negative so it is unnecessary to test it. [unsignedPositive]\n"
                      "[test.cpp:9:11]: (style) Checking if unsigned expression 'u' is less than zero. [unsignedLessThanZero]\n",
                      errout_str());
    }

    void checkSignOfPointer() {
        check("void foo(const int* x) {\n"
              "  if (x >= 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not. [pointerPositive]\n", errout_str());

        {
            const char code[] = "void foo(const int* x) {\n"
                                "  int y = 0;\n"
                                "  if (x >= y) {}\n"
                                "}";
            check(code, dinit(CheckOptions, $.inconclusive = false));
            ASSERT_EQUALS("[test.cpp:3:9]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not. [pointerPositive]\n", errout_str());
            check(code, dinit(CheckOptions, $.inconclusive = false, $.verbose = true));
            ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:9]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not. [pointerPositive]\n", errout_str());
        }
        check("void foo(const int* x) {\n"
              "  if (*x >= 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const int* x) {\n"
              "  if (x < 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) A pointer can not be negative so it is either pointless or an error to check if it is. [pointerLessThanZero]\n", errout_str());

        {
            const char code[] = "void foo(const int* x) {\n"
                                "  unsigned y = 0u;\n"
                                "  if (x < y) {}\n"
                                "}";

            check(code, dinit(CheckOptions, $.inconclusive = false));
            ASSERT_EQUALS("[test.cpp:3:9]: (style) A pointer can not be negative so it is either pointless or an error to check if it is. [pointerLessThanZero]\n", errout_str());
            check(code, dinit(CheckOptions, $.inconclusive = false, $.verbose = true));
            ASSERT_EQUALS("[test.cpp:2:16] -> [test.cpp:3:9]: (style) A pointer can not be negative so it is either pointless or an error to check if it is. [pointerLessThanZero]\n", errout_str());
        }

        check("void foo(const int* x) {\n"
              "  if (*x < 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const int* x, const int* y) {\n"
              "  if (x - y < 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const int* x, const int* y) {\n"
              "  if (x - y <= 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const int* x, const int* y) {\n"
              "  if (x - y > 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const int* x, const int* y) {\n"
              "  if (x - y >= 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const Bar* x) {\n"
              "  if (0 <= x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not. [pointerPositive]\n", errout_str());

        check("struct S {\n"
              "  int* ptr;\n"
              "};\n"
              "void foo(S* first) {\n"
              "  if (first.ptr >= 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:17]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not. [pointerPositive]\n"
                      "[test.cpp:4:13]: (style) Parameter 'first' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n"
              "  int* ptr;\n"
              "};\n"
              "void foo(S* first, S* second) {\n"
              "  if((first.ptr - second.ptr) >= 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Parameter 'first' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:4:23]: (style) Parameter 'second' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n"
              "  int* ptr;\n"
              "};\n"
              "void foo(S* first) {\n"
              "  if((first.ptr) >= 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:18]: (style) A pointer can not be negative so it is either pointless or an error to check if it is not. [pointerPositive]\n"
                      "[test.cpp:4:13]: (style) Parameter 'first' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n"
              "  int* ptr;\n"
              "};\n"
              "void foo(S* first, S* second) {\n"
              "  if(0 <= first.ptr - second.ptr) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Parameter 'first' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:4:23]: (style) Parameter 'second' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n"
              "  int* ptr;\n"
              "};\n"
              "void foo(S* first, S* second) {\n"
              "  if(0 <= (first.ptr - second.ptr)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Parameter 'first' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:4:23]: (style) Parameter 'second' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n"
              "  int* ptr;\n"
              "};\n"
              "void foo(S* first, S* second) {\n"
              "  if(first.ptr - second.ptr < 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Parameter 'first' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:4:23]: (style) Parameter 'second' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n"
              "  int* ptr;\n"
              "};\n"
              "void foo(S* first, S* second) {\n"
              "  if((first.ptr - second.ptr) < 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Parameter 'first' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:4:23]: (style) Parameter 'second' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n"
              "  int* ptr;\n"
              "};\n"
              "void foo(S* first, S* second) {\n"
              "  if(0 > first.ptr - second.ptr) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Parameter 'first' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:4:23]: (style) Parameter 'second' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct S {\n"
              "  int* ptr;\n"
              "};\n"
              "void foo(S* first, S* second) {\n"
              "  if(0 > (first.ptr - second.ptr)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Parameter 'first' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:4:23]: (style) Parameter 'second' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void foo(const int* x) {\n"
              "  if (0 <= x[0]) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(Bar* x) {\n"
              "  if (0 <= x.y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'x' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(Bar* x) {\n"
              "  if (0 <= x->y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'x' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(Bar* x, Bar* y) {\n"
              "  if (0 <= x->y - y->y ) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'x' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:1:23]: (style) Parameter 'y' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("void foo(const Bar* x) {\n"
              "  if (0 > x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) A pointer can not be negative so it is either pointless or an error to check if it is. [pointerLessThanZero]\n", errout_str());

        check("void foo(const int* x) {\n"
              "  if (0 > x[0]) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(Bar* x) {\n"
              "  if (0 > x.y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'x' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo(Bar* x) {\n"
              "  if (0 > x->y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:15]: (style) Parameter 'x' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("void foo() {\n"
              "  int (*t)(void *a, void *b);\n"
              "  if (t(a, b) < 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "  int (*t)(void *a, void *b);\n"
              "  if (0 > t(a, b)) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct object_info { int *typep; };\n"
              "void packed_object_info(struct object_info *oi) {\n"
              "  if (oi->typep < 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) A pointer can not be negative so it is either pointless or an error to check if it is. [pointerLessThanZero]\n"
                      "[test.cpp:2:45]: (style) Parameter 'oi' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct object_info { int typep[10]; };\n"
              "void packed_object_info(struct object_info *oi) {\n"
              "  if (oi->typep < 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) A pointer can not be negative so it is either pointless or an error to check if it is. [pointerLessThanZero]\n"
                      "[test.cpp:2:45]: (style) Parameter 'oi' can be declared as pointer to const [constParameterPointer]\n",
                      errout_str());

        check("struct object_info { int *typep; };\n"
              "void packed_object_info(struct object_info *oi) {\n"
              "  if (*oi->typep < 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:45]: (style) Parameter 'oi' can be declared as pointer to const [constParameterPointer]\n", errout_str());
    }

    void checkSuspiciousSemicolon1() {
        check("void foo() {\n"
              "  for(int i = 0; i < 10; ++i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Empty block
        check("void foo() {\n"
              "  for(int i = 0; i < 10; ++i); {\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:3]: (warning) Suspicious use of ; at the end of 'for' statement. [suspiciousSemicolon]\n", errout_str());

        check("void foo() {\n"
              "  while (!quit); {\n"
              "    do_something();\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:3]: (warning) Suspicious use of ; at the end of 'while' statement. [suspiciousSemicolon]\n", errout_str());
    }

    void checkSuspiciousSemicolon2() {
        check("void foo() {\n"
              "  if (i == 1); {\n"
              "    do_something();\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:3]: (warning) Suspicious use of ; at the end of 'if' statement. [suspiciousSemicolon]\n", errout_str());

        // Seen this in the wild
        check("void foo() {\n"
              "  if (Match());\n"
              "  do_something();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "  if (Match());\n"
              "  else\n"
              "    do_something();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "  if (i == 1)\n"
              "       ;\n"
              "  {\n"
              "    do_something();\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "  if (i == 1);\n"
              "\n"
              "  {\n"
              "    do_something();\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkSuspiciousSemicolon3() {
        checkP("#define REQUIRE(code) {code}\n"
               "void foo() {\n"
               "  if (x == 123);\n"
               "  REQUIRE(y=z);\n"
               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkSuspiciousComparison() {
        checkP("void f(int a, int b) {\n"
               "  a > b;\n"
               "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '>', result is not used. [constStatement]\n", errout_str());

        checkP("void f() {\n" // #10607
               "  for (auto p : m)\n"
               "    std::vector<std::pair<std::string, std::string>> k;\n"
               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkInvalidFree() {
        check("void foo(char *p) {\n"
              "  char *a; a = malloc(1024);\n"
              "  free(a + 10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3]: (error) Mismatching address is freed. The address you get from malloc() must be freed without offset. [invalidFree]\n", errout_str());

        check("void foo(char *p) {\n"
              "  char *a; a = malloc(1024);\n"
              "  free(a - 10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3]: (error) Mismatching address is freed. The address you get from malloc() must be freed without offset. [invalidFree]\n", errout_str());

        check("void foo(char *p) {\n"
              "  char *a; a = malloc(1024);\n"
              "  free(10 + a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3]: (error) Mismatching address is freed. The address you get from malloc() must be freed without offset. [invalidFree]\n", errout_str());

        check("void foo(char *p) {\n"
              "  char *a; a = new char[1024];\n"
              "  delete[] (a + 10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:16]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:3:3]: (error) Mismatching address is deleted. The address you get from new must be deleted without offset. [invalidFree]\n",
                      errout_str());

        check("void foo(char *p) {\n"
              "  char *a; a = new char;\n"
              "  delete a + 10;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:16]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:3:3]: (error) Mismatching address is deleted. The address you get from new must be deleted without offset. [invalidFree]\n",
                      errout_str());

        check("void foo(char *p) {\n"
              "  char *a; a = new char;\n"
              "  bar(a);\n"
              "  delete a + 10;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char *p) {\n"
              "  char *a; a = new char;\n"
              "  char *b; b = new char;\n"
              "  bar(a);\n"
              "  delete a + 10;\n"
              "  delete b + 10;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:3]: (error) Mismatching address is deleted. The address you get from new must be deleted without offset. [invalidFree]\n", errout_str());

        check("void foo(char *p) {\n"
              "  char *a; a = new char;\n"
              "  char *b; b = new char;\n"
              "  bar(a, b);\n"
              "  delete a + 10;\n"
              "  delete b + 10;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char *p) {\n"
              "  char *a; a = new char;\n"
              "  bar()\n"
              "  delete a + 10;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:16]: (style) Parameter 'p' can be declared as pointer to const [constParameterPointer]\n"
                      "[test.cpp:4:3]: (error) Mismatching address is deleted. The address you get from new must be deleted without offset. [invalidFree]\n",
                      errout_str());

        check("void foo(size_t xx) {\n"
              "  char *ptr; ptr = malloc(42);\n"
              "  ptr += xx;\n"
              "  free(ptr + 1 - xx);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:3]: (error) Mismatching address is freed. The address you get from malloc() must be freed without offset. [invalidFree]\n", errout_str());

        check("void foo(size_t xx) {\n"
              "  char *ptr; ptr = malloc(42);\n"
              "  std::cout << ptr;\n"
              "  ptr = otherPtr;\n"
              "  free(otherPtr - xx - 1);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:9]: (style) Variable 'ptr' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());
    }

    void checkRedundantCopy() {
        check("const std::string& getA(){static std::string a;return a;}\n"
              "void foo() {\n"
              "    const std::string a = getA();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:23]: (performance, inconclusive) Use const reference for 'a' to avoid unnecessary data copying. [redundantCopyLocalConst]\n", errout_str());

        check("class A { public: A() {} char x[100]; };\n"
              "const A& getA(){static A a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const A a = getA();\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (performance, inconclusive) Use const reference for 'a' to avoid unnecessary data copying. [redundantCopyLocalConst]\n", errout_str());

        check("const int& getA(){static int a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const int a = getA();\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("const int& getA(){static int a;return a;}\n"
              "int main()\n"
              "{\n"
              "    int getA = 0;\n"
              "    const int a = getA + 3;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:12] -> [test.cpp:4:9]: (style) Local variable 'getA' shadows outer function [shadowFunction]\n", errout_str());

        check("class A { public: A() {} char x[100]; };\n"
              "const A& getA(){static A a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const A a(getA());\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (performance, inconclusive) Use const reference for 'a' to avoid unnecessary data copying. [redundantCopyLocalConst]\n", errout_str());

        check("const int& getA(){static int a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const int a(getA());\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class A{\n"
              "public:A(int a=0){_a = a;}\n"
              "A operator+(const A & a){return A(_a+a._a);}\n"
              "private:int _a;};\n"
              "const A& getA(){static A a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const A a = getA() + 1;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class A{\n"
              "public:A(int a=0){_a = a;}\n"
              "A operator+(const A & a){return A(_a+a._a);}\n"
              "private:int _a;};\n"
              "const A& getA(){static A a;return a;}\n"
              "int main()\n"
              "{\n"
              "    const A a(getA()+1);\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5190 - FP when creating object with constructor that takes a reference
        check("class A {};\n"
              "class B { B(const A &a); };\n"
              "const A &getA();\n"
              "void f() {\n"
              "    const B b(getA());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class A {};\n"
              "class B { B(const A& a); };\n"
              "const A& getA();\n"
              "void f() {\n"
              "    const B b{ getA() };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5618
        const char code5618[] = "class Token {\n"
                                "public:\n"
                                "    const std::string& str();\n"
                                "};\n"
                                "void simplifyArrayAccessSyntax() {\n"
                                "    for (Token *tok = list.front(); tok; tok = tok->next()) {\n"
                                "        const std::string temp = tok->str();\n"
                                "        tok->str(tok->strAt(2));\n"
                                "    }\n"
                                "}";
        check(code5618);
        ASSERT_EQUALS("", errout_str());
        check(code5618, dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // #5890 - crash: wesnoth desktop_util.cpp / unicode.hpp
        check("typedef std::vector<char> X;\n"
              "X f<X>(const X &in) {\n"
              "    const X s = f<X>(in);\n"
              "    return f<X>(s);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7981 - False positive redundantCopyLocalConst - const ref argument to ctor
        check("class CD {\n"
              "        public:\n"
              "        CD(const CD&);\n"
              "        static const CD& getOne();\n"
              "};\n"
              " \n"
              "void foo() {\n"
              "  const CD cd(CD::getOne());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #10545
              "    int modify();\n"
              "    const std::string& get() const;\n"
              "};\n"
              "std::string f(S& s) {\n"
              "    const std::string old = s.get();\n"
              "    int i = s.modify();\n"
              "    if (i != 0)\n"
              "        return old;\n"
              "    return {};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct X { int x; };\n" // #10191
              "struct S {\n"
              "    X _x;\n"
              "    X& get() { return _x; }\n"
              "    void modify() { _x.x += 42; }\n"
              "    int copy() {\n"
              "        const X x = get();\n"
              "        modify();\n"
              "        return x.x;\n"
              "    }\n"
              "    int constref() {\n"
              "        const X& x = get();\n"
              "        modify();\n"
              "        return x.x;\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #10704
        check("struct C {\n"
              "    std::string str;\n"
              "    const std::string& get() const { return str; }\n"
              "};\n"
              "struct D {\n"
              "    C c;\n"
              "    bool f() const {\n"
              "        std::string s = c.get();\n"
              "        return s.empty();\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:8:21]: (performance, inconclusive) Use const reference for 's' to avoid unnecessary data copying. [redundantCopyLocalConst]\n", errout_str());

        check("struct C {\n"
              "    const std::string & get() const { return m; }\n"
              "    std::string m;\n"
              "};\n"
              "C getC();\n"
              "void f() {\n"
              "    const std::string s = getC().get();\n"
              "}\n"
              "void g() {\n"
              "    std::string s = getC().get();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12139
              "    int x, y;\n"
              "};\n"
              "struct T {\n"
              "    S s;\n"
              "    const S& get() const { return s; }\n"
              "};\n"
              "void f(const T& t) {\n"
              "    const S a = t.get();\n"
              "    if (a.x > a.y) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12740
              "    const std::string & get() const { return m; }\n"
              "    void set(const std::string& v) { m = v; }\n"
              "    std::string m;\n"
              "};\n"
              "struct T {\n"
              "    void f();\n"
              "    S* s;\n"
              "};\n"
              "void T::f() {\n"
              "    const std::string o = s->get();\n"
              "    s->set(\"abc\");\n"
              "    s->set(o);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12196
              "    std::string s;\n"
              "    const std::string& get() const { return s; }\n"
              "};\n"
              "struct T {\n"
              "    S* m;\n"
              "    void f();\n"
              "};\n"
              "struct U {\n"
              "    explicit U(S* p);\n"
              "    void g();\n"
              "    S* n;\n"
              "};\n"
              "void T::f() {\n"
              "    U u(m);\n"
              "    const std::string c = m->get();\n"
              "    u.g();\n"
              "    if (c == m->get()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12925
              "    const std::string & f() const { return str; }\n"
              "    std::string str;\n"
              "};\n"
              "void f(const S* s) {\n"
              "    const std::string v{ s->f() };\n"
              "    if (v.empty()) {}\n"
              "}\n"
              "void g(const S* s) {\n"
              "    const std::string w(s->f());\n"
              "    if (w.empty()) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:23]: (performance, inconclusive) Use const reference for 'v' to avoid unnecessary data copying. [redundantCopyLocalConst]\n"
                      "[test.cpp:10:23]: (performance, inconclusive) Use const reference for 'w' to avoid unnecessary data copying. [redundantCopyLocalConst]\n",
                      errout_str());

        check("struct T {\n"
              "    std::string s;\n"
              "    const std::string& get() const { return s; }\n"
              "};\n"
              "void f(const T& t) {\n"
              "    const auto s = t.get();\n"
              "    if (s.empty()) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:16]: (performance, inconclusive) Use const reference for 's' to avoid unnecessary data copying. [redundantCopyLocalConst]\n", errout_str());

        check("void f1(const std::string& s) {\n"
              "    std::string s1 = s;\n"
              "    (void)s1;\n"
              "}\n"
              "void f2() {\n"
              "    const std::string s;\n"
              "    std::string s1 = s;\n"
              "    (void)s1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:17]: (performance, inconclusive) Use const reference for 's1' to avoid unnecessary data copying. [redundantCopyLocalConst]\n"
                      "[test.cpp:7:17]: (performance, inconclusive) Use const reference for 's1' to avoid unnecessary data copying. [redundantCopyLocalConst]\n",
                      errout_str());

        check("struct S {\n"
              "    std::string m;\n"
              "    int f(const std::string& s);\n"
              "};\n"
              "int S::f(const std::string& s) {\n"
              "    std::string c = s;\n"
              "    m.clear();\n"
              "    return c.size();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    std::string m;\n"
              "    int f(std::string s);\n"
              "};\n"
              "int S::f(std::string s) {\n"
              "    s += m;\n"
              "    std::string c = s;\n"
              "    m.clear();\n"
              "    return c.size();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:17]: (performance, inconclusive) Use const reference for 'c' to avoid unnecessary data copying. [redundantCopyLocalConst]\n",
                      errout_str());

        check("int f(const char* c) {\n" // #14530
              "    std::string s = c;\n"
              "    return s.rfind('.');\n"
              "}\n"
              "struct M {\n"
              "    M(const std::array<double, 9>& a);\n"
              "    double m[3][3];\n"
              "    double trace() const;\n"
              "};\n"
              "double g(const std::array<double, 9>& a) {\n"
              "    M m = a;\n"
              "    return m.trace();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void checkNegativeShift() {
        check("void foo()\n"
              "{\n"
              "   int a; a = 123;\n"
              "   (void)(a << -1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Shifting by a negative value is undefined behaviour [shiftNegative]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "   int a; a = 123;\n"
              "   (void)(a >> -1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Shifting by a negative value is undefined behaviour [shiftNegative]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "   int a; a = 123;\n"
              "   a <<= -1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Shifting by a negative value is undefined behaviour [shiftNegative]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "   int a; a = 123;\n"
              "   a >>= -1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Shifting by a negative value is undefined behaviour [shiftNegative]\n", errout_str());
        check("void foo()\n"
              "{\n"
              "   std::cout << -1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "   std::cout << a << -1 ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo()\n"
              "{\n"
              "   std::cout << 3 << -1 ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo() {\n"
              "   x = (-10+2) << 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (portability) Shifting a negative value is technically undefined behaviour [shiftNegativeLHS]\n", errout_str());

        check("x = y ? z << $-1 : 0;");
        ASSERT_EQUALS("", errout_str());

        // Negative LHS
        check("const int x = -1 >> 2;");
        ASSERT_EQUALS("[test.cpp:1:18]: (portability) Shifting a negative value is technically undefined behaviour [shiftNegativeLHS]\n", errout_str());

        // #6383 - unsigned type
        check("const int x = (unsigned int)(-1) >> 2;");
        ASSERT_EQUALS("", errout_str());

        // #7814 - UB happening in valueflowcode when it tried to compute shifts.
        check("int shift1() { return 1 >> -1 ;}\n"
              "int shift2() { return 1 << -1 ;}\n"
              "int shift3() { return -1 >> 1 ;}\n"
              "int shift4() { return -1 << 1 ;}");
        ASSERT_EQUALS("[test.cpp:1:25]: (error) Shifting by a negative value is undefined behaviour [shiftNegative]\n"
                      "[test.cpp:2:25]: (error) Shifting by a negative value is undefined behaviour [shiftNegative]\n"
                      "[test.cpp:3:26]: (portability) Shifting a negative value is technically undefined behaviour [shiftNegativeLHS]\n"
                      "[test.cpp:4:26]: (portability) Shifting a negative value is technically undefined behaviour [shiftNegativeLHS]\n", errout_str());

        check("void f(int i) {\n" // #12916
              "    if (i < 0) {\n"
              "        g(\"abc\" << i);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #13326
        check("template<int b>\n"
              "int f(int a)\n"
              "{\n"
              "    if constexpr (b >= 0) {\n"
              "        return a << b;\n"
              "    } else {\n"
              "        return a << -b;\n"
              "    }\n"
              "}\n"
              "int g() {\n"
              "    return f<1>(2)\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("template<int b>\n"
              "int f(int a)\n"
              "{\n"
              "    if constexpr (b >= 0) {\n"
              "        return a << b;\n"
              "    } else {\n"
              "        return a << -b;\n"
              "    }\n"
              "}\n"
              "int g() {\n"
              "    return f<-1>(2)\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void incompleteArrayFill() {
        check("void f() {\n"
              "    int a[5];\n"
              "    memset(a, 123, 5);\n"
              "    memcpy(a, b, 5);\n"
              "    memmove(a, b, 5);\n"
              "}");
        ASSERT_EQUALS(// TODO "[test.cpp:4] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n"
            "[test.cpp:3:5]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'? [incompleteArrayFill]\n"
            "[test.cpp:4:5]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memcpy()' with 'sizeof(*a)'? [incompleteArrayFill]\n"
            "[test.cpp:5:5]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memmove()' with 'sizeof(*a)'? [incompleteArrayFill]\n", errout_str());

        check("int a[5];\n"
              "namespace Z { struct B { int a[5]; } b; }\n"
              "void f() {\n"
              "    memset(::a, 123, 5);\n"
              "    memset(Z::b.a, 123, 5);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4:5]: (warning, inconclusive) Array '::a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*::a)'? [incompleteArrayFill]\n"
                           "[test.cpp:5]: (warning, inconclusive) Array 'Z::b.a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*Z::b.a)'?\n",
                           "[test.cpp:4:5]: (warning, inconclusive) Array '::a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*::a)'? [incompleteArrayFill]\n", errout_str());

        check("void f() {\n"
              "    Foo* a[5];\n"
              "    memset(a, 'a', 5);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'? [incompleteArrayFill]\n", errout_str());

        check("class Foo {int a; int b;};\n"
              "void f() {\n"
              "    Foo a[5];\n"
              "    memset(a, 'a', 5);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'? [incompleteArrayFill]\n", errout_str());

        check("void f() {\n"
              "    Foo a[5];\n" // Size of foo is unknown
              "    memset(a, 'a', 5);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char a[5];\n"
              "    memset(a, 'a', 5);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a[5];\n"
              "    memset(a+15, 'a', 5);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    bool a[5];\n"
              "    memset(a, false, 5);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (portability, inconclusive) Array 'a' might be filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'? [incompleteArrayFill]\n", errout_str());

        check("void f() {\n"
              "    const int n = 5;"
              "    int a[n];\n"
              "    memset(a, 0, n);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning, inconclusive) Array 'a' is filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'? [incompleteArrayFill]\n", errout_str());
    }

    void redundantVarAssignment() {
        setMultiline();

        // Simple tests
        check("void f(int i) {\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: style: Variable 'i' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:2:7]: note: i is assigned\n"
                      "[test.cpp:3:7]: note: i is overwritten\n", errout_str());

        // non-local variable => only show warning when inconclusive is used
        check("int i;\n"
              "void f() {\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: style: Variable 'i' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:3:7]: note: i is assigned\n"
                      "[test.cpp:4:7]: note: i is overwritten\n", errout_str());

        check("void f() {\n"
              "    int i;\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: style: Variable 'i' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:3:7]: note: i is assigned\n"
                      "[test.cpp:4:7]: note: i is overwritten\n", errout_str());

        check("void f() {\n"
              "    static int i;\n"
              "    i = 1;\n"
              "    i = 1;\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        check("void f() {\n"
              "    int i[10];\n"
              "    i[2] = 1;\n"
              "    i[2] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: style: Variable 'i[2]' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:3:10]: note: i[2] is assigned\n"
                      "[test.cpp:4:10]: note: i[2] is overwritten\n", errout_str());

        check("void f(int x) {\n"
              "    int i[10];\n"
              "    i[x] = 1;\n"
              "    x=1;\n"
              "    i[x] = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const int x) {\n"
              "    int i[10];\n"
              "    i[x] = 1;\n"
              "    i[x] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: style: Variable 'i[x]' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:3:10]: note: i[x] is assigned\n"
                      "[test.cpp:4:10]: note: i[x] is overwritten\n", errout_str());

        // Testing different types
        check("void f() {\n"
              "    Foo& bar = foo();\n"
              "    bar = x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    Foo& bar = foo();\n"
              "    bar = x;\n"
              "    bar = y;\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        check("void f() {\n"
              "    Foo& bar = foo();\n" // #4425. bar might refer to something global, etc.
              "    bar = y();\n"
              "    foo();\n"
              "    bar = y();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Tests with function call between assignment
        check("void f(int i) {\n"
              "    i = 1;\n"
              "    bar();\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: style: Variable 'i' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:2:7]: note: i is assigned\n"
                      "[test.cpp:4:7]: note: i is overwritten\n", errout_str());

        check("int i;\n"
              "void f() {\n"
              "    i = 1;\n"
              "    bar();\n" // Global variable might be accessed in bar()
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    static int i;\n"
              "    i = 1;\n"
              "    bar();\n" // bar() might call f() recursively. This could be a false positive in more complex examples (when value of i is used somewhere. See #4229)
              "    i = 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int i;\n"
              "    i = 1;\n"
              "    bar();\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:7]: style: Variable 'i' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:3:7]: note: i is assigned\n"
                      "[test.cpp:5:7]: note: i is overwritten\n", errout_str());

        check("void bar(int i) {}\n"
              "void f(int i) {\n"
              "    i = 1;\n"
              "    bar(i);\n" // Passed as argument
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    Foo bar = foo();\n"
              "    bar();\n" // #5568. operator() called
              "    bar = y();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Branch tests
        check("void f(int i) {\n"
              "    i = 1;\n"
              "    if(x)\n"
              "        i = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i) {\n"
              "    if(x)\n"
              "        i = 0;\n"
              "    i = 1;\n"
              "    i = 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:7]: style: Variable 'i' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:4:7]: note: i is assigned\n"
                      "[test.cpp:5:7]: note: i is overwritten\n", errout_str());

        // #4513
        check("int x;\n"
              "int g() {\n"
              "    return x*x;\n"
              "}\n"
              "void f() {\n"
              "    x = 2;\n"
              "    x = g();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int g() {\n"
              "    return x*x;\n"
              "}\n"
              "void f(int x) {\n"
              "    x = 2;\n"
              "    x = g();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:7]: style: Variable 'x' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:5:7]: note: x is assigned\n"
                      "[test.cpp:6:7]: note: x is overwritten\n", errout_str());

        check("void f() {\n"
              "    Foo& bar = foo();\n"
              "    bar = x;\n"
              "    bar = y();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n"
              "    int x;\n"
              "    void g() { return x * x; }\n"
              "    void f();\n"
              "};\n"
              "\n"
              "void C::f() {\n"
              "    x = 2;\n"
              "    x = g();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n"
              "    int x;\n"
              "    void g() { return x*x; }\n"
              "    void f(Foo z);\n"
              "};\n"
              "\n"
              "void C::f(Foo z) {\n"
              "    x = 2;\n"
              "    x = z.g();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ({ })
        check("void f() {\n"
              "  int x;\n"
              "  x = 321;\n"
              "  x = ({ asm(123); })\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // from #3103 (avoid a false negative)
        check("int foo(){\n"
              "    int x;\n"
              "    x = 1;\n"
              "    x = 1;\n"
              "    return x + 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: style: Variable 'x' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:3:7]: note: x is assigned\n"
                      "[test.cpp:4:7]: note: x is overwritten\n", errout_str());

        // from #3103 (avoid a false positive)
        check("int foo(){\n"
              "    int x;\n"
              "    x = 1;\n"
              "    if (y)\n" // <-- cppcheck does not know anything about 'y'
              "        x = 2;\n"
              "    return x + 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // initialization, assignment with 0
        check("void f() {\n"  // Ticket #4356
              "    int x = 0;\n"  // <- ignore initialization with 0
              "    x = 3;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  state_t *x = NULL;\n"
              "  x = dostuff();\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:12]: style: Variable 'x' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("void f() {\n"
              "  state_t *x;\n"
              "  x = NULL;\n"
              "  x = dostuff();\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:12]: style: Variable 'x' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("int foo() {\n" // #4420
              "    int x;\n"
              "    bar(++x);\n"
              "    x = 5;\n"
              "    return bar(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // struct member..
        check("struct AB { int a; int b; };\n"
              "\n"
              "int f() {\n"
              "    struct AB ab;\n"
              "    ab.a = 1;\n"
              "    ab.a = 2;\n"
              "    return ab.a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:10]: style: Variable 'ab.a' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:5:10]: note: ab.a is assigned\n"
                      "[test.cpp:6:10]: note: ab.a is overwritten\n", errout_str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f() {\n"
              "    struct AB ab;\n"
              "    ab.a = 1;\n"
              "    ab = do_something();\n"
              "    return ab.a;\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f() {\n"
              "    struct AB ab;\n"
              "    ab.a = 1;\n"
              "    do_something(&ab);\n"
              "    ab.a = 2;\n"
              "    return ab.a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f(DO_SOMETHING do_something) {\n"
              "    struct AB ab;\n"
              "    ab.a = 1;\n"
              "    do_something(&ab);\n"
              "    ab.a = 2;\n"
              "    return ab.a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f(struct AB *ab) {\n"
              "    ab->a = 1;\n"
              "    ab->b = 2;\n"
              "    ab++;\n"
              "    ab->a = 1;\n"
              "    ab->b = 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct AB { int a; int b; };\n"
              "\n"
              "int f(struct AB *ab) {\n"
              "    ab->a = 1;\n"
              "    ab->b = 2;\n"
              "    ab = x;\n"
              "    ab->a = 1;\n"
              "    ab->b = 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(struct AB *ab) {\n" // #
              "    ab->data->x = 1;\n"
              "    ab = &ab1;\n"
              "    ab->data->x = 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5964
        check("void func(char *buffer, const char *format, int precision, unsigned value) {\n"
              "    (precision < 0) ? sprintf(buffer, format, value) : sprintf(buffer, format, precision, value);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // don't crash
        check("struct data {\n"
              "  struct { int i; } fc;\n"
              "};\n"
              "struct state {\n"
              "  struct data d[123];\n"
              "};\n"
              "void func(struct state *s) {\n"
              "  s->foo[s->x++] = 2;\n"
              "  s->d[1].fc.i++;\n"
              "}");

        // #6525 - inline assembly
        check("void f(int i) {\n"
              "    i = 1;\n"
              "    asm(\"foo\");\n"
              "    i = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6555
        check("void foo() {\n"
              "    char *p = 0;\n"
              "    try {\n"
              "        p = fred();\n"
              "        p = wilma();\n"
              "    }\n"
              "    catch (...) {\n"
              "        barney(p);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    char *p = 0;\n"
              "    try {\n"
              "        p = fred();\n"
              "        p = wilma();\n"
              "    }\n"
              "    catch (...) {\n"
              "        barney(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: style: The scope of the variable 'p' can be reduced. [variableScope]\n"
                      "[test.cpp:2:11]: style: Variable 'p' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("void foo() {\n"
              "    char *p = 0;\n"
              "    try {\n"
              "        if(z) {\n"
              "            p = fred();\n"
              "            p = wilma();\n"
              "        }\n"
              "    }\n"
              "    catch (...) {\n"
              "        barney(p);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Member variable pointers
        check("void podMemPtrs() {\n"
              "    const int POD::*memptr;\n"
              "    memptr = &POD::a;\n"
              "    memptr = &POD::b;\n"
              "    if (memptr)\n"
              "        memptr = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: style: Variable 'memptr' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:3:12]: note: memptr is assigned\n"
                      "[test.cpp:4:12]: note: memptr is overwritten\n", errout_str());

        // Pointer function argument (#3857)
        check("void f(float * var)\n"
              "{\n"
              "  var[0] = 0.2f;\n"
              "  var[0] = 0.2f;\n" // <-- is initialized twice
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: style: Variable 'var[0]' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:3:10]: note: var[0] is assigned\n"
                      "[test.cpp:4:10]: note: var[0] is overwritten\n", errout_str());

        check("void f(float * var)\n"
              "{\n"
              "  *var = 0.2f;\n"
              "  *var = 0.2f;\n" // <-- is initialized twice
              "}");
        ASSERT_EQUALS("[test.cpp:4:8]: style: Variable '*var' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:3:8]: note: *var is assigned\n"
                      "[test.cpp:4:8]: note: *var is overwritten\n", errout_str());

        // Volatile variables
        check("void f() {\n"
              "  volatile char *reg = (volatile char *)0x12345;\n"
              "  *reg = 12;\n"
              "  *reg = 34;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::map<int, int>& m, int key, int value) {\n" // #6379
              "    m[key] = value;\n"
              "    m[key] = value;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: style: Variable 'm[key]' is reassigned a value before the old one has been used. [redundantAssignment]\n"
                      "[test.cpp:2:12]: note: m[key] is assigned\n"
                      "[test.cpp:3:12]: note: m[key] is overwritten\n",
                      errout_str());
    }

    void redundantVarAssignment_trivial() {
        check("void f() {\n"
              "   int a = 0;\n"
              "   a = 4;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "   int a;\n"
              "   a = 0;\n"
              "   a = 4;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "   unsigned a;\n"
              "   a = 0u;\n"
              "   a = 2u;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "   void* a;\n"
              "   a = (void*)0;\n"
              "   a = p;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:10]: (style) Variable 'a' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("void f() {\n"
              "   void* a;\n"
              "   a = (void*)0U;\n"
              "   a = p;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:10]: (style) Variable 'a' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("std::string f() {\n" // #14534
              "    std::string s = \"abc\";\n"
              "    s = \"def\";\n"
              "    return s;\n"
              "}\n"
              "const char* g() {\n"
              "    const char* p = \"abc\";\n"
              "    p = \"def\";\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:19] -> [test.cpp:3:7]: (style) Redundant initialization for 's'. The initialized value is overwritten before it is read. [redundantInitialization]\n"
            "[test.cpp:7:19] -> [test.cpp:8:7]: (style) Redundant initialization for 'p'. The initialized value is overwritten before it is read. [redundantInitialization]\n",
            errout_str());
    }

    void redundantVarAssignment_struct() {
        check("struct foo {\n"
              "  int a,b;\n"
              "};\n"
              "\n"
              "int main() {\n"
              "  struct foo x;\n"
              "  x.a = _mm_set1_ps(1.0);\n"
              "  x.a = _mm_set1_ps(2.0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:7] -> [test.cpp:8:7]: (style) Variable 'x.a' is reassigned a value before the old one has been used. [redundantAssignment]\n", errout_str());

        check("void f() {\n"
              "  struct AB ab;\n"
              "  ab.x = 23;\n"
              "  ab.y = 41;\n"
              "  ab.x = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:5:8]: (style) Variable 'ab.x' is reassigned a value before the old one has been used. [redundantAssignment]\n", errout_str());

        check("void f() {\n"
              "  struct AB ab = {0};\n"
              "  ab = foo();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantVarAssignment_union() {
        // Ticket #5115 "redundantAssignment when using a union"
        check("void main(void)\n"
              "{\n"
              "    short lTotal = 0;\n"
              "    union\n"
              "    {\n"
              "        short l1;\n"
              "        struct\n"
              "        {\n"
              "            unsigned char b1;\n"
              "            unsigned char b2;\n"
              "        } b;\n"
              "    } u;\n"
              "    u.l1 = 1;\n"
              "    lTotal += u.b.b1;\n"
              "    u.l1 = 2;\n" //Should not show RedundantAssignment
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // Ticket #5115 "redundantAssignment when using a union"
        check("void main(void)\n"
              "{\n"
              "    short lTotal = 0;\n"
              "    union\n"
              "    {\n"
              "        short l1;\n"
              "        struct\n"
              "        {\n"
              "            unsigned char b1;\n"
              "            unsigned char b2;\n"
              "        } b;\n"
              "    } u;\n"
              "    u.l1 = 1;\n"
              "    u.l1 = 2;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:13:10] -> [test.cpp:14:10]: (style) Variable 'u.l1' is reassigned a value before the old one has been used. [redundantAssignment]\n", errout_str());

        // Ticket #10093 "redundantAssignment when using a union"
        check("typedef union fixed32_union {\n"
              "    struct {\n"
              "        unsigned32 abcd;\n"
              "    } u32;\n"
              "    struct {\n"
              "        unsigned16 ab;\n"
              "        unsigned16 cd;\n"
              "    } u16;"
              "    struct {\n"
              "        unsigned8 a;\n"
              "        unsigned8 b;\n"
              "        unsigned8 c;\n"
              "        unsigned8 d;\n"
              "        } b;\n"
              "} fixed32;\n"
              "void func1(void) {\n"
              "    fixed32 m;\n"
              "    m.u16.ab = 47;\n"
              "    m.u16.cd = 0;\n"
              "    m.u16.ab = m.u32.abcd / 53;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // Ticket #10093 "redundantAssignment when using a union"
        check("typedef union{\n"
              "    char as_char[4];\n"
              "    int as_int;\n"
              "} union_t;\n"
              "void fn(char *data, int len) {\n"
              "    int i;\n"
              "    for (i = 0; i < len; i++)\n"
              "        data[i] = 'a';\n"
              "}\n"
              "int main(int argc, char *argv[]) {\n"
              "    union_t u;\n"
              "    u.as_int = 42;\n"
              "    fn(&u.as_char[0], 4);\n"
              "    u.as_int = 0;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // Ticket #5115 "redundantAssignment when using a union"
        check("void foo(char *ptr) {\n"
              "    union {\n"
              "        char * s8;\n"
              "        unsigned long long u64;\n"
              "    } addr;\n"
              "    addr.s8 = ptr;\n"
              "    addr.u64 += 8;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12895
              "    int x, y;\n"
              "};\n"
              "union U {\n"
              "    S* s;\n"
              "};\n"
              "void f(const U& Src, const U& Dst) {\n"
              "    Dst.s->x = Src.s->x;\n"
              "    Dst.s->y = Src.s->y;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantVarAssignment_7133() {
        // #7133
        check("sal_Int32 impl_Export() {\n"
              "   try {\n"
              "        try  {\n"
              "          uno::Sequence< uno::Any > aArgs(2);\n"
              "          beans::NamedValue aValue;\n"
              "          aValue.Name = \"DocumentHandler\";\n"
              "          aValue.Value <<= xDocHandler;\n"
              "          aArgs[0] <<= aValue;\n"
              "          aValue.Name = \"Model\";\n"
              "          aValue.Value <<= xDocumentComp;\n"
              "          aArgs[1] <<= aValue;\n"
              "        }\n"
              "        catch (const uno::Exception&) {\n"
              "        }\n"
              "   }\n"
              "   catch (const uno::Exception&)  {\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void ConvertBitmapData(sal_uInt16 nDestBits) {\n"
              "    BitmapBuffer aSrcBuf;\n"
              "    aSrcBuf.mnBitCount = nSrcBits;\n"
              "    BitmapBuffer aDstBuf;\n"
              "    aSrcBuf.mnBitCount = nDestBits;\n"
              "    bConverted = ::ImplFastBitmapConversion( aDstBuf, aSrcBuf, aTwoRects );\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:24] -> [test.c:5:24]: (style) Variable 'aSrcBuf.mnBitCount' is reassigned a value before the old one has been used. [redundantAssignment]\n", errout_str());
        check("void ConvertBitmapData(sal_uInt16 nDestBits) {\n"
              "    BitmapBuffer aSrcBuf;\n"
              "    aSrcBuf.mnBitCount = nSrcBits;\n"
              "    BitmapBuffer aDstBuf;\n"
              "    aSrcBuf.mnBitCount = nDestBits;\n"
              "    bConverted = ::ImplFastBitmapConversion( aDstBuf, aSrcBuf, aTwoRects );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:24] -> [test.cpp:5:24]: (style) Variable 'aSrcBuf.mnBitCount' is reassigned a value before the old one has been used. [redundantAssignment]\n",
                      errout_str());

        check("class C { void operator=(int x); };\n" // #8368 - assignment operator might have side effects => inconclusive
              "void f() {\n"
              "    C c;\n"
              "    c = x;\n"
              "    c = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7] -> [test.cpp:5:7]: (style, inconclusive) Variable 'c' is reassigned a value before the old one has been used if variable is no semaphore variable. [redundantAssignment]\n", errout_str());
    }

    void redundantVarAssignment_stackoverflow() {
        check("typedef struct message_node {\n"
              "  char code;\n"
              "  size_t size;\n"
              "  struct message_node *next, *prev;\n"
              "} *message_list;\n"
              "static message_list remove_message_from_list(message_list m) {\n"
              "    m->prev->next = m->next;\n"
              "    m->next->prev = m->prev;\n"
              "    return m->next;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantVarAssignment_lambda() {
        // #7152
        check("int foo() {\n"
              "    int x = 0, y = 0;\n"
              "    auto f = [&]() { if (x < 5) ++y; };\n"
              "    x = 2;\n"
              "    f();\n"
              "    x = 6;\n"
              "    f();\n"
              "    return y;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #10228
        check("std::tuple<int, int> g();\n"
              "void h(int);\n"
              "void f() {\n"
              "    auto [a, b] = g();\n"
              "    auto l = [a = a]() { h(i); };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantVarAssignment_loop() {
        check("void f() {\n"
              "    char buf[10];\n"
              "    int i;\n"
              "    for (i = 0; i < 4; i++)\n"
              "        buf[i] = 131;\n"
              "    buf[i] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void bar() {\n" // #9262 do-while with break
              "    int x = 0;\n"
              "    x = 432;\n"
              "    do {\n"
              "        if (foo()) break;\n"
              "        x = 1;\n"
              "     } while (false);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int num) {\n" // #9420 FP
              "  int a = num;\n"
              "  for (int b = 0; b < num; a = b++)\n"
              "    dostuff(a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int num) {\n" // #9420 FN
              "  int a = num;\n"
              "  for (int b = 0; b < num; a = b++);\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

    void redundantVarAssignment_after_switch() {
        check("void f(int x) {\n" // #7907
              "    int ret;\n"
              "    switch (x) {\n"
              "    case 123:\n"
              "        ret = 1;\n" // redundant assignment
              "        break;\n"
              "    }\n"
              "    ret = 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13] -> [test.cpp:8:9]: (style) Variable 'ret' is reassigned a value before the old one has been used. [redundantAssignment]\n", errout_str());
    }

    void redundantVarAssignment_pointer() {
        check("void f(int *ptr) {\n"
              "    int *x = ptr + 1;\n"
              "    *x = 23;\n"
              "    foo(ptr);\n"
              "    *x = 32;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8997
        check("void f() {\n"
              "  char x[2];\n"
              "  char* p = x;\n"
              "  *p = 1;\n"
              "  p += 1;\n"
              "  *p = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantVarAssignment_pointer_parameter() {
        check("void f(int *p) {\n"
              "    *p = 1;\n"
              "    if (condition) return;\n"
              "    *p = 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantVarAssignment_array() {
        check("void f() {\n"
              "    int arr[10];\n"
              "    int i = 0;\n"
              "    arr[i] = 1;\n"
              "    i += 2;\n"
              "    arr[i] = 3;\n"
              "    dostuff(arr);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantVarAssignment_switch_break() {
        // #10058
        check("void f(int a, int b) {\n"
              "    int ret = 0;\n"
              "    switch (a) {\n"
              "    case 1:\n"
              "        ret = 543;\n"
              "        if (b) break;\n"
              "        ret = 1;\n"
              "        break;\n"
              "    }"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a, int b) {\n"
              "    int ret = 0;\n"
              "    switch (a) {\n"
              "    case 1:\n"
              "        ret = 543;\n"
              "        if (b) break;\n"
              "        ret = 1;\n"
              "        break;\n"
              "    }"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13] -> [test.cpp:7:13]: (style) Variable 'ret' is reassigned a value before the old one has been used. [redundantAssignment]\n", errout_str());
    }

    void redundantInitialization() {
        setMultiline();

        check("void f() {\n"
              "    int err = -ENOMEM;\n"
              "    err = dostuff();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: style: Redundant initialization for 'err'. The initialized value is overwritten before it is read. [redundantInitialization]\n"
                      "[test.cpp:2:13]: note: err is initialized\n"
                      "[test.cpp:3:9]: note: err is overwritten\n",
                      errout_str());

        check("void f() {\n"
              "    struct S s = {1,2,3};\n"
              "    s = dostuff();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: style: Redundant initialization for 's'. The initialized value is overwritten before it is read. [redundantInitialization]\n"
                      "[test.cpp:2:16]: note: s is initialized\n"
                      "[test.cpp:3:7]: note: s is overwritten\n",
                      errout_str());

        check("void f() {\n"
              "    int *p = NULL;\n"
              "    p = dostuff();\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:10]: style: Variable 'p' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        // "trivial" initialization => do not warn
        check("void f() {\n"
              "    struct S s = {0};\n"
              "    s = dostuff();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("namespace N { enum E {e0,e1}; }\n"
              "void f() {\n"
              "    N::E e = N::e0;\n" // #9261
              "    e = dostuff();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #10143
              "    std::shared_ptr<int> i = g();\n"
              "    h();\n"
              "    i = nullptr;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(const std::vector<int>& v) {\n" // #9815
              "    int i = g();\n"
              "    i = std::distance(v.begin(), std::find_if(v.begin(), v.end(), [=](int j) { return i == j; }));\n"
              "    return i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // cppcheck-suppress unusedPrivateFunction
    void redundantMemWrite() {
        // Simple tests
        check("void f() {\n"
              "    char a[10];\n"
              "    memcpy(a, foo, bar);\n"
              "    memset(a, 0, bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout_str());

        check("void f() {\n"
              "    char a[10];\n"
              "    strcpy(a, foo);\n"
              "    strncpy(a, 0, bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout_str());

        check("void f() {\n"
              "    char a[10];\n"
              "    sprintf(a, \"foo\");\n"
              "    memmove(a, 0, bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout_str());

        check("void f(char *filename) {\n"
              "    char *p = strrchr(filename,'.');\n"
              "    strcpy(p, \"foo\");\n"
              "    dostuff(filename);\n"
              "    strcpy(p, \"foo\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Writing to different parts of a buffer
        check("void f(void* a) {\n"
              "    memcpy(a, foo, bar);\n"
              "    memset(a+5, 0, bar);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Use variable as second argument
        check("void f(void* a, void* b) {\n"
              "    memset(a, 0, 5);\n"
              "    memcpy(b, a, 5);\n"
              "    memset(a, 1, 5);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // strcat is special
        check("void f() {\n"
              "    char a[10];\n"
              "    strcpy(a, foo);\n"
              "    strcat(a, bar);\n" // Not redundant
              "    strcpy(a, x);\n" // Redundant
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout_str());

        // Tests with function call between copy
        check("void f() {\n"
              "    char a[10];\n"
              "    snprintf(a, foo, bar);\n"
              "    bar();\n"
              "    memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n", errout_str());

        check("void* a;\n"
              "void f() {\n"
              "    memset(a, 0, size);\n"
              "    bar();\n" // Global variable might be accessed in bar()
              "    memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char a[10];\n"
              "    memset(a, 0, size);\n"
              "    bar();\n"
              "    memset(a, 0, size);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (performance) Buffer 'a' is being written before its old content has been used.\n", "", errout_str());

        check("void bar(void* a) {}\n"
              "void f(void* a) {\n"
              "    memset(a, 0, size);\n"
              "    bar(a);\n" // Passed as argument
              "    memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Branch tests
        check("void f(void* a) {\n"
              "    memset(a, 0, size);\n"
              "    if(x)\n"
              "        memset(a, 0, size);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #4455 - initialization of local buffer
        check("void f(void) {"
              "    char buf[10];\n"
              "    memset(buf, 0, 10);\n"
              "    strcpy(buf, string);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(void) {\n"
              "    char buf[10] = {0};\n"
              "    memset(buf, 0, 10);\n"
              "    strcpy(buf, string);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (performance) Buffer 'buf' is being written before its old content has been used.\n", errout_str());

        // #5689 - use return value of strcpy
        check("int f(void* a) {\n"
              "    int i = atoi(strcpy(a, foo));\n"
              "    strncpy(a, 0, bar);\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7175 - read+write
        check("void f() {\n"
              "    char buf[100];\n"
              "    strcpy(buf, x);\n"
              "    strcpy(buf, dostuff(buf));\n" // <- read + write
              "    strcpy(buf, x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char buf[100];\n"
              "    strcpy(buf, x);\n"
              "    strcpy(buf, dostuff(buf));\n"
              "    strcpy(buf, x);\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

    void redundantAssignmentSameValue() {
        check("int main() {\n" // #11642
              "    int a = 0;\n"
              "    int b = a;\n"
              "    int c = 1;\n"
              "    a = b;\n"
              "    return a * b * c;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:5:5]: (style) Variable 'a' is assigned an expression that holds the same value. [redundantAssignment]\n", errout_str());

        check("int main() {\n"
              "    int a = 0;\n"
              "    int b = a;\n"
              "    int c = 1;\n"
              "    a = b + 1;\n"
              "    return a * b * c;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int main() {\n"
              "    int a = 0;\n"
              "    int b = a;\n"
              "    int c = 1;\n"
              "    a = b = 5;\n"
              "    return a * b * c;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:5:11]: (style) Redundant initialization for 'b'. The initialized value is overwritten before it is read. [redundantInitialization]\n", errout_str());

        check("int f(int i) {\n" // #12874
              "    int j = i + 1;\n"
              "    if (i > 5)\n"
              "        j = i;\n"
              "    return j;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12894
              "    std::string a;\n"
              "    void f(const S& s);\n"
              "    void g(const S& s);\n"
              "};\n"
              "void S::f(const S& s) {\n"
              "    std::string x = a;\n"
              "    this->operator=(s);\n"
              "    a = x;\n"
              "}\n"
              "void S::g(const S& s) {\n"
              "    std::string x = a;\n"
              "    operator=(s);\n"
              "    a = x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int a, int b, int c) {\n" // #4491
              "    int x = a + b;\n"
              "    if (c)\n"
              "        x = a + b;\n"
              "    return x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:4:9]: (style) Variable 'x' is assigned an expression that holds the same value. [redundantAssignment]\n",
                      errout_str());
    }

    void varFuncNullUB() { // #4482
        check("void a(...);\n"
              "void b() { a(NULL); }");
        ASSERT_EQUALS("[test.cpp:2:13]: (portability) Passing NULL after the last typed argument to a variadic function leads to undefined behaviour. [varFuncNullUB]\n", errout_str());

        check("void a(char *p, ...);\n"
              "void b() { a(NULL, 2); }");
        ASSERT_EQUALS("", errout_str());

        checkP("extern const int sentinel;\n"
               "void a(int, ...);\n"
               "#define b(x, ...) a((x), __VA_ARGS__, &sentinel)\n"
               "void c() { b(1, NULL); }");
        ASSERT_EQUALS("", errout_str());
    }

    void checkCastIntToCharAndBack() { // #160

        // check getchar
        check("void f() {\n"
              "unsigned char c; c = getchar();\n"
              "  while( c != EOF)\n"
              "  {\n"
              "    bar(c);\n"
              "    c = getchar();\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (warning) Storing getchar() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f() {\n"
              "unsigned char c = getchar();\n"
              "  while( EOF != c)\n"
              "  {\n"
              "    bar(c);\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (warning) Storing getchar() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f() {\n"
              "  unsigned char c; c = getchar();\n"
              "  while( EOF != c )\n"
              "  {\n"
              "    bar(c);\n"
              "    c = getchar();\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (warning) Storing getchar() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f() {\n"
              "  unsigned char c;\n"
              "  while( EOF != ( c = getchar() ) )\n"
              "  {\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (warning) Storing getchar() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f() {\n"
              "  int i; i = getchar();\n"
              "  while( i != EOF)\n"
              "  {\n"
              "    bar(i);\n"
              "    i = getchar();\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int i; i = getchar();\n"
              "  while( EOF != i )\n"
              "  {\n"
              "    bar(i);\n"
              "    i = getchar();\n"
              "  } ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        // check getc
        check("void f (FILE * pFile){\n"
              "unsigned char c;\n"
              "do {\n"
              "  c = getc (pFile);\n"
              "} while (c != EOF);"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (warning) Storing getc() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f (FILE * pFile){\n"
              "unsigned char c;\n"
              "do {\n"
              "  c = getc (pFile);\n"
              "} while (EOF != c);"
              "}");
        ASSERT_EQUALS("[test.cpp:5:17]: (warning) Storing getc() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = getc (pFile);\n"
              "} while (i != EOF);"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = getc (pFile);\n"
              "} while (EOF != i);"
              "}");
        ASSERT_EQUALS("", errout_str());


        // check fgetc
        check("void f (FILE * pFile){\n"
              "unsigned char c;\n"
              "do {\n"
              "  c = fgetc (pFile);\n"
              "} while (c != EOF);"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (warning) Storing fgetc() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f (FILE * pFile){\n"
              "char c;\n"
              "do {\n"
              "  c = fgetc (pFile);\n"
              "} while (EOF != c);"
              "}");
        ASSERT_EQUALS("[test.cpp:5:17]: (warning) Storing fgetc() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f (FILE * pFile){\n"
              "signed char c;\n"
              "do {\n"
              "  c = fgetc (pFile);\n"
              "} while (EOF != c);"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = fgetc (pFile);\n"
              "} while (i != EOF);"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f (FILE * pFile){\n"
              "int i;\n"
              "do {\n"
              "  i = fgetc (pFile);\n"
              "} while (EOF != i);"
              "}");
        ASSERT_EQUALS("", errout_str());

        // cin.get()
        check("void f(){\n"
              "   char ch; ch = std::cin.get();\n"
              "   while (EOF != ch) {\n"
              "        std::cout << ch;\n"
              "        ch = std::cin.get();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (warning) Storing cin.get() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f(){\n"
              "   char ch; ch = std::cin.get();\n"
              "   while (ch != EOF) {\n"
              "        std::cout << ch;\n"
              "        ch = std::cin.get();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (warning) Storing cin.get() return value in char variable and then comparing with EOF. [checkCastIntToCharAndBack]\n", errout_str());

        check("void f(){\n"
              "   int i; i = std::cin.get();\n"
              "   while ( EOF != i ) {\n"
              "        std::cout << i;\n"
              "        i = std::cin.get();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(){\n"
              "   int i; i = std::cin.get();\n"
              "   while ( i != EOF ) {\n"
              "        std::cout << i;\n"
              "        i = std::cin.get();\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkCommaSeparatedReturn() {
        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return a++,\n"
              "  do_something();\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Comma is used in return statement. The comma can easily be misread as a ';'.\n", "", errout_str());

        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return a++, do_something();\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return a+5,\n"
              "  do_something();\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Comma is used in return statement. The comma can easily be misread as a ';'.\n", "", errout_str());

        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return a+5, do_something();\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("int fun(int a) {\n"
              "  if (a < 0)\n"
              "    return c<int,\nint>::b;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        // #4943 take care of C++11 initializer lists
        check("std::vector<Foo> Bar() {\n"
              "    return\n"
              "    {\n"
              "        { \"1\" },\n"
              "        { \"2\" },\n"
              "        { \"3\" }\n"
              "    };\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
    }

    void checkPassByReference() {
        // #8570 passByValue when std::move is used
        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    explicit B(A a) : a(std::move(a)) {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "};", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    explicit B(A a) : a{std::move(a)} {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "};", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    B(A a, A a2) : a{std::move(a)}, a2{std::move(a2)} {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "    A a2;"
              "};", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    B(A a, A a2) : a{std::move(a)}, a2{a2} {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "    A a2;"
              "};", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:8:14]: (performance) Function parameter 'a2' should be passed by const reference. [passedByValue]\n", errout_str());

        check("struct A\n"
              "{\n"
              "    std::vector<int> x;\n"
              "};\n"
              "\n"
              "struct B\n"
              "{\n"
              "    B(A a, A a2) : a{std::move(a)}, a2(a2) {}\n"
              "    void Init(A _a) { a = std::move(_a); }\n"
              "    A a;"
              "    A a2;"
              "};", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:8:14]: (performance) Function parameter 'a2' should be passed by const reference. [passedByValue]\n", errout_str());

        check("std::map<int, int> m;\n" // #10817
              "void f(const decltype(m)::const_iterator i) {}");
        ASSERT_EQUALS("", errout_str());

        check("int (*pf) (std::vector<int>) = nullptr;\n" // #12118
              "int f(std::vector<int> v) {\n"
              "    return v.size();\n"
              "}\n"
              "void g() {\n"
              "    pf = f;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:10] -> [test.cpp:2:24]: (performance) Function parameter 'v' should be passed by const reference. However it seems that 'f' is a callback function. [passedByValueCallback]\n",
                      errout_str());

        check("template<typename T> struct A;\n" // #12621
              "template<typename T>\n"
              "struct B { A<T> a; };\n"
              "template<typename T>\n"
              "struct A { B<T> b; };\n"
              "template<typename T>\n"
              "struct C : public virtual A<T>, public virtual B<T> {\n"
              "    A<T> x;\n"
              "    B<T> y;\n"
              "    C(A<T> x_, B<T> y_) : x(x_), y(y_) {}\n"
              "};\n");
        ASSERT_EQUALS("", errout_str()); // don't crash

        check("template <typename T, int N>\n" // #13537
              "    struct S {\n"
              "    T a[N];\n"
              "};\n"
              "void f(S<char, 3> s) {}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n" // #14667
              "    U u1, u2;\n"
              "    union {\n"
              "        enum { E0, E1 } e;\n"
              "        U u3;\n"
              "        T i;\n"
              "    } x;\n"
              "};\n"
              "T f(T t) {\n"
              "    return t;\n"
              "}");
        ASSERT_EQUALS("", errout_str()); // don't crash

        Settings settingsUnix32 = settingsBuilder().platform(Platform::Type::Unix32).build();
        check("struct S {\n" // #13850
              "    int i0 : 32;\n"
              "    int i1 : 16;\n"
              "    unsigned short u16;\n"
              "};\n"
              "void f(S s) {}\n", dinit(CheckOptions, $.settings = &settingsUnix32));
        ASSERT_EQUALS("", errout_str());
    }

    void checkComparisonFunctionIsAlwaysTrueOrFalse() {
        // positive test
        check("bool f(int x){\n"
              "   return isless(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of two identical variables with isless(x,x) always evaluates to false. [comparisonFunctionIsAlwaysTrueOrFalse]\n", errout_str());

        check("bool f(int x){\n"
              "   return isgreater(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of two identical variables with isgreater(x,x) always evaluates to false. [comparisonFunctionIsAlwaysTrueOrFalse]\n", errout_str());

        check("bool f(int x){\n"
              "   return islessgreater(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of two identical variables with islessgreater(x,x) always evaluates to false. [comparisonFunctionIsAlwaysTrueOrFalse]\n", errout_str());

        check("bool f(int x){\n"
              "   return islessequal(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of two identical variables with islessequal(x,x) always evaluates to true. [comparisonFunctionIsAlwaysTrueOrFalse]\n", errout_str());

        check("bool f(int x){\n"
              "   return isgreaterequal(x,x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of two identical variables with isgreaterequal(x,x) always evaluates to true. [comparisonFunctionIsAlwaysTrueOrFalse]\n", errout_str());

        // no warning should be reported for
        check("bool f(int x, int y){\n"
              "   return isgreaterequal(x,y) && islessequal(x,y) && islessgreater(x,y) && isgreater(x,y) && isless(x,y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void integerOverflow() { // 5895
        // no signed integer overflow should happen
        check("void f(unsigned long long ull) {\n"
              "    if (ull == 0x89504e470d0a1a0a || ull == 0x8a4d4e470d0a1a0a) ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantPointerOp() {
        check("int *f(int *x) {\n"
              "    return &*x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:12]: (style) Redundant pointer operation on 'x' - it's already a pointer. [redundantPointerOp]\n", errout_str());

        check("int *f(int *y) {\n"
              "    return &(*y);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:12]: (style) Redundant pointer operation on 'y' - it's already a pointer. [redundantPointerOp]\n", errout_str());

        check("int f() {\n" // #10991
              "    int value = 4;\n"
              "    int result1 = *(&value);\n"
              "    int result2 = *&value;\n"
              "    return result1 + result2;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:19]: (style) Redundant pointer operation on 'value' - it's already a variable. [redundantPointerOp]\n"
                      "[test.cpp:4:19]: (style) Redundant pointer operation on 'value' - it's already a variable. [redundantPointerOp]\n",
                      errout_str());

        check("void f(int& a, int b) {\n"
              "    *(&a) = b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Redundant pointer operation on 'a' - it's already a variable. [redundantPointerOp]\n",
                      errout_str());

        check("void f(int**& p) {}\n");
        ASSERT_EQUALS("", errout_str());

        checkP("#define	RESTORE(ORIG, COPY) { *ORIG = *COPY; }\n"
               "void f(int* p, int i) {\n"
               "    RESTORE(p, &i);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        // no warning for bitwise AND
        check("void f(const int *b) {\n"
              "    int x = 0x20 & *b;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // No message for double pointers to structs
        check("void f(struct foo **my_struct) {\n"
              "    char **pass_to_func = &(*my_struct)->buf;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // another double pointer to struct - with an array
        check("void f(struct foo **my_struct) {\n"
              "    char **pass_to_func = &(*my_struct)->buf[10];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // double pointer to array
        check("void f(char **ptr) {\n"
              "    int *x = &(*ptr)[10];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Variable 'x' can be declared as pointer to const [constVariablePointer]\n", errout_str());

        // function calls
        check("void f(Mutex *mut) {\n"
              "    pthread_mutex_lock(&*mut);\n"
              "}\n", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:2:24]: (style) Redundant pointer operation on 'mut' - it's already a pointer. [redundantPointerOp]\n", errout_str());

        // make sure we got the AST match for "(" right
        check("void f(char *ptr) {\n"
              "    if (&*ptr == NULL)\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Redundant pointer operation on 'ptr' - it's already a pointer. [redundantPointerOp]\n", errout_str());

        // no warning for macros
        checkP("#define MUTEX_LOCK(m) pthread_mutex_lock(&(m))\n"
               "void f(struct mutex *mut) {\n"
               "    MUTEX_LOCK(*mut);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkP("#define B(op)        bar(op)\n"
               "#define C(orf)       B(&orf)\n"
               "void foo(const int * pkey) {\n"
               "    C(*pkey);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void test_isSameExpression() { // see #5738
        check("bool isInUnoIncludeFile(StringRef name) {"
              "   return  name.startswith(SRCDIR \"/com/\") || name.startswith(SRCDIR \"/uno/\");\n"
              "};", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
    }

    void raceAfterInterlockedDecrement() {
        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    whatever();\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (counter)\n"
                                  "        return;\n"
                                  "    destroy();\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (!counter)\n"
                                  "        destroy();\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (counter > 0)\n"
                                  "        return;\n"
                                  "    destroy();\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (0 < counter)\n"
                                  "        return;\n"
                                  "    destroy();\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (counter == 0)\n"
                                  "        destroy();\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (0 == counter)\n"
                                  "        destroy();\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (0 != counter)\n"
                                  "        return;\n"
                                  "    destroy()\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (counter != 0)\n"
                                  "        return;\n"
                                  "    destroy()\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (counter <= 0)\n"
                                  "        destroy();\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    InterlockedDecrement(&counter);\n"
                                  "    if (0 >= counter)\n"
                                  "        destroy();\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (newCount)\n"
                                  "        return;\n"
                                  "    destroy();\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (!newCount)\n"
                                  "        destroy();\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (newCount > 0)\n"
                                  "        return;\n"
                                  "    destroy();\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (0 < newCount)\n"
                                  "        return;\n"
                                  "    destroy();\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (newCount == 0)\n"
                                  "        destroy();\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (0 == newCount)\n"
                                  "        destroy();\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (0 != newCount)\n"
                                  "        return;\n"
                                  "    destroy()\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (newCount != 0)\n"
                                  "        return;\n"
                                  "    destroy()\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (newCount <= 0)\n"
                                  "        destroy();\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("void f() {\n"
                                  "    int counter = 0;\n"
                                  "    int newCount = InterlockedDecrement(&counter);\n"
                                  "    if (0 >= newCount)\n"
                                  "        destroy;\n"
                                  "}");
        ASSERT_EQUALS("", errout_str());

        checkInterlockedDecrement("int f() {\n"
                                  "    int counter = 0;\n"
                                  "    if (InterlockedDecrement(&counter) == 0) {\n"
                                  "        destroy();\n"
                                  "        return 0;\n"
                                  "    } else {\n"
                                  "        return counter;\n"
                                  "    }\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:7:16]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("int f() {\n"
                                  "    int counter = 0;\n"
                                  "    if (::InterlockedDecrement(&counter) == 0) {\n"
                                  "        destroy();\n"
                                  "        return 0;\n"
                                  "    } else {\n"
                                  "        return counter;\n"
                                  "    }\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:7:16]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());


        checkInterlockedDecrement("int f() {\n"
                                  "    int counter = 0;\n"
                                  "    if (InterlockedDecrement(&counter) == 0) {\n"
                                  "        destroy();\n"
                                  "        return 0;\n"
                                  "    }\n"
                                  "    return counter;\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:7:12]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("int f() {\n"
                                  "    int counter = 0;\n"
                                  "    if (::InterlockedDecrement(&counter) == 0) {\n"
                                  "        destroy();\n"
                                  "        return 0;\n"
                                  "    }\n"
                                  "    return counter;\n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:7:12]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("int f() {\n"
                                  "    int counter = 0;\n"
                                  "    if (InterlockedDecrement(&counter) == 0) {\n"
                                  "        destroy();\n"
                                  "        return 0;\n"
                                  "    } else\n"
                                  "        return counter;\n"
                                  "   \n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:7:16]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());

        checkInterlockedDecrement("int f() {\n"
                                  "    int counter = 0;\n"
                                  "    if (::InterlockedDecrement(&counter) == 0) {\n"
                                  "        destroy();\n"
                                  "        return 0;\n"
                                  "    } else\n"
                                  "        return counter;\n"
                                  "   \n"
                                  "}");
        ASSERT_EQUALS("[test.cpp:7:16]: (error) Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead. [raceAfterInterlockedDecrement]\n", errout_str());
    }

    void testUnusedLabel() {
        check("void f() {\n"
              "    label:\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Label 'label' is not used. [unusedLabel]\n", errout_str());

        check("void f() {\n"
              "    label:\n"
              "    foo();\n"
              "    goto label;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    label:\n"
              "    foo();\n"
              "    goto label;\n"
              "}\n"
              "void g() {\n"
              "    label:\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:5]: (style) Label 'label' is not used. [unusedLabel]\n", errout_str());

        check("void f() {\n"
              "    switch(a) {\n"
              "        default:\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    class X {\n"
              "        protected:\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    class X {\n"
              "        my_protected:\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int test(char art) {\n"
              "    switch (art) {\n"
              "    caseZERO:\n"
              "        return 0;\n"
              "    case1:\n"
              "        return 1;\n"
              "    case 2:\n"
              "        return 2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Label 'caseZERO' is not used. Should this be a 'case' of the enclosing switch()? [unusedLabelSwitch]\n"
                      "[test.cpp:5:5]: (warning) Label 'case1' is not used. Should this be a 'case' of the enclosing switch()? [unusedLabelSwitch]\n", errout_str());

        check("int test(char art) {\n"
              "    switch (art) {\n"
              "    case 2:\n"
              "        return 2;\n"
              "    }\n"
              "    label:\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (style) Label 'label' is not used. [unusedLabel]\n", errout_str());
    }


    void testUnusedLabelConfiguration() {
        checkP("void f() {\n"
               "#ifdef X\n"
               "    goto END;\n"
               "#endif\n"
               "END:\n"
               "}");
        ASSERT_EQUALS("[test.cpp:5:1]: (style) Label 'END' is not used. There is #if in function body so the label might be used in code that is removed by the preprocessor. [unusedLabelConfiguration]\n",
                      errout_str());
    }

    void testUnusedLabelSwitchConfiguration() {
        checkP("void f(int i) {\n"
               "    switch (i) {\n"
               "    default:\n"
               "        break;\n"
               "#ifdef X\n"
               "    case 1:\n"
               "        goto END;\n"
               "#endif\n"
               "    case 2:\n"
               "    END:\n"
               "        return;\n"
               "    }\n"
               "}");
        ASSERT_EQUALS("[test.cpp:10:5]: (warning) Label 'END' is not used. There is #if in function body so the label might be used in code that is removed by the preprocessor. Should this be a 'case' of the enclosing switch()? [unusedLabelSwitchConfiguration]\n",
                      errout_str());
    }

    void testUnusedLabelPremiumMisra() { // #14467 - enable unusedLabel with --premium=misra-c-20xx flag
        Settings s;
        check("void f() {\n"
              "    label:\n"
              "}", dinit(CheckOptions, $.settings = &s));
        ASSERT_EQUALS("", errout_str());
        s.premiumArgs = "--premium=misra-c-2012"; // <- activates unusedLabel checking
        check("void f() {\n"
              "    label:\n"
              "}", dinit(CheckOptions, $.settings = &s));
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Label 'label' is not used. [unusedLabel]\n", errout_str());
    }

    // TODO: only used in a single place
#define checkCustomSettings(...) checkCustomSettings_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkCustomSettings_(const char* file, int line, const char (&code)[size], const Settings& settings, bool cpp = true) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check..
        runChecks<CheckOther>(tokenizer, this);
    }

    void testEvaluationOrder() {
        check("void f() {\n"
              "  int x = dostuff();\n"
              "  return x + x++;\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:12]: (error) Expression 'x+x++' depends on order of evaluation of side effects [unknownEvaluationOrder]\n", errout_str());

        // #7226
        check("long int f1(const char *exp) {\n"
              "  return strtol(++exp, (char **)&exp, 10);\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("long int f1(const char *exp) {\n"
              "  return dostuff(++exp, exp, 10);\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:2:23]: (error) Expression '++exp,exp' depends on order of evaluation of side effects [unknownEvaluationOrder]\n", errout_str());

        check("void f() {\n"
              "  int a;\n"
              "  while (a=x(), a==123) {}\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        // # 8717
        check("void f(int argc, char *const argv[]) {\n"
              "    char **local_argv = safe_malloc(sizeof (*local_argv));\n"
              "    int local_argc = 0;\n"
              "    local_argv[local_argc++] = argv[0];\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int x = 0;\n"
              "  return 0 + x++;\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, int y) {\n"
              "  int a[10];\n"
              "  a[x+y] = a[y+x]++;;\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:10]: (error) Expression 'a[x+y]=a[y+x]++' depends on order of evaluation of side effects [unknownEvaluationOrder]\n", errout_str());

        check("void f(int i) {\n"
              "  int n = ++i + i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (error) Expression '++i+i' depends on order of evaluation of side effects [unknownEvaluationOrder]\n", errout_str());

        check("long int f1(const char *exp) {\n"
              "  return dostuff(++exp, ++exp, 10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:23]: (portability) Expression '++exp,++exp' depends on order of evaluation of side effects. Behavior is Unspecified according to c++17 [unknownEvaluationOrder]\n"
                      "[test.cpp:2:23]: (portability) Expression '++exp,++exp' depends on order of evaluation of side effects. Behavior is Unspecified according to c++17 [unknownEvaluationOrder]\n", errout_str());

        check("void f(int i) {\n"
              "  int n = (~(-(++i)) + i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (error) Expression '~(-(++i))+i' depends on order of evaluation of side effects [unknownEvaluationOrder]\n", errout_str());

        const Settings settings11 = settingsBuilder(settings0).cpp(Standards::CPP11).certainty(Certainty::inconclusive).build();

        checkCustomSettings("void f(int i) {\n"
                            "  i = i++ + 2;\n"
                            "}", settings11);
        ASSERT_EQUALS("[test.cpp:2:11]: (error) Expression 'i+++2' depends on order of evaluation of side effects [unknownEvaluationOrder]\n", errout_str());

        // #14431
        checkCustomSettings("int f(void) {\n"
                            "  struct baz baz = {.bar = {.foo = {.foo = 1}}};\n"
                            "}\n", settings0, false);
        ASSERT_EQUALS("", errout_str());
    }

    void testEvaluationOrderSelfAssignment() {
        // self assignment
        check("void f() {\n"
              "  int x = x = y + 1;\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS(
            "[test.c:2:9]: (style) Redundant assignment of 'x' to itself. [selfAssignment]\n"
            "[test.c:2:9]: (style) Redundant assignment of 'x' to itself. [selfAssignment]\n",   // duplicate
            errout_str());
    }

    void testEvaluationOrderMacro() {
        // macro, don't bailout (#7233)
        checkP("#define X x\n"
               "void f(int x) {\n"
               "  return x + X++;\n"
               "}", dinit(CheckPOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:12]: (error) Expression 'x+x++' depends on order of evaluation of side effects [unknownEvaluationOrder]\n", errout_str());
    }

    void testEvaluationOrderSequencePointsFunctionCall() {
        // FP
        check("void f(int id) {\n"
              "  id = dostuff(id += 42);\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        // FN
        check("void f(int id) {\n"
              "  id = id + dostuff(id += 42);\n"
              "}", dinit(CheckOptions, $.cpp = false));
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

    void testEvaluationOrderSequencePointsComma() {
        check("int f(void) {\n"
              "  int t;\n"
              "  return (unsigned char)(t=1,t^c);\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("void f(void) {\n"
              "  int t;\n"
              "  dostuff(t=1,t^c);\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:14]: (error) Expression 't=1,t^c' depends on order of evaluation of side effects [unknownEvaluationOrder]\n", errout_str());

        check("void f(void) {\n"
              "  int t;\n"
              "  dostuff((t=1,t),2);\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        // #8230
        check("void hprf(const char* fp) {\n"
              "    do\n"
              "        ;\n"
              "    while (++fp, (*fp) <= 0177);\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("void hprf(const char* fp) {\n"
              "    do\n"
              "        ;\n"
              "    while (i++, ++fp, (*fp) <= 0177);\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("void f(const char* fp) {\n"
              "    do\n"
              "        ;\n"
              "    while (f(++fp, (*fp) <= 7));\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:4:18]: (error) Expression '++fp,(*fp)<=7' depends on order of evaluation of side effects [unknownEvaluationOrder]\n", errout_str());
    }

    void testEvaluationOrderSizeof() {
        check("void f(char *buf) {\n"
              "  dostuff(buf++, sizeof(*buf));"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
    }

    void testUnsignedLessThanZero() {
        check("struct d {\n"
              "  unsigned n;\n"
              "};\n"
              "void f(void) {\n"
              "  struct d d;\n"
              "  d.n = 3;\n"
              "\n"
              "  if (d.n < 0) {\n"
              "    return;\n"
              "  }\n"
              "\n"
              "  if (0 > d.n) {\n"
              "    return;\n"
              "  }\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:8:11]: (style) Checking if unsigned expression 'd.n' is less than zero. [unsignedLessThanZero]\n"
                      "[test.c:12:9]: (style) Checking if unsigned expression 'd.n' is less than zero. [unsignedLessThanZero]\n",
                      errout_str());
    }

    void doubleMove1() {
        check("void g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    g(std::move(a));\n"
              "    g(std::move(a));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:17]: (warning) Access of moved variable 'a'. [accessMoved]\n", errout_str());
    }

    void doubleMoveMemberInitialization1() {
        check("class A\n"
              "{\n"
              "    A(B && b)\n"
              "    :b1(std::move(b))\n"
              "    {\n"
              "        b2 = std::move(b);\n"
              "    }\n"
              "    B b1;\n"
              "    B b2;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:24]: (warning) Access of moved variable 'b'. [accessMoved]\n", errout_str());
    }

    void doubleMoveMemberInitialization2() {
        check("class A\n"
              "{\n"
              "    A(B && b)\n"
              "    :b1(std::move(b)),\n"
              "     b2(std::move(b))\n"
              "    {}\n"
              "    B b1;\n"
              "    B b2;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:19]: (warning) Access of moved variable 'b'. [accessMoved]\n", errout_str());
    }

    void doubleMoveMemberInitialization3() { // #9974
        check("struct A { int i; };\n"
              "struct B { A a1; A a2; };\n"
              "B f() {\n"
              "    A a1 = { 1 };\n"
              "    A a2 = { 2 };\n"
              "    return { .a1 = std::move(a1), .a2 = std::move(a2) };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void doubleMoveMemberInitialization4() { // #11440
        check("struct S { void f(int); };\n"
              "struct T {\n"
              "    T(int c, S&& d) : c{ c }, d{ std::move(d) } { d.f(c); }\n"
              "    int c;\n"
              "    S d;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:3:51]: (warning, inconclusive) Access of moved variable 'd'. [accessMoved]\n", errout_str());
    }

    void moveAndAssign1() {
        check("A g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    a = g(std::move(a));\n"
              "    a = g(std::move(a));\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void moveAndAssign2() {
        check("A g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    B b = g(std::move(a));\n"
              "    C c = g(std::move(a));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (warning) Access of moved variable 'a'. [accessMoved]\n", errout_str());
    }

    void moveAssignMoveAssign() {
        check("void h(A a);\n"
              "void f() {"
              "    A a;\n"
              "    g(std::move(a));\n"
              "    h(a);\n"
              "    a = b;\n"
              "    h(a);\n"
              "    g(std::move(a));\n"
              "    h(a);\n"
              "    a = b;\n"
              "    h(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (warning) Access of moved variable 'a'. [accessMoved]\n"
                      "[test.cpp:8:7]: (warning) Access of moved variable 'a'. [accessMoved]\n", errout_str());
    }

    void moveAndReset1() {
        check("A g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    a.reset(g(std::move(a)));\n"
              "    a.reset(g(std::move(a)));\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void moveAndReset2() {
        check("A g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    A b;\n"
              "    A c;\n"
              "    b.reset(g(std::move(a)));\n"
              "    c.reset(g(std::move(a)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:25]: (warning) Access of moved variable 'a'. [accessMoved]\n", errout_str());
    }

    void moveResetMoveReset() {
        check("void h(A a);\n"
              "void f() {"
              "    A a;\n"
              "    g(std::move(a));\n"
              "    h(a);\n"
              "    a.reset(b);\n"
              "    h(a);\n"
              "    g(std::move(a));\n"
              "    h(a);\n"
              "    a.reset(b);\n"
              "    h(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (warning) Access of moved variable 'a'. [accessMoved]\n"
                      "[test.cpp:8:7]: (warning) Access of moved variable 'a'. [accessMoved]\n", errout_str());
    }

    void moveAndFunctionParameter() {
        check("void g(A a);\n"
              "void f() {\n"
              "    A a;\n"
              "    A b = std::move(a);\n"
              "    g(a);\n"
              "    A c = a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:7]: (warning) Access of moved variable 'a'. [accessMoved]\n"
                      "[test.cpp:6:11]: (warning) Access of moved variable 'a'. [accessMoved]\n", errout_str());
    }

    void moveAndFunctionParameterReference() {
        check("void g(A & a);\n"
              "void f() {\n"
              "    A a;\n"
              "    A b = std::move(a);\n"
              "    g(a);\n"
              "    A c = a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void moveAndFunctionParameterConstReference() {
        check("void g(A const & a);\n"
              "void f() {\n"
              "    A a;\n"
              "    A b = std::move(a);\n"
              "    g(a);\n"
              "    A c = a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:7]: (warning) Access of moved variable 'a'. [accessMoved]\n"
                      "[test.cpp:6:11]: (warning) Access of moved variable 'a'. [accessMoved]\n", errout_str());
    }

    void moveAndFunctionParameterUnknown() {
        check("void f() {\n"
              "    A a;\n"
              "    A b = std::move(a);\n"
              "    g(a);\n"
              "    A c = a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (warning, inconclusive) Access of moved variable 'a'. [accessMoved]\n"
                      "[test.cpp:5:11]: (warning, inconclusive) Access of moved variable 'a'. [accessMoved]\n", errout_str());
    }

    void moveAndReturn() {
        check("int f(int i) {\n"
              "    A a;\n"
              "    A b;\n"
              "    g(std::move(a));\n"
              "    if (i)\n"
              "        return g(std::move(b));\n"
              "    return h(std::move(a),std::move(b));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:24]: (warning) Access of moved variable 'a'. [accessMoved]\n", errout_str());
    }

    void moveAndClear() {
        check("void f() {\n"
              "    V v;\n"
              "    g(std::move(v));\n"
              "    v.clear();\n"
              "    if (v.empty()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void movedPointer() {
        check("void f() {\n"
              "    P p;\n"
              "    g(std::move(p));\n"
              "    x = p->x;\n"
              "    y = p->y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (warning) Access of moved variable 'p'. [accessMoved]\n"
                      "[test.cpp:5:9]: (warning) Access of moved variable 'p'. [accessMoved]\n", errout_str());
    }

    void moveAndAddressOf() {
        check("void f() {\n"
              "    std::string s1 = x;\n"
              "    std::string s2 = std::move(s1);\n"
              "    p = &s1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void partiallyMoved() {
        check("void f() {\n"
              "    A a;\n"
              "    gx(std::move(a).x());\n"
              "    gy(std::move(a).y());\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void moveAndLambda() {
        check("void f() {\n"
              "    A a;\n"
              "    auto h = [a=std::move(a)](){return g(std::move(a));};"
              "    b = a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void moveInLoop()
    {
        check("void g(std::string&& s);\n"
              "void f() {\n"
              "    std::string p;\n"
              "    while(true)\n"
              "        g(std::move(p));\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:21]: (warning) Access of moved variable 'p'. [accessMoved]\n", errout_str());

        check("std::list<int> g(std::list<int>&&);\n"
              "void f(std::list<int>l) {\n"
              "    for(int i = 0; i < 10; ++i) {\n"
              "        for (auto &j : g(std::move(l))) { (void)j; }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:36]: (warning) Access of moved variable 'l'. [accessMoved]\n", errout_str());

        check("struct S {\n" // #13179
              "    operator bool() const { return !m.empty(); }\n"
              "    std::string m;\n"
              "};\n"
              "S get();\n"
              "void set(S);\n"
              "void f() {\n"
              "    while (S s = get()) {\n"
              "        set(std::move(s));\n"
              "    }\n"
              "}\n"
              "void g() {\n"
              "    while (S s{ get() }) {\n"
              "        set(std::move(s));\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void moveCallback()
    {
        check("bool f(std::function<void()>&& callback);\n"
              "void func(std::function<void()> callback) {\n"
              "    if(!f(std::move(callback)))\n"
              "        callback();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:9]: (warning) Access of moved variable 'callback'. [accessMoved]\n", errout_str());
    }

    void moveClassVariable()
    {
        check("struct B {\n"
              "    virtual void f();\n"
              "};\n"
              "struct D : B {\n"
              "    void f() override {\n"
              "        auto p = std::unique_ptr<D>(new D(std::move(m)));\n"
              "    }\n"
              "    D(std::unique_ptr<int> c) : m(std::move(c)) {}\n"
              "    std::unique_ptr<int> m;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void forwardAndUsed() {
        check("template<typename T>\n"
              "void f(T && t) {\n"
              "    g(std::forward<T>(t));\n"
              "    T s = t;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (warning) Access of forwarded variable 't'. [accessForwarded]\n", errout_str());
    }

    void moveAndReference() { // #9791
        check("void g(std::string&&);\n"
              "void h(const std::string&);\n"
              "void f() {\n"
              "    std::string s;\n"
              "    const std::string& r = s;\n"
              "    g(std::move(s));\n"
              "    h(r);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:7]: (warning) Access of moved variable 'r'. [accessMoved]\n", errout_str());
    }

    void moveForRange()
    {
        check("struct C {\n"
              "    void f() {\n"
              "        for (auto r : mCategory.find(std::move(mWhere))) {}\n"
              "    }\n"
              "    cif::category mCategory;\n"
              "    cif::condition mWhere;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void moveTernary()
    {
        check("void gA(std::string);\n" // #12174
              "void gB(std::string);\n"
              "void f(bool b) {\n"
              "    std::string s = \"abc\";\n"
              "    b ? gA(std::move(s)) : gB(std::move(s));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int gA(std::string);\n"
              "int gB(std::string);\n"
              "void h(int);\n"
              "void f(bool b) {\n"
              "    std::string s = \"abc\";\n"
              "    h(b ? gA(std::move(s)) : gB(std::move(s)));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int gA(int, std::string);\n"
              "int gB(int, std::string);\n"
              "int h(int);\n"
              "void f(bool b) {\n"
              "    std::string s = \"abc\";\n"
              "    h(b ? h(gA(5, std::move(s))) : h(gB(7, std::move(s))));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void movePointerAlias()
    {
        check("void f() {\n"
              "    std::string s;\n"
              "    std::string s1 = std::move(s);\n"
              "    const std::string* s_p = &s;\n"
              "    s_p->size();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:8]: (warning) Access of moved variable '.'. [accessMoved]\n", errout_str());
    }

    void moveOutparam()
    {
        check("void f(std::vector<std::string>& v) {\n" // #11300
              "    std::string l;\n"
              "    while (std::getline(std::cin, l)) {\n"
              "        if (!l.empty()) {\n"
              "            v.emplace_back(std::move(l));\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::ifstream& fin, std::set<std::string>& s) {\n"
              "    std::string line;\n"
              "    while (std::getline(fin, line)) {\n"
              "        s.emplace(std::move(line));\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void moveTryEmplace()
    {
        check("void f(std::map<std::string, std::string>& m, std::string& s) {\n" // #12773
              "    bool b = m.try_emplace(\"a\", std::move(s)).second;\n"
              "    if (!b) {\n"
              "        std::cout << s;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void funcArgNamesDifferent() {
        check("void func1(int a, int b, int c);\n"
              "void func1(int a, int b, int c) { }\n"
              "void func2(int a, int b, int c);\n"
              "void func2(int A, int B, int C) { }\n"
              "class Fred {\n"
              "    void func1(int a, int b, int c);\n"
              "    void func2(int a, int b, int c);\n"
              "    void func3(int a = 0, int b = 0, int c = 0);\n"
              "    void func4(int a = 0, int b = 0, int c = 0);\n"
              "};\n"
              "void Fred::func1(int a, int b, int c) { }\n"
              "void Fred::func2(int A, int B, int C) { }\n"
              "void Fred::func3(int a, int b, int c) { }\n"
              "void Fred::func4(int A, int B, int C) { }");
        ASSERT_EQUALS("[test.cpp:3:16] -> [test.cpp:4:16]: (style, inconclusive) Function 'func2' argument 1 names different: declaration 'a' definition 'A'. [funcArgNamesDifferent]\n"
                      "[test.cpp:3:23] -> [test.cpp:4:23]: (style, inconclusive) Function 'func2' argument 2 names different: declaration 'b' definition 'B'. [funcArgNamesDifferent]\n"
                      "[test.cpp:3:30] -> [test.cpp:4:30]: (style, inconclusive) Function 'func2' argument 3 names different: declaration 'c' definition 'C'. [funcArgNamesDifferent]\n"
                      "[test.cpp:7:20] -> [test.cpp:12:22]: (style, inconclusive) Function 'func2' argument 1 names different: declaration 'a' definition 'A'. [funcArgNamesDifferent]\n"
                      "[test.cpp:7:27] -> [test.cpp:12:29]: (style, inconclusive) Function 'func2' argument 2 names different: declaration 'b' definition 'B'. [funcArgNamesDifferent]\n"
                      "[test.cpp:7:34] -> [test.cpp:12:36]: (style, inconclusive) Function 'func2' argument 3 names different: declaration 'c' definition 'C'. [funcArgNamesDifferent]\n"
                      "[test.cpp:9:20] -> [test.cpp:14:22]: (style, inconclusive) Function 'func4' argument 1 names different: declaration 'a' definition 'A'. [funcArgNamesDifferent]\n"
                      "[test.cpp:9:31] -> [test.cpp:14:29]: (style, inconclusive) Function 'func4' argument 2 names different: declaration 'b' definition 'B'. [funcArgNamesDifferent]\n"
                      "[test.cpp:9:42] -> [test.cpp:14:36]: (style, inconclusive) Function 'func4' argument 3 names different: declaration 'c' definition 'C'. [funcArgNamesDifferent]\n", errout_str());

        check("using F1 = void (*)();\n" // #14633
              "void f(F1 a);\n"
              "void f(F1 b) {}\n"
              "typedef void (*F2)();\n"
              "void g(F2 a);\n"
              "void g(F2 b) {}\n"
              "void h(void (*a)());\n"
              "void h(void (*b)()) {}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:11] -> [test.cpp:3:11]: (style, inconclusive) Function 'f' argument 1 names different: declaration 'a' definition 'b'. [funcArgNamesDifferent]\n"
            "[test.cpp:5:11] -> [test.cpp:6:11]: (style, inconclusive) Function 'g' argument 1 names different: declaration 'a' definition 'b'. [funcArgNamesDifferent]\n"
            "[test.cpp:7:15] -> [test.cpp:8:15]: (style, inconclusive) Function 'h' argument 1 names different: declaration 'a' definition 'b'. [funcArgNamesDifferent]\n",
            errout_str());

        check("void f(int a);\n" // #14632
              "void f(int) {}\n"
              "void g(int);\n"
              "void g(int b) {}\n"
              "void h(int);\n"
              "void h(int) {}\n");
        ASSERT_EQUALS(
            "[test.cpp:1:12]: (style, inconclusive) Function 'f' argument 1 names different: declaration 'a' definition '<unnamed>'. [funcArgNamesDifferentUnnamed]\n"
            "[test.cpp:4:12]: (style, inconclusive) Function 'g' argument 1 names different: declaration '<unnamed>' definition 'b'. [funcArgNamesDifferentUnnamed]\n",
            errout_str());
    }

    void funcArgOrderDifferent() {
        check("void func1(int a, int b, int c);\n"
              "void func1(int a, int b, int c) { }\n"
              "void func2(int a, int b, int c);\n"
              "void func2(int c, int b, int a) { }\n"
              "void func3(int, int b, int c);\n"
              "void func3(int c, int b, int a) { }\n"
              "class Fred {\n"
              "    void func1(int a, int b, int c);\n"
              "    void func2(int a, int b, int c);\n"
              "    void func3(int a = 0, int b = 0, int c = 0);\n"
              "    void func4(int, int b = 0, int c = 0);\n"
              "};\n"
              "void Fred::func1(int a, int b, int c) { }\n"
              "void Fred::func2(int c, int b, int a) { }\n"
              "void Fred::func3(int c, int b, int a) { }\n"
              "void Fred::func4(int c, int b, int a) { }\n",
              dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:3:16] -> [test.cpp:4:16]: (warning) Function 'func2' argument order different: declaration 'a, b, c' definition 'c, b, a' [funcArgOrderDifferent]\n"
                      "[test.cpp:5:12] -> [test.cpp:6:16]: (warning) Function 'func3' argument order different: declaration ', b, c' definition 'c, b, a' [funcArgOrderDifferent]\n"
                      "[test.cpp:9:20] -> [test.cpp:14:22]: (warning) Function 'func2' argument order different: declaration 'a, b, c' definition 'c, b, a' [funcArgOrderDifferent]\n"
                      "[test.cpp:10:20] -> [test.cpp:15:22]: (warning) Function 'func3' argument order different: declaration 'a, b, c' definition 'c, b, a' [funcArgOrderDifferent]\n"
                      "[test.cpp:11:16] -> [test.cpp:16:22]: (warning) Function 'func4' argument order different: declaration ', b, c' definition 'c, b, a' [funcArgOrderDifferent]\n", errout_str());
    }

    // #7846 - Syntax error when using C++11 braced-initializer in default argument
    void cpp11FunctionArgInit() {
        // syntax error is not expected
        ASSERT_NO_THROW(check("\n void foo(int declaration = {}) {"
                              "\n   for (int i = 0; i < 10; i++) {}\n"
                              "\n }"
                              "\n  "));
        ASSERT_EQUALS("", errout_str());
    }

    void shadowVariables() {
        check("int x;\n"
              "void f() { int x; }");
        ASSERT_EQUALS("[test.cpp:1:5] -> [test.cpp:2:16]: (style) Local variable 'x' shadows outer variable [shadowVariable]\n", errout_str());

        check("int x();\n"
              "void f() { int x; }");
        ASSERT_EQUALS("[test.cpp:1:5] -> [test.cpp:2:16]: (style) Local variable 'x' shadows outer function [shadowFunction]\n", errout_str());

        check("struct C {\n"
              "    C(int x) : x(x) {}\n" // <- we do not want a FP here
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  if (cond) {int x;}\n" // <- not a shadow variable
              "  int x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int size() {\n"
              "  int size;\n" // <- not a shadow variable
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #8954 - lambda
              "  int x;\n"
              "  auto f = [](){ int x; }"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) { int x; }");
        ASSERT_EQUALS("[test.cpp:1:12] -> [test.cpp:1:21]: (style) Local variable 'x' shadows outer argument [shadowArgument]\n", errout_str());

        check("class C { C(); void foo() { static int C = 0; } }"); // #9195 - shadow constructor
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n" // #10091 - shadow destructor
              "    ~C();\n"
              "    void f() {\n"
              "        bool C{};\n"
              "    }\n"
              "};\n"
              "C::~C() = default;");
        ASSERT_EQUALS("", errout_str());

        // 10752 - no
        check("struct S {\n"
              "    int i;\n"
              "\n"
              "    static int foo() {\n"
              "        int i = 0;\n"
              "        return i;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    int i{};\n"
              "    void f() { int i; }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:20]: (style) Local variable 'i' shadows outer variable [shadowVariable]\n", errout_str());

        check("struct S {\n"
              "    int i{};\n"
              "    std::vector<int> v;\n"
              "    void f() const { for (const int& i : v) {} }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:4:38]: (style) Local variable 'i' shadows outer variable [shadowVariable]\n", errout_str());

        check("struct S {\n" // #10405
              "    F* f{};\n"
              "    std::list<F> fl;\n"
              "    void S::f() const;\n"
              "};\n"
              "void S::f() const {\n"
              "    for (const F& f : fl) {}\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:2:8] -> [test.cpp:7:19]: (style) Local variable 'f' shadows outer variable [shadowVariable]\n", errout_str());

        check("extern int a;\n"
              "int a;\n"
              "static int f(void) {\n"
              "    int a;\n"
              "    return 0;\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:1:12] -> [test.c:4:9]: (style) Local variable 'a' shadows outer variable [shadowVariable]\n", errout_str());

        check("int f() {\n" // #12591
              "    int g = 0;\n"
              "    return g;\n"
              "}\n"
              "int g() { return 1; }\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #13888
              "    int i;\n"
              "    friend int f() { int i = 5; return i; }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void knownArgument() {
        check("void g(int);\n"
              "void f(int x) {\n"
              "   g((x & 0x01) >> 7);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Argument '(x&0x01)>>7' to function g is always 0. It does not matter what value 'x' has. [knownArgument]\n", errout_str());

        check("void g(int);\n"
              "void f(int x) {\n"
              "   g((int)((x & 0x01) >> 7));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (style) Argument '(int)((x&0x01)>>7)' to function g is always 0. It does not matter what value 'x' has. [knownArgument]\n", errout_str());

        check("void g(int, int);\n"
              "void f(int x) {\n"
              "   g(x, (x & 0x01) >> 7);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:20]: (style) Argument '(x&0x01)>>7' to function g is always 0. It does not matter what value 'x' has. [knownArgument]\n",
            errout_str());

        check("void g(int);\n"
              "void f(int x) {\n"
              "    g(0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int);\n"
              "void h() { return 1; }\n"
              "void f(int x) {\n"
              "    g(h());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int);\n"
              "void f(int x) {\n"
              "    g(std::strlen(\"a\"));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int);\n"
              "void f(int x) {\n"
              "    g((int)0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(Foo *);\n"
              "void f() {\n"
              "    g(reinterpret_cast<Foo*>(0));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int);\n"
              "void f(int x) {\n"
              "    x = 0;\n"
              "    g(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int);\n"
              "void f() {\n"
              "    const int x = 0;\n"
              "    g(x + 1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int);\n"
              "void f() {\n"
              "    char i = 1;\n"
              "    g(static_cast<int>(i));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("char *yytext;\n"
              "void re_init_scanner() {\n"
              "  int size = 256;\n"
              "  yytext = xmalloc(size * sizeof *yytext);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const char *c) {\n"
              "    if (*c == '+' && (operand || !isalnum(*c))) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8986
        check("void f(int);\n"
              "void g() {\n"
              "    const int x[] = { 10, 10 };\n"
              "    f(x[0]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int);\n"
              "void g() {\n"
              "    int x[] = { 10, 10 };\n"
              "    f(x[0]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Variable 'x' can be declared as const array [constVariable]\n", errout_str());

        check("struct A { int x; };"
              "void g(int);\n"
              "void f(int x) {\n"
              "    A y;\n"
              "    y.x = 1;\n"
              "    g(y.x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // allow known argument value in assert call
        check("void g(int);\n"
              "void f(int x) {\n"
              "   ASSERT((int)((x & 0x01) >> 7));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9905 - expression that does not use integer calculation at all
        check("void foo() {\n"
              "    const std::string heading = \"Interval\";\n"
              "    std::cout << std::setw(heading.length());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9909 - struct member with known value
        check("struct LongStack {\n"
              "    int maxsize;\n"
              "};\n"
              "\n"
              "void growLongStack(LongStack* self) {\n"
              "    self->maxsize = 32;\n"
              "    dostuff(self->maxsize * sizeof(intptr_t));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #11894
        check("struct S {\n"
              "    int *p, n;\n"
              "};\n"
              "S* g() {\n"
              "    S* s = static_cast<S*>(calloc(1, sizeof(S)));\n"
              "    s->n = 100;\n"
              "    s->p = static_cast<int*>(malloc(s->n * sizeof(int)));\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11679
        check("bool g(int);\n"
              "void h(int);\n"
              "int k(int a) { h(a); return 0; }\n"
              "void f(int i) {\n"
              "    if (g(k(i))) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11889
        check("struct S {\n"
              "    int a[5];\n"
              "    void f(int i);\n"
              "}\n"
              "void g(int);\n"
              "void S::f(int i) {\n"
              "    if (a[i] == 1) {\n"
              "        a[i] = 0;\n"
              "        g(a[i]);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11927
        check("void f(func_t func, int i) {\n"
              "    (func)(i, 0);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { void operator()(int, int); };\n"
              "void f(int i) {\n"
              "    S()(i, 1);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int& r) {\n"
              "    g(static_cast<char>(r = 42));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n"
              "void f(int i) {\n"
              "    const int a[] = { i - 1 * i, 0 };\n"
              "    auto s = S{ i - 1 * i };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:25]: (style) Argument 'i-1*i' to init list { is always 0. It does not matter what value 'i' has. [knownArgument]\n"
                      "[test.cpp:4:19]: (style) Argument 'i-1*i' to constructor S is always 0. It does not matter what value 'i' has. [knownArgument]\n",
                      errout_str());

        checkP("#define MACRO(X) std::abs(X ? 0 : a)\n"
               "int f(int a) {\n"
               "    return MACRO(true);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void knownArgumentHiddenVariableExpression() {
        // #9914 - variable expression is explicitly hidden
        check("void f(int x) {\n"
              "    dostuff(x && false);\n"
              "    dostuff(false && x);\n"
              "    dostuff(x || true);\n"
              "    dostuff(true || x);\n"
              "    dostuff(x * 0);\n"
              "    dostuff(0 * x);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:19]: (style) Argument 'false&&x' to function dostuff is always 0. Constant literal calculation disable/hide variable expression 'x'. [knownArgumentHiddenVariableExpression]\n"
                      "[test.cpp:5:18]: (style) Argument 'true||x' to function dostuff is always 1. Constant literal calculation disable/hide variable expression 'x'. [knownArgumentHiddenVariableExpression]\n"
                      "[test.cpp:6:15]: (style) Argument 'x*0' to function dostuff is always 0. Constant literal calculation disable/hide variable expression 'x'. [knownArgumentHiddenVariableExpression]\n"
                      "[test.cpp:7:15]: (style) Argument '0*x' to function dostuff is always 0. Constant literal calculation disable/hide variable expression 'x'. [knownArgumentHiddenVariableExpression]\n", errout_str());
    }

    void knownArgumentTernaryOperator() { // #10374
        check("void f(bool a, bool b) {\n"
              "    const T* P = nullptr; \n"
              "    long N = 0; \n"
              "    const bool c = foo(); \n"
              "    bar(P, N); \n"
              "    if (c ? a : b)\n"
              "      baz(P, N); \n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkComparePointers() {
        check("int f() {\n"
              "    const int foo[1] = {0};\n"
              "    const int bar[1] = {0};\n"
              "    int diff = 0;\n"
              "    if(foo > bar) {\n"
              "       diff = 1;\n"
              "    }\n"
              "    return diff;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:15] -> [test.cpp:5:8] -> [test.cpp:3:15] -> [test.cpp:5:14] -> [test.cpp:5:12]: (error) Comparing pointers that point to different objects [comparePointers]\n",
            errout_str());

        check("bool f() {\n"
              "    int x = 0;\n"
              "    int y = 0;\n"
              "    int* xp = &x;\n"
              "    int* yp = &y;\n"
              "    return xp > yp;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:9] -> [test.cpp:4:15] -> [test.cpp:3:9] -> [test.cpp:5:15] -> [test.cpp:6:15]: (error) Comparing pointers that point to different objects [comparePointers]\n"
            "[test.cpp:4:10]: (style) Variable 'xp' can be declared as pointer to const [constVariablePointer]\n"
            "[test.cpp:5:10]: (style) Variable 'yp' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("bool f() {\n"
              "    int x = 0;\n"
              "    int y = 1;\n"
              "    return &x > &y;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:9] -> [test.cpp:4:12] -> [test.cpp:3:9] -> [test.cpp:4:17] -> [test.cpp:4:15]: (error) Comparing pointers that point to different objects [comparePointers]\n",
            errout_str());

        check("struct A {int data;};\n"
              "bool f() {\n"
              "    A x;\n"
              "    A y;\n"
              "    int* xp = &x.data;\n"
              "    int* yp = &y.data;\n"
              "    return xp > yp;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:7] -> [test.cpp:5:15] -> [test.cpp:4:7] -> [test.cpp:6:15] -> [test.cpp:7:15]: (error) Comparing pointers that point to different objects [comparePointers]\n"
            "[test.cpp:5:10]: (style) Variable 'xp' can be declared as pointer to const [constVariablePointer]\n"
            "[test.cpp:6:10]: (style) Variable 'yp' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("struct A {int data;};\n"
              "bool f(A ix, A iy) {\n"
              "    A* x = &ix;\n"
              "    A* y = &iy;\n"
              "    int* xp = &x->data;\n"
              "    int* yp = &y->data;\n"
              "    return xp > yp;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:10] -> [test.cpp:3:12] -> [test.cpp:5:15] -> [test.cpp:2:16] -> [test.cpp:4:12] -> [test.cpp:6:15] -> [test.cpp:7:15]: (error) Comparing pointers that point to different objects [comparePointers]\n"
            "[test.cpp:5:10]: (style) Variable 'xp' can be declared as pointer to const [constVariablePointer]\n"
            "[test.cpp:6:10]: (style) Variable 'yp' can be declared as pointer to const [constVariablePointer]\n",
            errout_str());

        check("bool f(int * xp, int* yp) {\n"
              "    return &xp > &yp;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:1:14] -> [test.cpp:2:12] -> [test.cpp:1:23] -> [test.cpp:2:18] -> [test.cpp:2:16]: (error) Comparing pointers that point to different objects [comparePointers]\n",
            errout_str());

        check("int f() {\n"
              "    int x = 0;\n"
              "    int y = 1;\n"
              "    return &x - &y;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:9] -> [test.cpp:4:12] -> [test.cpp:3:9] -> [test.cpp:4:17] -> [test.cpp:4:15]: (error) Subtracting pointers that point to different objects [subtractPointers]\n",
            errout_str());

        check("bool f() {\n"
              "    int x[2] = {1, 2}m;\n"
              "    int* xp = &x[0];\n"
              "    int* yp = &x[1];\n"
              "    return xp > yp;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Variable 'xp' can be declared as pointer to const [constVariablePointer]\n"
                      "[test.cpp:4:10]: (style) Variable 'yp' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("bool f(const int * xp, const int* yp) {\n"
              "    return xp > yp;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(const int & x, const int& y) {\n"
              "    return &x > &y;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int& g();\n"
              "bool f() {\n"
              "    const int& x = g();\n"
              "    const int& y = g();\n"
              "    const int* xp = &x;\n"
              "    const int* yp = &y;\n"
              "    return xp > yp;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {int data;};\n"
              "bool f(A ix) {\n"
              "    A* x = &ix;\n"
              "    A* y = x;\n"
              "    int* xp = &x->data;\n"
              "    int* yp = &y->data;\n"
              "    return xp > yp;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (style) Variable 'xp' can be declared as pointer to const [constVariablePointer]\n"
                      "[test.cpp:6:10]: (style) Variable 'yp' can be declared as pointer to const [constVariablePointer]\n",
                      errout_str());

        check("struct S { int i; };\n" // #11576
              "int f(S s) {\n"
              "    return &s.i - (int*)&s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:19]: (warning) Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast [dangerousTypeCast]\n", errout_str());

        check("struct S { int i; };\n"
              "int f(S s1, S s2) {\n"
              "    return &s1.i - reinterpret_cast<int*>(&s2);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:12] -> [test.cpp:2:15] -> [test.cpp:3:43] -> [test.cpp:3:18]: (error) Subtracting pointers that point to different objects [subtractPointers]\n",
                      errout_str());

        check("struct S { int a; int b; };\n" // #12422
              "int f() {\n"
              "    S s;\n"
              "    return &s.b - &s.a;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void unusedVariableValueTemplate() {
        check("#include <functional>\n"
              "class A\n"
              "{\n"
              "public:\n"
              "    class Hash\n"
              "    {\n"
              "    public:\n"
              "        std::size_t operator()(const A& a) const\n"
              "        {\n"
              "            (void)a;\n"
              "            return 0;\n"
              "        }\n"
              "    };\n"
              "};\n"
              "namespace std\n"
              "{\n"
              "    template <>\n"
              "    struct hash<A>\n"
              "    {\n"
              "        std::size_t operator()(const A& a) const noexcept\n"
              "        {\n"
              "            return A::Hash{}(a);\n"
              "        }\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void moduloOfOne() {
        check("void f(unsigned int x) {\n"
              "  int y = x % 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Modulo of one is always equal to zero [moduloofone]\n", errout_str());

        check("void f() {\n"
              "  for (int x = 1; x < 10; x++) {\n"
              "    int y = 100 % x;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i, int j) {\n" // #11191
              "    const int c = pow(2, i);\n"
              "    if (j % c) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int i = 0;\n"
              "    i %= 1;\n"
              "    (void)i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) Modulo of one is always equal to zero [moduloofone]\n", errout_str());
    }

    void sameExpressionPointers() {
        check("int f(int *i);\n"
              "void g(int *a, const int *b) {\n"
              "    int c = *a;\n"
              "    f(a);\n"
              "    if (b && c != *a) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void checkOverlappingWrite() {
        // union
        check("void foo() {\n"
              "    union { int i; float f; } u;\n"
              "    u.i = 0;\n"
              "    u.i = u.f;\n" // <- error
              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Overlapping read/write of union is undefined behavior [overlappingWriteUnion]\n", errout_str());

        check("void foo() {\n" // #11013
              "    union { struct { uint8_t a; uint8_t b; }; uint16_t c; } u;\n"
              "    u.a = u.b = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // memcpy
        check("void foo() {\n"
              "    char a[10];\n"
              "    memcpy(&a[5], &a[4], 2u);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Overlapping read/write in memcpy() is undefined behavior [overlappingWriteFunction]\n", errout_str());

        check("void foo() {\n"
              "    char a[10];\n"
              "    memcpy(a+5, a+4, 2u);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Overlapping read/write in memcpy() is undefined behavior [overlappingWriteFunction]\n", errout_str());

        check("void foo() {\n"
              "    char a[10];\n"
              "    memcpy(a, a+1, 2u);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Overlapping read/write in memcpy() is undefined behavior [overlappingWriteFunction]\n", errout_str());

        check("void foo() {\n"
              "    char a[8];\n"
              "    memcpy(&a[0], &a[4], 4u);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("_Bool a[10];\n" // #10350
              "void foo() {\n"
              "    memcpy(&a[5], &a[4], 2u * sizeof(a[0]));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Overlapping read/write in memcpy() is undefined behavior [overlappingWriteFunction]\n", errout_str());

        check("int K[2];\n" // #12638
              "void f(int* p) {\n"
              "    memcpy(&K[0], &K[1], sizeof(K[0]));\n"
              "    memcpy(&K[1], &K[0], sizeof(K[0]));\n"
              "    memcpy(p, p + 1, sizeof(*p));\n"
              "    memcpy(p + 1, p, sizeof(*p));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int K[2];\n"
              "void f(int* p) {\n"
              "    memcpy(&K[0], &K[1], 2 * sizeof(K[0]));\n"
              "    memcpy(&K[1], &K[0], 2 *sizeof(K[0]));\n"
              "    memcpy(p, p + 1, 2 * sizeof(*p));\n"
              "    memcpy(p + 1, p, 2 * sizeof(*p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Overlapping read/write in memcpy() is undefined behavior [overlappingWriteFunction]\n"
                      "[test.cpp:4:5]: (error) Overlapping read/write in memcpy() is undefined behavior [overlappingWriteFunction]\n"
                      "[test.cpp:5:5]: (error) Overlapping read/write in memcpy() is undefined behavior [overlappingWriteFunction]\n"
                      "[test.cpp:6:5]: (error) Overlapping read/write in memcpy() is undefined behavior [overlappingWriteFunction]\n",
                      errout_str());

        // wmemcpy
        check("void foo() {\n"
              "    wchar_t a[10];\n"
              "    wmemcpy(&a[5], &a[4], 2u);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Overlapping read/write in wmemcpy() is undefined behavior [overlappingWriteFunction]\n", errout_str());

        check("void foo() {\n"
              "    wchar_t a[10];\n"
              "    wmemcpy(a+5, a+4, 2u);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Overlapping read/write in wmemcpy() is undefined behavior [overlappingWriteFunction]\n", errout_str());

        check("void foo() {\n"
              "    wchar_t a[10];\n"
              "    wmemcpy(a, a+1, 2u);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Overlapping read/write in wmemcpy() is undefined behavior [overlappingWriteFunction]\n", errout_str());

        // strcpy
        check("void foo(char *ptr) {\n"
              "    strcpy(ptr, ptr);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (error) Overlapping read/write in strcpy() is undefined behavior [overlappingWriteFunction]\n", errout_str());
    }

    void constVariableArrayMember() { // #10371
        check("class Foo {\n"
              "public:\n"
              "    Foo();\n"
              "    int GetVal() const { return m_Arr[0]; }\n"
              "    int m_Arr[1];\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void knownPointerToBool()
    {
        check("void g(bool);\n"
              "void f() {\n"
              "    int i = 5;\n"
              "    int* p = &i;\n"
              "    g(p);\n"
              "    g(&i);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:7]: (style) Pointer expression 'p' converted to bool is always true. [knownPointerToBool]\n"
                      "[test.cpp:6:7]: (style) Pointer expression '&i' converted to bool is always true. [knownPointerToBool]\n",
                      errout_str());

        check("void f() {\n"
              "    const int* x = nullptr;\n"
              "    std::empty(x);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    const int* x = nullptr;\n"
              "    std::empty(const_cast<int*>(x));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A { bool x; };\n" // #14481
              "bool f(A* a) {\n"
              "    if (a) {\n"
              "        return a->x;\n"
              "    }\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Parameter 'a' can be declared as pointer to const [constParameterPointer]\n", errout_str());

        check("struct A { int* x; };\n"
              "bool f(A a) {\n"
              "    if (a.x) {\n"
              "        return a.x;\n"
              "    }\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:17]: (style) Pointer expression 'a.x' converted to bool is always true. [knownPointerToBool]\n", errout_str());

        check("void f(bool* b) { if (b) *b = true; }");
        ASSERT_EQUALS("", errout_str());

        check("bool f() {\n"
              "    int* x = nullptr;\n"
              "    return bool(x);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Pointer expression 'x' converted to bool is always false. [knownPointerToBool]\n", errout_str());

        check("bool f() {\n"
              "    int* x = nullptr;\n"
              "    return bool{x};\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Pointer expression 'x' converted to bool is always false. [knownPointerToBool]\n", errout_str());

        check("struct A { A(bool); };\n"
              "A f() {\n"
              "    int* x = nullptr;\n"
              "    return A(x);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:14]: (style) Pointer expression 'x' converted to bool is always false. [knownPointerToBool]\n", errout_str());

        check("struct A { A(bool); };\n"
              "A f() {\n"
              "    int* x = nullptr;\n"
              "    return A{x};\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:14]: (style) Pointer expression 'x' converted to bool is always false. [knownPointerToBool]\n", errout_str());

        check("struct B { virtual void f() {} };\n" // #11929
              "struct D : B {};\n"
              "void g(B* b) {\n"
              "    if (!b)\n"
              "        return;\n"
              "    if (dynamic_cast<D*>(b)) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool (*ptr)();\n" // #12170
              "void f() {\n"
              "    if (!ptr || !ptr()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(bool b[2]);\n" // #12822
              "void f() {\n"
              "    bool b[2] = {};\n"
              "    g(b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterateByValue() {
        check("void f() {\n" // #9684
              "    const std::set<std::string> ss = { \"a\", \"b\", \"c\" };\n"
              "    for (auto s : ss)\n"
              "        (void)s.size();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:15]: (performance) Range variable 's' should be declared as const reference. [iterateByValue]\n",
                      errout_str());
        check("void f() {\n" // #13696
              "    struct T {\n"
              "        std::string name;\n"
              "        UnknownClass member;\n"
              "    };\n"
              "\n"
              "    const std::set<T> ss;\n"
              "    for (auto s : ss)\n"
              "        (void)s.name;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:15]: (performance) Range variable 's' should be declared as const reference. [iterateByValue]\n",
                      errout_str());
    }

    void knownConditionFloating()
    {
        check("void foo() {\n" // #11200
              "    float f = 1.0;\n"
              "    if (f > 1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:16] -> [test.cpp:3:11]: (style) The comparison 'f > 1.0' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #13508
              "    float f = 1.0;\n"
              "    if (f > -1.0) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:2] -> [test.cpp:3]: (style) The comparison 'f > -1.0' is always false.\n",
            "",
            errout_str());

        check("void foo() {\n" // #13506
              "    float f = 1.0;\n"
              "    if (f > +1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:16] -> [test.cpp:3:11]: (style) The comparison 'f > +1.0' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n"
              "    float f = 1.0;\n"
              "    if (f < +1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:16] -> [test.cpp:3:11]: (style) The comparison 'f < 1.0' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #11200
              "    float pf = +1.0;\n"
              "    if (pf > 1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:18] -> [test.cpp:3:12]: (style) The comparison 'pf > 1.0' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #13508
              "    float pf = +1.0;\n"
              "    if (pf > -1.0) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:2] -> [test.cpp:3]: (style) The comparison 'pf > -1.0' is always false.\n",
            "",
            errout_str());

        check("void foo() {\n" // #13506
              "    float pf = +1.0;\n"
              "    if (pf > +1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:18] -> [test.cpp:3:12]: (style) The comparison 'pf > +1.0' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n"
              "    float pf = +1.0;\n"
              "    if (pf < +1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:18] -> [test.cpp:3:12]: (style) The comparison 'pf < 1.0' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #11200
              "    float nf = -1.0;\n"
              "    if (nf > -1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:18] -> [test.cpp:3:12]: (style) The comparison 'nf > -1.0' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #13508
              "    float nf = -1.0;\n"
              "    if (nf > 1.0) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:2] -> [test.cpp:3]: (style) The comparison 'nf > 1.0' is always false.\n",
            "",
            errout_str());

        check("void foo() {\n" // #13508
              "    float nf = -1.0;\n"
              "    if (nf > +1.0) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:2] -> [test.cpp:3]: (style) The comparison 'nf > +1.0' is always false.\n",
            "",
            errout_str());

        check("void foo() {\n"
              "    float f = 1.0f;\n"
              "    if (f > 1.00f) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:16] -> [test.cpp:3:11]: (style) The comparison 'f > 1.00f' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #13508
              "    float f = 1.0f;\n"
              "    if (f > 1) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:2] -> [test.cpp:3]: (style) The comparison 'f > 1' is always false.\n",
            "",
            errout_str());

        check("void foo() {\n"
              "    float f = 1.0;\n"
              "    if (f > 1.00) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:16] -> [test.cpp:3:11]: (style) The comparison 'f > 1.00' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #13508
              "    float f = 1.0;\n"
              "    if (f > 1) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:2] -> [test.cpp:3]: (style) The comparison 'f > 1' is always false.\n",
            "",
            errout_str());
    }

    void knownConditionPrefixed()
    {
        check("void foo() {\n"
              "    int i = 1;\n"
              "    if (i < +1) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:13] -> [test.cpp:3:11]: (style) The comparison 'i < 1' is always false. [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #13506
              "    int i = 1;\n"
              "    if (i > +1) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:13] -> [test.cpp:3:11]: (style) The comparison 'i > +1' is always false. [knownConditionTrueFalse]\n",
            errout_str());
    }

    void unionZeroInitBasic() {
        check(
            "union bad_union_0 {\n"
            "  char c;\n"
            "  long long i64;\n"
            "  void *p;\n"
            "};\n"
            "\n"
            "typedef union {\n"
            "  char c;\n"
            "  int i;\n"
            "} bad_union_1;\n"
            "\n"
            "extern void e(union bad_union_0 *);\n"
            "\n"
            "void\n"
            "foo(void)\n"
            "{\n"
            "  union { int i; char c; } good0 = {0};\n"
            "  union { int i; char c; } good1 = {};\n"
            "\n"
            "  union { char c; int i; } bad0 = {0};\n"
            "  union bad_union_0 bad1 = {0};\n"
            "  e(&bad1);\n"
            "  bad_union_1 bad2 = {0};\n"
            "}");
        const std::string exp = unionZeroInitMessage(20, 28, "bad0", "i") +
                                unionZeroInitMessage(21, 21, "bad1", "i64") +
                                unionZeroInitMessage(23, 15, "bad2", "i");
        ASSERT_EQUALS(exp, errout_str());
    }

    void unionZeroInitArrayMember() {
        check(
            "void foo(void) {\n"
            " union { int c; char s8[2]; } u = {0};\n"
            "}");
        ASSERT_EQUALS("", errout_str());
    }

    void unionZeroInitStructMember() {
        check(
            "void foo(void) {\n"
            "  union {\n"
            "    int c;\n"
            "     struct {\n"
            "       char x;\n"
            "       struct {\n"
            "         char y;\n"
            "       } s1;\n"
            "    } s0;\n"
            "  } u = {0};\n"
            "}");
        ASSERT_EQUALS("", errout_str());
    }

    void unionZeroInitUnknownType() {
        check(
            "union u {\n"
            "  Unknown x;\n"
            "}");
        ASSERT_EQUALS("", errout_str());
    }

    void unionZeroInitBitfields() {
        check(
            "typedef union Evex {\n"
            "  int u32;\n"
            "  struct {\n"
            "    char mmm:3,\n"
            "         b4:1,\n"
            "         r4:1,\n"
            "         b3:1,\n"
            "         x3:1,\n"
            "         r3:1;\n"
            "  } extended;\n"
            "} Evex;\n"
            "\n"
            "void foo(void) {\n"
            "  Evex evex = {0};\n"
            "}");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestOther)
