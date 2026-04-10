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

#include "astutils.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"

#include <cstdint>
#include <cstring>

class TestAstUtils : public TestFixture {
public:
    TestAstUtils() : TestFixture("TestAstUtils") {}

private:

    // TODO: test with C code

    void run() override {
        TEST_CASE(findLambdaEndTokenTest);
        TEST_CASE(findLambdaStartTokenTest);
        TEST_CASE(isNullOperandTest);
        TEST_CASE(isReturnScopeTest);
        TEST_CASE(isSameExpressionCpp);
        TEST_CASE(isSameExpressionC);
        TEST_CASE(isVariableChangedTest);
        TEST_CASE(isVariableChangedByFunctionCallTest);
        TEST_CASE(isExpressionChangedTest);
        TEST_CASE(nextAfterAstRightmostLeafTest);
        TEST_CASE(isUsedAsBool);
    }

#define findLambdaEndToken(...) findLambdaEndToken_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool findLambdaEndToken_(const char* file, int line, const char (&code)[size], const char pattern[] = nullptr, bool checkNext = true) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token* const tokStart = pattern ? Token::findsimplematch(tokenizer.tokens(), pattern, strlen(pattern)) : tokenizer.tokens();
        const Token * const tokEnd = (::findLambdaEndToken)(tokStart);
        return tokEnd && (!checkNext || tokEnd->next() == nullptr);
    }

    void findLambdaEndTokenTest() {
        const Token* nullTok = nullptr;
        ASSERT(nullptr == (::findLambdaEndToken)(nullTok));
        ASSERT_EQUALS(false, findLambdaEndToken("void f() { }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[]{ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[]{ return 0; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](){ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[&](){ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[&, i](){ }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) { return -1; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](int a, int b) { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](int a, int b) mutable { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](int a, int b) constexpr { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) mutable -> int { return -1; }"));
        ASSERT_EQUALS(false, findLambdaEndToken("[](void) foo -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> int* { return x; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> const * int { return x; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) mutable -> const * int { return x; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> const ** int { return x; }"));
        ASSERT_EQUALS(true, findLambdaEndToken("[](void) constexpr -> const * const* int { return x; }"));
        ASSERT_EQUALS(false, findLambdaEndToken("int** a[] { new int*[2] { new int, new int} };", "[ ]"));
        ASSERT_EQUALS(false, findLambdaEndToken("int** a[] { new int*[2] { new int, new int} };", "[ 2"));
        ASSERT_EQUALS(false, findLambdaEndToken("shared_ptr<Type *[]> sp{ new Type *[2] {new Type, new Type}, Deleter<Type>{ 2 } };", "[ 2"));
        ASSERT_EQUALS(true, findLambdaEndToken("int i = 5 * []{ return 7; }();", "[", /*checkNext*/ false));
    }

#define findLambdaStartToken(...) findLambdaStartToken_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool findLambdaStartToken_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token * const tokStart = (::findLambdaStartToken)(tokenizer.list.back());
        return tokStart && tokStart == tokenizer.list.front();
    }

    void findLambdaStartTokenTest() {
        ASSERT(nullptr == (::findLambdaStartToken)(nullptr));
        ASSERT_EQUALS(false, findLambdaStartToken("void f() { }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[]{ }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[]{ return 0; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](){ }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[&](){ }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[&, i](){ }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) { return -1; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](int a, int b) { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](int a, int b) mutable { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](int a, int b) constexpr { return a + b; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) mutable -> int { return -1; }"));
        ASSERT_EQUALS(false, findLambdaStartToken("[](void) foo -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> int { return -1; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> int* { return x; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> const * int { return x; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) mutable -> const * int { return x; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> const ** int { return x; }"));
        ASSERT_EQUALS(true, findLambdaStartToken("[](void) constexpr -> const * const* int { return x; }"));
    }

#define isNullOperand(...) isNullOperand_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool isNullOperand_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        return (::isNullOperand)(tokenizer.tokens());
    }

    void isNullOperandTest() {
        ASSERT_EQUALS(true, isNullOperand("(void*)0;"));
        ASSERT_EQUALS(true, isNullOperand("(void*)0U;"));
        ASSERT_EQUALS(true, isNullOperand("(void*)0x0LL;"));
        ASSERT_EQUALS(true, isNullOperand("NULL;"));
        ASSERT_EQUALS(true, isNullOperand("nullptr;"));
        ASSERT_EQUALS(true, isNullOperand("(void*)NULL;"));
        ASSERT_EQUALS(true, isNullOperand("static_cast<int*>(0);"));
        ASSERT_EQUALS(false, isNullOperand("0;"));
        ASSERT_EQUALS(false, isNullOperand("(void*)0.0;"));
        ASSERT_EQUALS(false, isNullOperand("(void*)1;"));
    }

#define isReturnScope(...) isReturnScope_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool isReturnScope_(const char* file, int line, const char (&code)[size], int offset) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token * const tok = (offset < 0)
                                  ? tokenizer.list.back()->tokAt(1+offset)
                                  : tokenizer.tokens()->tokAt(offset);
        return (isReturnScope)(tok, settingsDefault.library);
    }

    void isReturnScopeTest() {
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { return; } }", -2));
        ASSERT_EQUALS(true, isReturnScope("int f() { if (a) { return {}; } }", -2));                        // #8891
        ASSERT_EQUALS(true, isReturnScope("std::string f() { if (a) { return std::string{}; } }", -2));     // #8891
        ASSERT_EQUALS(true, isReturnScope("std::string f() { if (a) { return std::string{\"\"}; } }", -2)); // #8891
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { return (ab){0}; } }", -2)); // #7103
        ASSERT_EQUALS(false, isReturnScope("void f() { if (a) { return (ab){0}; } }", -4)); // #7103
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { {throw new string(x);}; } }", -4)); // #7144
        ASSERT_EQUALS(true, isReturnScope("void f() { if (a) { {throw new string(x);}; } }", -2)); // #7144
        ASSERT_EQUALS(false, isReturnScope("void f() { [=]() { return data; }; }", -1));
        ASSERT_EQUALS(true, isReturnScope("auto f() { return [=]() { return data; }; }", -1));
        ASSERT_EQUALS(true, isReturnScope("auto f() { return [=]() { return data; }(); }", -1));
        ASSERT_EQUALS(false, isReturnScope("auto f() { [=]() { return data; }(); }", -1));

        ASSERT_EQUALS(true, isReturnScope("void negativeTokenOffset() { return; }", -1));
        ASSERT_EQUALS(false, isReturnScope("void zeroTokenOffset() { return; }", 0));
        ASSERT_EQUALS(true, isReturnScope("void positiveTokenOffset() { return; }", 7));
    }

#define isSameExpression(...) isSameExpression_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool isSameExpression_(const char* file, int line, const char (&code)[size], const char tokStr1[], const char tokStr2[], bool cpp) {
        SimpleTokenizer tokenizer(settingsDefault, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token * const tok1 = Token::findsimplematch(tokenizer.tokens(), tokStr1, strlen(tokStr1));
        const Token * const tok2 = Token::findsimplematch(tok1->next(), tokStr2, strlen(tokStr2));
        return (isSameExpression)(false, tok1, tok2, settingsDefault, false, true);
    }

    void isSameExpressionTestInternal(bool cpp) {
        ASSERT_EQUALS(true,  isSameExpression("x = 1 + 1;", "1", "1", cpp));
        ASSERT_EQUALS(false, isSameExpression("x = 1 + 1u;", "1", "1u", cpp));
        ASSERT_EQUALS(true,  isSameExpression("x = 1.0 + 1.0;", "1.0", "1.0", cpp));
        ASSERT_EQUALS(false, isSameExpression("x = 1.0f + 1.0;", "1.0f", "1.0", cpp));
        ASSERT_EQUALS(false, isSameExpression("x = 1L + 1;", "1L", "1", cpp));
        ASSERT_EQUALS(true,  isSameExpression("x = 0.0f + 0x0p+0f;", "0.0f", "0x0p+0f", cpp));
        ASSERT_EQUALS(true,  isSameExpression("x < x;", "x", "x", cpp));
        ASSERT_EQUALS(false, isSameExpression("x < y;", "x", "y", cpp));
        ASSERT_EQUALS(true,  isSameExpression("(x + 1) < (x + 1);", "+", "+", cpp));
        ASSERT_EQUALS(false, isSameExpression("(x + 1) < (x + 1L);", "+", "+", cpp));
        ASSERT_EQUALS(!cpp,  isSameExpression("(1 + x) < (x + 1);", "+", "+", cpp));
        ASSERT_EQUALS(false, isSameExpression("(1.0l + x) < (1.0 + x);", "+", "+", cpp));
        ASSERT_EQUALS(!cpp,  isSameExpression("(0.0 + x) < (x + 0x0p+0);", "+", "+", cpp));
        ASSERT_EQUALS(true,  isSameExpression("void f() {double y = 1e1; (x + y) < (x + 10.0); } ", "+", "+", cpp));
        ASSERT_EQUALS(!cpp,  isSameExpression("void f() {double y = 1e1; (x + 10.0) < (y + x); } ", "+", "+", cpp));
        ASSERT_EQUALS(true,  isSameExpression("void f() {double y = 1e1; double z = 10.0; (x + y) < (x + z); } ", "+", "+", cpp));
        ASSERT_EQUALS(true,  isSameExpression("A + A", "A", "A", cpp));

        // the remaining test cases are not valid C code
        if (!cpp)
            return;

        //https://trac.cppcheck.net/ticket/9700
        ASSERT_EQUALS(true, isSameExpression("A::B + A::B;", "::", "::", cpp));
        ASSERT_EQUALS(false, isSameExpression("A::B + A::C;", "::", "::", cpp));
        ASSERT_EQUALS(true, isSameExpression("A::B* get() { if(x) return new A::B(true); else return new A::B(true); }", "new", "new", cpp));
        ASSERT_EQUALS(false, isSameExpression("A::B* get() { if(x) return new A::B(true); else return new A::C(true); }", "new", "new", cpp));
    }

    void isSameExpressionCpp() {
        isSameExpressionTestInternal(true);
    }

    void isSameExpressionC() {
        isSameExpressionTestInternal(false);
    }

#define isVariableChanged(...) isVariableChanged_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool isVariableChanged_(const char* file, int line, const char (&code)[size], const char startPattern[], const char endPattern[]) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token * const tok1 = Token::findsimplematch(tokenizer.tokens(), startPattern, strlen(startPattern));
        const Token * const tok2 = Token::findsimplematch(tokenizer.tokens(), endPattern, strlen(endPattern));
        return (isVariableChanged)(tok1, tok2, 1, false, settingsDefault);
    }

    void isVariableChangedTest() {
        // #8211 - no lhs for >> , do not crash
        (void)isVariableChanged("void f() {\n"
                                "  int b;\n"
                                "  if (b) { (int)((INTOF(8))result >> b); }\n"
                                "}", "if", "}");
        // #9235
        ASSERT_EQUALS(false,
                      isVariableChanged("void f() {\n"
                                        "    int &a = a;\n"
                                        "}\n",
                                        "= a",
                                        "}"));

        ASSERT_EQUALS(false, isVariableChanged("void f(const A& a) { a.f(); }", "{", "}"));
        ASSERT_EQUALS(true,
                      isVariableChanged("void g(int*);\n"
                                        "void f(int x) { g(&x); }\n",
                                        "{",
                                        "}"));
        ASSERT_EQUALS(false, isVariableChanged("const int A[] = { 1, 2, 3 };", "[", "]"));
    }

#define isVariableChangedByFunctionCall(...) isVariableChangedByFunctionCall_( __FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool isVariableChangedByFunctionCall_(const char* file, int line, const char (&code)[size], const char pattern[], bool *inconclusive) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token * const argtok = Token::findmatch(tokenizer.tokens(), pattern);
        ASSERT_LOC(argtok, file, line);
        int indirect = (argtok->variable() && argtok->variable()->isArray());
        return (isVariableChangedByFunctionCall)(argtok, indirect, settingsDefault, inconclusive);
    }

    void isVariableChangedByFunctionCallTest() {

        { // #8271 - template method
            const char code[] = "void f(int x) {\n"
                                "  a<int>(x);\n"
                                "}";
            bool inconclusive = false;
            ASSERT_EQUALS(false, isVariableChangedByFunctionCall(code, "x ) ;", &inconclusive));
            ASSERT_EQUALS(true, inconclusive);
        }

        {
            const char code[] = "int f(int x) {\n"
                                "return int(x);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(false, isVariableChangedByFunctionCall(code, "x ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(int* p);\n"
                                "void f(int x) {\n"
                                "    return g(&x);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(true, isVariableChangedByFunctionCall(code, "x ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(const int* p);\n"
                                "void f(int x) {\n"
                                "    return g(&x);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(false, isVariableChangedByFunctionCall(code, "x ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(int** pp);\n"
                                "void f(int* p) {\n"
                                "    return g(&p);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(true, isVariableChangedByFunctionCall(code, "p ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(int* const* pp);\n"
                                "void f(int* p) {\n"
                                "    return g(&p);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(false, isVariableChangedByFunctionCall(code, "p ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(int a[2]);\n"
                                "void f() {\n"
                                "    int b[2] = {};\n"
                                "    return g(b);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(true, isVariableChangedByFunctionCall(code, "b ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(const int a[2]);\n"
                                "void f() {\n"
                                "    int b[2] = {};\n"
                                "    return g(b);\n"
                                "}\n";
            bool inconclusive = false;
            TODO_ASSERT_EQUALS(false, true, isVariableChangedByFunctionCall(code, "b ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(std::array<int, 2> a);\n"
                                "void f() {\n"
                                "    std::array<int, 2> b = {};\n"
                                "    return g(b);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(false, isVariableChangedByFunctionCall(code, "b ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(std::array<int, 2>& a);\n"
                                "void f() {\n"
                                "    std::array<int, 2> b = {};\n"
                                "    return g(b);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(true, isVariableChangedByFunctionCall(code, "b ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(const std::array<int, 2>& a);\n"
                                "void f() {\n"
                                "    std::array<int, 2> b = {};\n"
                                "    return g(b);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(false, isVariableChangedByFunctionCall(code, "b ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "void g(std::array<int, 2>* p);\n"
                                "void f() {\n"
                                "    std::array<int, 2> b = {};\n"
                                "    return g(&b);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(true, isVariableChangedByFunctionCall(code, "b ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "struct S {};\n"
                                "void g(S);\n"
                                "void f(S* s) {\n"
                                "    g(*s);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(false, isVariableChangedByFunctionCall(code, "s ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }

        {
            const char code[] = "struct S {};\n"
                                "void g(const S&);\n"
                                "void f(S* s) {\n"
                                "    g(*s);\n"
                                "}\n";
            bool inconclusive = false;
            ASSERT_EQUALS(false, isVariableChangedByFunctionCall(code, "s ) ;", &inconclusive));
            ASSERT_EQUALS(false, inconclusive);
        }
    }

#define isExpressionChanged(...) isExpressionChanged_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool isExpressionChanged_(const char* file,
                              int line,
                              const char (&code)[size],
                              const char var[],
                              const char startPattern[],
                              const char endPattern[])
    {
        const Settings settings = settingsBuilder().library("std.cfg").build();
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token* const start = Token::findsimplematch(tokenizer.tokens(), startPattern, strlen(startPattern));
        const Token* const end = Token::findsimplematch(start, endPattern, strlen(endPattern));
        const Token* const expr = Token::findsimplematch(tokenizer.tokens(), var, strlen(var));
        return (findExpressionChanged)(expr, start, end, settings);
    }

    void isExpressionChangedTest()
    {
        ASSERT_EQUALS(true, isExpressionChanged("void f(std::map<int, int>& m) { m[0]; }", "m [", "{", "}"));
        ASSERT_EQUALS(false, isExpressionChanged("void f(const A& a) { a.f(); }", "a .", "{", "}"));

        ASSERT_EQUALS(true,
                      isExpressionChanged("void g(int*);\n"
                                          "void f(int x) { g(&x); }\n",
                                          "x",
                                          ") {",
                                          "}"));

        ASSERT_EQUALS(true,
                      isExpressionChanged("struct S { void f(); int i; };\n"
                                          "void call_f(S& s) { (s.*(&S::f))(); }\n",
                                          "s .",
                                          "{ (",
                                          "}"));
    }

#define nextAfterAstRightmostLeaf(...) nextAfterAstRightmostLeaf_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool nextAfterAstRightmostLeaf_(const char* file, int line, const char (&code)[size], const char parentPattern[], const char rightPattern[]) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        const Token * tok = Token::findsimplematch(tokenizer.tokens(), parentPattern, strlen(parentPattern));
        return Token::simpleMatch((::nextAfterAstRightmostLeaf)(tok), rightPattern, strlen(rightPattern));
    }

    void nextAfterAstRightmostLeafTest() {
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("void f(int a, int b) { int x = a + b; }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a); }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a)[b]; }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(g(a)[b]); }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(g(a)[b] + a); }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a)[b + 1]; }", "=", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("void f() { int a; int b; int x = [](int a){}; }", "=", "; }"));

        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = a + b; }", "+", "; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a)[b + 1]; }", "+", "] ; }"));
        ASSERT_EQUALS(true, nextAfterAstRightmostLeaf("int * g(int); void f(int a, int b) { int x = g(a + 1)[b]; }", "+", ") ["));
    }

    enum class Result : std::uint8_t {False, True, Fail};

    template<size_t size>
    Result isUsedAsBool(const char (&code)[size], const char pattern[]) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        if (!tokenizer.tokenize(code))
            return Result::Fail;
        const Token * const argtok = Token::findmatch(tokenizer.tokens(), pattern);
        if (!argtok)
            return Result::Fail;
        return ::isUsedAsBool(argtok, settingsDefault) ? Result::True : Result::False;
    }

    void isUsedAsBool() {
        ASSERT(Result::True == isUsedAsBool("void f() { bool b = true; }", "b"));
        ASSERT(Result::False ==isUsedAsBool("void f() { int i = true; }", "i"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; while (i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; for (;i;) {} }", "i ; )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; for (;;i) {} }", "i )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; for (i;;) {} }", "i ; ; )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; for (int j=0; i; ++j) {} }", "i ; ++"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (i == 2) {} }", "i =="));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (i == true) {} }", "i =="));
        ASSERT(Result::False == isUsedAsBool("void f() { int i,j; if (i == (j&&f())) {} }", "i =="));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (!i == 0) {} }", "i =="));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (!i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (!!i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (i && f()) {} }", "i &&"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; int j = i && f(); }", "i &&"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (i & f()) {} }", "i &"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (static_cast<bool>(i)) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if ((bool)i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (1+static_cast<bool>(i)) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (1+(bool)i) {} }", "i )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (1+static_cast<int>(i)) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; if (1+!static_cast<int>(i)) {} }", "i )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (1+(int)i) {} }", "i )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int i; if (i + 2) {} }", "i +"));
        ASSERT(Result::True == isUsedAsBool("void f() { int i; bool b = i; }", "i ; }"));
        ASSERT(Result::True == isUsedAsBool("void f(bool b); void f() { int i; f(i); }","i )"));
        ASSERT(Result::False == isUsedAsBool("void f() { int *i; if (*i) {} }", "i )"));
        ASSERT(Result::True == isUsedAsBool("void f() { int *i; if (*i) {} }", "* i )"));
        ASSERT(Result::True == isUsedAsBool("int g(); void h(bool); void f() { h(g()); }", "( ) )"));
        ASSERT(Result::True == isUsedAsBool("int g(int); void h(bool); void f() { h(g(0)); }", "( 0 ) )"));
    }
};

REGISTER_TEST(TestAstUtils)
