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

#include "check.h"
#include "checkuninitvar.h"
#include "ctu.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>
#include <list>
#include <string>

class TestUninitVar : public TestFixture {
public:
    TestUninitVar() : TestFixture("TestUninitVar") {}

private:
    const Settings settings = settingsBuilder().library("std.cfg").build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(uninitvar1);
        TEST_CASE(uninitvar_warn_once); // only write 1 warning at a time
        TEST_CASE(uninitvar_decl);      // handling various types in C and C++ files
        TEST_CASE(uninitvar_bitop);     // using uninitialized operand in bit operation
        TEST_CASE(uninitvar_alloc);     // data is allocated but not initialized
        TEST_CASE(uninitvar_arrays);    // arrays
        TEST_CASE(uninitvar_class);     // class/struct
        TEST_CASE(uninitvar_enum);      // enum variables
        TEST_CASE(uninitvar_if);        // handling if
        TEST_CASE(uninitvar_loops);     // handling for/while
        TEST_CASE(uninitvar_switch);    // handling switch
        TEST_CASE(uninitvar_references); // references
        TEST_CASE(uninitvar_return);    // return
        TEST_CASE(uninitvar_assign);    // = {..}
        TEST_CASE(uninitvar_strncpy);   // strncpy doesn't always null-terminate
        TEST_CASE(func_uninit_var);     // analyse function calls for: 'int a(int x) { return x+x; }'
        TEST_CASE(func_uninit_pointer); // analyse function calls for: 'void a(int *p) { *p = 0; }'
        TEST_CASE(uninitvar_typeof);    // typeof
        TEST_CASE(uninitvar_ignore);    // ignore cast, *&x, ..
        TEST_CASE(uninitvar2);
        TEST_CASE(uninitvar3);          // #3844
        TEST_CASE(uninitvar4);          // #3869 (reference)
        TEST_CASE(uninitvar5);          // #3861
        TEST_CASE(uninitvar6);          // #13227
        TEST_CASE(uninitvar2_func);     // function calls
        TEST_CASE(uninitvar2_value);    // value flow
        TEST_CASE(valueFlowUninit2_value);
        TEST_CASE(valueFlowUninit_uninitvar2);
        TEST_CASE(valueFlowUninit_functioncall);
        TEST_CASE(uninitStructMember);  // struct members
        TEST_CASE(uninitvar2_while);
        TEST_CASE(uninitvar2_4494);      // #4494
        TEST_CASE(uninitvar2_malloc);    // malloc returns uninitialized data
        TEST_CASE(uninitvar8); // ticket #6230
        TEST_CASE(uninitvar9); // ticket #6424
        TEST_CASE(uninitvar10); // ticket #9467
        TEST_CASE(uninitvar11); // ticket #9123
        TEST_CASE(uninitvar12); // #10218 - stream read
        TEST_CASE(uninitvar13); // #9772
        TEST_CASE(uninitvar14);
        TEST_CASE(uninitvar15); // #13685
        TEST_CASE(uninitvar_unconditionalTry);
        TEST_CASE(uninitvar_funcptr); // #6404
        TEST_CASE(uninitvar_operator); // #6680
        TEST_CASE(uninitvar_ternaryexpression); // #4683
        TEST_CASE(uninitvar_pointertoarray);
        TEST_CASE(uninitvar_cpp11ArrayInit); // #7010
        TEST_CASE(uninitvar_rangeBasedFor); // #7078
        TEST_CASE(uninitvar_static); // #8734
        TEST_CASE(uninitvar_configuration);
        TEST_CASE(checkExpr);
        TEST_CASE(trac_4871);
        TEST_CASE(syntax_error); // Ticket #5073
        TEST_CASE(trac_5970);
        TEST_CASE(valueFlowUninitTest);
        TEST_CASE(valueFlowUninitBreak);
        TEST_CASE(valueFlowUninitStructMembers);
        TEST_CASE(valueFlowUninitForLoop);
        TEST_CASE(uninitvar_ipa);
        TEST_CASE(uninitvar_memberfunction);
        TEST_CASE(uninitvar_nonmember); // crash in ycmd test
        TEST_CASE(uninitvarDesignatedInitializers);
        TEST_CASE(uninitvarMemberPointer);

        TEST_CASE(isVariableUsageDeref); // *p
        TEST_CASE(isVariableUsageDerefValueflow); // *p

        TEST_CASE(uninitvar_memberaccess); // (&(a))->b <=> a.b

        // whole program analysis
        TEST_CASE(ctuTest);
    }

    struct CheckUninitVarOptions
    {
        bool cpp = true;
    };

#define checkUninitVar(...) checkUninitVar_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkUninitVar_(const char* file, int line, const char (&code)[size], const CheckUninitVarOptions& options = make_default_obj()) {
        checkUninitVar_(file, line, code, settings, options.cpp);
    }

    template<size_t size>
    void checkUninitVar_(const char* file, int line, const char (&code)[size], const Settings& settings1, bool cpp = true) {
        SimpleTokenizer tokenizer(settings1, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for redundant code..
        CheckUninitVar checkuninitvar(&tokenizer, &settings1, this);
        checkuninitvar.check();
    }

    // TODO: get rid of this
    void checkUninitVar_(const char* file, int line, const std::string& code) {
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for redundant code..
        CheckUninitVar checkuninitvar(&tokenizer, &settings, this);
        checkuninitvar.check();
    }

    void uninitvar1() {
        // extracttests.start: int b; int c;

        // Ticket #2207 - False negative
        checkUninitVar("void foo() {\n"
                       "    int a;\n"
                       "    b = c - a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo() {\n"
                       "    int a;\n"
                       "    b = a - c;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        // Ticket #6455 - some compilers allow const variables to be uninitialized
        // extracttests.disable
        checkUninitVar("void foo() {\n"
                       "    const int a;\n"
                       "    b = c - a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());
        // extracttests.enable

        checkUninitVar("void foo() {\n"
                       "    int *p;\n"
                       "    realloc(p,10);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo() {\n" // #5240
                       "    char *p = malloc(100);\n"
                       "    char *tmp = realloc(p,1000);\n"
                       "    if (!tmp) free(p);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo() {\n"
                       "    int *p = NULL;\n"
                       "    realloc(p,10);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // dereferencing uninitialized pointer..
        // extracttests.start: struct Foo { void abcd(); };
        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    p->abcd();\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        // extracttests.start: template<class T> struct Foo { void abcd(); };
        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo<int> *p;\n"
                       "    p->abcd();\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        // extracttests.start: struct Foo { void* a; };
        checkUninitVar("void f(Foo *p)\n"
                       "{\n"
                       "    int a;\n"
                       "    p->a = malloc(4 * a);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:23]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *p;\n"
                       "    delete p;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *p;\n"
                       "    delete [] p;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:15]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *p;\n"
                       "    *p = 135;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *p;\n"
                       "    p[0] = 135;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *x;\n"
                       "    int y = *x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:14]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int *x;\n"
                       "    int &y(*x);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    int x;\n"
                       "    int *y = &x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    int *x;\n"
                       "    int *&y = x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    int x = xyz::x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a;\n"
                       "    a = 5 + a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    extern int a;\n"
                       "    a++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: void bar(int);
        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a;\n"
                       "    bar(4 * a);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    if (i);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    for (int x = 0; i < 10; x++);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:21]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    for (int x = 0; x < 10; i++);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:29]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo(int x)\n"
                       "{\n"
                       "    int i;\n"
                       "    if (x)\n"
                       "        i = 0;\n"
                       "    i++;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int ar[10];\n"
                       "    int i;\n"
                       "    ar[i] = 0;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:8]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    int x, y;\n"
                       "    x = (y = 10);\n"
                       "    int z = y * 2;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("static void foo() {\n"
                       "    int x, y;\n"
                       "    x = ((y) = 10);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #3597
        checkUninitVar("void f() {\n"
                       "    int a;\n"
                       "    int b = 1;\n"
                       "    (b += a) = 1;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("int f() {\n"
                       "    int a,b,c;\n"
                       "    a = b = c;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:0]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo p;\n"
                       "    p.abcd();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("static void foo()\n"
                       "{\n"
                       "    Foo p;\n"
                       "    int x = p.abcd();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Unknown types
        // extracttests.disable
        {
            checkUninitVar("void a()\n"
                           "{\n"
                           "    A ret;\n"
                           "    return ret;\n"
                           "}");
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("void a()\n"
                           "{\n"
                           "    A ret;\n"
                           "    return ret;\n"
                           "}\n",
                           dinit(CheckUninitVarOptions, $.cpp = false));
            ASSERT_EQUALS("[test.c:4:12]: (error) Uninitialized variable: ret [legacyUninitvar]\n", errout_str());
        }
        // extracttests.enable

        // #3916 - avoid false positive
        checkUninitVar("void f(float x) {\n"
                       "  union lf { long l; float f; } u_lf;\n"
                       "  float hx = (u_lf.f = (x), u_lf.l);\n"
                       "}",
                       dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    int x[10];\n"
                       "    int *y = x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    int x;\n"
                       "    int *y = &x;\n"
                       "    *y = 0;\n"
                       "    x++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    char x[10], y[10];\n"
                       "    char *z = x;\n"
                       "    memset(z, 0, sizeof(x));\n"
                       "    memcpy(y, x, sizeof(x));\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Handling >> and <<
        {
            checkUninitVar("int a() {\n"
                           "    int ret;\n"
                           "    std::cin >> ret;\n"
                           "    ret++;\n"
                           "}");
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("void f(int b) {\n"
                           "    int a;\n"
                           "    std::cin >> b >> a;\n"
                           "    return a;"
                           "}");
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("void f() {\n"
                           "    int ret[2];\n"
                           "    std::cin >> ret[0];\n"
                           "}");
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("void f(int i) {\n"
                           "    int a;\n"
                           "    i >> a;\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:3:10]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

            checkUninitVar("int a() {\n"
                           "    int ret;\n"
                           "    int a = value >> ret;\n"
                           "}\n",
                           dinit(CheckUninitVarOptions, $.cpp = false));
            ASSERT_EQUALS("[test.c:3:22]: (error) Uninitialized variable: ret [legacyUninitvar]\n", errout_str());

            checkUninitVar("void foo() {\n"   // #3707
                           "    Node node;\n"
                           "    int x;\n"
                           "    node[\"abcd\"] >> x;\n"
                           "}");
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("int a(FArchive &arc) {\n"   // #3060 (initialization through operator<<)
                           "    int *p;\n"
                           "    arc << p;\n" // <- TODO initialization?
                           "    return *p;\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

            checkUninitVar("void a() {\n"
                           "    int ret;\n"
                           "    a = value << ret;\n"
                           "}\n",
                           dinit(CheckUninitVarOptions, $.cpp = false));
            ASSERT_EQUALS("[test.c:3:18]: (error) Uninitialized variable: ret [legacyUninitvar]\n", errout_str());

            // #4320 says this is a FP. << is overloaded.
            checkUninitVar("int f() {\n"
                           "    int a;\n"
                           "    a << 1;\n"  // <- TODO initialization?
                           "    return a;\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

            // #4673
            checkUninitVar("void f() {\n"
                           "    int a;\n"
                           "    std::cout << a;\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:3:18]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

            checkUninitVar("void f(std::ostringstream& os) {\n"
                           "    int a;\n"
                           "    os << a;\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:3:11]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

            checkUninitVar("void f() {\n"
                           "    int a;\n"
                           "    std::cout << 1 << a;\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:3:23]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

            checkUninitVar("void f(std::ostringstream& os) {\n"
                           "    int a;\n"
                           "    os << 1 << a;\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:3:16]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

            {
                // #9422
                checkUninitVar("void f() {\n"
                               "  char *p = new char[10];\n"
                               "  std::cout << (void *)p << 1;\n"
                               "}");
                ASSERT_EQUALS("", errout_str());

                checkUninitVar("void f() {\n"
                               "  char p[10];\n"
                               "  std::cout << (void *)p << 1;\n"
                               "}");
                ASSERT_EQUALS("", errout_str());

                checkUninitVar("void f() {\n"
                               "  char *p = new char[10];\n"
                               "  std::cout << p << 1;\n"
                               "}");
                ASSERT_EQUALS("[test.cpp:3:16]: (error) Memory is allocated but not initialized: p [uninitdata]\n", errout_str());

                checkUninitVar("void f() {\n" // #9696
                               "  int *p = new int[10];\n"
                               "  std::cout << p << 1;\n"
                               "}");
                ASSERT_EQUALS("", errout_str());

                checkUninitVar("void f() {\n"
                               "  int i[10];\n"
                               "  std::cout << i;\n"
                               "  char c[10];\n"
                               "  std::cout << c;\n"
                               "  wchar_t w[10];\n"
                               "  std::cout << w;\n"
                               "}");
                ASSERT_EQUALS("[test.cpp:5:16]: (error) Uninitialized variable: c [legacyUninitvar]\n"
                              "[test.cpp:7:16]: (error) Uninitialized variable: w [legacyUninitvar]\n",
                              errout_str());

                checkUninitVar("void f() {\n"
                               "  char p[10];\n"
                               "  std::cout << p << 1;\n"
                               "}");
                ASSERT_EQUALS("[test.cpp:3:16]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

                checkUninitVar("void f() {\n"
                               "  char p[10];\n"
                               "  std::cout << *p << 1;\n"
                               "}");
                ASSERT_EQUALS("[test.cpp:3:17]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());
            }

            checkUninitVar("void f() {\n" // #13908
                           "    int* i = new int;\n"
                           "    std::cout << i << \", \" << *i;\n"
                           "    delete i;\n"
                           "}\n");
            ASSERT_EQUALS("[test.cpp:3:32]: (error) Memory is allocated but not initialized: i [uninitdata]\n", errout_str());
        }

        // #8494 : Overloaded & operator
        checkUninitVar("void f() {\n"
                       "  int x;\n"
                       "  a & x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int a) {\n"
                       "  int x;\n"
                       "  a & x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "  int a,b,c;\n"
                       "  ar & a & b & c;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a() {\n"   // asm
                       "    int x;\n"
                       "    asm();\n"
                       "    x++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    int x[10];\n"
                       "    struct xyz xyz1 = { .x = x };\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a()\n"
                       "{\n"
                       "    struct S *s;\n"
                       "    s->x = 0;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Uninitialized variable: s [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "   char *buf = malloc(100);\n"
                       "   struct ABC *abc = buf;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("class Fred {\n"
                       "public:\n"
                       "    FILE *f;\n"
                       "    ~Fred();\n"
                       "}\n"
                       "Fred::~Fred()\n"
                       "{\n"
                       "    fclose(f);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int c;\n"
                       "    ab(sizeof(xyz), &c);\n"
                       "    if (c);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int c;\n"
                       "    a = (f2(&c));\n"
                       "    c++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int a)\n"
                       "{\n"
                       "    if (a) {\n"
                       "        char *p;\n"
                       "        *p = 0;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        // +=
        checkUninitVar("void f()\n"
                       "{\n"
                       "    int c;\n"
                       "    c += 2;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a[10];\n"
                       "    a[0] = 10 - a[1];\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (error) Uninitialized variable: a[1] [legacyUninitvar]\n", errout_str());

        // goto/setjmp/longjmp..
        checkUninitVar("void foo(int x)\n"
                       "{\n"
                       "    long b;\n"
                       "    if (g()) {\n"
                       "        b =2;\n"
                       "        goto found;\n"
                       "    }\n"
                       "\n"
                       "    return;\n"
                       "\n"
                       "found:\n"
                       "    int a = b;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    jmp_buf env;\n"
                       "    int a;\n"
                       "    int val = setjmp(env);\n"
                       "    if(val)\n"
                       "        return a;\n"
                       "    a = 1;\n"
                       "    longjmp(env, 1);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // macro_for..
        checkUninitVar("int foo()\n"
                       "{\n"
                       "  int retval;\n"
                       "  if (condition) {\n"
                       "    for12(1,2) { }\n"
                       "    retval = 1;\n"
                       "  }\n"
                       "  else\n"
                       "    retval = 2;\n"
                       "  return retval;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    goto exit;\n"
                       "    i++;\n"
                       "exit:\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo() {\n"
                       "    int x,y=0;\n"
                       "again:\n"
                       "    if (y) return x;\n"
                       "    x = a;\n"
                       "    y = 1;\n"
                       "    goto again;\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        // Ticket #3873 (false positive)
        checkUninitVar("MachineLoopRange *MachineLoopRanges::getLoopRange(const MachineLoop *Loop) {\n"
                       "  MachineLoopRange *&Range = Cache[Loop];\n"
                       "  if (!Range)\n"
                       "    Range = new MachineLoopRange(Loop, Allocator, *Indexes);\n"
                       "  return Range;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #4040 - False positive
        checkUninitVar("int f(int x)  {\n"
                       "    int iter;\n"
                       "    {\n"
                       "        union\n"
                       "        {\n"
                       "            int asInt;\n"
                       "            double asDouble;\n"
                       "        };\n"
                       "\n"
                       "        iter = x;\n"
                       "    }\n"
                       "    return 1 + iter;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // C++11 style initialization
        checkUninitVar("int f() {\n"
                       "    int i = 0;\n"
                       "    int j{ i };\n"
                       "    return j;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #5646
        checkUninitVar("float foo() {\n"
                       "  float source[2] = {3.1, 3.1};\n"
                       "  float (*sink)[2] = &source;\n"
                       "  return (*sink)[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #9296
        checkUninitVar("void f(void)\n"
                       "{\n"
                       "    int x;\n"
                       "    int z = (x) & ~__round_mask(1, 1);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:14]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f(void)\n"
                       "{\n"
                       "    int x;\n"
                       "    int z = (x) | ~__round_mask(1, 1);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:14]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("int __round_mask(int, int);\n"
                       "void f(void)\n"
                       "{\n"
                       "    int x;\n"
                       "    int* z = &x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_warn_once() {
        // extracttests.start: int a; int b;

        checkUninitVar("void f() {\n"
                       "  int x;\n"
                       "  a = x;\n"
                       "  b = x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());
    }

    // Handling of unknown types. Assume they are POD in C.
    void uninitvar_decl() {
        const char code[] = "void f() {\n"
                            "    dfs a;\n"
                            "    return a;\n"
                            "}";

        // Assume dfs is a non POD type if file is C++
        checkUninitVar(code);
        ASSERT_EQUALS("", errout_str());

        // Assume dfs is a POD type if file is C
        checkUninitVar(code, dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:12]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        const char code2[] = "struct AB { int a,b; };\n"
                             "void f() {\n"
                             "    struct AB ab;\n"
                             "    return ab;\n"
                             "}";
        checkUninitVar(code2);
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n"
                      "[test.cpp:4:12]: (error) Uninitialized struct member: ab.b [uninitStructMember]\n", errout_str());
        checkUninitVar(code2, dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:4:12]: (error) Uninitialized variable: ab [legacyUninitvar]\n", errout_str());

        // Ticket #3890 - False positive for std::map
        checkUninitVar("void f() {\n"
                       "    std::map<int,bool> x;\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #3906 - False positive for std::vector pointer
        checkUninitVar("void f() {\n"
                       "    std::vector<int> *x = NULL;\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        {
            // Ticket #6701 - Variable name is a POD type according to cfg
            constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                       "<def format=\"1\">"
                                       "  <podtype name=\"_tm\"/>"
                                       "</def>";
            const Settings s = settingsBuilder(settings).libraryxml(xmldata).build();
            checkUninitVar("void f() {\n"
                           "  Fred _tm;\n"
                           "  _tm.dostuff();\n"
                           "}", s);
            ASSERT_EQUALS("", errout_str());
        }

        // Ticket #7822 - Array type
        checkUninitVar("A *f() {\n"
                       "    A a,b;\n"
                       "    b[0] = 0;"
                       "    return a;\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar3() { // #3844
        // avoid false positive
        checkUninitVar("namespace std _GLIBCXX_VISIBILITY(default)\n"
                       "{\n"
                       "_GLIBCXX_BEGIN_NAMESPACE_CONTAINER\n"
                       "    typedef unsigned long _Bit_type;\n"
                       "    struct _Bit_reference\n"
                       "    {\n"
                       "        _Bit_type * _M_p;\n"
                       "        _Bit_type _M_mask;\n"
                       "        _Bit_reference(_Bit_type * __x, _Bit_type __y)\n"
                       "         : _M_p(__x), _M_mask(__y) { }\n"
                       "    };\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_bitop() {
        // extracttests.start: int a; int c;

        checkUninitVar("void foo() {\n"
                       "    int b;\n"
                       "    c = a | b;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: b [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo() {\n"
                       "    int b;\n"
                       "    c = b | a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: b [legacyUninitvar]\n", errout_str());
    }

    // if..
    void uninitvar_if() {
        // extracttests.start: struct Foo { void abcd(); };
        checkUninitVar("static void foo(int x)\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    if (x)\n"
                       "        p = new Foo;\n"
                       "    p->abcd();\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        checkUninitVar("static void foo(int x)\n"
                       "{\n"
                       "    int a;\n"
                       "    if (x==1);\n"
                       "    if (x==2);\n"
                       "    x = a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:6:9]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("int foo() {\n"
                       "    int i;\n"
                       "    if (1)\n"
                       "        i = 11;\n"
                       "    return i;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int bar(int x) {\n"
                       "    int n;\n"
                       "    if ( x == 23)\n"
                       "      n = 1;\n"
                       "    else if ( x == 11 )\n"
                       "      n = 2;\n"
                       "    return n;\n"
                       "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    if (x)\n"
                       "        i = 22;\n"
                       "    else\n"
                       "        i = 33;\n"
                       "    return i;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo(int x)\n" // #5503
                       "{\n"
                       "    int i;\n"
                       "    if (x < 2)\n"
                       "        i = 22;\n"
                       "    else if (x >= 2)\n" // condition is always true
                       "        i = 33;\n"
                       "    return i;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    if (x)\n"
                       "        i = 22;\n"
                       "    else\n"
                       "    {\n"
                       "        char *y = {0};\n"
                       "        i = 33;\n"
                       "    }\n"
                       "    return i;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int i;\n"
                       "    if (x)\n"
                       "    {\n"
                       "        struct abc abc1 = (struct abc) { .a=0, .b=0, .c=0 };\n"
                       "        i = 22;\n"
                       "    }\n"
                       "    else\n"
                       "    {\n"
                       "        i = 33;\n"
                       "    }\n"
                       "    return i;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("static void foo(int x)\n"
                       "{\n"
                       "    Foo *p;\n"
                       "    if (x)\n"
                       "        p = new Foo;\n"
                       "    if (x)\n"
                       "        p->abcd();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo(int a)\n"
                       "{\n"
                       "    int n;\n"
                       "    int condition;\n"
                       "    if(a == 1) {\n"
                       "        n=0;\n"
                       "        condition=0;\n"
                       "    }\n"
                       "    else {\n"
                       "        n=1;\n"
                       "    }\n"
                       "\n"
                       "    if( n == 0) {\n"
                       "        a=condition;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    C *c;\n"
                       "    if (fun(&c));\n"
                       "    c->Release();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    C c;\n"
                       "    if (fun(&c.d));\n"
                       "    return c;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "   char a[10];\n"
                       "   if (a[0] = x){}\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo(int x)\n"
                       "{\n"
                       "    int i;\n"
                       "    if (one())\n"
                       "        i = 1;\n"
                       "    else\n"
                       "        return 3;\n"
                       "    return i;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #2207 - False positive
        checkUninitVar("void foo(int x) {\n"
                       "    int a;\n"
                       "    if (x)\n"
                       "        a = 1;\n"
                       "    if (!x)\n"
                       "        return;\n"
                       "    b = (c - a);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo()\n"
                       "{\n"
                       "    int ret;\n"
                       "    if (one())\n"
                       "        ret = 1;\n"
                       "    else\n"
                       "        throw 3;\n"
                       "    return ret;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f(int a)\n"
                       "{\n"
                       "    int ret;\n"
                       "    if (a == 1)\n"
                       "        ret = 1;\n"
                       "    else\n"
                       "        XYZ ret = 2;\n"  // XYZ may be an unexpanded macro so bailout the checking of "ret".
                       "    return ret;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:8:12]: (error) Uninitialized variable: ret [legacyUninitvar]\n", errout_str());

        checkUninitVar("int f(int a, int b)\n"
                       "{\n"
                       "   int x;\n"
                       "   if (a)\n"
                       "      x = a;\n"
                       "   else {\n"
                       "      do { } while (f2());\n"
                       "      x = b;\n"
                       "   }\n"
                       "   return x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo(long verbose,bool bFlag)\n"
                       "{\n"
                       "  double t;\n"
                       "  if (bFlag)\n"
                       "  {\n"
                       "    if (verbose)\n"
                       "      t = 1;\n"
                       "    if (verbose)\n"
                       "      std::cout << (12-t);\n"
                       "  }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int test(int cond1, int cond2) {\n"
                       "  int foo;\n"
                       "  if (cond1 || cond2) {\n"
                       "     if (cond2)\n"
                       "        foo = 0;\n"
                       "  }\n"
                       "  if (cond2) {\n"
                       "    int t = foo*foo;\n"
                       "  }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo(int *pix) {\n"
                       "    int dest_x;\n"
                       "    {\n"
                       "        if (pix)\n"
                       "            dest_x = 123;\n"
                       "    }\n"
                       "    if (pix)\n"
                       "        a = dest_x;\n" // <- not uninitialized
                       "}");
        ASSERT_EQUALS("", errout_str());

        // ? :
        checkUninitVar("static void foo(int v) {\n"
                       "    int x;\n"
                       "    x = v <= 0 ? -1 : x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:23]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    const char *msgid1, *msgid2;\n"
                       "    int ret = bar(&msgid1);\n"
                       "    if (ret > 0) {\n"
                       "        ret = bar(&msgid2);\n"
                       "    }\n"
                       "    ret = ret <= 0 ? -1 :\n"
                       "          strcmp(msgid1, msgid2) == 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo(int a, int b)\n"
                       "{\n"
                       "    int x; x = (a<b) ? 1 : 0;\n"
                       "    int y = y;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized variable: y [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo() {\n" // pidgin-2.11.0/finch/libgnt/gnttree.c
                       "  int x = (x = bar()) ? x : 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // ; { .. }
        checkUninitVar("int foo()\n"
                       "{\n"
                       "  int retval;\n"
                       "  if (condition) {\n"
                       "    { }\n"
                       "    retval = 1; }\n"
                       "  else\n"
                       "    retval = 2;\n"
                       "  return retval;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "  {\n"
                       "    for (int i = 0; i < 10; ++i)\n"
                       "    { }\n"
                       "  }\n"
                       "\n"
                       "  { }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // ({ .. })
        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    if (abc) { x = 123; }\n"
                       "    else { a = ({b=c;}); x = 456; }\n"
                       "    ++x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #3098 - False negative uninitialized variable
        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *c1,*c2;\n"
                       "    if(strcoll(c1,c2))\n"
                       "    {\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (error) Uninitialized variable: c1 [legacyUninitvar]\n"
                      "[test.cpp:4:19]: (error) Uninitialized variable: c2 [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f(char *c1, char *c2)\n"
                       "{\n"
                       "    if(strcoll(c1,c2))\n"
                       "    {\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *c1;\n"
                       "    c1=strcpy(c1,\"test\");\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:15]: (error) Uninitialized variable: c1 [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f(char *c1)\n"
                       "{\n"
                       "    c1=strcpy(c1,\"test\");\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "  X var;\n"
                       "  memset(var, 0, sizeof(var));\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f() {\n" // #8692
                       "    bool b = e();\n"
                       "    int v;\n"
                       "    if (b)\n"
                       "        doStuff(&v);\n"
                       "    int v2 = (b) ? v / 5 : 0;\n"
                       "    int v3;\n"
                       "    if (b)\n"
                       "        v3 = 50;\n"
                       "    int v4 = (b) ? v3 + 5 : 0;\n"
                       "    int v5;\n"
                       "    int v6 = v5;\n"
                       "    doStuff(&v5);\n"
                       "    int v7 = v5;\n"
                       "    return v2 + v4 + v6 + v7;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:12:14]: (error) Uninitialized variable: v5 [legacyUninitvar]\n", errout_str());

        checkUninitVar("bool set(int *p);\n"
                       "\n"
                       "void foo(bool a) {\n"
                       "    bool flag{false};\n"
                       "    int x;\n"
                       "    if (!a) {\n"
                       "        flag = set(&x);\n"
                       "    }\n"
                       "    if (!flag || x == 0) {}\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int n, int a) {\n" // #12011
                       "    double x[10];\n"
                       "    if (n == 1) {\n"
                       "        if (a)\n"
                       "            x[0] = 4;\n"
                       "    }\n"
                       "    else\n"
                       "        for (int i = 0; i < n; i++) {\n"
                       "            if (a)\n"
                       "                x[i] = 8;\n"
                       "        }\n"
                       "    if (n == 1)\n"
                       "        if (a)\n"
                       "            (void)x[0];\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());
    }


    // handling for/while loops..
    void uninitvar_loops() {
        // for..
        // extracttests.start: void b(int);
        checkUninitVar("void f()\n"
                       "{\n"
                       "    for (int i = 0; i < 4; ++i) {\n"
                       "        int a;\n"
                       "        b(4*a);\n"
                       "    }"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int k;\n"
                       "    for (int i = 0; i < 4; ++i) {\n"
                       "        k = k + 2;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized variable: k [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    gchar sel[10];\n"
                       "    for (int i = 0; i < 4; ++i) {\n"
                       "        int sz = sizeof(sel);\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("enum ABCD { A, B, C, D };\n"
                       "\n"
                       "static void f(char *str ) {\n"
                       "    enum ABCD i;\n"
                       "    for (i = 0; i < D; i++) {\n"
                       "        str[i] = 0;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void x() {\n"
                       "    do  {\n"
                       "        Token * tok;\n"
                       "        for (tok = a; tok; tok = tok->next())\n"
                       "        {\n"
                       "        }\n"
                       "    } while (tok2);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo(void) {\n"
                       "    int a = 0;\n"
                       "    int x;\n"
                       "\n"
                       "    for (;;) {\n"
                       "        if (!a || 12 < x) {\n" // <- x is not uninitialized
                       "            a = 1;\n"
                       "            x = 2;\n"
                       "        }\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo(void) {\n"
                       "    int a = 0;\n"
                       "    int x;\n"
                       "\n"
                       "    for (;;) {\n"
                       "        if (!a || 12 < x) {\n" // <- x is uninitialized
                       "            a = 1;\n"
                       "        }\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:6:24]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo(int n) {\n"
                       "  int one[10];\n"
                       "  for (int rank = 0; rank < n; ++rank) {\n"
                       "    for (int i=0;i<rank;i++)\n"
                       "      f = one[i];\n"
                       "    one[rank] = -1;\n"
                       "  }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #2226: C++0x loop
        checkUninitVar("void f() {\n"
                       "    container c;\n"
                       "    for (iterator it : c) {\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #2345: False positive in sub-condition in if inside a loop
        checkUninitVar("void f(int x) {\n"
                       "    const PoolItem* pItem;\n"
                       "    while (x > 0) {\n"
                       "        if (GetItem(&pItem) && (*pItem != rPool))\n"
                       "        { }\n"
                       "        x--;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
        // extracttests.start: struct PoolItem { bool operator!=(const PoolItem&) const; };
        checkUninitVar("void f(int x, const PoolItem& rPool) {\n"
                       "    const PoolItem* pItem;\n"
                       "    while (x > 0) {\n"
                       "        if (*pItem != rPool)\n"
                       "        { }\n"
                       "        x--;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:14]: (error) Uninitialized variable: pItem [legacyUninitvar]\n", errout_str());

        // #2231 - conditional initialization in loop..
        checkUninitVar("int foo(char *a) {\n"
                       "    int x;\n"
                       "\n"
                       "    for (int i = 0; i < 10; ++i) {\n"
                       "        if (a[i] == 'x') {\n"
                       "            x = i;\n"
                       "            break;\n"
                       "        }\n"
                       "    }\n"
                       "\n"
                       "    return x;\n"
                       "}");
        TODO_ASSERT_EQUALS("[test.cpp:11]: (error) Uninitialized variable: x\n", "", errout_str());

        // Ticket #2796
        checkUninitVar("void foo() {\n"
                       "    while (true) {\n"
                       "        int x;\n"
                       "        if (y) x = 0;\n"
                       "        else break;\n"
                       "        return x;\n"   // <- x is initialized
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Assignment in for. Ticket #3287
        checkUninitVar("int foo(char* in, bool b) {\n"
                       "    char* c;\n"
                       "    if (b) for (c = in; *c == 0; ++c) {}\n"
                       "    else c = in + strlen(in) - 1;\n"
                       "    *c = 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #10273 - assignment in conditional code
        // extracttests.start: extern const int PORT_LEARN_DISABLE;
        checkUninitVar("void foo() {\n"
                       "    int learn;\n"
                       "    for (int index = 0; index < 10; index++) {\n"
                       "        if (!(learn & PORT_LEARN_DISABLE))\n"
                       "            learn = 123;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:15]: (error) Uninitialized variable: learn [legacyUninitvar]\n", errout_str());

        // extracttests.start: struct Entry { Entry *next; }; Entry *buckets[10];
        checkUninitVar("void foo() {\n"
                       "  Entry *entry, *nextEntry;\n"
                       "  for(int i = 0; i < 10; i++) {\n"
                       "    for(entry = buckets[i]; entry != NULL; entry = nextEntry) {\n" // <- nextEntry is not uninitialized
                       "      nextEntry = entry->next;\n"
                       "    }\n"
                       "  }\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo() {\n"
                       "  Entry *entry, *nextEntry;\n"
                       "  for(int i = 0; i < 10; i++) {\n"
                       "    for(entry = buckets[i]; entry != NULL; entry = nextEntry) {\n"
                       "    }\n"
                       "  }\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4:52]: (error) Uninitialized variable: nextEntry [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "    list *f = NULL;\n"
                       "    list *l;\n"
                       "\n"
                       "    while (--x) {\n"
                       "        if (!f)\n"
                       "            f = c;\n"
                       "        else\n"
                       "            l->next = c;\n" // <- not uninitialized
                       "        l = c;\n"
                       "    }\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        // #6952 - do-while-loop
        checkUninitVar("void f(void)\n"
                       "{\n"
                       "    int* p;\n"
                       "    do\n"
                       "    {\n"
                       "        if (true) {;}\n"
                       "        else\n"
                       "        {\n"
                       "            return;\n"
                       "        }\n"
                       "        *p = 7;\n" // <<
                       "        p = new int(9);\n"
                       "    } while (*p != 8);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:11:10]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        // #6952 - while-loop
        checkUninitVar("void f(void)\n"
                       "{\n"
                       "    int* p;\n"
                       "    while (*p != 8) {\n" // <<
                       "        *p = 7;\n"
                       "        p = new int(9);\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        // switch in loop
        checkUninitVar("int foo(int *p) {\n"
                       "  int x;\n"
                       "  while (true) {\n"
                       "    switch (*p) {\n"
                       "    case 1:\n"
                       "        return x;\n"
                       "    case 2:\n"
                       "        x = 123;\n"
                       "        break;\n"
                       "    };\n"
                       "    ++p\n"
                       "  }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    // switch..
    void uninitvar_switch() {
        checkUninitVar("void f(int x)\n"
                       "{\n"
                       "    short c;\n"
                       "    switch(x) {\n"
                       "    case 1:\n"
                       "        c++;\n"
                       "        break;\n"
                       "    };\n"
                       "}");
        TODO_ASSERT_EQUALS("[test.cpp:6:5]: (error) Uninitialized variable: c [uninitvar]\n", "", errout_str());

        checkUninitVar("char * f()\n"
                       "{\n"
                       "    static char ret[200];\n"
                       "    memset(ret, 0, 200);\n"
                       "    switch (x)\n"
                       "    {\n"
                       "        case 1: return ret;\n"
                       "        case 2: return ret;\n"
                       "    }\n"
                       "    return 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo(const int iVar, unsigned int slot, unsigned int pin)\n"
                       "{\n"
                       "    int i;\n"
                       "    if (iVar == 0)\n"
                       "    {\n"
                       "        switch (slot)\n"
                       "        {\n"
                       "            case 4:  return 5;\n"
                       "            default: return -1;\n"
                       "        }\n"
                       "    }\n"
                       "    else\n"
                       "    {\n"
                       "        switch (pin)\n"
                       "        {\n"
                       "            case 0:   i =  2; break;\n"
                       "            default:  i = -1; break;\n"
                       "        }\n"
                       "    }\n"
                       "    return i;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #1855 - switch(foo(&x))
        checkUninitVar("int a()\n"
                       "{\n"
                       "    int x;\n"
                       "    switch (foo(&x))\n"
                       "    {\n"
                       "        case 1:\n"
                       "            return x;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #3231 - ({ switch .. })
        checkUninitVar("void f() {\n"
                       "    int a;\n"
                       "    ({\n"
                       "    switch(sizeof(int)) {\n"
                       "    case 4:\n"
                       "    default:\n"
                       "        (a)=0;\n"
                       "        break;\n"
                       "    };\n"
                       "    })\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    // arrays..
    void uninitvar_arrays() {
        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[10];\n"
                       "    a[a[0]] = 0;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:8]: (error) Uninitialized variable: a[0] [legacyUninitvar]\n", errout_str());

        checkUninitVar("int f()\n"
                       "{\n"
                       "    char a[10];\n"
                       "    *a = '\\0';\n"
                       "    int i = strlen(a);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a, b[10];\n"
                       "    a = b[0] = 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[10], b[10];\n"
                       "    a[0] = b[0] = 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[10], *p;\n"
                       "    *(p = a) = 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    char a[10], *p;\n"
                       "    p = &(a[10]);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // array usage in ?: (tests that the isVariableUsed() works)
        checkUninitVar("void f() {\n"
                       "    char a[10], *p;\n"
                       "    p = c?a:0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "    char a[10], c;\n"
                       "    c = *(x?a:0);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    char a[10], c;\n"
                       "    strcpy(dest, x?a:\"\");\n"
                       "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "  int a[2];\n"
                       "  y *= (x ? 1 : 2);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // passing array to library functions
        checkUninitVar("void f()\n"
                       "{\n"
                       "    char c[50] = \"\";\n"
                       "    strcat(c, \"test\");\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(char *s2) {\n"
                       "    char s[20];\n"
                       "    strcpy(s2, s);\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Uninitialized variable: s [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    char s[20];\n"
                       "    strcat(s, \"abc\");\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: s [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    char s[20];\n"
                       "    strchr(s, ' ');\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: s [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "        int y[2];\n"
                       "        int s;\n"
                       "        GetField( y + 0, y + 1 );\n"
                       "        s = y[0] * y[1];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "        int a[2];\n"
                       "        init(a - 1);\n"
                       "        int b = a[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "        Fred a[2];\n"
                       "        Fred b = a[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo() {\n"
                       "  char buf[1024];\n"
                       "  char *b = (char *) (((uintptr_t) buf + 63) & ~(uintptr_t) 63);\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo() {\n"
                       "  char buf[1024];\n"
                       "  char x = *(char *) (((uintptr_t) buf + 63) & ~(uintptr_t) 63);\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:3:36]: (error) Uninitialized variable: buf [legacyUninitvar]\n", errout_str());

        // Passing array to function
        checkUninitVar("void f(int i);\n"
                       "void foo()\n"
                       "{\n"
                       "    int a[10];\n"
                       "    f(a[0]);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:7]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        // Ticket #2320
        checkUninitVar("void foo() {\n"
                       "        char a[2];\n"
                       "        unsigned long b = (unsigned long)(a+2) & ~7;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"   // Ticket #3050
                       "    char a[2];\n"
                       "    printf(\"%s\", a);\n"
                       "}");
        TODO_ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: a [legacyUninitvar]\n", "", errout_str());

        checkUninitVar("void f() {\n"   // Ticket #5108 (fp)
                       "    const char *a;\n"
                       "    printf(\"%s\", a=\"abc\");\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"    // Ticket #3497
                       "    char header[1];\n"
                       "    *((unsigned char*)(header)) = 0xff;\n"
                       "    return header[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"    // Ticket #3497
                       "    char header[1];\n"
                       "    *((unsigned char*)((unsigned char *)(header))) = 0xff;\n"
                       "    return header[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    ABC abc;\n"
                       "    int a[1];\n"
                       "\n"
                       "    abc.a = a;\n"
                       "    init(&abc);\n"
                       "    return a[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #3344
        checkUninitVar("void f(){\n"
                       "   char *strMsg = \"This is a message\";\n"
                       "   char *buffer=(char*)malloc(128*sizeof(char));\n"
                       "   strcpy(strMsg,buffer);\n"
                       "   free(buffer);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (error) Memory is allocated but not initialized: buffer [uninitdata]\n", errout_str());

        checkUninitVar("void f(){\n"
                       "   char *strMsg = \"This is a message\";\n"
                       "   char *buffer=static_cast<char*>(malloc(128*sizeof(char)));\n"
                       "   strcpy(strMsg,buffer);\n"
                       "   free(buffer);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (error) Memory is allocated but not initialized: buffer [uninitdata]\n", errout_str());

        // #3845
        checkUninitVar("int foo() {\n"
                       "    int a[1] = {5};\n"
                       "    return a[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo() {\n"
                       "    int a[2][2] = {{3,4}, {5,6}};\n"
                       "    return a[0][1];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo() {\n"
                       "    int a[1];\n"
                       "    return a[0];\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("int foo() {\n"
                       "    int a[2][2];\n"
                       "    return a[0][1];\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("int foo() {\n"
                       "    int a[10];\n"
                       "    dostuff(a[0]);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // # 4740
        checkUninitVar("void f() {\n"
                       "    int *a[2][19];\n"
                       "    int **b = a[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #6869 - FP when passing uninit array to function
        checkUninitVar("void bar(PSTR x);\n"
                       "void foo() {\n"
                       "  char x[10];\n"
                       "  bar(x);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // struct
        checkUninitVar("struct Fred { int x; int y; };\n"
                       ""
                       "void f() {\n"
                       "  struct Fred fred[10];\n"
                       "  fred[1].x = 0;\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("char f() {\n"
                       "    std::array<char, 1> a;\n"
                       "    return a[0];\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("std::string f() {\n"
                       "    std::array<std::string, 1> a;\n"
                       "    return a[0];\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f() {\n" // #12355
                       "    const int x[10](1, 2);\n"
                       "    if (x[0] == 1) {}\n"
                       "    return x[0];\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_pointertoarray() {
        checkUninitVar("void draw_quad(float z)  {\n"
                       "    int i;\n"
                       "    float (*vertices)[2][4];\n"
                       "    vertices[0][0][0] = z;\n"
                       "    vertices[0][0][1] = z;\n"
                       "    vertices[1][0][0] = z;\n"
                       "    vertices[1][0][1] = z;\n"
                       "    vertices[2][0][0] = z;\n"
                       "    vertices[2][0][1] = z;\n"
                       "    vertices[3][0][0] = z;\n"
                       "    vertices[3][0][1] = z;\n"
                       "    for (i = 0; i < 4; i++) {\n"
                       "        vertices[i][0][2] = z;\n"
                       "        vertices[i][0][3] = 1.0;\n"
                       "        vertices[i][1][0] = 2.0;\n"
                       "        vertices[i][1][1] = 3.0;\n"
                       "        vertices[i][1][2] = 4.0;\n"
                       "        vertices[i][1][3] = 5.0;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Uninitialized variable: vertices [legacyUninitvar]\n",
                      errout_str());

        checkUninitVar("void f() {\n"
                       "    std::array<int, 3> *PArr[2] = { p0, p1 };\n"
                       "    (*PArr[0])[2] = 0;\n"
                       "    (*PArr[1])[2] = 0;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_cpp11ArrayInit() { // #7010
        checkUninitVar("double foo(bool flag) {\n"
                       "    double adIHPoint_local[4][4]{};\n"
                       "    function(*adIHPoint_local);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    // alloc..
    void uninitvar_alloc() {
        checkUninitVar("void f() {\n"
                       "    char *s = (char *)malloc(100);\n"
                       "    strcat(s, \"abc\");\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Memory is allocated but not initialized: s [uninitdata]\n", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *s1 = new char[10];\n"
                       "    char *s2 = new char[strlen(s1)];\n"
                       "};");
        ASSERT_EQUALS("[test.cpp:4:32]: (error) Memory is allocated but not initialized: s1 [uninitdata]\n", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *p = (char*)malloc(64);\n"
                       "    int x = p[0];\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Memory is allocated but not initialized: p [uninitdata]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    char *p = (char*)malloc(64);\n"
                       "    if (p[0]) { }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Memory is allocated but not initialized: p[0] [uninitdata]\n", errout_str());

        checkUninitVar("char f() {\n"
                       "    char *p = (char*)malloc(64);\n"
                       "    return p[0];\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Memory is allocated but not initialized: p [uninitdata]\n", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    Fred *fred = new Fred;\n"
                       "    fred->foo();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct Fred { int i; Fred(int, float); };\n"
                       "void f() {\n"
                       "    Fred *fred = new Fred(1, 2);\n"
                       "    fred->foo();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    Fred *fred = malloc(sizeof(Fred));\n"
                       "    x(&fred->f);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    Fred *fred = malloc(sizeof(Fred));\n"
                       "    x(fred->f);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo(char *s)\n"
                       "{\n"
                       "    char *a = malloc(100);\n"
                       "    *a = *s;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    char *a;\n"
                       "    if (a);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    char *a = malloc(100);\n"
                       "    if (a);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    ABC *abc = malloc(100);\n"
                       "    abc->a = 123;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    ABC *abc = malloc(100);\n"
                       "    abc->a.word = 123;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    ABC *abc = malloc(100);\n"
                       "    abc->a = 123;\n"
                       "    abc->a += 123;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    ABC *abc = malloc(100);\n"
                       "    free(abc);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char *s = (char*)malloc(100);\n"
                       "    if (!s)\n"
                       "        return;\n"
                       "    char c = *s;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:6:15]: (error) Memory is allocated but not initialized: s [uninitdata]\n", errout_str());

        // #3708 - false positive when using ptr typedef
        checkUninitVar("void f() {\n"
                       "    uintptr_t x = malloc(100);\n"
                       "    uintptr_t y = x + 10;\n"  // <- not bad usage
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "  z_stream strm;\n"
                       "  char* buf = malloc(10);\n"
                       "  strm.next_out = buf;\n"
                       "  deflate(&strm, Z_FINISH);\n"
                       "  memcpy(body, buf, 10);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #6451 - allocation in subscope
        checkUninitVar("struct StgStrm {\n"
                       "    StgIo& rIo;\n"
                       "    StgStrm(StgIo&);\n"
                       "    virtual sal_Int32 Write();\n"
                       "};\n"
                       "void Tmp2Strm() {\n"
                       "    StgStrm* pNewStrm;\n"
                       "    if (someflag)\n"
                       "        pNewStrm = new StgStrm(rIo);\n"
                       "    else\n"
                       "        pNewStrm = new StgStrm(rIo);\n"
                       "    pNewStrm->Write();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #6450 - calling a member function is allowed if memory was allocated by new
        checkUninitVar("struct EMFPFont {\n"
                       "    bool family;\n"
                       "    void Initialize();\n"
                       "};\n"
                       "void processObjectRecord() {\n"
                       "    EMFPFont *font = new EMFPFont();\n"
                       "    font->Initialize();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #7623 - new can also initialize the memory, don't warn in this case
        checkUninitVar("void foo(){\n"
                       "    int* p1 = new int(314);\n"
                       "    int* p2 = new int();\n"
                       "    int* arr = new int[5]();\n"
                       "    std::cout << *p1 << *p2 << arr[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // new in C code does not allocate..
        checkUninitVar("int main() {\n"
                       "    char * pBuf = new(10);\n"
                       "    a = *pBuf;\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("class A {};\n" // #10698
                       "void f() {\n"
                       "    A* a = new A{};\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        // #1175
        checkUninitVar("void f() {\n"
                       "    int* p = new int;\n"
                       "    *((int*)*p) = 42;\n"
                       "    delete p;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Memory is allocated but not initialized: p [uninitdata]\n", errout_str());

        checkUninitVar("int f() {\n" // #10596
                       "    int* a = new int;\n"
                       "    int i{};\n"
                       "    i += *a;\n"
                       "    delete a;\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Memory is allocated but not initialized: a [uninitdata]\n", errout_str());

        checkUninitVar("void* f(size_t n, int i) {\n" // #11766
                       "    char* p = (char*)malloc(n);\n"
                       "    *(int*)p = i;\n"
                       "    return p;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void* f(size_t n, int i) {\n"
                       "    char* p = (char*)malloc(n);\n"
                       "    *(int*)(void*)p = i;\n"
                       "    return p;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int n) {\n"
                       "    int* p = (int*)malloc(n * sizeof(int));\n"
                       "    for (int i = 0; i < n; ++i)\n"
                       "        *(p + i) = 0;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int* f() {\n" // #14448
                       "    int* p = (int*)malloc(4);\n"
                       "    if (!p)\n"
                       "        return nullptr;\n"
                       "    if (*p) {}\n"
                       "    return p;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5:9]: (error) Memory is allocated but not initialized: *p [uninitdata]\n", errout_str());

        checkUninitVar("int* f() {\n"
                       "    int* p = (int*)malloc(4);\n"
                       "    if (p == nullptr)\n"
                       "        return nullptr;\n"
                       "    if (*p) {}\n"
                       "    return p;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5:9]: (error) Memory is allocated but not initialized: *p [uninitdata]\n", errout_str());

        checkUninitVar("char* f(size_t nBytes, size_t nAlign) {\n" // #14485
                       "    char* p = reinterpret_cast<char*>(malloc(nBytes));\n"
                       "    if ((uintptr_t)p % nAlign != 0) {}\n"
                       "    return p;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct S { char c[10]; };\n" // #11290
                       "S* f() {\n"
                       "    S* s = (S*)malloc(sizeof(S));\n"
                       "    sprintf(s->c, \"abc\");\n"
                       "    return s;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // class / struct..
    void uninitvar_class() {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "    int i;\n"
                       "    int a() { return i; }\n"
                       "};");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    struct Relative {\n"
                       "        Surface *surface;\n"
                       "        void MoveTo(int x, int y) {\n"
                       "            surface->MoveTo();\n"
                       "        }\n"
                       "    };\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    static const struct ab {\n"
                       "        int a,b;\n"
                       "        int get_a() { return a; }"
                       "    } x = { 0, 0 };\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    int i;\n"
                       "    {\n"
                       "        union ab {\n"
                       "            int a,b;\n"
                       "        }\n"
                       "        i = 0;\n"
                       "    }\n"
                       "    return i;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "    struct AB ab;\n"
                       "    x = ab.x = 12;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    // enum..
    void uninitvar_enum() {
        checkUninitVar("void f()\n"
                       "{\n"
                       "    enum AB { a, b };\n"
                       "    AB ab;\n"
                       "    if (ab);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:9]: (error) Uninitialized variable: ab [legacyUninitvar]\n", errout_str());
    }

    // references..
    void uninitvar_references() {
        checkUninitVar("void f()\n"
                       "{\n"
                       "    int a;\n"
                       "    int &b = a;\n"
                       "    b = 0;\n"
                       "    int x = a;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(struct blame_entry *ent)\n"
                       "{\n"
                       "    struct origin *suspect = ent->suspect;\n"
                       "    char hex[41];\n"
                       "    strcpy(hex, sha1_to_hex(suspect->commit->object.sha1));\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo()\n"
                       "{\n"
                       "    const std::string s(x());\n"
                       "    strchr(s.c_str(), ',');\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #6717
        checkUninitVar("void f() {\n"
                       "    struct thing { int value; };\n"
                       "    thing it;\n"
                       "    int& referenced_int = it.value;\n"
                       "    referenced_int = 123;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_return() {

        checkUninitVar("static int foo() {\n"
                       "    int ret;\n"
                       "    return ret;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: ret [legacyUninitvar]\n", errout_str());

        checkUninitVar("static int foo() {\n"
                       "    int ret;\n"
                       "    return ret+5;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: ret [legacyUninitvar]\n", errout_str());

        checkUninitVar("static int foo() {\n"
                       "    int ret;\n"
                       "    return ret = 5;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        {
            checkUninitVar("static int foo() {\n"
                           "    int ret;\n"
                           "    cin >> ret;\n"
                           "    return ret;\n"
                           "}");
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("static int foo() {\n"
                           "    int ret;\n"
                           "    return cin >> ret;\n"
                           "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
            ASSERT_EQUALS("[test.c:3:19]: (error) Uninitialized variable: ret [legacyUninitvar]\n", errout_str());
        }

        // Ticket #6341- False negative
        {
            checkUninitVar("wchar_t f() { int i; return btowc(i); }");
            ASSERT_EQUALS("[test.cpp:1:35]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

            checkUninitVar("wchar_t f(int i) { return btowc(i); }");
            ASSERT_EQUALS("", errout_str());

            // Avoid a potential false positive (#6341)
            checkUninitVar(
                "void setvalue(int &x) {\n"
                "  x = 0;\n"
                "  return 123;\n"
                "}\n"
                "int f() {\n"
                "  int x;\n"
                "  return setvalue(x);\n"
                "}");
            ASSERT_EQUALS("", errout_str());
        }

        // Ticket #5412 - False negative
        {
            checkUninitVar("void f(bool b) {\n"
                           "    double f;\n"
                           "    if (b)   {  }\n"
                           "    else  {\n"
                           "        f = 0.0;\n"
                           "    }\n"
                           "    printf (\"%f\",f);\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:7:18]: (error) Uninitialized variable: f [legacyUninitvar]\n", errout_str());

            // Check for potential FP
            checkUninitVar("void f(bool b) {\n"
                           "    double f;\n"
                           "    if (b)   { f = 1.0 }\n"
                           "    else  {\n"
                           "        f = 0.0;\n"
                           "    }\n"
                           "    printf (\"%f\",f);\n"
                           "}");
            ASSERT_EQUALS("", errout_str());
        }

        // Ticket #2146 - False negative
        checkUninitVar("int f(int x) {\n"
                       "    int y;\n"
                       "    return x ? 1 : y;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Uninitialized variable: y [legacyUninitvar]\n", errout_str());

        // Ticket #3106 - False positive
        {
            checkUninitVar("int f() {\n"
                           "    int i;\n"
                           "    return x(&i) ? i : 0;\n"
                           "}");
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("int f() {\n"
                           "    int i;\n"
                           "    return x() ? i : 0;\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:3:16]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());
        }
    }

    void uninitvar_assign() {  // = { .. }
        // #1533
        checkUninitVar("char a()\n"
                       "{\n"
                       "    char key;\n"
                       "    struct A msg = { .buf = {&key} };\n"
                       "    init(&msg);\n"
                       "    key++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #5660 - False positive
        checkUninitVar("int f() {\n"
                       "    int result;\n"
                       "    int *res[] = {&result};\n"
                       "    foo(res);\n"
                       "    return result;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #6873
        checkUninitVar("int f() {\n"
                       "    char a[10];\n"
                       "    char *b[] = {a};\n"
                       "    foo(b);\n"
                       "    return atoi(a);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // = { .. }
        checkUninitVar("int f() {\n"
                       "    int a;\n"
                       "    int *p[] = { &a };\n"
                       "    *p[0] = 0;\n"
                       "    return a;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

    }

    // strncpy doesn't always null-terminate..
    void uninitvar_strncpy() {
        // TODO: Add this checking
        // Can it be added without hardcoding?

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[100];\n"
                       "    strncpy(a, s, 20);\n"
                       "    strncat(a, s, 20);\n"
                       "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Dangerous usage of 'a' (strncpy doesn't always null-terminate it).\n", "", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[100];\n"
                       "    strncpy(a, \"hello\", 3);\n"
                       "    strncat(a, \"world\", 20);\n"
                       "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Dangerous usage of 'a' (strncpy doesn't always null-terminate it).\n", "", errout_str());

        checkUninitVar("void f()\n"
                       "{\n"
                       "    char a[100];\n"
                       "    strncpy(a, \"hello\", sizeof(a));\n"
                       "    strncat(a, \"world\", 20);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #3245 - false positive
        {
            checkUninitVar("void f() {\n"
                           "    char a[100];\n"
                           "    strncpy(a,p,10);\n"
                           "    memcmp(a,q,10);\n"
                           "}");
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("void f() {\n"
                           "    char a[100];\n"
                           "    strncpy(a,p,10);\n"
                           "    if (memcmp(a,q,10)==0);\n"
                           "}");
            ASSERT_EQUALS("", errout_str());
        }

        // Using strncpy isn't necessarily dangerous usage
        checkUninitVar("void f(const char dev[], char *str) {\n"
                       "    char buf[10];\n"
                       "    strncpy(buf, dev, 10);\n"
                       "    strncpy(str, buf, 10);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "  char dst[4];\n"
                       "  const char* source = \"You\";\n"
                       "  strncpy(dst, source, sizeof(dst));\n"
                       "  char value = dst[2];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    // valid and invalid use of 'int a(int x) { return x + x; }'
    void func_uninit_var() {
        const std::string funca("int a(int x) { return x + x; }");

        checkUninitVar(funca +
                       "void b() {\n"
                       "    int x;\n"
                       "    a(x);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar(funca +
                       "void b() {\n"
                       "    int *p;\n"
                       "    a(*p);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:8]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());
    }


    // valid and invalid use of 'void a(int *p) { *p = 0; }'
    void func_uninit_pointer() {
        const std::string funca("void a(int *p) { *p = 0; }");

        // ok - initialized pointer
        checkUninitVar(funca +
                       "void b() {\n"
                       "    int buf[10];\n"
                       "    a(buf);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // not ok - uninitialized pointer
        checkUninitVar(funca +
                       "void b() {\n"
                       "    int *p;\n"
                       "    a(p);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());
    }

    void uninitvar_typeof() {
        checkUninitVar("void f() {\n"
                       "    struct Fred *fred;\n"
                       "    typeof(fred->x);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    struct SData * s;\n"
                       "    ab(typeof(s->status));\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    struct SData * s;\n"
                       "    TYPEOF(s->status);\n"
                       "}");
        TODO_ASSERT_EQUALS("", "[test.cpp:3:12]: (error) Uninitialized variable: s [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int *n = ({ typeof(*n) z;  (typeof(*n)*)z; })\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_ignore() {
        checkUninitVar("void foo() {\n"
                       "  int i;\n"
                       "  dostuff((int&)i, 0);\n" // <- cast is not use
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo() {\n"
                       "  int i;\n"
                       "  return (int&)i + 2;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("void foo() {\n"
                       "  int i;\n"
                       "  dostuff(*&i, 0);\n" // <- *& is not use
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int foo() {\n"
                       "  int i;\n"
                       "  return *&i;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());
    }

    void uninitvar2() {
        // using uninit var
        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    x++;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        // extracttests.start: char str[10];
        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    str[x] = 0;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n" // #7736
                       "    int buf[12];\n"
                       "    printf (\"%d\", buf[0] );\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (error) Uninitialized variable: buf [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    int y = x & 3;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    int y = 3 & x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    x = 3 + x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    x = x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        // extracttests.start: struct ABC {int a;};
        checkUninitVar("void f() {\n"
                       "    struct ABC *abc;\n"
                       "    abc->a = 0;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: abc [legacyUninitvar]\n", errout_str());

        checkUninitVar("int f() {\n"
                       "    static int x;\n"
                       "    return ++x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f() {\n"
                       "    extern int x;\n"
                       "    return ++x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"  // #3926 - weird cast.
                       "    int x;\n"
                       "    *(((char *)&x) + 0) = 0;\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n" // #4737 - weird cast.
                       "    int x;\n"
                       "    do_something(&((char*)&x)[0], 1);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    char *p = (char*)&x + 1;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int i;\n"
                       "    i=f(), i!=2;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // using uninit var in condition
        checkUninitVar("void f(void) {\n"
                       "    int x;\n"
                       "    if (x) { }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    if (1 == (3 & x)) { }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        // ?:
        checkUninitVar("int f(int *ptr) {\n"
                       "    int a;\n"
                       "    int *p = ptr ? ptr : &a;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f(int a) {\n"
                       "    int x;\n"
                       "    if (a==3) { x=2; }\n"
                       "    y = (a==3) ? x : a;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // = ({ .. })
        checkUninitVar("void f() {\n"
                       "    int x = ({ 1 + 2; });\n"
                       "    int y = 1 + (x ? y : y);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:22]: (error) Uninitialized variable: y [legacyUninitvar]\n", errout_str());

        // >> => initialization / usage
        {
            const char code[] = "void f() {\n"
                                "    int x;\n"
                                "    if (i >> x) { }\n"
                                "}";
            checkUninitVar(code);
            ASSERT_EQUALS("", errout_str());

            checkUninitVar(code, dinit(CheckUninitVarOptions, $.cpp = false));
            ASSERT_EQUALS("[test.c:3:14]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());
        }

        checkUninitVar("void f() {\n"
                       "    int i, i2;\n"
                       "    strm >> i >> i2;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // unconditional initialization
        checkUninitVar("int f() {\n"
                       "    int ret;\n"
                       "    if (a) { ret = 1; }\n"
                       "    else { {} ret = 2; }\n"
                       "    return ret;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f() {\n"
                       "    int ret;\n"
                       "    if (a) { ret = 1; }\n"
                       "    else { s=foo(1,{2,3},4); ret = 2; }\n"
                       "    return ret;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // conditional initialization
        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    if (y == 1) { x = 1; }\n"
                       "    else { if (y == 2) { x = 1; } }\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    if (y == 1) { x = 1; }\n"
                       "    else { if (y == 2) { x = 1; } }\n"
                       "    if (y == 3) { }\n"   // <- ignore condition
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:6:12]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        // initialization in condition
        checkUninitVar("void f() {\n"
                       "    int a;\n"
                       "    if (init(&a)) { }\n"
                       "    a++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // return, break, continue, goto
        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    if (y == 1) { return; }\n"
                       "    else { x = 1; }\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    if (y == 1) { return; }\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("int f(int x) {\n"
                       "    int ret;\n"
                       "    if (!x) {\n"
                       "        ret = -123;\n"
                       "        goto out1;\n"
                       "    }\n"
                       "    return 0;\n"
                       "out1:\n"
                       "out2:\n"
                       "    return ret;\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int i;\n"
                       "    if (x) {\n"
                       "        i = 1;\n"
                       "    } else {\n"
                       "        goto out;\n"
                       "    }\n"
                       "    i++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f() {\n"
                       "    int i,x;\n"
                       "    for (i=0;i<9;++i)\n"
                       "        if (foo) break;\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("int f() {\n"
                       "    int x;\n"
                       "    while (foo)\n"
                       "        if (bar) break;\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        // try/catch : don't warn about exception variable
        checkUninitVar("void f() {\n"
                       "    try {\n"
                       "    } catch (CException* e) {\n"
                       "        trace();\n"
                       "        e->Delete();\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n" // #5347
                       "    try {\n"
                       "    } catch (const char* e) {\n"
                       "        A a = e;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // exit
        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    if (y == 1) { exit(0); }\n"
                       "    else { x = 1; }\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // strange code.. don't crash (#3415)
        checkUninitVar("void foo() {\n"
                       "    int i;\n"
                       "    ({ if (0); });\n"
                       "    for_each(i) { }\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));

        // if, if
        checkUninitVar("void f(int a) {\n"
                       "    int i;\n"
                       "    if (a) i = 0;\n"
                       "    if (a) i++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int a,b=0;\n"
                       "    if (x) {\n"
                       "        if (y) {\n"
                       "            a = 0;\n"
                       "            b = 1;\n"
                       "        }\n"
                       "    }\n"
                       "    if (b) a++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int a=0, b;\n"
                       "    if (x) { }\n"
                       "    else { if (y==2) { a=1; b=2; } }\n"
                       "    if (a) { ++b; }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("static void f(int x, int y) {\n"
                       "    int a;\n"
                       "    if (x == 0) { a = y; }\n"
                       "    if (x == 0 && (a == 1)) { }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("static void f() {\n"
                       "    int a=0, b;\n"
                       "    if (something) { a = dostuff(&b); }\n"
                       "    if (!a || b) { }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("static void f(int x, int y) {\n"
                       "    int a;\n"
                       "    if (x == 0 && (a == 1)) { }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:20]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int a;\n"
                       "    if (x) { a = 0; }\n"
                       "    if (x) { if (y) { a++; } }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int a;\n"
                       "    if (x) { a = 0; }\n"
                       "    if (x) { if (y) { } else { a++; } }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    if (x) ab = getAB();\n"
                       "    else ab.a = 0;\n"
                       "    if (ab.a == 1) b = ab.b;\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f(void) {\n"
                       "    int a;\n"
                       "    int i;\n"
                       "    if (x) { noreturn(); }\n"
                       "    else { i = 0; }\n"
                       "    if (i==1) { a = 0; }\n"
                       "    else { a = 1; }\n"
                       "    return a;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f(int a) {\n" // #4560
                       "    int x = 0, y;\n"
                       "    if (a) x = 1;\n"
                       "    else return 0;\n"
                       "    if (x) y = 123;\n" // <- y is always initialized
                       "    else {}\n"
                       "    return y;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f(int a) {\n" // #6583
                       "    int x;\n"
                       "    if (a < 2) exit(1);\n"
                       "    else if (a == 2) x = 0;\n"
                       "    else exit(2);\n"
                       "    return x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f(int a) {\n" // #4560
                       "    int x = 1, y;\n"
                       "    if (a) x = 0;\n"
                       "    else return 0;\n"
                       "    if (x) {}\n"
                       "    else y = 123;\n" // <- y is always initialized
                       "    return y;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int x) {\n" // #3948
                       "  int value;\n"
                       "  if (x !=-1)\n"
                       "    value = getvalue();\n"
                       "  if (x == -1 || value > 300) {}\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("enum t_err { ERR_NONE, ERR_BAD_ARGS };\n" // #9649
                       "struct box_t { int value; };\n"
                       "int init_box(box_t *p, int v);\n"
                       "\n"
                       "void foo(int ret) {\n"
                       "    box_t box2;\n"
                       "    if (ret == ERR_NONE)\n"
                       "        ret = init_box(&box2, 20);\n"
                       "    if (ret == ERR_NONE)\n"
                       "        z = x + box2.value;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "  int value;\n"
                       "  if (x == 32)\n"
                       "    value = getvalue();\n"
                       "  if (x == 1)\n"
                       "    v = value;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:6:9]: (error) Uninitialized variable: value [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "  int value;\n"
                       "  if (x == 32)\n"
                       "    value = getvalue();\n"
                       "  if (x == 32) {}\n"
                       "  else v = value;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:6:12]: (error) Uninitialized variable: value [legacyUninitvar]\n", errout_str());

        checkUninitVar("static int x;" // #4773
                       "int f() {\n"
                       "    int y;\n"
                       "    if (x) g();\n"
                       "    if (x) y = 123;\n"
                       "    else y = 456;\n"
                       "    return y;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("static int x;" // #4773
                       "int f() {\n"
                       "    int y;\n"
                       "    if (!x) g();\n"
                       "    if (x) y = 123;\n"
                       "    else y = 456;\n"
                       "    return y;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int a) {\n"
                       "  int x;\n"
                       "  if (a) x=123;\n"
                       "  if (!a) {\n"
                       "    if (!a) {}\n"
                       "    else if (x) {}\n"
                       "  }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // asm
        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    asm();\n"
                       "    x++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // sizeof / typeof / offsetof / etc
        checkUninitVar("void f() {\n"
                       "    int i;\n"
                       "    sizeof(i+1);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int i;\n"
                       "    if (100 == sizeof(i+1));\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    struct ABC *abc;\n"
                       "    int i = ARRAY_SIZE(abc.a);"
                       "}");
        TODO_ASSERT_EQUALS("", "[test.cpp:3:24]: (error) Uninitialized variable: abc [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int *abc;\n"
                       "    typeof(*abc);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    struct ABC *abc;\n"
                       "    return do_something(typeof(*abc));\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    A *a;\n"
                       "    a = malloc(sizeof(*a));\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // &
        checkUninitVar("void f() {\n"  // #4426 - address of uninitialized variable
                       "    int a,b;\n"
                       "    if (&a == &b);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"  // #4439 - cast address of uninitialized variable
                       "    int a;\n"
                       "    x((LPARAM)(RECT*)&a);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int main() {\n"
                       "    int done;\n"
                       "    dostuff(1, (AuPointer) &done);\n" // <- It is not conclusive if the "&" is a binary or unary operator. Bailout.
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n" // #4778 - cast address of uninitialized variable
                       "    long a;\n"
                       "    &a;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n" // #4717 - ({})
                       "    int a = ({ long b = (long)(123); 2 + b; });\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
    }

    // #3869 - reference variable
    void uninitvar4() {
        checkUninitVar("void f() {\n"
                       "    int buf[10];\n"
                       "    int &x = buf[0];\n"
                       "    buf[0] = 0;\n"
                       "    x++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    // #3861
    void uninitvar5() {
        // ensure there is no false positive
        checkUninitVar("void f() {\n"
                       "    x<char> c;\n"
                       "    c << 2345;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // ensure there is no false negative
        checkUninitVar("void f() {\n"
                       "    char c;\n"
                       "    char a = c << 2;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());
    }

    // #13227
    void uninitvar6() {
        checkUninitVar("int foo(int x) {\n"
                       "    int i;\n"
                       "    if (x == 1) {\n"
                       "        i = 3;\n"
                       "    } else {\n"
                       "        do {\n"
                       "            return 2;\n"
                       "        } while (0);\n"
                       "    }\n"
                       "    return i;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar8() {
        const char code[] = "struct Fred {\n"
                            "    void Sync(dsmp_t& type, int& len, int limit = 123);\n"
                            "    void Sync(int& syncpos, dsmp_t& type, int& len, int limit = 123);\n"
                            "    void FindSyncPoint();\n"
                            "};\n"
                            "void Fred::FindSyncPoint() {\n"
                            "    dsmp_t type;\n"
                            "    int syncpos, len;\n"
                            "    Sync(syncpos, type, len);\n"
                            "}";
        checkUninitVar(code);
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar9() { // 6424
        const char code[] = "namespace Ns { class C; }\n"
                            "void f1() { char *p; *p = 0; }\n"
                            "class Ns::C* p;\n"
                            "void f2() { char *p; *p = 0; }";
        checkUninitVar(code);
        ASSERT_EQUALS("[test.cpp:2:23]: (error) Uninitialized variable: p [legacyUninitvar]\n"
                      "[test.cpp:4:23]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());
    }

    void uninitvar10() { // 9467
        const char code[] = "class Foo {\n"
                            "    template <unsigned int i>\n"
                            "    bool bar() {\n"
                            "        return true;\n"
                            "    }\n"
                            "};\n"
                            "template <>\n"
                            "bool Foo::bar<9>() {\n"
                            "    return true;\n"
                            "}\n"
                            "int global() {\n"
                            "    int bar = 1;\n"
                            "    return bar;\n"
                            "}";
        checkUninitVar(code);
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar11() { // 9123
        const char code[] = "bool get(int &var);\n"
                            "void foo () {\n"
                            "    int x;\n"
                            "    x = get(x) && x;\n"
                            "}";
        checkUninitVar(code);
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar12() { // 10218
        const char code[] = "void fp() {\n"
                            "  std::stringstream ss;\n"
                            "  for (int i; ss >> i;) {}\n"
                            "}";
        checkUninitVar(code);
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar13() { // #9772 - FP
        const char code[] = "int func(void)\n"
                            "{ int rez;\n"
                            "  struct sccb* ccb;\n"
                            " \n"
                            "  do\n"
                            "  { if ((ccb = calloc(1, sizeof(*ccb))) == NULL)\n"
                            "    { rez = 1;\n"
                            "      break;\n"
                            "    }\n"
                            "    rez = 0;\n"
                            "  } while (0);\n"
                            " \n"
                            "  if (rez != 0)\n"
                            "    free(ccb);\n"
                            " \n"
                            "  return rez;\n"
                            "}";
        checkUninitVar(code);
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar14() { // #11832
        const char code[] = "void f() {\n"
                            "    int b;\n"
                            "    *(&b) = 0;\n"
                            "}";
        checkUninitVar(code);
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_unconditionalTry() {
        // Unconditional scopes and try{} scopes
        checkUninitVar("int f() {\n"
                       "    int i;\n"
                       "    {\n"
                       "        return i;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("int f() {\n"
                       "    int i;\n"
                       "    try {\n"
                       "        return i;\n"
                       "    } catch(...) {}\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f(bool x) {\n"
                       "    bool b;\n"
                       "    {\n"
                       "        auto g = []{};\n"
                       "        b = x;\n"
                       "    }\n"
                       "    if (b) {}\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(bool x) {\n"
                       "    bool b;\n"
                       "    {\n"
                       "        int i[2]{ 1, 2 };\n"
                       "        b = x;\n"
                       "    }\n"
                       "    if (b) {}\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(bool x) {\n"
                       "    bool b;\n"
                       "    {\n"
                       "        auto g = []{};\n"
                       "    }\n"
                       "    if (b) {}\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:6:9]: (error) Uninitialized variable: b [legacyUninitvar]\n", errout_str());
    }

    void uninitvar_funcptr() {
        // extracttests.disable

        checkUninitVar("void getLibraryContainer() {\n"
                       "    Reference< XStorageBasedLibraryContainer >(*Factory)(const Reference< XComponentContext >&, const Reference< XStorageBasedDocument >&)\n"
                       "        = &DocumentDialogLibraryContainer::create;\n"
                       "    rxContainer.set((*Factory)(m_aContext, xDocument));\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void foo() {\n"
                       "    void* x;\n"
                       "    int (*f)(int, int) = x;\n"
                       "    dostuff((*f)(a,b));\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:26]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void getLibraryContainer() {\n"
                       "    Reference< XStorageBasedLibraryContainer >(*Factory)(const Reference< XComponentContext >&, const Reference< XStorageBasedDocument >&);\n"
                       "    rxContainer.set((*Factory)(m_aContext, xDocument));\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:23]: (error) Uninitialized variable: Factory [legacyUninitvar]\n", errout_str());

        // extracttests.enable
    }

    void uninitvar_operator() { // Ticket #6463, #6680
        checkUninitVar("struct Source { Source& operator>>(int& i) { i = 0; return *this; } };\n"
                       "struct Sink { int v; };\n"
                       "Source foo;\n"
                       "void uninit() {\n"
                       "  Sink s;\n"
                       "  int n = 1 >> s.v;\n" // Not initialized
                       "};\n"
                       "void notUninit() {\n"
                       "  Sink s1;\n"
                       "  foo >> s1.v;\n" // Initialized by operator>>
                       "  Sink s2;\n"
                       "  int n;\n"
                       "  foo >> s2.v >> n;\n" // Initialized by operator>>
                       "}");
        ASSERT_EQUALS("[test.cpp:6:16]: (error) Uninitialized struct member: s.v [uninitStructMember]\n", errout_str());

        checkUninitVar("struct Fred { int a; };\n"
                       "void foo() {\n"
                       "  Fred fred;\n"
                       "  std::cin >> fred.a;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    // Handling of function calls
    void uninitvar2_func() {
        // #4716
        checkUninitVar("void bar(const int a, const int * const b);\n"
                       "int foo(void) {\n"
                       "   int a;\n"
                       "   int *b = 0;\n"
                       "   bar(a,b);\n" // <<
                       "   return 0;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:8]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        // non-pointer variable
        checkUninitVar("void a(char);\n"  // value => error
                       "void b() {\n"
                       "    char c;\n"
                       "    a(c);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());

        checkUninitVar("void a(char c);\n"  // value => error
                       "void b() {\n"
                       "    char c;\n"
                       "    a(c);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());

        checkUninitVar("void a(const char c);\n"  // const value => error
                       "void b() {\n"
                       "    char c;\n"
                       "    a(c);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());

        checkUninitVar("void a(char *c);\n"  // address => no error
                       "void b() {\n"
                       "    char c;\n"
                       "    a(&c);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a(pstr s);\n"  // address => no error
                       "void b() {\n"
                       "    char c;\n"
                       "    a(&c);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a(const char *c);\n"  // const address => data is not changed
                       "void b() {\n"
                       "    char c;\n"
                       "    a(&c);\n"  // <- no warning
                       "    c++;\n"  // <- uninitialized variable
                       "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized variable: c\n", "", errout_str());

        // pointer variable
        checkUninitVar("void a(char c);\n"  // value => error
                       "void b() {\n"
                       "    char *c;\n"
                       "    a(*c);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:8]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());


        checkUninitVar("void a(char *c);\n"  // address => error
                       "void b() {\n"
                       "    char *c;\n"
                       "    a(c);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());

        checkUninitVar("typedef struct { int a; int b; } AB;\n"
                       "void a(AB *ab);\n"
                       "void b() {\n"
                       "    AB *ab;\n"
                       "    a(ab);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:7]: (error) Uninitialized variable: ab [legacyUninitvar]\n", errout_str());

        checkUninitVar("void a(const char *c);\n"  // const address => error
                       "void b() {\n"
                       "    char *c;\n"
                       "    a(c);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());

        checkUninitVar("void a(const char c[]);\n"  // const address => error
                       "void b() {\n"
                       "    char *c;\n"
                       "    a(c);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Uninitialized variable: c [legacyUninitvar]\n", errout_str());

        checkUninitVar("void a(char **c);\n"  // address of pointer => no error
                       "void b() {\n"
                       "    char *c;\n"
                       "    a(&c);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a(char *c);\n"  // address of pointer (suspicious cast to pointer) => no error
                       "void b() {\n"
                       "    char *c;\n"
                       "    a(&c);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void a(const char **c);\n"  // const address of pointer => no error
                       "void b() {\n"
                       "    const char *c;\n"
                       "    a(&c);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // array
        checkUninitVar("int calc(const int *p, int n);\n"
                       "void f() {\n"
                       "    int x[10];\n"
                       "    calc(x,10);\n"
                       "}");
        TODO_ASSERT_EQUALS("[test.cpp:4:14]: (error) Uninitialized variable: x [legacyUninitvar]\n",
                           "", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x[10];\n"
                       "    int &x0(*x);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // ....
        checkUninitVar("struct ABC { int a; };\n"  // struct initialization
                       "void clear(struct ABC &abc);\n"
                       "int f() {\n"
                       "    struct ABC abc;\n"
                       "    clear(abc);\n"
                       "    return abc.a;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void write_packet() {\n"
                       "    time_t now0;\n"
                       "    time(&now0);\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void write_packet() {\n"
                       "    time_t* now0;\n"
                       "    time(now0);\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:10]: (error) Uninitialized variable: now0 [legacyUninitvar]\n", errout_str());

        checkUninitVar("void write_packet() {\n"
                       "    char now0;\n"
                       "    strcmp(&now0, sth);\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:13]: (error) Uninitialized variable: now0 [legacyUninitvar]\n", errout_str());

        // #2775 - uninitialized struct pointer in subfunction
        // extracttests.start: struct Fred {int x;};
        checkUninitVar("void a(struct Fred *fred) {\n"
                       "    fred->x = 0;\n"
                       "}\n"
                       "\n"
                       "void b() {\n"
                       "    struct Fred *p;\n"
                       "    a(p);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:7:7]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        // #2946 - FP array is initialized in subfunction
        checkUninitVar("void a(char *buf) {\n"
                       "    buf[0] = 0;\n"
                       "}\n"
                       "void b() {\n"
                       "    char buf[10];\n"
                       "    a(buf);\n"
                       "    buf[1] = buf[0];\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // unknown macro
        checkUninitVar("void f() {\n"
                       "  struct listnode *item;\n"
                       "  list_for_each(item, &key_list) {}\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar2_value() {
        checkUninitVar("void f() {\n"
                       "    int i;\n"
                       "    if (x) {\n"
                       "        int y = -ENOMEM;\n"  // assume constant ENOMEM is nonzero since it's negated
                       "        if (y != 0) return;\n"
                       "        i++;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int i, y;\n"
                       "    if (x) {\n"
                       "        y = -ENOMEM;\n"  // assume constant ENOMEM is nonzero since it's negated
                       "        if (y != 0) return;\n"
                       "        i++;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int i, y;\n"
                       "    if (x) y = -ENOMEM;\n"  // assume constant ENOMEM is nonzero since it's negated
                       "    else y = get_value(i);\n"
                       "    if (y != 0) return;\n" // <- condition is always true if i is uninitialized
                       "    i++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "    int i;\n"
                       "    if (x) i = 0;\n"
                       "    if (!x || i>0) {}\n" // <- no error
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "    int i;\n"
                       "    if (!x) { }\n"
                       "    else i = 0;\n"
                       "    if (x || i>0) {}\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:14]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "    int i;\n"
                       "    if (x) { }\n"
                       "    else i = 0;\n"
                       "    if (x || i>0) {}\n" // <- no error
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f(int x) {\n"
                       "    int y;\n"
                       "    if (x) y = do_something();\n"
                       "    if (!x) return 0;\n"
                       "    return y;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: int y;
        checkUninitVar("int f(int x) {\n" // FP with ?:
                       "    int a;\n"
                       "    if (x)\n"
                       "        a = y;\n"
                       "    return x ? 2*a : 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f(int x) {\n"
                       "    int a;\n"
                       "    if (x)\n"
                       "        a = y;\n"
                       "    return y ? 2*a : 3*a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:14]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        ASSERT_THROW_INTERNAL(checkUninitVar("void f() {\n" // Don't crash
                                             "    int a;\n"
                                             "    dostuff(\"ab\" cd \"ef\", x?a:z);\n" // <- No AST is created for ?
                                             "}"), UNKNOWN_MACRO);

        // Unknown => bail out..
        checkUninitVar("void f(int x) {\n"
                       "    int i;\n"
                       "    if (a(x)) i = 0;\n"
                       "    if (b(x)) return;\n"
                       "    i++;\n" // <- no error if b(x) is always true when a(x) is false
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "    int i;\n"
                       "    if (x) i = 0;\n"
                       "    while (condition) {\n"
                       "        if (x) i++;\n" // <- no error
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(int x) {\n"
                       "    int i;\n"
                       "    if (x) i = 0;\n"
                       "    while (condition) {\n"
                       "        i++;\n"
                       "    }\n"
                       "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

#define valueFlowUninit(...) valueFlowUninit_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void valueFlowUninit_(const char* file, int line, const char (&code)[size], bool cpp = true)
    {
        SimpleTokenizer tokenizer(settings, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for redundant code..
        CheckUninitVar checkuninitvar(&tokenizer, &settings, this);
        (checkuninitvar.valueFlowUninit)();
    }

    void uninitvar15() { // #13685
        const char code[] = "int f() {\n"
                            "    int x;\n"
                            "    if (!({ int *p = &x; *p = 1; 1; }))\n"
                            "        return 0;\n"
                            "    return x;\n"
                            "}";
        valueFlowUninit(code, false);
        ASSERT_EQUALS("", errout_str());
    }

    void valueFlowUninit2_value()
    {
        valueFlowUninit("void f() {\n"
                        "    int i;\n"
                        "    if (x) {\n"
                        "        int y = -ENOMEM;\n" // assume constant ENOMEM is nonzero since it's negated
                        "        if (y != 0) return;\n"
                        "        i++;\n"
                        "    }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int i, y;\n"
                        "    if (x) {\n"
                        "        y = -ENOMEM;\n" // assume constant ENOMEM is nonzero since it's negated
                        "        if (y != 0) return;\n"
                        "        i++;\n"
                        "    }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int i, y;\n"
                        "    if (x) y = -ENOMEM;\n" // assume constant ENOMEM is nonzero since it's negated
                        "    else y = get_value(i);\n"
                        "    if (y != 0) return;\n" // <- condition is always true if i is uninitialized
                        "    i++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(int x) {\n"
                        "    int i;\n"
                        "    if (!x) i = 0;\n"
                        "    if (!x || i>0) {}\n" // <- error
                        "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:4:15]: (warning) Uninitialized variable: i [uninitvar]\n", errout_str());

        valueFlowUninit("void f(int x) {\n"
                        "    int i;\n"
                        "    if (x) i = 0;\n"
                        "    if (!x || i>0) {}\n" // <- no error
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(int x) {\n"
                        "    int i;\n"
                        "    if (!x) { }\n"
                        "    else i = 0;\n"
                        "    if (x || i>0) {}\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:5:14]: (warning) Uninitialized variable: i [uninitvar]\n", errout_str());

        valueFlowUninit("void f(int x) {\n"
                        "    int i;\n"
                        "    if (x) { }\n"
                        "    else i = 0;\n"
                        "    if (x || i>0) {}\n" // <- no error
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f(int x) {\n"
                        "    int y;\n"
                        "    if (x) y = do_something();\n"
                        "    if (!x) return 0;\n"
                        "    return y;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: int y;
        valueFlowUninit("int f(int x) {\n" // FP with ?:
                        "    int a;\n"
                        "    if (x)\n"
                        "        a = y;\n"
                        "    return x ? 2*a : 0;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f(int x, int y) {\n"
                        "    int a;\n"
                        "    if (x)\n"
                        "        a = y;\n"
                        "    return y ? 2*a : 3*a;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:5:18]: (warning) Uninitialized variable: a [uninitvar]\n", errout_str());

        ASSERT_THROW_INTERNAL(valueFlowUninit("void f() {\n" // Don't crash
                                              "    int a;\n"
                                              "    dostuff(\"ab\" cd \"ef\", x?a:z);\n" // <- No AST is created for ?
                                              "}"), UNKNOWN_MACRO);

        // Unknown => bail out..
        valueFlowUninit("void f(int x) {\n"
                        "    int i;\n"
                        "    if (a(x)) i = 0;\n"
                        "    if (b(x)) return;\n"
                        "    i++;\n" // <- no error if b(x) is always true when a(x) is false
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(int x) {\n"
                        "    int i;\n"
                        "    if (x) i = 0;\n"
                        "    while (condition) {\n"
                        "        if (x) i++;\n" // <- no error
                        "    }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(int x) {\n"
                        "    int i;\n"
                        "    if (x) i = 0;\n"
                        "    while (condition) {\n"
                        "        i++;\n"
                        "    }\n"
                        "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        valueFlowUninit("void f ( void ){\n" // #9313 - FN
                        " int *p;\n"
                        " int a[ 2 ] = { [ 0 ] = *p++, [ 1 ] = 1 };\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:26]: (error) Uninitialized variable: p [uninitvar]\n", errout_str());

        valueFlowUninit("void f(int height) {\n"
                        "    int a[11];\n"
                        "    int *p = a;\n"
                        "    int step = 2;\n"
                        "    for (int i = 0; i < (height * step); i += step)\n"
                        "        *p++ = 0;\n"
                        "    for (int i = 0; i < height; i++)\n"
                        "        if (a[i]) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(void) {\n"
                        "    char *c;\n"
                        "    char x;\n"
                        "    while (true) {\n"
                        "        c = &x;\n"
                        "        break;\n"
                        "    }\n"
                        "    ++c;\n"
                        "}", false);
        ASSERT_EQUALS("", errout_str());

        // #12030
        valueFlowUninit("int set(int *x);\n"
                        "void foo(bool a) {\n"
                        "    bool flag{0};\n"
                        "    int x;\n"
                        "    if (!a) {\n"
                        "        flag = set(&x);\n"
                        "    }\n"
                        "    if (!flag || x==3) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int set(int *x);\n"
                        "void foo(bool a) {\n"
                        "    bool flag{0};\n"
                        "    int x;\n"
                        "    if (!a) {\n"
                        "        flag = set(&x);\n"
                        "    }\n"
                        "    if (!flag && x==3) {}\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:8:18]: (warning) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("int do_something();\n" // #11983
                        "int set_st(int *x);\n"
                        "int bar();\n"
                        "void foo() {\n"
                        "    int x, y;\n"
                        "    int status = 1;\n"
                        "    if (bar() == 1) {\n"
                        "        status = 0;\n"
                        "    }\n"
                        "    if (status == 1) {\n"
                        "        status = set_st(&x);\n"
                        "    }\n"
                        "    for (int i = 0; status == 1 && i < x; i++) {\n"
                        "        if (do_something() == 0) {\n"
                        "            status = 0;\n"
                        "        }\n"
                        "    }\n"
                        "    if(status == 1 && x > 0){}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int h(bool, bool*);\n" // #11760
                        "int f(bool t) {\n"
                        "    int i = 0;\n"
                        "    bool b;\n"
                        "    if (t)\n"
                        "        g();\n"
                        "    if (i == 0)\n"
                        "        i = h(t, &b);\n"
                        "    if (i == 0 && b)\n"
                        "        i = h(t, &b);\n"
                        "    if (i == 0 && b)\n"
                        "        i = h(t, &b);\n"
                        "    return i;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void valueFlowUninit_uninitvar2()
    {
        // using uninit var
        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    x++;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        // extracttests.start: char str[10];
        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    str[x] = 0;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n" // #7736
                        "    int buf[12];\n"
                        "    printf (\"%d\", buf[0] );\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (error) Uninitialized variable: buf [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    int y = x & 3;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    int y = 3 & x;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    x = 3 + x;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    x = x;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        // extracttests.start: struct ABC {int a;};
        valueFlowUninit("void f() {\n"
                        "    struct ABC *abc;\n"
                        "    abc->a = 0;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: abc [uninitvar]\n", errout_str());

        valueFlowUninit("int f() {\n"
                        "    static int x;\n"
                        "    return ++x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f() {\n"
                        "    extern int x;\n"
                        "    return ++x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n" // #3926 - weird cast.
                        "    int x;\n"
                        "    *(((char *)&x) + 0) = 0;\n"
                        "}",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n" // #4737 - weird cast.
                        "    int x;\n"
                        "    do_something(&((char*)&x)[0], 1);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    char *p = (char*)&x + 1;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int i;\n"
                        "    i=f(), i!=2;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // using uninit var in condition
        valueFlowUninit("void f(void) {\n"
                        "    int x;\n"
                        "    if (x) { }\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    if (1 == (3 & x)) { }\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        // ?:
        valueFlowUninit("int f(int *ptr) {\n"
                        "    int a;\n"
                        "    int *p = ptr ? ptr : &a;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f(int a) {\n"
                        "    int x;\n"
                        "    if (a==3) { x=2; }\n"
                        "    y = (a==3) ? x : a;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // = ({ .. })
        valueFlowUninit("void f() {\n"
                        "    int x = ({ 1 + 2; });\n"
                        "    int y = 1 + (x ? y : y);\n"
                        "}");
        TODO_ASSERT_EQUALS("[test.cpp:3:14]: (error) Uninitialized variable: y [legacyUninitvar]\n", "", errout_str());

        // >> => initialization / usage
        {
            const char code[] = "void f() {\n"
                                "    int x;\n"
                                "    if (i >> x) { }\n"
                                "}";
            valueFlowUninit(code, true);
            ASSERT_EQUALS("", errout_str());

            valueFlowUninit(code, false);
            ASSERT_EQUALS("[test.c:3:14]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());
        }

        valueFlowUninit("void f() {\n"
                        "    int i, i2;\n"
                        "    strm >> i >> i2;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // unconditional initialization
        valueFlowUninit("int f() {\n"
                        "    int ret;\n"
                        "    if (a) { ret = 1; }\n"
                        "    else { {} ret = 2; }\n"
                        "    return ret;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f() {\n"
                        "    int ret;\n"
                        "    if (a) { ret = 1; }\n"
                        "    else { s=foo(1,{2,3},4); ret = 2; }\n"
                        "    return ret;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // conditional initialization
        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    if (y == 1) { x = 1; }\n"
                        "    else { if (y == 2) { x = 1; } }\n"
                        "    return x;\n"
                        "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized variable: x [legacyUninitvar]\n", "", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    if (y == 1) { x = 1; }\n"
                        "    else { if (y == 2) { x = 1; } }\n"
                        "    if (y == 3) { }\n" // <- ignore condition
                        "    return x;\n"
                        "}");
        TODO_ASSERT_EQUALS("[test.cpp:6:24]: (error) Uninitialized variable: x [legacyUninitvar]\n", "", errout_str());

        // initialization in condition
        valueFlowUninit("void f() {\n"
                        "    int a;\n"
                        "    if (init(&a)) { }\n"
                        "    a++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // return, break, continue, goto
        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    if (y == 1) { return; }\n"
                        "    else { x = 1; }\n"
                        "    return x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    if (y == 1) { return; }\n"
                        "    return x;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("int f(int x) {\n"
                        "    int ret;\n"
                        "    if (!x) {\n"
                        "        ret = -123;\n"
                        "        goto out1;\n"
                        "    }\n"
                        "    return 0;\n"
                        "out1:\n"
                        "out2:\n"
                        "    return ret;\n"
                        "}",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int i;\n"
                        "    if (x) {\n"
                        "        i = 1;\n"
                        "    } else {\n"
                        "        goto out;\n"
                        "    }\n"
                        "    i++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f() {\n"
                        "    int i,x;\n"
                        "    for (i=0;i<9;++i)\n"
                        "        if (foo) break;\n"
                        "    return x;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("int f() {\n"
                        "    int x;\n"
                        "    while (foo)\n"
                        "        if (bar) break;\n"
                        "    return x;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        // try/catch : don't warn about exception variable
        valueFlowUninit("void f() {\n"
                        "    try {\n"
                        "    } catch (CException* e) {\n"
                        "        trace();\n"
                        "        e->Delete();\n"
                        "    }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n" // #5347
                        "    try {\n"
                        "    } catch (const char* e) {\n"
                        "        A a = e;\n"
                        "    }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // exit
        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    if (y == 1) { exit(0); }\n"
                        "    else { x = 1; }\n"
                        "    return x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // strange code.. don't crash (#3415)
        valueFlowUninit("void foo() {\n"
                        "    int i;\n"
                        "    ({ if (0); });\n"
                        "    for_each(i) { }\n"
                        "}",
                        false);

        // if, if
        valueFlowUninit("void f(int a) {\n"
                        "    int i;\n"
                        "    if (a) i = 0;\n"
                        "    if (a) i++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int a,b=0;\n"
                        "    if (x) {\n"
                        "        if (y) {\n"
                        "            a = 0;\n"
                        "            b = 1;\n"
                        "        }\n"
                        "    }\n"
                        "    if (b) a++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int a=0, b;\n"
                        "    if (x) { }\n"
                        "    else { if (y==2) { a=1; b=2; } }\n"
                        "    if (a) { ++b; }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("static void f(int x, int y) {\n"
                        "    int a;\n"
                        "    if (x == 0) { a = y; }\n"
                        "    if (x == 0 && (a == 1)) { }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("static void f() {\n"
                        "    int a=0, b;\n"
                        "    if (something) { a = dostuff(&b); }\n"
                        "    if (!a || b) { }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("static void f(int x, int y) {\n"
                        "    int a;\n"
                        "    if (x == 0 && (a == 1)) { }\n"
                        "}",
                        true);
        ASSERT_EQUALS("[test.cpp:3:20]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int a;\n"
                        "    if (x) { a = 0; }\n"
                        "    if (x) { if (y) { a++; } }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int a;\n"
                        "    if (x) { a = 0; }\n"
                        "    if (x) { if (y) { } else { a++; } }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    if (x) ab = getAB();\n"
                        "    else ab.a = 0;\n"
                        "    if (ab.a == 1) b = ab.b;\n"
                        "}",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f(void) {\n"
                        "    int a;\n"
                        "    int i;\n"
                        "    if (x) { noreturn(); }\n"
                        "    else { i = 0; }\n"
                        "    if (i==1) { a = 0; }\n"
                        "    else { a = 1; }\n"
                        "    return a;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f(int a) {\n" // #4560
                        "    int x = 0, y;\n"
                        "    if (a) x = 1;\n"
                        "    else return 0;\n"
                        "    if (x) y = 123;\n" // <- y is always initialized
                        "    else {}\n"
                        "    return y;\n"
                        "}");
        TODO_ASSERT_EQUALS("", "[test.cpp:5:9] -> [test.cpp:7:12]: (warning) Uninitialized variable: y [uninitvar]\n", errout_str());

        valueFlowUninit("int f(int a) {\n" // #6583
                        "    int x;\n"
                        "    if (a < 2) exit(1);\n"
                        "    else if (a == 2) x = 0;\n"
                        "    else exit(2);\n"
                        "    return x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f(int a) {\n" // #4560
                        "    int x = 1, y;\n"
                        "    if (a) x = 0;\n"
                        "    else return 0;\n"
                        "    if (x) {}\n"
                        "    else y = 123;\n" // <- y is always initialized
                        "    return y;\n"
                        "}");
        TODO_ASSERT_EQUALS("", "[test.cpp:5:9] -> [test.cpp:7:12]: (warning) Uninitialized variable: y [uninitvar]\n", errout_str());

        valueFlowUninit("void f(int x) {\n" // #3948
                        "  int value;\n"
                        "  if (x !=-1)\n"
                        "    value = getvalue();\n"
                        "  if (x == -1 || value > 300) {}\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("enum t_err { ERR_NONE, ERR_BAD_ARGS };\n" // #9649
                        "struct box_t { int value; };\n"
                        "int init_box(box_t *p, int v);\n"
                        "\n"
                        "void foo(int ret) {\n"
                        "    box_t box2;\n"
                        "    if (ret == ERR_NONE)\n"
                        "        ret = init_box(&box2, 20);\n"
                        "    if (ret == ERR_NONE)\n"
                        "        z = x + box2.value;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(int x) {\n"
                        "  int value;\n"
                        "  if (x == 32)\n"
                        "    value = getvalue();\n"
                        "  if (x == 1)\n"
                        "    v = value;\n"
                        "}");
        TODO_ASSERT_EQUALS("[test.cpp:6:9]: (error) Uninitialized variable: value [legacyUninitvar]\n", "", errout_str());

        valueFlowUninit("void f(int x) {\n"
                        "  int value;\n"
                        "  if (x == 32)\n"
                        "    value = getvalue();\n"
                        "  if (x == 32) {}\n"
                        "  else v = value;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:6:12]: (warning) Uninitialized variable: value [uninitvar]\n", errout_str());

        valueFlowUninit("static int x;" // #4773
                        "int f() {\n"
                        "    int y;\n"
                        "    if (x) g();\n"
                        "    if (x) y = 123;\n"
                        "    else y = 456;\n"
                        "    return y;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("static int x;" // #4773
                        "int f() {\n"
                        "    int y;\n"
                        "    if (!x) g();\n"
                        "    if (x) y = 123;\n"
                        "    else y = 456;\n"
                        "    return y;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(int a) {\n"
                        "  int x;\n"
                        "  if (a) x=123;\n"
                        "  if (!a) {\n"
                        "    if (!a) {}\n"
                        "    else if (x) {}\n"
                        "  }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // asm
        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    asm();\n"
                        "    x++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // sizeof / typeof / offsetof / etc
        valueFlowUninit("void f() {\n"
                        "    int i;\n"
                        "    sizeof(i+1);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int i;\n"
                        "    if (100 == sizeof(i+1));\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    struct ABC *abc;\n"
                        "    int i = ARRAY_SIZE(abc.a);"
                        "}");
        // FP ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int *abc;\n"
                        "    typeof(*abc);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    struct ABC *abc;\n"
                        "    return do_something(typeof(*abc));\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    A *a;\n"
                        "    a = malloc(sizeof(*a));\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // &
        valueFlowUninit("void f() {\n" // #4426 - address of uninitialized variable
                        "    int a,b;\n"
                        "    if (&a == &b);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n" // #4439 - cast address of uninitialized variable
                        "    int a;\n"
                        "    x((LPARAM)(RECT*)&a);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit(
            "int main() {\n"
            "    int done;\n"
            "    dostuff(1, (AuPointer) &done);\n" // <- It is not conclusive if the "&" is a binary or unary operator. Bailout.
            "}");
        TODO_ASSERT_EQUALS("", "[test.cpp:3:29]: (error) Uninitialized variable: done [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n" // #4778 - cast address of uninitialized variable
                        "    long a;\n"
                        "    &a;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n" // #4717 - ({})
                        "    int a = ({ long b = (long)(123); 2 + b; });\n"
                        "}",
                        false);
        ASSERT_EQUALS("", errout_str());
    }

    void valueFlowUninit_functioncall() {

        // #12462 - pointer data is not read
        valueFlowUninit("struct myst { int a; };\n"
                        "void bar(const void* p) {}\n"
                        "void foo() {\n"
                        "  struct myst item;\n"
                        "  bar(&item);\n"
                        "}", false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct myst { int a; };\n"
                        "void bar(const void* p) { a = (p != 0); }\n"
                        "void foo() {\n"
                        "  struct myst item;\n"
                        "  bar(&item);\n"
                        "  a = item.a;\n" // <- item.a is not initialized
                        "}", false);
        ASSERT_EQUALS("[test.c:6:12]: (error) Uninitialized variable: item.a [uninitvar]\n", errout_str());

        valueFlowUninit("struct myst { int a; };\n"
                        "void bar(struct myst* p) { p->a = 0; }\n"
                        "void foo() {\n"
                        "  struct myst item;\n"
                        "  bar(&item);\n"
                        "  a = item.a;\n"
                        "}", false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(int& r) {}\n" // #12536
                        "void g() {\n"
                        "    int i;\n"
                        "    f(i);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f(int& i, int j, int k) {\n" // #12514
                        "    if (k)\n"
                        "        i = 2;\n"
                        "    return i + j;\n"
                        "}\n"
                        "int main() {\n"
                        "    int i;\n"
                        "    return f(i, 1, 0);\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:8:14] -> [test.cpp:4:12]: (warning) Uninitialized variable: i [uninitvar]\n", errout_str());

        valueFlowUninit("int f(int& i, int k) {\n"
                        "    if (k)\n"
                        "        i = 2;\n"
                        "    return i;\n"
                        "}\n"
                        "int main() {\n"
                        "    int i;\n"
                        "    return f(i, 0);\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:8:14] -> [test.cpp:4:12]: (warning) Uninitialized variable: i [uninitvar]\n", errout_str());
    }

    void uninitStructMember() { // struct members
        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    int a = ab.a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    AB ab;\n"
                       "    int a = ab.a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab.a = ab.a + 1;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void do_something(const struct AB ab);\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab.a = 0;\n"
                       "    do_something(ab);\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:6:18]: (error) Uninitialized struct member: ab.b [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n" // #4760
                       "void do_something(int a);\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    do_something(ab.a);\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:5:18]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void do_something(const struct AB &ab) { a = ab.a; }\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab.a = 0;\n"
                       "    do_something(ab);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    int a = ab.a;\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:4:13]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    AB ab1;\n"
                       "    AB ab2 = { ab1.a, 0 };\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:20]: (error) Uninitialized struct member: ab1.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    buf[ab.a] = 0;\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:4:9]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab.a = 1;\n"
                       "    x = ab;\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:5:9]: (error) Uninitialized struct member: ab.b [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab.a = 1;\n"
                       "    x = *(&ab);\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:5:12]: (error) Uninitialized struct member: ab.b [uninitStructMember]\n", errout_str());

        checkUninitVar("void f(void) {\n"
                       "    struct AB ab;\n"
                       "    int x;\n"
                       "    ab.a = (addr)&x;\n"
                       "    dostuff(&ab,0);\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct Element {\n"
                       "    static void f() { }\n"
                       "};\n"
                       "void test() {\n"
                       "    Element *element; element->f();\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (error) Uninitialized variable: element [legacyUninitvar]\n", errout_str());

        checkUninitVar("struct Element {\n"
                       "    static void f() { }\n"
                       "};\n"
                       "void test() {\n"
                       "    Element *element; (*element).f();\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:25]: (error) Uninitialized variable: element [legacyUninitvar]\n", errout_str());

        checkUninitVar("struct Element {\n"
                       "    static int v;\n"
                       "};\n"
                       "void test() {\n"
                       "    Element *element; element->v;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (error) Uninitialized variable: element [legacyUninitvar]\n", errout_str());

        checkUninitVar("struct Element {\n"
                       "    static int v;\n"
                       "};\n"
                       "void test() {\n"
                       "    Element *element; (*element).v;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:25]: (error) Uninitialized variable: element [legacyUninitvar]\n", errout_str());

        checkUninitVar("struct Element {\n"
                       "    void f() { }\n"
                       "};\n"
                       "void test() {\n"
                       "    Element *element; element->f();\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (error) Uninitialized variable: element [legacyUninitvar]\n", errout_str());

        checkUninitVar("struct Element {\n"
                       "    void f() { }\n"
                       "};\n"
                       "void test() {\n"
                       "    Element *element; (*element).f();\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:25]: (error) Uninitialized variable: element [legacyUninitvar]\n", errout_str());

        checkUninitVar("struct Element {\n"
                       "    int v;\n"
                       "};\n"
                       "void test() {\n"
                       "    Element *element; element->v;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (error) Uninitialized variable: element [legacyUninitvar]\n", errout_str());

        checkUninitVar("struct Element {\n"
                       "    int v;\n"
                       "};\n"
                       "void test() {\n"
                       "    Element *element; (*element).v;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:25]: (error) Uninitialized variable: element [legacyUninitvar]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"  // pass struct member by address
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    assign(&ab.a, 0);\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct Cstring { char *text; int size, alloc; };\n"
                       "int maybe();\n"
                       "void f() {\n"
                       "    Cstring res;\n"
                       "    if ( ! maybe() ) return;\n"  // <- fp goes away if this is removed
                       "    ( ((res).text = (void*)0), ((res).size = (res).alloc = 0) );\n"  // <- fp goes away if parentheses are removed
                       "}");
        ASSERT_EQUALS("", errout_str());

        {
            constexpr char argDirectionsTestXmlData[] = "<?xml version=\"1.0\"?>\n"
                                                        "<def>\n"
                                                        "  <function name=\"uninitvar_funcArgInTest\">\n"
                                                        "    <arg nr=\"1\" direction=\"in\"/>\n"
                                                        "  </function>\n"
                                                        "  <function name=\"uninitvar_funcArgOutTest\">\n"
                                                        "    <arg nr=\"1\" direction=\"out\"/>\n"
                                                        "  </function>\n"
                                                        "</def>";
            const Settings s = settingsBuilder(settings).libraryxml(argDirectionsTestXmlData).build();

            checkUninitVar("struct AB { int a; };\n"
                           "void f(void) {\n"
                           "    struct AB ab;\n"
                           "    uninitvar_funcArgInTest(&ab);\n"
                           "    x = ab;\n"
                           "}\n", s, false);
            ASSERT_EQUALS("[test.c:5:9]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

            checkUninitVar("struct AB { int a; };\n"
                           "void f(void) {\n"
                           "    struct AB ab;\n"
                           "    uninitvar_funcArgOutTest(&ab);\n"
                           "    x = ab;\n"
                           "}\n", s, false);
            ASSERT_EQUALS("", errout_str());
        }

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void do_something(const struct AB ab);\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab.a = 0;\n"
                       "    ab.b = 0;\n"
                       "    do_something(ab);\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        {
            checkUninitVar("struct AB { char a[10]; };\n"
                           "void f(void) {\n"
                           "    struct AB ab;\n"
                           "    strcpy(ab.a, STR);\n"
                           "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("struct AB { unsigned char a[10]; };\n" // #8999 - cast
                           "void f(void) {\n"
                           "    struct AB ab;\n"
                           "    strcpy((char *)ab.a, STR);\n"
                           "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("struct AB { char a[10]; };\n"
                           "void f(void) {\n"
                           "    struct AB ab;\n"
                           "    strcpy(x, ab.a);\n"
                           "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
            TODO_ASSERT_EQUALS("[test.c:4:16]: (error) Uninitialized variable: ab.a [uninitvar]\n", "", errout_str());

            checkUninitVar("struct AB { int a; };\n"
                           "void f(void) {\n"
                           "    struct AB ab;\n"
                           "    dosomething(ab.a);\n"
                           "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
            ASSERT_EQUALS("", errout_str());
        }

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void do_something(const struct AB ab);\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab = getAB();\n"
                       "    do_something(ab);\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        {
            // #6769 - calling method that might assign struct members
            checkUninitVar("struct AB { int a; int b; void set(); };\n"
                           "void f(void) {\n"
                           "    struct AB ab;\n"
                           "    ab.set();\n"
                           "    x = ab;\n"
                           "}");
            ASSERT_EQUALS("", errout_str());

            checkUninitVar("struct AB { int a; int get() const; };\n"
                           "void f(void) {\n"
                           "    struct AB ab;\n"
                           "    ab.get();\n"
                           "    x = ab;\n"
                           "}");
            ASSERT_EQUALS("[test.cpp:5:9]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

            checkUninitVar("struct AB { int a; void dostuff() {} };\n"
                           "void f(void) {\n"
                           "    struct AB ab;\n"
                           "    ab.dostuff();\n"
                           "    x = ab;\n"
                           "}");
            TODO_ASSERT_EQUALS("error", "", errout_str());
        }

        checkUninitVar("struct AB { int a; struct { int b; int c; } s; };\n"
                       "void do_something(const struct AB ab);\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab.a = 1;\n"
                       "    ab.s.b = 2;\n"
                       "    ab.s.c = 3;\n"
                       "    do_something(ab);\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct conf {\n"
                       "    char x;\n"
                       "};\n"
                       "\n"
                       "void do_something(struct conf ant_conf);\n"
                       "\n"
                       "void f(void) {\n"
                       "   struct conf c;\n"
                       "   initdata(&c);\n"
                       "   do_something(c);\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct PIXEL {\n"
                       "    union  {\n"
                       "        struct { unsigned char red,green,blue,alpha; };\n"
                       "        unsigned int color;\n"
                       "    };\n"
                       "};\n"
                       "\n"
                       "unsigned char f() {\n"
                       "    struct PIXEL p1;\n"
                       "    p1.color = 255;\n"
                       "    return p1.red;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "int f() {\n"
                       "  struct AB *ab;\n"
                       "  for (i = 1; i < 10; i++) {\n"
                       "    if (condition && (ab = getab()) != NULL) {\n"
                       "      a = ab->a;\n"
                       "    }\n"
                       "  }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "int f(int x) {\n"
                       "  struct AB *ab;\n"
                       "  if (x == 0) {\n"
                       "    ab = getab();\n"
                       "  }\n"
                       "  if (x == 0 && (ab != NULL || ab->a == 0)) { }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct A { int *x; };\n" // declarationId is 0 for "delete"
                       "void foo(void *info, void*p);\n"
                       "void bar(void) {\n"
                       "  struct A *delete = 0;\n"
                       "  foo( info, NULL );\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct ABC { int a; int b; int c; };\n"
                       "void foo(int x, const struct ABC *abc);\n"
                       "void bar(void) {\n"
                       "  struct ABC abc;\n"
                       "  foo(123, &abc);\n"
                       "  return abc.b;\n"
                       "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Uninitialized struct member: abc.a\n"
                           "[test.cpp:5]: (error) Uninitialized struct member: abc.b\n"
                           "[test.cpp:5]: (error) Uninitialized struct member: abc.c\n",
                           "[test.cpp:6:10]: (error) Uninitialized struct member: abc.b [uninitStructMember]\n",
                           errout_str());

        checkUninitVar("struct ABC { int a; int b; int c; };\n"
                       "void foo() {\n"
                       "  struct ABC abc;\n"
                       "  dostuff((uint32_t *)&abc.a);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(void) {\n"
                       "    struct tm t;\n"
                       "    t.tm_year = 123;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // return
        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab.a = 0;\n"
                       "    return ab.b;\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:5:12]: (error) Uninitialized struct member: ab.b [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    ab.a = 0;\n"
                       "    return ab.a;\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct S { int a; int b; };\n" // #8299
                       "void f(void) {\n"
                       "    struct S s;\n"
                       "    s.a = 0;\n"
                       "    return s;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized struct member: s.b [uninitStructMember]\n", errout_str());

        checkUninitVar("struct S { int a; int b; };\n" // #9810
                       "void f(void) {\n"
                       "    struct S s;\n"
                       "    return s.a ? 1 : 2;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Uninitialized struct member: s.a [uninitStructMember]\n", errout_str());

        // checkIfForWhileHead
        checkUninitVar("struct FRED {\n"
                       "    int a;\n"
                       "    int b;\n"
                       "};\n"
                       "\n"
                       "void f(void) {\n"
                       "   struct FRED fred;\n"
                       "   fred.a = do_something();\n"
                       "   if (fred.a == 0) { }\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct FRED {\n"
                       "    int a;\n"
                       "    int b;\n"
                       "};\n"
                       "\n"
                       "void f(void) {\n"
                       "   struct FRED fred;\n"
                       "   fred.a = do_something();\n"
                       "   if (fred.b == 0) { }\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:9:8]: (error) Uninitialized struct member: fred.b [uninitStructMember]\n", errout_str());

        checkUninitVar("struct Fred { int a; };\n"
                       "void f() {\n"
                       "    struct Fred fred;\n"
                       "    if (fred.a==1) {}\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:4:9]: (error) Uninitialized struct member: fred.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct S { int n; int m; };\n"
                       "void f(void) {\n"
                       " struct S s;\n"
                       " for (s.n = 0; s.n <= 10; s.n++) { }\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void test2() {\n"
                       "  struct { char type; } s_d;\n"
                       "  if (foo(&s_d.type)){}\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // for
        checkUninitVar("struct AB { int a; };\n"
                       "void f() {\n"
                       "    struct AB ab;\n"
                       "    while (x) { clear(ab); z = ab.a; }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct AB { int a; };\n"
                       "void f() {\n"
                       "    struct AB ab;\n"
                       "    while (x) { ab.a = ab.a + 1; }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:24]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; };\n"
                       "void f() {\n"
                       "    struct AB ab;\n"
                       "    while (x) { init(&ab); z = ab.a; }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // address of member
        checkUninitVar("struct AB { int a[10]; int b; };\n"
                       "void f() {\n"
                       "    struct AB ab;\n"
                       "    int *p = ab.a;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Reference
        checkUninitVar("struct A { int x; };\n"
                       "void foo() {\n"
                       "  struct A a;\n"
                       "  int& x = a.x;\n"
                       "  x = 0;\n"
                       "  return a.x;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // non static data-member initialization
        checkUninitVar("struct AB { int a=1; int b; };\n"
                       "void f(void) {\n"
                       "    struct AB ab;\n"
                       "    int a = ab.a;\n"
                       "    int b = ab.b;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Uninitialized struct member: ab.b [uninitStructMember]\n", errout_str());

        // STL class member
        checkUninitVar("struct A {\n"
                       "    std::map<int, int> m;\n"
                       "    int i;\n"
                       "};\n"
                       "void foo() {\n"
                       "    A a;\n"
                       "    x = a.m;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Unknown type (C++)
        checkUninitVar("struct A {\n"
                       "    C m;\n"
                       "    int i;\n"
                       "};\n"
                       "void foo() {\n"
                       "    A a;\n"
                       "    x = a.m;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // Unknown type (C)
        checkUninitVar("struct A {\n"
                       "    C m;\n"
                       "    int i;\n"
                       "};\n"
                       "void foo() {\n"
                       "    A a;\n"
                       "    x = a.m;\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:7:9]: (error) Uninitialized struct member: a.m [uninitStructMember]\n", errout_str());

        // Type with constructor
        checkUninitVar("class C { C(); }\n"
                       "struct A {\n"
                       "    C m;\n"
                       "    int i;\n"
                       "};\n"
                       "void foo() {\n"
                       "    A a;\n"
                       "    x = a.m;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // #11002
        checkUninitVar("struct S { char *p; int len; };\n"
                       "void f() {\n"
                       "    S s;\n"
                       "    s.p = nullptr;\n"
                       "    char* q = (s).p;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        // if with flag
        checkUninitVar("struct AB { int a; int b; };\n"
                       "int f(int x) {\n"
                       "  struct AB ab;\n"
                       "  int flag = 0;\n"
                       "  if (x == 0) {\n"
                       "    flag = dostuff(&ab);\n"
                       "  }\n"
                       "  if (flag) {\n"
                       "    a = ab.a;\n"
                       "  }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct S { int x; };\n"
                       "S h() {\n"
                       "    S s;\n"
                       "    S& r = s;\n"
                       "    r.x = 0;\n"
                       "    return s;\n"
                       "}\n"
                       "S i() {\n"
                       "    S s;\n"
                       "    S& r{ s };\n"
                       "    r.x = 0;\n"
                       "    return s;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct S { int i; };\n" // #12142
                       "int f() {\n"
                       "    S s;\n"
                       "    int S::* p = &S::i;\n"
                       "    s.*p = 123;\n"
                       "    return s.i;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct S { int a; };\n" // #13848
                       "int f(std::istream& is) {\n"
                       "    S s;\n"
                       "    is >> s;\n"
                       "    return s.a;\n"
                       "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar2_while() {
        // extracttests.start: int a;

        // for, while
        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    while (a) {\n"
                       "        x = x + 1;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    do {\n"
                       "        x = x + 1;\n"
                       "    } while (a);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    for (int x = x; x < 10; x++) {}\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        // extracttests.start: struct Element{Element*Next();};
        checkUninitVar("void f() {\n"
                       "    for (Element *ptr3 = ptr3->Next(); ptr3; ptr3 = ptr3->Next()) {}\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:2:26]: (error) Uninitialized variable: ptr3 [legacyUninitvar]\n", errout_str());

        // extracttests.start: int a;
        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    while (a) {\n"
                       "        init(&x);\n"
                       "        x++;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    while (a) {\n"
                       "        if (b) x++;\n"
                       "        else x = 0;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    for (int i = 0; i < 10; i += x) {\n"
                       "        x = y;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int x;\n"
                       "    for (int i = 0; i < 10; i += x) { }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:34]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("int f() {\n"
                       "    int i;\n"
                       "    for (i=0;i<9;++i)\n"
                       "        if (foo()) return i;\n"
                       "    return 9;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int i;\n"
                       "    do {} while (!getvalue(&i));\n"
                       "    i++;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int f(void) {\n"
                       "   int x;\n"
                       "   while (a()) {\n"  // <- condition must always be true or there will be problem
                       "       if (b()) {\n"
                       "           x = 1;\n"
                       "           break;"
                       "       }\n"
                       "   }\n"
                       "   return x;\n"
                       "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        checkUninitVar("int f(void) {\n"
                       "   int x;\n"
                       "   while (a()) {\n"
                       "       if (b() && (x=1)) {\n"
                       "           return x;\n"
                       "       }\n"
                       "   }\n"
                       "   return 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: void do_something(int);
        checkUninitVar("void f(void) {\n"
                       "   int x;\n"
                       "   for (;;) {\n"
                       "       int a = x+1;\n"
                       "       do_something(a);\n"
                       "   }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:16]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());

        checkUninitVar("struct AB {int a; int b;};\n"
                       "void f(void) {\n"
                       "   struct AB ab;\n"
                       "   while (true) {\n"
                       "       int a = 1+ab.a;\n"
                       "       do_something(a);\n"
                       "   }\n"
                       "}\n", dinit(CheckUninitVarOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:5:18]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

        checkUninitVar("void f(int i) {\n" // #4569 fp
                       "    float *buffer;\n"
                       "    if(i>10) buffer = f;\n"
                       "    if(i>10) {\n"
                       "        for (int i=0;i<10;i++)\n"
                       "            buffer[i] = 0;\n" // <- fp
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(){\n" // #4519 - fp: inline assembler in loop
                       "    int x;\n"
                       "    for (int i = 0; i < 10; i++) {\n"
                       "        asm(\"foo\");\n"
                       "        if (x & 0xf1) { }\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("static void f(void) {\n"
                       "    struct ABC *abc;\n"
                       "    for (i = 0; i < 10; i++)\n"
                       "        x += sizeof(*abc);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(void) {\n" // #4879
                       "    int i;\n"
                       "    while (x) {\n"
                       "        for (i = 0; i < 5; i++)\n"
                       "            a[i] = b[i];\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(void) {\n" // #5658
                       "    struct Foo *foo;\n"
                       "    while (true) {\n"
                       "            foo = malloc(sizeof(*foo));\n"
                       "            foo->x = 0;\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f(void) {\n"
                       "  int i;\n"
                       "  while (x) {\n"
                       "    for (i=0,y=i;;){}\n"
                       "  }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "  char *p = (char *)malloc(256);\n"
                       "  while(*p && *p == '_')\n"
                       "    p++;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Memory is allocated but not initialized: *p [uninitdata]\n", errout_str());

        // #6646 - init in for loop
        checkUninitVar("void f() {\n" // No FP
                       "  for (int i;;i++)\n"
                       "    dostuff(&i);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: int a;
        checkUninitVar("void f() {\n" // No FN
                       "  for (int i;;i++)\n"
                       "    a=i;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (error) Uninitialized variable: i [legacyUninitvar]\n", errout_str());

        checkUninitVar("namespace N {\n" // #7377
                       "    template<typename T>\n"
                       "    class C {};\n"
                       "    using V = class C<void>;\n"
                       "}\n"
                       "int f() {\n"
                       "    int r = 0;\n"
                       "    for (int x; x < 4; x++)\n"
                       "        r += x;\n"
                       "    return r;\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:8:17]: (error) Uninitialized variable: x [legacyUninitvar]\n", errout_str());
    }

    void uninitvar2_4494() {
        checkUninitVar("namespace N1 {\n"
                       "    class Fred {\n"
                       "    public:\n"
                       "        static void f1(char *p) { *p = 0; }\n"
                       "    };\n"
                       "    void fa(void) { char *p; Fred::f1(p); }\n"
                       "    void fb(void) { char *p; Fred::f2(p); }\n"
                       "    void fc(void) { char *p; ::N1::Fred::f1(p); }\n"
                       "    void fd(void) { char *p; ::N1::Fred::f2(p); }\n"
                       "}\n"
                       "namespace N2 {\n"
                       "    static void f1(char *p) { *p = 0; }\n"
                       "    void fa(void) { char *p; f1(p); }\n"
                       "    void fb(void) { char *p; f2(p); }\n"
                       "    void fc(void) { char *p; N1::Fred::f1(p); }\n"
                       "    void fd(void) { char *p; N1::Fred::f2(p); }\n"
                       "    void fe(void) { char *p; ::N1::Fred::f1(p); }\n"
                       "    void ff(void) { char *p; ::N1::Fred::f2(p); }\n"
                       "    void fg(void) { char *p; Foo::f1(p); }\n"
                       "    void fh(void) { char *p; Foo::f2(p); }\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:6:39]: (error) Uninitialized variable: p [legacyUninitvar]\n"
                      "[test.cpp:8:45]: (error) Uninitialized variable: p [legacyUninitvar]\n"
                      "[test.cpp:13:33]: (error) Uninitialized variable: p [legacyUninitvar]\n"
                      "[test.cpp:15:43]: (error) Uninitialized variable: p [legacyUninitvar]\n"
                      "[test.cpp:17:45]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        checkUninitVar("class Fred {\n"
                       "public:\n"
                       "    void f1(char *p) { *p = 0; }\n"
                       "};\n"
                       "Fred fred;\n"
                       "void f(void) {\n"
                       "    char *p;\n"
                       "    fred.f1(p);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:8:13]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());

        checkUninitVar("class Fred {\n"
                       "public:\n"
                       "    class Wilma {\n"
                       "    public:\n"
                       "        class Barney {\n"
                       "        public:\n"
                       "            class Betty {\n"
                       "            public:\n"
                       "                void f1(char *p) { *p = 0; }\n"
                       "            };\n"
                       "            Betty betty;\n"
                       "        };\n"
                       "        Barney barney;\n"
                       "    };\n"
                       "    Wilma wilma;\n"
                       "};\n"
                       "Fred fred;\n"
                       "void f(void) {\n"
                       "    char *p;\n"
                       "    fred.wilma.barney.betty.f1(p);\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:20:32]: (error) Uninitialized variable: p [legacyUninitvar]\n", errout_str());
    }

    void uninitvar2_malloc() {
        checkUninitVar("int f() {\n"
                       "    int *p = (int*)malloc(40);\n"
                       "    return *p;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Memory is allocated but not initialized: p [uninitdata]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    int *p = (int*)malloc(40);\n"
                       "    int var = *p;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Memory is allocated but not initialized: p [uninitdata]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "int f() {\n"
                       "    struct AB *ab = (AB*)malloc(sizeof(struct AB));\n"
                       "    return ab->a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Memory is allocated but not initialized: ab [uninitdata]\n"
                      "[test.cpp:4:12]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n",
                      errout_str());

        checkUninitVar("struct t_udf_file {  int dir_left; };\n"
                       "\n"
                       "void f() {\n"
                       "  struct t_udf_file *newf;\n"
                       "  newf = malloc(sizeof(*newf));\n"
                       "  if (!newf) {};\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    char *s = malloc(100);\n"
                       "    if (s != NULL) { }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    char *p = malloc(100);\n"
                       "    p || assert_failed();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    char *p = malloc(100);\n"
                       "    x = p;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("int* f() {\n"
                       "    int *p = (int*)malloc(40);\n"
                       "    return p;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // function parameter (treat it as initialized until malloc is used)
        checkUninitVar("int f(int *p) {\n"
                       "    if (*p == 1) {}\n" // no error
                       "    p = (int*)malloc(256);\n"
                       "    return *p;\n" // error
                       "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Memory is allocated but not initialized: p [uninitdata]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "int f(struct AB *ab) {\n"
                       "    if (ab->a == 1) {}\n" // no error
                       "    ab = (AB*)malloc(sizeof(struct AB));\n"
                       "    return ab->a;\n" // error
                       "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized struct member: ab.a [uninitStructMember]\n", errout_str());

        checkUninitVar("struct AB { int a; int b; };\n"
                       "void do_something(struct AB *ab);\n" // unknown function
                       "void f() {\n"
                       "    struct AB *ab = (AB*)malloc(sizeof(struct AB));\n"
                       "    do_something(ab);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        // analysis failed. varid 0.
        checkUninitVar("void *vlc_custom_create (vlc_object_t *parent, size_t length, const char *typename) {\n"
                       "  assert (length >= sizeof (vlc_object_t));\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_ternaryexpression() { // #4683
        checkUninitVar("struct B { int asd; };\n"
                       "int f() {\n"
                       "    int a=0;\n"
                       "    struct B *b;\n"
                       "    if (x) {\n"
                       "        a = 1;\n"
                       "        b = p;\n"
                       "    }\n"
                       "    return a ? b->asd : 0;\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_rangeBasedFor() {
        checkUninitVar("void function(Entry& entry) {\n" // #7078
                       "    for (auto* expr : entry.exprs) {\n"
                       "        expr->operate();\n"
                       "        expr->operate();\n"
                       "    }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int *item;\n"
                       "    for (item: itemList) {}\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    int buf[10];\n"
                       "    for (int &i: buf) { i = 0; }\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_static() { // #8734
        checkUninitVar("struct X { "
                       "  typedef struct { int p; } P_t; "
                       "  static int arr[]; "
                       "}; "
                       "int X::arr[] = {42}; "
                       "void f() { "
                       "  std::vector<X::P_t> result; "
                       "  X::P_t P; "
                       "  P.p = 0; "
                       "  result.push_back(P); "
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_configuration() {
        const Settings s = settingsBuilder(settings).checkLibrary().build();

        checkUninitVar("int f() {\n"
                       "    int i, j;\n"
                       "    do {\n"
                       "        i = 0;\n"
                       "        return i;\n"
                       "    } while (0);\n"
                       "}\n", s);
        ASSERT_EQUALS("", errout_str());
    }

    void checkExpr() {
        checkUninitVar("struct AB { int a; int b; };\n"
                       "void f() {\n"
                       "    struct AB *ab = (struct AB*)calloc(1, sizeof(*ab));\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void trac_4871() { // #4871
        checkUninitVar("void pickup(int a) {\n"
                       "bool using_planner_action;\n"
                       "if (a)   {\n"
                       "  using_planner_action = false;\n"
                       "}\n"
                       "else {\n"
                       "  try\n"
                       "  {}\n"
                       "  catch (std::exception &ex) {\n"
                       "    return;\n"
                       "  }\n"
                       "  using_planner_action = true;\n"
                       "}\n"
                       "if (using_planner_action) {}\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void syntax_error() { // Ticket #5073
        const char code[] = "struct flex_array {};\n"
                            "struct cgroup_taskset {};\n"
                            "void cgroup_attach_task() {\n"
                            "  struct flex_array *group;\n"
                            "  struct cgroup_taskset tset = { };\n"
                            "  do { } while_each_thread(leader, tsk);\n"
                            "}";
        ASSERT_THROW_INTERNAL(checkUninitVar(code), SYNTAX);
    }

    void trac_5970() { // Ticket #5970
        checkUninitVar("void DES_ede3_ofb64_encrypt() {\n"
                       "  DES_cblock d;\n"
                       "  char *dp;\n"
                       "  dp=(char *)d;\n"
                       "  init(dp);\n"
                       "}", dinit(CheckUninitVarOptions, $.cpp = false));
        // Unknown type
        TODO_ASSERT_EQUALS("", "[test.c:4:14]: (error) Uninitialized variable: d [legacyUninitvar]\n", errout_str());
    }

#define ctu(...) ctu_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void ctu_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        CTU::FileInfo *ctu = CTU::getFileInfo(tokenizer);

        // Check code..
        std::list<Check::FileInfo*> fileInfo;
        Check& c = getCheck<CheckUninitVar>();
        fileInfo.push_back(c.getFileInfo(tokenizer, settings, ""));
        c.analyseWholeProgram(*ctu, fileInfo, settings, *this); // TODO: check result
        while (!fileInfo.empty()) {
            delete fileInfo.back();
            fileInfo.pop_back();
        }
        delete ctu;
    }

    void valueFlowUninitTest() {
        // #9735 - FN
        valueFlowUninit("typedef struct\n"
                        "{\n"
                        "    int x;\n"
                        "    unsigned int flag : 1;\n" // bit filed gets never initialized
                        "} status;\n"
                        "bool foo(const status * const s)\n"
                        "{\n"
                        "    return s->flag;\n" // << uninitvar
                        "}\n"
                        "void bar(const status * const s)\n"
                        "{\n"
                        "    if( foo(s) == 1) {;}\n"
                        "}\n"
                        "void f(void)\n"
                        "{\n"
                        "    status s;\n"
                        "    s.x = 42;\n"
                        "    bar(&s);\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:18:9] -> [test.cpp:12:13] -> [test.cpp:8:15]: (warning) Uninitialized variable: s->flag [uninitvar]\n", errout_str());

        // Ticket #2207 - False negative
        valueFlowUninit("void foo() {\n"
                        "    int a;\n"
                        "    b = c - a;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        valueFlowUninit("void foo() {\n"
                        "    int a;\n"
                        "    b = a - c;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        // Ticket #6455 - some compilers allow const variables to be uninitialized
        // extracttests.disable
        valueFlowUninit("void foo() {\n"
                        "    const int a;\n"
                        "    b = c - a;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());
        // extracttests.enable

        valueFlowUninit("void foo() {\n"
                        "    int *p;\n"
                        "    realloc(p,10);\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Uninitialized variable: p [uninitvar]\n", errout_str());

        valueFlowUninit("void foo() {\n" // #5240
                        "    char *p = malloc(100);\n"
                        "    char *tmp = realloc(p,1000);\n"
                        "    if (!tmp) free(p);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void foo() {\n"
                        "    int *p = NULL;\n"
                        "    realloc(p,10);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "  int x;\n"
                        "  switch (x) {}\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("int f() {\n"
                        "  int x;\n"
                        "  init(x);\n"
                        "  return x;\n" // TODO: inconclusive ?
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n" // #8172
                        "  char **x;\n"
                        "  if (2 < sizeof(*x)) {}\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void foo() {\n" // #5259 - False negative
                        "    int a;\n"
                        "    int x[] = {a,2};\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        valueFlowUninit("void foo()\n"
                        "{\n"
                        "    int x;\n"
                        "    int *y = &x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void foo()\n"
                        "{\n"
                        "    int *x;\n"
                        "    int *&y = x;\n"
                        "    y = nullptr;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void foo()\n"
                        "{\n"
                        "    int x = xyz::x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f()\n"
                        "{\n"
                        "    extern int a;\n"
                        "    a++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("static void foo()\n"
                        "{\n"
                        "    int x, y;\n"
                        "    x = (y = 10);\n"
                        "    int z = y * 2;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("static void foo() {\n"
                        "    int x, y;\n"
                        "    x = ((y) = 10);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("static void foo()\n"
                        "{\n"
                        "    Foo p;\n"
                        "    p.abcd();\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("static void foo()\n"
                        "{\n"
                        "    Foo p;\n"
                        "    int x = p.abcd();\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // struct
        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "  AB ab;\n"
                        "  AB *p = &ab;\n"
                        "  p->a = 1;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S {\n"
                        "    S& rIo;\n"
                        "    S(S&);\n"
                        "    void Write();\n"
                        "};\n"
                        "void foo(bool b, struct S &io) {\n"
                        "    S* p;\n"
                        "    if (b)\n"
                        "        p = new S(io);\n"
                        "    p->Write();\n"
                        "}");
        TODO_ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:10]: (warning) Uninitialized variable: p\n",
                           "[test.cpp:8:9] -> [test.cpp:10:5]: (warning) Uninitialized variable: p.rIo [uninitvar]\n",
                           errout_str());

        // Unknown types
        {
            valueFlowUninit("void a()\n"
                            "{\n"
                            "    A ret;\n"
                            "    return ret;\n"
                            "}");
            ASSERT_EQUALS("", errout_str());

            // #3916 - avoid false positive
            valueFlowUninit("void f(float x) {\n"
                            "  union lf { long l; float f; } u_lf;\n"
                            "  float hx = (u_lf.f = (x), u_lf.l);\n"
                            "}");
            ASSERT_EQUALS("", errout_str());
        }

        valueFlowUninit("void a()\n"
                        "{\n"
                        "    int x[10];\n"
                        "    int *y = x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void a()\n"
                        "{\n"
                        "    int x;\n"
                        "    int *y = &x;\n"
                        "    *y = 0;\n"
                        "    x++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void a()\n"
                        "{\n"
                        "    char x[10], y[10];\n"
                        "    char *z = x;\n"
                        "    memset(z, 0, sizeof(x));\n"
                        "    memcpy(y, x, sizeof(x));\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // Handling >> and <<
        {
            valueFlowUninit("int a() {\n"
                            "    int ret;\n"
                            "    std::cin >> ret;\n"
                            "    ret++;\n"
                            "}");
            ASSERT_EQUALS("", errout_str());

            valueFlowUninit("void f(int b) {\n"
                            "    int a;\n"
                            "    std::cin >> b >> a;\n"
                            "    return a;"
                            "}");
            ASSERT_EQUALS("", errout_str());

            valueFlowUninit("void foo() {\n"   // #3707
                            "    Node node;\n"
                            "    int x;\n"
                            "    node[\"abcd\"] >> x;\n"
                            "}");
            ASSERT_EQUALS("", errout_str());

            valueFlowUninit("int a(FArchive &arc) {\n"  // #3060 (initialization through operator<<)
                            "    int *p;\n"
                            "    arc << p;\n"  // <- TODO initialization?
                            "    return *p;\n"
                            "}");
            ASSERT_EQUALS("[test.cpp:3:12]: (error) Uninitialized variable: p [uninitvar]\n", errout_str());

            // #4320
            valueFlowUninit("void f() {\n"
                            "    int a;\n"
                            "    a << 1;\n"
                            "    return a;\n"
                            "}");
            ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

            // #9750
            valueFlowUninit("struct S {\n"
                            "    int one;\n"
                            "    int two;\n"
                            "};\n"
                            "\n"
                            "void test(std::istringstream& in) {\n"
                            "    S p;\n"
                            "    in >> p.one >> p.two;\n"
                            "}");
            ASSERT_EQUALS("", errout_str());
        }

        valueFlowUninit("struct S { int x; };\n" // #9417
                        "void f() {\n"
                        "    S s;\n"
                        "    return s(1);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void a() {\n"   // asm
                        "    int x;\n"
                        "    asm();\n"
                        "    x++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void a()\n"
                        "{\n"
                        "    int x[10];\n"
                        "    struct xyz xyz1 = { .x = x };\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void foo()\n"
                        "{\n"
                        "   char *buf = malloc(100);\n"
                        "   struct ABC *abc = buf;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("class Fred {\n"
                        "public:\n"
                        "    FILE *f;\n"
                        "    ~Fred();\n"
                        "}\n"
                        "Fred::~Fred()\n"
                        "{\n"
                        "    fclose(f);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f()\n"
                        "{\n"
                        "    int c;\n"
                        "    ab(sizeof(xyz), &c);\n"
                        "    if (c);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f()\n"
                        "{\n"
                        "    int c;\n"
                        "    a = (f2(&c));\n"
                        "    c++;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // goto/setjmp/longjmp..
        valueFlowUninit("void foo(int x)\n"
                        "{\n"
                        "    long b;\n"
                        "    if (g()) {\n"
                        "        b =2;\n"
                        "        goto found;\n"
                        "    }\n"
                        "\n"
                        "    return;\n"
                        "\n"
                        "found:\n"
                        "    int a = b;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int foo()\n"
                        "{\n"
                        "    jmp_buf env;\n"
                        "    int a;\n"
                        "    int val = setjmp(env);\n"
                        "    if(val)\n"
                        "        return a;\n"
                        "    a = 1;\n"
                        "    longjmp(env, 1);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // range for..
        valueFlowUninit("void f() {\n"
                        "    X *item;\n"
                        "    for (item: itemList) {}\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("X f() {\n"
                        "    if (!itemList.empty()) {\n"
                        "        X* item;\n"
                        "        for(item: itemList) {}\n"
                        "        return *item;\n"
                        "    }\n"
                        "    return X{};\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // macro_for..
        valueFlowUninit("int foo()\n"
                        "{\n"
                        "  int retval;\n"
                        "  if (condition) {\n"
                        "    for12(1,2) { }\n"
                        "    retval = 1;\n"
                        "  }\n"
                        "  else\n"
                        "    retval = 2;\n"
                        "  return retval;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void foo(struct qb_list_head *list) {\n"
                        "    struct qb_list_head *iter;\n"
                        "    qb_list_for_each(iter, list) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void json_parse_nat_type_flags(json_t *root) {\n"
                        "    int index;\n"
                        "    json_array_foreach(root, index, value) {}\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int foo()\n"
                        "{\n"
                        "    int i;\n"
                        "    goto exit;\n"
                        "    i++;\n"
                        "exit:\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int foo() {\n"
                        "    int x,y=0;\n"
                        "again:\n"
                        "    if (y) return x;\n"
                        "    x = a;\n"
                        "    y = 1;\n"
                        "    goto again;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // #4040 - False positive
        valueFlowUninit("int f(int x)  {\n"
                        "    int iter;\n"
                        "    {\n"
                        "        union\n"
                        "        {\n"
                        "            int asInt;\n"
                        "            double asDouble;\n"
                        "        };\n"
                        "\n"
                        "        iter = x;\n"
                        "    }\n"
                        "    return 1 + iter;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // C++11 style initialization
        valueFlowUninit("int f() {\n"
                        "    int i = 0;\n"
                        "    int j{ i };\n"
                        "    return j;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #5646
        valueFlowUninit("float foo() {\n"
                        "  float source[2] = {3.1, 3.1};\n"
                        "  float (*sink)[2] = &source;\n"
                        "  return (*sink)[0];\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #8755
        valueFlowUninit("void f(int b) {\n"
                        "    int a;\n"
                        "    if (b == 10)\n"
                        "        a = 1;\n"
                        "    if (b == 13)\n"
                        "        a = 1;\n"
                        "    if (b == 'x') {}\n"
                        "    if (a) {}\n"
                        "}");
        TODO_ASSERT_EQUALS("[test.cpp:8]: (error) Uninitialized variable: a\n", "", errout_str());

        valueFlowUninit("void h() {\n"
                        "  int i;\n"
                        "  int* v = &i;\n"
                        "  sscanf(\"0\", \"%d\", v);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void test(int p) {\n"
                        "    int f;\n"
                        "    if (p > 0)\n"
                        "        f = 0;\n"
                        "    if (p > 1)\n"
                        "        f += 1;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("unsigned char get();\n"
                        "char f() {\n"
                        "    unsigned char c;\n"
                        "    do {\n"
                        "        c = get();\n"
                        "    } while (isalpha(c) == 0);\n"
                        "    return static_cast<char>(c);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(int x)\n"
                        "{\n"
                        "   int i;\n"
                        "   char value;\n"
                        "   for(i = 0; i < 1; i++) {\n"
                        "       if(x > 1)\n"
                        "           value = 0;\n"
                        "    }\n"
                        "    printf(\"\", value);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:6:13] -> [test.cpp:9:16]: (warning) Uninitialized variable: value [uninitvar]\n", errout_str());

        valueFlowUninit("void f(int x)\n"
                        "{\n"
                        "   int i;\n"
                        "   char value;\n"
                        "   for(i = 0; i < 1; i++) {\n"
                        "       if(x > 1)\n"
                        "           value = 0;\n"
                        "       else\n"
                        "           value = 1;\n"
                        "    }\n"
                        "    printf(\"\", value);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // function pointers
        valueFlowUninit("int f (const struct FileFuncDefs *ffd) {\n" // #10279
                        "  int c;\n"
                        "  (*ffd->zread)(&c, 1);\n"
                        "  return c;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int foo(unsigned int code) {\n" // #10279
                        "  int res;\n\n"
                        "  (* (utility_table[code])) (&res);\n"
                        "  return (res);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct Archive {\n"
                        "    bool isNull;\n"
                        "    friend void operator&(const Archive &, bool &isNull);\n"
                        "};\n"
                        "void load(Archive& ar) {\n"
                        "    bool isNull;\n"
                        "    ar & isNull;\n"
                        "    if (!isNull) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10119
        valueFlowUninit("struct Foo {\n"
                        "    int i{};\n"
                        "    static const float cf;\n"
                        "};\n"
                        "const float Foo::cf = 0.1f;\n"
                        "int bar() {\n"
                        "    Foo f;\n"
                        "    return f.i;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10326
        valueFlowUninit("void foo() {\n"
                        "    int cnt;\n"
                        "    do {\n"
                        "        cnt = 32 ;\n"
                        "    }\n"
                        "    while ( 0 ) ;\n"
                        "    if (cnt != 0) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10327 - avoid extra warnings for uninitialized variable
        valueFlowUninit("void dowork( int me ) {\n"
                        "    if ( me == 0 ) {}\n" // <- not uninitialized
                        "}\n"
                        "\n"
                        "int main() {\n"
                        "    int me;\n"
                        "     dowork(me);\n" // <- me is uninitialized
                        "}");
        ASSERT_EQUALS("[test.cpp:7:13]: (error) Uninitialized variable: me [uninitvar]\n", errout_str());

        valueFlowUninit("int foo() {\n"
                        "  int x;\n"
                        "  int a = x;\n" // <- x is uninitialized
                        "  return a;\n" // <- a has been initialized
                        "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        // #10468
        valueFlowUninit("uint32_t foo(uint32_t in) {\n"
                        "    uint32_t out, mask = 0x7F;\n"
                        "    while (mask ^ 0x7FFFFFFF)\n"
                        "        out = in & ~mask;\n"
                        "    return out;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #6597
        valueFlowUninit("int f(int b) {\n"
                        "    int a;\n"
                        "    if (!b)\n"
                        "        a = 1;\n"
                        "    if (b)\n"
                        "        return a;\n"
                        "    else\n"
                        "        return -1;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:6:16]: (warning) Uninitialized variable: a [uninitvar]\n", errout_str());

        // #9772
        valueFlowUninit("int func(void) {\n"
                        "    int rez;\n"
                        "    struct sccb* ccb;\n"
                        "    do {\n"
                        "        if ((ccb = calloc(1, sizeof(*ccb))) == NULL) {\n"
                        "            rez = 1;\n"
                        "            break;\n"
                        "        }\n"
                        "        rez = 0;\n"
                        "    } while (0);\n"
                        "    if (rez != 0)\n"
                        "        free(ccb);\n"
                        "    return rez;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10553
        valueFlowUninit("struct CharDataOnly {\n"
                        "   char data[100];\n"
                        "};\n"
                        "CharDataOnly f() {\n"
                        "   CharDataOnly testData;\n"
                        "   strcpy(testData.data, \"string smaller than size\");\n"
                        "   return testData;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10089
        valueFlowUninit("typedef union {\n"
                        "    struct { int x; };\n"
                        "    int v[1];\n"
                        "} U;\n"
                        "void init(int* d) {\n"
                        "    *d = 42;\n"
                        "}\n"
                        "void f() {\n"
                        "    U u;\n"
                        "    init(u.v);\n"
                        "    printf(\"%d\\n\", u.x);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10280
        valueFlowUninit("union U {\n"
                        "    char c[2];\n"
                        "    uint16_t u16;\n"
                        "};\n"
                        "uint16_t f(std::istream& is) {\n"
                        "    U u;\n"
                        "    is.read(u.c, 2);\n"
                        "    return u.u16;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "  char src, dest;\n"
                        "  std::memcpy(&dest, &src, 1);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:3:22]: (error) Uninitialized variable: &src [uninitvar]\n", errout_str());

        // #10988
        valueFlowUninit("void f(const void* ptr, bool* result) {\n"
                        "  int dummy;\n"
                        "  *result = (&dummy < ptr);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct A {\n"
                        "    int x;\n"
                        "};\n"
                        "void f() {\n"
                        "    A a;\n"
                        "    A* p = &a;\n"
                        "    p->x = 1;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct A {\n"
                        "    int x;\n"
                        "};\n"
                        "void g(const int&);\n"
                        "void f() {\n"
                        "    A a;\n"
                        "    g(a.x);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:7:9]: (error) Uninitialized variable: a.x [uninitvar]\n", errout_str());

        valueFlowUninit("struct A {\n"
                        "    int x;\n"
                        "};\n"
                        "void g(const int&);\n"
                        "void f() {\n"
                        "    A a;\n"
                        "    A* p = &a;\n"
                        "    g(p->x);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:8:10]: (error) Uninitialized variable: p->x [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int a;\n"
                        "    a++;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        // #11006
        valueFlowUninit("int g(int);\n"
                        "void f() {\n"
                        "    int received[NSIG];\n"
                        "    for (int sig = 0; sig < NSIG; sig++)\n"
                        "        received[sig] = g(sig);\n"
                        "    for (int sig = 0; sig < NSIG; sig++)\n"
                        "        if (received[sig]) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void increment(int& i) { ++i; }\n" // #6475
                        "int f() {\n"
                        "    int n;\n"
                        "    increment(n);\n"
                        "    return n;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:4:15] -> [test.cpp:1:28]: (warning) Uninitialized variable: i [uninitvar]\n", errout_str());

        // #11412
        valueFlowUninit("void f(int n) {\n"
                        "	short* p;\n"
                        "	if (n) {\n"
                        "		p = g(n);\n"
                        "	}\n"
                        "	for (int i = 0; i < n; i++)\n"
                        "		(void)p[i];\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11492
        valueFlowUninit("void f() {\n"
                        "    int i;\n"
                        "    try {\n"
                        "        i = 0;\n"
                        "    }\n"
                        "    catch (...) {\n"
                        "        if (i) {}\n"
                        "    }\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11624
        valueFlowUninit("const int N = 2;\n"
                        "void g(int a[N]) {\n"
                        "    for (int i = 0; i < N; ++i)\n"
                        "        a[i] = 1;\n"
                        "}\n"
                        "void f() {\n"
                        "    int a[N];\n"
                        "    g(a);\n"
                        "    if (a[0]) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11673
        valueFlowUninit("void f() {\n"
                        "    bool b;\n"
                        "    auto g = [&b]() {\n"
                        "        b = true;\n"
                        "    };\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #6619
        valueFlowUninit("void f() {\n"
                        "    int nok, i;\n"
                        "    for (i = 1; i < 5; i++) {\n"
                        "        if (i == 8)\n"
                        "            nok = 8;\n"
                        "    }\n"
                        "    printf(\"nok = %d\\n\", nok);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:7:26]: (error) Uninitialized variable: nok [uninitvar]\n", errout_str());

        // #7475
        valueFlowUninit("struct S {\n"
                        "    int a, b, c;\n"
                        "} typedef s_t;\n"
                        "void f() {\n"
                        "    s_t s;\n"
                        "    printf(\"%d\", s.a);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:6:20]: (error) Uninitialized variable: s.a [uninitvar]\n", errout_str());

        valueFlowUninit("void f(char* src) {\n" // #11608
                        "    char envar[64], *cp, c;\n"
                        "    for (src += 2, cp = envar; (c = *src) != '\\0'; src++)\n"
                        "        *cp++ = c;\n"
                        "    if (cp != envar)\n"
                        "        if ((cp = getenv(envar)) != NULL) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11459
        valueFlowUninit("struct S {\n"
                        "    enum E { N = 3 };\n"
                        "    static const int A[N];\n"
                        "    static void f();\n"
                        "};\n"
                        "const int S::A[N] = { 0, 1, 2 };\n"
                        "void S::f() {\n"
                        "    int tmp[N];\n"
                        "    for (int i = 0; i < N; i++)\n"
                        "        tmp[i] = 0;\n"
                        "    if (tmp[0]) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11055
        valueFlowUninit("void g(int*);\n"
                        "void f(bool b) {\n"
                        "    int i;\n"
                        "    int* p = b ? &i : nullptr;\n"
                        "    g(p);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct T {};\n" // #11075
                        "struct S {\n"
                        "    int n;\n"
                        "    struct T t[10];\n"
                        "};\n"
                        "void f(struct S* s, char** tokens) {\n"
                        "    struct T t[10];\n"
                        "    int n = 0;\n"
                        "    for (int i = 0; i < s->n; i++)\n"
                        "        if (tokens[i])\n"
                        "            t[n++] = s->t[i];\n"
                        "    for (int i = 0; i < n; i++)\n"
                        "        t[i];\n"
                        "}\n", false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("bool g();\n"
                        "void f() {\n"
                        "    int a[10];\n"
                        "    int idx = 0;\n"
                        "    if (g())\n"
                        "        a[idx++] = 1;\n"
                        "    for (int i = 0; i < idx; i++)\n"
                        "        (void)a[i];\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "  int x;\n"
                        "  int *p = 0 ? 0 : &x;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void g() {\n"
                        "  int y;\n"
                        "  int *q = 1 ? &y : 0;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(std::stringstream& ss) {\n" // #11805
                        "    int x;\n"
                        "    int* p = &x;\n"
                        "    ss >> *p;\n"
                        "}\n"
                        "void g() {\n"
                        "    int x;\n"
                        "    int* p = &x;\n"
                        "    int& r = *p;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(std::stringstream& ss) {\n" // #11805
                        "    int x;\n"
                        "    int* p = &x;\n"
                        "    ss >> *p;\n"
                        "}\n"
                        "void g() {\n"
                        "    int x;\n"
                        "    int* p = &x;\n"
                        "    int& r = *p;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S1 { char a[10]; };\n" // #11804
                        "struct S2 { struct S1 s1; };\n"
                        "void init(char* c);\n"
                        "void f() {\n"
                        "    struct S2 s2;\n"
                        "    init(s2.s1.a);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S { int i; };\n" // #11731
                        "void f(const S*& p);\n"
                        "int g() {\n"
                        "   const S* s;\n"
                        "   f(s);\n"
                        "   return s->i;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f(int i) {\n"
                        "    int x;\n"
                        "    int* p = &x;\n"
                        "    return i >> *p;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:4:17]: (error) Uninitialized variable: *p [uninitvar]\n", errout_str());

        valueFlowUninit("void f(int& x) {\n"
                        "    int i;\n"
                        "    x = i;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Uninitialized variable: i [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n" // #11890
                        "    int x;\n"
                        "    int* a[] = { &x };\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11992
        valueFlowUninit("void foo(const int &x) {\n"
                        "    if(x==42) {;}\n"
                        "}\n"
                        "void test(void) {\n"
                        "    int t;\n"
                        "    int &p = t;\n"
                        "    int &s = p;\n"
                        "    int &q = s;\n"
                        "    foo(q);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:9:9]: (error) Uninitialized variable: q [uninitvar]\n", errout_str());

        valueFlowUninit("int g();\n" // #12082
                        "void f() {\n"
                        "    int a[1], b[1];\n"
                        "    while (a[0] = g()) {}\n"
                        "    if ((b[0] = g()) == 0) {}\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(const char *x, char *y);\n" // #4527
                        "void g(char* b) {\n"
                        "    char a[1000];\n"
                        "    f(a, b);\n"
                        "    printf(\"%s\", a);\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        valueFlowUninit("void usage(const char *);\n" // #10330
                        "int main(int argc, char* argv[]) {\n"
                        "    int user = 0;\n"
                        "    struct passwd* pwd;\n"
                        "    while (1) {\n"
                        "        int c = getc();\n"
                        "        if (c == -1)\n"
                        "            break;\n"
                        "        switch (c) {\n"
                        "        case 'u': user = 123; break;\n"
                        "        }\n"
                        "    }\n"
                        "    if (argc == 1)\n"
                        "        usage(argv[0]);\n"
                        "    if (user)\n"
                        "        pwd = getpwnam(user);\n"
                        "    if (pwd == NULL) {}\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:15:9] -> [test.cpp:17:9]: (warning) Uninitialized variable: pwd [uninitvar]\n", errout_str());

        valueFlowUninit("size_t Read(unsigned char* buffer, size_t len);\n" // #11540
                        "void f() {\n"
                        "    const int N = 100;\n"
                        "    uint8_t data[N];\n"
                        "    size_t data_size = 0;\n"
                        "    for (int i = 0; i < 10; i++) {\n"
                        "        if (!data_size)\n"
                        "            data_size = Read(data, N);\n"
                        "        if (!data_size)\n"
                        "            return;\n"
                        "        if (data[0] == 0x47) {}\n"
                        "    }\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #12033
        valueFlowUninit("void g(const char*p);\n"
                        "void f() {\n"
                        "    char buf[10];\n"
                        "    g(buf);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Uninitialized variable: buf [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n" // #12288
                        "    char buf[100];\n"
                        "    char* p = new (buf) char[100];\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #12355
        valueFlowUninit("int f() {\n"
                        "    const int x[10](1, 2);\n"
                        "    if (x[0] == 1) {}\n"
                        "    return x[0];\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(bool* p) {\n" // #12287
                        "    if (p)\n"
                        "        *p = true; \n"
                        "}\n"
                        "void g() {\n"
                        "    bool b;\n"
                        "    f(&b);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S { int x; };\n" // #12394
                        "int f() {\n"
                        "    struct S s[1];\n"
                        "    s->x = 0;\n"
                        "    return s[0].x;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S { int* p; };\n" // #12473
                        "void f() {\n"
                        "    struct S s;\n"
                        "    s.p[0] = 0;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Uninitialized variable: s.p [uninitvar]\n", errout_str());

        // #12460
        valueFlowUninit("typedef struct {\n"
                        "  int a;\n"
                        "} st;\n"
                        "void foo(int* p, bool success) {\n"
                        "  st myst;\n"
                        "  if (success == 1) {\n"
                        "    myst.a = 5;\n"
                        "  }\n"
                        "  if ((success == 1) && (myst.a != 0)) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #12606
        valueFlowUninit("void f(int& r) { if (r) {} }\n"
                        "void g() {\n"
                        "    int i;\n"
                        "    f(i);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:4:7] -> [test.cpp:1:22]: (warning) Uninitialized variable: r [uninitvar]\n", errout_str());

        // #12197
        valueFlowUninit("void f() {\n"
                        "    char a[N];\n"
                        "    for (int i = 0; i < N; i++)\n"
                        "        a[i] = 1;\n"
                        "    const int* p = a;\n"
                        "    for (int i = 0; i < N; i++) {\n"
                        "        if (p[i]) {}\n"
                        "    }\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #12247
        valueFlowUninit("void f() {\n"
                        "    char a[10], *p = &a[0];\n"
                        "    p = getenv(\"abc\");\n"
                        "    printf(\"%\", p);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(char *q) {\n"
                        "    char a[1];\n"
                        "    char *p = a;\n"
                        "    p = q;\n"
                        "    printf(\"%s\", p);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("std::string f() {\n" // #12922
                        "    std::string a[2];\n"
                        "    std::array<std::string, 2> b;\n"
                        "    return a[1] + b.at(1);\n" // don't warn
                        "}\n"
                        "struct S { int i; };\n"
                        "struct T { int i = 0; };\n"
                        "int g() {\n"
                        "    std::array<int, 2> a;\n"
                        "    std::array<S, 2> b;\n"
                        "    std::array<T, 2> c;\n" // don't warn
                        "    return a.at(1) + b.at(1).i + c.at(1).i;\n"
                        "}\n"
                        "int h() {\n"
                        "    std::array<int, 2> a;\n"
                        "    return a[1];\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:12:12]: (error) Uninitialized variable: a [uninitvar]\n"
                      "[test.cpp:12:22]: (error) Uninitialized variable: b [uninitvar]\n"
                      "[test.cpp:16:12]: (error) Uninitialized variable: a [uninitvar]\n",
                      errout_str());

        valueFlowUninit("void f() {\n" // # 12932
                        "    std::array<int, 0> a;\n"
                        "    if (a.begin() == a.end()) {}\n"
                        "    std::array<int, 1> b;\n"
                        "    auto it = b.begin();\n"
                        "    *it = 0;\n"
                        "    std::array<int, 1> c;\n"
                        "    return c.front();\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:8:12]: (error) Uninitialized variable: c [uninitvar]\n", errout_str());

        // #13930
        valueFlowUninit("extern int32_t g_items[10U];\n"
                        "uint16_t write_item(uint8_t n);\n"
                        "uint16_t write_item(uint8_t n) {\n"
                        "    int32_t *p_item = NULL;\n"
                        "    uint16_t ret;\n"
                        "    ret = 0U;\n"
                        "    if (n < 10U)\n"
                        "        p_item = &g_items[n];\n"
                        "    else\n"
                        "        ret = 1U;\n"
                        "    if (ret == 0U) \n"
                        "        *p_item = 5;\n"
                        "    return ret;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("int f() {\n" // #12944
                        "    int i;\n"
                        "    auto x = [&]() { return i; };\n"
                        "    i = 5;\n"
                        "    return x();\n"
                        "}\n"
                        "int g() {\n"
                        "    int i;\n"
                        "    {\n"
                        "        auto x = [&]() { return i; };\n"
                        "        i = 5;\n"
                        "        return x();\n"
                        "    }\n"
                        "}\n"
                        "int h() {\n"
                        "    int j;\n"
                        "    return [&]() { return j; }();\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:17:27]: (error) Uninitialized variable: j [uninitvar]\n", errout_str());

        valueFlowUninit("int f() {\n" // #14582
                        "    int a[1];\n"
                        "    static_cast<int*>(a)[0] = 0;\n"
                        "    return a[0];\n"
                        "}\n"
                        "int g() {\n"
                        "    int a[1];\n"
                        "    ((int*)a)[0] = 0;\n"
                        "    return a[0];\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void valueFlowUninitBreak() { // Do not show duplicate warnings about the same uninitialized value
        valueFlowUninit("struct wcsstruct {\n"
                        "    int *wcsprm;\n"
                        "};\n"
                        "\n"
                        "void copy_wcs(wcsstruct *wcsin) {\n"
                        "    wcsstruct *x;\n"
                        "    memcpy(wcsin, x, sizeof(wcsstruct));\n" // <- warning
                        "    x->wcsprm = NULL;\n" // <- no warning
                        "}");
        ASSERT_EQUALS("[test.cpp:7:19]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("struct wcsstruct {\n"
                        "    int *wcsprm;\n"
                        "};\n"
                        "\n"
                        "void copy_wcs(wcsstruct *wcsin) {\n"
                        "    wcsstruct *x;\n"
                        "    sizeof(x);\n"
                        "    x->wcsprm = NULL;\n" // <- Warn
                        "}");
        ASSERT_EQUALS("[test.cpp:8:5]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());

        valueFlowUninit("struct wcsstruct {\n"
                        "    int *wcsprm;\n"
                        "};\n"
                        "\n"
                        "void init_wcs(wcsstruct *x) { if (x->wcsprm != NULL); }\n" // <- no warning
                        "\n"
                        "void copy_wcs() {\n"
                        "    wcsstruct *x;\n"
                        "    x->wcsprm = NULL;\n" // <- warn here
                        "    init_wcs(x);\n" // <- no warning
                        "}");
        ASSERT_EQUALS("[test.cpp:9:5]: (error) Uninitialized variable: x [uninitvar]\n", errout_str());
    }

    void uninitvar_ipa() {
        // #8825
        valueFlowUninit("typedef struct  {\n"
                        "    int flags;\n"
                        "} someType_t;\n"
                        "void bar(const someType_t * const p)  {\n"
                        "    if( (p->flags & 0xF000) == 0xF000){}\n"
                        "}\n"
                        "void f(void) {\n"
                        "    someType_t gVar;\n"
                        "    bar(&gVar);\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:9:9]: (error) Uninitialized variable: &gVar [uninitvar]\n", errout_str());

        valueFlowUninit("typedef struct\n"
                        "{\n"
                        "        int flags[3];\n"
                        "} someType_t;\n"
                        "void f(void) {\n"
                        "        someType_t gVar;\n"
                        "        if(gVar.flags[1] == 42){}\n"
                        "}");
        // TODO : find bugs for member arrays
        TODO_ASSERT_EQUALS("[test.cpp:7]: (error) Uninitialized variable: gVar.flags\n", "", errout_str());

        valueFlowUninit("void foo() {\n" // #10293
                        "  union {\n"
                        "    struct hdr cm;\n"
                        "    char control[123];\n"
                        "  } u;\n"
                        "  char *x = u.control;\n" // <- no error
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct pc_data {\n"
                        "    struct {\n"
                        "        char   * strefa;\n"
                        "    } wampiryzm;\n"
                        "};\n"
                        "void f() {\n"
                        "    struct pc_data *pcdata;\n"
                        "    if ( *pcdata->wampiryzm.strefa == '\\0' ) { }\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:8:11]: (error) Uninitialized variable: pcdata [uninitvar]\n", errout_str());

        // # 9293
        valueFlowUninit("struct S {\n"
                        "  int x;\n"
                        "  int y;\n"
                        "};\n"
                        "\n"
                        "void f() {\n"
                        "    struct S s1;\n"
                        "    int * x = &s1.x;\n"
                        "    struct S s2 = {*x, 0};\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:9:20]: (error) Uninitialized variable: *x [uninitvar]\n", errout_str());

        valueFlowUninit("struct S {\n"
                        "  int x;\n"
                        "  int y;\n"
                        "};\n"
                        "\n"
                        "void f() {\n"
                        "    struct S s1;\n"
                        "    struct S s2;\n"
                        "    int * x = &s1.x;\n"
                        "    s2.x = *x;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:10:12]: (error) Uninitialized variable: *x [uninitvar]\n", errout_str());

        valueFlowUninit("void f(bool * x) {\n"
                        "    *x = false;\n"
                        "}\n"
                        "void g() {\n"
                        "    bool b;\n"
                        "    f(&b);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(bool * x) {\n"
                        "    if (x != nullptr)\n"
                        "        x = 1;\n"
                        "}\n"
                        "void g() {\n"
                        "    bool x;\n"
                        "    f(&x);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    bool b;\n"
                        "    bool * x = &b;\n"
                        "    if (x != nullptr)\n"
                        "        x = 1;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct A { bool b; };"
                        "void f(A * x) {\n"
                        "    x->b = false;\n"
                        "}\n"
                        "void g() {\n"
                        "    A b;\n"
                        "    f(&b);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("std::string f() {\n"
                        "    std::ostringstream ostr;\n"
                        "    ostr << \"\";\n"
                        "    return ostr.str();\n"
                        "}");
        ASSERT_EQUALS("", errout_str());
        // #9281
        valueFlowUninit("struct s {\n"
                        "    char a[20];\n"
                        "};\n"
                        "void c(struct s *sarg) {\n"
                        "    sarg->a[0] = '\\0';\n"
                        "}\n"
                        "void b(struct s *sarg) {\n"
                        "    c(sarg);\n"
                        "}\n"
                        "void a() {\n"
                        "    struct s s1;\n"
                        "    b(&s1);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // # 9290
        valueFlowUninit("struct A {\n"
                        "    double x;\n"
                        "};\n"
                        "double b() {\n"
                        "    A * c;\n"
                        "    c->x = 42;\n"
                        "    return c->x;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Uninitialized variable: c [uninitvar]\n",
                      errout_str());

        valueFlowUninit("struct A {\n"
                        "    double x;\n"
                        "};\n"
                        "double b() {\n"
                        "    A c;\n"
                        "    c.x = 42;\n"
                        "    return c.x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct A {\n"
                        "    double x;\n"
                        "};\n"
                        "double d(A * e) {\n"
                        "    e->x = 42;\n"
                        "    return e->x;\n"
                        "}\n"
                        "double b() {\n"
                        "    A c;\n"
                        "    return d(&c);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // # 9302
        valueFlowUninit("struct VZ   {\n"
                        "    double typ;\n"
                        "};\n"
                        "void read() {\n"
                        "    struct VZ vz;\n"
                        "    struct VZ* pvz = &vz;\n"
                        "    vz.typ      = 42;\n"
                        "    if (pvz->typ == 0)\n"
                        "        return;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // # 9305
        valueFlowUninit("struct kf {\n"
                        "    double x;\n"
                        "};\n"
                        "void set(kf* k) {\n"
                        "    k->x = 0;\n"
                        "}\n"
                        "void cal() {\n"
                        "    KF b;\n"
                        "    KF* pb = &b;\n"
                        "    set( pb);\n"
                        "    if (pb->x)\n"
                        "        return;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // # 9348
        valueFlowUninit("void f(int *a) {\n"
                        "  int b = 0;\n"
                        "  memcpy(a, &b, sizeof(b));\n"
                        "}\n"
                        "void g() {\n"
                        "  int i;\n"
                        "  f(&i);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // # 9631
        valueFlowUninit("static void g(bool * result, int num, int num2, size_t * buflen) {\n"
                        "  if (*result && *buflen >= 5) {}\n"
                        "}\n"
                        "void f() {\n"
                        "  size_t bytesCopied;\n"
                        "  bool copied_all = true;\n"
                        "  g(&copied_all, 5, 6, &bytesCopied);\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:7:24] -> [test.cpp:2:18]: (warning) Uninitialized variable: *buflen [uninitvar]\n", errout_str());

        // # 9953
        valueFlowUninit("uint32_t f(uint8_t *mem) {\n"
                        "    uint32_t u32;\n"
                        "    uint8_t *buf = (uint8_t *)(&u32);\n"
                        "    buf[0] = mem[0];\n"
                        "    return(*(uint32_t *)buf);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void valueFlowUninitStructMembers()
    {
        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab.a = 1;\n"
                        "    if (ab.b == 2) {}\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized variable: ab.b [uninitvar]\n", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void do_something(const struct AB &ab) { a = ab.a; }\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab.a = 0;\n"
                        "    do_something(ab);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void do_something(const struct AB &ab) { a = ab.b; }\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab.a = 0;\n"
                        "    do_something(ab);\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:6:18] -> [test.cpp:2:49]: (warning) Uninitialized variable: ab.b [uninitvar]\n", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    int a = ab.a;\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("[test.c:4:16]: (error) Uninitialized variable: ab.a [uninitvar]\n", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "    AB ab1;\n"
                        "    AB ab2 = { ab1.a, 0 };\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:4:20]: (error) Uninitialized variable: ab1.a [uninitvar]\n", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    buf[ab.a] = 0;\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("[test.c:4:12]: (error) Uninitialized variable: ab.a [uninitvar]\n", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab.a = 1;\n"
                        "    x = ab;\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("[test.c:5:9]: (error) Uninitialized variable: ab.b [uninitvar]\n", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab.a = 1;\n"
                        "    x = *(&ab);\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("[test.c:5:9]: (error) Uninitialized variable: *(&ab).b [uninitvar]\n", errout_str());

        valueFlowUninit("void f(void) {\n"
                        "    struct AB ab;\n"
                        "    int x;\n"
                        "    ab.a = (void*)&x;\n"
                        "    dostuff(&ab,0);\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct Element {\n"
                        "    static void f() { }\n"
                        "};\n"
                        "void test() {\n"
                        "    Element *element; element->f();\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (error) Uninitialized variable: element [uninitvar]\n", errout_str());

        valueFlowUninit("struct Element {\n"
                        "    static void f() { }\n"
                        "};\n"
                        "void test() {\n"
                        "    Element *element; (*element).f();\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:25]: (error) Uninitialized variable: element [uninitvar]\n", errout_str());

        valueFlowUninit("struct Element {\n"
                        "    static int v;\n"
                        "};\n"
                        "void test() {\n"
                        "    Element *element; element->v;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (error) Uninitialized variable: element [uninitvar]\n", errout_str());

        valueFlowUninit("struct Element {\n"
                        "    static int v;\n"
                        "};\n"
                        "void test() {\n"
                        "    Element *element; (*element).v;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:25]: (error) Uninitialized variable: element [uninitvar]\n", errout_str());

        valueFlowUninit("struct Element {\n"
                        "    void f() { }\n"
                        "};\n"
                        "void test() {\n"
                        "    Element *element; element->f();\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (error) Uninitialized variable: element [uninitvar]\n", errout_str());

        valueFlowUninit("struct Element {\n"
                        "    void f() { }\n"
                        "};\n"
                        "void test() {\n"
                        "    Element *element; (*element).f();\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:25]: (error) Uninitialized variable: element [uninitvar]\n", errout_str());

        valueFlowUninit("struct Element {\n"
                        "    int v;\n"
                        "};\n"
                        "void test() {\n"
                        "    Element *element; element->v;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:23]: (error) Uninitialized variable: element [uninitvar]\n", errout_str());

        valueFlowUninit("struct Element {\n"
                        "    int v;\n"
                        "};\n"
                        "void test() {\n"
                        "    Element *element; (*element).v;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:25]: (error) Uninitialized variable: element [uninitvar]\n", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n" // pass struct member by address
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    assign(&ab.a, 0);\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit(
            "struct Cstring { char *text; int size, alloc; };\n"
            "int maybe();\n"
            "void f() {\n"
            "    Cstring res;\n"
            "    if ( ! maybe() ) return;\n"                                     // <- fp goes away if this is removed
            "    ( ((res).text = (void*)0), ((res).size = (res).alloc = 0) );\n" // <- fp goes away if parentheses are removed
            "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void do_something(const struct AB ab);\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab.a = 0;\n"
                        "    ab.b = 0;\n"
                        "    do_something(ab);\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("", errout_str());

        {
            valueFlowUninit("struct AB { char a[10]; };\n"
                            "void f(void) {\n"
                            "    struct AB ab;\n"
                            "    strcpy(ab.a, STR);\n"
                            "}\n",
                            false);
            ASSERT_EQUALS("", errout_str());

            valueFlowUninit("struct AB { unsigned char a[10]; };\n" // #8999 - cast
                            "void f(void) {\n"
                            "    struct AB ab;\n"
                            "    strcpy((char *)ab.a, STR);\n"
                            "}\n",
                            false);
            ASSERT_EQUALS("", errout_str());

            valueFlowUninit("struct AB { char a[10]; };\n"
                            "void f(void) {\n"
                            "    struct AB ab;\n"
                            "    strcpy(x, ab.a);\n"
                            "}\n",
                            false);
            TODO_ASSERT_EQUALS("[test.c:4:16]: (error) Uninitialized variable: ab.a [uninitvar]\n", "", errout_str());

            valueFlowUninit("struct AB { int a; };\n"
                            "void f(void) {\n"
                            "    struct AB ab;\n"
                            "    dosomething(ab.a);\n"
                            "}\n",
                            false);
            ASSERT_EQUALS("", errout_str());
        }

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void do_something(const struct AB ab);\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab = getAB();\n"
                        "    do_something(ab);\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("", errout_str());

        {
            // #6769 - calling method that might assign struct members
            valueFlowUninit("struct AB { int a; int b; void set(); };\n"
                            "void f(void) {\n"
                            "    struct AB ab;\n"
                            "    ab.set();\n"
                            "    x = ab;\n"
                            "}");
            ASSERT_EQUALS("", errout_str());

            valueFlowUninit("struct AB { int a; int get() const; };\n"
                            "void f(void) {\n"
                            "    struct AB ab;\n"
                            "    ab.get();\n"
                            "    x = ab;\n"
                            "}");
            ASSERT_EQUALS("[test.cpp:4:5]: (error) Uninitialized variable: ab [uninitvar]\n", errout_str());

            valueFlowUninit("struct AB { int a; void dostuff() {} };\n"
                            "void f(void) {\n"
                            "    struct AB ab;\n"
                            "    ab.dostuff();\n"
                            "    x = ab;\n"
                            "}");
            TODO_ASSERT_EQUALS("error", "", errout_str());
        }

        valueFlowUninit("struct AB { int a; struct { int b; int c; } s; };\n"
                        "void do_something(const struct AB ab);\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab.a = 1;\n"
                        "    ab.s.b = 2;\n"
                        "    ab.s.c = 3;\n"
                        "    do_something(ab);\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct conf {\n"
                        "    char x;\n"
                        "};\n"
                        "\n"
                        "void do_something(struct conf ant_conf);\n"
                        "\n"
                        "void f(void) {\n"
                        "   struct conf c;\n"
                        "   initdata(&c);\n"
                        "   do_something(c);\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct PIXEL {\n"
                        "    union  {\n"
                        "        struct { unsigned char red,green,blue,alpha; };\n"
                        "        unsigned int color;\n"
                        "    };\n"
                        "};\n"
                        "\n"
                        "unsigned char f() {\n"
                        "    struct PIXEL p1;\n"
                        "    p1.color = 255;\n"
                        "    return p1.red;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "int f() {\n"
                        "  struct AB *ab;\n"
                        "  for (i = 1; i < 10; i++) {\n"
                        "    if (condition && (ab = getab()) != NULL) {\n"
                        "      a = ab->a;\n"
                        "    }\n"
                        "  }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "int f(int x) {\n"
                        "  struct AB *ab;\n"
                        "  if (x == 0) {\n"
                        "    ab = getab();\n"
                        "  }\n"
                        "  if (x == 0 && (ab != NULL || ab->a == 0)) { }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct A { int *x; };\n" // declarationId is 0 for "delete"
                        "void foo(void *info, void*p);\n"
                        "void bar(void) {\n"
                        "  struct A *delete = 0;\n"
                        "  foo( info, NULL );\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct ABC { int a; int b; int c; };\n"
                        "void foo(int x, const struct ABC *abc);\n"
                        "void bar(void) {\n"
                        "  struct ABC abc;\n"
                        "  foo(123, &abc);\n"
                        "  return abc.b;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized variable: &abc [uninitvar]\n", errout_str());

        valueFlowUninit("struct ABC { int a; int b; int c; };\n"
                        "void foo() {\n"
                        "  struct ABC abc;\n"
                        "  dostuff((uint32_t *)&abc.a);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f(void) {\n"
                        "    struct tm t;\n"
                        "    t.tm_year = 123;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // return
        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab.a = 0;\n"
                        "    return ab.b;\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("[test.c:5:15]: (error) Uninitialized variable: ab.b [uninitvar]\n", errout_str());

        valueFlowUninit("struct AB { int a; int b; };\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    ab.a = 0;\n"
                        "    return ab.a;\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S { int a; int b; };\n" // #8299
                        "void f(void) {\n"
                        "    struct S s;\n"
                        "    s.a = 0;\n"
                        "    return s;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Uninitialized variable: s.b [uninitvar]\n", errout_str());

        valueFlowUninit("struct S { int a; int b; };\n" // #9810
                        "void f(void) {\n"
                        "    struct S s;\n"
                        "    return s.a ? 1 : 2;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:4:14]: (error) Uninitialized variable: s.a [uninitvar]\n", errout_str());

        // checkIfForWhileHead
        valueFlowUninit("struct FRED {\n"
                        "    int a;\n"
                        "    int b;\n"
                        "};\n"
                        "\n"
                        "void f(void) {\n"
                        "   struct FRED fred;\n"
                        "   fred.a = do_something();\n"
                        "   if (fred.a == 0) { }\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct FRED {\n"
                        "    int a;\n"
                        "    int b;\n"
                        "};\n"
                        "\n"
                        "void f(void) {\n"
                        "   struct FRED fred;\n"
                        "   fred.a = do_something();\n"
                        "   if (fred.b == 0) { }\n"
                        "}\n",
                        false);
        ASSERT_EQUALS("[test.c:9:13]: (error) Uninitialized variable: fred.b [uninitvar]\n", errout_str());

        valueFlowUninit("struct Fred { int a; };\n"
                        "void f() {\n"
                        "    struct Fred fred;\n"
                        "    if (fred.a==1) {}\n"
                        "}",
                        false);
        ASSERT_EQUALS("[test.c:4:14]: (error) Uninitialized variable: fred.a [uninitvar]\n", errout_str());

        valueFlowUninit("struct S { int n; int m; };\n"
                        "void f(void) {\n"
                        " struct S s;\n"
                        " for (s.n = 0; s.n <= 10; s.n++) { }\n"
                        "}",
                        false);
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void test2() {\n"
                        "  struct { char type; } s_d;\n"
                        "  if (foo(&s_d.type)){}\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // for
        valueFlowUninit("struct AB { int a; };\n"
                        "void f() {\n"
                        "    struct AB ab;\n"
                        "    while (x) { clear(ab); z = ab.a; }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct AB { int a; };\n"
                        "void f() {\n"
                        "    struct AB ab;\n"
                        "    while (x) { ab.a = ab.a + 1; }\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:4:27]: (error) Uninitialized variable: ab.a [uninitvar]\n", errout_str());

        valueFlowUninit("struct AB { int a; };\n"
                        "void f() {\n"
                        "    struct AB ab;\n"
                        "    while (x) { init(&ab); z = ab.a; }\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // address of member
        valueFlowUninit("struct AB { int a[10]; int b; };\n"
                        "void f() {\n"
                        "    struct AB ab;\n"
                        "    int *p = ab.a;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // Reference
        valueFlowUninit("struct A { int x; };\n"
                        "void foo() {\n"
                        "  struct A a;\n"
                        "  int& x = a.x;\n"
                        "  x = 0;\n"
                        "  return a.x;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // non static data-member initialization
        valueFlowUninit("struct AB { int a=1; int b; };\n"
                        "void f(void) {\n"
                        "    struct AB ab;\n"
                        "    int a = ab.a;\n"
                        "    int b = ab.b;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:5:16]: (error) Uninitialized variable: ab.b [uninitvar]\n", errout_str());

        // STL class member
        valueFlowUninit("struct A {\n"
                        "    std::map<int, int> m;\n"
                        "    int i;\n"
                        "};\n"
                        "void foo() {\n"
                        "    A a;\n"
                        "    x = a.m;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // Unknown type (C++)
        valueFlowUninit("struct A {\n"
                        "    C m;\n"
                        "    int i;\n"
                        "};\n"
                        "void foo() {\n"
                        "    A a;\n"
                        "    x = a.m;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        // Unknown type (C)
        valueFlowUninit("struct A {\n"
                        "    C m;\n"
                        "    int i;\n"
                        "};\n"
                        "void foo() {\n"
                        "    A a;\n"
                        "    x = a.m;\n"
                        "}",
                        false);
        ASSERT_EQUALS("[test.c:7:11]: (error) Uninitialized variable: a.m [uninitvar]\n", errout_str());

        // Type with constructor
        valueFlowUninit("class C { C(); }\n"
                        "struct A {\n"
                        "    C m;\n"
                        "    int i;\n"
                        "};\n"
                        "void foo() {\n"
                        "    A a;\n"
                        "    x = a.m;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S {\n"
                        "   int t[1];\n"
                        "};\n"
                        "int f(const S* ps) {\n"
                        "   return ps->t[0];\n"
                        "}\n"
                        "void g() {\n"
                        "   S s;\n"
                        "   s.t[0] = 1;\n"
                        "   f(&s);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S {\n"
                        "   int t[1];\n"
                        "   int u;\n"
                        "};\n"
                        "\n"
                        "int f(const S* ps) {\n"
                        "   return ps->t[0];\n"
                        "}\n"
                        "\n"
                        "int main(void)\n"
                        "{\n"
                        "   S s;\n"
                        "   s.t[0] = 1;\n"
                        "   f(&s);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct X {\n"
                        "   int a, b;\n"
                        "};\n"
                        "struct S {\n"
                        "   X t;\n"
                        "};\n"
                        "int f(const S* ps) {\n"
                        "   return ps->t.a;\n"
                        "}\n"
                        "void g() {\n"
                        "   S s;\n"
                        "   s.t.a = 1;\n"
                        "   f(&s);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("typedef struct { int a; int b; int c; } ABC;\n" // #5777
                        "void setabc(int x, const ABC* const abc) {\n"
                        "    sum = abc->a + abc->b + abc->c;\n"
                        "}\n"
                        "void f(void) {\n"
                        "    ABC abc;\n"
                        "    abc.a = 1;\n"
                        "    setabc(123, &abc);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:8:17] -> [test.cpp:3:25]: (warning) Uninitialized variable: abc->b [uninitvar]\n", errout_str());

        valueFlowUninit("struct S { int* p; };\n" // #10463
                        "void f(S* in) {\n"
                        "    S* s;\n"
                        "    memcpy(in, s, sizeof(S));\n"
                        "    s->p = NULL;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:4:16]: (error) Uninitialized variable: s [uninitvar]\n", errout_str());

        valueFlowUninit("struct S {\n" // #11321
                        "    int a = 0;\n"
                        "    int b;\n"
                        "};\n"
                        "void f() {\n"
                        "    S s1;\n"
                        "    s1.b = 1;\n"
                        "    S s2 = s1;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11460
        valueFlowUninit("struct B { int i; };\n"
                        "    struct H {\n"
                        "    void e() const;\n"
                        "    static const B b;\n"
                        "};\n"
                        "void f() {\n"
                        "    H h;\n"
                        "    h.e();\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11597
        valueFlowUninit("void f(size_t f) {\n"
                        "    struct {\n"
                        "        int i;\n"
                        "        enum { offset = 1062 };\n"
                        "    } s;\n"
                        "    if (f < s.offset + sizeof(s)) {}\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11776 - function call initialises struct array member
        valueFlowUninit("typedef struct {\n"
                        "  int arr[1];\n"
                        "  int count;\n"
                        "} arr_struct;\n"
                        "\n"
                        "void init(int *a, int b);\n"
                        "\n"
                        "void foo(arr_struct const *var);\n" // <- inconclusive if var->count is used
                        "\n"
                        "void uninitvar_FP7() {\n"
                        "  arr_struct my_st;\n"
                        "  init(my_st.arr, 12);\n" // <- assume that my_st.arr is written
                        "  foo(&my_st);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("typedef struct {\n"
                        "  int arr[1];\n"
                        "  int count;\n"
                        "} arr_struct;\n"
                        "\n"
                        "void init(int *a, int b);\n"
                        "\n"
                        "void foo(arr_struct const *var) {\n"
                        "  x = var->arr[0];\n"
                        "}\n"
                        "\n"
                        "void uninitvar_FP7() {\n"
                        "  arr_struct my_st;\n"
                        "  init(my_st.arr, 12);\n" // <- assume that my_st.arr is written
                        "  foo(&my_st);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S {\n" // #12188
                        "    int i;\n"
                        "    struct T { int j; } t;\n"
                        "};\n"
                        "void f() {\n"
                        "    S s;\n"
                        "    ++s.i;\n"
                        "}\n"
                        "void g() {\n"
                        "    S s;\n"
                        "    s.i--;\n"
                        "}\n"
                        "void h() {\n"
                        "    S s;\n"
                        "    s.i &= 3;\n"
                        "}\n"
                        "void k() {\n"
                        "    S s;\n"
                        "    if (++s.i < 3) {}\n"
                        "}\n"
                        "void m() {\n"
                        "    S s;\n"
                        "    ++s.t.j;\n"
                        "}\n"
                        "void n() {\n"
                        "    S s;\n"
                        "    if (s.t.j-- < 3) {}\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:7:9]: (error) Uninitialized variable: s.i [uninitvar]\n"
                      "[test.cpp:11:7]: (error) Uninitialized variable: s.i [uninitvar]\n"
                      "[test.cpp:15:7]: (error) Uninitialized variable: s.i [uninitvar]\n"
                      "[test.cpp:19:13]: (error) Uninitialized variable: s.i [uninitvar]\n"
                      "[test.cpp:23:11]: (error) Uninitialized variable: s.t.j [uninitvar]\n"
                      "[test.cpp:27:13]: (error) Uninitialized variable: s.t.j [uninitvar]\n",
                      errout_str());

        valueFlowUninit("struct S { int x; };\n" // #6933
                        "void f() {\n"
                        "    int i;\n"
                        "    S s(i);\n"
                        "}\n"
                        "void g() {\n"
                        "    int i;\n"
                        "    S t{ i };\n"
                        "}\n"
                        "void h() {\n"
                        "    int i;\n"
                        "    std::vector<int> v(i);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Uninitialized variable: i [uninitvar]\n"
                      "[test.cpp:8:10]: (error) Uninitialized variable: i [uninitvar]\n"
                      "[test.cpp:12:24]: (error) Uninitialized variable: i [uninitvar]\n",
                      errout_str());

        valueFlowUninit("struct S {\n"
                        "    S(char**);\n"
                        "    int i;\n"
                        "};\n"
                        "void f() {\n"
                        "    char* p;\n"
                        "    S s(&p);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S {\n" // #12354
                        "    int i{};\n"
                        "    void f();\n"
                        "};\n"
                        "void f(bool b) {\n"
                        "    S* p;\n"
                        "    if (b)\n"
                        "        p = new S();\n"
                        "    p->f();\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:7:9] -> [test.cpp:9:5]: (warning) Uninitialized variable: p [uninitvar]\n", errout_str());

        // #12461
        valueFlowUninit("struct stry_type {\n"
                        "  void *out;\n"
                        "};\n"
                        "void bar(str_type *items);\n"
                        "void foo() {\n"
                        "  str_type st_arr[1];\n"
                        "  char arr[5];\n"
                        "  st_arr[0].out = &arr;\n"
                        "  bar(st_arr);\n"
                        "  int len = strlen(arr);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct stry_type {\n"
                        "  void *out;\n"
                        "};\n"
                        "void foo() {\n"
                        "  str_type st_arr[1];\n"
                        "  char arr[5];\n"
                        "  st_arr[0].out = &arr;\n"
                        "  int len = strlen(arr);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:8:20]: (error) Uninitialized variable: arr [uninitvar]\n", errout_str());

        valueFlowUninit("struct S {\n"
                        "  void *out;\n"
                        "};\n"
                        "void bar(S* s);\n"
                        "void foo() {\n"
                        "  S s[1][1];\n"
                        "  char arr[5];\n"
                        "  s[0][0].out = &arr;\n"
                        "  bar(s[0]);\n"
                        "  int len = strlen(arr);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S1 { int x; };\n" // #12401
                        "struct S2 { struct S1 s1; };\n"
                        "struct S2 f() {\n"
                        "    struct S2 s2;\n"
                        "    struct S1* s1 = &s2.s1;\n"
                        "    s1->x = 0;\n"
                        "    return s2;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S {\n" // #12685
                        "    explicit S(double v);\n"
                        "    double m;\n"
                        "};\n"
                        "void f() {\n"
                        "    double d;\n"
                        "    S s(d);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:7:9]: (error) Uninitialized variable: d [uninitvar]\n", errout_str());

        valueFlowUninit("struct S {\n"
                        "    explicit S(double v) : m(v) {}\n"
                        "    double m;\n"
                        "};\n"
                        "void f() {\n"
                        "    double d;\n"
                        "    S s{ d };\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:7:10]: (error) Uninitialized variable: d [uninitvar]\n", errout_str());

        valueFlowUninit("struct S { int x; int y; };\n"
                        "int f() {\n"
                        "  S s;\n"
                        "  s.x = 0;\n"
                        "  return (&s)->x;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S { int x; int y; };\n"
                        "int f() {\n"
                        "  S s;\n"
                        "  s.x = 0;\n"
                        "  return (&s)->y;\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:5:16]: (error) Uninitialized variable: s.y [uninitvar]\n", errout_str());

        valueFlowUninit("struct S { int* p; };\n" // #14640
                        "void f() {\n"
                        "    int x;\n"
                        "    S s{ &x };\n"
                        "    *s.p = 0;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    int x;\n"
                        "    std::vector<int*> v{ &x };\n"
                        "    *v[0] = 0;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void valueFlowUninitForLoop()
    {
        valueFlowUninit("void f() {\n" // #9924
                        "    for (unsigned int a = 0, b; a < 42; a++) {\n"
                        "        std::cout << ++b << std::endl;\n"
                        "    }\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:3:24]: (error) Uninitialized variable: b [uninitvar]\n", errout_str());

        // #14137
        valueFlowUninit("int f(int n) {\n"
                        "    int x;\n"
                        "    assert(n > 0);\n"
                        "    for(int i=0;i<n;i++)\n"
                        "        x = n;\n"
                        "    return x;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_memberfunction() {
        // # 8715
        valueFlowUninit("struct C {\n"
                        "    int x();\n"
                        "};\n"
                        "void f() {\n"
                        "    C *c;\n"
                        "    if (c->x() == 4) {}\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:6:9]: (error) Uninitialized variable: c [uninitvar]\n", errout_str());

        valueFlowUninit("struct A { \n"
                        "    int i; \n"
                        "    void f();\n"
                        "};\n"
                        "void g() {\n"
                        "    A a;\n"
                        "    a.f();\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_nonmember() {
        valueFlowUninit("struct Foo {\n"
                        "  int bar;\n"
                        "};\n"
                        "\n"
                        "int main() {\n"
                        "  Foo* foo;\n"
                        "  foo->bar = 3;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:7:3]: (error) Uninitialized variable: foo [uninitvar]\n", errout_str());
    }

    void uninitvarDesignatedInitializers() {
        checkUninitVar("struct a { int b; };\n"
                       "int main() {\n"
                       "  char *b;\n"
                       "  extern int f(struct a *);\n"
                       "  return f(&(struct a){.b = 0});\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("struct a { int b, c; };\n"
                       "int main() {\n"
                       "  char *c;\n"
                       "  extern int f(struct a *);\n"
                       "  return f(&(struct a){.b = 0, .c = 0});\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvarMemberPointer() {
        checkUninitVar("void f()\n"
                       "{\n"
                       "  struct S {};\n"
                       "  int S::* mp;\n"
                       "  if (mp) {}\n"
                       "}\n");
        ASSERT_EQUALS("[test.cpp:5:7]: (error) Uninitialized variable: mp [legacyUninitvar]\n", errout_str());
    }

    void isVariableUsageDeref() {
        // *p
        checkUninitVar("void f() {\n"
                       "    char a[10];\n"
                       "    char c = *a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        // extracttests.start: extern const int SIZE;
        checkUninitVar("void f() {\n"
                       "    char a[SIZE+10];\n"
                       "    char c = *a;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "    char a[10];\n"
                       "    *a += 10;\n"
                       "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Uninitialized variable: a [legacyUninitvar]\n", errout_str());

        checkUninitVar("void f() {\n"
                       "  int a[10][10];\n"
                       "  dostuff(*a);\n"
                       "}");
        ASSERT_EQUALS("", errout_str());

        checkUninitVar("void f() {\n"
                       "    void (*fp[1]) (void) = {function1};\n"
                       "    (*fp[0])();\n"
                       "}");
        ASSERT_EQUALS("", errout_str());
    }

    void isVariableUsageDerefValueflow()
    {
        // *p
        valueFlowUninit("void f() {\n"
                        "    char a[10];\n"
                        "    char c = *a;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Uninitialized variable: *a [uninitvar]\n", errout_str());

        // extracttests.start: extern const int SIZE;
        valueFlowUninit("void f() {\n"
                        "    char a[SIZE+10];\n"
                        "    char c = *a;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Uninitialized variable: *a [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n"
                        "    char a[10];\n"
                        "    *a += 10;\n"
                        "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: *a [uninitvar]\n", errout_str());

        valueFlowUninit("void f() {\n"
                        "  int a[10][10];\n"
                        "  dostuff(*a);\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n"
                        "    void (*fp[1]) (void) = {function1};\n"
                        "    (*fp[0])();\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("template <typename T, int value> T Get() {return value;}\n"
                        "char f() {\n"
                        "  char buf[10];\n"
                        "  for(int i = 0; i < Get<int,10>() ; ++i) \n"
                        "     buf[i] = 0;\n"
                        "  return buf[0];\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("static void Foo(double* p) {\n"
                        " p[0] = 0;\n"
                        " p[1] = 0;\n"
                        " p[2] = 0;\n"
                        " p[3] = 0;\n"
                        "}\n"
                        "double f() {\n"
                        "  double L[2][2];\n"
                        "  Foo(*L);\n"
                        "  return L[0][0];\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("void f() {\n" // #11305
                        "    type_t a;\n"
                        "    a[0] = 0;\n"
                        "}\n", false);
        ASSERT_EQUALS("", errout_str());
    }

    void uninitvar_memberaccess() {
        valueFlowUninit("struct foo{char *bar;};\n"
                        "void f(unsigned long long *p) {\n"
                        "    foo a;\n"
                        "    ((&a)->bar) =  reinterpret_cast<char*>(*p);\n"
                        "    if ((&a)->bar) ;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct foo{char *bar;};\n"
                        "void f(unsigned long long *p) {\n"
                        "    foo a;\n"
                        "    ((&(a))->bar) =  reinterpret_cast<char*>(*p);\n"
                        "    if ((&a)->bar) ;\n"
                        "}");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct A {\n" // #10200
                        "    struct B {\n"
                        "        int i;\n"
                        "    };\n"
                        "    int j;\n"
                        "};\n"
                        "void f(std::vector<A::B>& x) {\n"
                        "    A::B b;\n"
                        "    b.i = 123;\n"
                        "    x.push_back(b);\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct A {\n"
                        "    struct B {\n"
                        "        int i;\n"
                        "    };\n"
                        "    int j;\n"
                        "};\n"
                        "void f(std::vector<A::B>& x) {\n"
                        "    A::B b;\n"
                        "    x.push_back(b);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:9:17]: (error) Uninitialized variable: b [uninitvar]\n", errout_str());

        valueFlowUninit("struct A {\n"
                        "    struct B {\n"
                        "        int i;\n"
                        "    };\n"
                        "    int j;\n"
                        "};\n"
                        "void f(std::vector<A>&x) {\n"
                        "    A a;\n"
                        "    a.j = 123;\n"
                        "    x.push_back(a);\n"
                        "}\n");

        valueFlowUninit("struct A {\n"
                        "    struct B {\n"
                        "        int i;\n"
                        "    };\n"
                        "    int j;\n"
                        "};\n"
                        "void f(std::vector<A>& x) {\n"
                        "    A a;\n"
                        "    x.push_back(a);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:9:17]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        valueFlowUninit("struct S { struct T { int* p; } t[2]; };\n" // #11018
                        "void f() {\n"
                        "    S s;\n"
                        "    *&s.t[0].p = 0;\n"
                        "}\n"
                        "void g() {\n"
                        "    S s;\n"
                        "    ((*&(*&s.t[0].p))) = 0;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S { int i; };\n" // #6323
                        "void f() {\n"
                        "    struct S s;\n"
                        "    int x = -3;\n"
                        "    int y = x < (1, s.i);\n"
                        "}\n");
        ASSERT_EQUALS("[test.cpp:5:23]: (error) Uninitialized variable: s.i [uninitvar]\n", errout_str());

        valueFlowUninit("struct S { int x; };\n" // #11353
                        "struct S f() {\n"
                        "    struct S s;\n"
                        "    int* p = &s.x;\n"
                        "    *p = 0;\n"
                        "    return s;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());

        valueFlowUninit("struct S { int i; };\n" // #14191
                        "bool g(S*);\n"
                        "void f( struct S *p) {\n"
                        "    struct S s;\n"
                        "    if (!p) {\n"
                        "        p = &s;\n"
                        "        if (g(p))\n"
                        "            return;\n"
                        "    }\n"
                        "    printf(\"%i\", p->i);\n"
                        "    p = &s;\n"
                        "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void ctuTest() {
        ctu("void f(int *p) {\n"
            "    a = *p;\n"
            "}\n"
            "int main() {\n"
            "  int x;\n"
            "  f(&x);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:6:4] -> [test.cpp:2:10]: (error) Using argument p that points at uninitialized variable x [ctuuninitvar]\n", errout_str());

        ctu("void use(int *p) { a = *p + 3; }\n"
            "void call(int x, int *p) { x++; use(p); }\n"
            "int main() {\n"
            "  int x;\n"
            "  call(4,&x);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:5:7] -> [test.cpp:2:33] -> [test.cpp:1:25]: (error) Using argument p that points at uninitialized variable x [ctuuninitvar]\n", errout_str());

        ctu("void dostuff(int *x, int *y) {\n"
            "  if (!var)\n"
            "    return -1;\n"  // <- early return
            "  *x = *y;\n"
            "}\n"
            "\n"
            "void f() {\n"
            "  int x;\n"
            "  dostuff(a, &x);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        ctu("void dostuff(int *x, int *y) {\n"
            "  if (cond)\n"
            "    *y = -1;\n"  // <- conditionally written
            "  *x = *y;\n"
            "}\n"
            "\n"
            "void f() {\n"
            "  int x;\n"
            "  dostuff(a, &x);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        ctu("void f(int *p) {\n"
            "    a = sizeof(*p);\n"
            "}\n"
            "int main() {\n"
            "  int x;\n"
            "  f(&x);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        ctu("void f(int *v) {\n"
            "  std::cin >> *v;\n"
            "}\n"
            "int main() {\n"
            "  int x;\n"
            "  f(&x);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        ctu("void increment(int& i) { ++i; }\n" // #6475
            "int f() {\n"
            "    int n;\n"
            "    increment(n);\n"
            "    return n;\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4:14] -> [test.cpp:1:28]: (error) Using argument i that points at uninitialized variable n [ctuuninitvar]\n", errout_str());

        ctu("void increment(int* i) { ++(*i); }\n"
            "int f() {\n"
            "    int n;\n"
            "    increment(&n);\n"
            "    return n;\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4:14] -> [test.cpp:1:30]: (error) Using argument i that points at uninitialized variable n [ctuuninitvar]\n", errout_str());

        ctu("typedef struct { int type; int id; } Stem;\n"
            "void lookupStem(recodeCtx h, Stem *stem) {\n"
            "    i = stem->type & STEM_VERT;\n"
            "}\n"
            "void foo() {\n"
            "    Stem stem;\n"
            "    stem.type = 0;\n"
            "    lookupStem(h, &stem);\n"
            "}\n");
        ASSERT_EQUALS("", errout_str());

        ctu("void increment(int& i) { ++i; }\n" // #6475
            "int f() {\n"
            "    int n;\n"
            "    increment(n);\n"
            "    return n;\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4:14] -> [test.cpp:1:28]: (error) Using argument i that points at uninitialized variable n [ctuuninitvar]\n", errout_str());
    }
};

REGISTER_TEST(TestUninitVar)
