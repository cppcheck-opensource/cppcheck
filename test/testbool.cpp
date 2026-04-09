/*
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

#include "checkbool.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>

class TestBool : public TestFixture {
public:
    TestBool() : TestFixture("TestBool") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::style).severity(Severity::warning).certainty(Certainty::inconclusive).build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(bitwiseOnBoolean);      // if (bool & bool)
        TEST_CASE(incrementBoolean);
        TEST_CASE(assignBoolToPointer);
        TEST_CASE(assignBoolToFloat);

        TEST_CASE(comparisonOfBoolExpressionWithInt1);
        TEST_CASE(comparisonOfBoolExpressionWithInt2);
        TEST_CASE(comparisonOfBoolExpressionWithInt3);
        TEST_CASE(comparisonOfBoolExpressionWithInt4);

        TEST_CASE(comparisonOfBoolWithInt1);
        TEST_CASE(comparisonOfBoolWithInt2);
        TEST_CASE(comparisonOfBoolWithInt3);
        TEST_CASE(comparisonOfBoolWithInt4);
        TEST_CASE(comparisonOfBoolWithInt5);
        TEST_CASE(comparisonOfBoolWithInt6); // #4224 - integer is casted to bool
        TEST_CASE(comparisonOfBoolWithInt7); // #4846 - (!x == true)
        TEST_CASE(comparisonOfBoolWithInt8); // #9165
        TEST_CASE(comparisonOfBoolWithInt9); // #9304
        TEST_CASE(comparisonOfBoolWithInt10); // #10935

        TEST_CASE(checkComparisonOfFuncReturningBool1);
        TEST_CASE(checkComparisonOfFuncReturningBool2);
        TEST_CASE(checkComparisonOfFuncReturningBool3);
        TEST_CASE(checkComparisonOfFuncReturningBool4);
        TEST_CASE(checkComparisonOfFuncReturningBool5);
        TEST_CASE(checkComparisonOfFuncReturningBool6);
        TEST_CASE(checkComparisonOfFuncReturningBool7); // #7197
        TEST_CASE(checkComparisonOfFuncReturningBool8); // #4103
        // Integration tests..
        TEST_CASE(checkComparisonOfFuncReturningBoolIntegrationTest1); // #7798 overloaded functions

        TEST_CASE(checkComparisonOfBoolWithBool);

        // Converting pointer addition result to bool
        TEST_CASE(pointerArithBool1);

        TEST_CASE(returnNonBool);
        TEST_CASE(returnNonBoolLambda);
        TEST_CASE(returnNonBoolLogicalOp);
        TEST_CASE(returnNonBoolClass);
    }

    struct CheckOptions
    {
        bool cpp = true;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this, options.cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check...
        runChecks<CheckBool>(tokenizer, this);
    }


    void assignBoolToPointer() {
        check("void foo(bool *p) {\n"
              "    p = false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (error) Boolean value assigned to pointer. [assignBoolToPointer]\n", errout_str());

        check("void foo(bool *p) {\n"
              "    p = (x<y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (error) Boolean value assigned to pointer. [assignBoolToPointer]\n", errout_str());

        check("void foo(bool *p) {\n"
              "    p = (x||y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (error) Boolean value assigned to pointer. [assignBoolToPointer]\n", errout_str());

        check("void foo(bool *p) {\n"
              "    p = (x&&y);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (error) Boolean value assigned to pointer. [assignBoolToPointer]\n", errout_str());

        // check against potential false positives
        check("void foo(bool *p) {\n"
              "    *p = false;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #5046 - false positive: Boolean value assigned to pointer
        check("struct S {\n"
              "    bool *p;\n"
              "};\n"
              "void f() {\n"
              "    S s = {0};\n"
              "    *s.p = true;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    bool *p;\n"
              "};\n"
              "void f() {\n"
              "    S s = {0};\n"
              "    s.p = true;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:9]: (error) Boolean value assigned to pointer. [assignBoolToPointer]\n", errout_str());

        // ticket #5627 - false positive: template
        check("void f() {\n"
              "    X *p = new ::std::pair<int,int>[rSize];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #6588 (c mode)
        check("struct MpegEncContext { int *q_intra_matrix, *q_chroma_intra_matrix; };\n"
              "void dnxhd_10bit_dct_quantize(MpegEncContext *ctx, int n, int qscale) {\n"
              "  const int *qmat = n < 4;\n" /* KO */
              "  const int *rmat = n < 4 ? " /* OK */
              "                       ctx->q_intra_matrix :"
              "                       ctx->q_chroma_intra_matrix;\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:19]: (error) Boolean value assigned to pointer. [assignBoolToPointer]\n", errout_str());

        // ticket #6588 (c++ mode)
        check("struct MpegEncContext { int *q_intra_matrix, *q_chroma_intra_matrix; };\n"
              "void dnxhd_10bit_dct_quantize(MpegEncContext *ctx, int n, int qscale) {\n"
              "  const int *qmat = n < 4;\n" /* KO */
              "  const int *rmat = n < 4 ? " /* OK */
              "                       ctx->q_intra_matrix :"
              "                       ctx->q_chroma_intra_matrix;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (error) Boolean value assigned to pointer. [assignBoolToPointer]\n", errout_str());

        // ticket #6665
        check("void pivot_big(char *first, int compare(const void *, const void *)) {\n"
              "  char *a = first, *b = first + 1, *c = first + 2;\n"
              "  char* m1 = compare(a, b) < 0\n"
              "      ? (compare(b, c) < 0 ? b : (compare(a, c) < 0 ? c : a))\n"
              "      : (compare(a, c) < 0 ? a : (compare(b, c) < 0 ? c : b));\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        // #7381
        check("void foo(bool *p, bool b) {\n"
              "    p = b;\n"
              "    p = &b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (error) Boolean value assigned to pointer. [assignBoolToPointer]\n", errout_str());
    }

    void assignBoolToFloat() {
        check("void foo1() {\n"
              "    double d = false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (style) Boolean value assigned to floating point variable. [assignBoolToFloat]\n", errout_str());

        check("void foo2() {\n"
              "    float d = true;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (style) Boolean value assigned to floating point variable. [assignBoolToFloat]\n", errout_str());

        check("void foo3() {\n"
              "    long double d = (2>1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:19]: (style) Boolean value assigned to floating point variable. [assignBoolToFloat]\n", errout_str());

        // stability - don't crash:
        check("void foo4() {\n"
              "    unknown = false;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    float p;\n"
              "};\n"
              "void f() {\n"
              "    S s = {0};\n"
              "    s.p = true;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:9]: (style) Boolean value assigned to floating point variable. [assignBoolToFloat]\n", errout_str());

        check("struct S {\n"
              "    float* p[1];\n"
              "};\n"
              "void f() {\n"
              "    S s = {0};\n"
              "    *s.p[0] = true;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:13]: (style) Boolean value assigned to floating point variable. [assignBoolToFloat]\n", errout_str());
    }

    void comparisonOfBoolExpressionWithInt1() {
        check("void f(int x) {\n"
              "    if ((x && 0x0f)==6)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x && 0x0f)==0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if ((x || 0x0f)==6)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:20]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((x || 0x0f)==0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if ((x & 0x0f)==6)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if ((x | 0x0f)==6)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        check("void f(int x) {\n"
              "    if ((5 && x)==3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((5 && x)==3 || (8 && x)==9)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:17]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n"
            "[test.cpp:2:32]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n",
            errout_str());

        check("void f(int x) {\n"
              "    if ((5 && x)!=3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());


        check("void f(int x) {\n"
              "    if ((5 && x) > 3)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((5 && x) > 0)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if ((5 && x) < 0)\n"
              "        a++;\n"
              "}"
              );
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Comparison of a boolean expression with an integer. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x) {\n"
              "    if ((5 && x) < 1)\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if ((5 && x) > 1)\n"
              "        a++;\n"
              "}"
              );
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Comparison of a boolean expression with an integer. [compareBoolExpressionWithInt]\n", errout_str());


        check("void f(int x) {\n"
              "    if (0 < (5 && x))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (0 > (5 && x))\n"
              "        a++;\n"
              "}"
              );
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of a boolean expression with an integer. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x) {\n"
              "    if (1 > (5 && x))\n"
              "        a++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (1 < (5 && x))\n"
              "        a++;\n"
              "}"
              );
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of a boolean expression with an integer. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(bool x ) {\n"
              "  if ( x > false )\n"
              "      a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=). [comparisonOfBoolWithInvalidComparator]\n", errout_str());

        check("void f(bool x ) {\n"
              "  if ( false < x )\n"
              "      a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=). [comparisonOfBoolWithInvalidComparator]\n", errout_str());

        check("void f(bool x ) {\n"
              "  if ( x < false )\n"
              "      a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=). [comparisonOfBoolWithInvalidComparator]\n", errout_str());

        check("void f(bool x ) {\n"
              "  if ( false > x )\n"
              "      a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=). [comparisonOfBoolWithInvalidComparator]\n", errout_str());

        check("void f(bool x ) {\n"
              "  if ( x >= false )\n"
              "      a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=). [comparisonOfBoolWithInvalidComparator]\n", errout_str());

        check("void f(bool x ) {\n"
              "  if ( false >= x )\n"
              "      a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=). [comparisonOfBoolWithInvalidComparator]\n", errout_str());

        check("void f(bool x ) {\n"
              "  if ( x <= false )\n"
              "      a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=). [comparisonOfBoolWithInvalidComparator]\n", errout_str());

        check("void f(bool x ) {\n"
              "  if ( false <= x )\n"
              "      a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=). [comparisonOfBoolWithInvalidComparator]\n", errout_str());

        check("typedef int (*func)(bool invert);\n"
              "void x(int, func f);\n"
              "void foo(int error) {\n"
              "  if (error == ABC) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() { return !a+b<c; }"); // #5072
        ASSERT_EQUALS("",errout_str());

        check("int f() { return (!a+b<c); }");
        ASSERT_EQUALS("",errout_str());

        check("int f() { return (a+(b<5)<=c); }");
        ASSERT_EQUALS("",errout_str());
    }

    void comparisonOfBoolExpressionWithInt2() {
        check("void f(int x) {\n"
              "    if (!x == 10) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x) {\n"
              "    if (!x != 10) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x) {\n"
              "    if (x != 10) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (10 == !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x) {\n"
              "    if (10 != !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int x, int y) {\n"
              "    if (y != !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, bool y) {\n"
              "    if (y != !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    if (10 != x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, int y) {\n"
              "    return (!y == !x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int a) {\n"
              "  return (x()+1 == !a);\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        check("void f() { if (!!a+!!b+!!c>1){} }");
        ASSERT_EQUALS("",errout_str());

        check("void f(int a, int b, int c) { if (a != !b || c) {} }");
        ASSERT_EQUALS("",errout_str());

        check("void f(int a, int b, int c) { if (1 < !!a + !!b + !!c) {} }");
        ASSERT_EQUALS("",errout_str());

        check("void f(int a, int b, int c) { if (1 < !(a+b)) {} }");
        ASSERT_EQUALS("[test.cpp:1:37]: (warning) Comparison of a boolean expression with an integer. [compareBoolExpressionWithInt]\n",errout_str());
    }

    void comparisonOfBoolExpressionWithInt3() {
        check("int f(int x) {\n"
              "    return t<0>() && x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void comparisonOfBoolExpressionWithInt4() {
        // #5016
        check("void f() {\n"
              "  for(int i = 4; i > -1 < 5 ; --i) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(int a, int b, int c) {\n"
              "  return (a > b) < c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a, int b, int c) {\n"
              "  return x(a > b) < c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a, int b, int c) {\n"
              "  return a > b == c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // templates
        check("struct Tokenizer { TokenList list; };\n"
              "void Tokenizer::f() {\n"
              "  std::list<Token*> locationList;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5063 - or
        check("void f() {\n"
              "  return a > b or c < d;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "  return (a < b) != 0U;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("int f() {\n"
              "  return (a < b) != 0x0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("int f() {\n"
              "  return (a < b) != 42U;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());
    }

    void checkComparisonOfFuncReturningBool1() {
        check("void f(){\n"
              "     int temp = 4;\n"
              "     if(compare1(temp) > compare2(temp)){\n"
              "         printf(\"foo\");\n"
              "     }\n"
              "}\n"
              "bool compare1(int temp){\n"
              "     if(temp==4){\n"
              "         return true;\n"
              "     }\n"
              "     else\n"
              "         return false;\n"
              "}\n"
              "bool compare2(int temp){\n"
              "     if(temp==4){\n"
              "         return false;\n"
              "     }\n"
              "     else\n"
              "         return true;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfTwoFuncsReturningBoolError]\n", errout_str());
    }

    void checkComparisonOfFuncReturningBool2() {
        check("void leftOfComparison(){\n"
              " int temp = 4;\n"
              " bool a = true;\n"
              " if(compare(temp) > a){\n"
              "     printf(\"foo\");\n"
              " }\n"
              "}\n"
              "void rightOfComparison(){\n"
              " int temp = 4;\n"
              " bool a = true;\n"
              " if(a < compare(temp)){\n"
              "     printf(\"foo\");\n"
              " }\n"
              "}\n"
              "bool compare(int temp){\n"
              "  if(temp==4){\n"
              "     return true;\n"
              "  }\n"
              "    else\n"
              "     return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Comparison of a function returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfFuncReturningBoolError]\n"
                      "[test.cpp:11:7]: (style) Comparison of a function returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfFuncReturningBoolError]\n", errout_str());
    }

    void checkComparisonOfFuncReturningBool3() {
        check("void f(){\n"
              " int temp = 4;\n"
              " if(compare(temp) > temp){\n"
              "         printf(\"foo\");\n"
              "   }\n"
              "}\n"
              "bool compare(int temp);");
        ASSERT_EQUALS("[test.cpp:3:19]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n"
                      "[test.cpp:3:12]: (style) Comparison of a function returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfFuncReturningBoolError]\n", errout_str());
    }

    void checkComparisonOfFuncReturningBool4() {
        check("void f(){\n"
              "   int temp = 4;\n"
              " bool b = compare2(6);\n"
              " if(compare1(temp)> b){\n"
              "         printf(\"foo\");\n"
              " }\n"
              "}\n"
              "bool compare1(int temp){\n"
              " if(temp==4){\n"
              "     return true;\n"
              "     }\n"
              " else\n"
              "     return false;\n"
              "}\n"
              "bool compare2(int temp){\n"
              " if(temp == 5){\n"
              "     return true;\n"
              " }\n"
              " else\n"
              "     return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Comparison of a function returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfFuncReturningBoolError]\n", errout_str());
    }

    void checkComparisonOfFuncReturningBool5() {
        check("void f(){\n"
              "     int temp = 4;\n"
              "     if(compare1(temp) > !compare2(temp)){\n"
              "         printf(\"foo\");\n"
              "     }\n"
              "}\n"
              "bool compare1(int temp){\n"
              "     if(temp==4){\n"
              "         return true;\n"
              "     }\n"
              "     else\n"
              "         return false;\n"
              "}\n"
              "bool compare2(int temp){\n"
              "     if(temp==4){\n"
              "         return false;\n"
              "     }\n"
              "     else\n"
              "         return true;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfTwoFuncsReturningBoolError]\n", errout_str());
    }

    void checkComparisonOfFuncReturningBool6() {
        check("int compare1(int temp);\n"
              "namespace Foo {\n"
              "    bool compare1(int temp);\n"
              "}\n"
              "void f(){\n"
              "    int temp = 4;\n"
              "    if(compare1(temp) > compare2(temp)){\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("namespace Foo {\n"
              "    bool compare1(int temp);\n"
              "}\n"
              "int compare1(int temp);\n"
              "void f(){\n"
              "    int temp = 4;\n"
              "    if(compare1(temp) > compare2(temp)){\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int compare1(int temp);\n"
              "namespace Foo {\n"
              "    bool compare1(int temp);\n"
              "    void f(){\n"
              "        int temp = 4;\n"
              "        if(compare1(temp) > compare2(temp)){\n"
              "            printf(\"foo\");\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:20]: (style) Comparison of a function returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfFuncReturningBoolError]\n", errout_str());

        check("int compare1(int temp);\n"
              "namespace Foo {\n"
              "    bool compare1(int temp);\n"
              "    void f(){\n"
              "        int temp = 4;\n"
              "        if(::compare1(temp) > compare2(temp)){\n"
              "            printf(\"foo\");\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool compare1(int temp);\n"
              "void f(){\n"
              "    int temp = 4;\n"
              "    if(foo.compare1(temp) > compare2(temp)){\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkComparisonOfFuncReturningBool7() { // #7197
        check("struct C {\n"
              "    bool isEmpty();\n"
              "};\n"
              "void f() {\n"
              "    C c1, c2;\n"
              "    if ((c1.isEmpty()) < (c2.isEmpty())) {}\n"
              "    if (!c1.isEmpty() < !!c2.isEmpty()) {}\n"
              "    if ((int)c1.isEmpty() < (int)c2.isEmpty()) {}\n"
              "    if (static_cast<int>(c1.isEmpty()) < static_cast<int>(c2.isEmpty())) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:20]: (style) Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfTwoFuncsReturningBoolError]\n"
                      "[test.cpp:7:20]: (style) Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfTwoFuncsReturningBoolError]\n"
                      "[test.cpp:8:24]: (style) Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfTwoFuncsReturningBoolError]\n"
                      "[test.cpp:9:36]: (style) Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator. [comparisonOfTwoFuncsReturningBoolError]\n",
                      errout_str());
    }

    void checkComparisonOfFuncReturningBool8() { // #4103
        // op: >
        check("int main(void){\n"
              "    bool a = true;\n"
              "    bool b = false;\n"
              "    if(b > a){ \n"                             // here warning should be displayed
              "        ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Comparison of a variable having boolean value using relational (<, >, <= or >=) operator. [comparisonOfBoolWithBoolError]\n", errout_str());
        // op: <
        check("int main(void){\n"
              "    bool a = true;\n"
              "    bool b = false;\n"
              "    if(b < a){ \n"                             // here warning should be displayed
              "        ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Comparison of a variable having boolean value using relational (<, >, <= or >=) operator. [comparisonOfBoolWithBoolError]\n", errout_str());
        // op: >=
        check("int main(void){\n"
              "    bool a = true;\n"
              "    bool b = false;\n"
              "    if(b >= a){ \n"                             // here warning should be displayed
              "        ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Comparison of a variable having boolean value using relational (<, >, <= or >=) operator. [comparisonOfBoolWithBoolError]\n", errout_str());
        // op: <=
        check("int main(void){\n"
              "    bool a = true;\n"
              "    bool b = false;\n"
              "    if(b <= a){ \n"                             // here warning should be displayed
              "        ;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Comparison of a variable having boolean value using relational (<, >, <= or >=) operator. [comparisonOfBoolWithBoolError]\n", errout_str());
    }

    void checkComparisonOfFuncReturningBoolIntegrationTest1() { // #7798
        check("bool eval(double *) { return false; }\n"
              "double eval(char *) { return 1.0; }\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "  if ( eval(argv[1]) > eval(argv[2]) )\n"
              "    return 1;\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkComparisonOfBoolWithBool() {
        const char code[] = "void f(){\n"
                            "    int temp = 4;\n"
                            "    bool b = compare2(6);\n"
                            "    bool a = compare1(4);\n"
                            "    if(b > a){\n"
                            "        printf(\"foo\");\n"
                            "    }\n"
                            "}\n"
                            "bool compare1(int temp){\n"
                            "    if(temp==4){\n"
                            "        return true;\n"
                            "    }\n"
                            "    else\n"
                            "        return false;\n"
                            "}\n"
                            "bool compare2(int temp){\n"
                            "    if(temp == 5){\n"
                            "        return true;\n"
                            "    }\n"
                            "    else\n"
                            "        return false;\n"
                            "}\n";
        check(code);
        ASSERT_EQUALS("[test.cpp:5:10]: (style) Comparison of a variable having boolean value using relational (<, >, <= or >=) operator. [comparisonOfBoolWithBoolError]\n", errout_str());
    }

    void bitwiseOnBoolean() { // 3062
        check("void f(_Bool a, _Bool b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '&&'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(_Bool a, _Bool b) {\n"
              "    if(a | b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '||'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "    if(a & !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '&&'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(bool a, bool b) {\n"
              "    if(a | !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '||'? [bitwiseOnBoolean]\n", errout_str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '&&'? [bitwiseOnBoolean]\n", errout_str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a & !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '&&'? [bitwiseOnBoolean]\n", errout_str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a | b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '||'? [bitwiseOnBoolean]\n", errout_str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a | !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '||'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(bool a, int b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '&&'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(int a, bool b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style, inconclusive) Boolean expression 'b' is used in bitwise operation. Did you mean '&&'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(int a, int b) {\n"
              "    if((a > 0) & (b < 0)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (style, inconclusive) Boolean expression 'a>0' is used in bitwise operation. Did you mean '&&'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(bool a, int b) {\n"
              "    if(a | b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '||'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(int a, bool b) {\n"
              "    if(a | b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style, inconclusive) Boolean expression 'b' is used in bitwise operation. Did you mean '||'? [bitwiseOnBoolean]\n", errout_str());

        check("int f(bool a, int b) {\n"
              "    return a | b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(bool a, int b) {\n"
              "    return a | b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (style, inconclusive) Boolean expression 'a' is used in bitwise operation. Did you mean '||'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(int a, int b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n"
              "    foo(bar, &b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n" // #9405
              "    class C { void foo(bool &b) {} };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f();\n"
              "bool g() {\n"
              "  return f() | f();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("uint8 outcode(float p) {\n"
              "    float d = 0.;\n"
              "    return ((p - xm >= d) << 1) | (x - p > d);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int g();\n" // #10655
              "void f(bool b) {\n"
              "    if (g() | b) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:13]: (style, inconclusive) Boolean expression 'b' is used in bitwise operation. Did you mean '||'? [bitwiseOnBoolean]\n", errout_str());

        check("int g();\n"
              "void f(bool b) {\n"
              "    if (b | g()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int g();\n"
              "bool f(bool b, bool c) {\n"
              "    return b | g() | c;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:20]: (style, inconclusive) Boolean expression 'c' is used in bitwise operation. Did you mean '||'? [bitwiseOnBoolean]\n", errout_str());

        check("void f(int i) {\n" // #4233
              "    bool b = true, c = false;\n"
              "    b &= i;\n"
              "    c |= i;\n"
              "    if (b || c) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:7]: (style, inconclusive) Boolean expression 'b' is used in bitwise operation. [bitwiseOnBoolean]\n"
                      "[test.cpp:4:7]: (style, inconclusive) Boolean expression 'c' is used in bitwise operation. [bitwiseOnBoolean]\n",
                      errout_str());

        check("void f(int i, int j, bool b) {\n"
              "    i &= b;\n"
              "    j |= b;\n"
              "    if (b || c) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool f(bool b, int i) {\n"
              "    b &= (i == 5);\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { bool b{}; };\n" // #12455
              "void f(const std::unordered_map<int, S> m) {\n"
              "    for (const auto& e : m) {\n"
              "        S s;\n"
              "        s.b |= e.second.b;\n"
              "        (void)s.b;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void incrementBoolean() {
        check("bool bValue = true;\n"
              "void f() { bValue++; }");
        ASSERT_EQUALS("[test.cpp:2:12]: (style) Incrementing a variable of type 'bool' with postfix operator++ is deprecated by the C++ Standard. You should assign it the value 'true' instead. [incrementboolean]\n", errout_str());

        check("void f(bool test){\n"
              "    test++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Incrementing a variable of type 'bool' with postfix operator++ is deprecated by the C++ Standard. You should assign it the value 'true' instead. [incrementboolean]\n", errout_str());

        check("void f(bool* test){\n"
              "    (*test)++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:6]: (style) Incrementing a variable of type 'bool' with postfix operator++ is deprecated by the C++ Standard. You should assign it the value 'true' instead. [incrementboolean]\n", errout_str());

        check("void f(bool* test){\n"
              "    test[0]++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Incrementing a variable of type 'bool' with postfix operator++ is deprecated by the C++ Standard. You should assign it the value 'true' instead. [incrementboolean]\n", errout_str());

        check("void f(int test){\n"
              "    test++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void comparisonOfBoolWithInt1() {
        check("void f(bool x) {\n"
              "    if (x < 10) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(bool x) {\n"
              "    if (10 >= x) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(bool x) {\n"
              "    if (x != 0) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool x) {\n"  // #3356
              "    if (x == 1) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool x) {\n"
              "    if (x != 10) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(bool x) {\n"
              "    if (x == 10) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());

        check("void f(bool x) {\n"
              "    if (x == 0) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("DensePropertyMap<int, true> visited;"); // #4075
        ASSERT_EQUALS("", errout_str());
    }

    void comparisonOfBoolWithInt2() {
        check("void f(bool x, int y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, bool y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool x, bool y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool x, fooClass y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void comparisonOfBoolWithInt3() {
        check("void f(int y) {\n"
              "    if (y > false) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=). [comparisonOfBoolWithInvalidComparator]\n", errout_str());

        check("void f(int y) {\n"
              "    if (true == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool y) {\n"
              "    if (y == true) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool y) {\n"
              "    if (false < 5) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Comparison of a boolean expression with an integer other than 0 or 1. [compareBoolExpressionWithInt]\n", errout_str());
    }

    void comparisonOfBoolWithInt4() {
        check("void f(int x) {\n"
              "    if (!x == 1) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void comparisonOfBoolWithInt5() {
        check("void SetVisible(int index, bool visible) {\n"
              "    bool (SciTEBase::*ischarforsel)(char ch);\n"
              "    if (visible != GetVisible(index)) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void comparisonOfBoolWithInt6() { // #4224 - integer is casted to bool
        check("void SetVisible(bool b, int i) {\n"
              "    if (b == (bool)i) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void comparisonOfBoolWithInt7() { // #4846 - (!x==true)
        check("void f(int x) {\n"
              "    if (!x == true) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void comparisonOfBoolWithInt8() { // #9165
        check("bool Fun();\n"
              "void Test(bool expectedResult) {\n"
              "    auto res = Fun();\n"
              "    if (expectedResult == res)\n"
              "        throw 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int Fun();\n"
              "void Test(bool expectedResult) {\n"
              "    auto res = Fun();\n"
              "    if (expectedResult == res)\n"
              "        throw 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool Fun();\n"
              "void Test(bool expectedResult) {\n"
              "    auto res = Fun();\n"
              "    if (5 + expectedResult == res)\n"
              "        throw 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int Fun();\n"
              "void Test(bool expectedResult) {\n"
              "    auto res = Fun();\n"
              "    if (5 + expectedResult == res)\n"
              "        throw 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int Fun();\n"
              "void Test(bool expectedResult) {\n"
              "    auto res = Fun();\n"
              "    if (expectedResult == res + 5)\n"
              "        throw 2;\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

    void comparisonOfBoolWithInt9() { // #9304
        check("bool f(int a, bool b)\n"
              "{\n"
              "    if ((a == 0 ? false : true) != b) {\n"
              "        b = !b;\n"
              "    }\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void comparisonOfBoolWithInt10() { // #10935
        check("enum class E { H = 2 };\n"
              "template <bool H>\n"
              "void f(bool v) {\n"
              "    if (v == H) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("namespace N {\n"
              "    enum class E { H = 2 };\n"
              "}\n"
              "void f(bool v) {\n"
              "    if (v == N::H) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }


    void pointerArithBool1() { // #5126
        check("void f(char *p) {\n"
              "    if (p+1){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Converting pointer arithmetic result to bool. The bool is always true unless there is undefined behaviour. [pointerArithBool]\n", errout_str());

        check("void f(char *p) {\n"
              "    do {} while (p+1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:19]: (error) Converting pointer arithmetic result to bool. The bool is always true unless there is undefined behaviour. [pointerArithBool]\n", errout_str());

        check("void f(char *p) {\n"
              "    while (p-1) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (error) Converting pointer arithmetic result to bool. The bool is always true unless there is undefined behaviour. [pointerArithBool]\n", errout_str());

        check("void f(char *p) {\n"
              "    for (int i = 0; p+1; i++) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (error) Converting pointer arithmetic result to bool. The bool is always true unless there is undefined behaviour. [pointerArithBool]\n", errout_str());

        check("void f(char *p) {\n"
              "    if (p && p+1){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (error) Converting pointer arithmetic result to bool. The bool is always true unless there is undefined behaviour. [pointerArithBool]\n", errout_str());

        check("void f(char *p) {\n"
              "    if (p+2 || p) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Converting pointer arithmetic result to bool. The bool is always true unless there is undefined behaviour. [pointerArithBool]\n", errout_str());
    }

    void returnNonBool() {
        check("bool f(void) {\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(void) {\n"
              "    return 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(void) {\n"
              "    return 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());

        check("bool f(void) {\n"
              "    return -1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());

        check("bool f(void) {\n"
              "    return 1 + 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());

        check("bool f(void) {\n"
              "    int x = 0;\n"
              "    return x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(void) {\n"
              "    int x = 10;\n"
              "    return x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());

        check("bool f(void) {\n"
              "    return 2 < 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(void) {\n"
              "    int ret = 0;\n"
              "    if (a)\n"
              "        ret = 1;\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(void) {\n"
              "    int ret = 0;\n"
              "    if (a)\n"
              "        ret = 3;\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());

        check("bool f(void) {\n"
              "    if (a)\n"
              "        return 3;\n"
              "    return 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n"
                      "[test.cpp:4:5]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());

        check("bool f(void) {\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnNonBoolLambda() {
        check("bool f(void) {\n"
              "    auto x = [](void) { return -1; };\n"
              "    return false;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(void) {\n"
              "    auto x = [](void) { return -1; };\n"
              "    return 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());

        check("bool f(void) {\n"
              "    auto x = [](void) -> int { return -1; };\n"
              "    return false;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(void) {\n"
              "    auto x = [](void) -> int { return -1; };\n"
              "    return 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());
    }

    void returnNonBoolLogicalOp() {
        check("bool f(int x) {\n"
              "    return x & 0x4;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int x, int y) {\n"
              "    return x | y;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int x) {\n"
              "    return (x & 0x2);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnNonBoolClass() {
        check("class X {\n"
              "    public:\n"
              "        bool f() { return -1;}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:20]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());

        check("bool f() {\n"
              "    struct X {\n"
              "        public:\n"
              "            int f() { return -1;}\n"
              "    };\n"
              "    return false;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f() {\n"
              "    class X {\n"
              "        public:\n"
              "            int f() { return -1;}\n"
              "    };\n"
              "    return false;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f() {\n"
              "    class X {\n"
              "        public:\n"
              "            bool f() { return -1;}\n"
              "    };\n"
              "    return -1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n"
                      "[test.cpp:4:24]: (style) Non-boolean value returned from function returning bool [returnNonBoolInBooleanFunction]\n", errout_str());
    }
};

REGISTER_TEST(TestBool)
