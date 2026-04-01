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

#include "checkcondition.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "platform.h"
#include "settings.h"

#include <cstddef>
#include <string>

class TestCondition : public TestFixture {
public:
    TestCondition() : TestFixture("TestCondition") {}

private:
    const Settings settings0 = settingsBuilder().library("qt.cfg").library("std.cfg").severity(Severity::style).severity(Severity::warning).build();
    /*const*/ Settings settings1 = settingsBuilder().severity(Severity::style).severity(Severity::warning).build();
    const Settings settings2 = settingsBuilder(settings0).severity(Severity::performance).certainty(Certainty::inconclusive).build();

    void run() override {
        const char cfg[] = "<?xml version=\"1.0\"?>\n"
                           "<def>\n"
                           "  <function name=\"bar\"> <pure/> </function>\n"
                           "</def>";
        settings1 = settingsBuilder(settings1).libraryxml(cfg).build();

        mNewTemplate = true;
        TEST_CASE(assignAndCompare);   // assignment and comparison don't match
        TEST_CASE(mismatchingBitAnd);  // overlapping bitmasks
        TEST_CASE(comparison);         // CheckCondition::comparison test cases
        TEST_CASE(multicompare);       // mismatching comparisons
        TEST_CASE(overlappingElseIfCondition);  // overlapping conditions in if and else-if
        TEST_CASE(oppositeElseIfCondition); // opposite conditions in if and else-if

        TEST_CASE(checkBadBitmaskCheck);

        TEST_CASE(incorrectLogicOperator1);
        TEST_CASE(incorrectLogicOperator2);
        TEST_CASE(incorrectLogicOperator3);
        TEST_CASE(incorrectLogicOperator4);
        TEST_CASE(incorrectLogicOperator5); // complex expressions
        TEST_CASE(incorrectLogicOperator6); // char literals
        TEST_CASE(incorrectLogicOperator7); // opposite expressions: (expr || !expr)
        TEST_CASE(incorrectLogicOperator8); // !
        TEST_CASE(incorrectLogicOperator9);
        TEST_CASE(incorrectLogicOperator10); // enum
        TEST_CASE(incorrectLogicOperator11);
        TEST_CASE(incorrectLogicOperator12);
        TEST_CASE(incorrectLogicOperator13);
        TEST_CASE(incorrectLogicOperator14);
        TEST_CASE(incorrectLogicOperator15);
        TEST_CASE(incorrectLogicOperator16); // #10070
        TEST_CASE(incorrectLogicOperator17);
        TEST_CASE(secondAlwaysTrueFalseWhenFirstTrueError);
        TEST_CASE(incorrectLogicOp_condSwapping);
        TEST_CASE(testBug5895);
        TEST_CASE(testBug5309);

        TEST_CASE(modulo);

        TEST_CASE(oppositeInnerCondition);
        TEST_CASE(oppositeInnerConditionPointers);
        TEST_CASE(oppositeInnerConditionClass);
        TEST_CASE(oppositeInnerConditionUndeclaredVariable);
        TEST_CASE(oppositeInnerConditionAlias);
        TEST_CASE(oppositeInnerCondition2);
        TEST_CASE(oppositeInnerCondition3);
        TEST_CASE(oppositeInnerConditionAnd);
        TEST_CASE(oppositeInnerConditionOr);
        TEST_CASE(oppositeInnerConditionEmpty);
        TEST_CASE(oppositeInnerConditionFollowVar);
        TEST_CASE(oppositeInnerConditionLambda);

        TEST_CASE(identicalInnerCondition);

        TEST_CASE(identicalConditionAfterEarlyExit);
        TEST_CASE(innerConditionModified);

        TEST_CASE(clarifyCondition1);     // if (a = b() < 0)
        TEST_CASE(clarifyCondition2);     // if (a & b == c)
        TEST_CASE(clarifyCondition3);     // if (! a & b)
        TEST_CASE(clarifyCondition4);     // ticket #3110
        TEST_CASE(clarifyCondition5);     // #3609 CWinTraits<WS_CHILD|WS_VISIBLE>..
        TEST_CASE(clarifyCondition6);     // #3818
        TEST_CASE(clarifyCondition7);
        TEST_CASE(clarifyCondition8);

        TEST_CASE(alwaysTrue);
        TEST_CASE(alwaysTrueSymbolic);
        TEST_CASE(alwaysTrueInfer);
        TEST_CASE(alwaysTrueContainer);
        TEST_CASE(alwaysTrueLoop);
        TEST_CASE(alwaysTrueTryCatch);
        TEST_CASE(alwaysTrueSideEffect);
        TEST_CASE(alwaysTruePremiumMisra);
        TEST_CASE(multiConditionAlwaysTrue);
        TEST_CASE(duplicateCondition);

        TEST_CASE(checkInvalidTestForOverflow);
        TEST_CASE(checkConditionIsAlwaysTrueOrFalseInsideIfWhile);
        TEST_CASE(alwaysTrueFalseInLogicalOperators);
        TEST_CASE(pointerAdditionResultNotNull);
        TEST_CASE(duplicateConditionalAssign);

        TEST_CASE(checkAssignmentInCondition);
        TEST_CASE(compareOutOfTypeRange);
        TEST_CASE(knownConditionCast); // #9976
        TEST_CASE(knownConditionIncrementLoop); // #9808
        TEST_CASE(knownConditionAfterBailout); // #12526
        TEST_CASE(knownConditionIncDecOperator);
        TEST_CASE(knownConditionFloating);
    }

    struct CheckOptions
    {
        bool cpp = true;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        check_(file, line, code, settings0, options.cpp);
    }

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const Settings& settings, bool cpp = true) {
        SimpleTokenizer2 tokenizer(settings, *this, code, cpp ? "test.cpp" : "test.c");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Run checks..
        runChecks<CheckCondition>(tokenizer, this);
    }

#define checkP(...) checkP_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkP_(const char* file, int line, const char (&code)[size])
    {
        SimpleTokenizer2 tokenizer(settings2, *this, code, "test.cpp");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Run checks..
        runChecks<CheckCondition>(tokenizer, this);
    }

    void assignAndCompare() {
        // &
        check("void foo(int x)\n"
              "{\n"
              "    int y = x & 4;\n"
              "    if (y == 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:4:8]: (style) Mismatching assignment and comparison, comparison 'y==3' is always false. [assignIfError]\n", errout_str());

        check("void foo(int x)\n"
              "{\n"
              "    int y = x & 4;\n"
              "    if (y != 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:4:8]: (style) Mismatching assignment and comparison, comparison 'y!=3' is always true. [assignIfError]\n", errout_str());

        // |
        check("void foo(int x) {\n"
              "    int y = x | 0x14;\n"
              "    if (y == 0x710);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:8]: (style) Mismatching assignment and comparison, comparison 'y==0x710' is always false. [assignIfError]\n", errout_str());

        check("void foo(int x) {\n"
              "    int y = x | 0x14;\n"
              "    if (y == 0x71f);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // various simple assignments
        check("void foo(int x) {\n"
              "    int y = (x+1) | 1;\n"
              "    if (y == 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:8]: (style) Mismatching assignment and comparison, comparison 'y==2' is always false. [assignIfError]\n", errout_str());

        check("void foo() {\n"
              "    int y = 1 | x();\n"
              "    if (y == 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:8]: (style) Mismatching assignment and comparison, comparison 'y==2' is always false. [assignIfError]\n", errout_str());

        // multiple conditions
        check("void foo(int x) {\n"
              "    int y = x & 4;\n"
              "    if ((y == 3) && (z == 1));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:9]: (style) Mismatching assignment and comparison, comparison 'y==3' is always false. [assignIfError]\n", errout_str());

        check("void foo(int x) {\n"
              "    int y = x & 4;\n"
              "    if ((x==123) || ((y == 3) && (z == 1)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:22]: (style) Mismatching assignment and comparison, comparison 'y==3' is always false. [assignIfError]\n", errout_str());

        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    if (setvalue(&y) && y != 8);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // recursive checking into scopes
        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    if (z) y=0;\n"
              "    else { if (y==8); }\n" // always false
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:4:15]: (style) Mismatching assignment and comparison, comparison 'y==8' is always false. [assignIfError]\n", errout_str());

        // while
        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    while (y==8);\n" // local variable => always false
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:11]: (style) Mismatching assignment and comparison, comparison 'y==8' is always false. [assignIfError]\n", errout_str());

        check("void f(int x) {\n"
              "    extern int y; y = x & 7;\n"
              "    while (y==8);\n" // non-local variable => no error
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    int a = 100;\n"
              "    while (x) {\n"
              "        int y = 16 | a;\n"
              "        while (y != 0) y--;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int x);\n"
              "void f(int x) {\n"
              "    int a = 100;\n"
              "    while (x) {\n"
              "        int y = 16 | a;\n"
              "        while (y != 0) g(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:5:15] -> [test.cpp:6:15]: (style) Mismatching assignment and comparison, comparison 'y!=0' is always true. [assignIfError]\n",
            errout_str());

        check("void g(int &x);\n"
              "void f(int x) {\n"
              "    int a = 100;\n"
              "    while (x) {\n"
              "        int y = 16 | a;\n"
              "        while (y != 0) g(y);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // calling function
        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    do_something();\n"
              "    if (y==8);\n" // local variable => always false
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:4:8]: (style) Mismatching assignment and comparison, comparison 'y==8' is always false. [assignIfError]\n", errout_str());

        check("void f(int x) {\n"
              "    int y = x & 7;\n"
              "    do_something(&y);\n" // passing variable => no error
              "    if (y==8);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void do_something(int);\n"
              "void f(int x) {\n"
              "    int y = x & 7;\n"
              "    do_something(y);\n"
              "    if (y==8);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:5:8]: (style) Mismatching assignment and comparison, comparison 'y==8' is always false. [assignIfError]\n", errout_str());

        check("void f(int x) {\n"
              "    extern int y; y = x & 7;\n"
              "    do_something();\n"
              "    if (y==8);\n" // non-local variable => no error
              "}");
        ASSERT_EQUALS("", errout_str());

        // #4434 : false positive: ?:
        check("void f(int x) {\n"
              "    x = x & 1;\n"
              "    x = x & 1 ? 1 : -1;\n"
              "    if(x != -1) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #4735
        check("void f() {\n"
              "    int x = *(char*)&0x12345678;\n"
              "    if (x==18) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // bailout: no variable info
        check("void foo(int x) {\n"
              "    y = 2 | x;\n"  // y not declared => no error
              "    if(y == 1) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // bailout: negative number
        check("void foo(int x) {\n"
              "    int y = -2 | x;\n" // negative number => no error
              "    if (y==1) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // bailout: pass variable to function
        check("void foo(int x) {\n"
              "    int y = 2 | x;\n"
              "    bar(&y);\n"  // pass variable to function => no error
              "    if (y==1) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // no crash on unary operator& (#5643)
        // #11610
        check("SdrObject* ApplyGraphicToObject() {\n"
              "    if (&rHitObject) {}\n"
              "    else if (rHitObject.IsClosedObj() && !&rHitObject) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Condition '&rHitObject' is always true [knownConditionTrueFalse]\n"
                      "[test.cpp:3:42]: (style) Condition '!&rHitObject' is always false [knownConditionTrueFalse]\n",
                      errout_str());

        // #5695: increment
        check("void f(int a0, int n) {\n"
              "  int c = a0 & 3;\n"
              "  for (int a = 0; a < n; a++) {\n"
              "    c++;\n"
              "    if (c == 4)\n"
              "      c  = 0;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a) {\n" // #6662
              "  int x = a & 1;\n"
              "  while (x <= 4) {\n"
              "    if (x != 5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:4:8]: (style) Mismatching assignment and comparison, comparison 'x!=5' is always true. [assignIfError]\n", errout_str());

        check("void f(int a) {\n" // #6662
              "  int x = a & 1;\n"
              "  while ((x += 4) < 10) {\n"
              "    if (x != 5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int x = 100;\n"
              "    while (x) {\n"
              "        g(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int x);\n"
              "void f() {\n"
              "    int x = 100;\n"
              "    while (x) {\n"
              "        g(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Condition 'x' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void g(int & x);\n"
              "void f() {\n"
              "    int x = 100;\n"
              "    while (x) {\n"
              "        g(x);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

    }

    void mismatchingBitAnd() {
        check("void f(int a) {\n"
              "    int b = a & 0xf0;\n"
              "    b &= 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:5]: (style) Mismatching bitmasks. Result is always 0 (X = Y & 0xf0; Z = X & 0x1; => Z=0). [mismatchingBitAnd]\n", errout_str());

        check("void f(int a) {\n"
              "    int b = a & 0xf0;\n"
              "    int c = b & 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:9]: (style) Mismatching bitmasks. Result is always 0 (X = Y & 0xf0; Z = X & 0x1; => Z=0). [mismatchingBitAnd]\n", errout_str());

        check("void f(int a) {\n"
              "    int b = a;"
              "    switch (x) {\n"
              "    case 1: b &= 1; break;\n"
              "    case 2: b &= 2; break;\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void comparison() {
        // CheckCondition::comparison test cases
        // '=='
        check("void f(int a) {\n assert( (a & 0x07) == 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X & 0x7) == 0x8' is always false. [comparisonError]\n",errout_str());
        check("void f(int a) {\n assert( (a & b & 4 & c ) == 3 );\n}");
        ASSERT_EQUALS("[test.cpp:2:21]: (style) Expression '(X & 0x4) == 0x3' is always false. [comparisonError]\n", errout_str());
        check("void f(int a) {\n assert( (a | 0x07) == 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X | 0x7) == 0x8' is always false. [comparisonError]\n",errout_str());
        check("void f(int a) {\n assert( (a & 0x07) == 7U );\n}");
        ASSERT_EQUALS("", errout_str());
        check("void f(int a) {\n assert( (a | 0x01) == -15 );\n}");
        ASSERT_EQUALS("", errout_str());
        // '!='
        check("void f(int a) {\n assert( (a & 0x07) != 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X & 0x7) != 0x8' is always true. [comparisonError]\n",errout_str());
        check("void f(int a) {\n assert( (a | 0x07) != 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X | 0x7) != 0x8' is always true. [comparisonError]\n",errout_str());
        check("void f(int a) {\n assert( (a & 0x07) != 7U );\n}");
        ASSERT_EQUALS("", errout_str());
        check("void f(int a) {\n assert( (a | 0x07) != 7U );\n}");
        ASSERT_EQUALS("", errout_str());
        // '>='
        check("void f(int a) {\n assert( (a & 0x07) >= 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X & 0x7) >= 0x8' is always false. [comparisonError]\n",errout_str());
        check("void f(unsigned int a) {\n assert( (a | 0x7) >= 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X | 0x7) >= 0x7' is always true. [comparisonError]\n",errout_str());
        check("void f(int a) {\n assert( (a & 0x07) >= 7U );\n}");
        ASSERT_EQUALS("",errout_str());
        check("void f(int a) {\n assert( (a | 0x07) >= 8U );\n}");
        ASSERT_EQUALS("",errout_str()); //correct for negative 'a'
        // '>'
        check("void f(int a) {\n assert( (a & 0x07) > 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X & 0x7) > 0x7' is always false. [comparisonError]\n",errout_str());
        check("void f(unsigned int a) {\n assert( (a | 0x7) > 6U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X | 0x7) > 0x6' is always true. [comparisonError]\n",errout_str());
        check("void f(int a) {\n assert( (a & 0x07) > 6U );\n}");
        ASSERT_EQUALS("",errout_str());
        check("void f(int a) {\n assert( (a | 0x07) > 7U );\n}");
        ASSERT_EQUALS("",errout_str()); //correct for negative 'a'
        // '<='
        check("void f(int a) {\n assert( (a & 0x07) <= 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X & 0x7) <= 0x7' is always true. [comparisonError]\n",errout_str());
        check("void f(unsigned int a) {\n assert( (a | 0x08) <= 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X | 0x8) <= 0x7' is always false. [comparisonError]\n",errout_str());
        check("void f(int a) {\n assert( (a & 0x07) <= 6U );\n}");
        ASSERT_EQUALS("",errout_str());
        check("void f(int a) {\n assert( (a | 0x08) <= 7U );\n}");
        ASSERT_EQUALS("",errout_str()); //correct for negative 'a'
        // '<'
        check("void f(int a) {\n assert( (a & 0x07) < 8U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X & 0x7) < 0x8' is always true. [comparisonError]\n",errout_str());
        check("void f(unsigned int a) {\n assert( (a | 0x07) < 7U );\n}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Expression '(X | 0x7) < 0x7' is always false. [comparisonError]\n",errout_str());
        check("void f(int a) {\n assert( (a & 0x07) < 3U );\n}");
        ASSERT_EQUALS("",errout_str());
        check("void f(int a) {\n assert( (a | 0x07) < 7U );\n}");
        ASSERT_EQUALS("",errout_str()); //correct for negative 'a'

        check("void f(int i) {\n" // #11998
              "    if ((i & 0x100) == 0x200) {}\n"
              "    if (0x200 == (i & 0x100)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:21] -> [test.cpp:3:15]: (style) The if condition is the same as the previous if condition [duplicateCondition]\n"
                      "[test.cpp:2:12]: (style) Expression '(X & 0x100) == 0x200' is always false. [comparisonError]\n"
                      "[test.cpp:3:21]: (style) Expression '(X & 0x100) == 0x200' is always false. [comparisonError]\n",
                      errout_str());

        checkP("#define MACRO1 (0x0010)\n" // #13222
               "#define MACRO2 (0x0020)\n"
               "#define MACRO_ALL (MACRO1 | MACRO2)\n"
               "void f() {\n"
               "    if (MACRO_ALL == 0) {}\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkP("void f(int i, int j) {\n" // #13360
               "    int X = 0x10;\n"
               "    if ((i & 0xff00) == X) {}\n"
               "    if (X == (j & 0xff00)) {}\n"
               "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) Expression '(X & 0xff00) == 0x10' is always false. [comparisonError]\n"
                      "[test.cpp:4:17]: (style) Expression '(X & 0xff00) == 0x10' is always false. [comparisonError]\n",
                      errout_str());
    }

#define checkPureFunction(...) checkPureFunction_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkPureFunction_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings1, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        runChecks<CheckCondition>(tokenizer, this);
    }

    void multicompare() {
        check("void foo(int x)\n"
              "{\n"
              "    if (x & 7);\n"
              "    else { if (x == 1); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (style) Expression is always false because 'else if' condition matches previous condition at line 3. [multiCondition]\n", errout_str());

        check("void foo(int x)\n"
              "{\n"
              "    if (x & 7);\n"
              "    else { if (x & 1); }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (style) Expression is always false because 'else if' condition matches previous condition at line 3. [multiCondition]\n", errout_str());

        check("extern int bar() __attribute__((pure));\n"
              "void foo(int x)\n"
              "{\n"
              "    if ( bar() >1 && b) {}\n"
              "    else if (bar() >1 && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (style) Expression is always false because 'else if' condition matches previous condition at line 4. [multiCondition]\n", errout_str());

        checkPureFunction("extern int bar();\n"
                          "void foo(int x)\n"
                          "{\n"
                          "    if ( bar() >1 && b) {}\n"
                          "    else if (bar() >1 && b) {}\n"
                          "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (style) Expression is always false because 'else if' condition matches previous condition at line 4. [multiCondition]\n", errout_str());

        // 7284
        check("void foo() {\n"
              "    if (a) {}\n"
              "    else if (!!a) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        // #11059
        check("int f();\n"
              "void g() {\n"
              "    int i = f();\n"
              "    if (i == 3) {}\n"
              "    else if ((i = f()) == 5) {}\n"
              "    else if (i == 3) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f();\n"
              "void g() {\n"
              "    int i = f();\n"
              "    if (i == 3) {}\n"
              "    else if ((i = f()) == 5) {}\n"
              "    else if (i != 3) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void overlappingElseIfCondition() {
        check("void f(int a, int &b) {\n"
              "    if (a) { b = 1; }\n"
              "    else { if (a) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(int a, int &b) {\n"
              "    if (a) { b = 1; }\n"
              "    else { if (a) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1) { b = 1; }\n"
              "    else { if (a == 2) { b = 2; }\n"
              "    else { if (a == 1) { b = 3; } } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(int a, int &b) {\n"
              "    if (a == 1) { b = 1; }\n"
              "    else { if (a == 2) { b = 2; }\n"
              "    else { if (a == 2) { b = 3; } } }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (style) Expression is always false because 'else if' condition matches previous condition at line 3. [multiCondition]\n", errout_str());

        check("void f(int a, int &b) {\n"
              "    if (a++) { b = 1; }\n"
              "    else { if (a++) { b = 2; }\n"
              "    else { if (a++) { b = 3; } } }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a, int &b) {\n"
              "    if (!strtok(NULL, \" \")) { b = 1; }\n"
              "    else { if (!strtok(NULL, \" \")) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        {
            check("void f(Class &c) {\n"
                  "    if (c.dostuff() == 3) {}\n"
                  "    else { if (c.dostuff() == 3) {} }\n"
                  "}");
            ASSERT_EQUALS("", errout_str());

            check("void f(const Class &c) {\n"
                  "    if (c.dostuff() == 3) {}\n"
                  "    else { if (c.dostuff() == 3) {} }\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:3:28]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());
        }

        check("void f(int a, int &b) {\n"
              "   x = x / 2;\n"
              "   if (x < 100) { b = 1; }\n"
              "   else { x = x / 2; if (x < 100) { b = 2; } }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int64_t i) {\n"
              "   if(i == 0x02e2000000 || i == 0xa0c6000000)\n"
              "       foo(i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket 3689 ( avoid false positive )
        check("int fitInt(long long int nValue){\n"
              "    if( nValue < 0x7fffffffLL )\n"
              "    {\n"
              "        return 32;\n"
              "    }\n"
              "    if( nValue < 0x7fffffffffffLL )\n"
              "    {\n"
              "        return 48;\n"
              "    }\n"
              "    else {\n"
              "        if( nValue < 0x7fffffffffffffffLL )\n"
              "        {\n"
              "            return 64;\n"
              "        } else\n"
              "        {\n"
              "            return -1;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(WIDGET *widget) {\n"
              "  if (dynamic_cast<BUTTON*>(widget)){}\n"
              "  else if (dynamic_cast<LABEL*>(widget)){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class B { virtual void v() {} };\n" // #11037
              "class D1 : public B {};\n"
              "class D2 : public B {};\n"
              "void f(const std::shared_ptr<B>&p) {\n"
              "    const auto d1 = dynamic_cast<D1*>(p.get());\n"
              "    const auto d2 = dynamic_cast<D2*>(p.get());\n"
              "    if (d1) {}\n"
              "    else if (d2) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n" // #6482
              "  if (x & 1) {}\n"
              "  else if (x == 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "  if (x & 15) {}\n"
              "  else if (x == 40) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(int x) {\n"
              "  if (x == sizeof(double)) {}\n"
              "  else { if (x == sizeof(long double)) {} }"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "  if (x & 0x08) {}\n"
              "  else if (x & 0xF8) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "  if (x & 0xF8) {}\n"
              "  else if (x & 0x08) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( !!b && !!a){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( !!b && a){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( b && !!a){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( b && !(!a)){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( !!b && !(!a)){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Expression is always false because 'else if' condition matches previous condition at line 2. [multiCondition]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "   if(a && b){}\n"
              "   else if( !!(b) && !!(a+b)){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8168
        check("enum MaskValues\n"
              "{\n"
              "    Value1 = 0x00000001,\n"
              "    Value2 = 0x00000002\n"
              "};\n"
              "void TestFunction(int value) {\n"
              "    if ( value & (int)Value1 ) {}\n"
              "    else if ( value & (int)Value2 ) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(size_t x) {\n"
              "    if (x == sizeof(int)) {}\n"
              "    else { if (x == sizeof(long)) {} }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(size_t x) {\n"
              "    if (x == sizeof(long)) {}\n"
              "    else { if (x == sizeof(long long)) {} }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void oppositeElseIfCondition() {
        setMultiline();

        check("void f(int x) {\n"
              "    if (x) {}\n"
              "    else if (!x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: style: Expression is always true because 'else if' condition is opposite to previous condition at line 2. [multiCondition]\n"
                      "[test.cpp:2:9]: note: first condition\n"
                      "[test.cpp:3:14]: note: else if condition is opposite to first condition\n", errout_str());

        check("void f(int x) {\n"
              "    int y = x;\n"
              "    if (x) {}\n"
              "    else if (!y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:14]: style: Expression is always true because 'else if' condition is opposite to previous condition at line 3. [multiCondition]\n"
                      "[test.cpp:2:13]: note: 'y' is assigned value 'x' here.\n"
                      "[test.cpp:3:9]: note: first condition\n"
                      "[test.cpp:4:14]: note: else if condition is opposite to first condition\n", errout_str());
    }

    void checkBadBitmaskCheck() {
        check("bool f(int x) {\n"
              "    bool b = x | 0x02;\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("bool f(int x) {\n"
              "    bool b = 0x02 | x;\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:19]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("int f(int x) {\n"
              "    int b = x | 0x02;\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int x) {\n"
              "    bool b = x & 0x02;\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int x) {\n"
              "    if(x | 0x02)\n"
              "        return b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("bool f(int x) {\n"
              "    int y = 0x1;\n"
              "    if(b) y = 0;\n"
              "    if(x | y)\n"
              "        return b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int x) {\n"
              "    foo(a && (x | 0x02));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("int f(int x) {\n"
              "    return (x | 0x02) ? 0 : 5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("int f(int x) {\n"
              "    return x ? (x | 0x02) : 5;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int x) {\n"
              "    return x | 0x02;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("bool f(int x) {\n"
              "  if (x) {\n"
              "    return x | 0x02;\n"
              "  }\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("const bool f(int x) {\n"
              "    return x | 0x02;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("struct F {\n"
              "  static const bool f(int x) {\n"
              "      return x | 0x02;\n"
              "  }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3:16]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("struct F {\n"
              "  typedef bool b_t;\n"
              "};\n"
              "F::b_t f(int x) {\n"
              "  return x | 0x02;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (warning) Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'? [badBitmaskCheck]\n", errout_str());

        check("int f(int x) {\n"
              "    return x | 0x02;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void create_rop_masks_4( rop_mask_bits *bits) {\n"
              "DWORD mask_offset;\n"
              "BYTE *and_bits = bits->and;\n"
              "rop_mask *rop_mask;\n"
              "and_bits[mask_offset] |= (rop_mask->and & 0x0f);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(unsigned a, unsigned b) {\n"
              "  unsigned cmd1 = b & 0x0F;\n"
              "  if (cmd1 | a) {\n"
              "    if (b == 0x0C) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i) {\n" // #11082
              "    int j = 0;\n"
              "    if (i | j) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Operator '|' with one operand equal to zero is redundant. [badBitmaskCheck]\n", errout_str());

        check("#define EIGHTTOIS(x) (((x) << 8) | (x))\n"
              "int f() {\n"
              "    return EIGHTTOIS(0);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("#define O_RDONLY 0\n"
              "void f(const char* s, int* pFd) {\n"
              "    *pFd = open(s, O_RDONLY | O_BINARY, 0);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("const int FEATURE_BITS = x |\n"
              "#if FOO_ENABLED\n"
              "    FEATURE_FOO |\n"
              "#endif\n"
              "#if BAR_ENABLED\n"
              "    FEATURE_BAR |\n"
              "#endif\n"
              "    0;");
        ASSERT_EQUALS("", errout_str());

        check("enum precedence { PC0, UNARY };\n"
              "int x = PC0   | UNARY;\n"
              "int y = UNARY | PC0;\n");
        ASSERT_EQUALS("", errout_str());

        check("#define MASK 0\n"
              "#define SHIFT 1\n"
              "int x = 1 | (MASK << SHIFT);\n");
        ASSERT_EQUALS("", errout_str());
    }


    void incorrectLogicOperator1() {
        check("void f(int x) {\n"
              "    if ((x != 1) || (x != 3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Logical disjunction always evaluates to true: x != 1 || x != 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (1 != x || 3 != x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical disjunction always evaluates to true: x != 1 || x != 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "  if (x<0 && !x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Logical conjunction always evaluates to false: x < 0 && !x. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "  if (x==0 && x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Logical conjunction always evaluates to false: x == 0 && x. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n" // ast..
              "    if (y == 1 && x == 1 && x == 7) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:26]: (warning) Logical conjunction always evaluates to false: x == 1 && x == 7. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x, int y) {\n"
              "    if (x != 1 || y != 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, int y) {\n"
              "    if ((y == 1) && (x != 1) || (x != 3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, int y) {\n"
              "    if ((x != 1) || (x != 3) && (y == 1))\n"
              "        a++;\n"
              "}"
              );
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:2:24]: (style) Condition 'x!=3' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x != 1) && (x != 3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if ((x == 1) || (x == 3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, int y) {\n"
              "    if ((x != 1) || (y != 3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, int y) {\n"
              "    if ((x != hotdog) || (y != hotdog))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, int y) {\n"
              "    if ((x != 5) || (y != 5))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        check("void f(int x) {\n"
              "    if ((x != 5) || (x != 6))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Logical disjunction always evaluates to true: x != 5 || x != 6. [incorrectLogicOperator]\n", errout_str());

        check("void f(unsigned int a, unsigned int b, unsigned int c) {\n"
              "    if((a != b) || (c != b) || (c != a))\n"
              "    {\n"
              "        return true;\n"
              "    }\n"
              "    return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:23] -> [test.cpp:2:35]: (style) Condition 'c!=a' is always false [knownConditionTrueFalse]\n", errout_str());
    }

    void incorrectLogicOperator2() {
        check("void f(float x) {\n"
              "    if ((x == 1) && (x == 1.0))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if ((x == 1) && (x == 0x00000001))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:2:24]: (style) Condition 'x==0x00000001' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x == 1 && x == 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical conjunction always evaluates to false: x == 1 && x == 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x == 1.0 && x == 3.0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str()); // float comparisons with == and != are not checked right now - such comparison is a bad idea

        check("void f(float x) {\n"
              "    if (x == 1 && x == 1.0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void bar(float f) {\n" // #5246
              "    if ((f > 0) && (f < 1)) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 1. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x < 1.0 && x > 1.0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Logical conjunction always evaluates to false: x < 1.0 && x > 1.0. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 1.0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 1.0. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x >= 1.0 && x <= 1.001)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (x < 1 && x > 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(float x) {\n"
              "    if (x < 1.0 && x > 3.0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Logical conjunction always evaluates to false: x < 1.0 && x > 3.0. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (1 > x && 3 < x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x < 3 && x > 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (x > 3 || x < 10)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical disjunction always evaluates to true: x > 3 || x < 10. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x <= 10)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical disjunction always evaluates to true: x >= 3 || x <= 10. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x < 10)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical disjunction always evaluates to true: x >= 3 || x < 10. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 3 || x <= 10)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical disjunction always evaluates to true: x > 3 || x <= 10. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 3 || x < 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x <= 3)\n"
              "        a++;\n"
              "}"
              );
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical disjunction always evaluates to true: x >= 3 || x <= 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x >= 3 || x < 3)\n"
              "        a++;\n"
              "}"
              );
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical disjunction always evaluates to true: x >= 3 || x < 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 3 || x <= 3)\n"
              "        a++;\n"
              "}"
              );
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical disjunction always evaluates to true: x > 3 || x <= 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "   if((x==3) && (x!=4))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (style) Redundant condition: The condition 'x != 4' is redundant since 'x == 3' is sufficient. [redundantCondition]\n", errout_str());

        check("void f(const std::string &s) {\n" // #8860
              "    const std::size_t p = s.find(\"42\");\n"
              "    const std::size_t * const ptr = &p;\n"
              "    if(p != std::string::npos && p == 0 && *ptr != 1){;}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:36] -> [test.cpp:4:49]: (style) Condition '*ptr!=1' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x!=4) && (x==3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Redundant condition: The condition 'x != 4' is redundant since 'x == 3' is sufficient. [redundantCondition]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x==3) || (x!=4))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Redundant condition: The condition 'x == 3' is redundant since 'x != 4' is sufficient. [redundantCondition]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x!=4) || (x==3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (style) Redundant condition: The condition 'x == 3' is redundant since 'x != 4' is sufficient. [redundantCondition]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x==3) && (x!=3))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical conjunction always evaluates to false: x == 3 && x != 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x==6) || (x!=6))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical disjunction always evaluates to true: x == 6 || x != 6. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 10 || x < 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (x > 5 && x == 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x > 5 && x == 1. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 5 && x == 6)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Redundant condition: The condition 'x > 5' is redundant since 'x == 6' is sufficient. [redundantCondition]\n", errout_str());

        // #3419
        check("void f() {\n"
              "    if ( &q != &a && &q != &b ) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3676
        check("void f(int m_x2, int w, int x) {\n"
              "    if (x + w - 1 > m_x2 || m_x2 < 0 )\n"
              "        m_x2 = x + w - 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(float x) {\n" // x+1 => x
              "  if (x <= 1.0e20 && x >= -1.0e20) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(float x) {\n" // x+1 => x
              "  if (x >= 1.0e20 && x <= 1.0e21) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(float x) {\n" // x+1 => x
              "  if (x <= -1.0e20 && x >= -1.0e21) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void incorrectLogicOperator3() {
        check("void f(int x, bool& b) {\n"
              "    b = x > 5 && x == 1;\n"
              "    c = x < 1 && x == 3;\n"
              "    d = x >= 5 && x == 1;\n"
              "    e = x <= 1 && x == 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x > 5 && x == 1. [incorrectLogicOperator]\n"
                      "[test.cpp:3:15]: (warning) Logical conjunction always evaluates to false: x < 1 && x == 3. [incorrectLogicOperator]\n"
                      "[test.cpp:4:16]: (warning) Logical conjunction always evaluates to false: x >= 5 && x == 1. [incorrectLogicOperator]\n"
                      "[test.cpp:5:16]: (warning) Logical conjunction always evaluates to false: x <= 1 && x == 3. [incorrectLogicOperator]\n", errout_str());
    }

    void incorrectLogicOperator4() {
        check("#define ZERO 0\n"
              "void f(int x) {\n"
              "  if (x && x != ZERO) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int N) {\n" // #9789
              "    T a[20] = { 0 };\n"
              "    for (int i = 0; i < N; ++i) {\n"
              "        if (0 < a[i] && a[i] < 1) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void incorrectLogicOperator5() { // complex expressions
        check("void f(int x) {\n"
              "  if (x+3 > 2 || x+3 < 10) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical disjunction always evaluates to true: x+3 > 2 || x+3 < 10. [incorrectLogicOperator]\n", errout_str());
    }

    void incorrectLogicOperator6() { // char literals
        const Settings s = settingsBuilder(settings0).certainty(Certainty::inconclusive).build();
        check("void f(char x) {\n"
              "  if (x == '1' || x == '2') {}\n"
              "}", s);
        ASSERT_EQUALS("", errout_str());

        check("void f(char x) {\n"
              "  if (x == '1' && x == '2') {}\n"
              "}", s);
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical conjunction always evaluates to false: x == '1' && x == '2'. [incorrectLogicOperator]\n", errout_str());

        check("int f(char c) {\n"
              "  return (c >= 'a' && c <= 'z');\n"
              "}", s);
        ASSERT_EQUALS("", errout_str());

        check("int f(char c) {\n"
              "  return (c <= 'a' && c >= 'z');\n"
              "}", s);
        ASSERT_EQUALS("[test.cpp:2:20]: (warning, inconclusive) Logical conjunction always evaluates to false: c <= 'a' && c >= 'z'. [incorrectLogicOperator]\n", errout_str());

        check("int f(char c) {\n"
              "  return (c <= 'a' && c >= 'z');\n"
              "}"); // TODO: use s?
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:2:25]: (style) Return value 'c>='z'' is always false [knownConditionTrueFalse]\n", errout_str());
    }

    void incorrectLogicOperator7() { // opposite expressions
        check("void f(int i) {\n"
              "  if (i || !i) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (warning) Logical disjunction always evaluates to true: i || !(i). [incorrectLogicOperator]\n", errout_str());

        check("void f(int a, int b) {\n"
              "  if (a>b || a<=b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Logical disjunction always evaluates to true: a > b || a <= b. [incorrectLogicOperator]\n", errout_str());

        check("void f(int a, int b) {\n"
              "  if (a>b || a<b) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6064 False positive incorrectLogicOperator - invalid assumption about template type?
        check("template<typename T> T icdf( const T uniform ) {\n"
              "   if ((0<uniform) && (uniform<1))\n"
              "     {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6081 False positive: incorrectLogicOperator, with close negative comparisons
        check("double neg = -1.0 - 1.0e-13;\n"
              "void foo() {\n"
              "    if ((neg < -1.0) && (neg > -1.0 - 1.0e-12))\n"
              "        return;\n"
              "    else\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void incorrectLogicOperator8() { // opposite expressions
        check("void f(int i) {\n"
              "  if (!(i!=10) && !(i!=20)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Logical conjunction always evaluates to false: !(i != 10) && !(i != 20). [incorrectLogicOperator]\n", errout_str());
    }

    void incorrectLogicOperator9() { //  #6069 "False positive incorrectLogicOperator due to dynamic_cast"
        check("class MyType;\n"
              "class OtherType;\n"
              "void foo (OtherType* obj) {\n"
              "    assert((!obj) || dynamic_cast<MyType*>(obj));\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void incorrectLogicOperator10() { //  #7794 - enum
        check("typedef enum { A, B } Type_t;\n"
              "void f(Type_t t) {\n"
              "    if ((t == A) && (t == B))\n"
              "    {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (warning) Logical conjunction always evaluates to false: t == 0 && t == 1. [incorrectLogicOperator]\n", errout_str());
    }

    void incorrectLogicOperator11() {
        check("void foo(int i, const int n) { if ( i < n && i == n ) {} }");
        ASSERT_EQUALS("[test.cpp:1:43]: (warning) Logical conjunction always evaluates to false: i < n && i == n. [incorrectLogicOperator]\n", errout_str());

        check("void foo(int i, const int n) { if ( i > n && i == n ) {} }");
        ASSERT_EQUALS("[test.cpp:1:43]: (warning) Logical conjunction always evaluates to false: i > n && i == n. [incorrectLogicOperator]\n", errout_str());

        check("void foo(int i, const int n) { if ( i == n && i > n ) {} }");
        ASSERT_EQUALS("[test.cpp:1:44]: (warning) Logical conjunction always evaluates to false: i == n && i > n. [incorrectLogicOperator]\n", errout_str());

        check("void foo(int i, const int n) { if ( i == n && i < n ) {} }");
        ASSERT_EQUALS("[test.cpp:1:44]: (warning) Logical conjunction always evaluates to false: i == n && i < n. [incorrectLogicOperator]\n", errout_str());
    }

    void incorrectLogicOperator12() { // #8696
        check("struct A {\n"
              "    void f() const;\n"
              "};\n"
              "void foo(A a, A b) {\n"
              "  A x = b;\n"
              "  A y = b;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:5:9] -> [test.cpp:6:9] -> [test.cpp:8:13]: (warning) Logical conjunction always evaluates to false: a > x && a < y. [incorrectLogicOperator]\n",
            errout_str());

        check("struct A {\n"
              "    void f();\n"
              "};\n"
              "void foo(A a, A b) {\n"
              "  A x = b;\n"
              "  A y = b;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(A a, A b) {\n"
              "  A x = b;\n"
              "  A y = b;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(A a, A b) {\n"
              "  const A x = b;\n"
              "  const A y = b;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:15] -> [test.cpp:3:15] -> [test.cpp:5:13]: (warning) Logical conjunction always evaluates to false: a > x && a < y. [incorrectLogicOperator]\n",
            errout_str());

        check("struct A {\n"
              "    void f() const;\n"
              "};\n"
              "void foo(A a) {\n"
              "  A x = a;\n"
              "  A y = a;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:9]: (style) Condition 'a>x' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:8:18]: (style) Condition 'a<y' is always false [knownConditionTrueFalse]\n",
                      errout_str());

        check("struct A {\n"
              "    void f();\n"
              "};\n"
              "void foo(A a) {\n"
              "  A x = a;\n"
              "  A y = a;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:9]: (style) Condition 'a>x' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo(A a) {\n"
              "  A x = a;\n"
              "  A y = a;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9]: (style) Condition 'a>x' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo(A a) {\n"
              "  const A x = a;\n"
              "  const A y = a;\n"
              "  y.f();\n"
              "  if (a > x && a < y)\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9]: (style) Condition 'a>x' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:5:18]: (style) Condition 'a<y' is always false [knownConditionTrueFalse]\n",
                      errout_str());
    }

    void incorrectLogicOperator13() {
        // 8780
        check("void f(const int &v) {\n"
              "    const int x=v;\n"
              "    if ((v == 1) && (x == 2)) {;}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17] -> [test.cpp:3:18]: (warning) Logical conjunction always evaluates to false: v == 1 && x == 2. [incorrectLogicOperator]\n", errout_str());

        check("void f2(const int *v) {\n"
              "    const int *x=v;\n"
              "    if ((*v == 1) && (*x == 2)) {;}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18] -> [test.cpp:3:19]: (warning) Logical conjunction always evaluates to false: *(v) == 1 && *(x) == 2. [incorrectLogicOperator]\n", errout_str());
    }

    void incorrectLogicOperator14() {
        check("static const std ::string h;\n"
              "class i {\n"
              "public:\n"
              "  struct j {\n"
              "    std ::string k;\n"
              "    std ::string l;\n"
              "  };\n"
              "  struct a {\n"
              "    enum { m = 1 };\n"
              "  };\n"
              "} b;\n"
              "namespace n {\n"
              "class c;\n"
              "}\n"
              "struct o {\n"
              "  enum { p, d, q, r };\n"
              "  enum { e, f };\n"
              "\n"
              "public:\n"
              "  class j {\n"
              "  public:\n"
              "    class s {\n"
              "      std ::string a;\n"
              "    };\n"
              "  };\n"
              "};\n"
              "namespace n {\n"
              "class b;\n"
              "}\n"
              "namespace aa {\n"
              "class d {\n"
              "public:\n"
              "  char t;\n"
              "  enum {} u;\n"
              "};\n"
              "} // namespace aa\n"
              "namespace aa {\n"
              "struct e {};\n"
              "} // namespace aa\n"
              "class a;\n"
              "class w {\n"
              "public:\n"
              "  enum { x };\n"
              "  struct {\n"
              "  } y;\n"
              "  std ::string z;\n"
              "};\n"
              "class ab {\n"
              "  friend class c;\n"
              "\n"
              "public:\n"
              "  class ac {\n"
              "    void e(const ac &v) const;\n"
              "  };\n"
              "};\n"
              "class f;\n"
              "class ad {\n"
              "  friend class e;\n"
              "  enum { e, ae, ag, ah, ai, aj, ak, a, b };\n"
              "  class c {};\n"
              "  class d {\n"
              "    enum am { f, an, ao, ap, aq, ar, b, as, at, c, au };\n"
              "    enum av { aw, ax, ay, az, e, ba, bb, bc, bd, a };\n"
              "    struct b {\n"
              "      am action;\n"
              "      av c;\n"
              "    };\n"
              "  };\n"
              "  class e {\n"
              "  public:\n"
              "    std ::string e;\n"
              "    class f {\n"
              "    } f;\n"
              "    class be {\n"
              "    public:\n"
              "    };\n"
              "    std ::vector<be> bf;\n"
              "    enum { bg, b } c;\n"
              "  };\n"
              "  struct bh {\n"
              "    std ::map<int, d> b;\n"
              "  };\n"
              "  std ::map<std ::string, bh> bi;\n"
              "  struct {\n"
              "    int b;\n"
              "    char bj;\n"
              "  } bk;\n"
              "  class a {\n"
              "  public:\n"
              "    std ::set<std ::string> b;\n"
              "  };\n"
              "};\n"
              "class bl;\n"
              "class al;\n"
              "class bm;\n"
              "class f;\n"
              "class b;\n"
              "class bn;\n"
              "namespace bo {\n"
              "class bp {\n"
              "public:\n"
              "  typedef std ::pair<const f *, std ::string> bq;\n"
              "  typedef std ::list<bq> br;\n"
              "};\n"
              "const bo ::bp *dg(const f *a, const al *b);\n"
              "} // namespace bo\n"
              "const bn *dh(const f *d, bo ::bp ::br &bs);\n"
              "class f {\n"
              "public:\n"
              "  struct bt {};\n"
              "  std ::vector<a> f;\n"
              "};\n"
              "class bu;\n"
              "class a;\n"
              "class c;\n"
              "struct bv {};\n"
              "class af {\n"
              "private:\n"
              "public:\n"
              "  enum { b, d, e, f, c, bw };\n"
              "  void a(int c);\n"
              "  af *bx() const;\n"
              "};\n"
              "namespace by {\n"
              "class b;\n"
              "}\n"
              "class b {\n"
              "public:\n"
              "  bool d, c;\n"
              "};\n"
              "class bz;\n"
              "class f;\n"
              "class ca {\n"
              "  friend class b;\n"
              "\n"
              "public:\n"
              "  const bm *cb() const { return cc; }\n"
              "  f *d(f *e, bool f) const;\n"
              "  int e() { return ++cd; }\n"
              "  bl *const c;\n"
              "  bm *cc;\n"
              "  std ::map<std ::string, int> ce;\n"
              "  int cd;\n"
              "  bz *a;\n"
              "};\n"
              "namespace n {\n"
              "class c;\n"
              "class d;\n"
              "} // namespace n\n"
              "class cf {\n"
              "public:\n"
              "  explicit cf(const std ::string &aname);\n"
              "  cf(const std ::string &aname, const ca *cg, const al *ch, bl *ci)\n"
              "      : cj(cg), ck(ch), cl(ci), cn(aname) {}\n"
              "\n"
              "protected:\n"
              "  const ca *const cj;\n"
              "  const al *const ck;\n"
              "  bl *const cl;\n"
              "  const std ::string cn;\n"
              "};\n"
              "class cm : public cf {\n"
              "public:\n"
              "  void cp();\n"
              "  std ::string d() const;\n"
              "};\n"
              "struct co {\n"
              "  co();\n"
              "  const bu *a;\n"
              "  enum f {};\n"
              "  enum {\n"
              "    b = (1 << 0),\n"
              "    c = (1 << 1),\n"
              "  };\n"
              "  void d(bool e);\n"
              "};\n"
              "class bu {\n"
              "  friend class e;\n"
              "\n"
              "public:\n"
              "  struct f {};\n"
              "  enum { d, cr, cq, ct, cs, e, a, b, c, dd, cu, cv, cw, cx, cy, cz, da };\n"
              "  const f *db;\n"
              "  const af *dc;\n"
              "} f{};\n"
              "class bm {\n"
              "public:\n"
              "  std ::list<bu> df;\n"
              "  std ::vector<const bu *> de;\n"
              "  mutable std ::set<std ::string> f;\n"
              "};\n"
              "void cm ::cp() {\n"
              "  const bm *a = cj->cb();\n"
              "  for (const bu *b : a->de)\n"
              "    for (af *c = b->dc->bx();;) {\n"
              "      af *d = c;\n"
              "      af *e = c;\n"
              "      bool f(d);\n"
              "      bool g(e);\n"
              "      if (f && g)\n"
              "        ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:200:11] -> [test.cpp:200:16]: (style) Condition 'g' is always true [knownConditionTrueFalse]\n", errout_str());
    }

    void incorrectLogicOperator15() {
        // 10022
        check("struct PipeRoute {\n"
              "    std::deque<int> points;\n"
              "    std::deque<int> estimates;\n"
              "};\n"
              "void CleanPipeRoutes(std::map<int, PipeRoute*>& pipeRoutes) {\n"
              "    for (auto it = pipeRoutes.begin(); it != pipeRoutes.end(); ) {\n"
              "        PipeRoute* curRoute = it->second;\n"
              "        if (curRoute->points.empty() && curRoute->estimates.size() != 2)\n"
              "        {\n"
              "            delete curRoute;\n"
              "            it = pipeRoutes.erase(it);\n"
              "        }\n"
              "        else\n"
              "        {\n"
              "            ++it;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void incorrectLogicOperator16() { // #10070
        check("void foo(void* p) {\n"
              "    if (!p || p == -1) { }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void incorrectLogicOperator17() { // #12471
        check("struct R {\n"
              "    void set() { i = 1; }\n"
              "    int get() const { return i; }\n"
              "    int i;\n"
              "};\n"
              "struct P {\n"
              "    void f();\n"
              "    R* r;\n"
              "};\n"
              "void P::f() {\n"
              "    int a = r->get();\n"
              "    r->set();\n"
              "    if (a == 0 && r->get()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void secondAlwaysTrueFalseWhenFirstTrueError() {
        check("void f(void) {\n" // #8892
              "    const char c[1] = { \'x\' }; \n"
              "    if(c[0] == \'x\'){;}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Condition 'c[0]=='x'' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 5 && x != 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Redundant condition: The condition 'x != 1' is redundant since 'x > 5' is sufficient. [redundantCondition]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 5 && x != 6)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if ((x > 5) && (x != 1))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Redundant condition: The condition 'x != 1' is redundant since 'x > 5' is sufficient. [redundantCondition]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x > 5) && (x != 6))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, bool& b) {\n"
              "    b = x > 3 || x == 4;\n"
              "    c = x < 5 || x == 4;\n"
              "    d = x >= 3 || x == 4;\n"
              "    e = x <= 5 || x == 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Redundant condition: The condition 'x == 4' is redundant since 'x > 3' is sufficient. [redundantCondition]\n"
                      "[test.cpp:3:15]: (style) Redundant condition: The condition 'x == 4' is redundant since 'x < 5' is sufficient. [redundantCondition]\n"
                      "[test.cpp:4:16]: (style) Redundant condition: The condition 'x == 4' is redundant since 'x >= 3' is sufficient. [redundantCondition]\n"
                      "[test.cpp:5:16]: (style) Redundant condition: The condition 'x == 4' is redundant since 'x <= 5' is sufficient. [redundantCondition]\n",
                      errout_str());

        check("void f(int x, bool& b) {\n"
              "    b = x > 5 || x != 1;\n"
              "    c = x < 1 || x != 3;\n"
              "    d = x >= 5 || x != 1;\n"
              "    e = x <= 1 || x != 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Redundant condition: The condition 'x > 5' is redundant since 'x != 1' is sufficient. [redundantCondition]\n"
                      "[test.cpp:3:15]: (style) Redundant condition: The condition 'x < 1' is redundant since 'x != 3' is sufficient. [redundantCondition]\n"
                      "[test.cpp:4:16]: (style) Redundant condition: The condition 'x >= 5' is redundant since 'x != 1' is sufficient. [redundantCondition]\n"
                      "[test.cpp:5:16]: (style) Redundant condition: The condition 'x <= 1' is redundant since 'x != 3' is sufficient. [redundantCondition]\n",
                      errout_str());

        check("void f(int x, bool& b) {\n"
              "    b = x > 6 && x > 5;\n"
              "    c = x > 5 || x > 6;\n"
              "    d = x < 6 && x < 5;\n"
              "    e = x < 5 || x < 6;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Redundant condition: The condition 'x > 5' is redundant since 'x > 6' is sufficient. [redundantCondition]\n"
                      "[test.cpp:3:15]: (style) Redundant condition: The condition 'x > 6' is redundant since 'x > 5' is sufficient. [redundantCondition]\n"
                      "[test.cpp:4:15]: (style) Redundant condition: The condition 'x < 6' is redundant since 'x < 5' is sufficient. [redundantCondition]\n"
                      "[test.cpp:5:15]: (style) Redundant condition: The condition 'x < 5' is redundant since 'x < 6' is sufficient. [redundantCondition]\n",
                      errout_str());

        check("void f(double x, bool& b) {\n"
              "    b = x > 6.5 && x > 5.5;\n"
              "    c = x > 5.5 || x > 6.5;\n"
              "    d = x < 6.5 && x < 5.5;\n"
              "    e = x < 5.5 || x < 6.5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Redundant condition: The condition 'x > 5.5' is redundant since 'x > 6.5' is sufficient. [redundantCondition]\n"
                      "[test.cpp:3:17]: (style) Redundant condition: The condition 'x > 6.5' is redundant since 'x > 5.5' is sufficient. [redundantCondition]\n"
                      "[test.cpp:4:17]: (style) Redundant condition: The condition 'x < 6.5' is redundant since 'x < 5.5' is sufficient. [redundantCondition]\n"
                      "[test.cpp:5:17]: (style) Redundant condition: The condition 'x < 5.5' is redundant since 'x < 6.5' is sufficient. [redundantCondition]\n",
                      errout_str());

        check("void f(const char *p) {\n" // #10320
              "    if (!p || !*p || *p != 'x') {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:19]: (style) Redundant condition: The condition '!*p' is redundant since '*p != 'x'' is sufficient. [redundantCondition]\n",
                      errout_str());
    }

    void incorrectLogicOp_condSwapping() {
        check("void f(int x) {\n"
              "    if (x < 1 && x > 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (1 > x && x > 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x < 1 && 3 < x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (1 > x && 3 < x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x < 1 && x > 3. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 3 && x < 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x > 3 && x < 1. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (3 < x && x < 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x > 3 && x < 1. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 3 && 1 > x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x > 3 && x < 1. [incorrectLogicOperator]\n", errout_str());

        check("void f(int x) {\n"
              "    if (3 < x && 1 > x)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Logical conjunction always evaluates to false: x > 3 && x < 1. [incorrectLogicOperator]\n", errout_str());
    }

    void modulo() {
        check("bool f(bool& b1, bool& b2, bool& b3) {\n"
              "    b1 = a % 5 == 4;\n"
              "    b2 = a % c == 100000;\n"
              "    b3 = a % 5 == c;\n"
              "    return a % 5 == 5-p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(bool& b1, bool& b2, bool& b3, bool& b4, bool& b5) {\n"
              "    b1 = a % 5 < 5;\n"
              "    b2 = a % 5 <= 5;\n"
              "    b3 = a % 5 == 5;\n"
              "    b4 = a % 5 != 5;\n"
              "    b5 = a % 5 >= 5;\n"
              "    return a % 5 > 5;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:16]: (warning) Comparison of modulo result is predetermined, because it is always less than 5. [moduloAlwaysTrueFalse]\n"
            "[test.cpp:3:16]: (warning) Comparison of modulo result is predetermined, because it is always less than 5. [moduloAlwaysTrueFalse]\n"
            "[test.cpp:4:16]: (warning) Comparison of modulo result is predetermined, because it is always less than 5. [moduloAlwaysTrueFalse]\n"
            "[test.cpp:5:16]: (warning) Comparison of modulo result is predetermined, because it is always less than 5. [moduloAlwaysTrueFalse]\n"
            "[test.cpp:6:16]: (warning) Comparison of modulo result is predetermined, because it is always less than 5. [moduloAlwaysTrueFalse]\n"
            "[test.cpp:7:18]: (warning) Comparison of modulo result is predetermined, because it is always less than 5. [moduloAlwaysTrueFalse]\n",
            errout_str());

        check("void f(bool& b1, bool& b2) {\n"
              "    b1 = bar() % 5 < 889;\n"
              "    if(x[593] % 5 <= 5)\n"
              "        b2 = x.a % 5 == 5;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:20]: (warning) Comparison of modulo result is predetermined, because it is always less than 5. [moduloAlwaysTrueFalse]\n"
            "[test.cpp:3:19]: (warning) Comparison of modulo result is predetermined, because it is always less than 5. [moduloAlwaysTrueFalse]\n"
            "[test.cpp:4:22]: (warning) Comparison of modulo result is predetermined, because it is always less than 5. [moduloAlwaysTrueFalse]\n",
            errout_str());

        check("void f() {\n"
              "    if (a % 2 + b % 2 == 2)\n"
              "        foo();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void oppositeInnerCondition() {
        check("void foo(int a, int b) {\n"
              "    if(a==b)\n"
              "        if(a!=b)\n"
              "            cout << a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:13]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("bool foo(int a, int b) {\n"
              "    if(a==b)\n"
              "        return a!=b;\n"
              "    return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:17]: (warning) Opposite inner 'return' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void foo(int a, int b) {\n"
              "    if(a==b)\n"
              "        if(b!=a)\n"
              "            cout << a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:13]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void foo(int a) {\n"
              "    if(a >= 50) {\n"
              "        if(a < 50)\n"
              "            cout << a;\n"
              "        else\n"
              "            cout << 100;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10] -> [test.cpp:3:14]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        // #4186
        check("void foo(int a) {\n"
              "    if(a >= 50) {\n"
              "        if(a > 50)\n"
              "            cout << a;\n"
              "        else\n"
              "            cout << 100;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // 4170
        check("class foo {\n"
              "    void bar() {\n"
              "        if (tok == '(') {\n"
              "            next();\n"
              "            if (tok == ',') {\n"
              "                next();\n"
              "                if (tok != ',') {\n"
              "                    op->reg2 = asm_parse_reg();\n"
              "                }\n"
              "                skip(',');\n"
              "            }\n"
              "        }\n"
              "    }\n"
              "    void next();\n"
              "    const char *tok;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int i)\n"
              "{\n"
              "   if(i > 5) {\n"
              "       i = bar();\n"
              "       if(i < 5) {\n"
              "           cout << a;\n"
              "       }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int& i) {\n"
              "    i=6;\n"
              "}\n"
              "void bar(int i) {\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int& i);\n"
              "void bar() {\n"
              "    int i; i = func();\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int i);\n"
              "void bar(int i) {\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:5:13]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void foo(const int &i);\n"
              "void bar(int i) {\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:5:13]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void foo(int i);\n"
              "void bar() {\n"
              "    int i; i = func();\n"
              "    if(i>5) {\n"
              "        foo(i);\n"
              "        if(i<5) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:6:13]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("class C { void f(int &i) const; };\n" // #7028 - variable is changed by const method
              "void foo(C c, int i) {\n"
              "  if (i==5) {\n"
              "    c.f(i);\n"
              "    if (i != 5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // see linux revision 1f80c0cc
        check("int generic_write_sync(int,int,int);\n"
              "\n"
              "void cifs_writev(int i) {\n"
              "   int rc = __generic_file_aio_write();\n"
              "   if (rc > 0){\n"
              "       err = generic_write_sync(file, iocb->ki_pos - rc, rc);\n"
              "       if(rc < 0) {\n"  // <- condition is always false
              "           err = rc;\n"
              "       }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:11] -> [test.cpp:7:14]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());


        // #5874 - array
        check("void testOppositeConditions2() {\n"
              "  int array[2] = { 0, 0 };\n"
              "  if (array[0] < 2) {\n"
              "    array[0] += 5;\n"
              "    if (array[0] > 2) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6227 - FP caused by simplifications of casts and known variables
        check("void foo(A *a) {\n"
              "   if(a) {\n"
              "       B *b = dynamic_cast<B*>(a);\n"
              "       if(!b) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a) {\n"
              "   if(a) {\n"
              "       int b = a;\n"
              "       if(!b) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:16] -> [test.cpp:2:7] -> [test.cpp:4:11]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void foo(unsigned u) {\n"
              "  if (u != 0) {\n"
              "    for (int i=0; i<32; i++) {\n"
              "      if (u == 0) {}\n"  // <- don't warn
              "      u = x;\n"
              "    }\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8186
        check("void f() {\n"
              "  for (int i=0;i<4;i++) {\n"
              "    if (i==5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17] -> [test.cpp:3:10]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        // #8938
        check("void Delete(SS_CELLCOORD upperleft) {\n"
              "    if ((upperleft.Col == -1) && (upperleft.Row == -1)) {\n"
              "        GetActiveCell(&(upperleft.Col), &(upperleft.Row));\n"
              "        if (upperleft.Row == -1) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9702
        check("struct A {\n"
              "    void DoTest() {\n"
              "        if (!IsSet()) {\n"
              "            m_value = true;\n"
              "            if (IsSet());\n"
              "        }\n"
              "    }\n"
              "    bool IsSet() const { return m_value; }\n"
              "    bool m_value = false;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        // #12725
        check("bool f(bool b) {\n"
              "    if (b)\n"
              "        return !b;\n"
              "    b = g();\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:16]: (style) Return value '!b' is always false [knownConditionTrueFalse]\n", errout_str());
    }

    void oppositeInnerConditionPointers() {
        check("void f(struct ABC *abc) {\n"
              "   struct AB *ab = abc->ab;\n"
              "   if (ab->a == 123){\n"
              "       do_something(abc);\n" // might change ab->a
              "       if (ab->a != 123) {\n"
              "           err = rc;\n"
              "       }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void Fred::f() {\n" // daca: ace
              "  if (this->next_ == map_man_->table_) {\n"
              "    this->next_ = n;\n"
              "    if (this->next_ != map_man_->table_) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test(float *f) {\n" // #7405
              "  if(*f>10) {\n"
              "    (*f) += 0.1f;\n"
              "    if(*f<10) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int * f(int * x, int * y) {\n"
              "    if(!x) return x;\n"
              "    return y;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void oppositeInnerConditionClass() {
        // #6095 - calling member function that might change the state
        check("void f() {\n"
              "  const Fred fred;\n" // <- fred is const, warn
              "  if (fred.isValid()) {\n"
              "    fred.dostuff();\n"
              "    if (!fred.isValid()) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:19] -> [test.cpp:5:9]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("class Fred { public: bool isValid() const; void dostuff() const; };\n"
              "void f() {\n"
              "  Fred fred;\n"
              "  if (fred.isValid()) {\n"
              "    fred.dostuff();\n" // <- dostuff() is const, warn
              "    if (!fred.isValid()) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:19] -> [test.cpp:6:9]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f() {\n"
              "  Fred fred;\n"
              "  if (fred.isValid()) {\n"
              "    fred.dostuff();\n"
              "    if (!fred.isValid()) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6385 "crash in Variable::getFlag()"
        check("class TranslationHandler {\n"
              "QTranslator *mTranslator;\n"
              "void SetLanguage() {\n"
              "   if (mTranslator) {\n"
              "             qApp->removeTranslator(mTranslator);\n"
              "        }\n"
              "   }\n"
              "};");
        ASSERT_EQUALS("", errout_str()); // just don't crash...

        check("bool f(std::ofstream &CFileStream) {\n" // #8198
              "  if(!CFileStream.good()) { return; }\n"
              "  CFileStream << \"abc\";\n"
              "  if (!CFileStream.good()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void oppositeInnerConditionUndeclaredVariable() {
        // #5731 - fp when undeclared variable is used
        check("void f() {\n"
              "   if (x == -1){\n"
              "       x = do_something();\n"
              "       if (x != -1) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5750 - another fp when undeclared variable is used
        check("void f() {\n"
              "   if (r < w){\n"
              "       r += 3;\n"
              "       if (r > w) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6574 - another fp when undeclared variable is used
        check("void foo() {\n"
              "   if(i) {\n"
              "       i++;\n"
              "       if(!i) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // undeclared array
        check("void f(int x) {\n"
              "  if (a[x] > 0) {\n"
              "    a[x] -= dt;\n"
              "    if (a[x] < 0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6313 - false positive: opposite conditions in nested if blocks when condition changed
        check("void Foo::Bar() {\n"
              "   if(var){\n"
              "      --var;\n"
              "      if(!var){}\n"
              "      else {}\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // daca hyphy
        check("bool f() {\n"
              "  if (rec.lLength==0) {\n"
              "    rec.Delete(i);\n"
              "    if (rec.lLength!=0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void oppositeInnerConditionAlias() {
        check("void f() {\n"
              "  struct S s;\n"
              "  bool hasFailed = false;\n"
              "  s.status = &hasFailed;\n"
              "\n"
              "  if (! hasFailed) {\n"
              "    doStuff(&s);\n"
              "    if (hasFailed) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:7]: (style) Condition '!hasFailed' is always true [knownConditionTrueFalse]\n", errout_str());
    }

    void oppositeInnerCondition2() {
        // first comparison: <
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x==5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n",
                      errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x!=5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x!=5' is always true [knownConditionTrueFalse]\n", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x>5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n",
                      errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x>=5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n",
                      errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x<5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x<5' is always true [knownConditionTrueFalse]\n", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<4) {\n"
              "    if (x<=5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x<=5' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x==4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x!=4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x!=6) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x!=6' is always true [knownConditionTrueFalse]\n", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x>4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x>4' is always false [knownConditionTrueFalse]\n", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x>=4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x<4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x<5) {\n"
              "    if (x<=4) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x<=4' is always true [knownConditionTrueFalse]\n", errout_str());

        // first comparison: >
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x==5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x>5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x>=5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x>=5' is always true [knownConditionTrueFalse]\n", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x<5) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x<5' is always false [knownConditionTrueFalse]\n", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>4) {\n"
              "    if (x<=5) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x==4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n",
                      errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x>4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x>4' is always true [knownConditionTrueFalse]\n", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x>=4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (style) Condition 'x>=4' is always true [knownConditionTrueFalse]\n", errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x<4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n",
                      errout_str());
        check("void f(int x) {\n"
              "\n"
              "  if (x>5) {\n"
              "    if (x<=4) {}\n" // <- Warning
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:4:10]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n",
                      errout_str());

        check("void f(int x) {\n"
              "  if (x < 4) {\n"
              "    if (10 < x) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:12]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());
    }

    void oppositeInnerCondition3() {
        check("void f3(char c) { if(c=='x') if(c=='y') {}}");
        ASSERT_EQUALS("[test.cpp:1:23] -> [test.cpp:1:34]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f4(char *p) { if(*p=='x') if(*p=='y') {}}");
        ASSERT_EQUALS("[test.cpp:1:25] -> [test.cpp:1:37]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f5(const char * const p) { if(*p=='x') if(*p=='y') {}}");
        ASSERT_EQUALS("[test.cpp:1:38] -> [test.cpp:1:50]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f5(const char * const p) { if('x'==*p) if('y'==*p) {}}");
        ASSERT_EQUALS("[test.cpp:1:39] -> [test.cpp:1:51]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f6(char * const p) { if(*p=='x') if(*p=='y') {}}");
        ASSERT_EQUALS("[test.cpp:1:32] -> [test.cpp:1:44]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f7(const char * p) { if(*p=='x') if(*p=='y') {}}");
        ASSERT_EQUALS("[test.cpp:1:32] -> [test.cpp:1:44]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f8(int i) { if(i==4) if(i==2) {}}");
        ASSERT_EQUALS("[test.cpp:1:22] -> [test.cpp:1:31]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f9(int *p) { if (*p==4) if(*p==2) {}}");
        ASSERT_EQUALS("[test.cpp:1:25] -> [test.cpp:1:35]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f10(int * const p) { if (*p==4) if(*p==2) {}}");
        ASSERT_EQUALS("[test.cpp:1:33] -> [test.cpp:1:43]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f11(const int *p) { if (*p==4) if(*p==2) {}}");
        ASSERT_EQUALS("[test.cpp:1:32] -> [test.cpp:1:42]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f12(const int * const p) { if (*p==4) if(*p==2) {}}");
        ASSERT_EQUALS("[test.cpp:1:39] -> [test.cpp:1:49]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("struct foo {\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "void f(foo x) { if(x.a==4) if(x.b==2) {}}");
        ASSERT_EQUALS("", errout_str());

        check("struct foo {\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "void f(foo x) { if(x.a==4) if(x.b==4) {}}");
        ASSERT_EQUALS("", errout_str());

        check("void f3(char a, char b) { if(a==b) if(a==0) {}}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) { if (x == 1) if (x != 1) {} }");
        ASSERT_EQUALS("[test.cpp:1:23] -> [test.cpp:1:35]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());
    }

    void oppositeInnerConditionAnd() {
        check("void f(int x) {\n"
              "  if (a>3 && x > 100) {\n"
              "    if (x < 10) {}\n"
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16] -> [test.cpp:3:11]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f(bool x, const int a, const int b) {\n"
              "        if(x && a < b)\n"
              "            if( x && a > b){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:14] -> [test.cpp:3:19]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());
    }

    void oppositeInnerConditionOr()
    {
        check("void f(int x) {\n"
              "    if (x == 1 || x == 2) {\n"
              "        if (x == 3) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:16] -> [test.cpp:3:15]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n",
                      errout_str());

        check("void f(int x) {\n"
              "    if (x == 1 || x == 2) {\n"
              "        if (x == 1) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (x == 1 || x == 2) {\n"
              "        if (x == 2) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string x) {\n"
              "    if (x == \"1\" || x == \"2\") {\n"
              "        if (x == \"1\") {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (x < 1 || x > 3) {\n"
              "        if (x == 3) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:15] -> [test.cpp:3:15]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n",
                      errout_str());
    }

    void oppositeInnerConditionEmpty() {
        check("void f1(const std::string &s) { if(s.size() > 42) if(s.empty()) {}}");
        ASSERT_EQUALS("[test.cpp:1:45] -> [test.cpp:1:61]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f1(const std::string &s) { if(s.size() > 0) if(s.empty()) {}}");
        ASSERT_EQUALS("[test.cpp:1:45] -> [test.cpp:1:60]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f1(const std::string &s) { if(s.size() < 0) if(s.empty()) {}} "); // <- CheckOther reports: checking if unsigned expression is less than zero
        ASSERT_EQUALS("[test.cpp:1:45] -> [test.cpp:1:60]: (style) Condition 's.empty()' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f1(const std::string &s) { if(s.empty()) if(s.size() > 42) {}}");
        ASSERT_EQUALS("[test.cpp:1:43] -> [test.cpp:1:59]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("template<class T> void f1(const T &s) { if(s.size() > 42) if(s.empty()) {}}");
        ASSERT_EQUALS("", errout_str()); //We don't know the type of T so we don't know the relationship between size() and empty(). e.g. s might be a 50 tonne truck with nothing in it.

        check("void f2(const std::wstring &s) { if(s.empty()) if(s.size() > 42) {}}");
        ASSERT_EQUALS("[test.cpp:1:44] -> [test.cpp:1:60]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f1(QString s) { if(s.isEmpty()) if(s.length() > 42) {}}");
        ASSERT_EQUALS("[test.cpp:1:34] -> [test.cpp:1:52]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f1(const std::string &s, bool b) { if(s.empty() || ((s.size() == 1) && b)) {}}");
        ASSERT_EQUALS("", errout_str());

        check("void f1(const std::string &x, const std::string &y) { if(x.size() > 42) if(y.empty()) {}}");
        ASSERT_EQUALS("", errout_str());

        check("void f1(const std::string &x, const std::string &y) { if(y.empty()) if(x.size() > 42) {}}");
        ASSERT_EQUALS("", errout_str());

        check("void f1(const std::string v[10]) { if(v[0].size() > 42) if(v[1].empty()) {}}");
        ASSERT_EQUALS("", errout_str());

        check("void f1(const std::string &s) { if(s.size() <= 1) if(s.empty()) {}}");
        ASSERT_EQUALS("", errout_str());

        check("void f1(const std::string &s) { if(s.size() <= 2) if(s.empty()) {}}");
        ASSERT_EQUALS("", errout_str());

        check("void f1(const std::string &s) { if(s.size() < 2) if(s.empty()) {}}");
        ASSERT_EQUALS("", errout_str());

        check("void f1(const std::string &s) { if(s.size() >= 0) if(s.empty()) {}} "); // CheckOther says: Unsigned expression 's.size()' can't be negative so it is unnecessary to test it. [unsignedPositive]
        ASSERT_EQUALS("", errout_str());

        check("void f1(const std::string &s) { if(42 < s.size()) if(s.empty()) {}}");
        ASSERT_EQUALS("[test.cpp:1:39] -> [test.cpp:1:61]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        check("void f1(const std::string &s) { if(s.empty()) if(42 < s.size()) {}}");
        ASSERT_EQUALS("[test.cpp:1:43] -> [test.cpp:1:53]: (warning) Opposite inner 'if' condition leads to a dead code block. [oppositeInnerCondition]\n", errout_str());

        // TODO: These are identical condition since size cannot be negative
        check("void f1(const std::string &s) { if(s.size() <= 0) if(s.empty()) {}}");
        ASSERT_EQUALS("", errout_str());

        // TODO: These are identical condition since size cannot be negative
        check("void f1(const std::string &s) { if(s.size() < 1) if(s.empty()) {}}");
        ASSERT_EQUALS("", errout_str());
    }

    void oppositeInnerConditionFollowVar() {
        check("struct X {\n"
              "    void f() {\n"
              "        const int flag = get();\n"
              "        if (flag) {\n"
              "            bar();\n"
              "            if (!get()) {}\n"
              "        }\n"
              "    }\n"
              "    void bar();\n"
              "    int get() const;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct CD {\n"
              "    bool state;\n"
              "    void foo()  {\n"
              "        const bool flag = this->get();\n"
              "        if (flag) {\n"
              "            this->bar();\n"
              "            if (!this->get()) return;\n"
              "        }\n"
              "    }\n"
              "    bool get() const;\n"
              "    void bar();\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n"
              "public:\n"
              "  bool f() const { return x > 0; }\n"
              "  void g();\n"
              "  int x = 0;\n"
              "};\n"
              "\n"
              "void C::g() {\n"
              "  bool b = f();\n"
              "  x += 1;\n"
              "  if (!b && f()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(double d) {\n"
              "    if (d != 0) {\n"
              "        int i = d;\n"
              "        if (i == 0) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void oppositeInnerConditionLambda() {
        check("void f() {\n" // #13728
              "    for (int i = 0; i < 2;) {\n"
              "        auto inc = [&]() {\n"
              "            if (i >= 2)\n"
              "                throw 0;\n"
              "            return i++;\n"
              "        };\n"
              "        inc();\n"
              "        inc();\n"
              "        inc();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void identicalInnerCondition() {
        check("void f1(int a, int b) { if(a==b) if(a==b) {}}");
        ASSERT_EQUALS("[test.cpp:1:29] -> [test.cpp:1:38]: (warning) Identical inner 'if' condition is always true. [identicalInnerCondition]\n", errout_str());

        check("void f2(int a, int b) { if(a!=b) if(a!=b) {}}");
        ASSERT_EQUALS("[test.cpp:1:29] -> [test.cpp:1:38]: (warning) Identical inner 'if' condition is always true. [identicalInnerCondition]\n", errout_str());

        // #6645 false negative: condition is always false
        check("void f(bool a, bool b) {\n"
              "  if(a && b) {\n"
              "     if(a) {}\n"
              "     else  {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:6] -> [test.cpp:3:9]: (warning) Identical inner 'if' condition is always true. [identicalInnerCondition]\n", errout_str());

        check("bool f(int a, int b) {\n"
              "    if(a == b) { return a == b; }\n"
              "    return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10] -> [test.cpp:2:27]: (warning) Identical inner 'return' condition is always true. [identicalInnerCondition]\n", errout_str());

        check("bool f(bool a) {\n"
              "    if(a) { return a; }\n"
              "    return false;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int* f(int* a, int * b) {\n"
              "    if(a) { return a; }\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int* f(std::shared_ptr<int> a, std::shared_ptr<int> b) {\n"
              "    if(a.get()) { return a.get(); }\n"
              "    return b.get();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int * x; };\n"
              "int* f(A a, int * b) {\n"
              "    if(a.x) { return a.x; }\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    uint32_t value;\n"
              "    get_value(&value);\n"
              "    int opt_function_capable = (value >> 28) & 1;\n"
              "    if (opt_function_capable) {\n"
              "        value = 0;\n"
              "        get_value (&value);\n"
              "        if ((value >> 28) & 1) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        checkP("#define TYPE_1 \"a\"\n" // #13202
               "#define TYPE_2 \"b\"\n"
               "#define TYPE_3 \"c\"\n"
               "void f(const std::string& s) {\n"
               "    if (s == TYPE_1) {}\n"
               "    else if (s == TYPE_2 || s == TYPE_3) {\n"
               "        if (s == TYPE_2) {}\n"
               "        else if (s == TYPE_3) {}\n"
               "    }\n"
               "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const int* p, const int* e) {\n" // #14595
              "    for (; p;) {\n"
              "        if (p == e) {}\n"
              "        if (p) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:4:13]: (warning) Identical inner 'if' condition is always true. [identicalInnerCondition]\n", errout_str());
    }

    void identicalConditionAfterEarlyExit() {
        check("void f(int x) {\n" // #8137
              "  if (x > 100) { return; }\n"
              "  if (x > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:9]: (warning) Identical condition 'x>100', second condition is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        check("bool f(int x) {\n"
              "  if (x > 100) { return false; }\n"
              "  return x > 100;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:12]: (warning) Identical condition and return expression 'x>100', return value is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        check("void f(int x) {\n"
              "  if (x > 100) { return; }\n"
              "  if (x > 100 || y > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:9]: (warning) Identical condition 'x>100', second condition is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        check("void f(int x) {\n"
              "  if (x > 100) { return; }\n"
              "  if (x > 100 && y > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:9]: (warning) Identical condition 'x>100', second condition is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        check("void f(int x) {\n"
              "  if (x > 100) { return; }\n"
              "  if (abc) {}\n"
              "  if (x > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:4:9]: (warning) Identical condition 'x>100', second condition is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        check("void f(int x) {\n"
              "  if (x > 100) { return; }\n"
              "  while (abc) { y = x; }\n"
              "  if (x > 100) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:4:9]: (warning) Identical condition 'x>100', second condition is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        ASSERT_THROW_INTERNAL(check("void f(int x) {\n"  // #8217 - crash for incomplete code
                                    "  if (x > 100) { return; }\n"
                                    "  X(do);\n"
                                    "  if (x > 100) {}\n"
                                    "}"),
                              SYNTAX);

        check("void f(const int *i) {\n"
              "  if (!i) return;\n"
              "  if (!num1tok) { *num1 = *num2; }\n"
              "  if (!i) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7] -> [test.cpp:4:7]: (warning) Identical condition '!i', second condition is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        check("void C::f(Tree &coreTree) {\n" // daca
              "  if(!coreTree.build())\n"
              "    return;\n"
              "  coreTree.dostuff();\n"
              "  if(!coreTree.build()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct C { void f(const Tree &coreTree); };\n"
              "void C::f(const Tree &coreTree) {\n"
              "  if(!coreTree.build())\n"
              "    return;\n"
              "  coreTree.dostuff();\n"
              "  if(!coreTree.build()) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:6] -> [test.cpp:6:6]: (warning) Identical condition '!coreTree.build()', second condition is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        check("void f(int x) {\n" // daca: labplot
              "  switch(type) {\n"
              "  case 1:\n"
              "    if (x == 0) return 1;\n"
              "    else return 2;\n"
              "  case 2:\n"
              "    if (x == 0) return 3;\n"
              "    else return 4;\n"
              "  }\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("static int failed = 0;\n"
              "void f() {\n"
              "  if (failed) return;\n"
              "  checkBuffer();\n"
              "  if (failed) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // daca icu
        check("void f(const uint32_t *section, int32_t  start) {\n"
              "  if(10<=section[start]) { return; }\n"
              "  if(++start<100 && 10<=section[start]) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // daca iqtree
        check("void readNCBITree(std::istream &in) {\n"
              "  char ch;\n"
              "  in >> ch;\n"
              "  if (ch != '|') return;\n"
              "  in >> ch;\n"
              "  if (ch != '|') {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8924
        check("struct A {\n"
              "    void f() {\n"
              "        if (this->FileIndex >= 0) return;\n"
              "        this->FileIndex = 1 ;\n"
              "        if (this->FileIndex < 0) return;\n"
              "    }\n"
              "    int FileIndex;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:29]: (style) Condition 'this->FileIndex<0' is always false [knownConditionTrueFalse]\n", errout_str());

        // #8858 - #if
        check("short Do() {\n"
              "    short ret = bar1();\n"
              "    if ( ret )\n"
              "        return ret;\n"
              "#ifdef FEATURE\n"
              "    ret = bar2();\n"
              "#endif\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #10456
        check("int f() {\n"
              "    int i = 0;\n"
              "    auto f = [&](bool b) { if (b) ++i; };\n"
              "    if (i) return i;\n"
              "    f(true);\n"
              "    if (i) return i;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11478
        check("struct S {\n"
              "    void run();\n"
              "    bool b = false;\n"
              "    const std::function<void(S&)> f;\n"
              "};\n"
              "void S::run() {\n"
              "    while (true) {\n"
              "        if (b)\n"
              "            return;\n"
              "        f(*this);\n"
              "        if (b)\n"
              "            return;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void innerConditionModified() {
        check("void f(int x, int y) {\n"
              "  if (x == 0) {\n"
              "    x += y;\n"
              "    if (x == 0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "  if (x == 0) {\n"
              "    x += y;\n"
              "    if (x == 1) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int * x, int * y) {\n"
              "  if (x[*y] == 0) {\n"
              "    (*y)++;\n"
              "    if (x[*y] == 0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // clarify conditions with = and comparison
    void clarifyCondition1() {
        check("void f() {\n"
              "    if (x = b() < 0) {}\n" // don't simplify and verify this code
              "}");
        ASSERT_EQUALS("[test.cpp:2:8]: (style) Suspicious condition (assignment + comparison); Clarify expression with parentheses. [clarifyCondition]\n", errout_str());

        check("void f(int i) {\n"
              "    for (i = 0; i < 10; i++) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    x = a<int>(); if (x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    if (x = b < 0 ? 1 : 2) {}\n" // don't simplify and verify this code
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int y = rand(), z = rand();\n"
              "    if (y || (!y && z));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Redundant condition: !y. 'y || (!y && z)' is equivalent to 'y || z' [redundantCondition]\n", errout_str());

        check("void f() {\n"
              "    int y = rand(), z = rand();\n"
              "    if (y || !y && z);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Redundant condition: !y. 'y || (!y && z)' is equivalent to 'y || z' [redundantCondition]\n", errout_str());

        check("void f() {\n"
              "    if (!a || a && b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (style) Redundant condition: a. '!a || (a && b)' is equivalent to '!a || b' [redundantCondition]\n", errout_str());


        check("void f(const Token *tok) {\n"
              "    if (!tok->next()->function() ||\n"
              "        (tok->next()->function() && tok->next()->function()->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:34]: (style) Redundant condition: tok->next()->function(). '!A || (A && B)' is equivalent to '!A || B' [redundantCondition]\n", errout_str());

        check("void f() {\n"
              "    if (!tok->next()->function() ||\n"
              "        (!tok->next()->function() && tok->next()->function()->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    if (!tok->next()->function() ||\n"
              "        (!tok2->next()->function() && tok->next()->function()->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const Token *tok) {\n"
              "    if (!tok->next(1)->function(1) ||\n"
              "        (tok->next(1)->function(1) && tok->next(1)->function(1)->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:36]: (style) Redundant condition: tok->next(1)->function(1). '!A || (A && B)' is equivalent to '!A || B' [redundantCondition]\n", errout_str());

        check("void f() {\n"
              "    if (!tok->next()->function(1) ||\n"
              "        (tok->next()->function(2) && tok->next()->function()->isConstructor()));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "   int y = rand(), z = rand();\n"
              "   if (y==0 || y!=0 && z);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Redundant condition: y!=0. 'y==0 || (y!=0 && z)' is equivalent to 'y==0 || z' [redundantCondition]\n", errout_str());

        check("void f() {\n"
              "  if (x>0 || (x<0 && y)) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Test Token::expressionString, TODO move this test
        check("void f() {\n"
              "  if (!dead || (dead && (*it).ticks > 0)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Redundant condition: dead. '!dead || (dead && (*it).ticks>0)' is equivalent to '!dead || (*it).ticks>0' [redundantCondition]\n", errout_str());

        check("void f() {\n"
              "  if (!x || (x && (2>(y-1)))) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Redundant condition: x. '!x || (x && 2>(y-1))' is equivalent to '!x || 2>(y-1)' [redundantCondition]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "    if (a || (a && b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Redundant condition: a. 'a || (a && b)' is equivalent to 'a' [redundantCondition]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "    if (a && (a || b)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Redundant condition: a. 'a && (a || b)' is equivalent to 'a' [redundantCondition]\n", errout_str());
    }

    // clarify conditions with bitwise operator and comparison
    void clarifyCondition2() {
        check("void f() {\n"
              "    if (x & 3 == 2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:8]: (style) Suspicious condition (bitwise operator + comparison); Clarify expression with parentheses. [clarifyCondition]\n"
                      "[test.cpp:2:11]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses. [clarifyCondition]\n"
                      "[test.cpp:2:11]: (style) Condition 'x&3==2' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "    if (a & fred1.x == fred2.y) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:8]: (style) Suspicious condition (bitwise operator + comparison); Clarify expression with parentheses. [clarifyCondition]\n"
                      "[test.cpp:2:11]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses. [clarifyCondition]\n"
                      , errout_str());
    }

    // clarify condition that uses ! operator and then bitwise operator
    void clarifyCondition3() {
        check("void f(int w) {\n"
              "    if(!w & 0x8000) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses. [clarifyCondition]\n", errout_str());

        check("void f(int w) {\n"
              "    if((!w) & 0x8000) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    if (x == foo() & 2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses. [clarifyCondition]\n", errout_str());

        check("void f() {\n"
              "    if (2 & x == foo()) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses. [clarifyCondition]\n", errout_str());

        check("void f() {\n"
              "    if (2 & (x == foo())) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::list<int> &ints) { }");
        ASSERT_EQUALS("", errout_str());

        check("void f() { A<x &> a; }");
        ASSERT_EQUALS("", errout_str());

        check("void f() { a(x<y|z,0); }", dinit(CheckOptions, $.cpp = false));  // language is c => there are never templates
        ASSERT_EQUALS("[test.c:1:17]: (style) Boolean result is used in bitwise operation. Clarify expression with parentheses. [clarifyCondition]\n", errout_str());

        check("class A<B&,C>;");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    if (result != (char *)&inline_result) { }\n" // don't simplify and verify cast
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8495
        check("void f(bool a, bool b) {\n"
              "    C & a & b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void clarifyCondition4() { // ticket #3110
        check("typedef double SomeType;\n"
              "typedef std::pair<std::string,SomeType> PairType;\n"
              "struct S\n"
              "{\n"
              "     bool operator()\n"
              "         ( PairType const & left\n"
              "         , PairType const & right) const\n"
              "     {\n"
              "         return left.first < right.first;\n"
              "     }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void clarifyCondition5() { // ticket #3609 (using | in template instantiation)
        check("template<bool B> struct CWinTraits;\n"
              "CWinTraits<WS_CHILD|WS_VISIBLE>::GetWndStyle(0);");
        ASSERT_EQUALS("", errout_str());
    }

    void clarifyCondition6() {
        check("template<class Y>\n"
              "SharedPtr& operator=( SharedPtr<Y> const & r ) {\n"
              "    px = r.px;\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void clarifyCondition7() {
        // Ensure that binary and unary &, and & in declarations are distinguished properly
        check("void f(bool error) {\n"
              "    bool & withoutSideEffects=found.first->second;\n" // Declaring a reference to a boolean; & is no operator at all
              "    execute(secondExpression, &programMemory, &result, &error);\n" // Unary &
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void clarifyCondition8() {
        // don't warn when boolean result comes from function call, array index, etc
        // the operator precedence is not unknown then
        check("bool a();\n"
              "bool f(bool b) {\n"
              "    return (a() & b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(bool *a, bool b) {\n"
              "    return (a[10] & b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { bool a; };\n"
              "bool f(struct A a, bool b) {\n"
              "    return (a.a & b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { bool a; };\n"
              "bool f(struct A a, bool b) {\n"
              "    return (A::a & b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testBug5895() {
        check("void png_parse(uint64_t init, int buf_size) {\n"
              "    if (init == 0x89504e470d0a1a0a || init == 0x8a4d4e470d0a1a0a)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testBug5309() {
        check("extern uint64_t value;\n"
              "void foo() {\n"
              "    if( ( value >= 0x7ff0000000000001ULL )\n"
              "            && ( value <= 0x7fffffffffffffffULL ) );\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void alwaysTrue() {

        check("void f(const struct S *s) {\n" //#8196
              "  int x1 = s->x;\n"
              "  int x2 = s->x;\n"
              "  if (x1 == 10 && x2 == 10) {}\n" // <<
              "}");
        ASSERT_EQUALS("[test.cpp:4:10] -> [test.cpp:4:22]: (style) Condition 'x2==10' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f ()\n"// #8220
              "{\n"
              "    int a;\n"
              "    int b = 0;\n"
              "    int ret;\n"
              " \n"
              "    a = rand();\n"
              "    while (((0 < a) && (a < 2)) && ((8 < a) && (a < 10))) \n"
              "    {\n"
              "        b += a;\n"
              "        a ++;\n"
              "    }\n"
              "    ret = b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:27] -> [test.cpp:8:40]: (style) Condition '8<a' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n" // #4842
              "  int x = 0;\n"
              "  if (a) { return; }\n" // <- this is just here to fool simplifyKnownVariabels
              "  if (!x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Condition '!x' is always true [knownConditionTrueFalse]\n", errout_str());

        check("bool f(int x) {\n"
              "  if(x == 0) { x++; return x == 0; }\n"
              "  return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:8] -> [test.cpp:2:30]: (style) Return value 'x==0' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n" // #6898 (Token::expressionString)
              "  int x = 0;\n"
              "  A(x++ == 1);\n"
              "  A(x++ == 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Condition 'x++==1' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:4:9]: (style) Condition 'x++==2' is always false [knownConditionTrueFalse]\n",
                      errout_str());

        check("bool foo(int bar) {\n"
              "  bool ret = false;\n"
              "  if (bar == 1)\n"
              "    return ret;\n" // <- #9326 - FP condition is always false
              "  if (bar == 2)\n"
              "    ret = true;\n"
              "  return ret;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f1(const std::string &s) { if(s.empty()) if(s.size() == 0) {}}");
        ASSERT_EQUALS("[test.cpp:1:43] -> [test.cpp:1:59]: (style) Condition 's.size()==0' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   if( buf != 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) Condition 'buf!=0' is always true [knownConditionTrueFalse]\n", errout_str()); // #8924

        check("void f() {\n"
              "   int buf[42];\n"
              "   if( !buf ) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8]: (style) Condition '!buf' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   bool b = buf;\n"
              "   if( b ) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:8]: (style) Condition 'b' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   bool b = buf;\n"
              "   if( !b ) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:8]: (style) Condition '!b' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   int * p = nullptr;\n"
              "   if( buf == p ) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Condition 'buf==p' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(bool x) {\n"
              "   int buf[42];\n"
              "   if( buf || x ) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8]: (style) Condition 'buf' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int * p) {\n"
              "   int buf[42];\n"
              "   if( buf == p ) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   int p[42];\n"
              "   if( buf == p ) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "   int buf[42];\n"
              "   if( buf == 1) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Avoid FP when condition comes from macro
        check("#define NOT !\n"
              "void f() {\n"
              "  int x = 0;\n"
              "  if (a) { return; }\n" // <- this is just here to fool simplifyKnownVariabels
              "  if (NOT x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("#define M  x != 0\n"
              "void f() {\n"
              "  int x = 0;\n"
              "  if (a) { return; }\n" // <- this is just here to fool simplifyKnownVariabels
              "  if (M) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("#define IF(X)  if (X && x())\n"
              "void f() {\n"
              "  IF(1) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Avoid FP for sizeof condition
        check("void f() {\n"
              "  if (sizeof(char) != 123) {}\n"
              "  if (123 != sizeof(char)) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int x = 123;\n"
              "  if (sizeof(char) != x) {}\n"
              "  if (x != sizeof(char)) {}\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (style) Condition 'sizeof(char)!=x' is always true\n"
                           "[test.cpp:4]: (style) Condition 'x!=sizeof(char)' is always true\n", "", errout_str());

        // Don't warn in assertions. Condition is often 'always true' by intention.
        // If platform,defines,etc cause an 'always false' assertion then that is not very dangerous neither
        check("void f() {\n"
              "  int x = 0;\n"
              "  assert(x == 0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9363 - do not warn about value passed to function
        check("void f(bool b) {\n"
              "    if (b) {\n"
              "        if (bar(!b)) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        // #7783  FP knownConditionTrueFalse on assert(0 && "message")
        check("void foo(int x) {\n"
              "    if (x<0)\n"
              "    {\n"
              "        assert(0 && \"bla\");\n"
              "        ASSERT(0 && \"bla\");\n"
              "        assert_foo(0 && \"bla\");\n"
              "        ASSERT_FOO(0 && \"bla\");\n"
              "        assert((int)(0==0));\n"
              "        assert((int)(0==0) && \"bla\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7750 char literals in boolean expressions
        check("void f() {\n"
              "  if('a'){}\n"
              "  if(L'b'){}\n"
              "  if(1 && 'c'){}\n"
              "  int x = 'd' ? 1 : 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8206 - knownCondition always false
        check("void f(int i)\n"
              "{\n"
              "        if(i > 4)\n"
              "          for( int x = 0; i < 3; ++x){}\n" // <<
              "}");
        ASSERT_EQUALS("[test.cpp:3:14] -> [test.cpp:4:29]: (style) Condition 'i<3' is always false [knownConditionTrueFalse]\n", errout_str());

        // Skip literals
        check("void f() { if(true) {} }");
        ASSERT_EQUALS("", errout_str());

        check("void f() { if(false) {} }");
        ASSERT_EQUALS("", errout_str());

        check("void f() { if(!true) {} }");
        ASSERT_EQUALS("", errout_str());

        check("void f() { if(!false) {} }");
        ASSERT_EQUALS("", errout_str());

        check("void f() { if(0) {} }");
        ASSERT_EQUALS("", errout_str());

        check("void f() { if(1) {} }");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i) {\n"
              "    bool b = false;\n"
              "    if (i == 0) b = true;\n"
              "    else if (!b && i == 1) {}\n"
              "    if (b)\n"
              "    {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:14]: (style) Condition '!b' is always true [knownConditionTrueFalse]\n", errout_str());

        check("bool f() { return nullptr; }");
        ASSERT_EQUALS("", errout_str());

        check("enum E { A };\n"
              "bool f() { return A; }");
        ASSERT_EQUALS("", errout_str());

        check("bool f() {\n"
              "    const int x = 0;\n"
              "    return x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(void){return 1/abs(10);}");
        ASSERT_EQUALS("", errout_str());

        check("bool f() {\n"
              "    int x = 0;\n"
              "    return x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f() {\n"
              "    const int a = 50;\n"
              "    const int b = 52;\n"
              "    return a+b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Return value 'a+b' is always true [knownConditionTrueFalse]\n", errout_str());

        check("int f() {\n"
              "    int a = 50;\n"
              "    int b = 52;\n"
              "    a++;\n"
              "    b++;\n"
              "    return a+b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool& g();\n"
              "bool f() {\n"
              "    bool & b = g();\n"
              "    b = false;\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    bool b;\n"
              "    bool f() {\n"
              "        b = false;\n"
              "        return b;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("bool f(long maxtime) {\n"
              "  if (std::time(0) > maxtime)\n"
              "    return std::time(0) > maxtime;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(double param) {\n"
              "  while(bar()) {\n"
              "    if (param<0.)\n"
              "       return;\n"
              "  }\n"
              "  if (param<0.)\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int i) {\n"
              "  if (i==42)\n"
              "  {\n"
              "    bar();\n"
              "  }\n"
              "  if (cond && (42==i))\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // 8842 crash
        check("class a {\n"
              "  int b;\n"
              "  c(b);\n"
              "  void f() {\n"
              "    if (b) return;\n"
              "  }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("void f(const char* x, const char* t) {\n"
              "    if (!(strcmp(x, y) == 0)) { return; }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const int a[]){ if (a == 0){} }");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "  bool operator<(const S&);\n"
              "};\n"
              "int main() {\n"
              "  S s;\n"
              "  bool c = s<s;\n"
              "  if (c) return 0;\n"
              "  else return 42;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("long X::g(bool unknown, int& result) {\n"
              "    long ret = 0;\n"
              "    bool f = false;\n"
              "    f = f || unknown;\n"
              "    f ? result = 42 : ret = -1;\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(void *handle) {\n"
              "    if (!handle) return 0;\n"
              "    if (handle) return 1;\n"
              "    else return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:9]: (style) Condition 'handle' is always true [knownConditionTrueFalse]\n", errout_str());

        check("int f(void *handle) {\n"
              "    if (handle == 0) return 0;\n"
              "    if (handle) return 1;\n"
              "    else return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Condition 'handle' is always true [knownConditionTrueFalse]\n", errout_str());

        check("int f(void *handle) {\n"
              "    if (handle != 0) return 0;\n"
              "    if (handle) return 1;\n"
              "    else return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16] -> [test.cpp:3:9]: (warning) Identical condition 'handle!=0', second condition is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        check("int f(void *handle) {\n"
              "    if (handle != nullptr) return 0;\n"
              "    if (handle) return 1;\n"
              "    else return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16] -> [test.cpp:3:9]: (warning) Identical condition 'handle!=nullptr', second condition is always false [identicalConditionAfterEarlyExit]\n", errout_str());

        check("void f(const char* p) {\n" // #13716
              "    if (!p) {}\n"
              "    if (p == NULL) {}\n"
              "    if (p == nullptr) {}\n"
              "    if (p == 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:11]: (style) The if condition is the same as the previous if condition [duplicateCondition]\n",
                      errout_str());

        check("int f(const char* p) {\n" // #13717
              "    if (p) {}\n"
              "    else if (!p) {}\n"
              "    else if (p == NULL) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:14]: (style) Expression is always true because 'else if' condition is opposite to previous condition at line 2. [multiCondition]\n"
                      "[test.cpp:4:16]: (style) Expression is always false because 'else if' condition matches previous condition at line 3. [multiCondition]\n",
                      errout_str());

        check("void f(void* x, void* y) {\n"
              "    if (x == nullptr && y == nullptr)\n"
              "        return;\n"
              "    if (x == nullptr || y == nullptr)\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void* g();\n"
              "void f(void* a, void* b) {\n"
              "    while (a) {\n"
              "        a = g();\n"
              "        if (a == b)\n"
              "            break;\n"
              "    }\n"
              "    if (a) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void* g();\n"
              "void f(void* a, void* b) {\n"
              "    while (a) {\n"
              "        a = g();\n"
              "    }\n"
              "    if (a) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12] -> [test.cpp:6:9]: (style) Condition 'a' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(int * x, bool b) {\n"
              "    if (!x && b) {}\n"
              "    else if (x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    const std::string x=\"xyz\";\n"
              "    if(!x.empty()){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8]: (style) Condition '!x.empty()' is always true [knownConditionTrueFalse]\n", errout_str());

        check("std::string g();\n"
              "void f() {\n"
              "    const std::string msg = g();\n"
              "    if(!msg.empty()){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *array, int size ) {\n"
              "    for(int i = 0; i < size; ++i) {\n"
              "        if(array == 0)\n"
              "            continue;\n"
              "        if(array){}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (style) Condition 'array' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int *array, int size ) {\n"
              "    for(int i = 0; i < size; ++i) {\n"
              "        if(array == 0)\n"
              "            continue;\n"
              "        else if(array){}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:17]: (style) Condition 'array' is always true [knownConditionTrueFalse]\n", errout_str());

        // #9277
        check("int f() {\n"
              "    constexpr bool x = true;\n"
              "    if constexpr (x)\n"
              "        return 0;\n"
              "    else\n"
              "        return 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9954
        check("void f() {\n"
              "    const size_t a(8 * sizeof(short));\n"
              "    const size_t b(8 * sizeof(int));\n"
              "    if constexpr (a == 16 && b == 16) {}\n"
              "    else if constexpr (a == 16 && b == 32) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9319
        check("struct S {\n"
              "  int a;\n"
              "  int b;\n"
              "};\n"
              "void g(S s, bool& x);\n"
              "void f() {\n"
              "  bool x = false;\n"
              "  g({0, 1}, x);\n"
              "  if (x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9318
        check("class A {};\n"
              "class B : public A {};\n"
              "void f(A* x) {\n"
              "  if (!x)\n"
              "    return;\n"
              "  auto b = dynamic_cast<B*>(x);\n"
              "  if (b) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int foo() {\n"
              "    auto x = getX();\n"
              "    if (x == nullptr)\n"
              "        return 1;\n"
              "    auto y = dynamic_cast<Y*>(x)\n"
              "    if (y == nullptr)\n"
              "        return 2;\n"
              "    return 3;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // handleKnownValuesInLoop
        check("bool g();\n"
              "void f(bool x) {\n"
              "    if (x) while(x) x = g();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // isLikelyStream
        check("void f(std::istringstream& iss) {\n"
              "   std::string x;\n"
              "   while (iss) {\n"
              "       iss >> x;\n"
              "       if (!iss) break;\n"
              "   }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9332
        check("struct A { void* g(); };\n"
              "void f() {\n"
              "    A a;\n"
              "    void* b = a.g();\n"
              "    if (!b) return;\n"
              "    void* c = a.g();\n"
              "    if (!c) return;\n"
              "    bool compare = c == b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9361
        check("void f(char c) {\n"
              "    if (c == '.') {}\n"
              "    else if (isdigit(c) != 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9351
        check("int f(int x) {\n"
              "    const bool b = x < 42;\n"
              "    if(b) return b?0:-1;\n"
              "    return 42;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8] -> [test.cpp:3:18]: (style) Condition 'b' is always true [knownConditionTrueFalse]\n", errout_str());

        // #9362
        check("uint8_t g();\n"
              "void f() {\n"
              "    const uint8_t v = g();\n"
              "    if((v != 0x00)) {\n"
              "        if( (v & 0x01) == 0x00) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9367
        check("void f(long x) {\n"
              "    if (x <= 0L)\n"
              "        return;\n"
              "    if (x % 360L == 0)\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int a, int b) {\n"
              "    static const int x = 10;\n"
              "    return x == 1 ? a : b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("const bool x = false;\n"
              "void f() {\n"
              "    if (x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("const bool x = false;\n"
              "void f() {\n"
              "    if (!x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9709
        check("void f(int a) {\n"
              "    bool ok = false;\n"
              "    const char * r = nullptr;\n"
              "    do_something(&r);\n"
              "    if (r != nullptr)\n"
              "        ok = a != 0;\n"
              "    if (ok) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9816
        check("bool g();\n"
              "void f() {\n"
              "    bool b = false;\n"
              "    do {\n"
              "        do {\n"
              "            if (g())\n"
              "                break;\n"
              "            b = true;\n"
              "        } while(false);\n"
              "    } while(!b);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9865
        check("void f(const std::string &s) {\n"
              "    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {\n"
              "        const unsigned char c = static_cast<unsigned char>(*it);\n"
              "        if (c == '0') {}\n"
              "        else if ((c == 'a' || c == 'A')\n"
              "                 || (c == 'b' || c == 'B')) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9711
        check("int main(int argc, char* argv[]) {\n"
              "  int foo = 0;\n"
              "  struct option options[] = {\n"
              "    {\"foo\", no_argument, &foo, \'f\'},\n"
              "    {NULL, 0, NULL, 0},\n"
              "  };\n"
              "  getopt_long(argc, argv, \"f\", options, NULL);\n"
              "  if (foo) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // TODO: if (!v) is a known condition as well
        check("struct a {\n"
              "  int *b();\n"
              "};\n"
              "bool g(a c, a* d) {\n"
              "  a *v, *e = v = &c;\n"
              "  if (!v)\n"
              "    return true;\n"
              "  int *f = v->b();\n"
              "  if (f)\n"
              "    v = nullptr;\n"
              "  if (v == nullptr && e) {}\n"
              "  return d;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:11:23]: (style) Condition 'e' is always true [knownConditionTrueFalse]\n", errout_str());

        // #10037
        check("struct a {\n"
              "    int* p;\n"
              "};\n"
              "void g(a*);\n"
              "void f() {\n"
              "    struct a b;\n"
              "    uint32_t p = (uint32_t) -1;\n"
              "    b.p = (void *) &p;\n"
              "    int r = g(&b);\n"
              "    if (r == 0)\n"
              "        if (p != (uint32_t) -1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9890
        check("int g(int);\n"
              "bool h(int*);\n"
              "int f(int *x) {\n"
              "    int y = g(0);\n"
              "    if (!y) {\n"
              "        if (h(x)) {\n"
              "            y = g(1);\n"
              "            if (y) {}\n"
              "            return 0;\n"
              "        }\n"
              "        if (!y) {}\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:11:13]: (style) Condition '!y' is always true [knownConditionTrueFalse]\n", errout_str());

        // #10134
        check("bool foo(bool b);\n"
              "bool thud(const std::vector<std::wstring>& Arr, const std::wstring& Str) {\n"
              "  if (Arr.empty() && Str.empty())\n"
              "    return false;\n"
              "  bool OldFormat = Arr.empty() && !Str.empty();\n"
              "  if (OldFormat)\n"
              "    return foo(OldFormat);\n"
              "  return false;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10208
        check("bool GetFirst(std::string &first);\n"
              "bool GetNext(std::string &next);\n"
              "void g(const std::string& name);\n"
              "void f() {\n"
              "  for (std::string name; name.empty() ? GetFirst(name) : GetNext(name);)\n"
              "    g(name);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool GetFirst(std::string &first);\n"
              "bool GetNext(std::string &next);\n"
              "void g(const std::string& name);\n"
              "void f() {\n"
              "  for (std::string name{}; name.empty() ? GetFirst(name) : GetNext(name);)\n"
              "    g(name);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool GetFirst(std::string &first);\n"
              "bool GetNext(std::string &next);\n"
              "void g(const std::string& name);\n"
              "void f() {\n"
              "  for (std::string name{'a', 'b'}; name.empty() ? GetFirst(name) : GetNext(name);)\n"
              "    g(name);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool GetFirst(const std::string &first);\n"
              "bool GetNext(const std::string &next);\n"
              "void g(const std::string& name);\n"
              "void f() {\n"
              "  for (std::string name; name.empty() ? GetFirst(name) : GetNext(name);)\n"
              "    g(name);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:36]: (style) Condition 'name.empty()' is always true [knownConditionTrueFalse]\n", errout_str());

        // #10278
        check("void foo(unsigned int x) {\n"
              "    if ((100 - x) > 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10298
        check("void foo(unsigned int x) {\n"
              "    if (x == -1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10121
        check("struct AB {\n"
              "    int a;\n"
              "};\n"
              "struct ABC {\n"
              "    AB* ab;\n"
              "};\n"
              "void g(ABC*);\n"
              "int f(struct ABC *abc) {\n"
              "    int err = 0;\n"
              "    AB *ab = abc->ab;\n"
              "    if (ab->a == 123){\n"
              "        g(abc);\n"
              "        if (ab->a != 123) {\n"
              "            err = 1;\n"
              "        }\n"
              "    }\n"
              "    return err;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10323
        check("void foo(int x) {\n"
              "    if(x)\n"
              "        if(x == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int x) {\n"
              "    if(x) {}\n"
              "    else\n"
              "        if(x == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:8] -> [test.cpp:4:14]: (style) Condition 'x==1' is always false [knownConditionTrueFalse]\n", errout_str());

        // do not report both unsignedLessThanZero and knownConditionTrueFalse
        check("void foo(unsigned int max) {\n"
              "    unsigned int num = max - 1;\n"
              "    if (num < 0) {}\n" // <- do not report knownConditionTrueFalse
              "}");
        ASSERT_EQUALS("", errout_str());

        // #10297
        check("void foo(size_t len, int start) {\n"
              "    if (start < 0) {\n"
              "        start = len+start;\n"
              "        if (start < 0) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10362
        check("int tok;\n"
              "void next();\n"
              "void parse_attribute() {\n"
              "    if (tok == '(') {\n"
              "        int parenthesis = 0;\n"
              "        do {\n"
              "            if (tok == '(')\n"
              "                parenthesis++;\n"
              "            else if (tok == ')')\n"
              "                parenthesis--;\n"
              "            next();\n"
              "        } while (parenthesis && tok != -1);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #7843
        check("void f(int i) {\n"
              "    if(abs(i) == -1) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Condition 'abs(i)==-1' is always false [knownConditionTrueFalse]\n", errout_str());

        // #7844
        check("void f(int i) {\n"
              "    if(i > 0 && abs(i) == i) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:24]: (style) Condition 'abs(i)==i' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    if(i < 0 && abs(i) == i) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:10] -> [test.cpp:2:24]: (style) Condition 'abs(i)==i' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    if(i > -3 && abs(i) == i) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9948
        check("bool f(bool a, bool b) {\n"
              "    return a || ! b || ! a;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:2:24]: (style) Return value '!a' is always true [knownConditionTrueFalse]\n", errout_str());

        // #10148
        check("void f(int i) {\n"
              "    if (i >= 64) {}\n"
              "    else if (i >= 32) {\n"
              "        i &= 31;\n"
              "        if (i == 0) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10548
        check("void f() {\n"
              "    int i = 0;\n"
              "    do {} while (i++ == 0);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10582
        check("static void fun(message_t *message) {\n"
              "    if (message->length >= 1) {\n"
              "        switch (data[0]) {}\n"
              "    }\n"
              "    uint8_t d0 = message->length > 0 ? data[0] : 0xff;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #8266
        check("void f(bool b) {\n"
              "    if (b)\n"
              "        return;\n"
              "    if (g(&b) || b)\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9720
        check("bool bar(int &);\n"
              "void f(int a, int b) {\n"
              "    if (a + b == 3)\n"
              "        return;\n"
              "    if (bar(a) && (a + b == 3)) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10437
        check("void f() {\n"
              "  Obj* PObj = nullptr;\n"
              "  bool b = false;\n"
              "  if (GetObj(PObj) && PObj != nullptr)\n"
              "    b = true;\n"
              "  if (b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10223
        check("static volatile sig_atomic_t is_running;\n"
              "static void handler(int signum) {\n"
              "    is_running = 0;\n"
              "}\n"
              "void f() {\n"
              "    signal(SIGINT, &handler);\n"
              "    is_running = 1;\n"
              "    while (is_running) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10659
        check("auto func(const std::tuple<int, int>& t) {\n"
              "  auto& [foo, bar] = t;\n"
              "  std::cout << foo << bar << std::endl;\n"
              "  return foo < bar;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10484
        check("void f() {\n"
              "    static bool init = true;\n"
              "    if (init)\n"
              "        init = false;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:14] -> [test.cpp:3:9]: (style) The statement 'if (init) init=false' is logically equivalent to 'init=false'. [duplicateConditionalAssign]\n", errout_str());

        check("void f() {\n"
              "    static bool init(true);\n"
              "    if (init)\n"
              "        init = false;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:14] -> [test.cpp:3:9]: (style) The statement 'if (init) init=false' is logically equivalent to 'init=false'. [duplicateConditionalAssign]\n", errout_str());

        check("void f() {\n"
              "    static bool init{ true };\n"
              "    if (init)\n"
              "        init = false;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:14] -> [test.cpp:3:9]: (style) The statement 'if (init) init=false' is logically equivalent to 'init=false'. [duplicateConditionalAssign]\n", errout_str());

        // #10248
        check("void f() {\n"
              "    static int var(1);\n"
              "    if (var == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    static int var{ 1 };\n"
              "    if (var == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void Fun();\n"
              "using Fn = void (*)();\n"
              "void f() {\n"
              "    static Fn logger = nullptr;\n"
              "    if (logger == nullptr)\n"
              "        logger = Fun;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void Fun();\n"
              "using Fn = void (*)();\n"
              "void f() {\n"
              "    static Fn logger(nullptr);\n"
              "    if (logger == nullptr)\n"
              "        logger = Fun;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void Fun();\n"
              "using Fn = void (*)();\n"
              "void f() {\n"
              "    static Fn logger{ nullptr };\n"
              "    if (logger == nullptr)\n"
              "        logger = Fun;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void Fun();\n"
              "typedef void (*Fn)();\n"
              "void f() {\n"
              "    static Fn logger = nullptr;\n"
              "    if (logger == nullptr)\n"
              "        logger = Fun;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void Fun();\n"
              "typedef void (*Fn)();\n"
              "void f() {\n"
              "    static Fn logger(nullptr);\n"
              "    if (logger == nullptr)\n"
              "        logger = Fun;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void Fun();\n"
              "typedef void (*Fn)();\n"
              "void f() {\n"
              "    static Fn logger{ nullptr };\n"
              "    if (logger == nullptr)\n"
              "        logger = Fun;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9256
        check("bool f() {\n"
              "    bool b = false;\n"
              "    b = true;\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10702
        check("struct Object {\n"
              "  int _count=0;\n"
              "   void increment() { ++_count;}\n"
              "   auto get() const { return _count; }\n"
              "};\n"
              "struct Modifier {\n"
              "Object & _object;\n"
              "  explicit Modifier(Object & object) : _object(object) {}\n"
              "  void do_something() { _object.increment(); }\n"
              "};\n"
              "struct Foo {\n"
              "  Object _object;\n"
              "  void foo() {\n"
              "    Modifier mod(_object);\n"
              "    if (_object.get()>0)\n"
              "      return;\n"
              "    mod.do_something();\n"
              "    if (_object.get()>0)\n"
              "      return;\n"
              "  }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct Object {\n"
              "  int _count=0;\n"
              "   auto get() const;\n"
              "};\n"
              "struct Modifier {\n"
              "Object & _object;\n"
              "  explicit Modifier(Object & object);\n"
              "  void do_something();\n"
              "};\n"
              "struct Foo {\n"
              "  Object _object;\n"
              "  void foo() {\n"
              "    Modifier mod(_object);\n"
              "    if (_object.get()>0)\n"
              "      return;\n"
              "    mod.do_something();\n"
              "    if (_object.get()>0)\n"
              "      return;\n"
              "  }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const uint32_t u) {\n"
              "	const uint32_t v = u < 4;\n"
              "	if (v) {\n"
              "		const uint32_t w = v < 2;\n"
              "		if (w) {}\n"
              "	}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:24]: (style) Condition 'v<2' is always true [knownConditionTrueFalse]\n"
                      "[test.cpp:5:7]: (style) Condition 'w' is always true [knownConditionTrueFalse]\n",
                      errout_str());

        check("void f(double d) {\n" // #10792
              "    if (d != 0) {\n"
              "        int i = (int)d;\n"
              "        if (i == 0) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(double d) {\n"
              "    if (0 != d) {\n"
              "        int i = (int)d;\n"
              "        if (i == 0) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A { double d; }\n"
              "void f(A a) {\n"
              "    if (a.d != 0) {\n"
              "        int i = a.d;\n"
              "        if (i == 0) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    if(strlen(\"abc\") == 3) {;}\n"
              "    if(strlen(\"abc\") == 1) {;}\n"
              "    if(wcslen(L\"abc\") == 3) {;}\n"
              "    if(wcslen(L\"abc\") == 1) {;}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:22]: (style) Condition 'strlen(\"abc\")==3' is always true [knownConditionTrueFalse]\n"
                      "[test.cpp:3:22]: (style) Condition 'strlen(\"abc\")==1' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:4:23]: (style) Condition 'wcslen(L\"abc\")==3' is always true [knownConditionTrueFalse]\n"
                      "[test.cpp:5:23]: (style) Condition 'wcslen(L\"abc\")==1' is always false [knownConditionTrueFalse]\n",
                      errout_str());

        check("int foo(bool a, bool b) {\n"
              "  if(!a && b && (!a == !b))\n"
              "   return 1;\n"
              "  return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:6] -> [test.cpp:2:21]: (style) Condition '!a==!b' is always false [knownConditionTrueFalse]\n", errout_str());

        // #10454
        check("struct S {\n"
              "    int f() const { return g() ? 0 : 1; }\n"
              "    bool g() const { return u == 18446744073709551615ULL; }\n"
              "    unsigned long long u{};\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #8358
        check("void f(double d) { if ((d * 0) != 0) {} }");
        ASSERT_EQUALS("", errout_str());

        // #6870
        check("struct S {\n"
              "    int* p;\n"
              "    void f() const;\n"
              "    int g();\n"
              "};\n"
              "void S::f() {\n"
              "    if ((p == NULL) || ((p) && (g() >= *p))) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:26]: (style) Condition 'p' is always true [knownConditionTrueFalse]\n", errout_str());

        // #10749
        check("struct Interface {\n"
              "    virtual int method() = 0;\n"
              "};\n"
              "struct Child : Interface {\n"
              "   int method() override { return 0; }\n"
              "   auto foo() {\n"
              "       if (method() == 0)\n"
              "           return true;\n"
              "       else\n"
              "           return false;\n"
              "   }\n"
              "};\n"
              "struct GrandChild : Child {\n"
              "   int method() override  { return 1; }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #6855
        check("struct S { int i; };\n"
              "void f(S& s) {\n"
              "    if (!(s.i > 0) && (s.i != 0))\n"
              "        s.i = 0;\n"
              "    else if (s.i < 0)\n"
              "        s.s = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:15] -> [test.cpp:5:18]: (style) Condition 's.i<0' is always false [knownConditionTrueFalse]\n", errout_str());

        // #6857
        check("int bar(int i) { return i; }\n"
              "void foo() {\n"
              "    if (bar(1) == 0 && bar(1) > 0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Condition 'bar(1)==0' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:3:31]: (style) Condition 'bar(1)>0' is always true [knownConditionTrueFalse]\n",
                      errout_str());

        check("struct S { int bar(int i) const; };\n"
              "void foo(const S& s) {\n"
              "    if (s.bar(1) == 0 && s.bar(1) > 0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:23]: (warning) Logical conjunction always evaluates to false: s.bar(1) == 0 && s.bar(1) > 0. [incorrectLogicOperator]\n",
                      errout_str());

        check("struct B {\n" // #10618
              "    void Modify();\n"
              "    static void Static();\n"
              "    virtual void CalledByModify();\n"
              "};\n"
              "struct D : B {\n"
              "    int i{};\n"
              "    void testV();\n"
              "    void testS();\n"
              "    void CalledByModify() override { i = 0; }\n"
              "};\n"
              "void D::testV() {\n"
              "    i = 1;\n"
              "    B::Modify();\n"
              "    if (i == 1) {}\n"
              "}\n"
              "void D::testS() {\n"
              "    i = 1;\n"
              "    B::Static();\n"
              "    if (i == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:20:11]: (style) Condition 'i==1' is always true [knownConditionTrueFalse]\n", errout_str());

        check("typedef struct { bool x; } s_t;\n" // #8446
              "unsigned f(bool a, bool b) {\n"
              "    s_t s;\n"
              "    const unsigned col = a ? (s.x = false) : (b = true);\n"
              "    if (!s.x) {}\n"
              "    return col;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #11233
              "    static std::string m;\n"
              "    static void f() { m = \"abc\"; }\n"
              "    static void g() {\n"
              "        m.clear();\n"
              "        f();\n"
              "        if (m.empty()) {}\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #11203
        check("void f() {\n"
              "    int i = 10;\n"
              "    if(i > 9.9){}\n"
              "    float f = 9.9f;\n"
              "    if(f < 10) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:10]: (style) Condition 'i>9.9' is always true [knownConditionTrueFalse]\n"
            "[test.cpp:5:10]: (style) Condition 'f<10' is always true [knownConditionTrueFalse]\n",
            errout_str());
        check("constexpr int f() {\n" // #11238
              "    return 1;\n"
              "}\n"
              "constexpr bool g() {\n"
              "    return f() == 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int g() { return -1; }\n"
              "void f() {\n"
              "    if (g() == 1 && g() == -1) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Condition 'g()==1' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:3:25]: (style) Condition 'g()==-1' is always true [knownConditionTrueFalse]\n",
                      errout_str());

        // #9817
        check("void f(float x) {\n"
              "    if (x <= 0) {}\n"
              "    else if (x < 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10426
        check("int f() {\n"
              "    std::string s;\n"
              "    for (; !s.empty();) {}\n"
              "    for (; s.empty();) {}\n"
              "    if (s.empty()) {}\n"
              "    if ((bool)0) {}\n"
              "    return s.empty();"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) Condition '!s.empty()' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:4:19]: (style) Condition 's.empty()' is always true [knownConditionTrueFalse]\n"
                      "[test.cpp:5:16]: (style) Condition 's.empty()' is always true [knownConditionTrueFalse]\n"
                      "[test.cpp:6:9]: (style) Condition '(bool)0' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:7:19]: (style) Return value 's.empty()' is always true [knownConditionTrueFalse]\n",
                      errout_str());

        check("int f(bool b) {\n"
              "    if (b) return static_cast<int>(1);\n"
              "    return (int)0;\n"
              "}\n"
              "bool g(bool b) {\n"
              "    if (b) return static_cast<int>(1);\n"
              "    return (int)0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:35]: (style) Return value 'static_cast<int>(1)' is always true [knownConditionTrueFalse]\n"
                      "[test.cpp:7:12]: (style) Return value '(int)0' is always false [knownConditionTrueFalse]\n",
                      errout_str());

        check("int f() { return 3; }\n"
              "int g() { return f(); }\n"
              "int h() { if (f()) {} }\n"
              "int i() { return f() == 3; }\n");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Condition 'f()' is always true [knownConditionTrueFalse]\n"
                      "[test.cpp:4:22]: (style) Return value 'f()==3' is always true [knownConditionTrueFalse]\n",
                      errout_str());

        check("int f() {\n"
              "    const char *n;\n"
              "    return((n=42) &&\n"
              "           *n == 'A');\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::istringstream& i) {\n" // #9327
              "    std::string s;\n"
              "    if (!(i >> s))\n"
              "        return;\n"
              "    if (!(i >> s))\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11227
        check("struct S {\n"
              "	int get();\n"
              "};\n"
              "void f(const S* s) {\n"
              "    if (!s)\n"
              "        return;\n"
              "    g(s ? s->get() : 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:7:7]: (style) Condition 's' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(const char* o) {\n" // #11558
              "    if (!o || !o[0])\n"
              "        return;\n"
              "    if (o[0] == '-' && o[1]) {\n"
              "        if (o[1] == '-') {}\n"
              "        if (o[1] == '\\0') {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:18]: (style) Condition 'o[1]=='\\0'' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x) {\n" // #11449
              "    int i = x;\n"
              "    i = (std::min)(i, 1);\n"
              "    if (i == 1) {}\n"
              "    int j = x;\n"
              "    j = (::std::min)(j, 1);\n"
              "    if (j == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void h(int);\n" // #11679
              "bool g(int a) { h(a); return false; }\n"
              "bool f(int i) {\n"
              "    return g(i);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string a) {\n" // #11051
              "    a = \"x\";\n"
              "    if (a == \"x\") {}\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'a==\"x\"' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void g(bool);\n"
              "void f() {\n"
              "    int i = 5;\n"
              "    int* p = &i;\n"
              "    g(i == 7);\n"
              "    g(p == nullptr);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:9]: (style) Condition 'i==7' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:6:9]: (style) Condition 'p==nullptr' is always false [knownConditionTrueFalse]\n",
                      errout_str());

        check("enum E { E0, E1 };\n"
              "void f() {\n"
              "	static_assert(static_cast<int>(E::E1) == 1);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct a {\n"
              "  bool g();\n"
              "  int h();\n"
              "};\n"
              "void f(a c, int d, int e) {\n"
              "  if (c.g() && c.h()) {}\n"
              "  else {\n"
              "    bool u = false;\n"
              "    if (d && e)\n"
              "      u = true;\n"
              "    if (u) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int i) {\n" // #11741
              "    i = -i - 1;\n"
              "    if (i < 0 || i >= 20)\n"
              "        return 0;\n"
              "    return 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("namespace S { int s{}; };\n" // #11046
              "void f(bool b) {\n"
              "    if (S::s) {\n"
              "        if (b) {\n"
              "            if (S::s) {}\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:5:18]: (warning) Identical inner 'if' condition is always true. [identicalInnerCondition]\n", errout_str());

        check("void f() {\n" // #10811
              "    int i = 0;\n"
              "    if ((i = g(), 1) != 0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:22]: (style) Condition '(i=g(),1)!=0' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(unsigned i) {\n"
              "    const int a[2] = {};\n"
              "    const int* q = a + i;\n"
              "    if (q) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:9]: (style) Condition 'q' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n" // #12786
              "    const int b[2] = {};\n"
              "    if (b) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Condition 'b' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    int j = 0;\n"
              "    switch (i) {\n"
              "    case 1:\n"
              "        j = 0;\n"
              "        break;\n"
              "    default:\n"
              "        j = 1;\n"
              "    }\n"
              "    if (j != 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    const char *s1 = foo();\n"
              "    const char *s2 = bar();\n"
              "    if (s2 == NULL)\n"
              "        return;\n"
              "    size_t len = s2 - s1;\n"
              "    if (len == 0)\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int h();\n" // #12858
              "bool g() {\n"
              "    bool b{};\n"
              "    try {\n"
              "        int x = h();\n"
              "        switch (x) {\n"
              "        default:\n"
              "            b = true;\n"
              "        }\n"
              "    }\n"
              "    catch (...) {\n"
              "        b = false;\n"
              "    }\n"
              "    return b;\n"
              "}\n"
              "void f() {\n"
              "    if (g()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int x, int y) {\n" // #11822
              "    if (x) {\n"
              "        switch (y) {\n"
              "        case 1:\n"
              "            return 7;\n"
              "        }\n"
              "    }\n"
              " \n"
              "    if (y)\n"
              "        return 8;\n"
              " \n"
              "    if (x)\n"
              "        return 9;\n"
              " \n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int g();\n" // #10561
              "bool h();\n"
              "int f() {\n"
              "    bool b = false;\n"
              "    try {\n"
              "        switch (g()) {\n"
              "        default:\n"
              "            b = h();\n"
              "        }\n"
              "    }\n"
              "    catch (...) {}\n"
              "    return b ? 1 : 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    const S* get2() const {\n"
              "        if (mS)\n"
              "            return mS;\n"
              "        return this;\n"
              "    }\n"
              "    S* mS = nullptr;\n"
              "};\n"
              "void f2() {\n"
              "    const S s;\n"
              "    const S* sp2 = s.get2();\n"
              "    if (sp2) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:12:9]: (style) Condition 'sp2' is always true [knownConditionTrueFalse]\n", errout_str());

        check("struct S {\n"
              "    void f(int i);\n"
              "    bool g() const { return !m.empty(); }\n"
              "    std::set<int> m;\n"
              "};\n"
              "void S::f(int i) {\n"
              "    bool b = g();\n"
              "    auto it = m.find(i);\n"
              "    if (it != m.end()) {\n"
              "        m.erase(it);\n"
              "        if (g() != b) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #14392
              "    bool g() const { return m; }\n"
              "    bool m{};\n"
              "};\n"
              "bool f(S s) {\n"
              "    if (s.g()) {\n"
              "        bool b = s.g();\n"
              "        return b;\n"
              "    }\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:12] -> [test.cpp:7:21]: (style) Assigned value 's.g()' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(const void* p) {\n" // #11519
              "    bool b = false;\n"
              "    if (!p && !b) {}\n"
              "    if (!b) {}\n"
              "    (void)b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:15]: (style) Condition '!b' is always true [knownConditionTrueFalse]\n"
                      "[test.cpp:4:9]: (style) Condition '!b' is always true [knownConditionTrueFalse]\n",
                      errout_str());

        check("struct C {\n" // #12532
              "    void f() const;\n"
              "    int a, b;\n"
              "};\n"
              "void C::f() const {\n"
              "    if (a)\n"
              "        return;\n"
              "    if (!b) {}\n"
              "    if (a) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:9] -> [test.cpp:9:9]: (warning) Identical condition 'a', second condition is always false [identicalConditionAfterEarlyExit]\n",
                      errout_str());
    }

    void alwaysTrueSymbolic()
    {
        check("void f(const uint32_t x) {\n"
              "    uint32_t y[1];\n"
              "    y[0]=x;\n"
              "    if(x > 0 || y[0] < 42){}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10] -> [test.cpp:4:22]: (style) Condition 'y[0]<42' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x, int y) {\n"
              "    if(x < y && x < 42) {\n"
              "        --x;\n"
              "        if(x == y) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:10] -> [test.cpp:4:14]: (style) Condition 'x==y' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(bool a, bool b) {  if (a == b && a && !b){} }");
        ASSERT_EQUALS("[test.cpp:1:41] -> [test.cpp:1:46]: (style) Condition '!b' is always false [knownConditionTrueFalse]\n", errout_str());

        check("bool f(bool a, bool b) { if(a && b && (!a)){} }");
        ASSERT_EQUALS("[test.cpp:1:29] -> [test.cpp:1:40]: (style) Condition '!a' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x, int y) {\n"
              "  if (x < y) {\n"
              "    auto z = y - x;\n"
              "    if (z < 1) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:4:11]: (style) Condition 'z<1' is always false [knownConditionTrueFalse]\n", errout_str());

        check("bool f(int &index, const int s, const double * const array, double & x) {\n"
              "    if (index >= s)\n"
              "        return false;\n"
              "    else {\n"
              "        x = array[index];\n"
              "        return (index++) >= s;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:15] -> [test.cpp:6:26]: (style) Return value '(index++)>=s' is always false [knownConditionTrueFalse]\n", errout_str());

        check("struct a {\n"
              "  a *b() const;\n"
              "} c;\n"
              "void d() {\n"
              "  a *e = nullptr;\n"
              "  e = c.b();\n"
              "  if (e) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int g(int i) {\n"
              "  if (i < 256)\n"
              "    return 1;\n"
              "  const int N = 2 * i;\n"
              "  i -= 256;\n"
              "  if (i == 0)\n"
              "    return 0;\n"
              "  return N;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i, int j) {\n"
              "    if (i < j) {\n"
              "        i++;\n"
              "        if (i >= j)\n"
              "            return;\n"
              "        i++;\n"
              "        if (i >= j) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int get_delta() {\n"
              "    clock_t now_ms = (clock() / (CLOCKS_PER_SEC / 1000));\n"
              "    static clock_t last_clock_ms = now_ms;\n"
              "    clock_t delta = now_ms - last_clock_ms;\n"
              "    last_clock_ms = now_ms;\n"
              "    if (delta > 50)\n"
              "        delta = 50;\n"
              "    return delta;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10555
        check("struct C {\n"
              "  int GetI() const { return i; }\n"
              "  int i{};\n"
              "};\n"
              "struct B {\n"
              "    C *m_PC{};\n"
              "    Modify();\n"
              "};\n"
              "struct D : B {\n"
              "  void test();  \n"
              "};\n"
              "void D::test() {\n"
              "    const int I = m_PC->GetI();\n"
              "    Modify();\n"
              "    if (m_PC->GetI() != I) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10624
        check("struct Data {\n"
              "  Base* PBase{};\n"
              "};\n"
              "void f(Data* BaseData) {\n"
              "  Base* PObj = BaseData->PBase;\n"
              "  if (PObj == nullptr)\n"
              "    return;\n"
              "  Derived* pD = dynamic_cast<Derived*>(PObj);\n"
              "  if (pD) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9549
        check("void f(const uint32_t v) {\n"
              "    const uint32_t v16 = v >> 16;\n"
              "    if (v16) {\n"
              "        const uint32_t v8 = v16 >> 8;\n"
              "        if (v8) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10649
        check("void foo(struct diag_msg *msg) {\n"
              "    msg = msg->next;\n"
              "    if (msg == NULL)\n"
              "        return CMD_OK;\n"
              "    msg = msg->next;\n"
              "    if (msg == NULL)\n"
              "        return CMD_OK;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int foo(bool a, bool b) {\n"
              "  if((!a == !b) && !a && b)\n"
              "   return 1;\n"
              "  return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:20] -> [test.cpp:2:26]: (style) Condition 'b' is always false [knownConditionTrueFalse]\n", errout_str());

        // #11124
        check("struct Basket {\n"
              "	std::vector<int> getApples() const;\n"
              "	std::vector<int> getBananas() const;	\n"
              "};\n"
              "int getFruit(const Basket & b, bool preferApples)\n"
              "{\n"
              "    std::vector<int> apples = b.getApples();\n"
              "    int apple = apples.empty() ? -1 : apples.front();\n"
              "    std::vector<int> bananas = b.getBananas();\n"
              "    int banana = bananas.empty() ? -1 : bananas.front();\n"
              "    int fruit = std::max(apple, banana);\n"
              "    if (fruit == -1)\n"
              "        return fruit;\n"
              "    if (std::min(apple, banana) != -1)\n"
              "        fruit = preferApples ? apple : banana;\n"
              "    return fruit;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::string & s, int i) {\n"
              "    const char c = s[i];\n"
              "    if (!std::isalnum(c)) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #11404
              "    int f() const;\n"
              "    void g();\n"
              "};\n"
              "void h(std::vector<S*>::iterator it) {\n"
              "    auto i = (*it)->f();\n"
              "    (*it)->g();\n"
              "    auto j = (*it)->f();\n"
              "    if (i == j) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11384
        check("bool f(const int* it, const int* end) {\n"
              "	return (it != end) && *it++ &&\n"
              "           (it != end) && *it;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #12116
        check("void f(int n) {\n"
              "    for (int i = 0; i < N; ++i) {\n"
              "        if (i < n) {}\n"
              "        else if (i > n) {}\n"
              "        else {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #12681
        check("void f(unsigned u) {\n"
              "      if (u > 0) {\n"
              "             u--;\n"
              "             if (u == 0) {}\n"
              "      }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(unsigned u) {\n"
              "      if (u < 0xFFFFFFFF) {\n"
              "             u++;\n"
              "             if (u == 0xFFFFFFFF) {}\n"
              "      }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool a, bool b) {\n" // #12937
              "    bool c = !a && b;\n"
              "    if (a) {}\n"
              "    else {\n"
              "        if (c) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(unsigned x) {\n" // #13522
              "    unsigned u = x;\n"
              "    int i = u - 0;\n"
              "    if (i < 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void alwaysTrueInfer() {
        check("void f(int x) {\n"
              "    if (x > 5) {\n"
              "        x++;\n"
              "        if (x == 1) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:4:15]: (style) Condition 'x==1' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x > 5) {\n"
              "        x++;\n"
              "        if (x != 1) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:4:15]: (style) Condition 'x!=1' is always true [knownConditionTrueFalse]\n", errout_str());

        // #6890
        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x;\n"
              "        if (x == -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:6:15]: (style) Condition 'x==-1' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x;\n"
              "        if (x != -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:6:15]: (style) Condition 'x!=-1' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x;\n"
              "        if (x >= -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:6:15]: (style) Condition 'x>=-1' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x;\n"
              "        if (x > -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:6:15]: (style) Condition 'x>-1' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x;\n"
              "        if (x < -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:6:15]: (style) Condition 'x<-1' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x;\n"
              "        if (x <= -1) {}\n"
              "        else {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:6:15]: (style) Condition 'x<=-1' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x;\n"
              "        if (x > 7) {}\n"
              "        else {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:6:15]: (style) Condition 'x>7' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x;\n"
              "        if (x > 9) {}\n"
              "        else {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i) {\n"
              "    int x = i;\n"
              "    if (x >= 1) {}\n"
              "    else {\n"
              "        x = 8 - x;\n"
              "        if (x > 10) {}\n"
              "        else {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #11100
        check("struct T {\n"
              "  bool m{};\n"
              "  void f(bool b);\n"
              "  bool get() const { return m; }\n"
              "  void set(bool v) { m = v; }\n"
              "};\n"
              "void T::f(bool b) {\n"
              "	bool tmp = get();\n"
              "	set(b);\n"
              "	if (tmp != get()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9541
        check("int f(int pos, int a) {\n"
              "    if (pos <= 0)\n"
              "        pos = 0;\n"
              "    else if (pos < a)\n"
              "        if(pos > 0)\n"
              "            --pos;\n"
              "    return pos;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13] -> [test.cpp:5:16]: (style) Condition 'pos>0' is always true [knownConditionTrueFalse]\n", errout_str());

        // #9721
        check("void f(int x) {\n"
              "    if (x > 127) {\n"
              "        if ( (x>255) || (-128>x) )\n"
              "            return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:3:30]: (style) Condition '-128>x' is always false [knownConditionTrueFalse]\n", errout_str());

        // #8778
        check("void f() {\n"
              "    for(int i = 0; i < 19; ++i)\n"
              "        if(i<=18) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Condition 'i<=18' is always true [knownConditionTrueFalse]\n", errout_str());

        // #8209
        check("void f() {\n"
              "    for(int x = 0; x < 3; ++x)\n"
              "        if(x == -5) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (style) Condition 'x==-5' is always false [knownConditionTrueFalse]\n", errout_str());

        // #8407
        check("int f(void) {\n"
              "    for(int i = 0; i <1; ++i)\n"
              "        if(i == 0) return 1; \n" // <<
              "        else return 0;\n"
              "    return -1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (style) Condition 'i==0' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f(unsigned int u1, unsigned int u2) {\n"
              "    if (u1 <= 10 && u2 >= 20) {\n"
              "        if (u1 != u2) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:3:16]: (style) Condition 'u1!=u2' is always true [knownConditionTrueFalse]\n", errout_str());

        // #10544
        check("void f(int N) {\n"
              "    if (N > 0) {\n"
              "        while (N)\n"
              "            N = test();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11098
        check("void f(unsigned int x) { if (x == -1u) {} }\n");
        ASSERT_EQUALS("", errout_str());

        check("bool f(const int *p, const int *q) {\n"
              "    return p != NULL && q != NULL && p == NULL;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:40]: (style) Return value 'p==NULL' is always false [knownConditionTrueFalse]\n", errout_str());

        check("struct S {\n" // #11789
              "    std::vector<int> v;\n"
              "    void f(int i) const;\n"
              "};\n"
              "void S::f(int i) const {\n"
              "    int j = i - v.size();\n"
              "    if (j >= 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i) {\n" // #12039
              "    if ((128 + i < 255 ? 128 + i : 255) > 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12727
              "    bool f() const {\n"
              "        return g() > 0;\n"
              "    }\n"
              "    std::size_t g() const {\n"
              "        return 5 - h();\n"
              "    }\n"
              "    std::size_t h() const {\n"
              "        if (x > 7)\n"
              "            return 5;\n"
              "        return (5 + x) % 5;\n"
              "    }\n"
              "    std::size_t x;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void alwaysTrueContainer() {
        // #9329
        check("void c1(std::vector<double>&);\n"
              "void c2(std::vector<double>&);\n"
              "void foo(int flag) {\n"
              "    std::vector<double> g;\n"
              "    if (flag)\n"
              "        c1(g );\n"
              "    else\n"
              "        c2(g );\n"
              "    if ( !g.empty() )\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int flag) {\n"
              "    std::vector<double> g;\n"
              "    if (flag)\n"
              "        c1(g );\n"
              "    else\n"
              "        c2(g );\n"
              "    if ( !g.empty() )\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    std::vector<int> v;\n"
              "    void g();\n"
              "    void f(bool b) {\n"
              "        v.clear();\n"
              "        g();\n"
              "        return !v.empty();\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #10409
        check("void foo(const std::string& s) {\n"
              "    if( s.size() < 2 ) return;\n"
              "    if( s == \"ab\" ) return;\n"
              "    if( s.size() < 3 ) return;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const std::string& s) {\n"
              "    if( s.size() < 2 ) return;\n"
              "    if( s != \"ab\" )\n"
              "        if( s.size() < 3 ) return;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10226
        check("int f(std::vector<int>::iterator it, const std::vector<int>& vector) {\n"
              "    if (!(it != vector.end() && it != vector.begin()))\n"
              "        throw 0;\n"
              "    if (it != vector.end() && *it == 0)\n"
              "        return -1;\n"
              "    return *it;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Condition 'it!=vector.end()' is always true [knownConditionTrueFalse]\n", errout_str());

        // #11303
        check("void f(int n) {\n"
              "    std::vector<char> buffer(n);\n"
              "    if(buffer.back() == 0 ||\n"
              "       buffer.back() == '\\n' ||\n"
              "       buffer.back() == '\\0') {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:22]: (style) Condition 'buffer.back()=='\\0'' is always false [knownConditionTrueFalse]\n", errout_str());

        // #9353
        check("struct X { std::string s; };\n"
              "void f(const std::vector<X>&v) {\n"
              "    for (std::vector<X>::const_iterator it = v.begin(); it != v.end(); ++it)\n"
              "        if (!it->s.empty()) {\n"
              "            if (!it->s.empty()) {}\n"
              "        }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:13] -> [test.cpp:5:17]: (style) Condition '!it->s.empty()' is always true [knownConditionTrueFalse]\n", errout_str());

        check("struct X { std::string s; };\n"
              "void f(const std::vector<struct X>&v) {\n"
              "    for (std::vector<struct X>::const_iterator it = v.begin(); it != v.end(); ++it)\n"
              "        if (!it->s.empty()) {\n"
              "            if (!it->s.empty()) {}\n"
              "        }\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:4:13] -> [test.cpp:5:17]: (style) Condition '!it->s.empty()' is always true [knownConditionTrueFalse]\n", "", errout_str());

        // #10508
        check("bool f(const std::string& a, const std::string& b) {\n"
              "    return a.empty() || (b.empty() && a.empty());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:19] -> [test.cpp:2:46]: (style) Return value 'a.empty()' is always false [knownConditionTrueFalse]\n", errout_str());

        check("struct A {\n"
              "    struct iterator;\n"
              "    iterator begin() const;\n"
              "    iterator end() const;\n"
              "};\n"
              "A g();\n"
              "void f(bool b) {\n"
              "    std::set<int> s;\n"
              "    auto v = g();\n"
              "    s.insert(v.begin(), v.end());\n"
              "    if(!b && s.size() != 1)\n"
              "        return;\n"
              "    if(!s.empty()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(std::string s) {\n"
              "    if (s.empty())\n"
              "        return -1;\n"
              "    s += '\\n';\n"
              "    if (s.empty())\n"
              "        return -1;\n"
              "    return -1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:16]: (style) Condition 's.empty()' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(std::string& p) {\n"
              "    const std::string d{ \"abc\" };\n"
              "    p += d;\n"
              "    if(p.empty()) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:15]: (style) Condition 'p.empty()' is always false [knownConditionTrueFalse]\n", errout_str());

        check("bool f(int i, FILE* fp) {\n"
              "  std::string s = \"abc\";\n"
              "  s += std::to_string(i);\n"
              "  s += \"\\n\";\n"
              "  return fwrite(s.c_str(), 1, s.length(), fp) == s.length();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::string& s) {\n" // #9148
              "    if (s.empty() || s.size() < 1) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:16] -> [test.cpp:2:31]: (style) Condition 's.size()<1' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void bar(std::vector<int>& vv) {\n" // #11464
              "    class F {\n"
              "    public:\n"
              "        F(int, std::vector<int>& lv) : mV(lv) {\n"
              "            mV.push_back(0);\n"
              "        }\n"
              "    private:\n"
              "        std::vector<int>& mV;\n"
              "    } fi(1, vv);\n"
              "}\n"
              "void g() {\n"
              "    std::vector<int> v;\n"
              "    bar(v);\n"
              "    if (v.empty()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct F {\n"
              "    F(int, std::vector<int>&lv) : mV(lv) {\n"
              "        mV.push_back(0);\n"
              "    }\n"
              "    std::vector<int>& mV;\n"
              "};\n"
              "void g(std::vector<int>& vv) {\n"
              "    F(1, vv);\n"
              "}\n"
              "void f() {\n"
              "    std::vector<int> v;\n"
              "    g(v);\n"
              "    if (v.empty()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void alwaysTrueLoop()
    {
        check("long foo() {\n"
              "  bool bUpdated = false;\n"
              "  long Ret{};\n"
              "  do {\n"
              "    Ret = bar();\n"
              "    if (Ret == 0) {\n"
              "      if (bUpdated)\n"
              "        return 1;\n"
              "      bUpdated = true;\n"
              "    }\n"
              "    else\n"
              "      bUpdated = false;\n"
              "  }\n"
              "  while (bUpdated);\n"
              "  return Ret;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool foo() {\n"
              "  bool bFirst = true;\n"
              "  do {\n"
              "    if (bFirst)\n"
              "      bar();\n"
              "    if (baz())\n"
              "      break;       \n"
              "    bFirst = false;\n"
              "  } while (true);\n"
              "  return bFirst;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "   void * pool = NULL;\n"
              "   do {\n"
              "      pool = malloc(40);\n"
              "      if (dostuff())\n"
              "         break;\n"
              "      pool = NULL;\n"
              "   }\n"
              "   while (0);\n"
              "   if (pool) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #8499
        check("void f(void)\n"
              "{\n"
              "    for (int i = 0; i < 2; ++i)\n"
              "    {\n"
              "        for (int j = 0; j < 8; ++j)\n"
              "        {\n"
              "            if ( (i==0|| i==1)\n" // << always true
              "              && (j==0) )\n"
              "            {;}\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:20] -> [test.cpp:7:27]: (style) Condition 'i==1' is always true [knownConditionTrueFalse]\n", errout_str());

        // #10863
        check("void f(const int A[], int Len) {\n"
              "  if (Len <= 0)\n"
              "    return;\n"
              "  int I = 0;\n"
              "  while (I < Len) {\n"
              "    int K = I + 1;\n"
              "    for (; K < Len; K++) {\n"
              "      if (A[I] != A[K])\n"
              "        break;\n"
              "    } \n"
              "    I = K;   \n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #11434
              "    const int N = 5;\n"
              "    bool a[N];\n"
              "    for (int i = 0; i < N; a[i++] = false);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #8192
              "    for (int i = 0; i > 10; ++i) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (style) Condition 'i>10' is always false\n", "", errout_str());

        check("void f() {\n"
              "    for (int i = 1000; i < 20; ++i) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:26]: (style) Condition 'i<20' is always false [knownConditionTrueFalse]\n", errout_str());

        check("int foo(int foo, int bar, bool baz, bool flag) {\n"
              "    if (baz && flag) {\n"
              "        do {\n"
              "            if (bar==42)\n"
              "                return 0;\n"
              "        } while (flag);\n"
              "    }\n"
              "    return foo;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:16] -> [test.cpp:6:18]: (style) Condition 'flag' is always true [knownConditionTrueFalse]\n", errout_str());
    }

    void alwaysTrueTryCatch()
    {
        check("void g();\n"
              "void f(int x)\n"
              "{\n"
              "    if( x ) {\n"
              "        try {\n"
              "            g();\n"
              "        }\n"
              "        catch(...) {\n"
              "            return;\n"
              "        }\n"
              "    }\n"
              "    g();\n"
              "    if( x ) {\n"
              "        g();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g();\n"
              "void h();\n"
              "void f(int x) {\n"
              "    if( x ) {\n"
              "        try {\n"
              "            g();\n"
              "            return;\n"
              "        }\n"
              "        catch( ... ) {}\n"
              "    }\n"
              "    h();\n"
              "    if( x ) {\n"
              "        g();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #10701
              "    std::string s;\n"
              "    try {\n"
              "        try {\n"
              "            s = g();\n"
              "        }\n"
              "        catch (const Err& err) {}\n"
              "    }\n"
              "    catch (const std::exception& e) {}\n"
              "    if (s != \"abc\") {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void alwaysTrueSideEffect() {
        check("bool check(const char* const);\n" // #14416
              "void create(const char*);\n"
              "void f(const char* n) {\n"
              "    if (!check(n)) {\n"
              "        create(n);\n"
              "        if (check(n)) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void alwaysTruePremiumMisra() {
        const char code[] = "void f() {\n"
                            "  if (3 > 2) {}\n"
                            "}";

        check(code);
        ASSERT_EQUALS("", errout_str());

        Settings misraSettings;
        misraSettings.premiumArgs = "--misra-c-2012";
        check(code, misraSettings);
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Condition '3>2' is always true [knownConditionTrueFalse]\n", errout_str());
    }

    void multiConditionAlwaysTrue() {
        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) continue;\n"
              "  if (val > 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) {\n"
              "    if (val > 0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int val = 0;\n"
              "  if (val < 0) {\n"
              "    if (val < 0) {}\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int activate = 0;\n"
              "  int foo = 0;\n"
              "  if (activate) {}\n"
              "  else if (foo) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Condition 'activate' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:5:12]: (style) Condition 'foo' is always false [knownConditionTrueFalse]\n", errout_str());

        // #6904
        check("void f() {\n"
              "  const int b[2] = { 1,0 };\n"
              "  if(b[1] == 2) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'b[1]==2' is always false [knownConditionTrueFalse]\n", errout_str());

        // #9878
        check("void f(bool a, bool b) {\n"
              "    if (a && b){;}\n"
              "    else if (!a && b){;}\n"
              "    else if (!a && !b){;}\n"
              "    else {;}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void duplicateCondition() {
        check("void f(bool x) {\n"
              "    if(x) {}\n"
              "    if(x) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:8] -> [test.cpp:3:8]: (style) The if condition is the same as the previous if condition [duplicateCondition]\n",
                      errout_str());

        check("void f(int x) {\n"
              "    if(x == 1) {}\n"
              "    if(x == 1) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10] -> [test.cpp:3:10]: (style) The if condition is the same as the previous if condition [duplicateCondition]\n",
                      errout_str());

        check("void f(int x) {\n"
              "    if(x == 1) {}\n"
              "    if(x == 2) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if(x == 1) {}\n"
              "    if(x != 1) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool x) {\n"
              "    if(x) {}\n"
              "    g();\n"
              "    if(x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if(x == 1) { x++; }\n"
              "    if(x == 1) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8996
        check("void g(int** v);\n"
              "void f() {\n"
              "  int a = 0;\n"
              "  int b = 0;\n"
              "  int* d[] = {&a, &b};\n"
              "  g(d);\n"
              "  if (a) {}\n"
              "  if (b) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9311
        check("struct c {\n"
              "  int* p;\n"
              "};\n"
              "void g(struct c* v);\n"
              "void f() {\n"
              "  int a = 0;\n"
              "  int b = 0;\n"
              "  struct c d[] = {{&a}, {&b}};\n"
              "  g(d);\n"
              "  if (a) {}\n"
              "  if (b) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8993
        check("void f(const std::string& x) {\n"
              "  auto y = x;\n"
              "  if (x.empty()) y = \"1\";\n"
              "  if (y.empty()) return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9106
        check("struct A {int b;};\n"
              "void f(A a, int c) {\n"
              "    if (a.b) a.b = c;\n"
              "    if (a.b) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int a;\n"
              "    void b() const {\n"
              "        return a == 1;\n"
              "    }\n"
              "    void c();\n"
              "    void d() {\n"
              "        if(b()) {\n"
              "            c();\n"
              "        }\n"
              "        if (b()) {\n"
              "            a = 3;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int a;\n"
              "    void b() const {\n"
              "        return a == 1;\n"
              "    }\n"
              "    void d() {\n"
              "        if(b()) {\n"
              "            a = 2;\n"
              "        }\n"
              "        if (b()) {\n"
              "            a = 3;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int a;\n"
              "    void b() const {\n"
              "        return a == 1;\n"
              "    }\n"
              "    void d() {\n"
              "        if(b()) {\n"
              "        }\n"
              "        if (b()) {\n"
              "            a = 3;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:13] -> [test.cpp:9:14]: (style) The if condition is the same as the previous if condition [duplicateCondition]\n",
                      errout_str());

        check("void f(bool a, bool b) {\n"
              "    auto g = [&] { b = !a; };\n"
              "    if (b)\n"
              "        g();\n"
              "    if (b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(bool& a);\n"
              "void f(bool b) {\n"
              "    auto h = std::bind(&g, std::ref(b));\n"
              "    if (b)\n"
              "        h();\n"
              "    if (b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *i) {\n"
              "  if (*i == 0) {\n"
              "    *i = 1;\n"
              "  }\n"
              "  if (*i == 0) {\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(std::function<void()>);\n"
              "void f(std::vector<int> v) {\n"
              "    auto x = [&v] { v.push_back(1); };\n"
              "    if(v.empty()) {\n"
              "        g(x);\n"
              "    }\n"
              "    if(v.empty())\n"
              "        return;\n"
              "    return;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n"
              "int f(const S& s) {\n"
              "    int a = 0, b = 0;\n"
              "    if (s.i == 0)\n"
              "        a = 1;\n"
              "    if (s.i == 0)\n"
              "        b = 1;\n"
              "    return a + b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:13] -> [test.cpp:6:13]: (style) The if condition is the same as the previous if condition [duplicateCondition]\n", errout_str());

        check("void f(double d) {\n" // #12712
              "    if (std::isfinite(d)) {}\n"
              "    if (std::isfinite(d)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:22] -> [test.cpp:3:22]: (style) The if condition is the same as the previous if condition [duplicateCondition]\n", errout_str());

        check("struct S { int x; };\n" // #12391
              "int f(const struct S* a, const struct S* b) {\n"
              "    const struct S* p = b;\n"
              "    if (a->x < p->x) p++;\n"
              "    if (a->x < p->x) p++;\n"
              "    return p->x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // do not crash
        check("void assign(const MMA& other) {\n"
              "    if (mPA.cols != other.mPA.cols || mPA.rows != other.mPA.rows)\n"
              "        ;\n"
              "    if (other.mPA.cols > 0 && other.mPA.rows > 0)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n" // #11202
              "    float f = 0x1.4p+3;\n"
              "    if (f > 10.0) {}\n"
              "    if (f < 10.0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(char* s, const char* p) {\n" // #14070
              "    strcpy(s, p);\n"
              "}\n"
              "void g() {\n"
              "    char s1[10] = \"\";\n"
              "    char s2[10] = \"\";\n"
              "    f(s1, \"123\");\n"
              "    f(s2, \"1234\");\n"
              "    if (strlen(s1) > 0) {}\n"
              "    if (strlen(s2) > 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(int*);\n" // #14428
              "int f(int* const p) {\n"
              "    int i = 0;\n"
              "    if (*p == 0)\n"
              "        g(p);\n"
              "    if (*p == 0)\n"
              "        i = 1;\n"
              "    return i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(const int*);\n"
              "int f(const int* const p) {\n"
              "    int i = 0;\n"
              "    if (*p == 0)\n"
              "        g(p);\n"
              "    if (*p == 0)\n"
              "        i = 1;\n"
              "    return i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:12] -> [test.cpp:6:12]: (style) The if condition is the same as the previous if condition [duplicateCondition]\n", errout_str());
    }

    void checkInvalidTestForOverflow() {
        check("void f(char *p, unsigned int x) {\n"
              "    assert((p + x) < p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (warning) Invalid test for overflow '(p+x)<p'; pointer overflow is undefined behavior. Some mainstream compilers remove such overflow tests when optimising the code and assume it's always false. [invalidTestForOverflow]\n", errout_str());

        check("void f(char *p, unsigned int x) {\n"
              "    assert((p + x) >= p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (warning) Invalid test for overflow '(p+x)>=p'; pointer overflow is undefined behavior. Some mainstream compilers remove such overflow tests when optimising the code and assume it's always true. [invalidTestForOverflow]\n", errout_str());

        check("void f(char *p, unsigned int x) {\n"
              "    assert(p > (p + x));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (warning) Invalid test for overflow 'p>(p+x)'; pointer overflow is undefined behavior. Some mainstream compilers remove such overflow tests when optimising the code and assume it's always false. [invalidTestForOverflow]\n", errout_str());

        check("void f(char *p, unsigned int x) {\n"
              "    assert(p <= (p + x));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (warning) Invalid test for overflow 'p<=(p+x)'; pointer overflow is undefined behavior. Some mainstream compilers remove such overflow tests when optimising the code and assume it's always true. [invalidTestForOverflow]\n", errout_str());

        check("void f(signed int x) {\n" // unsigned overflow => don't warn
              "    assert(x + 100U < x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        // x + c < x

#define MSG(EXPR, RESULT)   "[test.cpp:1:30]: (warning) Invalid test for overflow '" EXPR "'; signed integer overflow is undefined behavior. Some mainstream compilers remove such overflow tests when optimising the code and assume it's always " RESULT ". [invalidTestForOverflow]\n"

        check("int f(int x) { return x + 10 > x; }");
        ASSERT_EQUALS(MSG("x+10>x", "true"), errout_str());

        check("int f(int x) { return x + 10 >= x; }");
        ASSERT_EQUALS(MSG("x+10>=x", "true"), errout_str());

        check("int f(int x) { return x + 10 < x; }");
        ASSERT_EQUALS(MSG("x+10<x", "false"), errout_str());

        check("int f(int x) { return x + 10 <= x; }");
        ASSERT_EQUALS(MSG("x+10<=x", "false"), errout_str());

        check("int f(int x) { return x - 10 > x; }");
        ASSERT_EQUALS(MSG("x-10>x", "false"), errout_str());

        check("int f(int x) { return x - 10 >= x; }");
        ASSERT_EQUALS(MSG("x-10>=x", "false"), errout_str());

        check("int f(int x) { return x - 10 < x; }");
        ASSERT_EQUALS(MSG("x-10<x", "true"), errout_str());

        check("int f(int x) { return x - 10 <= x; }");
        ASSERT_EQUALS(MSG("x-10<=x", "true"), errout_str());

        // x + y < x
#undef MSG
#define MSG(EXPR, RESULT)   "[test.cpp:1:36]: (warning) Invalid test for overflow '" EXPR "'; signed integer overflow is undefined behavior. Some mainstream compilers removes handling of overflows when optimising the code and change the code to '" RESULT "'. [invalidTestForOverflow]\n"

        check("int f(int x, int y) { return x + y < x; }");
        ASSERT_EQUALS(MSG("x+y<x", "y<0"), errout_str());

        check("int f(int x, int y) { return x + y <= x; }");
        ASSERT_EQUALS(MSG("x+y<=x", "y<=0"), errout_str());

        check("int f(int x, int y) { return x + y > x; }");
        ASSERT_EQUALS(MSG("x+y>x", "y>0"), errout_str());

        check("int f(int x, int y) { return x + y >= x; }");
        ASSERT_EQUALS(MSG("x+y>=x", "y>=0"), errout_str());

        // x - y < x
        check("int f(int x, int y) { return x - y < x; }");
        ASSERT_EQUALS(MSG("x-y<x", "y>0"), errout_str());

        check("int f(int x, int y) { return x - y <= x; }");
        ASSERT_EQUALS(MSG("x-y<=x", "y>=0"), errout_str());

        check("int f(int x, int y) { return x - y > x; }");
        ASSERT_EQUALS(MSG("x-y>x", "y<0"), errout_str());

        check("int f(int x, int y) { return x - y >= x; }");
        ASSERT_EQUALS(MSG("x-y>=x", "y<=0"), errout_str());
    }

    void checkConditionIsAlwaysTrueOrFalseInsideIfWhile() {
        check("void f() {\n"
              "    enum states {A,B,C};\n"
              "    const unsigned g_flags = B|C;\n"
              "    if(g_flags & A) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (style) Condition 'g_flags&A' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "    int a = 5;"
              "    if(a) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (style) Condition 'a' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void f() {\n"
              "    int a = 5;"
              "    while(a + 1) { a--; }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a = 5;"
              "    while(a + 1) { return; }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:27]: (style) Condition 'a+1' is always true [knownConditionTrueFalse]\n", errout_str());
    }

    void alwaysTrueFalseInLogicalOperators() {
        check("bool f();\n"
              "void foo() { bool x = true; if(x||f()) {}}");
        ASSERT_EQUALS("[test.cpp:2:32]: (style) Condition 'x' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void foo(bool b) { bool x = true; if(x||b) {}}");
        ASSERT_EQUALS("[test.cpp:1:38]: (style) Condition 'x' is always true [knownConditionTrueFalse]\n", errout_str());

        check("void foo(bool b) { if(true||b) {}}");
        ASSERT_EQUALS("", errout_str());

        check("bool f();\n"
              "void foo() { bool x = false; if(x||f()) {}}");
        ASSERT_EQUALS("[test.cpp:2:33]: (style) Condition 'x' is always false [knownConditionTrueFalse]\n", errout_str());

        check("bool f();\n"
              "void foo() { bool x = false; if(x&&f()) {}}");
        ASSERT_EQUALS("[test.cpp:2:33]: (style) Condition 'x' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo(bool b) { bool x = false; if(x&&b) {}}");
        ASSERT_EQUALS("[test.cpp:1:39]: (style) Condition 'x' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo(bool b) { if(false&&b) {}}");
        ASSERT_EQUALS("", errout_str());

        check("bool f();\n"
              "void foo() { bool x = true; if(x&&f()) {}}");
        ASSERT_EQUALS("[test.cpp:2:32]: (style) Condition 'x' is always true [knownConditionTrueFalse]\n", errout_str());

        // #9578
        check("bool f(const std::string &s) {\n"
              "        return s.size()>2U && s[0]=='4' && s[0]=='2';\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:35] -> [test.cpp:2:48]: (style) Return value 's[0]=='2'' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void f(int i) { if (i == 1 || 2) {} }\n"); // #12487
        ASSERT_EQUALS("[test.cpp:1:28]: (style) Condition 'i==1||2' is always true [knownConditionTrueFalse]\n", errout_str());

        check("enum E { E1 = 1, E2 = 2 };\n"
              "void f(int i) { if (i == E1 || E2) {} }\n");
        ASSERT_EQUALS("[test.cpp:2:29]: (style) Condition 'i==E1||E2' is always true [knownConditionTrueFalse]\n", errout_str());
    }

    void pointerAdditionResultNotNull() {
        check("void f(char *ptr) {\n"
              "  if (ptr + 1 != 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Comparison is wrong. Result of 'ptr+1' can't be 0 unless there is pointer overflow, and pointer overflow is undefined behaviour. [pointerAdditionResultNotNull]\n", errout_str());
    }

    void duplicateConditionalAssign() {
        setMultiline();

        check("void f(int& x, int y) {\n"
              "    if (x == y)\n"
              "        x = y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: style: Assignment 'x=y' is redundant with condition 'x==y'. [duplicateConditionalAssign]\n"
                      "[test.cpp:2:11]: note: Condition 'x==y'\n"
                      "[test.cpp:3:11]: note: Assignment 'x=y' is redundant\n", errout_str());

        check("void f(int& x, int y) {\n"
              "    if (x != y)\n"
              "        x = y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: style: The statement 'if (x!=y) x=y' is logically equivalent to 'x=y'. [duplicateConditionalAssign]\n"
                      "[test.cpp:3:11]: note: Assignment 'x=y'\n"
                      "[test.cpp:2:11]: note: Condition 'x!=y' is redundant\n", errout_str());

        check("void f(int& x, int y) {\n"
              "    if (x == y)\n"
              "        x = y;\n"
              "    else\n"
              "        x = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: style: Assignment 'x=y' is redundant with condition 'x==y'. [duplicateConditionalAssign]\n"
                      "[test.cpp:2:11]: note: Condition 'x==y'\n"
                      "[test.cpp:3:11]: note: Assignment 'x=y' is redundant\n", errout_str());

        check("void f(int& x, int y) {\n"
              "    if (x != y)\n"
              "        x = y;\n"
              "    else\n"
              "        x = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int& x, int y) {\n"
              "    if (x == y)\n"
              "        x = y + 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g();\n"
              "void f(int& x, int y) {\n"
              "    if (x == y) {\n"
              "        x = y;\n"
              "        g();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(bool b) {\n"
              "    if (b)\n"
              "        b = false;\n"
              "    else\n"
              "        g();\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int& i) {\n"
              "    if (!i)\n"
              "        i = 1; \n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #9406
              "    S() : b(false) {}\n"
              "    void f() {\n"
              "        if (b) b = true;\n"
              "        if (b) b = false;\n"
              "        if (!b) b = true;\n"
              "        if (!b) b = false;\n"
              "    }\n"
              "    bool b;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:4:13]: style: The statement 'if (b) b=true' is redundant. [duplicateConditionalAssign]\n"
                      "[test.cpp:4:18]: note: Assignment 'b=true'\n"
                      "[test.cpp:4:13]: note: Condition 'b' is redundant\n"
                      "[test.cpp:5:13]: style: The statement 'if (b) b=false' is logically equivalent to 'b=false'. [duplicateConditionalAssign]\n"
                      "[test.cpp:5:18]: note: Assignment 'b=false'\n"
                      "[test.cpp:5:13]: note: Condition 'b' is redundant\n"
                      "[test.cpp:6:13]: style: The statement 'if (!b) b=true' is logically equivalent to 'b=true'. [duplicateConditionalAssign]\n"
                      "[test.cpp:6:19]: note: Assignment 'b=true'\n"
                      "[test.cpp:6:13]: note: Condition '!b' is redundant\n"
                      "[test.cpp:7:13]: style: The statement 'if (!b) b=false' is redundant. [duplicateConditionalAssign]\n"
                      "[test.cpp:7:19]: note: Assignment 'b=false'\n"
                      "[test.cpp:7:13]: note: Condition '!b' is redundant\n",
                      errout_str());
    }

    void checkAssignmentInCondition() {
        check("void f(std::string s) {\n"
              "    if (s=\"123\"){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Suspicious assignment in condition. Condition 's=\"123\"' is always true. [assignmentInCondition]\n", errout_str());

        check("void f(std::string *p) {\n"
              "    if (p=foo()){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(uint32_t u) {\n" // #2490
              "    if ((u = 0x00000000) || (u = 0xffffffff)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:12]: (style) Condition 'u=0x00000000' is always false [knownConditionTrueFalse]\n"
                      "[test.cpp:2:32]: (style) Condition 'u=0xffffffff' is always true [knownConditionTrueFalse]\n",
                      errout_str());
    }

    void compareOutOfTypeRange() {
        const Settings settingsUnix64 = settingsBuilder().severity(Severity::style).platform(Platform::Type::Unix64).build();

        check("void f(unsigned char c) {\n"
              "  if (c == 256) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("[test.cpp:2:12]: (style) Comparing expression of type 'unsigned char' against value 256. Condition is always false. [compareValueOutOfTypeRangeError]\n", errout_str());

        check("void f(unsigned char* b, int i) {\n" // #6372
              "  if (b[i] == 256) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Comparing expression of type 'unsigned char' against value 256. Condition is always false. [compareValueOutOfTypeRangeError]\n", errout_str());

        check("void f(unsigned char c) {\n"
              "  if (c == 255) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n"
              "  if (b == true) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("", errout_str());

        // #10372
        check("void f(signed char x) {\n"
              "  if (x == 0xff) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("[test.cpp:2:12]: (style) Comparing expression of type 'signed char' against value 255. Condition is always false. [compareValueOutOfTypeRangeError]\n", errout_str());

        check("void f(short x) {\n"
              "  if (x == 0xffff) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("[test.cpp:2:12]: (style) Comparing expression of type 'signed short' against value 65535. Condition is always false. [compareValueOutOfTypeRangeError]\n", errout_str());

        check("void f(int x) {\n"
              "  if (x == 0xffffffff) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("", errout_str());

        check("void f(long x) {\n"
              "  if (x == ~0L) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("", errout_str());

        check("void f(long long x) {\n"
              "  if (x == ~0LL) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("", errout_str());

        check("int f(int x) {\n"
              "    const int i = 0xFFFFFFFF;\n"
              "    if (x == i) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  char c;\n"
              "  if ((c = foo()) != -1) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "  if (x < 3000000000) {}\n"
              "}", settingsUnix64);
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Comparing expression of type 'signed int' against value 3000000000. Condition is always true. [compareValueOutOfTypeRangeError]\n", errout_str());

        check("void f(const signed char i) {\n" // #8545
              "    if (i >  -129) {}\n" // warn
              "    if (i >= -128) {}\n" // warn
              "    if (i >= -127) {}\n"
              "    if (i <  +128) {}\n" // warn
              "    if (i <= +127) {}\n" // warn
              "    if (i <= +126) {}\n"
              "}\n", settingsUnix64);
        ASSERT_EQUALS("[test.cpp:2:15]: (style) Comparing expression of type 'const signed char' against value -129. Condition is always true. [compareValueOutOfTypeRangeError]\n"
                      "[test.cpp:3:15]: (style) Comparing expression of type 'const signed char' against value -128. Condition is always true. [compareValueOutOfTypeRangeError]\n"
                      "[test.cpp:5:15]: (style) Comparing expression of type 'const signed char' against value 128. Condition is always true. [compareValueOutOfTypeRangeError]\n"
                      "[test.cpp:6:15]: (style) Comparing expression of type 'const signed char' against value 127. Condition is always true. [compareValueOutOfTypeRangeError]\n",
                      errout_str());

        check("void f(const unsigned char u) {\n"
              "    if (u >  0) {}\n"
              "    if (u <  0) {}\n"
              "    if (u >= 0) {}\n"
              "    if (u <= 0) {}\n"
              "    if (u >  255) {}\n" // warn
              "    if (u <  255) {}\n"
              "    if (u >= 255) {}\n"
              "    if (u <= 255) {}\n" // warn
              "    if (0   <  u) {}\n"
              "    if (0   >  u) {}\n"
              "    if (0   <= u) {}\n"
              "    if (0   >= u) {}\n"
              "    if (255 <  u) {}\n" // warn
              "    if (255 >  u) {}\n"
              "    if (255 <= u) {}\n"
              "    if (255 >= u) {}\n" // warn
              "}\n", settingsUnix64);
        ASSERT_EQUALS("[test.cpp:6:14]: (style) Comparing expression of type 'const unsigned char' against value 255. Condition is always false. [compareValueOutOfTypeRangeError]\n"
                      "[test.cpp:9:14]: (style) Comparing expression of type 'const unsigned char' against value 255. Condition is always true. [compareValueOutOfTypeRangeError]\n"
                      "[test.cpp:14:9]: (style) Comparing expression of type 'const unsigned char' against value 255. Condition is always false. [compareValueOutOfTypeRangeError]\n"
                      "[test.cpp:17:9]: (style) Comparing expression of type 'const unsigned char' against value 255. Condition is always true. [compareValueOutOfTypeRangeError]\n",
                      errout_str());

        check("void f(bool b) {\n" // #14037
              "    if (b != 2) {}\n"
              "}\n", settingsUnix64);
        ASSERT_EQUALS("[test.cpp:2:14]: (style) Comparing expression of type 'bool' against value 2. Condition is always true. [compareValueOutOfTypeRangeError]\n",
                      errout_str());

        check("void f(const std::uint32_t& u) {\n" // #9078
              "    if (u >= UINT32_MAX) {}\n"
              "    if (u <= UINT32_MAX) {}\n"
              "    if (u > UINT32_MAX) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:14]: (style) Comparing expression of type 'const unsigned int &' against value 4294967295. Condition is always true. [compareValueOutOfTypeRangeError]\n"
                      "[test.cpp:4:13]: (style) Comparing expression of type 'const unsigned int &' against value 4294967295. Condition is always false. [compareValueOutOfTypeRangeError]\n",
                      errout_str());
    }

    void knownConditionCast() {
        check("void f(int i) {\n" // #9976
              "    if (i < 0 || (unsigned)i > 5) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct B {\n" // #12941
              "    virtual void f();\n"
              "};\n"
              "struct One : public B {};\n"
              "struct Two : public B {};\n"
              "void g(const B& b) {\n"
              "    const Two* two = nullptr;\n"
              "    const One* one = dynamic_cast<const One*>(&b);\n"
              "    if (one == nullptr)\n"
              "        two = dynamic_cast<const Two*>(&b);\n"
              "    if (two) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void knownConditionIncrementLoop() { // #9808
        check("void f() {\n"
              "    int a = 0;\n"
              "    while (++a < 5) {}\n"
              "    if (a == 1) {}\n"
              "    std::cout << a;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void knownConditionAfterBailout() { // #12526
        check(
            "#include <list>\n"
            "int func()\n"
            "{\n"
            "  return VALUE_1;"
            "}\n"
            "\n"
            "struct S1 {\n"
            "  bool b{};\n"
            "};\n"
            "\n"
            "struct S {\n"
            "  void f(const std::list<int>& l) const\n"
            "  {\n"
            "    if (mS.b)\n"
            "      return;\n"
            "    for (int i : l)\n"
            "    {\n"
            "      (void)i;\n"
            "      if (mS.b)\n"
            "        continue;\n"
            "    }\n"
            "  }\n"
            "\n"
            "  S1 mS;\n"
            "};"
            );
        ASSERT_EQUALS("[test.cpp:13:11] -> [test.cpp:18:13]: (style) Condition 'mS.b' is always false [knownConditionTrueFalse]\n", errout_str());
    }

    void knownConditionIncDecOperator() {
        check(
            "void f() {\n"
            "    unsigned int d = 0;\n"
            "    for (int i = 0; i < 4; ++i) {\n"
            "        if (i < 3)\n"
            "            ++d;\n"
            "        else if (--d == 0)\n"
            "            ;\n"
            "    }\n"
            "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void knownConditionFloating() {
        check("void foo() {\n"   // #11199
              "    float f = 1.0;\n"
              "    if (f > 1.0f) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'f>1.0f' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"     // #11199
              "    float f = 1.0;\n"
              "    if (f > 1.0L) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'f>1.0L' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"   // #11199
              "    float f = 1.0f;\n"
              "    if (f > 1.0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'f>1.0' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"     // #11199
              "    float f = 1.0f;\n"
              "    if (f > 1.0L) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'f>1.0L' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"     // #11199
              "    float f = 1.0L;\n"
              "    if (f > 1.0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'f>1.0' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"     // #11199
              "    float f = 1.0L;\n"
              "    if (f > 1.0f) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'f>1.0f' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"   // #11201
              "    float f = 0x1.4p+3;\n" // hex fraction 1.4 (decimal 1.25) scaled by 2^3, that is 10.0
              "    if (f > 9.9) {}\n"
              "    if (f < 9.9) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:11]: (style) Condition 'f>9.9' is always true [knownConditionTrueFalse]\n"
            "[test.cpp:4:11]: (style) Condition 'f<9.9' is always false [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #12330
              "    double d = 1.0;\n"
              "    if (d < 0.0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'd<0.0' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"     // #12330
              "    long double ld = 1.0;\n"
              "    if (ld < 0.0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) Condition 'ld<0.0' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"     // #12330
              "    float f = 1.0;\n"
              "    if (f < 0.0) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Condition 'f<0.0' is always false [knownConditionTrueFalse]\n", errout_str());

        check("void foo() {\n"     // #12774
              "    float f = 1.0f;\n"
              "    if (f > 1.01f) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:11]: (style) Condition 'f>1.01f' is always false [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n"     // #12774
              "    float f = 1.0;\n"
              "    if (f > 1.01) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:11]: (style) Condition 'f>1.01' is always false [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n"
              "    float f = 1.0f;\n"
              "    if (f > 1) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:11]: (style) Condition 'f>1' is always false [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #13508
              "    float f = 1.0f;\n"
              "    if (f > 1.00f) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:3]: (style) Condition 'f>1.00f' is always false\n",
            "",
            errout_str());

        check("void foo() {\n"
              "    float f = 1.0;\n"
              "    if (f > 1) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:11]: (style) Condition 'f>1' is always false [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n"// #13508
              "    float f = 1.0;\n"
              "    if (f > 1.00) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:3]: (style) Condition 'f>1.00' is always false\n",
            "",
            errout_str());

        check("void foo() {\n" // #13506
              "    float nf = -1.0;\n"
              "    if (nf > +1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12]: (style) Condition 'nf>+1.0' is always false [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #11200
              "    float f = 1.0;\n"
              "    if (f > -1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:11]: (style) Condition 'f>-1.0' is always true [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #13508
              "    float f = 1.0;\n"
              "    if (f > 1.0) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:3]: (style) Condition 'f>1.0' is always true\n",
            "",
            errout_str());

        check("void foo() {\n" // #11200
              "    float pf = +1.0;\n"
              "    if (pf > -1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12]: (style) Condition 'pf>-1.0' is always true [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // #13508
              "    float pf = +1.0;\n"
              "    if (pf > 1.0) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:3]: (style) Condition 'pf>1.0' is always true\n",
            "",
            errout_str());

        check("void foo() {\n" // #11200
              "    float nf = -1.0;\n"
              "    if (nf > 1.0) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12]: (style) Condition 'nf>1.0' is always false [knownConditionTrueFalse]\n",
            errout_str());

        check("void foo() {\n" // / #13508
              "    float nf = -1.0;\n"
              "    if (nf > -1.0) {}\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:3]: (style) Condition 'nf>-1.0' is always false\n",
            "",
            errout_str());
    }
};

REGISTER_TEST(TestCondition)
