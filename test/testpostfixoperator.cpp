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

#include "checkpostfixoperator.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>

class TestPostfixOperator : public TestFixture {
public:
    TestPostfixOperator() : TestFixture("TestPostfixOperator") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::performance).build();

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        CheckPostfixOperatorImpl checkPostfixOperator(&tokenizer, settings, this);
        checkPostfixOperator.postfixOperator();
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(testsimple);
        TEST_CASE(testfor);
        TEST_CASE(testvolatile);
        TEST_CASE(testiterator);
        TEST_CASE(test2168);
        TEST_CASE(pointerSimplest);
        TEST_CASE(pointer);   // #2321 - postincrement of pointer is OK
        TEST_CASE(testtemplate); // #4686
        TEST_CASE(testmember);
        TEST_CASE(testcomma);
        TEST_CASE(testauto); // #8350
    }

    void testsimple() {
        check("int main()\n"
              "{\n"
              "    unsigned int k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    k++;\n"
              "    std::cout << k << std::endl;\n"
              "    if(k) {\n"
              "        k++;\n"
              "    }\n"
              "    std::cout << k << std::endl;\n"
              "    k--;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class K {};"
              "int main()\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    k++;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("struct K {};"
              "void foo()\n"
              "{\n"
              "    K k(0);\n"
              "    k++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("struct K {};\n"
              "void foo(K& k)\n"
              "{\n"
              "    k++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("union K {};"
              "void foo()\n"
              "{\n"
              "    K k(0);\n"
              "    k++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("class K {};"
              "int main()\n"
              "{\n"
              "    K k(1);\n"
              "    std::cout << k << std::endl;\n"
              "    if(k) {\n"
              "        k++;\n"
              "    }\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:9]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("class K {};"
              "int main()\n"
              "{\n"
              "    K k(1);\n"
              "    std::cout << k << std::endl;\n"
              "    if(k) {\n"
              "        ++k;\n"
              "    }\n"
              "    k++;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:5]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());


        check("class K {};"
              "int main()\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    k--;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("class K {};"
              "int main()\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    ++k;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class K {};"
              "int main()\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    --k;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9042
        check("template <class T>\n"
              "class c {\n"
              "    int i = 0;\n"
              "    c() { i--; }\n"
              "};\n"
              "template <class T>\n"
              "class s {};\n"
              "using BOOL = char;");
        ASSERT_EQUALS("", errout_str());
    }

    void testfor() {
        check("int main()\n"
              "{\n"
              "    for ( unsigned int i=0; i <= 10; i++) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class K {};\n"
              "int main()\n"
              "{\n"
              "    for ( K i(0); i <= 10; i++) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:28]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("class K {};\n"
              "int main()\n"
              "{\n"
              "    for ( K i(0); i <= 10; ++i) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class K {};\n"
              "int main()\n"
              "{\n"
              "    for ( K i(10); i > 1; i--) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:27]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("class K {};\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    for ( K i=10; i > 1; --i) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::string &s) {\n" // #7731
              "    for (std::string::const_iterator i = s.begin(), r = s.end() - 1; i != r; r--) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:78]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());
    }

    void testvolatile() {
        check("class K {};\n"
              "int main()\n"
              "{\n"
              "    volatile K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    k++;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());
    }

    void testiterator() {
        check("class Base {};\n"
              "int main() {\n"
              "    std::vector<Base*> v;\n"
              "    v.push_back(new Base());\n"
              "    v.push_back(new Base());\n"
              "    for (std::vector<Base*>::iterator i=v.begin(); i!=v.end(); i++) {\n"
              "        ;;\n"
              "    }\n"
              "    v.clear();\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:64]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("int main() {\n"
              "    std::vector<int> v;\n"
              "    std::vector<int>::iterator it;\n"
              "    for( int i=0; i < 10; ++i ) v.push_back(i);\n"
              "        unsigned int total = 0;\n"
              "    it = v.begin();\n"
              "    while( it != v.end() ) {\n"
              "       total += *it;\n"
              "       it++;\n"
              "    }\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:8]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("int main() {\n"
              "    std::vector<int> v;\n"
              "    std::vector<int>::const_iterator it;\n"
              "    for( int i=0; i < 10; ++i ) v.push_back(i);\n"
              "        unsigned int total = 0;\n"
              "    it = v.begin();\n"
              "    while( it != v.end() ) {\n"
              "       it++;\n"
              "    }\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:8]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("int main() {\n"
              "    std::vector<int> v;\n"
              "    std::vector<int>::iterator it;\n"
              "    for( int i=0; i < 10; ++i ) v.push_back(i);\n"
              "        unsigned int total = 0;\n"
              "    std::vector<int>::reverse_iterator rit;\n"
              "    rit= v.rend();\n"
              "    while( rit != v.rbegin() ) {\n"
              "       rit--;\n"
              "    }\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:8]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

    }

    void test2168() {
        ASSERT_THROW_INTERNAL(check("--> declare allocator lock here\n"
                                    "int main(){}"),
                              AST);
    }

    void pointerSimplest() {
        check("void f(int* p){\n"
              "    p++;\n"
              "    std::cout << *p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void pointer() {
        check("static struct class * ab;\n"
              "int * p;\n"
              "\n"
              "void f() {\n"
              "    p++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testtemplate() {
        check("bool foo() {\n"
              "    std::vector<FilterConfigCacheEntry>::iterator aIter(aImport.begin());\n"
              "    aIter++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());
    }

    void testmember() {
        check("bool foo() {\n"
              "    class A {}; class B {A a;};\n"
              "    B b;\n"
              "    b.a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("bool foo() {\n"
              "    class A {}; class B {A a;};\n"
              "    B b;\n"
              "    foo(b.a++);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testcomma() {
        check("bool foo(int i) {\n"
              "    class A {};\n"
              "    A a;\n"
              "    i++, a++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (performance) Prefer prefix ++/-- operators for non-primitive types. [postfixOperator]\n", errout_str());

        check("bool foo(int i) {\n"
              "    class A {};\n"
              "    A a;\n"
              "    foo(i, a++);\n"
              "    foo(a++, i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testauto() { // #8350
        check("enum class Color { Red = 0, Green = 1, };\n"
              "int fun(const Color color) {\n"
              "    auto a = 0;\n"
              "    for (auto i = static_cast<int>(color); i < 10; i++) {\n"
              "        a += i;\n"
              "    }\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestPostfixOperator)
