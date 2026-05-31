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

#include "check64bit.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>

class Test64BitPortability : public TestFixture {
public:
    Test64BitPortability() : TestFixture("Test64BitPortability") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::portability).library("std.cfg").build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(novardecl);
        TEST_CASE(functionpar);
        TEST_CASE(structmember);
        TEST_CASE(ptrcompare);
        TEST_CASE(ptrarithmetic);
        TEST_CASE(returnIssues);
        TEST_CASE(assignment);
    }

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], bool cpp = true) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        Check64BitPortabilityImpl check64BitPortability(&tokenizer, settings, *this);
        check64BitPortability.pointerassignment();
    }

    void assignment() {
        // #8631
        check("using CharArray = char[16];\n"
              "void f() {\n"
              "    CharArray foo = \"\";\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct T { std::vector<int>*a[2][2]; };\n" // #11560
              "void f(T& t, int i, int j) {\n"
              "    t.a[i][j] = new std::vector<int>;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f();\n" // #11522
              "void g() {\n"
              "    int (*fp)() = *(int(*)())f;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void novardecl() {
        // if the variable declarations can't be seen then skip the warning
        check("void foo()\n"
              "{\n"
              "    a = p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void functionpar() {
        check("int foo(int *p)\n"
              "{\n"
              "    int a = p;\n"
              "    return a + 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (portability) Assigning a pointer to an integer is not portable. [AssignmentAddressToInteger]\n", errout_str());

        check("int foo(int p[])\n"
              "{\n"
              "    int a = p;\n"
              "    return a + 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (portability) Assigning a pointer to an integer is not portable. [AssignmentAddressToInteger]\n", errout_str());

        check("int foo(int p[])\n"
              "{\n"
              "    int *a = p;\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (portability) Returning an address value in a function with integer return type is not portable. [CastAddressToIntegerAtReturn]\n", errout_str());

        check("void foo(int x)\n"
              "{\n"
              "    int *p = x;\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (portability) Assigning an integer to a pointer is not portable. [AssignmentIntegerToAddress]\n", errout_str());

        check("int f(const char *p) {\n" // #4659
              "    return 6 + p[2] * 256;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int foo(int *p) {\n" // #6096
              "    bool a = p;\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::array<int,2> f();\n"
              "void g() {\n"
              "    std::array<int, 2> a = f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::array<int,2> f(int x);\n"
              "void g(int i) {\n"
              "    std::array<int, 2> a = f(i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("typedef std::array<int, 2> Array;\n"
              "Array f(int x);\n"
              "void g(int i) {\n"
              "    Array a = f(i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("typedef std::array<int, 2> Array;\n"
              "Array f();\n"
              "void g(int i) {\n"
              "    Array a = f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #9951
              "    enum E { E0 };\n"
              "    std::array<double, 1> g(S::E);\n"
              "};\n"
              "void f() {\n"
              "    std::array<double, 1> a = S::g(S::E::E0);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("char* f(char* p) {\n"
              "    return p ? p : 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int* p) {\n"
              "    int n = (int)(size_t)*p;\n"
              "    return n;\n"
              "}\n", false);
        ASSERT_EQUALS("", errout_str());
    }

    void structmember() {
        check("struct Foo { int *p; };\n"
              "void f(struct Foo *foo) {\n"
              "    int i = foo->p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (portability) Assigning a pointer to an integer is not portable. [AssignmentAddressToInteger]\n", errout_str());

        check("struct S {\n" // #10145
              "    enum class E { e1, e2 };\n"
              "    E e;\n"
              "    char* e1;\n"
              "};\n"
              "void f(S& s) {\n"
              "    s.e = S::E::e1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void ptrcompare() {
        // Ticket #2892
        check("void foo(int *p) {\n"
              "    int a = (p != NULL);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ptrarithmetic() {
        // #3073
        check("void foo(int *p) {\n"
              "    int x = 10;\n"
              "    int *a = p + x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int *p) {\n"
              "    int x = 10;\n"
              "    int *a = x + p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int *p) {\n"
              "    int x = 10;\n"
              "    int *a = x * x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (portability) Assigning an integer to a pointer is not portable. [AssignmentIntegerToAddress]\n", errout_str());

        check("void foo(int *start, int *end) {\n"
              "    int len;\n"
              "    int len = end + 10 - start;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnIssues() {
        check("void* foo(int i) {\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (portability) Returning an integer in a function with pointer return type is not portable. [CastIntegerToAddressAtReturn]\n", errout_str());

        check("void* foo(int* i) {\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void* foo() {\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int foo(int i) {\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Foo {};\n"
              "\n"
              "int* dostuff(Foo foo) {\n"
              "  return foo;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int foo(char* c) {\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (portability) Returning an address value in a function with integer return type is not portable. [CastAddressToIntegerAtReturn]\n", errout_str());

        check("int foo(char* c) {\n"
              "    return 1+c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (portability) Returning an address value in a function with integer return type is not portable. [CastAddressToIntegerAtReturn]\n", errout_str());

        check("std::string foo(char* c) {\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int foo(char *a, char *b) {\n" // #4486
              "    return a + 1 - b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct s {\n" // 4642
              "   int i;\n"
              "};\n"
              "int func(struct s *p) {\n"
              " return 1 + p->i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("static void __iomem *f(unsigned int port_no) {\n"
              "  void __iomem *mmio = hpriv->mmio;\n"
              "  return mmio + (port_no * 0x80);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7247: don't check return statements in nested functions..
        check("int foo() {\n"
              "  struct {\n"
              "    const char * name() { return \"abc\"; }\n"
              "  } table;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7451: Lambdas
        check("const int* test(std::vector<int> outputs, const std::string& text) {\n"
              "  auto it = std::find_if(outputs.begin(), outputs.end(),\n"
              "     [&](int ele) { return \"test\" == text; });\n"
              "  return nullptr;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12159
              "    std::future<int> f() const {\n"
              "        return {};\n"
              "    }\n"
              "};\n"
              "int g() {\n"
              "    std::shared_ptr<S> s = std::make_shared<S>();\n"
              "    auto x = s->f();\n"
              "    return x.get();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(int* p) {\n" // #14294
              "    return (int)p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (portability) Returning an address value in a function with integer return type is not portable. [CastAddressToIntegerAtReturn]\n",
                      errout_str());

        check("int f(int* p) {\n"
              "    return reinterpret_cast<int>(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (portability) Returning an address value in a function with integer return type is not portable. [CastAddressToIntegerAtReturn]\n",
                      errout_str());

        check("bool f(const int* p) {\n"
              "    return p;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(Test64BitPortability)
