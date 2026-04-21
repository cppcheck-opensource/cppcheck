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
#include "checknullpointer.h"
#include "ctu.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "library.h"
#include "settings.h"
#include "standards.h"
#include "token.h"

#include <cstddef>
#include <list>
#include <string>

class TestNullPointer : public TestFixture {
public:
    TestNullPointer() : TestFixture("TestNullPointer") {}

private:
    const Settings settings = settingsBuilder().library("std.cfg").severity(Severity::warning).build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(nullpointerAfterLoop);
        TEST_CASE(nullpointer1);
        TEST_CASE(nullpointer2);
        TEST_CASE(structDerefAndCheck);    // dereferencing struct and then checking if it's null
        TEST_CASE(pointerDerefAndCheck);
        TEST_CASE(nullpointer5);    // References should not be checked
        TEST_CASE(nullpointerExecutionPaths);
        TEST_CASE(nullpointerExecutionPathsLoop);
        TEST_CASE(nullpointer7);
        TEST_CASE(nullpointer9);
        TEST_CASE(nullpointer10);
        TEST_CASE(nullpointer11); // ticket #2812
        TEST_CASE(nullpointer12); // ticket #2470
        TEST_CASE(nullpointer15); // #3560 (fp: return p ? f(*p) : f(0))
        TEST_CASE(nullpointer16); // #3591
        TEST_CASE(nullpointer17); // #3567
        TEST_CASE(nullpointer18); // #1927
        TEST_CASE(nullpointer19); // #3811
        TEST_CASE(nullpointer20); // #3807 (fp: return p ? (p->x() || p->y()) : z)
        TEST_CASE(nullpointer21); // #4038 (fp: if (x) p=q; else return;)
        TEST_CASE(nullpointer23); // #4665 (false positive)
        TEST_CASE(nullpointer24); // #5082 fp: chained assignment
        TEST_CASE(nullpointer25); // #5061
        TEST_CASE(nullpointer26); // #3589
        TEST_CASE(nullpointer27); // #6568
        TEST_CASE(nullpointer28); // #6491
        TEST_CASE(nullpointer30); // #6392
        TEST_CASE(nullpointer31); // #8482
        TEST_CASE(nullpointer32); // #8460
        TEST_CASE(nullpointer33);
        TEST_CASE(nullpointer34);
        TEST_CASE(nullpointer35);
        TEST_CASE(nullpointer36); // #9264
        TEST_CASE(nullpointer37); // #9315
        TEST_CASE(nullpointer38);
        TEST_CASE(nullpointer39); // #2153
        TEST_CASE(nullpointer40);
        TEST_CASE(nullpointer41);
        TEST_CASE(nullpointer42);
        TEST_CASE(nullpointer43); // #9404
        TEST_CASE(nullpointer44); // #9395, #9423
        TEST_CASE(nullpointer45);
        TEST_CASE(nullpointer46); // #9441
        TEST_CASE(nullpointer47); // #6850
        TEST_CASE(nullpointer48); // #9196
        TEST_CASE(nullpointer49); // #7804
        TEST_CASE(nullpointer50); // #6462
        TEST_CASE(nullpointer51);
        TEST_CASE(nullpointer52);
        TEST_CASE(nullpointer53); // #8005
        TEST_CASE(nullpointer54); // #9573
        TEST_CASE(nullpointer55); // #8144
        TEST_CASE(nullpointer56); // #9701
        TEST_CASE(nullpointer57); // #9751
        TEST_CASE(nullpointer58); // #9807
        TEST_CASE(nullpointer59); // #9897
        TEST_CASE(nullpointer60); // #9842
        TEST_CASE(nullpointer61);
        TEST_CASE(nullpointer62);
        TEST_CASE(nullpointer63);
        TEST_CASE(nullpointer64);
        TEST_CASE(nullpointer65); // #9980
        TEST_CASE(nullpointer66); // #10024
        TEST_CASE(nullpointer67); // #10062
        TEST_CASE(nullpointer68);
        TEST_CASE(nullpointer69); // #8143
        TEST_CASE(nullpointer70);
        TEST_CASE(nullpointer71); // #10178
        TEST_CASE(nullpointer72); // #10215
        TEST_CASE(nullpointer73); // #10321
        TEST_CASE(nullpointer74);
        TEST_CASE(nullpointer75);
        TEST_CASE(nullpointer76); // #10408
        TEST_CASE(nullpointer77);
        TEST_CASE(nullpointer78); // #7802
        TEST_CASE(nullpointer79); // #10400
        TEST_CASE(nullpointer80); // #10410
        TEST_CASE(nullpointer81); // #8724
        TEST_CASE(nullpointer82); // #10331
        TEST_CASE(nullpointer83); // #9870
        TEST_CASE(nullpointer84); // #9873
        TEST_CASE(nullpointer85); // #10210
        TEST_CASE(nullpointer86);
        TEST_CASE(nullpointer87); // #9291
        TEST_CASE(nullpointer88); // #9949
        TEST_CASE(nullpointer89); // #10640
        TEST_CASE(nullpointer90); // #6098
        TEST_CASE(nullpointer91); // #10678
        TEST_CASE(nullpointer92);
        TEST_CASE(nullpointer93); // #3929
        TEST_CASE(nullpointer94); // #11040
        TEST_CASE(nullpointer95); // #11142
        TEST_CASE(nullpointer96); // #11416
        TEST_CASE(nullpointer97); // #11229
        TEST_CASE(nullpointer98); // #11458
        TEST_CASE(nullpointer99); // #10602
        TEST_CASE(nullpointer100);        // #11636
        TEST_CASE(nullpointer101);        // #11382
        TEST_CASE(nullpointer102);
        TEST_CASE(nullpointer103);
        TEST_CASE(nullpointer104); // #13881
        TEST_CASE(nullpointer105); // #13861
        TEST_CASE(nullpointer_addressOf); // address of
        TEST_CASE(nullpointerSwitch); // #2626
        TEST_CASE(nullpointer_cast); // #4692
        TEST_CASE(nullpointer_castToVoid); // #3771
        TEST_CASE(nullpointer_subfunction);
        TEST_CASE(pointerCheckAndDeRef);     // check if pointer is null and then dereference it
        TEST_CASE(nullConstantDereference);  // Dereference NULL constant
        TEST_CASE(gcc_statement_expression); // Don't crash
        TEST_CASE(snprintf_with_zero_size);
        TEST_CASE(snprintf_with_non_zero_size);
        TEST_CASE(printf_with_invalid_va_argument);
        TEST_CASE(scanf_with_invalid_va_argument);
        TEST_CASE(nullpointer_in_return);
        TEST_CASE(nullpointer_in_typeid);
        TEST_CASE(nullpointer_in_alignof); // #11401
        TEST_CASE(nullpointer_in_for_loop);
        TEST_CASE(nullpointerDeadCode); // #11311
        TEST_CASE(nullpointerDelete);
        TEST_CASE(nullpointerSubFunction);
        TEST_CASE(nullpointerExit);
        TEST_CASE(nullpointerStdString);
        TEST_CASE(nullpointerStdStream);
        TEST_CASE(nullpointerSmartPointer);
        TEST_CASE(nullpointerOutOfMemory);
        TEST_CASE(nullpointerOutOfResources);
        TEST_CASE(functioncall);
        TEST_CASE(functioncalllibrary); // use Library to parse function call
        TEST_CASE(functioncallDefaultArguments);
        TEST_CASE(nullpointer_internal_error); // #5080
        TEST_CASE(ticket6505);
        TEST_CASE(subtract);
        TEST_CASE(addNull);
        TEST_CASE(isPointerDeRefFunctionDecl);

        TEST_CASE(ctuTest);
    }

    struct CheckOptions
    {
        bool inconclusive = false;
        bool cpp = true;
        Standards::cstd_t cstd = Standards::CLatest;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings settings1 = settingsBuilder(settings).certainty(Certainty::inconclusive, options.inconclusive).c(options.cstd).build();

        // Tokenize..
        SimpleTokenizer tokenizer(settings1, *this, options.cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for null pointer dereferences..
        runChecks<CheckNullPointer>(tokenizer, this);
    }

#define checkP(...) checkP_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkP_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer2 tokenizer(settings, *this, code, "test.cpp");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check for null pointer dereferences..
        runChecks<CheckNullPointer>(tokenizer, this);
    }



    void nullpointerAfterLoop() {
        // extracttests.start: struct Token { const Token *next() const; std::string str() const; };
        check("void foo(const Token *tok)\n"
              "{\n"
              "    while (tok);\n"
              "    tok = tok->next();\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:12] -> [test.cpp:4:11]: (warning) Either the condition 'tok' is redundant or there is possible null pointer dereference: tok. [nullPointerRedundantCheck]\n", errout_str());

        // #2681
        {
            const char code[] = "void foo(const Token *tok)\n"
                                "{\n"
                                "    while (tok && tok->str() == \"=\")\n"
                                "        tok = tok->next();\n"
                                "\n"
                                "    if (tok->str() != \";\")\n"
                                "        ;\n"
                                "}\n";

            check(code);
            ASSERT_EQUALS("[test.cpp:3:12] -> [test.cpp:6:9]: (warning) Either the condition 'tok' is redundant or there is possible null pointer dereference: tok. [nullPointerRedundantCheck]\n", errout_str());
        }

        check("void foo()\n"
              "{\n"
              "    for (const Token *tok = tokens; tok; tok = tok->next())\n"
              "    {\n"
              "        while (tok && tok->str() != \";\")\n"
              "            tok = tok->next();\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:3]: (warning) Either the condition 'while' is redundant or there is possible null pointer dereference: tok.\n", "", errout_str());

        check("void foo(Token &tok)\n"
              "{\n"
              "    for (int i = 0; i < tok.size(); i++ )\n"
              "    {\n"
              "        while (!tok)\n"
              "            char c = tok.read();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    for (const Token *tok = tokens; tok; tok = tok->next())\n"
              "    {\n"
              "        while (tok && tok->str() != \";\")\n"
              "            tok = tok->next();\n"
              "        if( !tok ) break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    for (const Token *tok = tokens; tok; tok = tok ? tok->next() : NULL)\n"
              "    {\n"
              "        while (tok && tok->str() != \";\")\n"
              "            tok = tok->next();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(A*a)\n"
              "{\n"
              "  switch (a->b()) {\n"
              "    case 1:\n"
              "      while( a ){\n"
              "        a = a->next;\n"
              "      }\n"
              "    break;\n"
              "    case 2:\n"
              "      a->b();\n"
              "      break;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // dereference in outer scope..
        check("void foo(int x, const Token *tok) {\n"
              "    if (x == 123) {\n"
              "        while (tok) tok = tok->next();\n"
              "    }\n"
              "    tok->str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:16] -> [test.cpp:5:5]: (warning) Either the condition 'tok' is redundant or there is possible null pointer dereference: tok. [nullPointerRedundantCheck]\n", errout_str());

        check("int foo(const Token *tok)\n"
              "{\n"
              "    while (tok){;}\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("int foo(const Token *tok)\n"
              "{\n"
              "    while (tok){;}\n"
              "    char a[2] = {0,0};\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct b {\n"
              "    b * c;\n"
              "    int i;\n"
              "}\n"
              "void a(b * e) {\n"
              "  for (b *d = e;d; d = d->c)\n"
              "    while (d && d->i == 0)\n"
              "      d = d->c;\n"
              "  if (!d) throw;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct b {\n"
              "    b * c;\n"
              "    int i;\n"
              "};\n"
              "void f(b* e1, b* e2) {\n"
              "    for (const b* d = e1; d != e2; d = d->c) {\n"
              "        if (d && d->i != 0) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:13] -> [test.cpp:6:40]: (warning) Either the condition 'd' is redundant or there is possible null pointer dereference: d. [nullPointerRedundantCheck]\n", errout_str());
    }

    void nullpointer1() {
        // ticket #1923 - no false positive when using else if
        check("void f(A *a)\n"
              "{\n"
              "    if (a->x == 1)\n"
              "    {\n"
              "        a = a->next;\n"
              "    }\n"
              "    else if (a->x == 2) { }\n"
              "    if (a) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #2134 - sizeof doesn't dereference
        check("void f() {\n"
              "    int c = 1;\n"
              "    int *list = NULL;\n"
              "    sizeof(*list);\n"
              "    if (!list)\n"
              "        ;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // ticket #2245 - sizeof doesn't dereference
        check("void f(Bar *p) {\n"
              "    if (!p) {\n"
              "        int sz = sizeof(p->x);\n"
              "    }\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

    }

    void nullpointer2() {
        // Null pointer dereference can only happen with pointers
        check("void foo()\n"
              "{\n"
              "    Fred fred;\n"
              "    while (fred);\n"
              "    fred.hello();\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    // Dereferencing a struct and then checking if it is null
    // This is checked by this function:
    //        CheckOther::nullPointerStructByDeRefAndCheck
    void structDerefAndCheck() {
        // extracttests.start: struct ABC { int a; int b; int x; };

        // errors..
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    int a = abc->a;\n"
              "    if (!abc)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:13]: (warning) Either the condition '!abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(struct ABC *abc) {\n"
              "    bar(abc->a);\n"
              "    bar(x, abc->a);\n"
              "    bar(x, y, abc->a);\n"
              "    if (!abc)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:2:9]: (warning) Either the condition '!abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n"
                      "[test.cpp:5:9] -> [test.cpp:3:12]: (warning) Either the condition '!abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n"
                      "[test.cpp:5:9] -> [test.cpp:4:15]: (warning) Either the condition '!abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(ABC *abc) {\n"
              "    if (abc->a == 3) {\n"
              "        return;\n"
              "    }\n"
              "    if (abc) {}\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:5:9] -> [test.cpp:2:9]: (warning) Either the condition 'abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n",
            errout_str());

        check("void f(ABC *abc) {\n"
              "    if (abc->x == 0) {\n"
              "        return;\n"
              "    }\n"
              "    if (!abc);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:2:9]: (warning) Either the condition '!abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n", errout_str());

        // TODO: False negative if member of member is dereferenced
        check("void foo(ABC *abc) {\n"
              "    abc->next->a = 0;\n"
              "    if (abc->next)\n"
              "        ;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (warning) Possible null pointer dereference: abc - otherwise it is redundant to check it against null.\n", "", errout_str());

        check("void foo(ABC *abc) {\n"
              "    abc->a = 0;\n"
              "    if (abc && abc->b == 0)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:9] -> [test.cpp:2:5]: (warning) Either the condition 'abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n",
            errout_str());

        // ok dereferencing in a condition
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    if (abc && abc->a);\n"
              "    if (!abc)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(struct ABC *abc) {\n"
              "    int x = abc && a(abc->x);\n"
              "    if (abc) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ok to use a linked list..
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    abc = abc->next;\n"
              "    if (!abc)\n"
              "        ;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(struct ABC *abc) {\n"
              "    abc = (ABC *)(abc->_next);\n"
              "    if (abc) { }"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // reassign struct..
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    int a = abc->a;\n"
              "    abc = abc->next;\n"
              "    if (!abc)\n"
              "        ;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    int a = abc->a;\n"
              "    f(&abc);\n"
              "    if (!abc)\n"
              "        ;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // goto..
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    int a;\n"
              "    if (!abc)\n"
              "        goto out;"
              "    a = abc->a;\n"
              "    return;\n"
              "out:\n"
              "    if (!abc)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // loops..
        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    int a = abc->a;"
              "    do\n"
              "    {\n"
              "        if (abc)\n"
              "            abc = abc->next;\n"
              "        --a;\n"
              "    }\n"
              "    while (a > 0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())\n"
              "    {\n"
              "        while (tok && tok->str() != \"{\")\n"
              "            tok = tok->next();\n"
              "        if (!tok)\n"
              "            return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // dynamic_cast..
        check("void foo(ABC *abc)\n"
              "{\n"
              "    int a = abc->a;\n"
              "    if (!dynamic_cast<DEF *>(abc))\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #2641 - global pointer, function call
        check("ABC *abc;\n"
              "void f() {\n"
              "    abc->a = 0;\n"
              "    do_stuff();\n"
              "    if (abc) { }\n"
              "}");
        ASSERT_EQUALS("",errout_str());

        check("Fred *fred;\n"
              "void f() {\n"
              "    fred->foo();\n"
              "    if (fred) { }\n"
              "}");
        ASSERT_EQUALS("",errout_str());

        // #2641 - local pointer, function call
        check("void f() {\n"
              "    ABC *abc = abc1;\n"
              "    abc->a = 0;\n"
              "    do_stuff();\n"
              "    if (abc) { }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:5:9] -> [test.cpp:3:5]: (warning) Either the condition 'abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n",
            errout_str());

        // #2641 - local pointer, function call
        check("void f(ABC *abc) {\n"
              "    abc->a = 0;\n"
              "    do_stuff();\n"
              "    if (abc) { }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:9] -> [test.cpp:2:5]: (warning) Either the condition 'abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n",
            errout_str());

        // #2691 - switch/break
        check("void f(ABC *abc) {\n"
              "    switch ( x ) {\n"
              "        case 14:\n"
              "            sprintf(buf, \"%d\", abc->a);\n"
              "            break;\n"
              "        case 15:\n"
              "            if ( abc ) {}\n"
              "            break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3128
        check("void f(ABC *abc) {\n"
              "    x(!abc || y(abc->a));\n"
              "    if (abc) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(ABC *abc) {\n"
              "  x(def || !abc || y(def, abc->a));\n"
              "  if (abc) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(ABC *abc) {\n"
              "  x(abc && y(def, abc->a));\n"
              "  if (abc) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(ABC *abc) {\n"
              "    x(def && abc && y(def, abc->a));\n"
              "    if (abc) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3228 - calling function with null object
        {
            const char code[] = "void f(Fred *fred) {\n"
                                "    fred->x();\n"
                                "    if (fred) { }\n"
                                "}";
            check(code);
            ASSERT_EQUALS(
                "[test.cpp:3:9] -> [test.cpp:2:5]: (warning) Either the condition 'fred' is redundant or there is possible null pointer dereference: fred. [nullPointerRedundantCheck]\n",
                errout_str());
        }

        // #3425 - false positives when there are macros
        checkP("#define IF if\n"
               "void f(struct FRED *fred) {\n"
               "    fred->x = 0;\n"
               "    IF(!fred){}\n"
               "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "  BUFFER *buffer = get_buffer();\n"
              "  if (!buffer)\n"
              "    uv_fatal_error();\n"
              "  buffer->x = 11;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // Dereferencing a pointer and then checking if it is null
    void pointerDerefAndCheck() {
        // extracttests.start: void bar(int);

        // errors..
        check("void foo(int *p)\n"
              "{\n"
              "    *p = 0;\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:6]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(int *p)\n"
              "{\n"
              "    *p = 0;\n"
              "    if (p) { }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:9] -> [test.cpp:3:6]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
            errout_str());

        check("void foo(int *p)\n"
              "{\n"
              "    *p = 0;\n"
              "    if (p || q) { }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:9] -> [test.cpp:3:6]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
            errout_str());

        check("void foo(int *p)\n"
              "{\n"
              "    bar(*p);\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:10]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(char *p)\n"
              "{\n"
              "    strcpy(p, \"abc\");\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:12]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(char *p)\n"
              "{\n"
              "    if (*p == 0) { }\n"
              "    if (!p) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:10]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        // no error
        check("void foo()\n"
              "{\n"
              "    int *p;\n"
              "    f(&p);\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    int **p = f();\n"
              "    if (!p)\n"
              "        ;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo(int *p)\n"
              "{\n"
              "    if (x)\n"
              "        p = 0;\n"
              "    else\n"
              "        *p = 0;\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int x)\n"
              "{\n"
              "    int a = 2 * x;"
              "    if (x == 0)\n"
              "        ;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo(int *p)\n"
              "{\n"
              "    int var1 = p ? *p : 0;\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int *p, bool x)\n"
              "{\n"
              "    int var1 = x ? *p : 5;\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:9] -> [test.cpp:3:21]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
            errout_str());

        // while
        check("void f(int *p) {\n"
              "    *p = 0;\n"
              "    while (p) { p = 0; }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p) {\n"
              "    *p = 0;\n"
              "    while (p) { }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:2:6]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
            errout_str());

        // Ticket #3125
        check("void foo(ABC *p)\n"
              "{\n"
              "    int var1 = p ? (p->a) : 0;\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(ABC *p)\n"
              "{\n"
              "    int var1 = p ? (1 + p->a) : 0;\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int * a=0;\n"
              "    if (!a) {};\n"
              "    int c = a ? 0 : 1;\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #3686
        check("void f() {\n"
              "    int * a=0;\n"
              "    if (!a) {};\n"
              "    int c = a ? b : b+1;\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int * a=0;\n"
              "    if (!a) {};\n"
              "    int c = (a) ? b : b+1;\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo(P *p)\n"
              "{\n"
              "  while (p)\n"
              "    if (p->check())\n"
              "      break;\n"
              "    else\n"
              "      p = p->next();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(Document *doc) {\n"
              "    int x = doc && doc->x;\n"
              "    if (!doc) {\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3128 - false positive
        check("void f(int *p) {\n"
              "    assert(!p || (*p<=6));\n"
              "    if (p) { *p = 0; }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p) {\n"
              "    assert(p && (*p<=6));\n"
              "    if (p) { *p = 0; }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p) {\n"
              "    *p = 12;\n"
              "    assert(p && (*p<=6));\n"
              "    if (p) { *p = 0; }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:2:6]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
            errout_str());

        check("void foo(x *p)\n"
              "{\n"
              "    p = p->next;\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(x *p)\n"
              "{\n"
              "    p = bar(p->next);\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(x *p)\n"
              "{\n"
              "    p = aa->bar(p->next);\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(x *p)\n"
              "{\n"
              "    p = *p2 = p->next;\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(struct ABC *abc)\n"
              "{\n"
              "    abc = abc ? abc->next : 0;\n"
              "    if (!abc)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(struct ABC *abc) {\n" // #4523
              "    abc = (*abc).next;\n"
              "    if (abc) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(struct ABC *abc) {\n" // #4523
              "    abc = (*abc->ptr);\n"
              "    if (abc) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(Item *item) {\n"
              "    x = item ? ab(item->x) : 0;\n"
              "    if (item) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(Item *item) {\n"
              "    item->x = 0;\n"
              "    a = b ? c : d;\n"
              "    if (item) { }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:9] -> [test.cpp:2:5]: (warning) Either the condition 'item' is redundant or there is possible null pointer dereference: item. [nullPointerRedundantCheck]\n",
            errout_str());

        check("BOOL GotoFlyAnchor()\n"  // #2243
              "{\n"
              "    const SwFrm* pFrm = GetCurrFrm();\n"
              "    do {\n"
              "        pFrm = pFrm->GetUpper();\n"
              "    } while( pFrm && !pFrm->IsFlyFrm() );\n"
              "\n"
              "    if( !pFrm )\n"
              "        return FALSE;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #2463
        check("struct A\n"
              "{\n"
              "    B* W;\n"
              "\n"
              "    void f() {\n"
              "        switch (InData) {\n"
              "            case 2:\n"
              "                if (!W) return;\n"
              "                W->foo();\n"
              "                break;\n"
              "            case 3:\n"
              "                f();\n"
              "                if (!W) return;\n"
              "                break;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #2525 - sizeof
        check("void f() {\n"
              "    int *test = NULL;\n"
              "    int c = sizeof(test[0]);\n"
              "    if (!test)\n"
              "        ;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(type* p) {\n" // #4983
              "    x(sizeof p[0]);\n"
              "    if (!p)\n"
              "        ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3023 - checked deref
        check("void f(struct ABC *abc) {\n"
              "  WARN_ON(!abc || abc->x == 0);\n"
              "  if (!abc) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f(struct ABC *abc) {\n"
              "  WARN_ON(!abc || abc->x == 7);\n"
              "  if (!abc) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3425 - false positives when there are macros
        checkP("#define IF if\n"
               "void f(int *p) {\n"
               "    *p = 0;\n"
               "    IF(!p){}\n"
               "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #3914 - false positive
              "    int *p;\n"
              "    ((p=ret()) && (x=*p));\n"
              "    if (p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S { struct T { char c; } *p; };\n" // #6541
              "char f(S* s) { return s->p ? 'a' : s->p->c; }\n");
        ASSERT_EQUALS("[test.cpp:2:24] -> [test.cpp:2:37]: (warning) Either the condition 's->p' is redundant or there is possible null pointer dereference: s->p. [nullPointerRedundantCheck]\n",
                      errout_str());

        check("int f(const int a[]) {\n" // #14544
              "    int i = 0;\n"
              "    if (!a)\n"
              "        a = &i;\n"
              "    return *a;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer5() {
        // errors..
        check("void foo(A &a)\n"
              "{\n"
              " char c = a.c();\n"
              " if (!a)\n"
              "   return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // Execution paths..
    void nullpointerExecutionPaths() {
        // errors..
        check("static void foo()\n"
              "{\n"
              "    Foo *p = 0;\n"
              "    if (a == 1) {\n"
              "        p = new FooBar;\n"
              "    } else { if (a == 2) {\n"
              "        p = new FooCar; } }\n"
              "    p->abcd();\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:8]: (error) Possible null pointer dereference: p\n",
                           "", errout_str());

        check("static void foo() {\n"
              "    int &r = *(int*)0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (error) Null pointer dereference: (int*)0 [nullPointer]\n", errout_str());

        check("static void foo(int x) {\n"
              "    int y = 5 + *(int*)0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (error) Null pointer dereference: (int*)0 [nullPointer]\n", errout_str());

        {
            const char code[] = "static void foo() {\n"
                                "    Foo<int> *abc = 0;\n"
                                "    abc->a();\n"
                                "}\n";

            check(code);
            ASSERT_EQUALS("[test.cpp:3:5]: (error) Null pointer dereference: abc [nullPointer]\n", errout_str());
        }

        check("static void foo() {\n"
              "    std::cout << *(int*)0;"
              "}");
        ASSERT_EQUALS("[test.cpp:2:19]: (error) Null pointer dereference: (int*)0 [nullPointer]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    char *c = 0;\n"
              "    {\n"
              "        delete c;\n"
              "    }\n"
              "    c[0] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Null pointer dereference: c [nullPointer]\n", errout_str());

        check("static void foo() {\n"
              "    if (3 > *(int*)0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (error) Null pointer dereference: (int*)0 [nullPointer]\n", errout_str());

        // no false positive..
        check("static void foo()\n"
              "{\n"
              "    Foo *p = 0;\n"
              "    p = new Foo;\n"
              "    p->abcd();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    int sz = sizeof((*(struct dummy *)0).x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void get_offset(long &offset)\n"
              "{\n"
              "    mystruct * temp; temp = 0;\n"
              "    offset = (long)(&(temp->z));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #1893 - try/catch inside else
        check("int *test(int *Z)\n"
              "{\n"
              "    int *Q=NULL;\n"
              "    if (Z) {\n"
              "        Q = Z;\n"
              "    }\n"
              "    else {\n"
              "        Z = new int;\n"
              "        try {\n"
              "        } catch(...) {\n"
              "        }\n"
              "        Q = Z;\n"
              "    }\n"
              "    *Q=1;\n"
              "    return Q;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int *test(int *Z)\n"
              "{\n"
              "    int *Q=NULL;\n"
              "    if (Z) {\n"
              "        Q = Z;\n"
              "    }\n"
              "    else {\n"
              "        try {\n"
              "        } catch(...) {\n"
              "        }\n"
              "    }\n"
              "    *Q=1;\n"
              "    return Q;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:12:6]: (warning) Possible null pointer dereference: Q [nullPointer]\n", errout_str());

        // Ticket #2052 (false positive for 'else continue;')
        check("void f() {\n"
              "    for (int x = 0; x < 5; ++x) {"
              "        int *p = 0;\n"
              "        if (a(x)) p=b(x);\n"
              "        else continue;\n"
              "        *p = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // function pointer..
        check("void foo()\n"
              "{\n"
              "    void (*f)();\n"
              "    f = 0;\n"
              "    f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5]: (error) Null pointer dereference: f [nullPointer]\n", errout_str());

        check("int* g();\n" // #11007
              "int* f() {\n"
              "    static int* (*fun)() = 0;\n"
              "    if (!fun)\n"
              "        fun = g;\n"
              "    return fun();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // loops..
        check("void f() {\n"
              "    int *p = 0;\n"
              "    for (int i = 0; i < 10; ++i) {\n"
              "        int x = *p + 1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("void f(int a) {\n"
              "    const char *p = 0;\n"
              "    if (a) {\n"
              "        p = \"abcd\";\n"
              "    }\n"
              "    for (int i = 0; i < 3; i++) {\n"
              "        if (a && (p[i] == '1'));\n"
              "    }\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // ticket #2251: taking the address of member
        check("void f() {\n"
              "    Fred *fred = 0;\n"
              "    int x = &fred->x;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // ticket #3220: dereferencing a null pointer is UB
        check("void f() {\n"
              "    Fred *fred = NULL;\n"
              "    fred->do_something();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Null pointer dereference: fred [nullPointer]\n", errout_str());

        // ticket #3570 - parsing of conditions
        {
            check("void f() {\n"
                  "    int *p = NULL;\n"
                  "    if (x)\n"
                  "        p = q;\n"
                  "    if (p && *p) { }\n"
                  "}", dinit(CheckOptions, $.inconclusive = true));
            ASSERT_EQUALS("", errout_str());
            check("void f() {\n"
                  "    int *p = NULL;\n"
                  "    if (x)\n"
                  "        p = q;\n"
                  "    if (!p || *p) { }\n"
                  "}", dinit(CheckOptions, $.inconclusive = true));
            ASSERT_EQUALS("", errout_str());
            check("void f() {\n"
                  "    int *p = NULL;\n"
                  "    if (x)\n"
                  "        p = q;\n"
                  "    if (p || *p) { }\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:5:15]: (warning) Possible null pointer dereference: p [nullPointer]\n", errout_str());
        }

        // ticket #8831 - FP triggered by if/return/else sequence
        {
            check("void f(int *p, int *q) {\n"
                  "    if (p == NULL)\n"
                  "        return;\n"
                  "    else if (q == NULL)\n"
                  "        return;\n"
                  "    *q = 0;\n"
                  "}\n"
                  "\n"
                  "void g() {\n"
                  "    f(NULL, NULL);\n"
                  "}", dinit(CheckOptions, $.inconclusive = true));
            ASSERT_EQUALS("", errout_str());
        }

        check("void f() {\n" // #5979
              "    int* const crash = 0;\n"
              "    *crash = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Null pointer dereference: crash [nullPointer]\n", errout_str());
    }

    // Ticket #2350
    void nullpointerExecutionPathsLoop() {
        // No false positive:
        check("void foo() {\n"
              "    int n;\n"
              "    int *argv32 = p;\n"
              "    if (x) {\n"
              "        n = 0;\n"
              "        argv32 = 0;\n"
              "    }\n"
              "\n"
              "    for (int i = 0; i < n; i++) {\n"
              "        argv32[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // No false negative:
        check("void foo() {\n"
              "    int n;\n"
              "    int *argv32;\n"
              "    if (x) {\n"
              "        n = 10;\n"
              "        argv32 = 0;\n"
              "    }\n"
              "\n"
              "    for (int i = 0; i < n; i++) {\n"
              "        argv32[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:9]: (warning) Possible null pointer dereference: argv32 [nullPointer]\n", errout_str());

        // #2231 - error if assignment in loop is not used
        // extracttests.start: int y[20];
        check("void f() {\n"
              "    char *p = 0;\n"
              "\n"
              "    for (int x = 0; x < 3; ++x) {\n"
              "        if (y[x] == 0) {\n"
              "            p = (char *)malloc(10);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11:6]: (warning) Possible null pointer dereference: p [nullPointer]\n", errout_str());
    }

    void nullpointer7() {
        check("void foo()\n"
              "{\n"
              "  wxLongLong x = 0;\n"
              "  int y = x.GetValue();\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer9() { //#ticket 1778
        check("void foo()\n"
              "{\n"
              "  std::string * x = 0;\n"
              "  *x = \"test\";\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Null pointer dereference: x [nullPointer]\n", errout_str());
    }

    void nullpointer10() {
        // extracttests.start: struct my_type { int x; };
        check("void foo()\n"
              "{\n"
              "  struct my_type* p = 0;\n"
              "  p->x = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:3]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());
    }

    void nullpointer11() { // ticket #2812
        // extracttests.start: struct my_type { int x; };

        check("int foo()\n"
              "{\n"
              "  struct my_type* p;\n"
              "  p = 0;\n"
              "  return p->x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());
    }

    void nullpointer12() { // ticket #2470, #4035
        const char code[] = "int foo()\n"
                            "{\n"
                            "  int* i = nullptr;\n"
                            "  return *i;\n"
                            "}\n";

        check(code); // C++ file => nullptr means NULL
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Null pointer dereference: i [nullPointer]\n", errout_str());

        check(code, dinit(CheckOptions, $.cpp = false, $.cstd = Standards::C17)); // C17 file => nullptr does not mean NULL
        ASSERT_EQUALS("", errout_str());

        check(code, dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:4:11]: (error) Null pointer dereference: i [nullPointer]\n", errout_str());
    }

    void nullpointer15() {  // #3560
        check("void f() {\n"
              "    char *p = 0;\n"
              "    if (x) p = \"abcd\";\n"
              "    return p ? f(*p) : f(0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer16() {  // #3591
        check("void foo() {\n"
              "    int *p = 0;\n"
              "    bar(&p);\n"
              "    *p = 0;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer17() {  // #3567
        check("int foo() {\n"
              "    int *p = 0;\n"
              "    if (x) { return 0; }\n"
              "    return !p || *p;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("int foo() {\n"
              "    int *p = 0;\n"
              "    if (x) { return 0; }\n"
              "    return p && *p;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer18() {  // #1927
        check("void f ()\n"
              "{\n"
              "  int i=0;\n"
              "  char *str=NULL;\n"
              "  while (str[i])\n"
              "  {\n"
              "    i++;\n"
              "  };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Null pointer dereference: str [nullPointer]\n", errout_str());
    }

    void nullpointer19() { // #3811
        check("int foo() {\n"
              "    perror(0);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer20() {  // #3807
        check("void f(int x) {\n"
              "    struct xy *p = 0;\n"
              "    if (x) p = q;\n"
              "    if (p ? p->x || p->y : 0) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"   // false negative
              "    struct xy *p = 0;\n"
              "    if (x) p = q;\n"
              "    if (y ? p->x : p->y) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (warning) Possible null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:4:20]: (warning) Possible null pointer dereference: p [nullPointer]\n",
                      errout_str());
    }

    void nullpointer21() {  // #4038 - fp: if (x) p=q; else return;
        check("void f(int x) {\n"
              "    int *p = 0;\n"
              "    if (x) p = q;\n"
              "    else return;\n"
              "    *p = 0;\n" // <- p is not NULL
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer23() {  // #4665
        check("void f(){\n"
              "    char *c = NULL;\n"
              "    char cBuf[10];\n"
              "    sprintf(cBuf, \"%s\", c ? c : \"0\" );\n"
              "}");
        ASSERT_EQUALS("",errout_str());
    }

    void nullpointer24() {  // #5083 - fp: chained assignment
        check("void f(){\n"
              "    char *c = NULL;\n"
              "    x = c = new char[10];\n"
              "    *c = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer25() { // #5061
        check("void f(int *data, int i)\n"
              "{\n"
              "    int *array = NULL;\n"
              "    if (data == 1 && array[i] == 0)\n"
              "        std::cout << \"test\";\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:22]: (error) Null pointer dereference: array [nullPointer]\n", errout_str());
    }

    void nullpointer26() { // #3589
        check("double foo() {\n"
              "    sk *t1 = foo();\n"
              "    sk *t2 = foo();\n"
              "    if ((!t1) && (!t2))\n"
              "        return 0.0;\n"
              "    if (t1 && (!t2))\n"
              "        return t1->Inter();\n"
              "    if (t2->GetT() == t)\n"
              "        return t2->Inter();\n"
              "    if (t2 && (!t1))\n"
              "        return 0.0;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer27() { // #6568
        check("template<class Type>\n"
              "class Foo {\n"
              "    Foo<Type>& operator = ( Type* );\n"
              "};\n"
              "template<class Type>\n"
              "Foo<Type>& Foo<Type>::operator = ( Type* pointer_ ) {\n"
              "    pointer_=NULL;\n"
              "    *pointer_=0;\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:6]: (error) Null pointer dereference: pointer_ [nullPointer]\n", errout_str());
    }

    void nullpointer28() { // #6491
        check("typedef struct { int value; } S;\n"
              "int f(const S *s) {\n"
              "  int i = s ? s->value + 1\n"
              "            : s->value - 1; // <-- null ptr dereference\n"
              "  return i;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:11] -> [test.cpp:4:15]: (warning) Either the condition 's' is redundant or there is possible null pointer dereference: s. [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer30() { // #6392
        check("void f(std::vector<std::string> *values)\n"
              "{\n"
              "  values->clear();\n"
              "  if (values)\n"
              "  {\n"
              "    for (int i = 0; i < values->size(); ++i)\n"
              "    {\n"
              "      values->push_back(\"test\");\n"
              "    }\n"
              "  }\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:4:7] -> [test.cpp:3:3]: (warning) Either the condition 'values' is redundant or there is possible null pointer dereference: values. [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer31() { // #8482
        check("struct F\n"
              "{\n"
              "    int x;\n"
              "};\n"
              "\n"
              "static void foo(F* f)\n"
              "{\n"
              "    if( f ) {}\n"
              "    else { return; }\n"
              "    (void)f->x;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("typedef struct\n"
              "{\n"
              "    int x;\n"
              "} F;\n"
              "\n"
              "static void foo(F* f)\n"
              "{\n"
              "    if( !f || f->x == 0 )\n"
              "    {\n"
              "        if( !f )\n"
              "            return;\n"
              "    }\n"
              "\n"
              "    (void)f->x;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer32() { // #8460
        check("int f(int * ptr) {\n"
              "  if(ptr)\n"
              "  { return 0;}\n"
              "  else{\n"
              "    int *p1 = ptr;\n"
              "    return *p1;\n"
              "  }\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:6] -> [test.cpp:6:13]: (warning) Either the condition 'ptr' is redundant or there is possible null pointer dereference: p1. [nullPointerRedundantCheck]\n", errout_str());
    }

    void nullpointer33() {
        check("void f(int * x) {\n"
              "    if (x != nullptr)\n"
              "        *x = 2;\n"
              "    else\n"
              "        *x = 3;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:11] -> [test.cpp:5:10]: (warning) Either the condition 'x!=nullptr' is redundant or there is possible null pointer dereference: x. [nullPointerRedundantCheck]\n", errout_str());
    }

    void nullpointer34() {
        check("void g() {\n"
              "    throw " ";\n"
              "}\n"
              "bool f(int * x) {\n"
              "    if (x) *x += 1;\n"
              "    if (!x) g();\n"
              "    return *x;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer35() {
        check("bool f(int*);\n"
              "void g(int* x) {\n"
              "    if (f(x)) {\n"
              "        *x = 1;\n"
              "    }\n"
              "}\n"
              "void h() {\n"
              "    g(0);\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool f(int*);\n"
              "void g(int* x) {\n"
              "    bool b = f(x);\n"
              "    if (b) {\n"
              "        *x = 1;\n"
              "    }\n"
              "}\n"
              "void h() {\n"
              "    g(0);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer36() {
        check("char* f(char* s) {\n"
              "    char* start = s;\n"
              "    if (!s)\n"
              "        return (s);\n"
              "    while (isspace(*start))\n"
              "        start++;\n"
              "    return (start);\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer37() {
        check("void f(int value, char *string) {\n"
              "    char *ptr1 = NULL, *ptr2 = NULL;\n"
              "    unsigned long count = 0;\n"
              "    if(!string)\n"
              "        return;\n"
              "    ptr1 = string;\n"
              "    ptr2 = strrchr(string, 'a');\n"
              "    if(ptr2 == NULL)\n"
              "        return;\n"
              "    while(ptr1 < ptr2) {\n"
              "        count++;\n"
              "        ptr1++;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer38() {
        check("void f(int * x) {\n"
              "    std::vector<int*> v;\n"
              "    if (x) {\n"
              "        v.push_back(x);\n"
              "        *x;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer39() {
        check("struct A { int * x; };\n"
              "void f(struct A *a) {\n"
              "    if (a->x == NULL) {}\n"
              "    *(a->x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:14] -> [test.cpp:4:8]: (warning) Either the condition 'a->x==NULL' is redundant or there is possible null pointer dereference: a->x. [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer40() {
        check("struct A { std::unique_ptr<int> x; };\n"
              "void f(struct A *a) {\n"
              "    if (a->x == nullptr) {}\n"
              "    *(a->x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:14] -> [test.cpp:4:8]: (warning) Either the condition 'a->x==nullptr' is redundant or there is possible null pointer dereference: a->x. [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer41() {
        check("struct A { int * g() const; };\n"
              "void f(struct A *a) {\n"
              "    if (a->g() == nullptr) {}\n"
              "    *(a->g());\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:16] -> [test.cpp:4:11]: (warning) Either the condition 'a->g()==nullptr' is redundant or there is possible null pointer dereference: a->g(). [nullPointerRedundantCheck]\n",
            errout_str());

        check("struct A { int * g(); };\n"
              "void f(struct A *a) {\n"
              "    if (a->g() == nullptr) {}\n"
              "    *(a->g());\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer42() {
        check("struct A { std::unique_ptr<int> g() const; };\n"
              "void f(struct A *a) {\n"
              "    if (a->g() == nullptr) {}\n"
              "    *(a->g());\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:16] -> [test.cpp:4:11]: (warning) Either the condition 'a->g()==nullptr' is redundant or there is possible null pointer dereference: a->g(). [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer43() {
        check("struct A { int* x; };\n"
              "void f(A* a) {\n"
              "    int * x = a->x;\n"
              "    if (x) {\n"
              "        (void)*a->x;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer44() {
        // #9395
        check("int foo( ) {\n"
              "    const B* b = getB();\n"
              "    const double w = ( nullptr != b) ? 42. : 0.0;\n"
              "    if ( w == 0.0 )\n"
              "        return 0;\n"
              "    return b->get();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        // #9423
        check("extern F* GetF();\n"
              "extern L* GetL();\n"
              "void Foo() {\n"
              "    const F* const fPtr = GetF();\n"
              "    const bool fPtrOk = fPtr != NULL;\n"
              "    assert(fPtrOk);\n"
              "    if (!fPtrOk)\n"
              "        return;\n"
              "    L* const lPtr = fPtr->l;\n"
              "    const bool lPtrOk = lPtr != NULL;\n"
              "    assert(lPtrOk);\n"
              "    if (!lPtrOk)\n"
              "        return;\n"
              "    lPtr->Clear();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer45() {
        check("struct a {\n"
              "  a *b() const;\n"
              "};\n"
              "void g() { throw 0; }\n"
              "a h(a * c) {\n"
              "  if (c && c->b()) {}\n"
              "  if (!c)\n"
              "    g();\n"
              "  if (!c->b())\n"
              "    g();\n"
              "  a d = *c->b();\n"
              "  return d;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct a {\n"
              "  a *b() const;\n"
              "};\n"
              "void e() { throw 0; }\n"
              "a f() {\n"
              "  a *c = 0;\n"
              "  if (0 && c->b()) {}\n"
              "  if (!c)\n"
              "    e();\n"
              "  if (!c->b())\n"
              "    e();\n"
              "  a d = *c->b();\n"
              "  return d;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer46() {
        check("void f() {\n"
              "    char* p = new(std::nothrow) char[1];\n"
              "    if( p ) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer47() {
        check("void f(int *p) {\n"
              "   if(!p[0]) {}\n"
              "   const int *const a = p;\n"
              "   if(!a){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7] -> [test.cpp:2:8]: (warning) Either the condition '!a' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());
    }

    void nullpointer48() {
        check("template<class T>\n"
              "auto f(T& x) -> decltype(x);\n"
              "int& g(int* x) {\n"
              "    return f(*x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer49() {
        check("void f(int *p, int n) {\n"
              "    int *q = 0;\n"
              "    if(n > 10) q = p;\n"
              "    *p +=2;\n"
              "    if(n < 120) *q+=12;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:18]: (warning) Possible null pointer dereference: q [nullPointer]\n", errout_str());

        check("void f(int *p, int n) {\n"
              "    int *q = 0;\n"
              "    if(n > 10) q = p;\n"
              "    *p +=2;\n"
              "    if(n > 10) *q+=12;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer50() {
        check("void f(int *p, int a) {\n"
              "    if(!p) {\n"
              "        if(a > 0) {\n"
              "            if(a > 10){}\n"
              "            else {\n"
              "                *p = 0;\n"
              "            }\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:8] -> [test.cpp:6:18]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer51() {
        check("struct a {\n"
              "  a *b();\n"
              "};\n"
              "bool c(a *, const char *);\n"
              "a *d(a *e) {\n"
              "  if (e) {}\n"
              "  if (c(e, \"\"))\n"
              "    return nullptr;\n"
              "  return e->b();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer52() {
        check("int f(int a, int* b) {\n"
              "    int* c = nullptr;\n"
              "    if(b) c = b;\n"
              "    if (!c) c = &a;\n"
              "    return *c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int a, int* b) {\n"
              "    int* c = nullptr;\n"
              "    if(b) c = b;\n"
              "    bool d = !c;\n"
              "    if (d) c = &a;\n"
              "    return *c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int* x; };\n"
              "int f(int a, int* b) {\n"
              "    A c;\n"
              "    c.x = nullptr;\n"
              "    if(b) c.x = b;\n"
              "    if (!c.x) c.x = &a;\n"
              "    return *c.x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int* x; };\n"
              "int f(int a, int* b) {\n"
              "    A c;\n"
              "    c.x = nullptr;\n"
              "    if(b) c.x = b;\n"
              "    bool d = !c.x;\n"
              "    if (d) c.x = &a;\n"
              "    return *c.x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int* x; };\n"
              "int f(int a, int* b) {\n"
              "    A c;\n"
              "    c.x = nullptr;\n"
              "    if(b) c.x = b;\n"
              "    bool d = !c.x;\n"
              "    if (!d) c.x = &a;\n"
              "    return *c.x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:14]: (warning) Possible null pointer dereference: c.x [nullPointer]\n", errout_str());
    }

    void nullpointer53() {
        check("void f(int nParams, int* params) {\n"
              "  for (int n=1; n<nParams+10; ++n) {\n"
              "    params[n]=42;\n"
              "  }\n"
              "}\n"
              "void bar() {\n"
              "  f(0, 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Possible null pointer dereference: params [nullPointer]\n", errout_str());
    }

    void nullpointer54() {
        check("int foo (int **array, size_t n_array) {\n"
              "    size_t i;\n"
              "    for (i = 0; i < n_array; ++i) {\n"
              "        if (*array[i] == 1)\n"
              "            return 1;\n"
              "    }\n"
              "    return 0;\n"
              "}\n"
              "int bar() {\n"
              "    int **array = NULL;\n"
              "    foo (array, 0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer55() {
        check("void f(const Token* tok) {\n"
              "    const Token* tok3 = tok;\n"
              "    while (tok3->astParent() && tok3->str() == \",\")\n"
              "        tok3 = tok3->astParent();\n"
              "    if (tok3 && tok3->str() == \"(\") {}\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:5:9] -> [test.cpp:3:12]: (warning) Either the condition 'tok3' is redundant or there is possible null pointer dereference: tok3. [nullPointerRedundantCheck]\n",
            errout_str());

        check("void f(int* t1, int* t2) {\n"
              "    while (t1 && t2 &&\n"
              "       *t1 == *t2) {\n"
              "        t1 = nullptr;\n"
              "        t2 = nullptr;\n"
              "    }\n"
              "    if (!t1 || !t2)\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int* i);\n"
              "void g(int* i) {\n"
              "    while(f(i) && *i == 0)\n"
              "        i++;\n"
              "    if (!i) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer56() {
        check("struct ListEntry {\n"
              "    struct ListEntry *next;\n"
              "};\n"
              "static void dostuff(ListEntry * listHead) {\n"
              "    ListEntry *prev = NULL;\n"
              "    for (ListEntry *cursor = listHead; cursor != NULL; prev = cursor, cursor = cursor->next) {}\n"
              "    if (prev) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer57() {
        check("void f() {\n"
              "    FILE* fptr = fopen(\"test\", \"r\");\n"
              "    if (fptr != nullptr) {\n"
              "        std::function<void()> fn([&] {\n"
              "            fclose(fptr);\n"
              "            fptr = NULL;\n"
              "        });\n"
              "        fgetc(fptr);\n"
              "        fn();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer58() {
        check("struct myStruct { char entry[0]; };\n"
              "void f() {\n"
              "    struct myStruct* sPtr = NULL;\n"
              "    int sz = (!*(&sPtr) || ((*(&sPtr))->entry[0] > 15)) ?\n"
              "        sizeof((*(&sPtr))->entry[0]) : 123456789;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer59() {
        check("struct Box {\n"
              "    struct Box* prev;\n"
              "    struct Box* next;\n"
              "};\n"
              "void foo(Box** pfreeboxes) {\n"
              "    Box *b = *pfreeboxes;\n"
              "    *pfreeboxes = b->next;\n"
              "    if( *pfreeboxes )\n"
              "        (*pfreeboxes)->prev = nullptr;\n"
              "    b->next = nullptr;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer60() {
        check("void f(){\n"
              "    char uuid[128];\n"
              "    char *s1;\n"
              "    memset(uuid, 0, sizeof(uuid));\n"
              "    s1 = strchr(uuid, '=');\n"
              "    s1 = s1 ? s1 + 1 : &uuid[5];\n"
              "    if (!strcmp(\"00000000000000000000000000000000\", s1) )\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer61() {
        check("struct a {\n"
              "  int *e;\n"
              "};\n"
              "struct f {\n"
              "  a *g() const;\n"
              "};\n"
              "void h() {\n"
              "  for (f b;;) {\n"
              "    a *c = b.g();\n"
              "    int *d = c->e;\n"
              "    if (d)\n"
              "      ;\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "  A* g() const;\n"
              "  A* h() const;\n"
              "};\n"
              "void f(A* a) {\n"
              "  if (!a->h())\n"
              "    return;\n"
              "  const A *b = a;\n"
              "  while (b && !b->h())\n"
              "      b = b->g();\n"
              "  if (!b || b == b->g()->h())\n"
              "      return;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer62() {
        check("struct A {\n"
              "  bool f()() const;\n"
              "};\n"
              "void a(A *x) {\n"
              "  std::string b = x && x->f() ? \"\" : \"\";\n"
              "  if (x) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "  bool f()() const;\n"
              "};\n"
              "void a(A *x) {\n"
              "  std::string b = (!x || x->f()) ? \"\" : \"\";\n"
              "  if (x) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "  A * aa;\n"
              "};\n"
              "void b(A*);\n"
              "void a(A *x) {\n"
              "  b(x ? x->aa : nullptr);\n"
              "  if (!x) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer63() {
        check("struct A {\n"
              "    A* a() const;\n"
              "    A* b() const;\n"
              "};\n"
              "A* f(A*);\n"
              "void g(const A* x) {\n"
              "    A *d = x->a();\n"
              "    d = f(d->b()) ? d->a() : nullptr;\n"
              "    if (d && f(d->b())) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer64() {
        check("struct A {\n"
              "  A* f() const;\n"
              "  int g() const;\n"
              "};\n"
              "bool a;\n"
              "bool b(A* c) {\n"
              "    if (c->g() == 0)\n"
              "      ;\n"
              "    A *aq = c;\n"
              "    if (c->g() == 0)\n"
              "      c = c->f();\n"
              "    if (c)\n"
              "      for (A *d = c; d != aq; d = d->f()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "  A* g() const;\n"
              "  A* h() const;\n"
              "};\n"
              "bool i(A*);\n"
              "void f(A* x) {\n"
              "  if (i(x->g())) {\n"
              "    A *y = x->g();\n"
              "    x = x->g()->h();\n"
              "    if (x && x->g()) {\n"
              "        y = x->g()->h();\n"
              "    }\n"
              "    if (!y) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer65() {
        check("struct A {\n"
              "    double get();\n"
              "};\n"
              "double x;\n"
              "double run(A** begin, A** end) {\n"
              "    A* a = nullptr;\n"
              "    while (begin != end) {\n"
              "        a = *begin;\n"
              "        x = a->get();\n"
              "        ++begin;\n"
              "    }\n"
              "    x = 0;\n"
              "    if (a)\n"
              "        return a->get();\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer66() {
        check("int f() {\n"
              "    int ret = 0;\n"
              "    int *v = nullptr;\n"
              "    if (!MyAlloc(&v)) {\n"
              "        ret = -1;\n"
              "        goto done;\n"
              "    }\n"
              "    DoSomething(*v);\n"
              "done:\n"
              "    if (v)\n"
              "      MyFree(&v);\n"
              "    return ret;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer67() {
        check("int result;\n"
              "\n"
              "int test_b(void) {\n"
              "    char **string = NULL;\n"
              "\n"
              "    /* The bug disappears if \"result =\" is omitted. */\n"
              "    result = some_other_call(&string);\n"
              "    if (string && string[0])\n"
              "        return 0;\n"
              "    return -1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int result;\n"
              "\n"
              "int test_b(void) {\n"
              "    char **string = NULL;\n"
              "\n"
              "    some_other_call(&string);\n"
              "    if (string && string[0])\n"
              "        return 0;\n"
              "    return -1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer68() {
        check("struct A {\n"
              "    A* b;\n"
              "};\n"
              "void f(A* c) {\n"
              "    c = c->b;\n"
              "    if (c->b) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    A* b;\n"
              "};\n"
              "void f(A* c) {\n"
              "    A* d = c->b;\n"
              "    A *e = c;\n"
              "    while (nullptr != (e = e->b)) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer69() {
        check("void f(const Scope *scope) {\n"
              "    if (scope->definedType) {}\n"
              "    while (scope) {\n"
              "        scope = scope->nestedIn;\n"
              "        enumerator = scope->findEnumerator();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:5:22]: (warning) Either the condition 'scope' is redundant or there is possible null pointer dereference: scope. [nullPointerRedundantCheck]\n",
            errout_str());

        check("void f(const Scope *scope) {\n"
              "    if (scope->definedType) {}\n"
              "    while (scope && scope->nestedIn) {\n"
              "        if (scope->type == Scope::eFunction && scope->functionOf)\n"
              "            scope = scope->functionOf;\n"
              "        else\n"
              "            scope = scope->nestedIn;\n"
              "        enumerator = scope->findEnumerator();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:8:22]: (warning) Either the condition 'scope' is redundant or there is possible null pointer dereference: scope. [nullPointerRedundantCheck]\n",
            errout_str());

        check("struct a {\n"
              "  a *b() const;\n"
              "  void c();\n"
              "};\n"
              "void d() {\n"
              "  for (a *e;;) {\n"
              "    e->b()->c();\n"
              "    while (e)\n"
              "      e = e->b();\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer70() {
        check("struct Token {\n"
              "    const Token* nextArgument() const;\n"
              "    const Token* next() const;\n"
              "    int varId() const;\n"
              "};\n"
              "int f(const Token *first, const Token* second) {\n"
              "    first = first->nextArgument();\n"
              "    if (first)\n"
              "        first = first->next();\n"
              "    if (second->next()->varId() == 0) {\n"
              "        second = second->nextArgument();\n"
              "        if (!first || !second)\n"
              "            return 0;\n"
              "    } else if (!first) {\n"
              "        return 0;\n"
              "    }\n"
              "    return first->varId();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct Token {\n"
              "    const Token* nextArgument() const;\n"
              "    const Token* next() const;\n"
              "    int varId() const;\n"
              "    void str() const;"
              "};\n"
              "void f(const Token *first) {\n"
              "    first = first->nextArgument();\n"
              "    if (first)\n"
              "        first = first->next();\n"
              "    first->str();\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:8] -> [test.cpp:10]: (warning) Either the condition 'first' is redundant or there is possible null pointer dereference: first.\n",
            "",
            errout_str());
    }

    void nullpointer71() {
        check("void f() {\n"
              "  Device* dev = Get();\n"
              "  SetCount(dev == nullptr ? 0 : dev->size());\n"
              "  if (dev)\n"
              "    DoSomething(dev);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  Device* dev = Get();\n"
              "  SetCount(dev != nullptr ? dev->size() : 0);\n"
              "  if (dev)\n"
              "    DoSomething(dev);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer72() { // #10215
        check("int test() {\n"
              "  int* p0 = nullptr, *p1 = nullptr;\n"
              "  getFoo(p0);\n"
              "  getBar(p1);\n"
              "  if (!(p0 != nullptr && p1 != nullptr))\n"
              "    return {};\n"
              "  return *p0 + *p1;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("int test2() {\n"
              "  int* p0 = nullptr;\n"
              "  if (!(getBaz(p0) && p0 != nullptr))\n"
              "    return 0;\n"
              "  return *p0;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("int test3() {\n"
              "  Obj* PObj = nullptr;\n"
              "  if (!(GetObj(PObj) && PObj != nullptr))\n"
              "    return 1;\n"
              "  if (!PObj->foo())\n"
              "    test();\n"
              "  PObj->bar();\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer73() {
        check("void f(bool flag2, int* ptr) {\n"
              "    bool flag1 = true;\n"
              "    if (flag2) {\n"
              "        if (ptr != nullptr)\n"
              "            (*ptr)++;\n"
              "        else\n"
              "            flag1 = false;\n"
              "    }\n"
              "    if (flag1 && flag2)\n"
              "        (*ptr)++;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool flag2, int* ptr) {\n"
              "    bool flag1 = true;\n"
              "    if (flag2) {\n"
              "        if (ptr != nullptr)\n"
              "            (*ptr)++;\n"
              "        else\n"
              "            flag1 = false;\n"
              "    }\n"
              "    if (!flag1 && flag2)\n"
              "        (*ptr)++;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:17] -> [test.cpp:10:11]: (warning) Either the condition 'ptr!=nullptr' is redundant or there is possible null pointer dereference: ptr. [nullPointerRedundantCheck]\n", errout_str());
    }

    void nullpointer74() {
        check("struct d {\n"
              "  d* e();\n"
              "};\n"
              "void g(d* f) {\n"
              "  do {\n"
              "    f = f->e();\n"
              "    if (f) {}\n"
              "  } while (0);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct d {\n"
              "  d* e();\n"
              "};\n"
              "void g(d* f, int i) {\n"
              "  do {\n"
              "    i--;\n"
              "    f = f->e();\n"
              "    if (f) {}\n"
              "  } while (i > 0);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:8:9] -> [test.cpp:7:9]: (warning) Either the condition 'f' is redundant or there is possible null pointer dereference: f. [nullPointerRedundantCheck]\n",
            errout_str());

        check("struct d {\n"
              "  d* e();\n"
              "};\n"
              "void g(d* f, int i) {\n"
              "  do {\n"
              "    i--;\n"
              "    f = f->e();\n"
              "    if (f) {}\n"
              "  } while (f && i > 0);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer75() {
        check("struct a {\n"
              "  a *b() const;\n"
              "  void c();\n"
              "  int d() const;\n"
              "};\n"
              "void e(a *x) {\n"
              "  while (x->b()->d() == 0)\n"
              "    x->c();\n"
              "  x->c();\n"
              "  if (x->b()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer76()
    {
        check("int* foo(int y) {\n"
              "    std::unique_ptr<int> x = std::make_unique<int>(0);\n"
              "    if( y == 0 )\n"
              "        return x.release();\n"
              "    (*x) ++;\n"
              "    return x.release();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer77()
    {
        check("bool h(int*);\n"
              "void f(int* i) {\n"
              "    int* i = nullptr;\n"
              "    if (h(i) && *i == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool h(int*);\n"
              "void f(int* i) {\n"
              "    int* i = nullptr;\n"
              "    if (h(i))\n"
              "        if (*i == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool h(int*);\n"
              "void f(int* x) {\n"
              "    int* i = x;\n"
              "    if (h(i))\n"
              "        i = nullptr;\n"
              "    if (h(i) && *i == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i) {\n" // #13797
              "    int* p = nullptr;\n"
              "    if (!i) {\n"
              "        *p = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());
    }

    void nullpointer78() // #7802
    {
        check("void f()\n"
              "{\n"
              "    int **pp;\n"
              "    int *p = 0;\n"
              "    pp = &p;\n"
              "    **pp = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:6]: (error) Null pointer dereference: *pp [nullPointer]\n", errout_str());
    }

    void nullpointer79() // #10400
    {
        check("void resize(size_t nF, size_t nT) {\n"
              "    double* pValues = nullptr;\n"
              "    if (nF > 0 && nT > 0)\n"
              "        pValues = new double[nF * nT];\n"
              "    for (size_t cc = 0; cc < nF * nT; ++cc)\n"
              "        pValues[cc] = 42;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer80() // #10410
    {
        check("int f(int* a, int* b) {\n"
              "    if( a || b ) {\n"
              "        int n = a ? *a : *b;\n"
              "        if( b )\n"
              "            n++;\n"
              "        return n;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer81() // #8724
    {
        check("void f(A **list) {\n"
              "  A *tmp_List = NULL;\n"
              "  *list = NULL;\n"
              "  while (1) {\n"
              "    if (*list == NULL) {\n"
              "      tmp_List = malloc (sizeof (ArchiveList_struct));\n"
              "      *list = tmp_List;\n"
              "    } else {\n"
              "      tmp_List->next = malloc (sizeof (ArchiveList_struct));\n"
              "    }\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer82() // #10331
    {
        check("bool g();\n"
              "int* h();\n"
              "void f(int* ptr) {\n"
              "    if (!ptr) {\n"
              "        if (g())\n"
              "            goto done;\n"
              "        ptr = h();\n"
              "        if (!ptr)\n"
              "            return;\n"
              "    }\n"
              "    if (*ptr == 1)\n"
              "        return;\n"
              "\n"
              "done:\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer83() // #9870
    {
        check("int* qux();\n"
              "int* f7c2(int *x) {\n"
              "  int* p = 0;\n"
              "  if (nullptr == x)\n"
              "    p = qux();\n"
              "  if (nullptr == x)\n"
              "    return x;\n"
              "  *p = 1;\n"
              "  return x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:4]: (warning) Possible null pointer dereference: p [nullPointer]\n", errout_str());
    }

    void nullpointer84() // #9873
    {
        check("void f(std::unique_ptr<A> P) {\n"
              "  A *RP = P.get();\n"
              "  if (!RP) {\n"
              "    P->foo();\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:7] -> [test.cpp:4:5]: (warning) Either the condition '!RP' is redundant or there is possible null pointer dereference: P. [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer85() // #10210
    {
        check("struct MyStruct {\n"
              "  int GetId() const {\n"
              "      int id = 0;\n"
              "      int page = m_notebook->GetSelection();\n"
              "      if (m_notebook && (m_notebook->GetPageCount() > 0))\n"
              "        id = page;\n"
              "      return id;\n"
              "  }\n"
              "  wxNoteBook *m_notebook = nullptr;\n"
              "};\n"
              "int f() {\n"
              "  const MyStruct &s = Get();\n"
              "  return s.GetId();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:5:11] -> [test.cpp:4:18]: (warning) Either the condition 'm_notebook' is redundant or there is possible null pointer dereference: m_notebook. [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer86()
    {
        check("struct A {\n"
              "    A* a() const;\n"
              "    int b() const;\n"
              "};\n"
              "A* f(A* t) {\n"
              "    if (t->b() == 0) {\n"
              "        return t;\n"
              "    }\n"
              "    return t->a();\n"
              "}\n"
              "void g(A* t) {\n"
              "    t = f(t->a());\n"
              "    if (!t->a()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer87() // #9291
    {
        check("int f(bool b, int* x) {\n"
              "    if (b && x == nullptr)\n"
              "        return 0;\n"
              "    else if (!b && x == nullptr)\n"
              "        return 1;\n"
              "    else if (!b && x != nullptr)\n"
              "        return *x;\n"
              "    else\n"
              "        return *x + 1;\n"
              "}\n");
        TODO_ASSERT_EQUALS("", "[test.cpp:6:22] -> [test.cpp:9:17]: (warning) Either the condition 'x!=nullptr' is redundant or there is possible null pointer dereference: x. [nullPointerRedundantCheck]\n", errout_str());

        check("void f(int n, int* p) {\n"
              "    int* r = nullptr;\n"
              "    if (n < 0)\n"
              "        return;\n"
              "    if (n == 0)\n"
              "        r = p;\n"
              "    else if (n > 0)\n"
              "        r = p + 1;\n"
              "    *r;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer88() // #9949
    {
        check("struct S { char **ppc; };\n"
              "int alloc(struct S* s) {\n"
              "    char** ppc = malloc(4096);\n"
              "    if (ppc != NULL) {\n"
              "        s->ppc = ppc;\n"
              "        return 1;\n"
              "    }\n"
              "    return 0;\n"
              "}\n"
              "void f() {\n"
              "    struct S* s = malloc(sizeof(struct S));\n"
              "    if (!s) return;\n"
              "    s->ppc = NULL;\n"
              "    if (alloc(s))\n"
              "        s->ppc[0] = \"\";\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer89() // #10640
    {
        check("typedef struct {\n"
              "    int x;\n"
              "} foo_t;\n"
              "typedef struct {\n"
              "    foo_t *y;\n"
              "} bar_t;\n"
              "void f(bar_t *ptr) {\n"
              "    if(ptr->y->x)\n"
              "        if(ptr->y != nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:19] -> [test.cpp:8:11]: (warning) Either the condition 'ptr->y!=nullptr' is redundant or there is possible null pointer dereference: ptr->y. [nullPointerRedundantCheck]\n",
            errout_str());

        check("bool argsMatch(const Token *first, const Token *second) {\n" // #6145
              "    if (first->str() == \")\")\n"
              "        return true;\n"
              "    else if (first->next()->str() == \"=\")\n"
              "        first = first->nextArgument();\n"
              "    else if (second->next()->str() == \"=\") {\n"
              "        second = second->nextArgument();\n"
              "        if (second)\n"
              "            second = second->tokAt(-2);\n"
              "        if (!first || !second) {\n"
              "            return !first && !second;\n"
              "        }\n"
              "    }\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:10:13] -> [test.cpp:2:9]: (warning) Either the condition '!first' is redundant or there is possible null pointer dereference: first. [nullPointerRedundantCheck]\n"
            "[test.cpp:10:13] -> [test.cpp:4:14]: (warning) Either the condition '!first' is redundant or there is possible null pointer dereference: first. [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer90() // #6098
    {
        check("std::string definitionToName(Definition *ctx)\n"
              "{\n"
              "  if (ctx->definitionType()==Definition::TypeMember)\n"                           // possible null pointer dereference
              "  {\n"
              "     return \"y\";\n"
              "  }\n"
              "  else if (ctx)\n"                           // ctx is checked against null
              "  {\n"
              "    if(ctx->definitionType()!=Definition::TypeMember)\n"
              "    {\n"
              "       return \"x\";\n"
              "    }\n"
              "  }\n"
              "  return \"unknown\";\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:7:12] -> [test.cpp:3:7]: (warning) Either the condition 'ctx' is redundant or there is possible null pointer dereference: ctx. [nullPointerRedundantCheck]\n",
            errout_str());
    }

    void nullpointer91() // #10678
    {
        check("void f(const char* PBeg, const char* PEnd) {\n"
              "  while (PEnd != nullptr) {\n"
              "    const int N = h(PEnd);\n"
              "    PEnd = g();\n"
              "    const int Length = PEnd == nullptr ? 0 : PEnd - PBeg;\n"
              "  };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer92()
    {
        check("bool g(bool);\n"
              "int f(int* i) {\n"
              "    if (!g(!!i)) return 0;\n"
              "    return *i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool g(bool);\n"
              "int f(int* i) {\n"
              "    if (!g(!i)) return 0;\n"
              "    return *i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer93() // #3929
    {
        check("int* GetThing( ) { return 0; }\n"
              "int main() {\n"
              "        int* myNull = GetThing();\n"
              "        *myNull=42;\n"
              "        return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Null pointer dereference: myNull [nullPointer]\n", errout_str());

        check("struct foo {\n"
              "    int* GetThing(void) { return 0; }\n"
              "};\n"
              "int main(void) {\n"
              "        foo myFoo;\n"
              "        int* myNull = myFoo.GetThing();\n"
              "        *myNull=42;\n"
              "        return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:10]: (error) Null pointer dereference: myNull [nullPointer]\n", errout_str());

        check("struct T { bool g() const; };\n"
              "void f(T* p) {\n"
              "    if (!p)\n"
              "        return;\n"
              "    while (p->g())\n"
              "        p = nullptr;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:12]: (warning) Possible null pointer dereference: p [nullPointer]\n", errout_str());
    }

    void nullpointer94() // #11040
    {
        check("struct entry { struct entry* next; size_t len; };\n"
              "void f(struct entry **kep, size_t slen) {\n"
              "    while (*kep)\n"
              "        kep = &(*kep)->next;\n"
              "    *kep = (struct entry*)malloc(sizeof(**kep));\n"
              "    (*kep)->next = 0;\n"
              "    (*kep)->len = slen;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:6]: (warning) If memory allocation fails, then there is a possible null pointer dereference: *kep [nullPointerOutOfMemory]\n", errout_str());
    }

    void nullpointer95() // #11142
    {
        check("void f(std::vector<int*>& v) {\n"
              "    for (auto& p : v)\n"
              "        if (*p < 2)\n"
              "            p = nullptr;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer96()
    {
        check("struct S {\n"
              "  int x;\n"
              "};\n"
              "S *create_s();\n"
              "void test() {\n"
              "  S *s = create_s();\n"
              "  for (int i = 0; i < s->x; i++) {\n"
              "    if (s->x == 17) {\n"
              "      s = nullptr;\n"
              "      break;\n"
              "    }\n"
              "  }\n"
              "  if (s) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer97() // #11229
    {
        check("struct B { virtual int f() = 0; };\n"
              "struct D : public B { int f() override; };\n"
              "int g(B* p) {\n"
              "    if (p) {\n"
              "        auto d = dynamic_cast<D*>(p);\n"
              "        return d ? d->f() : 0;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer98() // #11458
    {
        check("struct S { double* d() const; };\n"
              "struct T {\n"
              "    virtual void g(double* b, double* d) const = 0;\n"
              "    void g(S* b) const { g(b->d(), nullptr); }\n"
              "    void g(S* b, S* d) const { g(b->d(), d->d()); }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer99() // #10602
    {
        check("class A\n"
              "{\n"
              "    int *foo(const bool b)\n"
              "    {\n"
              "        if(b)\n"
              "            return nullptr;\n"
              "        else\n"
              "            return new int [10];\n"
              "    }\n"
              "public:\n"
              "    void bar(void)\n"
              "    {\n"
              "        int * buf = foo(true);\n"
              "        buf[2] = 0;" // <<
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:14:9]: (error) Null pointer dereference: buf [nullPointer]\n", errout_str());
    }

    void nullpointer100() // #11636
    {
        check("const char* type_of(double) { return \"unknown\"; }\n"
              "void f() {\n"
              "    double tmp = 0.0;\n"
              "    const char* t = type_of(tmp);\n"
              "    std::cout << t;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer101() // #11382
    {
        check("struct Base { virtual ~Base(); };\n"
              "struct Derived : Base {};\n"
              "bool is_valid(const Derived&);\n"
              "void f(const Base* base) {\n"
              "    const Derived* derived = dynamic_cast<const Derived*>(base);\n"
              "    if (derived && !is_valid(*derived) || base == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer102()
    {
        check("struct S { std::string str; };\n" // #11534
              "struct T { S s; };\n"
              "struct U { T t[1]; };\n"
              "void f(const T& t, const U& u, std::string& str) {\n"
              "    if (str.empty())\n"
              "        str = t.s.str;\n"
              "    else\n"
              "        str = u.t[0].s.str;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer103()
    {
        check("struct S {\n" // #10572
              "    int f();\n"
              "    int* m_P{};\n"
              "};\n"
              "int S::f() {\n"
              "    if (!m_P) {\n"
              "        try {\n"
              "            m_P = new int(1);\n"
              "        }\n"
              "        catch (...) {\n"
              "            return 0;\n"
              "        }\n"
              "    }\n"
              "    return *m_P;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int* p, const int* q) {\n" // #11873
              "    if (*q == -1)\n"
              "        *p = 0;\n"
              "}\n"
              "void g() {\n"
              "    int x = -2;\n"
              "    f(nullptr, &x);\n"
              "}\n");
        TODO_ASSERT_EQUALS("", "[test.cpp:3:10]: (warning) Possible null pointer dereference: p [nullPointer]\n", errout_str());
    }

    void nullpointer104() // #13881
    {
        check("using std::max;\n"
              "void f(int i) {\n"
              "    const size_t maxlen = i == 1 ? 8 : (std::numeric_limits<std::size_t>::max());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer105() // #13861
    {
        check("struct AB { int a; int b; };\n"
              "namespace ns { typedef AB S[10]; }\n"
              "void foo(void) {\n"
              "    ns::S x = {0};\n"
              "    x[1].a = 2;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer_addressOf() { // address of
        check("void f() {\n"
              "  struct X *x = 0;\n"
              "  if (addr == &x->y) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  struct X *x = 0;\n"
              "  if (addr == &x->y.z[0]) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        checkP("typedef int Count;\n" // #10018
               "#define offsetof(TYPE, MEMBER) ((Count) & ((TYPE*)0)->MEMBER)\n"
               "struct S {\n"
               "    int a[20];\n"
               "};\n"
               "int g(int i) {\n"
               "    return offsetof(S, a[i]);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointerSwitch() { // #2626
        // extracttests.start: char *do_something();
        check("char *f(int x) {\n"
              "    char *p = do_something();\n"
              "    switch (x) {\n"
              "      case 1:\n"
              "        p = 0;\n"
              "      case 2:\n"
              "        *p = 0;\n"
              "        break;\n"
              "    }\n"
              "    return p;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:7:10]: (warning) Possible null pointer dereference: p [nullPointer]\n", errout_str());
    }

    void nullpointer_cast() {
        check("char *nasm_skip_spaces(const char *p) {\n" // #4692
              "    if (p)\n"
              "        while (*p && nasm_isspace(*p))\n"
              "            p++;\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char* origin) {\n" // #11449
              "    char* cp = (strchr)(origin, '\\0');\n"
              "    if (cp[-1] != '/')\n"
              "        *cp++ = '/';\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer_castToVoid() {  // #3771
        check("void f () {\n"
              "    int *buf; buf = NULL;\n"
              "    buf;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer_subfunction() {
        check("int f(int* x, int* y) {\n"
              "    if (!x)\n"
              "        return;\n"
              "    return *x + *y;\n"
              "}\n"
              "void g() {\n"
              "    f(nullptr, nullptr);\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    // Check if pointer is null and the dereference it
    void pointerCheckAndDeRef() {
        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "    }\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:4:6]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(char *p) {\n"
              "    if (p && *p == 0) {\n"
              "    }\n"
              "    printf(\"%c\", *p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:4:19]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(char *p) {\n"
              "    if (p && *p == 0) {\n"
              "    } else { *p = 0; }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:15]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(char *p) {\n"
              "    if (p) {\n"
              "    }\n"
              "    strcpy(p, \"abc\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:4:12]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(char *p) {\n"
              "    if (p) {\n"
              "    }\n"
              "    bar();\n"
              "    strcpy(p, \"abc\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:5:12]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("void foo(abc *p) {\n"
              "    if (!p) {\n"
              "    }\n"
              "    else { if (!p->x) {\n"
              "    } }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        {
            static const char code[] =
                "void foo(char *p) {\n"
                "    if (!p) {\n"
                "        abort();\n"
                "    }\n"
                "    *p = 0;\n"
                "}";
            check(code);
            ASSERT_EQUALS("", errout_str());

            check(code, dinit(CheckOptions, $.inconclusive = true));
            ASSERT_EQUALS("", errout_str());
        }

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "        (*bail)();\n"
              "    }\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "        throw x;\n"
              "    }\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "        ab.abort();\n"
              "    }\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "        switch (x) { }\n"
              "    }\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo(char *p) {\n"
              "    if (!p) {\n"
              "    }\n"
              "    return *x;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("int foo(int *p) {\n"
              "    if (!p) {\n"
              "        x = *p;\n"
              "        return 5+*p;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:14]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n"
                      "[test.cpp:2:9] -> [test.cpp:4:19]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        // operator!
        check("void f() {\n"
              "    A a;\n"
              "    if (!a) {\n"
              "        a.x();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // This is why this check can't be used on the simplified token list
        check("void f(Foo *foo) {\n"
              "    if (!dynamic_cast<bar *>(foo)) {\n"
              "        *foo = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket: #2300 - calling unknown function that may initialize the pointer
        check("Fred *fred;\n"
              "void a() {\n"
              "    if (!fred) {\n"
              "        initfred();\n"
              "        fred->x = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #1219
        check("void foo(char *p) {\n"
              "    if (p) {\n"
              "        return;\n"
              "    }\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:5:6]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        // #2467 - unknown macro may terminate the application
        check("void f(Fred *fred) {\n"
              "    if (fred == NULL) {\n"
              "        MACRO;\n"
              "    }\n"
              "    fred->a();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #2493 - switch
        check("void f(Fred *fred) {\n"
              "    if (fred == NULL) {\n"
              "        x = 0;\n"
              "    }\n"
              "    switch (x) {\n"
              "        case 1:\n"
              "            fred->a();\n"
              "            break;\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #4118 - second if
        check("void f(char *p) {\n"
              "    int x = 1;\n"
              "    if (!p) x = 0;\n"
              "    if (x) *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #2674 - different functions
        check("class Fred {\n"
              "public:\n"
              "    Wilma *wilma;\n"
              "    void a();\n"
              "    void b();\n"
              "};\n"
              "\n"
              "void Fred::a() {\n"
              "    if ( wilma ) { }\n"
              "}\n"
              "\n"
              "void Fred::b() {\n"
              "    wilma->Reload();\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void test(int *i) {\n"
              "  if(i == NULL) { }\n"
              "  else {\n"
              "    int b = *i;\n"
              "  }\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #2696 - false positives nr 1
        check("void f()\n"
              "{\n"
              "   struct foo *pFoo = NULL;\n"
              "   size_t len;\n"
              "\n"
              "   len = sizeof(*pFoo) - sizeof(pFoo->data);\n"
              "\n"
              "   if (pFoo)\n"
              "      bar();\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #2696 - false positives nr 2
        check("void f()\n"
              "{\n"
              "   struct foo *pFoo = NULL;\n"
              "   size_t len;\n"
              "\n"
              "   while (pFoo)\n"
              "      pFoo = pFoo->next;\n"
              "\n"
              "   len = sizeof(pFoo->data);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #2696 - false positives nr 3
        check("void f()\n"
              "{\n"
              "   struct foo *pFoo = NULL;\n"
              "   size_t len;\n"
              "\n"
              "   while (pFoo)\n"
              "      pFoo = pFoo->next;\n"
              "\n"
              "   len = decltype(*pFoo);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("int foo(struct Fred *fred) {\n"
              "    if (fred) { }\n"
              "    return fred->a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:12]: (warning) Either the condition 'fred' is redundant or there is possible null pointer dereference: fred. [nullPointerRedundantCheck]\n", errout_str());

        // #2789 - assign and check pointer
        check("void f() {\n"
              "    char *p; p = x();\n"
              "    if (!p) { }\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:4:6]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        // check, assign and use
        check("void f() {\n"
              "    char *p;\n"
              "    if (p == 0 && (p = malloc(10)) != 0) {\n"
              "        *p = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // check, assign and use
        check("void f() {\n"
              "    char *p;\n"
              "    if (p == 0 && (p = malloc(10)) != a && (*p = a)) {\n"
              "        *p = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // check, and use
        check("void f() {\n"
              "    char *p;\n"
              "    if (p == 0 && (*p = 0)) {\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:3:21]: (warning) Either the condition 'p==0' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        // check, and use
        check("void f() {\n"
              "    struct foo *p;\n"
              "    if (p == 0 && p->x == 10) {\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:3:19]: (warning) Either the condition 'p==0' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        // check, and use
        check("void f() {\n"
              "    struct foo *p;\n"
              "    if (p == 0 || p->x == 10) {\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // check, and use
        check("void f() {\n"
              "    char *p; p = malloc(10);\n"
              "    if (p == NULL && (*p = a)) {\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:3:24]: (warning) Either the condition 'p==NULL' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        // check, and use
        check("void f(struct X *p, int x) {\n"
              "    if (!p && x==1 || p && p->x==0) {\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        {
            const char code[] = "void f(Fred *fred) {\n"
                                "    if (fred == NULL) { }\n"
                                "    fred->x();\n"
                                "}";

            check(code);     // inconclusive
            ASSERT_EQUALS("[test.cpp:2:14] -> [test.cpp:3:5]: (warning) Either the condition 'fred==NULL' is redundant or there is possible null pointer dereference: fred. [nullPointerRedundantCheck]\n", errout_str());
        }

        check("void f(char *s) {\n"   // #3358
              "    if (s==0);\n"
              "    strcpy(a, s?b:c);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // sizeof
        check("void f(struct fred_t *fred) {\n"
              "    if (!fred)\n"
              "        int sz = sizeof(fred->x);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // check in macro
        check("void f(int *x) {\n"
              "    $if (!x) {}\n"
              "    *x = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // return ?:
        check("int f(ABC *p) {\n" // FP : return ?:
              "    if (!p) {}\n"
              "    return p ? p->x : 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("int f(ABC *p) {\n" // no fn
              "    if (!p) {}\n"
              "    return q ? p->x : 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:16]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("int f(ABC *p) {\n" // FP : return &&
              "    if (!p) {}\n"
              "    return p && p->x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x, int *p) {\n"
              "    if (x || !p) {}\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14] -> [test.cpp:3:6]: (warning) Either the condition '!p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        // sizeof
        check("void f() {\n"
              "  int *pointer = NULL;\n"
              "  pointer = func(sizeof pointer[0]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n" // #14164
              "    T* next;\n"
              "    char op;\n"
              "};\n"
              "void h(int, char);\n"
              "void g(const T* tok, bool b) {\n"
              "    if (tok->op == '<') {\n"
              "        while ((tok = tok->next) && tok->op != '>') {}\n"
              "    }\n"
              "    h(b ? 1 : 0, tok->op);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:21] -> [test.cpp:10:18]: (warning) Either the condition 'tok=tok->next' is redundant or there is possible null pointer dereference: tok. [nullPointerRedundantCheck]\n",
                      errout_str());
    }

    // Test CheckNullPointer::nullConstantDereference
    void nullConstantDereference() {
        check("int f() {\n"
              "    int* p = 0;\n"
              "    return p[4];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("void f() {\n"
              "    typeof(*NULL) y;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("int * f() {\n"
              "    return NULL;\n"
              "}\n"
              "int main() {\n"
              "  return *f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Null pointer dereference: f() [nullPointer]\n", errout_str());
    }

    void gcc_statement_expression() {
        // Ticket #2621
        check("void f(struct ABC *abc) {\n"
              "    ({ if (abc) dbg(); })\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void snprintf_with_zero_size() {
        // Ticket #2840
        check("void f() {\n"
              "    int bytes = snprintf(0, 0, \"%u\", 1);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void snprintf_with_non_zero_size() {
        // Ticket #2840
        check("void f() {\n"
              "    int bytes = snprintf(0, 10, \"%u\", 1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:26]: (error) Null pointer dereference [nullPointer]\n", errout_str());
    }

    void printf_with_invalid_va_argument() {
        check("void f() {\n"
              "    printf(\"%s\", 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (error) Null pointer dereference [nullPointer]\n", errout_str());

        check("void f(char* s) {\n"
              "    printf(\"%s\", s);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char* s = 0;\n"
              "    printf(\"%s\", s);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:18]: (error) Null pointer dereference: s [nullPointer]\n",
            errout_str());

        check("void f() {\n"
              "    char *s = 0;\n"
              "    printf(\"%s\", s == 0 ? a : s);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    printf(\"%u%s\", 0, 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:23]: (error) Null pointer dereference [nullPointer]\n", errout_str());

        check("void f(char* s) {\n"
              "    printf(\"%u%s\", 0, s);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char* s = 0;\n"
              "    printf(\"%u%s\", 123, s);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:25]: (error) Null pointer dereference: s [nullPointer]\n",
            errout_str());


        check("void f() {\n"
              "    printf(\"%%%s%%\", 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (error) Null pointer dereference [nullPointer]\n", errout_str());

        check("void f(char* s) {\n"
              "    printf(\"text: %s, %s\", s, 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:31]: (error) Null pointer dereference [nullPointer]\n", errout_str());


        check("void f() {\n"
              "    char* s = \"blabla\";\n"
              "    printf(\"%s\", s);\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        check("void f(char* s) {\n"
              "    printf(\"text: %m%s, %s\", s, 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:33]: (error) Null pointer dereference [nullPointer]\n", errout_str());

        check("void f(char* s) {\n"
              "    printf(\"text: %*s, %s\", s, 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:32]: (error) Null pointer dereference [nullPointer]\n", errout_str());

        // Ticket #3364
        check("void f() {\n"
              "    printf(\"%-*.*s\", s, 0);\n"
              "    sprintf(\"%*\", s);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void scanf_with_invalid_va_argument() {
        check("void f(char* s) {\n"
              "    sscanf(s, \"%s\", 0);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:21]: (error) Null pointer dereference [nullPointer]\n",
            errout_str());

        check("void f() {\n"
              "    scanf(\"%d\", 0);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:17]: (error) Null pointer dereference [nullPointer]\n",
            errout_str());

        check("void f(char* foo) {\n"
              "    char location[200];\n"
              "    int width, height;\n"
              "    sscanf(imgInfo, \"%s %d %d\", location, &width, &height);\n"
              "}");
        ASSERT_EQUALS("", errout_str()); // ticket #3207

        check("void f(char *dummy) {\n"
              "    int iVal;\n"
              "    sscanf(dummy, \"%d%c\", &iVal);\n"
              "}");
        ASSERT_EQUALS("", errout_str()); // ticket #3211

        check("void f(char *dummy) {\n"
              "    int* iVal = 0;\n"
              "    sscanf(dummy, \"%d\", iVal);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:25]: (error) Null pointer dereference: iVal [nullPointer]\n",
            errout_str());

        check("void f(char *dummy) {\n"
              "    int* iVal;\n"
              "    sscanf(dummy, \"%d\", foo(iVal));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char *dummy) {\n"
              "    int* iVal = 0;\n"
              "    sscanf(dummy, \"%d%d\", foo(iVal), iVal);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char* dummy) {\n"
              "    sscanf(dummy, \"%*d%u\", 0);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:28]: (error) Null pointer dereference [nullPointer]\n",
            errout_str());
    }

    void nullpointer_in_return() {
        // extracttests.start: int maybe(); int *g();
        check("int foo() {\n"
              "    int* iVal = 0;\n"
              "    if(maybe()) iVal = g();\n"
              "    return iVal[0];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (warning) Possible null pointer dereference: iVal [nullPointer]\n", errout_str());

        check("int foo(int* iVal) {\n"
              "    return iVal[0];\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer_in_typeid() {
        // Should throw std::bad_typeid
        check("struct PolymorphicA { virtual ~A() {} };\n"
              "bool foo() {\n"
              "     PolymorphicA* a = 0;\n"
              "     return typeid(*a) == typeid(*a);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct NonPolymorphicA { ~A() {} };\n"
              "bool foo() {\n"
              "     NonPolymorphicA* a = 0;\n"
              "     return typeid(*a) == typeid(*a);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool foo() {\n"
              "     char* c = 0;\n"
              "     return typeid(*c) == typeid(*c);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer_in_alignof() // #11401
    {
        check("size_t foo() {\n"
              "    char* c = 0;\n"
              "    return alignof(*c);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("size_t foo() {\n"
              "    return alignof(*0);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo(int *p) {\n"
              "    f(alignof(*p));\n"
              "    if (p) {}\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("size_t foo() {\n"
              "    char* c = 0;\n"
              "    return _Alignof(*c);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("size_t foo() {\n"
              "    return _alignof(*0);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("size_t foo() {\n"
              "    return __alignof(*0);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("size_t foo() {\n"
              "    return __alignof__(*0);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointer_in_for_loop() {
        // Ticket #3278
        check("void f(int* ptr, int cnt){\n"
              " if (!ptr)\n"
              "  cnt = 0;\n"
              " for (int i = 0; i < cnt; ++i)\n"
              "  *ptr++ = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #11635
        check("void f(char *cons, int rlen, int pos) {\n"
              "    int i;\n"
              "    char* cp1;\n"
              "    for (cp1 = &cons[pos], i = 1; i < rlen; cp1--)\n"
              "        if (*cp1 == '*')\n"
              "            continue;\n"
              "        else\n"
              "            i++;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const int* p) {\n" // #6710
              "    for (int i = *p; i < 5; ++i) {}\n"
              "    if (p) {}\n"
              "}\n"
              "struct S { int a; };\n"
              "void g(const S* s) {\n"
              "    for (int i = s->a; i < 5; ++i) {}\n"
              "    if (s) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:9] -> [test.cpp:2:19]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n"
                      "[test.cpp:8:9] -> [test.cpp:7:18]: (warning) Either the condition 's' is redundant or there is possible null pointer dereference: s. [nullPointerRedundantCheck]\n",
                      errout_str());
    }

    void nullpointerDeadCode() {
        // Ticket #11311
        check ("void f() {\n"
               "    if (0)\n"
               "        *(int *)0 = 1;\n"
               "    else\n"
               "         ;\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        check ("void f() {\n"
               "    if (0)\n"
               "        *(int *)0 = 1;\n"
               "    else {\n"
               "        if (0)\n"
               "            *(int *)0 = 2;\n"
               "        else\n"
               "            ;\n"
               "    }\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        check ("void f() {\n"
               "    while(0)\n"
               "        *(int*)0 = 1;\n"
               "    do {\n"
               "    } while(0);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        check ("int f() {\n"
               "    return 0 ? *(int*)0 = 1 : 1;\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        check ("int f() {\n"
               "    return 1 ? 1 : *(int*)0 = 1;\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        check ("struct S {\n" // #13220
               "    explicit S(int* p) : i(*p) {\n"
               "        if (p) {}\n"
               "    }\n"
               "    int i;\n"
               "};\n");
        ASSERT_EQUALS("[test.cpp:3:13] -> [test.cpp:2:29]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
                      errout_str());
    }

    void nullpointerDelete() {
        check("void f() {\n"
              "  K *k = getK();\n"
              "  if (k)\n"
              "     k->doStuff();\n"
              "  delete k;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  K *k = getK();\n"
              "  if (k)\n"
              "     k[0] = ptr;\n"
              "  delete [] k;\n"
              "  k = new K[10];\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointerSubFunction() {
        check("void g(int* x) { *x; }\n"
              "void f(int* x) {\n"
              "    if (x)\n"
              "        g(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n" // #14308
              "    bool b{};\n"
              "    T* next{};\n"
              "};\n"
              "bool g(const T*& r) {\n"
              "    const T* t = r;\n"
              "    r = t->next;\n"
              "    return t->b;\n"
              "}\n"
              "void f(const T* tok) {\n"
              "    if (g(tok)) {}\n"
              "    if (tok) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointerExit() {
        check("void f() {\n"
              "  K *k = getK();\n"
              "  if (!k)\n"
              "     exit(1);\n"
              "  k->f();\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void nullpointerStdString() {
        check("void f(std::string s1) {\n"
              "    void* p = 0;\n"
              "    s1 = 0;\n"
              "    s1 = '\\0';\n"
              "    std::string s2 = 0;\n"
              "    std::string s2 = '\\0';\n"
              "    std::string s3(0);\n"
              "    foo(std::string(0));\n"
              "    s1 = p;\n"
              "    std::string s4 = p;\n"
              "    std::string s5(p);\n"
              "    foo(std::string(p));\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:9:10]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:10:22]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:11:20]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:12:21]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:3:10]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:5:22]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:7:17]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:8:9]: (error) Null pointer dereference [nullPointer]\n"
                      , errout_str());

        check("void f(std::string s1) {\n"
              "    s1 = nullptr;\n"
              "    std::string s2 = nullptr;\n"
              "    std::string s3(nullptr);\n"
              "    foo(std::string(nullptr));\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:3:22]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:4:17]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:5:9]: (error) Null pointer dereference [nullPointer]\n"
                      , errout_str());

        check("void f(std::string s1) {\n"
              "    s1 = NULL;\n"
              "    std::string s2 = NULL;\n"
              "    std::string s3(NULL);\n"
              "    foo(std::string(NULL));\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:3:22]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:4:17]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:5:9]: (error) Null pointer dereference [nullPointer]\n"
                      , errout_str());

        check("void f(std::string s1, const std::string& s2, const std::string* s3) {\n"
              "    void* p = 0;\n"
              "    if (x) { return; }\n"
              "    foo(s1 == p);\n"
              "    foo(s2 == p);\n"
              "    foo(s3 == p);\n"
              "    foo(p == s1);\n"
              "    foo(p == s2);\n"
              "    foo(p == s3);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:15]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:5:15]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:7:9]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:8:9]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("void f(std::string s1, const std::string& s2, const std::string* s3) {\n"
              "    void* p = 0;\n"
              "    if (x) { return; }\n"
              "    foo(0 == s1.size());\n"
              "    foo(0 == s2.size());\n"
              "    foo(0 == s3->size());\n"
              "    foo(s1.size() == 0);\n"
              "    foo(s2.size() == 0);\n"
              "    foo(s3->size() == 0);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string s1, const std::string& s2) {\n"
              "    if (x) { return; }\n"
              "    foo(0 == s1[0]);\n"
              "    foo(0 == s2[0]);\n"
              "    foo(s1[0] == 0);\n"
              "    foo(s2[0] == 0);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string s1, const std::string& s2) {\n"
              "    if (x) { return; }\n"
              "    foo(s1 == '\\0');\n"
              "    foo(s2 == '\\0');\n"
              "    foo('\\0' == s1);\n"
              "    foo('\\0' == s2);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("class Bar {\n"
              "    std::string s;\n"
              "    Bar() : s(0) {}\n"
              "};\n"
              "class Foo {\n"
              "    std::string s;\n"
              "    Foo();\n"
              "};\n"
              "Foo::Foo() : s(0) {}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:9:14]: (error) Null pointer dereference [nullPointer]\n", errout_str());

        check("void f() {\n"
              "    std::string s = 0 == x ? \"a\" : \"b\";\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  const std::string s = g();\n"
              "  ASSERT_MESSAGE(\"Error on s\", 0 == s.compare(\"Some text\"));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int i, std::string s);\n"
              "void bar() {\n"
              "  foo(0, \"\");\n"
              "  foo(0, 0);\n"
              "  foo(var, 0);\n"
              "  foo(var, NULL);\n"
              "  foo(var, nullptr);\n"
              "  foo(0, var);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:5:12]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:6:12]: (error) Null pointer dereference [nullPointer]\n"
                      "[test.cpp:7:12]: (error) Null pointer dereference [nullPointer]\n", errout_str());

        check("std::string f() {\n" // #9827
              "  char* p = NULL;\n"
              "  int r = g(p);\n"
              "  if (!r)\n"
              "    return \"\";\n"
              "  std::string s(p);\n"
              "  return s;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:6:17]: (warning, inconclusive) Possible null pointer dereference: p [nullPointer]\n",
                      errout_str());

        check("void f() {\n" // #11078
              "    const char* p = nullptr;\n"
              "    std::string s1{ p };\n"
              "    std::string s2{ nullptr };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:21]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:4:17]: (error) Null pointer dereference [nullPointer]\n",
                      errout_str());

        check("const char* g(long) { return nullptr; }\n" // #11561
              "void f() { std::string s = g(0L); }\n");
        ASSERT_EQUALS("[test.cpp:2:29]: (error) Null pointer dereference: g(0L) [nullPointer]\n",
                      errout_str());

        check("const char* g() { return nullptr; }\n" // #14098
              "std::string f() {\n"
              "    std::string s{ g() };\n"
              "    return s + std::string(g());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:21]: (error) Null pointer dereference: g() [nullPointer]\n"
                      "[test.cpp:4:29]: (error) Null pointer dereference: g() [nullPointer]\n",
                      errout_str());
    }

    void nullpointerStdStream() {
        check("void f(std::ifstream& is) {\n"
              "    char* p = 0;\n"
              "    is >> p;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Possible null pointer dereference: p\n", "", errout_str());

        check("void f(const std::ostringstream& oss, char* q) {\n"
              "    char const* p = 0;\n" // Simplification makes detection of bug difficult
              "    oss << p;\n"
              "    oss << foo << p;\n"
              "    if(q == 0)\n"
              "        oss << foo << q;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:4:19]: (error) Null pointer dereference: p [nullPointer]\n"
                      "[test.cpp:5:10] -> [test.cpp:6:23]: (warning) Either the condition 'q==0' is redundant or there is possible null pointer dereference: q. [nullPointerRedundantCheck]\n", errout_str());

        check("void f(const char* p) {\n"
              "    if(p == 0) {\n"
              "        std::cout << p;\n"
              "        std::cerr << p;\n"
              "        std::cin >> p;\n"
              "        std::cout << abc << p;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:2:10] -> [test.cpp:3:22]: (warning) Either the condition 'p==0' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n"
                           "[test.cpp:2:10] -> [test.cpp:4:22]: (warning) Either the condition 'p==0' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n"
                           "[test.cpp:2:10] -> [test.cpp:5]: (warning) Either the condition 'p==0' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n"
                           "[test.cpp:2:10] -> [test.cpp:6]: (warning) Either the condition 'p==0' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
                           "[test.cpp:2:10] -> [test.cpp:3:22]: (warning) Either the condition 'p==0' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n"
                           "[test.cpp:2:10] -> [test.cpp:4:22]: (warning) Either the condition 'p==0' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
                           errout_str());

        check("void f() {\n"
              "    void* p1 = 0;\n"
              "    std::cout << p1;\n" // No char*
              "    char* p2 = 0;\n"
              "    std::cin >> (int)p;\n" // result casted
              "    std::cout << (int)p;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::string& str) {\n"
              "    long long ret = 0;\n"
              "    std::istringstream istr(str);\n"
              "    istr >> std::hex >> ret;\n" // Read integer
              "    return ret;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(int* i) {\n"
              "    if(i) return;\n"
              "    std::cout << i;\n" // Its no char* (#4240)
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #5811 false positive: (error) Null pointer dereference
        check("using namespace std;\n"
              "std::string itoip(int ip) {\n"
              "    stringstream out;\n"
              "    out << ((ip >> 0) & 0xFF);\n"
              "    return out.str();\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
        // avoid regression from first fix attempt for #5811...
        check("void deserialize(const std::string &data) {\n"
              "std::istringstream iss(data);\n"
              "unsigned int len = 0;\n"
              "if (!(iss >> len))\n"
              "    return;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

    }

    void nullpointerSmartPointer() {
        // extracttests.start: void dostuff(int);

        check("struct Fred { int x; };\n"
              "void f(std::shared_ptr<Fred> p) {\n"
              "  if (p) {}\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7] -> [test.cpp:4:11]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("struct Fred { int x; };\n"
              "void f(std::shared_ptr<Fred> p) {\n"
              "  p = nullptr;\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("struct Fred { int x; };\n"
              "void f(std::unique_ptr<Fred> p) {\n"
              "  if (p) {}\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7] -> [test.cpp:4:11]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n", errout_str());

        check("struct Fred { int x; };\n"
              "void f(std::unique_ptr<Fred> p) {\n"
              "  p = nullptr;\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("struct Fred { int x; };\n"
              "void f() {\n"
              "  std::shared_ptr<Fred> p;\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("struct Fred { int x; };\n"
              "void f(std::shared_ptr<Fred> p) {\n"
              "  p.reset();\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("struct Fred { int x; };\n"
              "void f(std::shared_ptr<Fred> p) {\n"
              "  Fred * pp = nullptr;\n"
              "  p.reset(pp);\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:11]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("struct Fred { int x; };\n"
              "void f(Fred& f) {\n"
              "  std::shared_ptr<Fred> p;\n"
              "  p.reset(&f);\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Fred { int x; };\n"
              "void f(std::shared_ptr<Fred> p) {\n"
              "  p.reset();\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("struct Fred { int x; };\n"
              "void f() {\n"
              "  std::shared_ptr<Fred> p(nullptr);\n"
              "  dostuff(p->x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Null pointer dereference: p [nullPointer]\n", errout_str());

        check("struct A {};\n"
              "void f(int n) {\n"
              "    std::unique_ptr<const A*[]> p;\n"
              "    p.reset(new const A*[n]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9216
        check("struct A {\n"
              "    void reset();\n"
              "    void f();\n"
              "};\n"
              "void g(std::unique_ptr<A> var) {\n"
              "    var->reset();\n"
              "    var->f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9439
        check("char* g();\n"
              "char* f() {\n"
              "    std::unique_ptr<char> x(g());\n"
              "    if( x ) {}\n"
              "    return x.release();\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #9496
        check("std::shared_ptr<int> f() {\n"
              "    return std::shared_ptr<int>(nullptr);\n"
              "}\n"
              "void g() {\n"
              "    int a = *f();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:15]: (error) Null pointer dereference: f() [nullPointer]\n", errout_str());
    }

    void nullpointerOutOfMemory() {
        check("void f() {\n"
              "    int *p = malloc(10);\n"
              "    *p = 0;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (warning) If memory allocation fails, then there is a possible null pointer dereference: p [nullPointerOutOfMemory]\n", errout_str());

        check("void f() {\n"
              "    int *p = malloc(10);\n"
              "    *(p+2) = 0;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8]: (error) If memory allocation fails: pointer addition with NULL pointer. [nullPointerArithmeticOutOfMemory]\n", errout_str());

        check("void f() {\n" // #13676
              "    int* q = static_cast<int*>(std::malloc(4));\n"
              "    *q = 0;\n"
              "    std::free(q);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (warning) If memory allocation fails, then there is a possible null pointer dereference: q [nullPointerOutOfMemory]\n", errout_str());
    }

    void functioncall() {    // #3443 - function calls
        // dereference pointer and then check if it's null
        {
            // function not seen
            check("void f(int *p) {\n"
                  "    *p = 0;\n"
                  "    foo(p);\n"
                  "    if (p) { }\n"
                  "}");
            ASSERT_EQUALS("", errout_str());

            // function seen (taking pointer parameter)
            check("void foo(int *p) { }\n"
                  "\n"
                  "void f(int *p) {\n"
                  "    *p = 0;\n"
                  "    foo(p);\n"
                  "    if (p) { }\n"
                  "}");
            ASSERT_EQUALS(
                "[test.cpp:6:9] -> [test.cpp:4:6]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
                errout_str());

            // function seen (taking reference parameter)
            check("void foo(int *&p) { }\n"
                  "\n"
                  "void f(int *p) {\n"
                  "    *p = 0;\n"
                  "    foo(p);\n"
                  "    if (p) { }\n"
                  "}", dinit(CheckOptions, $.inconclusive = true));
            ASSERT_EQUALS("", errout_str());

            // function implementation not seen
            check("void foo(int *p);\n"
                  "\n"
                  "void f(int *p) {\n"
                  "    *p = 0;\n"
                  "    foo(p);\n"
                  "    if (p) { }\n"
                  "}");
            ASSERT_EQUALS(
                "[test.cpp:6:9] -> [test.cpp:4:6]: (warning) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
                errout_str());

            // inconclusive
            check("void f(int *p) {\n"
                  "    *p = 0;\n"
                  "    foo(p);\n"
                  "    if (p) { }\n"
                  "}", dinit(CheckOptions, $.inconclusive = true));
            ASSERT_EQUALS(
                "[test.cpp:4:9] -> [test.cpp:2:6]: (warning, inconclusive) Either the condition 'p' is redundant or there is possible null pointer dereference: p. [nullPointerRedundantCheck]\n",
                errout_str());
        }

        // dereference struct pointer and then check if it's null
        {
            // function not seen
            check("void f(struct ABC *abc) {\n"
                  "    abc->a = 0;\n"
                  "    foo(abc);\n"
                  "    if (abc) { }\n"
                  "}");
            ASSERT_EQUALS("", errout_str());

            // function seen (taking pointer parameter)
            check("void foo(struct ABC *abc) { }\n"
                  "\n"
                  "void f(struct ABC *abc) {\n"
                  "    abc->a = 0;\n"
                  "    foo(abc);\n"
                  "    if (abc) { }\n"
                  "}");
            ASSERT_EQUALS(
                "[test.cpp:6:9] -> [test.cpp:4:5]: (warning) Either the condition 'abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n",
                errout_str());

            // function implementation not seen
            check("void foo(struct ABC *abc);\n"
                  "\n"
                  "void f(struct ABC *abc) {\n"
                  "    abc->a = 0;\n"
                  "    foo(abc);\n"
                  "    if (abc) { }\n"
                  "}");
            ASSERT_EQUALS(
                "[test.cpp:6:9] -> [test.cpp:4:5]: (warning) Either the condition 'abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n",
                errout_str());

            // inconclusive
            check("void f(struct ABC *abc) {\n"
                  "    abc->a = 0;\n"
                  "    foo(abc);\n"
                  "    if (abc) { }\n"
                  "}", dinit(CheckOptions, $.inconclusive = true));
            ASSERT_EQUALS(
                "[test.cpp:4:9] -> [test.cpp:2:5]: (warning, inconclusive) Either the condition 'abc' is redundant or there is possible null pointer dereference: abc. [nullPointerRedundantCheck]\n",
                errout_str());
        }
    }

    void nullpointerOutOfResources() {
        check("void f() {\n"
              "    FILE* fid = fopen(\"x.txt\", \"w\");\n"
              "    fprintf(fid, \"abcdef\");\n"
              "    fclose(fid);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:13]: (warning) If resource allocation fails, then there is a possible null pointer dereference: fid [nullPointerOutOfResources]\n"
            "[test.cpp:4:12]: (warning) If resource allocation fails, then there is a possible null pointer dereference: fid [nullPointerOutOfResources]\n",
            errout_str());
    }

    void functioncalllibrary() {
        SimpleTokenizer tokenizer(settingsDefault,*this,false);
        const char code[] = "void f() { int a,b,c; x(a,b,c); }";
        ASSERT_EQUALS(true, tokenizer.tokenize(code));
        const Token *xtok = Token::findsimplematch(tokenizer.tokens(), "x");

        // nothing bad..
        {
            constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                       "<def>\n"
                                       "  <function name=\"x\">\n"
                                       "    <arg nr=\"1\"></arg>\n"
                                       "    <arg nr=\"2\"></arg>\n"
                                       "    <arg nr=\"3\"></arg>\n"
                                       "  </function>\n"
                                       "</def>";

            Library library;
            ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));

            const std::list<const Token *> null = CheckNullPointer::parseFunctionCall(*xtok, library);
            ASSERT_EQUALS(0U, null.size());
        }

        // for 1st parameter null pointer is not ok..
        {
            constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                       "<def>\n"
                                       "  <function name=\"x\">\n"
                                       "    <arg nr=\"1\"><not-null/></arg>\n"
                                       "    <arg nr=\"2\"></arg>\n"
                                       "    <arg nr=\"3\"></arg>\n"
                                       "  </function>\n"
                                       "</def>";

            Library library;
            ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));

            const std::list<const Token *> null = CheckNullPointer::parseFunctionCall(*xtok, library);
            ASSERT_EQUALS(1U, null.size());
            ASSERT_EQUALS("a", null.front()->str());
        }
    }

    void functioncallDefaultArguments() {

        check("void f(int *p = 0) {\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:6]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n", errout_str());

        check("void f(int *p = 0) {\n"
              "    if (!p)\n"
              "        return;\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char a, int *p = 0) {\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:6]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n", errout_str());

        check("void f(int *p = 0) {\n"
              "    printf(\"p = %d\", *p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:23]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n", errout_str());

        check("void f(int *p = 0) {\n"
              "    printf(\"p[1] = %d\", p[1]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n", errout_str());

        check("void f(int *p = 0) {\n"
              "    buf[p] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    if (p != 0 && bar())\n"
              "      *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p) {\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    if (p != 0)\n"
              "      *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    int y;\n"
              "    if (p == 0)\n"
              "      p = &y;\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a, int *p = 0) {\n"
              "    if (a != 0)\n"
              "      *p = 0;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:3:8]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n",
            errout_str());

        check("void f(int *p = 0) {\n"
              "    p = a;\n"
              "    *p = 0;\n" // <- don't simplify and verify
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    p += a;\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int *p = 0) {\n"
              "    if (p == 0) {\n"
              "        return 0;\n"
              "    }\n"
              "    return *p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    std::cout << p ? *p : 0;\n" // Due to operator precedence, this is equivalent to: (std::cout << p) ? *p : 0;
              "}");
        ASSERT_EQUALS("[test.cpp:2:23]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n", errout_str()); // Check the first branch of ternary

        check("void f(char *p = 0) {\n"
              "    std::cout << p ? *p : 0;\n" // Due to operator precedence, this is equivalent to: (std::cout << p) ? *p : 0;
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:18]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n"
            "[test.cpp:2:23]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n",   // duplicate
            errout_str());

        check("void f(int *p = 0) {\n"
              "    std::cout << (p ? *p : 0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    std::cout << p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    std::cout << (p && p[0] ? *p : 42);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void isEmpty(int *p = 0) {\n"
              "    return p && *p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void g(int *p = 0) {\n"
              "    return !p || *p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        // bar may initialize p but be can't know for sure without knowing
        // if p is passed in by reference and is modified by bar()
        check("void f(int *p = 0) {\n"
              "    bar(p);\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    printf(\"%p\", p);\n"
              "    *p = 0;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:6]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n", errout_str());

        // The init() function may or may not initialize p, but since the address
        // of p is passed in, it's a good bet that p may be modified and
        // so we should not report an error.
        check("void f(int *p = 0) {\n"
              "    init(&p);\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void init(int* &g);\n"
              "void f(int *p = 0) {\n"
              "    init(p);\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    if (p == 0) {\n"
              "        init(&p);\n"
              "    }\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int *p = 0) {\n"
              "    if (p == 0) {\n"
              "        throw SomeException;\n"
              "    }\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int x, int *p = 0) {\n"
              "    int var1 = x ? *p : 5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:21]: (warning) Possible null pointer dereference if the default parameter value is used: p [nullPointerDefaultArg]\n", errout_str());

        check("void f(int* i = nullptr) { *i = 0; }\n" // #11567
              "void g() { f(); }\n");
        ASSERT_EQUALS("[test.cpp:1:29]: (warning) Possible null pointer dereference if the default parameter value is used: i [nullPointerDefaultArg]\n", errout_str());
    }

    void nullpointer_internal_error() { // ticket #5080
        check("struct A { unsigned int size; };\n"
              "struct B { struct A *a; };\n"
              "void f(struct B *b) {\n"
              "    unsigned int j;\n"
              "    for (j = 0; j < b[0].a->size; ++j) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ticket6505() {
        check("void foo(MythSocket *socket) {\n"
              "  bool do_write=0;\n"
              "  if (socket) {\n"
              "    do_write=something();\n"
              "  }\n"
              "  if (do_write) {\n"
              "    socket->func();\n"
              "  }\n"
              "}\n"
              "void bar() {\n"
              "  foo(0);\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
    }

    void subtract() {
        check("void foo(char *s) {\n"
              "  char *p = s - 20;\n"
              "}\n"
              "void bar() { foo(0); }");
        ASSERT_EQUALS("[test.cpp:2:15]: (error) Overflow in pointer arithmetic, NULL pointer is subtracted. [nullPointerArithmetic]\n",
                      errout_str());

        check("void foo(char *s) {\n"
              "  if (!s) {}\n"
              "  char *p = s - 20;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7] -> [test.cpp:3:15]: (warning) Either the condition '!s' is redundant or there is overflow in pointer subtraction. [nullPointerArithmeticRedundantCheck]\n", errout_str());

        check("void foo(char *s) {\n"
              "  s -= 20;\n"
              "}\n"
              "void bar() { foo(0); }");
        ASSERT_EQUALS("[test.cpp:2:5]: (error) Overflow in pointer arithmetic, NULL pointer is subtracted. [nullPointerArithmetic]\n",
                      errout_str());

        check("void foo(char *s) {\n"
              "  if (!s) {}\n"
              "  s -= 20;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7] -> [test.cpp:3:5]: (warning) Either the condition '!s' is redundant or there is overflow in pointer subtraction. [nullPointerArithmeticRedundantCheck]\n", errout_str());

        check("int* f8() { int *x = NULL; return --x; }");
        ASSERT_EQUALS("[test.cpp:1:35]: (error) Overflow in pointer arithmetic, NULL pointer is subtracted. [nullPointerArithmetic]\n", errout_str());

        check("int* f9() { int *x = NULL; return x--; }");
        ASSERT_EQUALS("[test.cpp:1:36]: (error) Overflow in pointer arithmetic, NULL pointer is subtracted. [nullPointerArithmetic]\n", errout_str());
    }

    void addNull() {
        check("void foo(char *s) {\n"
              "  char * p = s + 20;\n"
              "}\n"
              "void bar() { foo(0); }");
        ASSERT_EQUALS("[test.cpp:2:16]: (error) Pointer addition with NULL pointer. [nullPointerArithmetic]\n", errout_str());

        check("void foo(char *s) {\n"
              "  if (!s) {}\n"
              "  char * p = s + 20;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7] -> [test.cpp:3:16]: (warning) Either the condition '!s' is redundant or there is pointer arithmetic with NULL pointer. [nullPointerArithmeticRedundantCheck]\n", errout_str());

        check("void foo(char *s) {\n"
              "  char * p = 20 + s;\n"
              "}\n"
              "void bar() { foo(0); }");
        ASSERT_EQUALS("[test.cpp:2:17]: (error) Pointer addition with NULL pointer. [nullPointerArithmetic]\n", errout_str());

        check("void foo(char *s) {\n"
              "  if (!s) {}\n"
              "  char * p = 20 + s;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7] -> [test.cpp:3:17]: (warning) Either the condition '!s' is redundant or there is pointer arithmetic with NULL pointer. [nullPointerArithmeticRedundantCheck]\n", errout_str());

        check("void foo(char *s) {\n"
              "  s += 20;\n"
              "}\n"
              "void bar() { foo(0); }");
        ASSERT_EQUALS("[test.cpp:2:5]: (error) Pointer addition with NULL pointer. [nullPointerArithmetic]\n", errout_str());

        check("void foo(char *s) {\n"
              "  if (!s) {}\n"
              "  s += 20;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7] -> [test.cpp:3:5]: (warning) Either the condition '!s' is redundant or there is pointer arithmetic with NULL pointer. [nullPointerArithmeticRedundantCheck]\n", errout_str());

        check("int* f7() { int *x = NULL; return ++x; }");
        ASSERT_EQUALS("[test.cpp:1:35]: (error) Pointer addition with NULL pointer. [nullPointerArithmetic]\n", errout_str());

        check("int* f10() { int *x = NULL; return x++; }");
        ASSERT_EQUALS("[test.cpp:1:37]: (error) Pointer addition with NULL pointer. [nullPointerArithmetic]\n", errout_str());

        check("class foo {};\n"
              "const char* get() const { return 0; }\n"
              "void f(foo x) { if (get()) x += get(); }");
        ASSERT_EQUALS("", errout_str());

        check("typedef struct { uint8_t* buf, *buf_end; } S;\n" // #11117
              "void f(S* s, uint8_t* buffer, int buffer_size) {\n"
              "    if (buffer_size < 0) {\n"
              "        buffer_size = 0;\n"
              "        buffer = NULL;\n"
              "    }\n"
              "    s->buf = buffer;\n"
              "    s->buf_end = s->buf + buffer_size;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void isPointerDeRefFunctionDecl() {
        check("const char** get() { return 0; }");
        ASSERT_EQUALS("", errout_str());
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
        Check& c = getCheck<CheckNullPointer>();
        fileInfo.push_back(c.getFileInfo(tokenizer, settings, ""));
        c.analyseWholeProgram(*ctu, fileInfo, settings, *this); // TODO: check result
        while (!fileInfo.empty()) {
            delete fileInfo.back();
            fileInfo.pop_back();
        }
        delete ctu;
    }

    void ctuTest() {
        setMultiline();

        ctu("void f(int *fp) {\n"
            "    a = *fp;\n"
            "}\n"
            "int main() {\n"
            "  int *p = 0;\n"
            "  f(p);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2:10]: error: Null pointer dereference: fp [ctunullpointer]\n"
                      "[test.cpp:5:12]: note: Assignment 'p=0', assigned value is 0\n"
                      "[test.cpp:6:4]: note: Calling function f, 1st argument is null\n"
                      "[test.cpp:2:10]: note: Dereferencing argument fp that is null\n", errout_str());

        ctu("void use(int *p) { a = *p + 3; }\n"
            "void call(int x, int *p) { x++; use(p); }\n"
            "int main() {\n"
            "  call(4,0);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:1:25]: error: Null pointer dereference: p [ctunullpointer]\n"
                      "[test.cpp:4:7]: note: Calling function call, 2nd argument is null\n"
                      "[test.cpp:2:33]: note: Calling function use, 1st argument is null\n"
                      "[test.cpp:1:25]: note: Dereferencing argument p that is null\n", errout_str());

        ctu("void dostuff(int *x, int *y) {\n"
            "  if (!var)\n"
            "    return -1;\n"  // <- early return
            "  *x = *y;\n"
            "}\n"
            "\n"
            "void f() {\n"
            "  dostuff(a, 0);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        ctu("void dostuff(int *x, int *y) {\n"
            "  if (cond)\n"
            "    *y = -1;\n"  // <- conditionally written
            "  *x = *y;\n"
            "}\n"
            "\n"
            "void f() {\n"
            "  dostuff(a, 0);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // else
        ctu("void dostuff(int mask, int *p) {\n"
            "  if (mask == 13) ;\n"
            "  else *p = 45;\n"
            "}\n"
            "\n"
            "void f() {\n"
            "  dostuff(0, 0);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // ?, &&, ||
        ctu("void dostuff(int mask, int *p) {\n"
            "  x = (mask & 1) ? *p : 0;\n"
            "}\n"
            "\n"
            "void f() {\n"
            "  dostuff(0, 0);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        ctu("void g(int* x) { *x; }\n"
            "void f(int* x) {\n"
            "    if (x)\n"
            "        g(x);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        ctu("size_t f(int* p) {\n"
            "    size_t len = sizeof(*p);\n"
            "    return len;\n"
            "}\n"
            "void g() {\n"
            "    f(NULL);\n"
            "}\n");
        ASSERT_EQUALS("", errout_str());

        ctu("size_t f(int* p) {\n"
            "    size_t len = alignof(*p);\n"
            "    return len;\n"
            "}\n"
            "void g() {\n"
            "    f(NULL);\n"
            "}\n");
        ASSERT_EQUALS("", errout_str());

        // ctu: memory allocation fails
        ctu("void f(int* p) {\n"
            "    *p = 0;\n"
            "}\n"
            "void g() {\n"
            "    int* q = (int*)malloc(4);\n"
            "    f(q);\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:2:6]: warning: If memory allocation fails, then there is a possible null pointer dereference: p [ctunullpointerOutOfMemory]\n"
                      "[test.cpp:5:26]: note: Assuming allocation function fails\n"
                      "[test.cpp:5:14]: note: Assignment 'q=(int*)malloc(4)', assigned value is 0\n"
                      "[test.cpp:6:6]: note: Calling function f, 1st argument is null\n"
                      "[test.cpp:2:6]: note: Dereferencing argument p that is null\n", errout_str());

        // ctu: resource allocation fails
        ctu("void foo(FILE* f) {\n"
            "    fprintf(f, a);\n"
            "}\n"
            "void bar() {\n"
            "    FILE* f = fopen(notexist,t);\n"
            "    foo(f);\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:2:13]: warning: If resource allocation fails, then there is a possible null pointer dereference: f [ctunullpointerOutOfResources]\n"
                      "[test.cpp:5:20]: note: Assuming allocation function fails\n"
                      "[test.cpp:5:20]: note: Assignment 'f=fopen(notexist,t)', assigned value is 0\n"
                      "[test.cpp:6:8]: note: Calling function foo, 1st argument is null\n"
                      "[test.cpp:2:13]: note: Dereferencing argument f that is null\n", errout_str());
    }
};

REGISTER_TEST(TestNullPointer)
