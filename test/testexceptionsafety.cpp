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

#include "checkexceptionsafety.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>

class TestExceptionSafety : public TestFixture {
public:
    TestExceptionSafety() : TestFixture("TestExceptionSafety") {}

private:
    /*const*/ Settings settings;

    void run() override {
        settings.severity.fill();

        mNewTemplate = true;
        TEST_CASE(destructors);
        TEST_CASE(deallocThrow1);
        TEST_CASE(deallocThrow2);
        TEST_CASE(deallocThrow3);
        TEST_CASE(rethrowCopy1);
        TEST_CASE(rethrowCopy2);
        TEST_CASE(rethrowCopy3);
        TEST_CASE(rethrowCopy4);
        TEST_CASE(rethrowCopy5);
        TEST_CASE(catchExceptionByValue);
        TEST_CASE(noexceptThrow);
        TEST_CASE(nothrowThrow);
        TEST_CASE(unhandledExceptionSpecification1); // #4800
        TEST_CASE(unhandledExceptionSpecification2);
        TEST_CASE(unhandledExceptionSpecification3);
        TEST_CASE(nothrowAttributeThrow);
        TEST_CASE(nothrowAttributeThrow2); // #5703
        TEST_CASE(nothrowDeclspecThrow);
        TEST_CASE(rethrowNoCurrentException1);
        TEST_CASE(rethrowNoCurrentException2);
        TEST_CASE(rethrowNoCurrentException3);
        TEST_CASE(noFunctionCall);
        TEST_CASE(entryPoint);
    }

    struct CheckOptions
    {
        bool inconclusive = false;
        const Settings *s = nullptr;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size],  const CheckOptions& options = make_default_obj()) {
        const Settings settings1 = settingsBuilder(options.s ? *options.s : settings).certainty(Certainty::inconclusive, options.inconclusive).build();

        // Tokenize..
        SimpleTokenizer tokenizer(settings1, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check char variable usage..
        runChecks<CheckExceptionSafety>(tokenizer, this);
    }

    void destructors() {
        check("class x {\n"
              "    ~x() {\n"
              "        throw e;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3:9]: (warning) Class x is not safe, destructor throws exception [exceptThrowInDestructor]\n"
                      "[test.cpp:3:9]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n", errout_str());

        check("class x {\n"
              "    ~x();\n"
              "};\n"
              "x::~x() {\n"
              "    throw e;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5]: (warning) Class x is not safe, destructor throws exception [exceptThrowInDestructor]\n"
                      "[test.cpp:5:5]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n", errout_str());

        // #3858 - throwing exception in try block in destructor.
        check("class x {\n"
              "    ~x() {\n"
              "        try {\n"
              "            throw e;\n"
              "        } catch (...) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class x {\n"
              "    ~x() {\n"
              "        if(!std::uncaught_exception()) {\n"
              "            throw e;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n", errout_str());

        // #11031 should not warn when noexcept false
        check("class A {\n"
              "public:\n"
              "    ~A() noexcept(false) {\n"
              "        throw 30;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void deallocThrow1() {
        check("int * p;\n"
              "void f(int x) {\n"
              "    delete p;\n"
              "    if (x)\n"
              "        throw 123;\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9]: (warning) Exception thrown in invalid state, 'p' points at deallocated memory. [exceptDeallocThrow]\n", errout_str());

        check("void f() {\n"
              "    static int* p = foo;\n"
              "    delete p;\n"
              "    if (foo)\n"
              "        throw 1;\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9]: (warning) Exception thrown in invalid state, 'p' points at deallocated memory. [exceptDeallocThrow]\n", errout_str());
    }

    void deallocThrow2() {
        check("void f() {\n"
              "    int* p = 0;\n"
              "    delete p;\n"
              "    if (foo)\n"
              "        throw 1;\n"
              "    p = new int;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    static int* p = 0;\n"
              "    delete p;\n"
              "    reset(p);\n"
              "    throw 1;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void deallocThrow3() {
        check("void f() {\n"
              "    static int* p = 0;\n"
              "    delete p;\n"
              "    throw 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    static int* p = 0;\n"
              "    delete p;\n"
              "    throw 1;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Exception thrown in invalid state, 'p' points at deallocated memory. [exceptDeallocThrow]\n", errout_str());
    }

    void rethrowCopy1() {
        check("void f() {\n"
              "    try\n"
              "    {\n"
              "       foo();\n"
              "    }\n"
              "    catch(const exception& err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:9]: (style) Throwing a copy of the caught exception instead of rethrowing the original exception. [exceptRethrowCopy]\n", errout_str());
    }

    void rethrowCopy2() {
        check("void f() {\n"
              "    try\n"
              "    {\n"
              "       foo();\n"
              "    }\n"
              "    catch(exception& err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:9]: (style) Throwing a copy of the caught exception instead of rethrowing the original exception. [exceptRethrowCopy]\n", errout_str());
    }

    void rethrowCopy3() {
        check("void f() {\n"
              "    try {\n"
              "       foo();\n"
              "    }\n"
              "    catch(std::runtime_error& err) {\n"
              "        throw err;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:9]: (style) Throwing a copy of the caught exception instead of rethrowing the original exception. [exceptRethrowCopy]\n", errout_str());
    }

    void rethrowCopy4() {
        check("void f() {\n"
              "    try\n"
              "    {\n"
              "       foo();\n"
              "    }\n"
              "    catch(const exception& err)\n"
              "    {\n"
              "        exception err2;\n"
              "        throw err2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void rethrowCopy5() {
        check("void f() {\n"
              "    try {\n"
              "       foo();\n"
              "    }\n"
              "    catch(const exception& outer) {\n"
              "        try {\n"
              "           foo(outer);\n"
              "        }\n"
              "        catch(const exception& inner) {\n"
              "            throw inner;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:13]: (style) Throwing a copy of the caught exception instead of rethrowing the original exception. [exceptRethrowCopy]\n", errout_str());

        check("void f() {\n"
              "    try {\n"
              "       foo();\n"
              "    }\n"
              "    catch(const exception& outer) {\n"
              "        try {\n"
              "           foo(outer);\n"
              "        }\n"
              "        catch(const exception& inner) {\n"
              "            throw outer;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void catchExceptionByValue() {
        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch( ::std::exception err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5]: (style) Exception should be caught by reference. [catchExceptionByValue]\n", errout_str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(const exception err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5]: (style) Exception should be caught by reference. [catchExceptionByValue]\n", errout_str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch( ::std::exception& err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(exception* err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(const exception& err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(int err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(exception* const err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void noexceptThrow() {
        check("void func1() noexcept(false) { try {} catch(...) {;} throw 1; }\n"
              "void func2() noexcept { throw 1; }\n"
              "void func3() noexcept(true) { throw 1; }\n"
              "void func4() noexcept(false) { throw 1; }\n"
              "void func5() noexcept(true) { func1(); }\n"
              "void func6() noexcept(false) { func1(); }");
        ASSERT_EQUALS("[test.cpp:2:25]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n"
                      "[test.cpp:3:31]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n"
                      "[test.cpp:5:31]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n", errout_str());

        // avoid false positives
        check("const char *func() noexcept { return 0; }\n"
              "const char *func1() noexcept { try { throw 1; } catch(...) {} return 0; }");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n" // #14526
              "    void f(int = 0, int = 1) { throw 0; }\n"
              "};\n"
              "void g() noexcept {\n"
              "    A a;\n"
              "    a.f();\n"
              "}\n"
              "void h() noexcept {\n"
              "    A a;\n"
              "    a.f(1, 3);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:7]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n"
                      "[test.cpp:10:7]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n",
                      errout_str());
    }

    void nothrowThrow() {
        check("void func1() throw(int) { try {;} catch(...) { throw 1; } ; }\n"
              "void func2() throw() { throw 1; }\n"
              "void func3() throw(int) { throw 1; }\n"
              "void func4() throw() { func1(); }\n"
              "void func5() throw(int) { func1(); }");
        ASSERT_EQUALS("[test.cpp:2:24]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n"
                      "[test.cpp:4:24]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n", errout_str());

        // avoid false positives
        check("const char *func() throw() { return 0; }");
        ASSERT_EQUALS("", errout_str());


        // #11691: FP throwInNoexceptFunction with if constexpr in template
        check("template<bool IsNoThrow>\n"
              "static void foo(size_t Size) noexcept(IsNoThrow) {\n"
              "    if constexpr (!IsNoThrow) {\n"
              "        throw std::bad_alloc();\n"
              "    }\n"
              "}\n"
              "foo<true>(123);\n");
        ASSERT_EQUALS("", errout_str());
    }

    void unhandledExceptionSpecification1() { // #4800
        check("void myThrowingFoo() throw(MyException) {\n"
              "  throw MyException();\n"
              "}\n"
              "void myNonCatchingFoo() {\n"
              "  myThrowingFoo();\n"
              "}\n"
              "void myCatchingFoo() {\n"
              "  try {\n"
              "    myThrowingFoo();\n"
              "  } catch(MyException &) {}\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:3] -> [test.cpp:1:6]: (style, inconclusive) Unhandled exception specification when calling function myThrowingFoo(). [unhandledExceptionSpecification]\n", errout_str());
    }

    void unhandledExceptionSpecification2() {
        check("void f() const throw (std::runtime_error);\n"
              "int main()\n"
              "{\n"
              "    f();\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Unhandled exception thrown in function that is an entry point. [throwInEntryPoint]\n", errout_str());
    }

    void unhandledExceptionSpecification3() {
        const char code[] = "void f() const throw (std::runtime_error);\n"
                            "int _init() {\n"
                            "    f();\n"
                            "}\n"
                            "int _fini() {\n"
                            "    f();\n"
                            "}\n"
                            "int main()\n"
                            "{\n"
                            "    f();\n"
                            "}\n";

        check(code, dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:10:5]: (error) Unhandled exception thrown in function that is an entry point. [throwInEntryPoint]\n"
                      "[test.cpp:3:5] -> [test.cpp:1:6]: (style, inconclusive) Unhandled exception specification when calling function f(). [unhandledExceptionSpecification]\n"
                      "[test.cpp:6:5] -> [test.cpp:1:6]: (style, inconclusive) Unhandled exception specification when calling function f(). [unhandledExceptionSpecification]\n", errout_str());

        const Settings s = settingsBuilder().library("gnu.cfg").build();
        check(code, dinit(CheckOptions, $.inconclusive = true, $.s = &s));
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Unhandled exception thrown in function that is an entry point. [throwInEntryPoint]\n"
                      "[test.cpp:6:5]: (error) Unhandled exception thrown in function that is an entry point. [throwInEntryPoint]\n"
                      "[test.cpp:10:5]: (error) Unhandled exception thrown in function that is an entry point. [throwInEntryPoint]\n",
                      errout_str());
    }

    void nothrowAttributeThrow() {
        check("void func1() throw(int) { throw 1; }\n"
              "void func2() __attribute((nothrow)); void func2() { throw 1; }\n"
              "void func3() __attribute((nothrow)); void func3() { func1(); }");
        ASSERT_EQUALS("[test.cpp:2:53]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n"
                      "[test.cpp:3:53]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n", errout_str());

        // avoid false positives
        check("const char *func() __attribute((nothrow)); void func1() { return 0; }");
        ASSERT_EQUALS("", errout_str());
    }

    void nothrowAttributeThrow2() {
        check("class foo {\n"
              "  void copyMemberValues() throw () {\n"
              "      copyMemberValues();\n"
              "   }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void nothrowDeclspecThrow() {
        check("void func1() throw(int) { throw 1; }\n"
              "void __declspec(nothrow) func2() { throw 1; }\n"
              "void __declspec(nothrow) func3() { func1(); }");
        ASSERT_EQUALS("[test.cpp:2:36]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n"
                      "[test.cpp:3:36]: (error) Unhandled exception thrown in function declared not to throw exceptions. [throwInNoexceptFunction]\n", errout_str());

        // avoid false positives
        check("const char *func() __attribute((nothrow)); void func1() { return 0; }");
        ASSERT_EQUALS("", errout_str());
    }

    void rethrowNoCurrentException1() {
        check("void func1(const bool flag) { try{ if(!flag) throw; } catch (int&) { ; } }");
        ASSERT_EQUALS("[test.cpp:1:46]: (error) Rethrowing current exception with 'throw;', it seems there is no current exception to rethrow."
                      " If there is no current exception this calls std::terminate(). More: https://isocpp.org/wiki/faq/exceptions#throw-without-an-object [rethrowNoCurrentException]\n", errout_str());
    }

    void rethrowNoCurrentException2() {
        check("void func1() { try{ ; } catch (...) { ; } throw; }");
        ASSERT_EQUALS("[test.cpp:1:43]: (error) Rethrowing current exception with 'throw;', it seems there is no current exception to rethrow."
                      " If there is no current exception this calls std::terminate(). More: https://isocpp.org/wiki/faq/exceptions#throw-without-an-object [rethrowNoCurrentException]\n", errout_str());
    }

    void rethrowNoCurrentException3() {
        check("void on_error() { try { throw; } catch (const int &) { ; } catch (...) { ; } }\n"      // exception dispatcher idiom
              "void func2() { try{ ; } catch (const int&) { throw; } ; }\n"
              "void func3() { throw 0; }");
        ASSERT_EQUALS("", errout_str());
    }

    void noFunctionCall() {
        check("void f() {\n" // #13803
              "    throw \"error\";\n"
              "}\n"
              "void g() noexcept {\n"
              "    auto pF = &f;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void entryPoint() {
        check("void f(int i) {\n" // #14195
              "    if (i < 2)\n"
              "        throw 0;\n"
              "}\n"
              "int main(int argc, char* argv[]) {\n"
              "    f(argc);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Unhandled exception thrown in function that is an entry point. [throwInEntryPoint]\n",
                      errout_str());

        check("void f(int i) {\n"
              "    if (i < 2)\n"
              "        throw 0;\n"
              "}\n"
              "int main(int argc, char* argv[]) {\n"
              "    try {\n"
              "        f(argc);\n"
              "    } catch (...) {\n"
              "        return 1;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int i) {\n" // #9380
              "    throw i;\n"
              "}\n"
              "int main() {\n"
              "    f(123);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:5]: (error) Unhandled exception thrown in function that is an entry point. [throwInEntryPoint]\n",
                      errout_str());
    }
};

REGISTER_TEST(TestExceptionSafety)
