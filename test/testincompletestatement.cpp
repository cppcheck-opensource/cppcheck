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

#include "checkother.h"
#include "errortypes.h"
#include "helpers.h"
#include "settings.h"
#include "fixture.h"

#include <cstddef>
#include <string>

class TestIncompleteStatement : public TestFixture {
public:
    TestIncompleteStatement() : TestFixture("TestIncompleteStatement") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::warning).library("std.cfg").build();
    const Settings settings_i = settingsBuilder(settings).certainty(Certainty::inconclusive).build();

    struct CheckOptions
    {
        bool inconclusive = false;
        bool cpp = true;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings &settings1 = options.inconclusive ? settings_i : settings;

        SimpleTokenizer2 tokenizer(settings1, *this, code, options.cpp ? "test.cpp" : "test.c");

        // Tokenize..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check for incomplete statements..
        CheckOther checkOther(&tokenizer, &settings1, this);
        checkOther.checkIncompleteStatement();
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);
        TEST_CASE(test4);
        TEST_CASE(test5);
        TEST_CASE(test6);
        TEST_CASE(test7);
        TEST_CASE(test_numeric);
        TEST_CASE(void0); // #6327: No fp for statement "(void)0;"
        TEST_CASE(intarray);
        TEST_CASE(structarraynull);
        TEST_CASE(structarray);
        TEST_CASE(conditionalcall);     // ; 0==x ? X() : Y();
        TEST_CASE(structinit);          // #2462 : ABC abc{1,2,3};
        TEST_CASE(returnstruct);
        TEST_CASE(cast);                // #3009 : (struct Foo *)123.a = 1;
        TEST_CASE(increment);           // #3251 : FP for increment
        TEST_CASE(cpp11init);           // #5493 : int i{1};
        TEST_CASE(cpp11init2);          // #8449
        TEST_CASE(cpp11init3);          // #8995
        TEST_CASE(block);               // ({ do_something(); 0; })
        TEST_CASE(mapindex);
        TEST_CASE(commaoperator1);
        TEST_CASE(commaoperator2);
        TEST_CASE(redundantstmts);
        TEST_CASE(vardecl);
        TEST_CASE(archive);             // ar & x
        TEST_CASE(ast);
        TEST_CASE(oror);                // dostuff() || x=32;
        TEST_CASE(functioncall);
    }

    void test1() {
        check("void foo()\n"
              "{\n"
              "    const char def[] =\n"
              "    \"abc\";\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void test2() {
        check("void foo()\n"
              "{\n"
              "    \"abc\";\n"
              "}");

        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Redundant code: Found a statement that begins with string constant. [constStatement]\n", errout_str());
    }

    void test3() {
        check("void foo()\n"
              "{\n"
              "    const char *str[] = { \"abc\" };\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void test4() {
        check("void foo()\n"
              "{\n"
              "const char *a =\n"
              "{\n"
              "\"hello \"\n"
              "\"more \"\n"
              "\"world\"\n"
              "};\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void test5() {
        check("void foo()\n"
              "{\n"
              "    50;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Redundant code: Found a statement that begins with numeric constant. [constStatement]\n", errout_str());
    }

    void test6() {
        // don't crash
        check("void f() {\n"
              "  1 == (two + three);\n"
              "  2 != (two + three);\n"
              "  (one + two) != (two + three);\n"
              "}");
    }

    void test7() { // #9335
        check("namespace { std::string S = \"\"; }\n"
              "\n"
              "class C {\n"
              "public:\n"
              "  explicit C(const std::string& s);\n"
              "};\n"
              "\n"
              "void f() {\n"
              "  for (C c(S); ; ) {\n"
              "    (void)c;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void test_numeric() {
        check("struct P {\n"
              "    double a;\n"
              "    double b;\n"
              "};\n"
              "void f() {\n"
              "    const P values[2] =\n"
              "    {\n"
              "        { 346.1,114.1 }, { 347.1,111.1 }\n"
              "    };\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void void0() { // #6327
        check("void f() { (void*)0; }");
        ASSERT_EQUALS("", errout_str());

        check("#define X  0\n"
              "void f() { X; }");
        ASSERT_EQUALS("", errout_str());
    }

    void intarray() {
        check("int arr[] = { 100/2, 1*100 };");
        ASSERT_EQUALS("", errout_str());
    }

    void structarraynull() {
        check("struct st arr[] = {\n"
              "    { 100/2, 1*100 }\n"
              "    { 90, 70 }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structarray() {
        check("struct st arr[] = {\n"
              "    { 100/2, 1*100 }\n"
              "    { 90, 70 }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void conditionalcall() {
        check("void f() {\n"
              "    0==x ? X() : Y();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structinit() {
        // #2462 - C++11 struct initialization
        check("void f() {\n"
              "    ABC abc{1,2,3};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6260 - C++11 array initialization
        check("void foo() {\n"
              "    static const char* a[][2] {\n"
              "        {\"b\", \"\"},\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #2482 - false positive for empty struct
        check("struct A {};");
        ASSERT_EQUALS("", errout_str());

        // #4387 - C++11 initializer list
        check("A::A() : abc{0} {}");
        ASSERT_EQUALS("", errout_str());

        // #5042 - C++11 initializer list
        check("A::A() : abc::def<int>{0} {}");
        ASSERT_EQUALS("", errout_str());

        // #4503 - vector init
        check("void f() { vector<int> v{1}; }");
        ASSERT_EQUALS("", errout_str());
    }

    void returnstruct() {
        check("struct s foo() {\n"
              "    return (struct s){0,0};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #4754
        check("unordered_map<string, string>  foo() {\n"
              "    return {\n"
              "        {\"hi\", \"there\"},\n"
              "        {\"happy\", \"sad\"}\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct s foo() {\n"
              "  return (struct s){0};\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void cast() {
        check("void f() {\n"
              "    ((struct foo *)(0x1234))->xy = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f(const std::exception& e) {\n" // #10918
              "    try {\n"
              "        dynamic_cast<const InvalidTypeException&>(e);\n"
              "        return true;\n"
              "    }\n"
              "    catch (...) {\n"
              "        return false;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void increment() {
        check("void f() {\n"
              "    int x = 1;\n"
              "    x++, x++;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void cpp11init() {
        check("void f() {\n"
              "    int x{1};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<int> f(int* p) {\n"
              "    return std::vector<int>({ p[0] });\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void cpp11init2() {
        check("x<string> handlers{\n"
              "  { \"mode2\", []() { return 2; } },\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void cpp11init3() {
        check("struct A { void operator()(int); };\n"
              "void f() {\n"
              "A{}(0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("template<class> struct A { void operator()(int); };\n"
              "void f() {\n"
              "A<int>{}(0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void block() {
        check("void f() {\n"
              "    ({ do_something(); 0; });\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "out:\n"
              "    ({ do_something(); 0; });\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void mapindex() {
        check("void f() {\n"
              "  map[{\"1\",\"2\"}]=0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void commaoperator1() {
        check("void foo(int,const char*,int);\n" // #8827
              "void f(int value) {\n"
              "    foo(42,\"test\",42),(value&42);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:22]: (warning) Found suspicious operator ',', result is not used. [constStatement]\n", errout_str());

        check("int f() {\n" // #11257
              "    int y;\n"
              "    y = (3, 4);\n"
              "    return y;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (warning) Found suspicious operator ',', result is not used. [constStatement]\n", errout_str());
    }

    void commaoperator2() {
        check("void f() {\n"
              "    for(unsigned int a=0, b; a<10; a++ ) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g();\n" // #10952
              "bool f() {\n"
              "    return (void)g(), false;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a, int b, int c, int d) {\n"
              "    Eigen::Vector4d V;\n"
              "    V << a, b, c, d;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { Eigen::Vector4d V; };\n"
              "struct T { int a, int b, int c, int d; };\n"
              "void f(S& s, const T& t) {\n"
              "    s.V << t.a, t.b, t.c, t.d;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { Eigen::Vector4d V[2]; };\n"
              "void f(int a, int b, int c, int d) {\n"
              "    S s[1];\n"
              "    s[0].V[1] << a, b, c, d;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    a.b[4][3].c()->d << x , y, z;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct V {\n"
              "    Eigen::Vector3d& operator[](int i) { return v[i]; }\n"
              "    void f(int a, int b, int c);\n"
              "    Eigen::Vector3d v[1];\n"
              "};\n"
              "void V::f(int a, int b, int c) {\n"
              "    (*this)[0] << a, b, c;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #11359
              "    struct S {\n"
              "        S(int x, int y) {}\n"
              "    } s(1, 2);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // #8451
    void redundantstmts() {
        check("void f1(int x) {\n"
              "    1;\n"
              "    (1);\n"
              "    (char)1;\n"
              "    ((char)1);\n"
              "    !x;\n"
              "    (!x);\n"
              "    (unsigned int)!x;\n"
              "    ~x;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Redundant code: Found a statement that begins with numeric constant. [constStatement]\n"
                      "[test.cpp:3:6]: (warning) Redundant code: Found a statement that begins with numeric constant. [constStatement]\n"
                      "[test.cpp:4:5]: (warning) Redundant code: Found a statement that begins with numeric constant. [constStatement]\n"
                      "[test.cpp:5:6]: (warning) Redundant code: Found a statement that begins with numeric constant. [constStatement]\n"
                      "[test.cpp:6:5]: (warning, inconclusive) Found suspicious operator '!', result is not used. [constStatement]\n"
                      "[test.cpp:7:6]: (warning, inconclusive) Found suspicious operator '!', result is not used. [constStatement]\n"
                      "[test.cpp:8:5]: (warning) Redundant code: Found unused cast of expression '!x'. [constStatement]\n"
                      "[test.cpp:9:5]: (warning, inconclusive) Found suspicious operator '~', result is not used. [constStatement]\n",
                      errout_str());

        check("void f1(int x) { x; }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:1:18]: (warning) Unused variable value 'x' [constStatement]\n", errout_str());

        check("void f() { if (Type t; g(t)) {} }"); // #9776
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) { static_cast<unsigned>(x); }");
        ASSERT_EQUALS("[test.cpp:1:38]: (warning) Redundant code: Found unused cast of expression 'x'. [constStatement]\n", errout_str());

        check("void f(int x, int* p) {\n"
              "    static_cast<void>(x);\n"
              "    (void)x;\n"
              "    static_cast<void*>(p);\n"
              "    (void*)p;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() { false; }"); // #10856
        ASSERT_EQUALS("[test.cpp:1:12]: (warning) Redundant code: Found a statement that begins with bool constant. [constStatement]\n", errout_str());

        check("void f(int i) {\n"
              "    (float)(char)i;\n"
              "    static_cast<float>((char)i);\n"
              "    (char)static_cast<float>(i);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Redundant code: Found unused cast of expression 'i'. [constStatement]\n"
                      "[test.cpp:3:23]: (warning) Redundant code: Found unused cast of expression 'i'. [constStatement]\n"
                      "[test.cpp:4:5]: (warning) Redundant code: Found unused cast of expression 'i'. [constStatement]\n",
                      errout_str());

        check("namespace M {\n"
              "    namespace N { typedef char T; }\n"
              "}\n"
              "void f(int i) {\n"
              "    (M::N::T)i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:5]: (warning) Redundant code: Found unused cast of expression 'i'. [constStatement]\n", errout_str());

        check("void f(int (g)(int a, int b)) {\n" // #10873
              "    int p = 0, q = 1;\n"
              "    (g)(p, q);\n"
              "}\n"
              "void f() {\n"
              "  char buf[10];\n"
              "  (sprintf)(buf, \"%d\", 42);\n"
              "  (printf)(\"abc\");\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S; struct T; struct U;\n"
              "void f() {\n"
              "    T t;\n"
              "    (S)(U)t;\n"
              "    (S)static_cast<U>(t);\n"
              "    static_cast<S>((U)t);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) { b ? true : false; }\n"); // #10865
        ASSERT_EQUALS("[test.cpp:1:20]: (warning) Redundant code: Found unused result of ternary operator. [constStatement]\n", errout_str());

        check("struct S { void (*f)() = nullptr; };\n" // #10877
              "void g(S* s) {\n"
              "    (s->f == nullptr) ? nullptr : (s->f(), nullptr);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n"
              "    g() ? true : false;\n"
              "    true ? g() : false;\n"
              "    false ? true : g();\n"
              "    g(b ? true : false, 1);\n"
              "    C c{ b ? true : false, 1 };\n"
              "    b = (b ? true : false);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i) {\n"
              "    for (i; ;) {}\n"
              "    for ((long)i; ;) {}\n"
              "    for (1; ;) {}\n"
              "    for (true; ;) {}\n"
              "    for ('a'; ;) {}\n"
              "    for (L'b'; ;) {}\n"
              "    for (\"x\"; ;) {}\n"
              "    for (L\"y\"; ;) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Unused variable value 'i' [constStatement]\n"
                      "[test.cpp:3:10]: (warning) Redundant code: Found unused cast of expression 'i'. [constStatement]\n"
                      "[test.cpp:4:10]: (warning) Redundant code: Found a statement that begins with numeric constant. [constStatement]\n"
                      "[test.cpp:5:10]: (warning) Redundant code: Found a statement that begins with bool constant. [constStatement]\n"
                      "[test.cpp:6:10]: (warning) Redundant code: Found a statement that begins with character constant. [constStatement]\n"
                      "[test.cpp:7:10]: (warning) Redundant code: Found a statement that begins with character constant. [constStatement]\n"
                      "[test.cpp:8:10]: (warning) Redundant code: Found a statement that begins with string constant. [constStatement]\n"
                      "[test.cpp:9:10]: (warning) Redundant code: Found a statement that begins with string constant. [constStatement]\n",
                      errout_str());

        check("struct S { bool b{}; };\n"
              "struct T {\n"
              "    S s[2];\n"
              "    void g();\n"
              "};\n"
              "void f(const S& r, const S* p) {\n"
              "    r.b;\n"
              "    p->b;\n"
              "    S s;\n"
              "    (s.b);\n"
              "    T t, u[2];\n"
              "    t.s[1].b;\n"
              "    t.g();\n"
              "    u[0].g();\n"
              "    u[1].s[0].b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:6]: (warning) Redundant code: Found unused member access. [constStatement]\n"
                      "[test.cpp:8:6]: (warning) Redundant code: Found unused member access. [constStatement]\n"
                      "[test.cpp:10:7]: (warning) Redundant code: Found unused member access. [constStatement]\n"
                      "[test.cpp:12:11]: (warning) Redundant code: Found unused member access. [constStatement]\n"
                      "[test.cpp:15:14]: (warning) Redundant code: Found unused member access. [constStatement]\n",
                      errout_str());

        check("struct S { int a[2]{}; };\n"
              "struct T { S s; };\n"
              "void f() {\n"
              "    int i[2];\n"
              "    i[0] = 0;\n"
              "    i[0];\n" // <--
              "    S s[1];\n"
              "    s[0].a[1];\n" // <--
              "    T t;\n"
              "    t.s.a[1];\n" // <--
              "    int j[2][2][1] = {};\n"
              "    j[0][0][0];\n" // <--
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:6]: (warning) Redundant code: Found unused array access. [constStatement]\n"
                      "[test.cpp:8:11]: (warning) Redundant code: Found unused array access. [constStatement]\n"
                      "[test.cpp:10:10]: (warning) Redundant code: Found unused array access. [constStatement]\n"
                      "[test.cpp:12:6]: (warning) Redundant code: Found unused array access. [constStatement]\n",
                      errout_str());

        check("void g(std::map<std::string, std::string>& map) {\n"
              "    int j[2]{};\n"
              "    int k[2] = {};\n"
              "    int l[]{ 1, 2 };\n"
              "    int m[] = { 1, 2 };\n"
              "    h(0, j[0], 1);\n"
              "    C c{ 0, j[0], 1 };\n"
              "    c[0];\n"
              "    int j[2][2][2] = {};\n"
              "    j[h()][0][0];\n"
              "    j[0][h()][0];\n"
              "    j[0][0][h()];\n"
              "    std::map<std::string, int> M;\n"
              "    M[\"abc\"];\n"
              "    map[\"abc\"];\n" // #10928
              "    std::auto_ptr<Int> app[4];" // #10919
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { void* p; };\n" // #10875
              "void f(S s) {\n"
              "    delete (int*)s.p;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n" // #10874
              "    T* p;\n"
              "};\n"
              "void f(T* t) {\n"
              "    for (decltype(t->p) (c) = t->p; ;) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i, std::vector<int*> v);\n" // #10880
              "void g() {\n"
              "    f(1, { static_cast<int*>(nullptr) });\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n" // #10882
              "enum E {};\n"
              "void f(const S* s) {\n"
              "    E e = (E)!s->i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int* p) {\n" // #10932
              "    int& r(*p[0]);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n" // #10917
              "bool f(S s) {\n"
              "    return [](int i) { return i > 0; }(s.i);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("extern int (*p);\n" // #10936
              "void f() {\n"
              "    for (int i = 0; ;) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("class T {};\n" // #10849
              "void f() {\n"
              "    auto g = [](const T* t) -> int {\n"
              "        const T* u{}, * v{};\n"
              "        return 0;\n"
              "    };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("namespace N {\n" // #10876
              "    template <class R, class S, void(*T)(R&, float, S)>\n"
              "    inline void f() {}\n"
              "    template<class T>\n"
              "    void g(T& c) {\n"
              "        for (typename T::iterator v = c.begin(); v != c.end(); ++v) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string a, std::string b) {\n" // #7529
              "    const std::string s = \" x \" + a;\n"
              "    +\" y = \" + b;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:14]: (warning, inconclusive) Found suspicious operator '+', result is not used. [constStatement]\n", errout_str());

        check("void f() {\n"
              "    *new int;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Found suspicious operator '*', result is not used. [constStatement]\n", errout_str());

        check("void f(int x, int y) {\n" // #12525
              "    x * y;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:7]: (warning, inconclusive) Found suspicious operator '*', result is not used. [constStatement]\n", errout_str());

        check("void f() {\n" // #5475
              "    std::string(\"a\") + \"a\";\n"
              "}\n"
              "void f(std::string& a) {\n"
              "    a.erase(3) + \"suf\";\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:22]: (warning, inconclusive) Found suspicious operator '+', result is not used. [constStatement]\n"
                      "[test.cpp:5:16]: (warning, inconclusive) Found suspicious operator '+', result is not used. [constStatement]\n",
                      errout_str());

        check("void f(XMLElement& parent) {\n" // #11234
              "    auto** elem = &parent.firstChild;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #11301
              "    NULL;\n"
              "    nullptr;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Redundant code: Found a statement that begins with NULL constant. [constStatement]\n"
                      "[test.cpp:3:5]: (warning) Redundant code: Found a statement that begins with NULL constant. [constStatement]\n",
                      errout_str());

        check("struct S { int i; };\n" // #6504
              "void f(S* s) {\n"
              "    (*s).i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:9]: (warning) Redundant code: Found unused member access. [constStatement]\n", errout_str());

        check("int a[2];\n" // #11370
              "void f() {\n"
              "    auto g = [](decltype(a[0]) i) {};\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("enum E { E0 };\n"
              "void f() {\n"
              "    E0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Redundant code: Found a statement that begins with enumerator constant. [constStatement]\n",
                      errout_str());

        check("void f(int* a) {\n" // #12534
              "    a[a[3]];\n"
              "    a[a[g()]];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:6]: (warning) Redundant code: Found unused array access. [constStatement]\n",
                      errout_str());

        check("void f() {\n" // #13153
              "    []() {} ();\n"
              "    []() {};\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Redundant code: Found unused lambda. [constStatement]\n",
                      errout_str());

        check("int main() {\n" // #13177
              "    sizeof(int);\n"
              "    alignof(long double*);\n"
              "    noexcept(int());\n"
              "    typeid(int);\n"
              "    return(0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning) Redundant code: Found unused 'sizeof' expression. [constStatement]\n"
                      "[test.cpp:3:12]: (warning) Redundant code: Found unused 'alignof' expression. [constStatement]\n"
                      "[test.cpp:4:13]: (warning) Redundant code: Found unused 'noexcept' expression. [constStatement]\n"
                      "[test.cpp:5:11]: (warning) Redundant code: Found unused 'typeid' expression. [constStatement]\n",
                      errout_str());

        check("void f() {\n" // #14044
              "    g<sizeof(wchar_t)>();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S;\n" // #14664
              "S* a[1];\n"
              "void f() {\n"
              "    a[0];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:6]: (warning) Redundant code: Found unused array access. [constStatement]\n",
                      errout_str());
    }

    void vardecl() {
        // #8984
        check("void f() { a::b *c = d(); }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() { std::vector<b> *c; }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() { a::b &c = d(); }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() { std::vector<b> &c; }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() { a::b &&c = d(); }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() { std::vector<b> &&c; }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() { char * const * a, * const * b; }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() { char * const * a = 0, * volatile restrict * b; }", dinit(CheckOptions, $.inconclusive = true, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("void f() { char * const * a = 0, * volatile const * b; }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void archive() {
        check("void f(Archive &ar) {\n"
              "  ar & x;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(int ar) {\n"
              "  ar & x;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:6]: (warning, inconclusive) Found suspicious operator '&', result is not used. [constStatement]\n", errout_str());
    }

    void ast() {
        check("struct c { void a() const { for (int x=0; x;); } };", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void oror() {
        check("void foo() {\n"
              "    params_given (params, \"overrides\") || (overrides = \"1\");\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(std::ifstream& file) {\n" // #10930
              "    int a{}, b{};\n"
              "    (file >> a) || (file >> b);\n"
              "    (file >> a) && (file >> b);\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void functioncall() {
        check("void g();\n" // #13794
              "void f() {\n"
              "    g;\n"
              "    exit;\n"
              "    std::free;\n"
              "    ::std::terminate;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Redundant code: Found unused function. [constStatement]\n"
                      "[test.cpp:4:5]: (warning) Redundant code: Found unused function. [constStatement]\n"
                      "[test.cpp:5:8]: (warning) Redundant code: Found unused function. [constStatement]\n"
                      "[test.cpp:6:10]: (warning) Redundant code: Found unused function. [constStatement]\n",
                      errout_str());
    }
};

REGISTER_TEST(TestIncompleteStatement)
