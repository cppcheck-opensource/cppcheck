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


#include "checkassert.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>

class TestAssert : public TestFixture {
public:
    TestAssert() : TestFixture("TestAssert") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::warning).build();

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        CheckAssert check;
        runChecks(check, tokenizer, *this);
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(assignmentInAssert);
        TEST_CASE(functionCallInAssert);
        TEST_CASE(memberFunctionCallInAssert);
        TEST_CASE(safeFunctionCallInAssert);
        TEST_CASE(crash);
    }


    void safeFunctionCallInAssert() {
        check(
            "int a;\n"
            "bool b = false;\n"
            "int foo() {\n"
            "   if (b) { a = 1+2 };\n"
            "   return a;\n"
            "}\n"
            "assert(foo() == 3);");
        ASSERT_EQUALS("", errout_str());

        check(
            "int foo(int a) {\n"
            "    int b=a+1;\n"
            "    return b;\n"
            "}\n"
            "assert(foo(1) == 2);");
        ASSERT_EQUALS("", errout_str());
    }

    void functionCallInAssert() {
        check(
            "int a;\n"
            "int foo() {\n"
            "    a = 1+2;\n"
            "    return a;\n"
            "}\n"
            "assert(foo() == 3);");
        ASSERT_EQUALS("[test.cpp:6:8]: (warning) Assert statement calls a function which may have desired side effects: 'foo'. [assertWithSideEffect]\n", errout_str());

        //  Ticket #4937 "false positive: Assert calls a function which may have desired side effects"
        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square sq ) {\n"
              "      sq &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square &sq ) {\n"
              "      sq &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:25]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'. [assertWithSideEffect]\n", errout_str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square *sq ) {\n"
              "      *sq &= 0x38;\n"
              "      return *sq == 0 || *sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:25]: (warning) Assert statement calls a function which may have desired side effects: 'isRank1Or8'. [assertWithSideEffect]\n", errout_str());

        check("struct SquarePack {\n"
              "   static bool isRank1Or8( Square *sq ) {\n"
              "      sq &= 0x38;\n"
              "      return sq == 0 || sq == 0x38;\n"
              "    }\n"
              "};\n"
              "void foo() {\n"
              "   assert( !SquarePack::isRank1Or8(push2) );\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Geometry {\n"
              "    int nbv;\n"
              "    int empty() { return (nbv == 0); }\n"
              "    void ReadGeometry();\n"
              "};\n"
              "\n"
              "void Geometry::ReadGeometry() {\n"
              "    assert(empty());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #4811
              "    void f() const;\n"
              "    bool g(std::ostream& os = std::cerr) const;\n"
              "};\n"
              "void S::f() const {\n"
              "    assert(g());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void memberFunctionCallInAssert() {
        check("struct SquarePack {\n"
              "   void Foo();\n"
              "};\n"
              "void foo(SquarePack s) {\n"
              "   assert( s.Foo() );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:14]: (warning) Assert statement calls a function which may have desired side effects: 'Foo'. [assertWithSideEffect]\n", errout_str());

        check("struct SquarePack {\n"
              "   int Foo() const;\n"
              "};\n"
              "void foo(SquarePack* s) {\n"
              "   assert( s->Foo() );\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct SquarePack {\n"
              "   static int Foo();\n"
              "};\n"
              "void foo(SquarePack* s) {\n"
              "   assert( s->Foo() );\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct SquarePack {\n"
              "};\n"
              "void foo(SquarePack* s) {\n"
              "   assert( s->Foo() );\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assignmentInAssert() {
        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a = 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (warning) Assert statement modifies 'a'. [assignmentInAssert]\n", errout_str());

        check("void f(int a) {\n"
              "    assert(a == 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a, int b) {\n"
              "    assert(a == 2 && (b = 1));\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25]: (warning) Assert statement modifies 'b'. [assignmentInAssert]\n", errout_str());

        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a += 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (warning) Assert statement modifies 'a'. [assignmentInAssert]\n", errout_str());

        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a *= 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (warning) Assert statement modifies 'a'. [assignmentInAssert]\n", errout_str());

        check("void f() {\n"
              "    int a; a = 0;\n"
              "    assert(a -= 2);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (warning) Assert statement modifies 'a'. [assignmentInAssert]\n", errout_str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(a--);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (warning) Assert statement modifies 'a'. [assignmentInAssert]\n", errout_str());

        check("void f() {\n"
              "    int a = 0;\n"
              "    assert(--a);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (warning) Assert statement modifies 'a'. [assignmentInAssert]\n", errout_str());

        check("void f() {\n"
              "  assert(std::all_of(first, last, []() {\n"
              "                  auto tmp = x.someValue();\n"
              "                  auto const expected = someOtherValue;\n"
              "                  return tmp == expected;\n"
              "                }));\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void crash() {
        check("void foo() {\n"
              "  assert(sizeof(struct { int a[x++]; })==sizeof(int));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n" // #9790
              "  assert(kad_bucket_hash(&(kad_guid) { .bytes = { 0 } }, & (kad_guid){.bytes = { 0 }}) == -1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestAssert)
