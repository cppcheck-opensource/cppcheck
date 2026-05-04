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

#include "checkclass.h"
#include "errortypes.h"
#include "helpers.h"
#include "platform.h"
#include "settings.h"
#include "fixture.h"

#include <cstddef>
#include <string>

class TestUnusedPrivateFunction : public TestFixture {
public:
    TestUnusedPrivateFunction() : TestFixture("TestUnusedPrivateFunction") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::style).build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);
        TEST_CASE(test4);
        TEST_CASE(test5);
        TEST_CASE(test6); // ticket #2602
        TEST_CASE(test7); // ticket #9282

        // [ 2236547 ] False positive --style unused function, called via pointer
        TEST_CASE(func_pointer1);
        TEST_CASE(func_pointer2);
        TEST_CASE(func_pointer3);
        TEST_CASE(func_pointer4); // ticket #2807
        TEST_CASE(func_pointer5); // ticket #2233
        TEST_CASE(func_pointer6); // ticket #4787
        TEST_CASE(func_pointer7); // ticket #10516

        TEST_CASE(ctor);
        TEST_CASE(ctor2);
        TEST_CASE(ctor3);

        TEST_CASE(classInClass);
        TEST_CASE(sameFunctionNames);

        TEST_CASE(derivedClass);   // skip warning for derived classes. It might be a virtual function.

        TEST_CASE(friendClass);

        TEST_CASE(borland1);     // skip FP when using __property
        TEST_CASE(borland2);     // skip FP when using __published

        // No false positives when there are "unused" templates that are removed in the simplified token list
        TEST_CASE(template1);

        // #2407 - FP when called from operator()
        TEST_CASE(fp_operator);
        TEST_CASE(testDoesNotIdentifyMethodAsFirstFunctionArgument); // #2480
        TEST_CASE(testDoesNotIdentifyMethodAsMiddleFunctionArgument);
        TEST_CASE(testDoesNotIdentifyMethodAsLastFunctionArgument);

        TEST_CASE(multiFile);
        TEST_CASE(unknownBaseTemplate); // ticket #2580
        TEST_CASE(hierarchy_loop); // ticket 5590

        TEST_CASE(staticVariable); //ticket #5566

        TEST_CASE(templateSimplification); //ticket #6183
        TEST_CASE(maybeUnused);
        TEST_CASE(attributeUnused); // #14129
        TEST_CASE(trailingReturn);
    }

    struct CheckOptions
    {
        Platform::Type platform = Platform::Type::Native;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings settings1 = settingsBuilder(settings).platform(options.platform).build();

        SimpleTokenizer2 tokenizer(settings1, *this, code, "test.cpp");

        // Tokenize..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check for unused private functions..
        CheckClass checkClass(&tokenizer, &settings1, this);
        checkClass.privateFunctions();
    }

    void test1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    unsigned int f();\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{ }\n"
              "\n"
              "unsigned int Fred::f()\n"
              "{ }");

        ASSERT_EQUALS("[test.cpp:4:18] -> [test.cpp:12:20]: (style) Unused private function: 'Fred::f' [unusedPrivateFunction]\n", errout_str());

        check("#line 1 \"p.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    unsigned int f();\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "\n"
              "#line 1 \"p.cpp\"\n"
              "Fred::Fred()\n"
              "{ }\n"
              "\n"
              "unsigned int Fred::f()\n"
              "{ }");

        ASSERT_EQUALS("[p.h:4:18] -> [p.cpp:4:20]: (style) Unused private function: 'Fred::f' [unusedPrivateFunction]\n", errout_str());

        check("#line 1 \"p.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "void f();\n"
              "};\n"
              "\n"
              "\n"
              "#line 1 \"p.cpp\"\n"
              "\n"
              "void Fred::f()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("[p.h:4:6] -> [p.cpp:2:12]: (style) Unused private function: 'Fred::f' [unusedPrivateFunction]\n", errout_str());

        // Don't warn about include files which implementation we don't see
        check("#line 1 \"p.h\"\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "void f();\n"
              "void g() {}\n"
              "};\n"
              "\n"
              "#line 1 \"p.cpp\"\n"
              "\n"
              "int main()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    void test2() {
        check("class A {\n"
              "public:\n"
              "    A();\n"
              "\n"
              "    void a() const\n"
              "    { b(); }\n"
              "private:\n"
              "    void b( ) const\n"
              "    { }\n"
              "};\n"
              "\n"
              "A::A()\n"
              "{ }");
        ASSERT_EQUALS("", errout_str());
    }


    void test3() {
        check("class A {\n"
              "public:\n"
              "    A() { }\n"
              "    ~A();\n"
              "private:\n"
              "    void B() { }\n"
              "};\n"
              "\n"
              "A::~A()\n"
              "{ B(); }");
        ASSERT_EQUALS("", errout_str());
    }


    void test4() {
        check("class A {\n"
              "public:\n"
              "    A();\n"
              "private:\n"
              "    bool _owner;\n"
              "    void b() { }\n"
              "};\n"
              "\n"
              "A::A() : _owner(false)\n"
              "{ b(); }");
        ASSERT_EQUALS("", errout_str());
    }


    void test5() {
        check("class A {\n"
              "private:\n"
              "    A() : lock(new Lock())\n"
              "    { }\n"
              "    Lock *lock;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void test6() { // ticket #2602 segmentation fault
        check("class A {\n"
              "    A& operator=(const A&);\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void test7() { // ticket #9282
        check("class C {\n"
              "    double f1() const noexcept, f2(double) const noexcept;\n"
              "    void f3() const noexcept;\n"
              "};\n"
              "double C::f1() const noexcept { f3(); }\n"
              "void C::f3() const noexcept {}\n");
        ASSERT_EQUALS("", errout_str());
    }





    void func_pointer1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    typedef void (*testfp)();\n"
              "\n"
              "    testfp get()\n"
              "    {\n"
              "        return test;\n"
              "    }\n"
              "\n"
              "    static void test()\n"
              "    { }\n"
              "\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{}");

        ASSERT_EQUALS("[test.cpp:6:12] -> [test.cpp:6:12]: (style) Unused private function: 'Fred::get' [unusedPrivateFunction]\n", errout_str());
    }



    void func_pointer2() {
        check("class UnusedPrivateFunctionMemberPointer\n"
              "{\n"
              "public:\n"
              "    UnusedPrivateFunctionMemberPointer()\n"
              "    :   mObserver(this, &UnusedPrivateFunctionMemberPointer::callback)\n"
              "    {}\n"
              "\n"
              "private:\n"
              "    void callback(const& unsigned) const {}\n"
              "\n"
              "    Observer<UnusedPrivateFunctionMemberPointer, unsigned> mObserver;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }


    void func_pointer3() {
        check("class c1\n"
              "{\n"
              "public:\n"
              "    c1()\n"
              "    { sigc::mem_fun(this, &c1::f1); }\n"
              "\n"
              "private:\n"
              "    void f1() const {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }


    void func_pointer4() { // ticket #2807
        check("class myclass {\n"
              "public:\n"
              "    myclass();\n"
              "private:\n"
              "    static void f();\n"
              "    void (*fptr)();\n"
              "};\n"
              "myclass::myclass() { fptr = &f; }\n"
              "void myclass::f() {}");
        ASSERT_EQUALS("", errout_str());
    }


    void func_pointer5() {
        check("class A {\n"
              "public:\n"
              "    A() { f = A::func; }\n"
              "    void (*f)();\n"
              "private:\n"
              "    static void func() { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }


    void func_pointer6() { // #4787
        check("class Test {\n"
              "private:\n"
              "    static void a(const char* p) { }\n"
              "public:\n"
              "    void test(void* f = a) {\n"
              "        f(\"test\");\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void func_pointer7() { // #10516
        check("class C {\n"
              "    static void f() {}\n"
              "    static constexpr void(*p)() = f;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n"
              "    static void f() {}\n"
              "    static constexpr void(*p)() = &f;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n"
              "    static void f() {}\n"
              "    static constexpr void(*p)() = C::f;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n"
              "    static void f() {}\n"
              "    static constexpr void(*p)() = &C::f;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }


    void ctor() {
        check("class PrivateCtor\n"
              "{\n"
              "private:\n"
              "    PrivateCtor(int threadNum) :\n"
              "        numOfThreads(threadNum)\n"
              "    {\n"
              "    }\n"
              "\n"
              "    int numOfThreads;\n"
              "};");

        ASSERT_EQUALS("", errout_str());
    }

    void ctor2() {
        check("struct State {\n"
              "  State(double const totalWeighting= TotalWeighting()) :\n"
              "    totalWeighting_(totalWeighting) {}\n"
              "private:\n"
              "  double TotalWeighting() { return 123.0; }\n"  // called from constructor
              "public:\n"
              "  double totalWeighting_;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void ctor3() {
        check("class C {\n"
              "    C() = default;\n"
              "    void f() const { (void)this; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:3:10]: (style) Unused private function: 'C::f' [unusedPrivateFunction]\n", errout_str());
    }


    void classInClass() {
        check("class A\n"
              "{\n"
              "public:\n"
              "\n"
              "    class B\n"
              "    {\n"
              "    private:\n"
              "    };\n"
              "private:\n"
              "    static void f()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:10:17] -> [test.cpp:10:17]: (style) Unused private function: 'A::f' [unusedPrivateFunction]\n", errout_str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    A()\n"
              "    { }\n"
              "\n"
              "private:\n"
              "    void f()\n"
              "    { }\n"
              "\n"
              "    class B\n"
              "    {\n"
              "    public:\n"
              "        B(A *a)\n"
              "        { a->f(); }\n"
              "    };\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A {\n"  // #6968 - outer definition
              "public:\n"
              "  class B;\n"
              "private:\n"
              "  void f() {}\n"
              "};\n"
              "class A::B {"
              "  B() { A a; a.f(); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n" // #13790
              "    class I { I() = default; };\n"
              "    void f() const { (void)this; }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:3:10]: (style) Unused private function: 'C::f' [unusedPrivateFunction]\n", errout_str());
    }


    void sameFunctionNames() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    void a()\n"
              "    {\n"
              "        f(1);\n"
              "    }\n"
              "\n"
              "private:\n"
              "    void f() { }\n"
              "    void f(int) { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void derivedClass() {
        // skip warning in derived classes in case the base class is invisible
        check("class derived : public base\n"
              "{\n"
              "public:\n"
              "    derived() : base() { }\n"
              "private:\n"
              "    void f();\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class base {\n"
              "public:\n"
              "    virtual void foo();\n"
              "    void bar();\n"
              "};\n"
              "class derived : public base {\n"
              "private:\n"
              "    void foo() {}\n" // Skip for overrides of virtual functions of base
              "    void bar() {}\n" // Don't skip if no function is overridden
              "};");
        ASSERT_EQUALS("[test.cpp:9:10] -> [test.cpp:9:10]: (style) Unused private function: 'derived::bar' [unusedPrivateFunction]\n", errout_str());

        check("class Base {\n"
              "private:\n"
              "    virtual void func() = 0;\n"
              "public:\n"
              "    void public_func() {\n"
              "        func();\n"
              "    };\n"
              "};\n"
              "\n"
              "class Derived1: public Base {\n"
              "private:\n"
              "    void func() {}\n"
              "};\n"
              "class Derived2: public Derived1 {\n"
              "private:\n"
              "    void func() {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Base {\n"
              "public:\n"
              "    void dostuff() {\n"
              "      f();\n"
              "    }\n"
              "\n"
              "private:\n"
              "    virtual Base* f() = 0;\n"
              "};\n"
              "\n"
              "class Derived : public Base {\n"
              "private:\n"
              "    Derived* f() {\n"
              "      return 0;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void friendClass() {
        // ticket #2459 - friend class
        check("class Foo {\n"
              "private:\n"
              "    friend Bar;\n" // Unknown friend class
              "    void f() { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct Bar {\n"
              "    void g() { f(); }\n" // Friend class seen, but f not seen
              "};\n"
              "class Foo {\n"
              "private:\n"
              "    friend Bar;\n"
              "    void f();\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct Bar {\n"
              "    void g() { f(); }\n" // Friend class seen, but f() used in it
              "};\n"
              "class Foo {\n"
              "private:\n"
              "    friend Bar;\n"
              "    void f() { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Bar {\n" // Friend class seen, f() not used in it
              "};\n"
              "class Foo {\n"
              "    friend Bar;\n"
              "    void f() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:10] -> [test.cpp:5:10]: (style) Unused private function: 'Foo::f' [unusedPrivateFunction]\n", errout_str());

        check("struct F;\n" // #10265
              "struct S {\n"
              "    int i{};\n"
              "    friend struct F;\n"
              "private:\n"
              "    int f() const { return i; }\n"
              "};\n"
              "struct F {\n"
              "    bool operator()(const S& lhs, const S& rhs) const {\n"
              "        return lhs.f() < rhs.f();\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void borland1() {
        // ticket #2034 - Borland C++ __property
        check("class Foo {\n"
              "private:\n"
              "    int getx() {\n"
              "        return 123;\n"
              "    }\n"
              "public:\n"
              "    Foo() { }\n"
              "    __property int x = {read=getx}\n"
              "};", dinit(CheckOptions, $.platform = Platform::Type::Win32A));
        ASSERT_EQUALS("", errout_str());
    }

    void borland2() {
        // ticket #3661 - Borland C++ __published
        check("class Foo {\n"
              "__published:\n"
              "    int getx() {\n"
              "        return 123;\n"
              "    }\n"
              "public:\n"
              "    Foo() { }\n"
              "};", dinit(CheckOptions, $.platform = Platform::Type::Win32A));
        ASSERT_EQUALS("", errout_str());
    }

    void template1() {
        // ticket #2067 - Template methods do not "use" private ones
        check("class A {\n"
              "public:\n"
              "    template <class T>\n"
              "    T getVal() const;\n"
              "\n"
              "private:\n"
              "    int internalGetVal() const { return 8000; }\n"
              "};\n"
              "\n"
              "template <class T>\n"
              "T A::getVal() const {\n"
              "    return internalGetVal();\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void fp_operator() {
        // #2407 - FP when function is called from operator()
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void operator()(int x) {\n"
              "        startListening();\n"
              "    }\n"
              "\n"
              "private:\n"
              "    void startListening() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void operator()(int x) {\n"
              "    }\n"
              "\n"
              "private:\n"
              "    void startListening() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:8:10] -> [test.cpp:8:10]: (style) Unused private function: 'Fred::startListening' [unusedPrivateFunction]\n", errout_str());

        // #5059
        check("class Fred {\n"
              "    void* operator new(size_t obj_size, size_t buf_size) {}\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (style) Unused private function: 'Fred::operatornew'\n", "", errout_str()); // No message for operators - we currently cannot check their usage

        check("class Fred {\n"
              "    void* operator new(size_t obj_size, size_t buf_size) {}\n"
              "public:\n"
              "    void* foo() { return new(size) Fred(); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void testDoesNotIdentifyMethodAsFirstFunctionArgument() {
        check("void callback(void (*func)(int), int arg)"
              "{"
              "    (*func)(arg);"
              "}"
              "class MountOperation"
              "{"
              "    static void Completed(int i);"
              "public:"
              "    MountOperation(int i);"
              "};"
              "void MountOperation::Completed(int i)"
              "{"
              "    std::cerr << i << std::endl;"
              "}"
              "MountOperation::MountOperation(int i)"
              "{"
              "    callback(MountOperation::Completed, i);"
              "}"
              "int main(void)"
              "{"
              "    MountOperation aExample(10);"
              "}"
              );
        ASSERT_EQUALS("", errout_str());
    }

    void testDoesNotIdentifyMethodAsMiddleFunctionArgument() {
        check("void callback(char, void (*func)(int), int arg)"
              "{"
              "    (*func)(arg);"
              "}"
              "class MountOperation"
              "{"
              "    static void Completed(int i);"
              "public:"
              "    MountOperation(int i);"
              "};"
              "void MountOperation::Completed(int i)"
              "{"
              "    std::cerr << i << std::endl;"
              "}"
              "MountOperation::MountOperation(int i)"
              "{"
              "    callback('a', MountOperation::Completed, i);"
              "}"
              "int main(void)"
              "{"
              "    MountOperation aExample(10);"
              "}"
              );
        ASSERT_EQUALS("", errout_str());
    }

    void testDoesNotIdentifyMethodAsLastFunctionArgument() {
        check("void callback(int arg, void (*func)(int))"
              "{"
              "    (*func)(arg);"
              "}"
              "class MountOperation"
              "{"
              "    static void Completed(int i);"
              "public:"
              "    MountOperation(int i);"
              "};"
              "void MountOperation::Completed(int i)"
              "{"
              "    std::cerr << i << std::endl;"
              "}"
              "MountOperation::MountOperation(int i)"
              "{"
              "    callback(i, MountOperation::Completed);"
              "}"
              "int main(void)"
              "{"
              "    MountOperation aExample(10);"
              "}"
              );
        ASSERT_EQUALS("", errout_str());
    }

    void multiFile() { // ticket #2567
        check("#file \"test.h\"\n"
              "struct Fred\n"
              "{\n"
              "    Fred()\n"
              "    {\n"
              "        Init();\n"
              "    }\n"
              "private:\n"
              "    void Init();\n"
              "};\n"
              "#endfile\n" // TODO
              "void Fred::Init()\n"
              "{\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void unknownBaseTemplate() { // ticket #2580
        check("class Bla : public Base2<Base> {\n"
              "public:\n"
              "    Bla() {}\n"
              "private:\n"
              "    virtual void F() const;\n"
              "};\n"
              "void Bla::F() const { }");

        ASSERT_EQUALS("", errout_str());
    }

    void hierarchy_loop() {
        check("class InfiniteB : InfiniteA {\n"
              "    class D {\n"
              "    };\n"
              "};\n"
              "namespace N {\n"
              "    class InfiniteA : InfiniteB {\n"
              "    };\n"
              "}\n"
              "class InfiniteA : InfiniteB {\n"
              "    void foo();\n"
              "};\n"
              "void InfiniteA::foo() {\n"
              "    C a;\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void staticVariable() {
        check("class Foo {\n"
              "    static int i;\n"
              "    static int F() const { return 1; }\n"
              "};\n"
              "int Foo::i = Foo::F();");
        ASSERT_EQUALS("", errout_str());

        check("class Foo {\n"
              "    static int i;\n"
              "    int F() const { return 1; }\n"
              "};\n"
              "Foo f;\n"
              "int Foo::i = f.F();");
        ASSERT_EQUALS("", errout_str());

        check("class Foo {\n"
              "    static int i;\n"
              "    static int F() const { return 1; }\n"
              "};\n"
              "int Foo::i = sth();"
              "int i = F();");
        ASSERT_EQUALS("[test.cpp:3:16] -> [test.cpp:3:16]: (style) Unused private function: 'Foo::F' [unusedPrivateFunction]\n", errout_str());
    }

    void templateSimplification() { //ticket #6183
        check("class CTest {\n"
              "public:\n"
              "    template <typename T>\n"
              "    static void Greeting(T val) {\n"
              "        std::cout << val << std::endl;\n"
              "    }\n"
              "private:\n"
              "    static void SayHello() {\n"
              "        std::cout << \"Hello World!\" << std::endl;\n"
              "    }\n"
              "};\n"
              "template<>\n"
              "void CTest::Greeting(bool) {\n"
              "    CTest::SayHello();\n"
              "}\n"
              "int main() {\n"
              "    CTest::Greeting<bool>(true);\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void maybeUnused() {
        check("class C {\n"
              "    [[maybe_unused]] int f() { return 42; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n"
              "    [[maybe_unused]] static int f();\n"
              "};\n"
              "int C::f() { return 42; }\n");
        ASSERT_EQUALS("", errout_str());
    }

    void attributeUnused() {
        check("class C {\n"
              "    __attribute__((unused)) int f() { return 42; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n"
              "    __attribute__((unused)) int f();\n"
              "};\n"
              "int C::f() { return 42; }\n");
        ASSERT_EQUALS("", errout_str());
    }

    void trailingReturn() {
        check("struct B { virtual void f(); };\n"
              "struct D : B {\n"
              "    auto f() -> void override;\n"
              "private:\n"
              "    auto g() -> void;\n"
              "};\n"
              "auto D::f() -> void { g(); }\n"
              "auto D::g() -> void {}\n");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestUnusedPrivateFunction)
