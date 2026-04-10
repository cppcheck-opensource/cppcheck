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

#include "checksizeof.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>

class TestSizeof : public TestFixture {
public:
    TestSizeof() : TestFixture("TestSizeof") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::warning).severity(Severity::portability).certainty(Certainty::inconclusive).library("std.cfg").build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(sizeofsizeof);
        TEST_CASE(sizeofCalculation);
        TEST_CASE(sizeofFunction);
        TEST_CASE(checkPointerSizeof);
        TEST_CASE(checkPointerSizeofStruct);
        TEST_CASE(sizeofDivisionMemset);
        TEST_CASE(sizeofForArrayParameter);
        TEST_CASE(sizeofForNumericParameter);
        TEST_CASE(suspiciousSizeofCalculation);
        TEST_CASE(sizeofVoid);
        TEST_CASE(customStrncat);
    }

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check...
        runChecks<CheckSizeof>(tokenizer, this);
    }

#define checkP(...) checkP_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkP_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer2 tokenizer(settings, *this, code, "test.cpp");

        // Tokenize..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check...
        runChecks<CheckSizeof>(tokenizer, this);
    }

    void sizeofsizeof() {
        check("void foo()\n"
              "{\n"
              "    int i = sizeof sizeof char;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (warning) Calling 'sizeof' on 'sizeof'. [sizeofsizeof]\n", errout_str());

        check("void foo()\n"
              "{\n"
              "    int i = sizeof (sizeof long);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (warning) Calling 'sizeof' on 'sizeof'. [sizeofsizeof]\n", errout_str());

        check("void foo(long *p)\n"
              "{\n"
              "    int i = sizeof (sizeof (p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (warning) Calling 'sizeof' on 'sizeof'. [sizeofsizeof]\n", errout_str());
    }

    void sizeofCalculation() {
        check("int a, b; int a,sizeof(a+b)");
        ASSERT_EQUALS("[test.cpp:1:25]: (warning) Found calculation inside sizeof(). [sizeofCalculation]\n", errout_str());

        check("int a, b; sizeof(a*b)");
        ASSERT_EQUALS("[test.cpp:1:19]: (warning) Found calculation inside sizeof(). [sizeofCalculation]\n", errout_str());

        check("int a, b; sizeof(-a)");
        ASSERT_EQUALS("[test.cpp:1:18]: (warning) Found calculation inside sizeof(). [sizeofCalculation]\n", errout_str());

        check("int a, b; sizeof(*a)");
        ASSERT_EQUALS("", errout_str());

        check("sizeof(void * const)");
        ASSERT_EQUALS("", errout_str());

        check("sizeof(int*[2])");
        ASSERT_EQUALS("", errout_str());

        check("sizeof(Fred**)");
        ASSERT_EQUALS("", errout_str());

        check("sizeof(foo++)");
        ASSERT_EQUALS("[test.cpp:1:11]: (warning) Found calculation inside sizeof(). [sizeofCalculation]\n", errout_str());

        check("sizeof(--foo)");
        ASSERT_EQUALS("[test.cpp:1:8]: (warning) Found calculation inside sizeof(). [sizeofCalculation]\n", errout_str());

        // #6888
        checkP("#define SIZEOF1   sizeof(i != 2)\n"
               "#define SIZEOF2   ((sizeof(i != 2)))\n"
               "#define VOIDCAST1 (void)\n"
               "#define VOIDCAST2(SZ) static_cast<void>(SZ)\n"
               "int f(int i) {\n"
               "  VOIDCAST1 SIZEOF1;\n"
               "  VOIDCAST1 SIZEOF2;\n"
               "  VOIDCAST2(SIZEOF1);\n"
               "  VOIDCAST2(SIZEOF2);\n"
               "  return i + foo(1);\n"
               "}");
        ASSERT_EQUALS("", errout_str());

        checkP("#define SIZEOF1   sizeof(i != 2)\n"
               "#define SIZEOF2   ((sizeof(i != 2)))\n"
               "int f(int i) {\n"
               "  SIZEOF1;\n"
               "  SIZEOF2;\n"
               "  return i + foo(1);\n"
               "}");
        ASSERT_EQUALS("[test.cpp:4:3]: (warning, inconclusive) Found calculation inside sizeof(). [sizeofCalculation]\n"
                      "[test.cpp:5:3]: (warning, inconclusive) Found calculation inside sizeof(). [sizeofCalculation]\n", errout_str());

        checkP("#define MACRO(data)  f(data, sizeof(data))\n"
               "x = MACRO((unsigned int *)data + 4);");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Found calculation inside sizeof(). [sizeofCalculation]\n", errout_str());
    }

    void sizeofFunction() {
        check("class Foo\n"
              "{\n"
              "    int bar() { return 1; };\n"
              "}\n"
              "Foo f;int a=sizeof(f.bar());");
        ASSERT_EQUALS("[test.cpp:5:13]: (warning) Found function call inside sizeof(). [sizeofFunctionCall]\n", errout_str());

        check("class Foo\n"
              "{\n"
              "    int bar() { return 1; };\n"
              "    int bar() const { return 1; };\n"
              "}\n"
              "Foo f;int a=sizeof(f.bar());");
        ASSERT_EQUALS("", errout_str());

        check("class Foo\n"
              "{\n"
              "    int bar() { return 1; };\n"
              "}\n"
              "Foo * fp;int a=sizeof(fp->bar());");
        ASSERT_EQUALS("[test.cpp:5:16]: (warning) Found function call inside sizeof(). [sizeofFunctionCall]\n", errout_str());

        check("int a=sizeof(foo());");
        ASSERT_EQUALS("", errout_str());

        check("int foo() { return 1; }; int a=sizeof(foo());");
        ASSERT_EQUALS("[test.cpp:1:32]: (warning) Found function call inside sizeof(). [sizeofFunctionCall]\n", errout_str());

        check("int foo() { return 1; }; sizeof(decltype(foo()));");
        ASSERT_EQUALS("", errout_str());

        check("int foo(int) { return 1; }; int a=sizeof(foo(0))");
        ASSERT_EQUALS("[test.cpp:1:35]: (warning) Found function call inside sizeof(). [sizeofFunctionCall]\n", errout_str());

        check("char * buf; int a=sizeof(*buf);");
        ASSERT_EQUALS("", errout_str());

        check("int a=sizeof(foo())");
        ASSERT_EQUALS("", errout_str());

        check("int foo(int) { return 1; }; char buf[1024]; int a=sizeof(buf), foo(0)");
        ASSERT_EQUALS("", errout_str());

        check("template<class T>\n"
              "struct A\n"
              "{\n"
              "    static B f(const B &);\n"
              "    static A f(const A &);\n"
              "    static A &g();\n"
              "    static T &h();\n"
              "\n"
              "    enum {\n"
              "        X = sizeof(f(g() >> h())) == sizeof(A),\n"
              "        Y = sizeof(f(g() << h())) == sizeof(A),\n"
              "        Z = X & Y\n"
              "    };\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("int g();\n"
              "size_t f1() { return sizeof(x[0]); }\n"
              "size_t f2() { return sizeof(g()); }\n");
        ASSERT_EQUALS("[test.cpp:3:22]: (warning) Found function call inside sizeof(). [sizeofFunctionCall]\n", errout_str());
    }

    void sizeofForArrayParameter() {
        check("void f() {\n"
              "    int a[10];\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    unsigned int a = 2;\n"
              "    unsigned int b = 2;\n"
              "    int c[(a+b)];\n"
              "    std::cout << sizeof(c) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    unsigned int a = { 2 };\n"
              "    unsigned int b[] = { 0 };\n"
              "    int c[a[b[0]]];\n"
              "    std::cout << sizeof(c) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        check("void f() {\n"
              "    unsigned int a[] = { 1 };\n"
              "    unsigned int b = 2;\n"
              "    int c[(a[0]+b)];\n"
              "    std::cout << sizeof(c) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a[] = { 1, 2, 3 };\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a[3] = { 1, 2, 3 };\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f( int a[]) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer. [sizeofwithsilentarraypointer]\n", errout_str());

        check("void f( int a[]) {\n"
              "    std::cout << sizeof a / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer. [sizeofwithsilentarraypointer]\n", errout_str());

        check("void f( int a[3] ) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer. [sizeofwithsilentarraypointer]\n", errout_str());

        check("typedef char Fixname[1000];\n"
              "int f2(Fixname& f2v) {\n"
              "  int i = sizeof(f2v);\n"
              "  printf(\"sizeof f2v %d\", i);\n"
              "   }");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p) {\n"
              "    p[0] = 0;\n"
              "    int unused = sizeof(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char p[] = \"test\";\n"
              "    int unused = sizeof(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #2495
        check("void f() {\n"
              "    static float col[][3]={\n"
              "      {1,0,0},\n"
              "      {0,0,1},\n"
              "      {0,1,0},\n"
              "      {1,0,1},\n"
              "      {1,0,1},\n"
              "      {1,0,1},\n"
              "    };\n"
              "    const int COL_MAX=sizeof(col)/sizeof(col[0]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #155
        check("void f() {\n"
              "    char buff1[1024*64],buff2[sizeof(buff1)*2];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #2510
        check("void f( int a[], int b) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer. [sizeofwithsilentarraypointer]\n", errout_str());

        // ticket #2510
        check("void f( int a[3] , int b[2] ) {\n"
              "    std::cout << sizeof(a) / sizeof(int) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Using 'sizeof' on array given as "
                      "function argument returns size of a pointer. [sizeofwithsilentarraypointer]\n", errout_str());

        // ticket #2510
        check("void f() {\n"
              "    char buff1[1024*64],buff2[sizeof(buff1)*(2+1)];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

    }

    void sizeofForNumericParameter() {
        check("void f() {\n"
              "    std::cout << sizeof(10) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Suspicious usage of 'sizeof' with a numeric constant as parameter. [sizeofwithnumericparameter]\n", errout_str());

        check("void f() {\n"
              "    std::cout << sizeof(-10) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Suspicious usage of 'sizeof' with a numeric constant as parameter. [sizeofwithnumericparameter]\n", errout_str());

        check("void f() {\n"
              "    std::cout << sizeof 10  << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Suspicious usage of 'sizeof' with a numeric constant as parameter. [sizeofwithnumericparameter]\n", errout_str());

        check("void f() {\n"
              "    std::cout << sizeof -10  << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Suspicious usage of 'sizeof' with a numeric constant as parameter. [sizeofwithnumericparameter]\n", errout_str());
    }

    void suspiciousSizeofCalculation() {
        check("void f() {\n"
              "  int* p;\n"
              "  return sizeof(p)/5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (warning, inconclusive) Division of result of sizeof() on pointer type. [divideSizeof]\n", errout_str());

        check("void f() {\n"
              "  unknown p;\n"
              "  return sizeof(p)/5;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  return sizeof(unknown)/5;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int p;\n"
              "  return sizeof(p)/5;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int* p[5];\n"
              "  return sizeof(p)/5;\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        check("void f() {\n"
              "  return sizeof(foo)*sizeof(bar);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning, inconclusive) Multiplying sizeof() with sizeof() indicates a logic error. [multiplySizeof]\n", errout_str());

        check("void f() {\n"
              "  return (foo)*sizeof(bar);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  return sizeof(foo)*bar;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  return (end - source) / sizeof(encode_block_type) * sizeof(encode_block_type);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S { T* t; };\n" // #10179
              "int f(S* s) {\n"
              "    return g(sizeof(*s->t) / 4);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    const char* a[N];\n"
              "    for (int i = 0; i < (int)(sizeof(a) / sizeof(char*)); i++) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int** p) {\n"
              "    return sizeof(p[0]) / 4;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning, inconclusive) Division of result of sizeof() on pointer type. [divideSizeof]\n", errout_str());

        check("struct S {\n"
              "    unsigned char* s;\n"
              "};\n"
              "struct T {\n"
              "    S s[38];\n"
              "};\n"
              "void f(T* t) {\n"
              "    for (size_t i = 0; i < sizeof(t->s) / sizeof(t->s[0]); i++) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    struct T {\n"
              "        char* c[3];\n"
              "    } t[1];\n"
              "};\n"
              "void f(S* s) {\n"
              "    for (int i = 0; i != sizeof(s->t[0].c) / sizeof(char*); i++) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int32_t* buf, size_t len) {\n"
              "    for (int i = 0; i < len / sizeof(buf[0]); i++) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int32_t*** buf, size_t len) {\n"
              "    for (int i = 0; i < len / sizeof(**buf[0]); i++) {}\n"
              "    for (int i = 0; i < len / sizeof(*buf[0][0]); i++) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void checkPointerSizeof() {
        check("void f() {\n"
              "    char *x = malloc(10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(*x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = (int*)malloc(sizeof(x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = static_cast<int*>(malloc(sizeof(x)));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(&x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int*));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    free(x);\n"
              "    int **y = malloc(sizeof(int*));\n"
              "    free(y);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(100 * sizeof(x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(x) * 100);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof *x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(100 * sizeof x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof(*x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof *x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof(x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = calloc(1, sizeof(int));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char x[10];\n"
              "    memset(x, 0, sizeof(x));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char* x[10];\n"
              "    memset(x, 0, sizeof(x));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char x[10];\n"
              "    memset(x, 0, sizeof x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof(int));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof(*x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof *x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof x);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int));\n"
              "    memset(x, 0, sizeof(x));\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof(x) * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof x * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (warning) Size of pointer 'x' used instead of size of its data. [pointerSize]\n", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof(*x) * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof *x * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int *x = malloc(sizeof(int) * 10);\n"
              "    memset(x, 0, sizeof(int) * 10);\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "int fun(const char *buf1)\n"
            "{\n"
            "  const char *buf1_ex = \"foobarbaz\";\n"
            "  return strncmp(buf1, buf1_ex, sizeof(buf1_ex)) == 0;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:4:24]: (warning) Size of pointer 'buf1_ex' used instead of size of its data. [pointerSize]\n", errout_str());

        check(
            "int fun(const char *buf1) {\n"
            "  return strncmp(buf1, foo(buf2), sizeof(buf1)) == 0;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (warning) Size of pointer 'buf1' used instead of size of its data. [pointerSize]\n", errout_str());

        check("int fun(const char *buf2) {\n"
              "  return strncmp(buf1, buf2, sizeof(char*)) == 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:24]: (warning) Size of pointer 'buf2' used instead of size of its data. [pointerSize]\n", errout_str());

        // #ticket 3874
        check("void f()\n"
              "{\n"
              " int * pIntArray[10];\n"
              " memset(pIntArray, 0, sizeof(pIntArray));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void FreeFileName(const char *s) {\n"
              "  CxString tbuf;\n"
              "  const char *p;\n"
              "  memcpy(s, siezof(s));\n" // non-standard memcpy
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "  module_config_t *tab = module;\n"
              "  memset(tab + confsize, 0, sizeof(tab[confsize]));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(char* aug) {\n"
              "  memmove(aug + extra_string, aug, buf - (bfd_byte *)aug);\n" // #7100
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7518
        check("bool create_iso_definition(cpp_reader *pfile, cpp_macro *macro) {\n"
              "  cpp_token *token;\n"
              "  cpp_hashnode **params = malloc(sizeof(cpp_hashnode *) * macro->paramc);\n"
              "  memcpy(params, macro->params, sizeof(cpp_hashnode *) * macro->paramc);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void* foo() {\n"
              "  void* AtomName = malloc(sizeof(char *) * 34);\n"
              "  return AtomName;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkPointerSizeofStruct() {
        check("void f() {\n"
              "    struct foo *ptr;\n"
              "    memset( ptr->bar, 0, sizeof ptr->bar );\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    struct foo {\n"
              "        char bar[10];\n"
              "    }* ptr;\n"
              "    memset( ptr->bar, 0, sizeof ptr->bar );\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    struct foo {\n"
              "        char *bar;\n"
              "    }* ptr;\n"
              "    memset( ptr->bar, 0, sizeof ptr->bar );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:18]: (warning) Size of pointer 'bar' used instead of size of its data. [pointerSize]\n", errout_str());
    }

    void sizeofDivisionMemset() {
        check("void foo(memoryMapEntry_t* entry, memoryMapEntry_t* memoryMapEnd) {\n"
              "    memmove(entry, entry + 1, (memoryMapEnd - entry) / sizeof(entry));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:56]: (warning, inconclusive) Division of result of sizeof() on pointer type. [divideSizeof]\n"
                      "[test.cpp:2:54]: (warning) Division by result of sizeof(). memmove() expects a size in bytes, did you intend to multiply instead? [sizeofDivisionMemfunc]\n",
                      errout_str());

        check("Foo* allocFoo(int num) {\n"
              "    return malloc(num / sizeof(Foo));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:23]: (warning) Division by result of sizeof(). malloc() expects a size in bytes, did you intend to multiply instead? [sizeofDivisionMemfunc]\n", errout_str());

        check("void f() {\n"
              "  char str[100];\n"
              "  strncpy(str, xyz, sizeof(str)/sizeof(str[0]));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #9648
              "    int a[5] = { 0 };\n"
              "    int b[5];\n"
              "    memcpy(b, a, ((sizeof(a) / sizeof(a[0])) - 1) * sizeof(a[0]));\n"
              "    memcpy(b, a, sizeof(a[0]) * ((sizeof(a) / sizeof(a[0])) - 1));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void sizeofVoid() {
        check("void f() {\n"
              "  int size = sizeof(void);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (portability) Behaviour of 'sizeof(void)' is not covered by the ISO C standard. [sizeofVoid]\n", errout_str());

        check("void f() {\n"
              "  void* p;\n"
              "  int size = sizeof(*p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (portability) '*p' is of type 'void', the behaviour of 'sizeof(void)' is not covered by the ISO C standard. [sizeofDereferencedVoidPointer]\n", errout_str());

        check("void f() {\n"
              "  void* p = malloc(10);\n"
              "  int* p2 = p + 4;\n"
              "  int* p3 = p - 1;\n"
              "  int* p4 = 1 + p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (portability) 'p' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n"
                      "[test.cpp:4:15]: (portability) 'p' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n"
                      "[test.cpp:5:15]: (portability) 'p' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n", errout_str());

        check("void f() {\n"
              "  void* p1 = malloc(10);\n"
              "  void* p2 = malloc(5);\n"
              "  p1--;\n"
              "  p2++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (portability) 'p1' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n"
                      "[test.cpp:5:5]: (portability) 'p2' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n", errout_str());

        check("void f() {\n"
              "  void* p1 = malloc(10);\n"
              "  void* p2 = malloc(5);\n"
              "  p1-=4;\n"
              "  p2+=4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (portability) 'p1' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n"
                      "[test.cpp:5:5]: (portability) 'p2' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n", errout_str());

        check("void f() {\n"
              "  void* p = malloc(10);\n"
              "  int* p2 = &p + 4;\n"
              "  int* p3 = &p - 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  void** p1 = malloc(10);\n"
              "  p1--;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  void** p1;\n"
              "  int j = sizeof(*p1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  void* p1[5];\n"
              "  int j = sizeof(*p1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Calculations on void* with casts

        check("void f(void *data) {\n"
              "  *((unsigned char *)data + 1) = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(void *data) {\n"
              "  *((unsigned char *)(data) + 1) = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(void *data) {\n"
              "  unsigned char* c = (unsigned char *)(data + 1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:45]: (portability) 'data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n", errout_str());

        check("void f(void *data) {\n"
              "  unsigned char* c = (unsigned char *)data++;\n"
              "  unsigned char* c2 = (unsigned char *)++data;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:43]: (portability) 'data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n"
                      "[test.cpp:3:40]: (portability) 'data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n", errout_str());

        check("void f(void *data) {\n"
              "  void* data2 = data + 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (portability) 'data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n", errout_str());

        // #4908 (void pointer as a member of a struct/class)
        check("struct FOO {\n"
              "  void *data;\n"
              "};\n"
              "char f(struct FOO foo) {\n"
              "  char x = *((char*)(foo.data+1));\n"
              "  foo.data++;\n"
              "  return x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:30]: (portability) 'foo.data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n"
                      "[test.cpp:6:11]: (portability) 'foo.data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n", errout_str());

        check("struct FOO {\n"
              "  void *data;\n"
              "};\n"
              "char f(struct FOO foo) {\n"
              "  char x = *((char*)foo.data+1);\n"
              "  return x;\n"
              "}\n"
              "char f2(struct FOO foo) {\n"
              "  char x = *((char*)((FOO)foo).data + 1);\n"
              "  return x;\n"
              "}\n"
              "char f3(struct FOO* foo) {\n"
              "  char x = *((char*)foo->data + 1);\n"
              "  return x;\n"
              "}\n"
              "struct BOO {\n"
              "  FOO data;\n"
              "};\n"
              "void f4(struct BOO* boo) {\n"
              "  char c = *((char*)boo->data.data + 1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct FOO {\n"
              "  void *data;\n"
              "};\n"
              "char f(struct FOO* foo) {\n"
              "  *(foo[1].data + 1) = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:17]: (portability) 'foo[1].data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n", errout_str());

        check("struct FOO {\n"
              "  void *data;\n"
              "};\n"
              "void f2(struct FOO* foo) {\n"
              "  (foo[0]).data++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:16]: (portability) '(foo[0]).data' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined. [arithOperationsOnVoidPointer]\n", errout_str());

        // #6050 arithmetic on void**
        check("void* array[10];\n"
              "void** b = array + 3;");
        ASSERT_EQUALS("", errout_str());
    }

    void customStrncat() {
        // Ensure we don't crash on custom-defined strncat, ticket #5875
        check("char strncat ();\n"
              "int main () {\n"
              "  return strncat ();\n"
              "}");
    }

};

REGISTER_TEST(TestSizeof)

