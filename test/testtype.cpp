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

#include "checktype.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"

#include <cstddef>
#include <string>

class TestType : public TestFixture {
public:
    TestType() : TestFixture("TestType") {}

private:


    void run() override {
        TEST_CASE(checkTooBigShift_Unix32);
        mNewTemplate = true;
        TEST_CASE(checkIntegerOverflow);
        TEST_CASE(signConversion);
        TEST_CASE(longCastAssign);
        TEST_CASE(longCastReturn);
        TEST_CASE(checkFloatToIntegerOverflow);
        TEST_CASE(integerOverflow); // #11794
        TEST_CASE(shiftTooManyBits); // #11496
    }

    struct CheckOptions
    {
        const Settings* settings = nullptr; // TODO: split from this
        Standards::cppstd_t standard = Standards::cppstd_t::CPP11;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings settings1 = settingsBuilder(options.settings ? *options.settings : settingsDefault).severity(Severity::warning).severity(Severity::portability).cpp(options.standard).build();

        // Tokenize..
        SimpleTokenizer tokenizer(settings1, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check..
        runChecks<CheckType>(tokenizer, this);
    }

    // TODO: get rid of this
    void check_(const char* file, int line, const std::string& code, const CheckOptions& options = make_default_obj()) {
        const Settings settings1 = settingsBuilder(options.settings ? *options.settings : settingsDefault).severity(Severity::warning).severity(Severity::portability).cpp(options.standard).build();

        // Tokenize..
        SimpleTokenizer tokenizer(settings1, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check..
        runChecks<CheckType>(tokenizer, this);
    }

    struct CheckPOptions
    {
        const Settings* settings = nullptr;
        bool cpp = true;
    };

#define checkP(...) checkP_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkP_(const char* file, int line, const char (&code)[size], const CheckPOptions& options = make_default_obj()) {
        const Settings settings1 = settingsBuilder(options.settings ? *options.settings : settingsDefault).severity(Severity::warning).severity(Severity::portability).build();

        SimpleTokenizer2 tokenizer(settings1, *this, code, options.cpp ? "test.cpp" : "test.c");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check..
        runChecks<CheckType>(tokenizer, this);
    }

    void checkTooBigShift_Unix32() {
        const Settings settings = settingsBuilder().platform(Platform::Type::Unix32).build();

        // unsigned types getting promoted to int sizeof(int) = 4 bytes
        // and unsigned types having already a size of 4 bytes
        {
            const std::string types[] = {"unsigned char", "bool", "unsigned short", "unsigned int", "unsigned long"};
            for (const std::string& type : types) {
                check(type + " f(" + type +" x) { return x << 31; }", dinit(CheckOptions, $.settings = &settings));
                ASSERT_EQUALS_MSG("", errout_str(), type);
                check(type + " f(" + type +" x) { return x << 33; }", dinit(CheckOptions, $.settings = &settings));
                ASSERT_EQUALS_MSG("[test.cpp:1]: (error) Shifting 32-bit value by 33 bits is undefined behaviour\n", errout_str(), type);
                check(type + " f(int x) { return (x = (" + type + ")x << 32); }", dinit(CheckOptions, $.settings = &settings));
                ASSERT_EQUALS_MSG("[test.cpp:1]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n", errout_str(), type);
                check(type + " foo(" + type + " x) { return x << 31; }", dinit(CheckOptions, $.settings = &settings));
                ASSERT_EQUALS_MSG("", errout_str(), type);
            }
        }
        // signed types getting promoted to int sizeof(int) = 4 bytes
        // and signed types having already a size of 4 bytes
        {
            const std::string types[] = {"signed char", "signed short", /*[signed]*/ "short", "wchar_t", /*[signed]*/ "int", "signed int", /*[signed]*/ "long", "signed long"};
            for (const std::string& type : types) {
                // c++11
                check(type + " f(" + type +" x) { return x << 33; }", dinit(CheckOptions, $.settings = &settings));
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 33 bits is undefined behaviour\n", errout_str());
                check(type + " f(int x) { return (x = (" + type + ")x << 32); }", dinit(CheckOptions, $.settings = &settings));
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n", errout_str());
                check(type + " foo(" + type + " x) { return x << 31; }", dinit(CheckOptions, $.settings = &settings));
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting signed 32-bit value by 31 bits is undefined behaviour\n", errout_str());
                check(type + " foo(" + type + " x) { return x << 30; }", dinit(CheckOptions, $.settings = &settings));
                ASSERT_EQUALS("", errout_str());

                // c++14
                check(type + " foo(" + type + " x) { return x << 31; }", dinit(CheckOptions, $.settings = &settings, $.standard = Standards::CPP14));
                ASSERT_EQUALS("[test.cpp:1]: (portability) Shifting signed 32-bit value by 31 bits is implementation-defined behaviour\n", errout_str());
                check(type + " f(int x) { return (x = (" + type + ")x << 32); }", dinit(CheckOptions, $.settings = &settings, $.standard = Standards::CPP14));
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n", errout_str());
            }
        }
        // 64 bit width types
        {
            // unsigned long long
            check("unsigned long long foo(unsigned long long x) { return x << 64; }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout_str());
            check("unsigned long long f(int x) { return (x = (unsigned long long)x << 64); }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout_str());
            check("unsigned long long f(unsigned long long x) { return x << 63; }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("", errout_str());
            // [signed] long long
            check("long long foo(long long x) { return x << 64; }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout_str());
            check("long long f(int x) { return (x = (long long)x << 64); }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout_str());
            check("long long f(long long x) { return x << 63; }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting signed 64-bit value by 63 bits is undefined behaviour\n", errout_str());
            check("long long f(long long x) { return x << 62; }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("", errout_str());
            // signed long long
            check("signed long long foo(signed long long x) { return x << 64; }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout_str());
            check("signed long long f(long long x) { return (x = (signed long long)x << 64); }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout_str());
            check("signed long long f(signed long long x) { return x << 63; }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting signed 64-bit value by 63 bits is undefined behaviour\n", errout_str());
            check("signed long long f(signed long long x) { return x << 62; }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("", errout_str());

            // c++14
            check("signed long long foo(signed long long x) { return x << 64; }", dinit(CheckOptions, $.settings = &settings, $.standard = Standards::CPP14));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout_str());
            check("signed long long f(long long x) { return (x = (signed long long)x << 64); }", dinit(CheckOptions, $.settings = &settings, $.standard = Standards::CPP14));
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout_str());
            check("signed long long f(signed long long x) { return x << 63; }", dinit(CheckOptions, $.settings = &settings, $.standard = Standards::CPP14));
            ASSERT_EQUALS("[test.cpp:1]: (portability) Shifting signed 64-bit value by 63 bits is implementation-defined behaviour\n", errout_str());
            check("signed long long f(signed long long x) { return x << 62; }",dinit(CheckOptions, $.settings = &settings));
            ASSERT_EQUALS("", errout_str());
        }

        check("void f() { int x; x = 1 >> 64; }", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 64 bits is undefined behaviour\n", errout_str());

        check("void foo() {\n"
              "  QList<int> someList;\n"
              "  someList << 300;\n"
              "}", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        // Ticket #6793
        check("template<unsigned int I> int foo(unsigned int x) { return x << I; }\n"
              "const unsigned int f = foo<31>(0);\n"
              "const unsigned int g = foo<100>(0);\n"
              "template<unsigned int I> int hoo(unsigned int x) { return x << 32; }\n"
              "const unsigned int h = hoo<100>(0);", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:4]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n"
                      "[test.cpp:1]: (error) Shifting 32-bit value by 100 bits is undefined behaviour\n", errout_str());

        // #7266: C++, shift in macro
        check("void f(unsigned int x) {\n"
              "    UINFO(x << 1234);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8640
        check("int f (void)\n"
              "{\n"
              "    constexpr const int a = 1;\n"
              "    constexpr const int shift[1] = {32};\n"
              "    constexpr const int ret = a << shift[0];\n" // shift too many bits
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n"
                      "[test.cpp:5]: (error) Signed integer overflow for expression 'a<<shift[0]'.\n", errout_str());

        // #8885
        check("int f(int k, int rm) {\n"
              "  if (k == 32)\n"
              "    return 0;\n"
              "  if (k > 32)\n"
              "    return 0;\n"
              "  return rm>> k;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4] -> [test.cpp:6]: (warning) Shifting signed 32-bit value by 31 bits is undefined behaviour. See condition at line 4.\n",
            errout_str());

        check("int f(int k, int rm) {\n"
              "  if (k == 0 || k == 32)\n"
              "    return 0;\n"
              "  else if (k > 32)\n"
              "    return 0;\n"
              "  else\n"
              "    return rm>> k;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4] -> [test.cpp:7]: (warning) Shifting signed 32-bit value by 31 bits is undefined behaviour. See condition at line 4.\n",
            errout_str());

        check("int f(int k, int rm) {\n"
              "  if (k == 0 || k == 32 || k == 31)\n"
              "    return 0;\n"
              "  else if (k > 32)\n"
              "    return 0;\n"
              "  else\n"
              "    return rm>> k;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("static long long f(int x, long long y) {\n"
              "    if (x >= 64)\n"
              "        return 0;\n"
              "    return -(y << (x-1));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool f() {\n"
              "    std::ofstream outfile;\n"
              "    outfile << vec_points[0](0) << static_cast<int>(d) << ' ';\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(unsigned b, int len, unsigned char rem) {\n" // #10773
              "    int bits = 0;\n"
              "    while (len > 8) {\n"
              "        b = b >> rem;\n"
              "        bits += 8 - rem;\n"
              "        if (bits == 512)\n"
              "            len -= 8;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void checkIntegerOverflow() {
        const Settings settings = settingsBuilder().severity(Severity::warning).platform(Platform::Type::Unix32).build();

        check("x = (int)0x10000 * (int)0x10000;", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:1:18]: (error) Signed integer overflow for expression '(int)0x10000*(int)0x10000'. [integerOverflow]\n", errout_str());

        check("x = (long)0x10000 * (long)0x10000;", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:1:19]: (error) Signed integer overflow for expression '(long)0x10000*(long)0x10000'. [integerOverflow]\n", errout_str());

        check("void foo() {\n"
              "    int intmax = 0x7fffffff;\n"
              "    return intmax + 1;\n"
              "}",dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:3:19]: (error) Signed integer overflow for expression 'intmax+1'. [integerOverflow]\n", errout_str());

        check("void foo() {\n"
              "    int intmax = 0x7fffffff;\n"
              "    return intmax - 1;\n"
              "}",dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return x * x;\n"
              "}",dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:13]: (warning) Either the condition 'x==123456' is redundant or there is signed integer overflow for expression 'x*x'. [integerOverflowCond]\n", errout_str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return -123456 * x;\n"
              "}",dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:9] -> [test.cpp:3:19]: (warning) Either the condition 'x==123456' is redundant or there is signed integer underflow for expression '-123456*x'. [integerOverflowCond]\n", errout_str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return 123456U * x;\n"
              "}",dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        check("int f(int i) {\n" // #12117
              "    return (i == 31) ? 1 << i : 0;\n"
              "}", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:15] -> [test.cpp:2:26]: (warning) Shifting signed 32-bit value by 31 bits is undefined behaviour. See condition at line 2. [shiftTooManyBitsSigned]\n", errout_str());

        check("void f() {\n" // #13092
              "    int n = 0;\n"
              "    for (int i = 0; i < 10; i++) {\n"
              "        n = n * 47163 - 57412;\n"
              "    }\n"
              "}", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:4:15]: (error) Signed integer underflow for expression 'n*47163'. [integerOverflow]\n"
                      "[test.cpp:4:23]: (error) Signed integer underflow for expression 'n*47163-57412'. [integerOverflow]\n",
                      errout_str());
    }

    void signConversion() {
        const Settings settings = settingsBuilder().platform(Platform::Type::Unix64).build();
        check("x = -4 * (unsigned)y;");
        ASSERT_EQUALS("[test.cpp:1:6]: (warning) Expression '-4' has a negative value. That is converted to an unsigned value and used in an unsigned calculation. [signConversion]\n", errout_str());

        check("x = (unsigned)y * -4;");
        ASSERT_EQUALS("[test.cpp:1:20]: (warning) Expression '-4' has a negative value. That is converted to an unsigned value and used in an unsigned calculation. [signConversion]\n", errout_str());

        check("unsigned int dostuff(int x) {\n" // x is signed
              "  if (x==0) {}\n"
              "  return (x-1)*sizeof(int);\n"
              "}", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:8] -> [test.cpp:3:12]: (warning) Expression 'x-1' can have a negative value. That is converted to an unsigned value and used in an unsigned calculation. [signConversionCond]\n", errout_str());

        check("unsigned int f1(signed int x, unsigned int y) {" // x is signed
              "  return x * y;\n"
              "}\n"
              "void f2() { f1(-4,4); }");
        ASSERT_EQUALS(
            "[test.cpp:1:57]: (warning) Expression 'x' can have a negative value. That is converted to an unsigned value and used in an unsigned calculation. [signConversion]\n",
            errout_str());

        check("unsigned int f1(int x) {"
              "  return x * 5U;\n"
              "}\n"
              "void f2() { f1(-4); }");
        ASSERT_EQUALS(
            "[test.cpp:1:34]: (warning) Expression 'x' can have a negative value. That is converted to an unsigned value and used in an unsigned calculation. [signConversion]\n",
            errout_str());

        check("unsigned int f1(int x) {" // #6168: FP for inner calculation
              "  return 5U * (1234 - x);\n" // <- signed subtraction, x is not sign converted
              "}\n"
              "void f2() { f1(-4); }");
        ASSERT_EQUALS("", errout_str());

        // Don't warn for + and -
        check("void f1(int x) {"
              "  a = x + 5U;\n"
              "}\n"
              "void f2() { f1(-4); }");
        ASSERT_EQUALS("", errout_str());

        check("size_t foo(size_t x) {\n"
              " return -2 * x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Expression '-2' has a negative value. That is converted to an unsigned value and used in an unsigned calculation. [signConversion]\n", errout_str());

        checkP("void f() {\n" // #12110 FP signConversion with integer overflow
               "    if (LLONG_MIN / (-1)) {}\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void longCastAssign() {
        const Settings settings = settingsBuilder().severity(Severity::style).platform(Platform::Type::Unix64).build();
        const Settings settingsWin = settingsBuilder().severity(Severity::style).platform(Platform::Type::Win64).build();

        const char code[] = "long f(int x, int y) {\n"
                            "  const long ret = x * y;\n"
                            "  return ret;\n"
                            "}\n";
        check(code, dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:18]: (style) int result is assigned to long variable. If the variable is long to avoid loss of information, then you have loss of information. [truncLongCastAssignment]\n", errout_str());
        check(code, dinit(CheckOptions, $.settings = &settingsWin));
        ASSERT_EQUALS("", errout_str());

        check("long f(int x, int y) {\n"
              "  long ret = x * y;\n"
              "  return ret;\n"
              "}\n", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:12]: (style) int result is assigned to long variable. If the variable is long to avoid loss of information, then you have loss of information. [truncLongCastAssignment]\n", errout_str());

        check("long f() {\n"
              "  const long long ret = 256 * (1 << 10);\n"
              "  return ret;\n"
              "}\n", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        // typedef
        check("long f(int x, int y) {\n"
              "  const size_t ret = x * y;\n"
              "  return ret;\n"
              "}\n", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        // astIsIntResult
        check("long f(int x, int y) {\n"
              "  const long ret = (long)x * y;\n"
              "  return ret;\n"
              "}\n", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        check("double g(float f) {\n"
              "    return f * f;\n"
              "}\n", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:5]: (style) float result is returned as double value. If the return value is double to avoid loss of information, then you have loss of information. [truncLongCastReturn]\n",
                      errout_str());

        check("void f(int* p) {\n" // #11862
              "    long long j = *(p++);\n"
              "}\n", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());

        check("template <class T>\n" // #12393
              "struct S {\n"
              "    S& operator=(const S&) { return *this; }\n"
              "    struct U {\n"
              "        S<T>* p;\n"
              "    };\n"
              "    U u;\n"
              "};\n", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str()); // don't crash

        check("void f(long& r, long i) {\n"
              "    r = 1 << i;\n"
              "}\n", dinit(CheckOptions, $.settings = &settingsWin));
        ASSERT_EQUALS("", errout_str());
    }

    void longCastReturn() {
        const Settings settings = settingsBuilder().severity(Severity::style).platform(Platform::Type::Unix64).build();
        const Settings settingsWin = settingsBuilder().severity(Severity::style).platform(Platform::Type::Win64).build();

        const char code[] = "long f(int x, int y) {\n"
                            "  return x * y;\n"
                            "}\n";
        check(code, dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:3]: (style) int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information. [truncLongCastReturn]\n", errout_str());
        check(code, dinit(CheckOptions, $.settings = &settingsWin));
        ASSERT_EQUALS("", errout_str());

        const char code2[] = "long long f(int x, int y) {\n"
                             "  return x * y;\n"
                             "}\n";
        check(code2, dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:3]: (style) int result is returned as long long value. If the return value is long long to avoid loss of information, then you have loss of information. [truncLongCastReturn]\n", errout_str());
        check(code2, dinit(CheckOptions, $.settings = &settingsWin));
        ASSERT_EQUALS("[test.cpp:2:3]: (style) int result is returned as long long value. If the return value is long long to avoid loss of information, then you have loss of information. [truncLongCastReturn]\n", errout_str());

        const char code3a[] = "long f() {\n"
                              "    int n = 1;\n"
                              "    return n << 12;\n"
                              "}\n";
        check(code3a, dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());
        const char code3b[] = "long f(int n) {\n"
                              "    return n << 12;\n"
                              "}\n";
        check(code3b, dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:5]: (style) int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information. [truncLongCastReturn]\n", errout_str());

        const char code3c[] = "long f() {\n"
                              "    unsigned int n = 1U << 20;\n"
                              "    return n << 20;\n"
                              "}\n";
        check(code3c, dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information. [truncLongCastReturn]\n", errout_str());

        const char code4a[] = "long f(int x) {\n"
                              "    int y = 0x07FFFF;\n"
                              "    return ((x & y) << (12 & x));\n"
                              "}\n";
        check(code4a, dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("", errout_str());
        const char code4b[] = "long f(int x) {\n"
                              "    int y = 0x080000;\n"
                              "    return ((x & y) << (12 & x));\n"
                              "}\n";
        check(code4b, dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information. [truncLongCastReturn]\n", errout_str());

        // typedef
        check("size_t f(int x, int y) {\n"
              "  return x * y;\n"
              "}\n", dinit(CheckOptions, $.settings = &settings));
        ASSERT_EQUALS("[test.cpp:2:3]: (style) int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information. [truncLongCastReturn]\n", errout_str());
    }

    // This function ensure that test works with different compilers. Floats can
    // be stringified differently.
    static std::string removeFloat(const std::string& msg) {
        const std::string::size_type pos1 = msg.find("float (");
        const std::string::size_type pos2 = msg.find(") to integer conversion");
        if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 > pos2)
            return msg;
        return msg.substr(0,pos1+7) + msg.substr(pos2);
    }

    void checkFloatToIntegerOverflow() {
        check("x = (int)1E100;");
        ASSERT_EQUALS("[test.cpp:1:5]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        check("void f(void) {\n"
              "  return (int)1E100;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        check("void f(void) {\n"
              "  return (int)-1E100;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        check("void f(void) {\n"
              "  return (short)1E6;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        check("void f(void) {\n"
              "  return (unsigned char)256.0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        check("void f(void) {\n"
              "  return (unsigned char)255.5;\n"
              "}");
        ASSERT_EQUALS("", removeFloat(errout_str()));

        check("void f(void) {\n"
              "  char c = 1234.5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        check("char f(void) {\n"
              "  return 1234.5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:3]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        checkP("#define TEST(b, f) b ? 5000 : (unsigned short)f\n" // #11685
               "void f()\n"
               "{\n"
               "    unsigned short u = TEST(true, 75000.0);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkP("#define TEST(b, f) b ? 5000 : (unsigned short)f\n"
               "void f()\n"
               "{\n"
               "    unsigned short u = TEST(false, 75000.0);\n"
               "}\n");
        ASSERT_EQUALS("[test.cpp:4:24]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        check( "bool f(unsigned short x);\n"
               "bool g() {\n"
               "    return false && f((unsigned short)75000.0);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        check( "bool f(unsigned short x);\n"
               "bool g() {\n"
               "    return true && f((unsigned short)75000.0);\n"
               "}\n");
        ASSERT_EQUALS("[test.cpp:3:22]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        check( "bool f(unsigned short x);\n"
               "bool g() {\n"
               "    return true || f((unsigned short)75000.0);\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        check( "bool f(unsigned short x);\n"
               "bool g() {\n"
               "    return false || f((unsigned short)75000.0);\n"
               "}\n");
        ASSERT_EQUALS("[test.cpp:3:23]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

        checkP("#define TEST(b, f) b ? 5000 : (unsigned short)f\n" // #11685
               "void f()\n"
               "{\n"
               "    unsigned short u = TEST(true, 75000.0);\n"
               "}\n", dinit(CheckPOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkP("#define TEST(b, f) b ? 5000 : (unsigned short)f\n"
               "void f()\n"
               "{\n"
               "    unsigned short u = TEST(false, 75000.0);\n"
               "}\n", dinit(CheckPOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:4:24]: (error) Undefined behaviour: float () to integer conversion overflow. [floatConversionOverflow]\n", removeFloat(errout_str()));

    }

    void integerOverflow() { // #11794
        // std.cfg for int32_t
        // Platform::Unix32 for INT_MIN=-2147483648 and INT32_MAX=2147483647
        const Settings s = settingsBuilder().library("std.cfg").cpp(Standards::CPP11).platform(Platform::Unix32).build();

        checkP("int fun(int x)\n"
               "{\n"
               "  if(x < 0) x = -x;\n"
               "  return x >= 0;\n"
               "}\n"
               "int f()\n"
               "{\n"
               "    fun(INT_MIN);\n"
               "}", dinit(CheckPOptions, $.settings = &s));
        ASSERT_EQUALS("[test.cpp:3:17]: (error) Signed integer overflow for expression '-x'. [integerOverflow]\n", errout_str());

        checkP("void f() {\n" // #8399
               "    int32_t i = INT32_MAX;\n"
               "    i << 1;\n"
               "    i << 2;\n"
               "}", dinit(CheckPOptions, $.settings = &s));
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Signed integer overflow for expression 'i<<2'. [integerOverflow]\n", errout_str());
    }

    void shiftTooManyBits() { // #11496
        check("template<unsigned int width> struct B {\n"
              "    unsigned long long f(unsigned int n) const {\n"
              "        if (width == 1)\n"
              "            return 1ULL << width;\n"
              "        return 0;\n"
              "    }\n"
              "};\n"
              "static B<64> b;\n");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestType)
