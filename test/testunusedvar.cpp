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

#include "checkunusedvar.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "preprocessor.h"
#include "settings.h"

#include <cstddef>
#include <list>
#include <string>

#include <simplecpp.h>

class TestUnusedVar : public TestFixture {
public:
    TestUnusedVar() : TestFixture("TestUnusedVar") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::style).checkLibrary().library("std.cfg").build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(isRecordTypeWithoutSideEffects);
        TEST_CASE(cleanFunction);

        TEST_CASE(emptyclass);  // #5355 - False positive: Variable is not assigned a value.
        TEST_CASE(emptystruct);  // #5355 - False positive: Variable is not assigned a value.

        TEST_CASE(structmember1);
        TEST_CASE(structmember2);
        TEST_CASE(structmember3);
        TEST_CASE(structmember4);
        TEST_CASE(structmember5);
        TEST_CASE(structmember6);
        TEST_CASE(structmember7);
        TEST_CASE(structmember8);
        TEST_CASE(structmember9);  // #2017 - struct is inherited
        TEST_CASE(structmember_extern); // No false positives for extern structs
        TEST_CASE(structmember10);
        TEST_CASE(structmember11); // #4168 - initialization with {} / passed by address to unknown function
        TEST_CASE(structmember12); // #7179 - FP unused structmember
        TEST_CASE(structmember13); // #3088 - __attribute__((packed))
        TEST_CASE(structmember14); // #6508 - (struct x){1,2,..}
        TEST_CASE(structmember15); // #3088 - #pragma pack(1)
        TEST_CASE(structmember_sizeof);
        TEST_CASE(structmember16); // #10485
        TEST_CASE(structmember17); // #10591
        TEST_CASE(structmember18); // #10684
        TEST_CASE(structmember19); // #10826, #10848, #10852
        TEST_CASE(structmember20); // #10737
        TEST_CASE(structmember21); // #4759
        TEST_CASE(structmember22); // #11016
        TEST_CASE(structmember23);
        TEST_CASE(structmember24); // #10847
        TEST_CASE(structmember25);
        TEST_CASE(structmember26); // #13345
        TEST_CASE(structmember27); // #13367
        TEST_CASE(structmember28);
        TEST_CASE(structmember29); // #14075
        TEST_CASE(structmember30); // #14131
        TEST_CASE(structmember31); // #14130
        TEST_CASE(structmember32); // #14483
        TEST_CASE(structmember33);
        TEST_CASE(structmember_macro);
        TEST_CASE(structmember_template_argument); // #13887 - do not report that member used in template argument is unused
        TEST_CASE(classmember);
        TEST_CASE(structmemberStructuredBinding); // #13107

        TEST_CASE(localvar1);
        TEST_CASE(localvar2);
        TEST_CASE(localvar3);
        TEST_CASE(localvar4);
        TEST_CASE(localvar5);
        TEST_CASE(localvar6);
        TEST_CASE(localvar8);
        TEST_CASE(localvar9); // ticket #1605
        TEST_CASE(localvar10);
        TEST_CASE(localvar11);
        TEST_CASE(localvar12);
        TEST_CASE(localvar13); // ticket #1640
        TEST_CASE(localvar14); // ticket #5
        TEST_CASE(localvar15);
        TEST_CASE(localvar16); // ticket #1709
        TEST_CASE(localvar17); // ticket #1720
        TEST_CASE(localvar18); // ticket #1723
        TEST_CASE(localvar19); // ticket #1776
        TEST_CASE(localvar20); // ticket #1799
        TEST_CASE(localvar21); // ticket #1807
        TEST_CASE(localvar22); // ticket #1811
        TEST_CASE(localvar23); // ticket #1808
        TEST_CASE(localvar24); // ticket #1803
        TEST_CASE(localvar25); // ticket #1729
        TEST_CASE(localvar26); // ticket #1894
        TEST_CASE(localvar27); // ticket #2160
        TEST_CASE(localvar28); // ticket #2205
        TEST_CASE(localvar29); // ticket #2206 (array initialization)
        TEST_CASE(localvar30);
        TEST_CASE(localvar31); // ticket #2286
        TEST_CASE(localvar32); // ticket #2330
        TEST_CASE(localvar33); // ticket #2346
        TEST_CASE(localvar34); // ticket #2368
        TEST_CASE(localvar35); // ticket #2535
        TEST_CASE(localvar36); // ticket #2805
        TEST_CASE(localvar37); // ticket #3078
        TEST_CASE(localvar38);
        TEST_CASE(localvar39); // ticket #3454
        TEST_CASE(localvar40); // ticket #3473
        TEST_CASE(localvar41); // ticket #3603
        TEST_CASE(localvar42); // ticket #3742
        TEST_CASE(localvar43); // ticket #3602
        TEST_CASE(localvar44); // ticket #4020
        TEST_CASE(localvar45); // ticket #4899
        TEST_CASE(localvar46); // ticket #5491 (C++11 style initialization)
        TEST_CASE(localvar47); // ticket #6603
        TEST_CASE(localvar48); // ticket #6954
        TEST_CASE(localvar49); // ticket #7594
        TEST_CASE(localvar50); // ticket #6261 : dostuff(cond ? buf1 : buf2)
        TEST_CASE(localvar51); // ticket #8128 - FN : tok = tok->next();
        TEST_CASE(localvar52);
        TEST_CASE(localvar53); // continue
        TEST_CASE(localvar54); // ast, {}
        TEST_CASE(localvar55);
        TEST_CASE(localvar56);
        TEST_CASE(localvar57); // #8974 - increment
        TEST_CASE(localvar58); // #9901 - increment false positive
        TEST_CASE(localvar59); // #9737
        TEST_CASE(localvar60);
        TEST_CASE(localvar61); // #9407
        TEST_CASE(localvar62); // #10824
        TEST_CASE(localvar63); // #6928
        TEST_CASE(localvar64); // #9997
        TEST_CASE(localvar65); // #9876, #10006
        TEST_CASE(localvar66); // #11143
        TEST_CASE(localvar67); // #9946
        TEST_CASE(localvar68);
        TEST_CASE(localvar69);
        TEST_CASE(localvar70);
        TEST_CASE(localvar71);
        TEST_CASE(localvar72);
        TEST_CASE(localvarloops); // loops
        TEST_CASE(localvaralias1);
        TEST_CASE(localvaralias2); // ticket #1637
        TEST_CASE(localvaralias3); // ticket #1639
        TEST_CASE(localvaralias4); // ticket #1643
        TEST_CASE(localvaralias5); // ticket #1647
        TEST_CASE(localvaralias6); // ticket #1729
        TEST_CASE(localvaralias7); // ticket #1732
        TEST_CASE(localvaralias8);
        TEST_CASE(localvaralias9); // ticket #1996
        TEST_CASE(localvaralias10); // ticket #2004
        TEST_CASE(localvaralias11); // ticket #4423 - iterator
        TEST_CASE(localvaralias12); // ticket #4394
        TEST_CASE(localvaralias13); // ticket #4487
        TEST_CASE(localvaralias14); // ticket #5619
        TEST_CASE(localvaralias15); // ticket #6315
        TEST_CASE(localvaralias16);
        TEST_CASE(localvaralias17); // ticket #8911
        TEST_CASE(localvaralias18); // ticket #9234 - iterator
        TEST_CASE(localvaralias19); // ticket #9828
        TEST_CASE(localvaralias20); // ticket #10966
        TEST_CASE(localvaralias21);
        TEST_CASE(localvaralias22);
        TEST_CASE(localvaralias23);
        TEST_CASE(localvarasm);
        TEST_CASE(localvarstatic);
        TEST_CASE(localvarextern);
        TEST_CASE(localvardynamic1);
        TEST_CASE(localvardynamic2); // ticket #2904
        TEST_CASE(localvardynamic3); // ticket #3467
        TEST_CASE(localvararray1);  // ticket #2780
        TEST_CASE(localvararray2);  // ticket #3438
        TEST_CASE(localvararray3);  // ticket #3980
        TEST_CASE(localvararray4);  // ticket #4839
        TEST_CASE(localvararray5);  // ticket #7092
        TEST_CASE(localvararray6);
        TEST_CASE(localvarstring1);
        TEST_CASE(localvarstring2); // ticket #2929
        TEST_CASE(localvarconst1);
        TEST_CASE(localvarconst2);
        TEST_CASE(localvarreturn); // ticket #9167
        TEST_CASE(localvarmaybeunused);
        TEST_CASE(localvarrvalue); // ticket #13977

        TEST_CASE(localvarthrow); // ticket #3687

        TEST_CASE(localVarStd);
        TEST_CASE(localVarClass);
        TEST_CASE(localVarSmartPtr);

        // Don't give false positives for variables in structs/unions
        TEST_CASE(localvarStruct1);
        TEST_CASE(localvarStruct2);
        TEST_CASE(localvarStruct3);
        TEST_CASE(localvarStruct5);
        TEST_CASE(localvarStruct6);
        TEST_CASE(localvarStruct7);
        TEST_CASE(localvarStruct8);
        TEST_CASE(localvarStruct9);
        TEST_CASE(localvarStruct10);
        TEST_CASE(localvarStruct11); // 10095
        TEST_CASE(localvarStruct12); // #10495
        TEST_CASE(localvarStruct13); // #10398
        TEST_CASE(localvarStruct14);
        TEST_CASE(localvarStructArray);
        TEST_CASE(localvarUnion1);

        TEST_CASE(localvarOp);          // Usage with arithmetic operators
        TEST_CASE(localvarInvert);      // Usage with inverted variable
        TEST_CASE(localvarIf);          // Usage in if
        TEST_CASE(localvarIfElse);      // return tmp1 ? tmp2 : tmp3;
        TEST_CASE(localvarDeclaredInIf);
        TEST_CASE(localvarOpAssign);    // a |= b;
        TEST_CASE(localvarFor);         // for ( ; var; )
        TEST_CASE(localvarForEach);     // #4155 - BOOST_FOREACH, hlist_for_each, etc
        TEST_CASE(localvarShift1);      // 1 >> var
        TEST_CASE(localvarShift3);      // x << y
        TEST_CASE(localvarCast);
        TEST_CASE(localvarClass);
        TEST_CASE(localvarUnused);
        TEST_CASE(localvarFunction); // ticket #1799
        TEST_CASE(localvarIfNOT);    // #3104 - if ( NOT var )
        TEST_CASE(localvarAnd);      // #3672
        TEST_CASE(localvarSwitch);   // #3744 - false positive when localvar is used in switch
        TEST_CASE(localvarNULL);     // #4203 - Setting NULL value is not redundant - it is safe
        TEST_CASE(localvarUnusedGoto);    // #4447, #4558 goto
        TEST_CASE(localvarRangeBasedFor); // #7075
        TEST_CASE(localvarAssignInWhile);
        TEST_CASE(localvarTemplate); // #4955 - variable is used as template parameter
        TEST_CASE(localvarFuncPtr); // #7194
        TEST_CASE(localvarAddr); // #7477
        TEST_CASE(localvarDelete);
        TEST_CASE(localvarLambda); // #8941, #8948
        TEST_CASE(localvarStructuredBinding); // #10368

        TEST_CASE(localvarCppInitialization);
        TEST_CASE(localvarCpp11Initialization);

        TEST_CASE(chainedAssignment); // #5466

        TEST_CASE(crash1);
        TEST_CASE(crash2);
        TEST_CASE(crash3);
        TEST_CASE(usingNamespace);     // #4585

        TEST_CASE(lambdaFunction); // #5078
        TEST_CASE(namespaces); // #7557
        TEST_CASE(bracesInitCpp11);// #7895 - "int var{123}" initialization

        TEST_CASE(argument);
        TEST_CASE(argumentClass);
        TEST_CASE(escapeAlias); // #9150
        TEST_CASE(volatileData); // #9280
        TEST_CASE(globalData);

        TEST_CASE(structuredBinding); // #13269

        TEST_CASE(pointerCast1); // #14535
        TEST_CASE(pointerCast2);
    }

    struct FunctionVariableUsageOptions
    {
        bool cpp = true;
    };

#define functionVariableUsage(...) functionVariableUsage_(__FILE__, __LINE__, __VA_ARGS__)
    void functionVariableUsage_(const char* file, int line, const char code[], const FunctionVariableUsageOptions& options = make_default_obj()) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this, options.cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for unused variables..
        CheckUnusedVar checkUnusedVar(&tokenizer, &settings, this);
        checkUnusedVar.checkFunctionVariableUsage();
    }

    struct CheckStructMemberUsageOptions
    {
        const std::list<Directive>* directives = nullptr;
        bool cpp = true;
    };

#define checkStructMemberUsage(...) checkStructMemberUsage_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkStructMemberUsage_(const char* file, int line, const char (&code)[size], const CheckStructMemberUsageOptions& options = make_default_obj()) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this, options.cpp);
        if (options.directives)
            tokenizer.setDirectives(*options.directives);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for unused variables..
        CheckUnusedVar checkUnusedVar(&tokenizer, &settings, this);
        (checkUnusedVar.checkStructMemberUsage)();
    }

#define checkStructMemberUsageP(...) checkStructMemberUsageP_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkStructMemberUsageP_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer2 tokenizer(settings, *this, code, "test.cpp");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check for unused variables..
        CheckUnusedVar checkUnusedVar(&tokenizer, &settings, this);
        (checkUnusedVar.checkStructMemberUsage)();
    }

#define checkFunctionVariableUsageP(...) checkFunctionVariableUsageP_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkFunctionVariableUsageP_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer2 tokenizer(settings, *this, code, "test.cpp");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check for unused variables..
        CheckUnusedVar checkUnusedVar(&tokenizer, &settings, this);
        (checkUnusedVar.checkFunctionVariableUsage)();
    }


    void isRecordTypeWithoutSideEffects() {
        functionVariableUsage(
            "class A {};\n"
            "void f() {\n"
            "   A a;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (style) Unused variable: a [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "class A {};\n"
            "class B {\n"
            "public:\n"
            "   A a;\n"
            "};\n"
            "void f() {\n"
            "   B b;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7:6]: (style) Unused variable: b [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "class C {\n"
            "public:\n"
            "   C() = default;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:6:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "class D {\n"
            "public:\n"
            "   D() {}\n"
            "};\n"
            "void f() {\n"
            "   D d;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:6:6]: (style) Unused variable: d [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "class E {\n"
            "public:\n"
            "   uint32_t u{1};\n"
            "};\n"
            "void f() {\n"
            "   E e;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:6:6]: (style) Unused variable: e [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() : x(0) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7:6]: (style) Unused variable: f [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() : x{0} {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7:6]: (style) Unused variable: f [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "int y = 0;\n"
            "class F {\n"
            "public:\n"
            "   F() : x(y) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:8:6]: (style) Unused variable: f [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "int y = 0;"
            "class F {\n"
            "public:\n"
            "   F() : x(++y) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7:6]: (style) Unused variable: f [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "int y = 0;"
            "class F {\n"
            "public:\n"
            "   F() : x(--y) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7:6]: (style) Unused variable: f [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "int y = 0;"
            "class F {\n"
            "public:\n"
            "   F() : x(y+=1) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7:6]: (style) Unused variable: f [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "int y = 0;"
            "class F {\n"
            "public:\n"
            "   F() : x(y-=1) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:7:6]: (style) Unused variable: f [unusedVariable]\n", errout_str());

        // non-empty constructor
        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() {\n"
            "       int i = 0;\n"
            "       (void) i;"
            "   }\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        // constructor with hidden definition
        functionVariableUsage(
            "class B {\n"
            "public:\n"
            "   B();\n"
            "};\n"
            "class A {\n"
            "   B* b = new B;\n"
            "};\n"
            "int main() {\n"
            "   A a;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // side-effect variable
        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() {\n"
            "       int i = 0;\n"
            "       (void) i;"
            "   }\n"
            "};\n"
            "class G {\n"
            "public:\n"
            "   F f;\n"
            "};\n"
            "void f() {\n"
            "   G g;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // side-effect variable in initialization list
        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() {\n"
            "       int i = 0;\n"
            "       (void) i;"
            "   }\n"
            "};\n"
            "class G {\n"
            "public:\n"
            "   G() : f(F()) {}\n"
            "   F f;"
            "};\n"
            "void f() {\n"
            "   G g;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // unknown variable type
        functionVariableUsage(
            "class H {\n"
            "public:\n"
            "   unknown_type u{1};\n"
            "};\n"
            "void f() {\n"
            "   H h;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // unknown variable type in initialization list
        functionVariableUsage(
            "class H {\n"
            "public:\n"
            "   H() : x{0}, u(1) {}\n"
            "   int x;"
            "   unknown_type u;\n"
            "};\n"
            "void f() {\n"
            "   H h;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // unknown variable type used for initialization
        functionVariableUsage(
            "unknown_type y = 0;\n"
            "class F {\n"
            "public:\n"
            "   F() : x(y) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "int sideEffectFunc();\n"
            "class F {\n"
            "public:\n"
            "   F() : x(sideEffectFunc()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() : x(++unknownValue) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() : x(--unknownValue) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() : x(unknownValue+=1) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() : x(unknownValue-=1) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "struct S {\n"
            "    static void f() { std::cout << \"f()\"; }\n"
            "    ~S() { f(); }\n"
            "};\n"
            "void g() {\n"
            "    S s;\n"
            "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage( // #11109
            "class D { public: D(); };\n"
            "class E { public: ~E(); };\n"
            "class F {\n"
            "public:\n"
            "    F();\n"
            "    ~F();\n"
            "};\n"
            "void f() {\n"
            "    D d;\n"
            "    E e;\n"
            "    F f;\n"
            "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void cleanFunction() {
        // unknown function
        functionVariableUsage(
            "class F {\n"
            "public:\n"
            "   F() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   F f;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // function forward declaration
        functionVariableUsage(
            "int func();\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // return literal
        functionVariableUsage(
            "int func() { return 1; }\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:8:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // return variable without side effects
        functionVariableUsage(
            "int func() {\n"
            "   int x = 1;\n"
            "   return x;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:11:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // return variable with side effects
        functionVariableUsage(
            "int func() {\n"
            "   unknown_type x = 1;\n"
            "   return x;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // return unknown variable
        functionVariableUsage(
            "int func() {\n"
            "   return unknown_var;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // return variable is global, but not changed
        functionVariableUsage(
            "int x = 1;\n"
            "int func() {\n"
            "   return x;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:11:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // changing global variable in return
        functionVariableUsage(
            "int x = 1;\n"
            "int func() {\n"
            "   return x++;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // changing global variable in function body
        functionVariableUsage(
            "int x = 1;\n"
            "int func() {\n"
            "   x++;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "int x = 1;\n"
            "int func() {\n"
            "   --x;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "int x = 1;\n"
            "int func() {\n"
            "   x += 2;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "int x = 1;\n"
            "int func() {\n"
            "   x = 2;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // global variable use in function body without change
        functionVariableUsage(
            "int global = 1;\n"
            "int func() {\n"
            "   int x = global + 1;\n"
            "   return x;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:12:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // changing global array variable in function body
        functionVariableUsage(
            "int x[] = {0, 1, 3};\n"
            "int func() {\n"
            "   x[0] = 4;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "int x[] = {0, 1, 3};\n"
            "int func() {\n"
            "   *x = 2;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "int x[] = {0, 1, 3};\n"
            "int func() {\n"
            "   *(x) = 2;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // pointer arithmetic on global array
        functionVariableUsage(
            "int x[] = {0, 1, 3};\n"
            "int func() {\n"
            "   *(x + 1) = 2;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "int x[][] = {{0, 1}, {2, 3}};\n"
            "int func() {\n"
            "   *((x + 1) + 1) = 4;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "int x[] = {0, 1, 3};\n"
            "int func() {\n"
            "   int local = *(x + 1);\n"
            "   (void) local;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:13:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "int x[] = {0, 1, 3};\n"
            "int func() {\n"
            "   int* local = x + 2;\n"
            "   (void) local;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:13:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        functionVariableUsage(
            "int x[] = {0, 1, 3};\n"
            "int func() {\n"
            "   int* local = x + 2;\n"
            "   return *local;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "int x[] = {0, 1, 3};\n"
            "int func() {\n"
            "   return *(x + 1);\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // changing local variable
        functionVariableUsage(
            "int func() {\n"
            "   int x = 1;\n"
            "   x = 2;\n"
            "   x++;\n"
            "   return x;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:13:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // variable of user-defined class without side effects
        functionVariableUsage(
            "class A {};\n"
            "A func() {\n"
            "   A a;\n"
            "   return a;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   A x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:12:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // variable of user-defined class with side effects
        functionVariableUsage(
            "class A {\n"
            "public:\n"
            "   unknown_type u{1};\n"
            "};\n"
            "int func() {\n"
            "   A a;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // unknown type variable
        functionVariableUsage(
            "int func() {\n"
            "   unknown_type a;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // nested clean function call
        functionVariableUsage(
            "int another_func() { return 1;}\n"
            "int func() {\n"
            "   another_func();\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:12:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // nested side-effects function call
        functionVariableUsage(
            "int global = 1;"
            "int another_func() {\n"
            "   global++;\n"
            "   return global;}\n"
            "int func() {\n"
            "   another_func();\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // unknown nested function
        functionVariableUsage(
            "int func() {\n"
            "   unknown_func();\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // clean function recursion
        functionVariableUsage(
            "int func(int i) {\n"
            "   if (i != 2) {\n"
            "       func(i++);\n"
            "       return 2;\n"
            "   }\n"
            "   return i;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func(0)) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:14:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // indirect clean function recursion
        functionVariableUsage(
            "void another_func() {\n"
            "   func(0);\n"
            "}\n"
            "int func(int i) {\n"
            "   if (i != 2) {\n"
            "       another_func();\n"
            "       return 2;\n"
            "   }\n"
            "   return i;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func(0)) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:17:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // side-effect function recursion
        functionVariableUsage(
            "int global = 1;\n"
            "int func(int i) {\n"
            "   if (i != 2) {\n"
            "       global++;\n"
            "       func(i++);\n"
            "       return 2;\n"
            "   }\n"
            "   return i;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func(0)) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // multiple returns (side-effect & clean)
        functionVariableUsage(
            "int func(int i) {\n"
            "   if (i == 0) { return 0;}\n"
            "   else { return unknownSideEffectFunction(); }\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func(0)) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // multiple clean returns
        functionVariableUsage(
            "int func(int i) {\n"
            "   if (i == 0) { return 0;}\n"
            "   else { return i; }\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func(0)) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:11:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // multiple side-effect returns
        functionVariableUsage(
            "int func(int i) {\n"
            "   if (i == 0) { return unknownSideEffectFunction();}\n"
            "   else { return unknown_var; }\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func(0)) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // argument return
        functionVariableUsage(
            "int func(int i) {\n"
            "    return i;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func(0)) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:10:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // global variable modifying through function argument
        functionVariableUsage(
            "char buf[10];\n"
            "int func(char* p) {\n"
            "   *p = 0;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func(buf)) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // global variable modifying through local pointer
        functionVariableUsage(
            "int global = 1;\n"
            "int func() {\n"
            "   int* p = &global;\n"
            "   *p = 0;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // global variable assigning to local pointer, but not modifying
        functionVariableUsage(
            "int global = 1;\n"
            "int func() {\n"
            "   int* p = &global;\n"
            "   (void) p;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:13:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // global struct variable modification
        functionVariableUsage(
            "struct S { int x; } s;\n"
            "int func() {\n"
            "   s.x = 1;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // global struct variable without modification
        functionVariableUsage(
            "struct S { int x; } s;\n"
            "int func() {\n"
            "   int y = s.x + 1;\n"
            "   return y;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:12:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        // global pointer to struct variable modification
        functionVariableUsage(
            "struct S { int x; };\n"
            "struct S* s = new(struct S);\n"
            "int func() {\n"
            "   s->x = 1;\n"
            "   return 1;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // global pointer to struct variable without modification
        functionVariableUsage(
            "struct S { int x; };\n"
            "struct S* s = new(struct S);\n"
            "int func() {\n"
            "   int y = s->x + 1;\n"
            "   return y;\n"
            "}\n"
            "class C {\n"
            "public:\n"
            "   C() : x(func()) {}\n"
            "   int x;\n"
            "};\n"
            "void f() {\n"
            "   C c;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:13:6]: (style) Unused variable: c [unusedVariable]\n", errout_str());
    }

    // #5355 - False positive: Variable is not assigned a value.
    void emptyclass() {
        functionVariableUsage("class Carla {\n"
                              "};\n"
                              "class Fred : Carla {\n"
                              "};\n"
                              "void foo() {\n"
                              "    Fred fred;\n"
                              "    throw fred;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // #5355 - False positive: Variable is not assigned a value.
    void emptystruct() {
        functionVariableUsage("struct Fred {\n"
                              "};\n"
                              "void foo() {\n"
                              "    Fred fred;\n"
                              "    throw fred;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember1() {
        checkStructMemberUsage("struct abc\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "    int c;\n"
                               "};");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) struct member 'abc::a' is never used. [unusedStructMember]\n"
                      "[test.cpp:4:9]: (style) struct member 'abc::b' is never used. [unusedStructMember]\n"
                      "[test.cpp:5:9]: (style) struct member 'abc::c' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("union abc\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "    int c;\n"
                               "};");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) union member 'abc::a' is never used. [unusedStructMember]\n"
                      "[test.cpp:4:9]: (style) union member 'abc::b' is never used. [unusedStructMember]\n"
                      "[test.cpp:5:9]: (style) union member 'abc::c' is never used. [unusedStructMember]\n", errout_str());

        // #7458 - union with anonymous struct should not cause false positive
        checkStructMemberUsage("union DoubleInt {\n"
                               "    double asDouble;\n"
                               "    uint64_t asInt;\n"
                               "    struct {\n"
                               "        uint32_t lo, hi;\n" // <- no FP about lo because hi is used
                               "    } asIntel;\n"
                               "};\n"
                               "void f() {\n"
                               "    union DoubleInt di;\n"
                               "    di.asIntel.hi = 3;\n"
                               "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (style) union member 'DoubleInt::asDouble' is never used. [unusedStructMember]\n"
                      "[test.cpp:3:14]: (style) union member 'DoubleInt::asInt' is never used. [unusedStructMember]\n",
                      errout_str());
    }

    void structmember2() {
        checkStructMemberUsage("struct ABC\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "    int c;\n"
                               "};\n"
                               "\n"
                               "void foo()\n"
                               "{\n"
                               "    struct ABC abc;\n"
                               "    int a = abc.a;\n"
                               "    int b = abc.b;\n"
                               "    int c = abc.c;\n"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember3() {
        checkStructMemberUsage("struct ABC\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "    int c;\n"
                               "};\n"
                               "\n"
                               "static struct ABC abc[] = { {1, 2, 3} };\n"
                               "\n"
                               "void foo()\n"
                               "{\n"
                               "    int a = abc[0].a;\n"
                               "    int b = abc[0].b;\n"
                               "    int c = abc[0].c;\n"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember4() {
        checkStructMemberUsage("struct ABC\n"
                               "{\n"
                               "    const int a;\n"
                               "};\n"
                               "\n"
                               "void foo()\n"
                               "{\n"
                               "    ABC abc;\n"
                               "    if (abc.a == 2);\n"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember5() {
        checkStructMemberUsage("struct AB\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "    void reset()\n"
                               "    {\n"
                               "        a = 1;\n"
                               "        b = 2;\n"
                               "    }\n"
                               "};\n"
                               "\n"
                               "void foo()\n"
                               "{\n"
                               "    struct AB ab;\n"
                               "    ab.reset();\n"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember6() {
        checkStructMemberUsage("struct AB\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "};\n"
                               "\n"
                               "void foo(char *buf)\n"
                               "{\n"
                               "    struct AB *ab = (struct AB *)&buf[10];\n"
                               "}");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct AB\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "};\n"
                               "\n"
                               "void foo(char *buf)\n"
                               "{\n"
                               "    struct AB *ab = (AB *)&buf[10];\n"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember7() {
        checkStructMemberUsage("struct AB\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "};\n"
                               "\n"
                               "void foo(struct AB *ab)\n"
                               "{\n"
                               "    ab->a = 0;\n"
                               "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (style) struct member 'AB::b' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("struct AB\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "};\n"
                               "\n"
                               "void foo(struct AB _shuge *ab)\n"
                               "{\n"
                               "    ab->a = 0;\n"
                               "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (style) struct member 'AB::b' is never used. [unusedStructMember]\n", errout_str());
    }

    void structmember8() {
        checkStructMemberUsage("struct AB\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "};\n"
                               "\n"
                               "void foo(char *ab)\n"
                               "{\n"
                               "    ((AB *)ab)->b = 0;\n"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember9() {
        checkStructMemberUsage("struct base {\n"
                               "    int a;\n"
                               "};\n"
                               "\n"
                               "struct derived : public base {"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember10() {
        // Fred may have some useful side-effects
        checkStructMemberUsage("struct abc {\n"
                               "    Fred fred;\n"
                               "};");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember11() { // #4168
        checkStructMemberUsage("struct abc { int x; };\n"
                               "struct abc s = {0};\n"
                               "void f() { do_something(&s); }");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct abc { int x; };\n"
                               "struct abc s = {0};\n"
                               "void f() { }");
        TODO_ASSERT_EQUALS("abc::x is not used", "", errout_str());
    }

    void structmember12() { // #7179
        checkStructMemberUsage("#include <stdio.h>\n"
                               "struct\n"
                               "{\n"
                               "    union\n"
                               "    {\n"
                               "        struct\n"
                               "        {\n"
                               "            int a;\n"
                               "        } struct1;\n"
                               "    };\n"
                               "} var = {0};\n"
                               "int main(int argc, char *argv[])\n"
                               "{\n"
                               "    printf(\"var.struct1.a = %d\", var.struct1.a);\n"
                               "    return 1;\n"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember13() { // #3088 - struct members required by hardware
        checkStructMemberUsage("struct S {\n"
                               "  int x;\n"
                               "} __attribute__((packed));");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember14() { // #6508
        checkStructMemberUsage("struct bstr { char *bstart; size_t len; };\n"
                               "struct bstr bstr0(void) {\n"
                               "  return (struct bstr){\"hello\",6};\n"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember15() { // #3088
        std::list<Directive> directives;
        std::vector<std::string> f = { "test.cpp" };
        simplecpp::TokenList tokenList(f);
        simplecpp::Location loc;
        loc.line = 1;
        directives.emplace_back(tokenList, loc, "#pragma pack(1)");
        checkStructMemberUsage("\nstruct Foo { int x; int y; };", dinit(CheckStructMemberUsageOptions, $.directives = &directives));
        ASSERT_EQUALS("", errout_str());
    }

    void structmember_extern() {
        // extern struct => no false positive
        checkStructMemberUsage("extern struct AB\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "} ab;\n"
                               "\n"
                               "void foo()\n"
                               "{\n"
                               "    ab.b = 0;\n"
                               "}");
        ASSERT_EQUALS("", errout_str());

        // global linkage => no false positive
        checkStructMemberUsage("struct AB\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "} ab;\n"
                               "\n"
                               "void foo()\n"
                               "{\n"
                               "    ab.b = 0;\n"
                               "}");
        ASSERT_EQUALS("", errout_str());

        // static linkage => error message
        checkStructMemberUsage("static struct AB\n"
                               "{\n"
                               "    int a;\n"
                               "    int b;\n"
                               "} ab;\n"
                               "\n"
                               "void foo()\n"
                               "{\n"
                               "    ab.b = 0;\n"
                               "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) struct member 'AB::a' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("struct A\n"
                               "{\n"
                               "    static const int a = 0;\n"
                               "};\n"
                               "\n"
                               "int foo()\n"
                               "{\n"
                               "    return A::a;\n"
                               "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember_sizeof() {
        checkStructMemberUsage("struct Header {\n"
                               "  uint8_t message_type;\n"
                               "}\n"
                               "\n"
                               "input.skip(sizeof(Header));");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct Header {\n"
                               "  uint8_t message_type;\n"
                               "}\n"
                               "\n"
                               "input.skip(sizeof(struct Header));");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int a, b, c; };\n" // #6561
                               "int f(FILE * fp) {\n"
                               "    S s;\n"
                               "    ::fread(&s, sizeof(S), 1, fp);\n"
                               "    return s.b;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember16() {
        checkStructMemberUsage("struct S {\n"
                               "  static const int N = 128;\n" // <- used
                               "  char E[N];\n" // <- not used
                               "};\n");
        ASSERT_EQUALS("[test.cpp:3:8]: (style) struct member 'S::E' is never used. [unusedStructMember]\n", errout_str());
    }

    void structmember17() { // #10591
        checkStructMemberUsage("struct tagT { int i; };\n"
                               "void f() {\n"
                               "    struct tagT t{};\n"
                               "    t.i = 0;\n" // <- used
                               "    g(t);\n"
                               "};\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("typedef struct tagT { int i; } typeT;\n"
                               "void f() {\n"
                               "    struct typeT t{};\n"
                               "    t.i = 0;\n" // <- used
                               "    g(t);\n"
                               "};\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct T { int i; };\n"
                               "void f() {\n"
                               "    struct T t{};\n"
                               "    t.i = 0;\n" // <- used
                               "    g(t);\n"
                               "};\n");
        ASSERT_EQUALS("", errout_str()); // due to removeMacroInClassDef()
    }

    void structmember18() { // #10684
        checkStructMemberUsage("struct S { uint8_t padding[500]; };\n"
                               "static S s = { 0 };\n"
                               "uint8_t f() {\n"
                               "    uint8_t* p = (uint8_t*)&s;\n"
                               "    return p[10];\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:1:20]: (style) struct member 'S::padding' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("struct S { uint8_t padding[500]; };\n"
                               "uint8_t f(const S& s) {\n"
                               "    std::cout << &s;\n"
                               "    auto p = reinterpret_cast<const uint8_t*>(&s);\n"
                               "    return p[10];\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:1:20]: (style) struct member 'S::padding' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("struct S { int i, j; };\n" // #11577
                               "void f(S s) {\n"
                               "  void* p = (void*)&s;\n"
                               "  if (s.i) {}\n"
                               "}\n");
        ASSERT_EQUALS("[test.cpp:1:19]: (style) struct member 'S::j' is never used. [unusedStructMember]\n", errout_str());
    }

    void structmember19() {
        checkStructMemberUsage("class C {};\n" // #10826
                               "struct S {\n"
                               "    char* p;\n"
                               "    std::string str;\n"
                               "    C c;\n"
                               "};\n"
                               "void f(S* s) {}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) struct member 'S::p' is never used. [unusedStructMember]\n"
                      "[test.cpp:4:17]: (style) struct member 'S::str' is never used. [unusedStructMember]\n"
                      "[test.cpp:5:7]: (style) struct member 'S::c' is never used. [unusedStructMember]\n",
                      errout_str());

        checkStructMemberUsage("class C {};\n"
                               "struct S {\n"
                               "    char* p;\n"
                               "    std::string str;\n"
                               "    C c;\n"
                               "};\n"
                               "void f(S& s) {}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) struct member 'S::p' is never used. [unusedStructMember]\n"
                      "[test.cpp:4:17]: (style) struct member 'S::str' is never used. [unusedStructMember]\n"
                      "[test.cpp:5:7]: (style) struct member 'S::c' is never used. [unusedStructMember]\n",
                      errout_str());

        checkStructMemberUsage("struct S {\n" // #10848
                               "    struct T {\n"
                               "        int i;\n"
                               "    } t[2];\n"
                               "};\n"
                               "S s[1];\n"
                               "int f() {\n"
                               "    return s[0].t[1].i;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int a; };\n"
                               "struct T { S s; };\n"
                               "int f(const T** tp) {\n"
                               "    return tp[0]->s.a;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int a; };\n"
                               "int f(const S* sp) {\n"
                               "    return (*sp).a; \n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int a; };\n"
                               "int f(const S** spp) {\n"
                               "    return spp[0]->a;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int a; };\n"
                               "int f(const S** spp) {\n"
                               "    return spp[0][0].a;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int a; };\n"
                               "int f(const S* sp) {\n"
                               "    return sp[0].a;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int a; };\n"
                               "int f(const S* sp) {\n"
                               "    return sp->a;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("typedef struct { int i; } A;\n"
                               "typedef struct { std::vector<A> v; } B;\n"
                               "const A& f(const std::vector<const B*>& b, int idx) {\n"
                               "    const A& a = b[0]->v[idx];\n"
                               "    return a;\n"
                               "}\n");
        ASSERT_EQUALS("[test.cpp:1:22]: (style) struct member 'A::i' is never used. [unusedStructMember]\n",
                      errout_str());

        checkStructMemberUsage("struct A {\n" // #10852
                               "    struct B {\n"
                               "        int x;\n"
                               "    } b;\n"
                               "} a;\n"
                               "void f() {\n"
                               "    struct B* pb = &a.b;\n"
                               "    pb->x = 1;\n"
                               "}\n", dinit(CheckStructMemberUsageOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("union U {\n"
                               "    struct A {\n"
                               "        struct B {\n"
                               "            int x;\n"
                               "        } b;\n"
                               "    } a;\n"
                               "    struct C {\n"
                               "        short s[2];\n"
                               "    } c;\n"
                               "} u;\n"
                               "void f() {\n"
                               "    struct B* pb = &u.a.b;\n"
                               "    pb->x = 1;\n"
                               "    struct C* pc = &u.c;\n"
                               "    pc->s[0] = 1;\n"
                               "}\n", dinit(CheckStructMemberUsageOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
    }

    void structmember20() { // #10737
        checkStructMemberUsage("void f() {\n"
                               "    {\n"
                               "    }\n"
                               "    {\n"
                               "        struct S { int a; };\n"
                               "        S s{};\n"
                               "        {\n"
                               "            if (s.a) {}\n"
                               "        }\n"
                               "    }\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember21() { // #4759
        checkStructMemberUsage("class C {\n"
                               "public:\n"
                               "    int f() { return 0; }\n"
                               "};\n"
                               "C c;\n"
                               "int g() {\n"
                               "    return c.f();\n"
                               "}\n"
                               "struct S {\n"
                               "    int f;\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:10:9]: (style) struct member 'S::f' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("struct A { int i; };\n"
                               "struct B { struct A* pA; };");
        ASSERT_EQUALS("[test.cpp:1:16]: (style) struct member 'A::i' is never used. [unusedStructMember]\n"
                      "[test.cpp:2:22]: (style) struct member 'B::pA' is never used. [unusedStructMember]\n",
                      errout_str());
    }

    void structmember22() { // #11016
        checkStructMemberUsage("struct A { bool b; };\n"
                               "void f(const std::vector<A>& v) {\n"
                               "    std::vector<A>::const_iterator it = b.begin();\n"
                               "    if (it->b) {}\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember23() {
        checkStructMemberUsage("namespace N {\n"
                               "    struct S { std::string s; };\n"
                               "}\n"
                               "std::string f() {\n"
                               "    std::map<int, N::S> m = { { 0, { \"abc\" } } };\n"
                               "    return m[0].s;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember24() { // #10847
        checkStructMemberUsage("struct S { std::map<int, S*> m; };\n"
                               "std::map<int, S*> u;\n"
                               "std::map<int, S*>::iterator f() {\n"
                               "    return u.find(0)->second->m.begin();\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int i; };\n"
                               "void f() {\n"
                               "    std::map<int, S> m = { { 0, S() } };\n"
                               "    m[0].i = 1;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { bool b; };\n"
                               "std::vector<S> v;\n"
                               "bool f() {\n"
                               "    return v.begin()->b;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("int f(int s) {\n" // #10587
                               "    const struct S { int a, b; } Map[] = { { 0, 1 }, { 2, 3 } };\n"
                               "    auto it = std::find_if(std::begin(Map), std::end(Map), [&](const auto& m) { return s == m.a; });\n"
                               "    if (it != std::end(Map))\n"
                               "        return it->b;\n"
                               "    return 0;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("int f(int s) {\n"
                               "    const struct S { int a, b; } Map[] = { { 0, 1 }, { 2, 3 } };\n"
                               "    for (auto&& m : Map)\n"
                               "        if (m.a == s)\n"
                               "            return m.b;\n"
                               "    return 0;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct R { bool b{ false }; };\n" // #11539
                               "void f(std::optional<R> r) {\n"
                               "    if (r.has_value())\n"
                               "        std::cout << r->b;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember25() {
        checkStructMemberUsage("struct S {\n" // #12485
                               "    S* p;\n"
                               "    int i;\n"
                               "};\n"
                               "struct T {\n"
                               "    S s;\n"
                               "    int j;\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:2:8]: (style) struct member 'S::p' is never used. [unusedStructMember]\n"
                      "[test.cpp:3:9]: (style) struct member 'S::i' is never used. [unusedStructMember]\n"
                      "[test.cpp:6:7]: (style) struct member 'T::s' is never used. [unusedStructMember]\n"
                      "[test.cpp:7:9]: (style) struct member 'T::j' is never used. [unusedStructMember]\n",
                      errout_str());
    }

    void structmember26() { // #13345
        checkStructMemberUsage("struct foobar {\n"
                               "   char unused;\n"
                               "};\n"
                               "size_t offset_unused = offsetof(struct foobar, unused);\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember27() { // #13367
        checkStructMemberUsage("typedef struct pathNode_s {\n"
                               "    struct pathNode_s*  next;\n"
                               "} pathNode_t;\n"
                               "void f() {\n"
                               "    x<pathNode_t, offsetof( pathNode_t, next )> y;\n"
                               "}\n");
        ASSERT_EQUALS("", // don't crash
                      errout_str());
    }

    void structmember28() {
        checkStructMemberUsage("struct S {\n"
                               "    unsigned int a : 16;\n"
                               "    unsigned int : 16;\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:2:18]: (style) struct member 'S::a' is never used. [unusedStructMember]\n", errout_str());
    }

    void structmember29() { // #14075 - alignas false positive
        checkStructMemberUsage("struct S {\n"
                               "    static constexpr size_t cDataAlign = 8;\n"
                               "    static constexpr size_t cDataSize = 128;\n"
                               "    alignas(cDataAlign) std::array<uint8_t, cDataSize> storage{};\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:4:56]: (style) struct member 'S::storage' is never used. [unusedStructMember]\n", errout_str());
    }

    void structmember30() { // #14131
        checkStructMemberUsage("struct S\n"
                               "{\n"
                               "private:\n"
                               "    __attribute__((unused)) int i1{};\n"
                               "    int __attribute__((unused)) i2{}; // no warning\n"
                               "    int i3 __attribute__((unused)) {};\n"
                               "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember31() { // #14130
        checkStructMemberUsage("struct S\n"
                               "{\n"
                               "    [[maybe_unused]] int i1{};\n"
                               "    int i2 [[maybe_unused]] {};\n"
                               "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember32() { // #14483
        checkStructMemberUsage("struct S {\n"
                               "  int S::* mp;\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:2:12]: (style) struct member 'S::mp' is never used. [unusedStructMember]\n", errout_str());
    }

    void structmember33() {
        checkStructMemberUsage("struct S {\n"
                               "  int A::B<int>::C::* mp;\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:2:23]: (style) struct member 'S::mp' is never used. [unusedStructMember]\n", errout_str());
    }

    void structmember_macro() {
        checkStructMemberUsageP("#define S(n) struct n { int a, b, c; };\n"
                                "S(unused);\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmember_template_argument() { // #13887 - False positive
        checkStructMemberUsage("template <class T, int i> struct A{ T buf[i]; }\n"
                               "struct B {\n"
                               "    constexpr int x = 20;\n" // <- not unused
                               "    A<uint32_t, x> a;\n" // <- unused
                               "};");
        ASSERT_EQUALS("[test.cpp:4:20]: (style) struct member 'B::a' is never used. [unusedStructMember]\n", errout_str());
    }

    void classmember() {
        checkStructMemberUsage("class C {\n"
                               "    int i{};\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) class member 'C::i' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("class C {\n"
                               "    int i{}, j{};\n"
                               "public:\n"
                               "    int& get() { return i; }\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:2:14]: (style) class member 'C::j' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("class C {\n"
                               "private:\n"
                               "    int i;\n"
                               "};\n"
                               "class D : public C {};\n");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) class member 'C::i' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("class C {\n"
                               "public:\n"
                               "    int i;\n"
                               "};\n"
                               "class D : C {};\n");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) class member 'C::i' is never used. [unusedStructMember]\n", errout_str());

        checkStructMemberUsage("class C {\n"
                               "public:\n"
                               "    int i;\n"
                               "};\n"
                               "class D : public C {};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void structmemberStructuredBinding() { // #13107
        checkStructMemberUsage("struct S { int a, b; };\n"
                               "void f(S &s) {\n"
                               "    auto& [x, y] = s;\n"
                               "    x = y;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int a, b; };\n"
                               "struct T { S s; };\n"
                               "void f(T &t) {\n"
                               "    auto& [x, y] = t.s;\n"
                               "    x = y;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkStructMemberUsage("struct S { int a, b; };\n"
                               "void f(std::vector<S> &sv) {\n"
                               "    for (auto& [x, y] : sv)\n"
                               "        x = y;\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());

        // #13551 - don't crash
        checkStructMemberUsage("struct S {};\n"
                               "void f(vector<std::pair<int, int>> p) {\n"
                               "    for (auto [a, _] : p) {\n"
                               "        if (a == 0) {}\n"
                               "    }\n"
                               "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar1() {
        // extracttests.disable
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:11]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i(0);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        // if a is undefined then Cppcheck can't determine if "int i(a)" is a
        // * variable declaration
        // * function declaration
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i(a);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    int i(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    int & i = j;\n"
                              "    x(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    const int & i = j;\n"
                              "    x(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:19]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    int & i(j);\n"
                              "    x(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int j = 0;\n"
                              "    const int & i(j);\n"
                              "    x(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * j = Data;\n"
                              "    int * i(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * j = Data;\n"
                              "    const int * i(j);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    bool i = false;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:12]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    bool i = true;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:12]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *i;\n"
                              "    i = fgets();\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        // undefined variables are not reported because they may be classes with constructors
        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (information) --check-library: Provide <type-checks><unusedvar> configuration for undefined [checkLibraryCheckType]\n", errout_str());

        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i = 0;\n"
                              "}\n",
                              dinit(FunctionVariableUsageOptions, $.cpp = false));
        ASSERT_EQUALS(
            "[test.c:3:17]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = undefined;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:11]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * i = Data;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:13]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i = Data;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:14]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i = Data;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:20]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct S * i = DATA;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:18]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct S * i = DATA;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:24]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct S & i = j;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct S & i = j;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:24]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    undefined * i = X;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:19]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    int j = i;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:11]: (style) Variable 'j' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i[10] = { 0 };\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo(int n)\n"
                              "{\n"
                              "    int i[n] = { 0 };\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char i[10] = \"123456789\";\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *i = \"123456789\";\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:13]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:11]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,code=10;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,code=10,d=10;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        d = code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:24]: (style) Variable 'd' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:7:11]: (style) Variable 'd' is assigned a value that is never used. [unreadVariable]\n",
                      errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,code=10,d=10;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        g(d);\n"
                              "        d = code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,code=10,d=10;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        if (i == 3) {\n"
                              "            return d;\n"
                              "        }\n"
                              "        d = code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0,a=10,b=20;\n"
                              "    for(i = 0; i < 10; i++) {\n"
                              "        std::cout<<a<<std::endl;\n"
                              "        int tmp=a;\n"
                              "        a=b;\n"
                              "        b=tmp;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    while(code < 20) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    while(code < 20) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        d += code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (style) Variable 'd' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:7:11]: (style) Variable 'd' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    while(code < 20) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        g(d);\n"
                              "        d += code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    while(code < 20) {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        if (i == 3) {\n"
                              "            return d;\n"
                              "        }\n"
                              "        d += code;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a=10,b=20;\n"
                              "    while(a != 30) {\n"
                              "        std::cout<<a<<std::endl;\n"
                              "        int tmp=a;\n"
                              "        a=b;\n"
                              "        b=tmp;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    do {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    do {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        d += code;\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (style) Variable 'd' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:7:11]: (style) Variable 'd' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    do {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        g(d);\n"
                              "        d += code;\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10,d=10;\n"
                              "    do {\n"
                              "        std::cout<<code<<std::endl;\n"
                              "        code += 2;\n"
                              "        if (i == 3) {\n"
                              "            return d;\n"
                              "        }\n"
                              "        d += code;\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a=10,b=20;\n"
                              "    do {\n"
                              "        std::cout<<a<<std::endl;\n"
                              "        int tmp=a;\n"
                              "        a=b;\n"
                              "        b=tmp;\n"
                              "    } while( a!=30 );\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    for(int i=0; i < 10; i++) {\n"
                              "        if(true) {\n"
                              "            std::cout<<code<<std::endl;\n"
                              "            code += 2;\n"
                              "        }\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    for(int i=0; i < 10; i++) {\n"
                              "        if(true) {\n"
                              "            std::cout<<code<<std::endl;\n"
                              "        }\n"
                              "        code += 2;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    while(code < 20) {\n"
                              "        if(true) {\n"
                              "            std::cout<<code<<std::endl;\n"
                              "            code += 2;\n"
                              "        }\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int code=10;\n"
                              "    do {\n"
                              "        if(true) {\n"
                              "            std::cout<<code<<std::endl;\n"
                              "            code += 2;\n"
                              "        }\n"
                              "    } while(code < 20);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo(int j = 0) {\n" // #5985 - default function parameters should not affect checking results
                              "    int i = 0;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:11]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());
        // extracttests.enable
    }

    void localvar2() {
        // extracttests.disable: uninitialized variables and stuff

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Variable 'i' is not assigned a value. [unassignedVariable]\n", errout_str());

        functionVariableUsage("bool foo()\n"
                              "{\n"
                              "    bool i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Variable 'i' is not assigned a value. [unassignedVariable]\n", errout_str());

        // undefined variables are not reported because they may be classes with constructors
        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("undefined foo()\n"
                              "{\n"
                              "    undefined i;\n"
                              "    return i;\n"
                              "}\n",
                              dinit(FunctionVariableUsageOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:15]: (style) Variable 'i' is not assigned a value. [unassignedVariable]\n", errout_str());

        functionVariableUsage("undefined *foo()\n"
                              "{\n"
                              "    undefined * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Variable 'i' is not assigned a value. [unassignedVariable]\n", errout_str());

        functionVariableUsage("int *foo()\n"
                              "{\n"
                              "    int * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Variable 'i' is not assigned a value. [unassignedVariable]\n", errout_str());

        functionVariableUsage("const int *foo()\n"
                              "{\n"
                              "    const int * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Variable 'i' is not assigned a value. [unassignedVariable]\n", errout_str());

        functionVariableUsage("struct S *foo()\n"
                              "{\n"
                              "    struct S * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Variable 'i' is not assigned a value. [unassignedVariable]\n", errout_str());

        functionVariableUsage("const struct S *foo()\n"
                              "{\n"
                              "    const struct S * i;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:22]: (style) Variable 'i' is not assigned a value. [unassignedVariable]\n", errout_str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0], 0);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(0, a[0]);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // assume f() can write a
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(0, a[0], 0);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // f() can not write a (not supported yet)
        functionVariableUsage("void f(const int & i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'a' is not assigned a value.\n",
                           "", errout_str());

        // f() writes a
        functionVariableUsage("void f(int & i) { }\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a[0]);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(int * i);\n"
                              "void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    f(a+1);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
        // extracttests.enable
    }

    void localvar3() {
        functionVariableUsage("void foo(int abc)\n"
                              "{\n"
                              "    int i;\n"
                              "    if ( abc )\n"
                              "        ;\n"
                              "    else i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6:12]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar4() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    f(i);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    f(&i);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar5() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    b = (char)a;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar6() {
        functionVariableUsage("void foo() {\n"
                              "    int b[10];\n"
                              "    for (int i=0;i<10;++i)\n"
                              "        b[i] = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:14]: (style) Variable 'b[i]' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo() {\n"
                              "    int a = 0;\n"
                              "    int b[10];\n"
                              "    for (int i=0;i<10;++i)\n"
                              "        b[i] = ++a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:14]: (style) Variable 'b[i]' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo() {\n"
                              "    int b[10];\n"
                              "    for (int i=0;i<10;++i)\n"
                              "        *(b+i) = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Variable '*(b+i)' is assigned a value that is never used.\n", "", errout_str());

        functionVariableUsage("void f() {\n" // #11832, #11923
                              "    int b;\n"
                              "    *(&b) = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Variable '*(&b)' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar8() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i[2];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        // extracttests.start: struct A {int x;};

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    A * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct A * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct A * i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:22]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * i[2];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const int * i[2];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void * i[2];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const void * i[2];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct A * i[2];\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n", "", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    const struct A * i[2];\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n", "", errout_str());

        functionVariableUsage("void foo(int n)\n"
                              "{\n"
                              "    int i[n];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    int &j = i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Variable 'j' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:3:9]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
                      errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &j = i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Variable 'j' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    int &j = i;\n"
                              "    j = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'i' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("double foo()\n"
                              "{\n"
                              "    double i = 0.0;\n"
                              "    const double j = i;\n"
                              "    return j;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    A * i;\n"
                              "    i->f();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char * i;\n"
                              "    if (i);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char * i = 0;\n"
                              "    if (i);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char * i = new char[10];\n"
                              "    if (i);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *i;\n"
                              "    f(i);\n"
                              "}");

        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    return &a;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *p = a;\n"
                              "    for (int i = 0; i < 10; i++)\n"
                              "        p[i] = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *p = &a[0];\n"
                              "    for (int i = 0; i < 10; i++)\n"
                              "        p[i] = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int x;\n"
                              "    a[0] = 0;\n"
                              "    x = a[0];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6:7]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        // extracttests.start: int f();
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a, b, c;\n"
                              "    a = b = c = f();\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:4:11]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:4:15]: (style) Variable 'c' is assigned a value that is never used. [unreadVariable]\n",
                      errout_str());

        functionVariableUsage("int * foo()\n"
                              "{\n"
                              "    return &undefined[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar9() {
        // ticket #1605
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    for (int i = 0; i < 10; )\n"
                              "        a[i++] = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:16]: (style) Variable 'a[i++]' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar10() {
        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int i;\n"
                              "    if (x) {\n"
                              "        int i;\n"
                              "    } else {\n"
                              "        int i;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n"
                      "[test.cpp:5:13]: (style) Unused variable: i [unusedVariable]\n"
                      "[test.cpp:7:13]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int i;\n"
                              "    if (x)\n"
                              "        int i;\n"
                              "    else\n"
                              "        int i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n"
                      "[test.cpp:5:13]: (style) Unused variable: i [unusedVariable]\n"
                      "[test.cpp:7:13]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int i;\n"
                              "    if (x) {\n"
                              "        int i;\n"
                              "    } else {\n"
                              "        int i = 0;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:7:15]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n"
                      "[test.cpp:5:13]: (style) Unused variable: i [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int i;\n"
                              "    if (x) {\n"
                              "        int i;\n"
                              "    } else {\n"
                              "        int i;\n"
                              "    }\n"
                              "    i = 1;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:9:7]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:5:13]: (style) Unused variable: i [unusedVariable]\n"
                      "[test.cpp:7:13]: (style) Unused variable: i [unusedVariable]\n", errout_str());
    }

    void localvar11() {
        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    if (x == 1)\n"
                              "    {\n"
                              "        a = 123;\n"    // redundant assignment
                              "        return;\n"
                              "    }\n"
                              "    x = a;\n"  // redundant assignment
                              "}");
        ASSERT_EQUALS("[test.cpp:6:11]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:9:7]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        // The variable 'a' is initialized. But the initialized value is
        // never used. It is only initialized for security reasons.
        functionVariableUsage("void foo(int x)\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    if (x == 1)\n"
                              "        a = 123;\n"
                              "    else if (x == 2)\n"
                              "        a = 456;\n"
                              "    else\n"
                              "        return;\n"
                              "    x = a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:10:7]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar12() {
        // ticket #1574
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a, b, c, d, e, f;\n"
                              "    a = b = c = d = e = f = 15;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:7]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
            "[test.cpp:4:11]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n"
            "[test.cpp:4:15]: (style) Variable 'c' is assigned a value that is never used. [unreadVariable]\n"
            "[test.cpp:4:19]: (style) Variable 'd' is assigned a value that is never used. [unreadVariable]\n"
            "[test.cpp:4:23]: (style) Variable 'e' is assigned a value that is never used. [unreadVariable]\n"
            "[test.cpp:4:27]: (style) Variable 'f' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a, b, c = 0;\n"
                              "    a = b = c;\n"
                              "\n"
                              "}");

        TODO_ASSERT_EQUALS(
            "[test.cpp:4:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
            "[test.cpp:4:7]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
            "[test.cpp:3]: (style) Variable 'c' is assigned a value that is never used. [unreadVariable]\n",

            "[test.cpp:4:7]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
            "[test.cpp:4:11]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());
    }

    void localvar13() { // ticket #1640
        // extracttests.start: struct OBJECT { int ySize; };
        functionVariableUsage("void foo( OBJECT *obj )\n"
                              "{\n"
                              "    int x;\n"
                              "    x = obj->ySize / 8;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar14() {
        // ticket #5
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: a [unusedVariable]\n", errout_str());
    }

    void localvar15() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    int b[a];\n"
                              "    b[0] = 0;\n"
                              "    return b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    int * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return *b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int * foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    const int * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct B * foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    struct B * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("const struct B * foo()\n"
                              "{\n"
                              "    int a = 5;\n"
                              "    const struct B * b[a];\n"
                              "    b[0] = &c;\n"
                              "    return b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar16() { // ticket #1709
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[5];\n"
                              "    char *ptr = buf;\n"
                              "    *(ptr++) = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'buf' is assigned a value that is never used.\n", "", errout_str());

        // #3910
        functionVariableUsage("void foo() {\n"
                              "    char buf[5];\n"
                              "    char *data[2];\n"
                              "    data[0] = buf;\n"
                              "    do_something(data);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo() {\n"
                              "    char buf1[5];\n"
                              "    char buf2[5];\n"
                              "    char *data[2];\n"
                              "    data[0] = buf1;\n"
                              "    data[1] = buf2;\n"
                              "    do_something(data);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar17() { // ticket #1720
        // extracttests.disable
        // Don't crash when checking the code below!
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct DATA *data = DATA;\n"
                              "    char *k = data->req;\n"
                              "    char *ptr;\n"
                              "    char *line_start;\n"
                              "    ptr = data->buffer;\n"
                              "    line_start = ptr;\n"
                              "    data->info = k;\n"
                              "    line_start = ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:10:16]: (style) Variable 'line_start' is assigned a value that is never used. [unreadVariable]\n", errout_str());
        // extracttests.enable
    }

    void localvar18() { // ticket #1723
        functionVariableUsage("A::A(int iValue) {\n"
                              "    UserDefinedException* pe = new UserDefinedException();\n"
                              "    throw pe;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar19() { // ticket #1776
        functionVariableUsage("void foo() {\n"
                              "    int a[10];\n"
                              "    int c;\n"
                              "    c = *(a);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Variable 'c' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:2:9]: (style) Variable 'a' is not assigned a value. [unassignedVariable]\n", errout_str());
    }

    void localvar20() { // ticket #1799
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char c1 = 'c';\n"
                              "    char c2[] = { c1 };\n"
                              "    a(c2);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar21() { // ticket #1807
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buffer[1024];\n"
                              "    bar((void *)buffer);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar22() { // ticket #1811
        functionVariableUsage("int foo(int u, int v)\n"
                              "{\n"
                              "    int h, i;\n"
                              "    h = 0 ? u : v;\n"
                              "    i = 1 ? u : v;\n"
                              "    return h + i;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar23() { // ticket #1808
        functionVariableUsage("int foo(int c)\n"
                              "{\n"
                              "    int a;\n"
                              "    int b[10];\n"
                              "    a = b[c] = 0;\n"
                              "    return a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:14]: (style) Variable 'b[c]' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar24() { // ticket #1803
        functionVariableUsage("class MyException\n"
                              "{\n"
                              "    virtual void raise() const\n"
                              "    {\n"
                              "        throw *this;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar25() { // ticket #1729
        functionVariableUsage("int main() {\n"
                              "    int ppos = 1;\n"
                              "    int pneg = 0;\n"
                              "    const char*edge = ppos? \" +\" : pneg ? \" -\" : \"\";\n"
                              "    printf(\"This should be a '+' -> %s\\n\", edge);\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar26() { // ticket #1894
        functionVariableUsage("int main() {\n"
                              "    const Fred &fred = getfred();\n"
                              "    int *p = fred.x();\n"
                              "    *p = 0;"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar27() { // ticket #2160
        functionVariableUsage("void f(struct s *ptr) {\n"
                              "    int param = 1;\n"
                              "    ptr->param = param++;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:23]: (style) Variable 'param' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar28() { // ticket #2205
        functionVariableUsage("void f(char* buffer, int value) {\n"
                              "    char* pos = buffer;\n"
                              "    int size = value;\n"
                              "    *(int*)pos = size;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar29() { // ticket #2206
        functionVariableUsage("void f() {\n"
                              "    float s_ranges[] = { 0, 256 };\n"
                              "    float* ranges[] = { s_ranges };\n"
                              "    cout << ranges[0][0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar30() { // ticket #2264
        functionVariableUsage("void f() {\n"
                              "    Engine *engine = e;\n"
                              "    x->engine = engine->clone();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar31() { // ticket #2286
        functionVariableUsage("void f() {\n"
                              "    int x = 0;\n"
                              "    a.x = x - b;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar32() {
        // ticket #2330 - fstream >> x
        functionVariableUsage("void f() {\n"
                              "    int x;\n"
                              "    fstream &f = getfile();\n"
                              "    f >> x;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #4596 - if (c >>= x) {}
        functionVariableUsage("void f(int x) {\n"
                              "    C c;\n" // possibly some stream class
                              "    if (c >>= x) {}\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (information) --check-library: Provide <type-checks><unusedvar> configuration for C [checkLibraryCheckType]\n", errout_str());

        functionVariableUsage("void f(int x) {\n"
                              "    C c;\n"
                              "    if (c >>= x) {}\n"
                              "}", dinit(FunctionVariableUsageOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:11]: (style) Variable 'c' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void f() {\n"
                              "    int x, y;\n"
                              "    std::cin >> x >> y;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #8494
        functionVariableUsage("void f(C c) {\n"
                              "  int x;\n"
                              "  c & x;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar33() { // ticket #2345
        functionVariableUsage("void f() {\n"
                              "    Abc* abc = getabc();\n"
                              "    while (0 != (abc = abc->next())) {\n"
                              "        ++nOldNum;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar34() { // ticket #2368
        functionVariableUsage("void f() {\n"
                              "    int i = 0;\n"
                              "    if (false) {\n"
                              "    } else {\n"
                              "        j -= i;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar35() { // ticket #2535
        functionVariableUsage("void f() {\n"
                              "    int a, b;\n"
                              "    x(1,a,b);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar36() { // ticket #2805
        functionVariableUsage("int f() {\n"
                              "    int a, b;\n"
                              "    a = 2 * (b = 3);\n"
                              "    return a + b;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int f() {\n" // ticket #4318
                              "    int a,b;\n"
                              "    x(a, b=2);\n"  // <- if param2 is passed-by-reference then b might be used in x
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo() {\n" // ticket #6147
                              "    int a = 0;\n"
                              "    bar(a=a+2);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo() {\n" // ticket #6147
                              "    int a = 0;\n"
                              "    bar(a=2);\n"
                              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        functionVariableUsage("void bar(int);\n"
                              "int foo() {\n"
                              "    int a = 0;\n"
                              "    bar(a=a+2);\n"
                              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

    void localvar37() { // ticket #3078
        functionVariableUsage("void f() {\n"
                              "    int a = 2;\n"
                              "    ints.at(a) = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar38() {
        functionVariableUsage("std::string f() {\n"
                              "    const char code[] = \"foo\";\n"
                              "    const std::string s1(sizeof_(code));\n"
                              "    const std::string s2 = sizeof_(code);\n"
                              "    return(s1+s2);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar39() {
        functionVariableUsage("void f() {\n"
                              "    int a = 1;\n"
                              "    foo(x*a);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar40() {
        functionVariableUsage("int f() {\n"
                              "    int a = 1;\n"
                              "    return x & a;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar41() {
        // #3603 - false positive 'x is assigned a value that is never used'
        functionVariableUsage("int f() {\n"
                              "    int x = 1;\n"
                              "    int y = FOO::VALUE * x;\n"
                              "    return y;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar42() { // #3742
        functionVariableUsage("float g_float = 1;\n"
                              "extern void SomeTestFunc(float);\n"
                              "void MyFuncError()\n"
                              "{\n"
                              "    const float floatA = 2.2f;\n"
                              "    const float floatTot = g_float * floatA;\n"
                              "    SomeTestFunc(floatTot);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("float g_float = 1;\n"
                              "extern void SomeTestFunc(float);\n"
                              "void MyFuncNoError()\n"
                              "{\n"
                              "    const float floatB = 2.2f;\n"
                              "    const float floatTot = floatB * g_float;\n"
                              "    SomeTestFunc(floatTot);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("float g_float = 1;\n"
                              "extern void SomeTestFunc(float);\n"
                              "void MyFuncNoError2()\n"
                              "{\n"
                              "    const float floatC = 2.2f;\n"
                              "    float floatTot = g_float * floatC;\n"
                              "    SomeTestFunc(floatTot);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar43() { // ticket #3602 (false positive)
        functionVariableUsage("void bar()\n"
                              "{\n"
                              "    int * piArray = NULL;\n"
                              "    unsigned int uiArrayLength = 2048;\n"
                              "    unsigned int uiIndex;\n"
                              "\n"
                              "    try\n"
                              "    {\n"
                              "        piArray = new int[uiArrayLength];\n" // Allocate memory
                              "    }\n"
                              "    catch (...)\n"
                              "    {\n"
                              "        SOME_MACRO\n"
                              "        delete [] piArray;\n"
                              "        return;\n"
                              "    }\n"
                              "    for (uiIndex = 0; uiIndex < uiArrayLength; uiIndex++)\n"
                              "    {\n"
                              "        piArray[uiIndex] = -1234;\n"
                              "    }\n"
                              "    delete [] piArray;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int f() {\n" // #9877
                              "    const std::vector<int> x = get();\n"
                              "    MACRO(2U, x.size())\n"
                              "    int i = 0;\n"
                              "    return i;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar44() { // #4020 - FP
        functionVariableUsage("void func() {\n"
                              "    int *sp_mem[2] = { global1, global2 };\n"
                              "    sp_mem[0][3] = 123;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar45() { // #4899 - FP
        functionVariableUsage("int func() {\n"
                              "    int a = 123;\n"
                              "    int b = (short)-a;;\n"
                              "    return b;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar46() { // #5491/#5494/#6301
        functionVariableUsage("int func() {\n"
                              "    int i = 0;\n"
                              "    int j{i};\n"
                              "    return j;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(bool b, bool c, double& r) {\n"
                              "    double d{};\n"
                              "    if (b) {\n"
                              "      d = g();\n"
                              "      r += d;\n"
                              "    }\n"
                              "    if (c) {\n"
                              "      d = h();\n"
                              "      r += d;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int func() {\n"
                              "    std::mutex m;\n"
                              "    std::unique_lock<std::mutex> l{ m };\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int func() {\n"
                              "    std::shared_lock<std::shared_timed_mutex> lock( m );\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n" // #10490
                              "    std::shared_lock lock = GetLock();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n"
                              "    auto&& g = std::lock_guard<std::mutex> { mutex };\n"
                              "}\n");
        TODO_ASSERT_EQUALS("", "[test.cpp:2:14]: (style) Variable 'g' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void f() {\n"
                              "    auto a = RAII();\n"
                              "    auto b { RAII() };\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct RAIIWrapper {\n" // #10894
                              "    RAIIWrapper();\n"
                              "    ~RAIIWrapper();\n"
                              "};\n"
                              "static void foo() {\n"
                              "    auto const guard = RAIIWrapper();\n"
                              "    auto const& guard2 = RAIIWrapper();\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar47() { // #6603
        // extracttests.disable
        functionVariableUsage("void f() {\n"
                              "    int (SfxUndoManager::*retrieveCount)(bool) const\n"
                              "        = (flag) ? &SfxUndoManager::foo : &SfxUndoManager::bar;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Variable 'retrieveCount' is assigned a value that is never used. [unreadVariable]\n", errout_str());
        // extracttests.enable
    }

    void localvar48() { // #6954
        functionVariableUsage("void foo() {\n"
                              "  long (*pKoeff)[256] = new long[9][256];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar49() { // #7594
        functionVariableUsage("class A {\n"
                              "    public:\n"
                              "        typedef enum { ID1,ID2,ID3 } Id_t;\n"
                              "        typedef struct {Id_t id; std::string a; } x_t;\n"
                              "        std::vector<x_t> m_vec;\n"
                              "        std::vector<x_t> Get(void);\n"
                              "        void DoSomething();\n"
                              "};\n"
                              "std::vector<A::x_t> A::Get(void) {\n"
                              "    return m_vec;\n"
                              "}\n"
                              "const std::string Bar() {\n"
                              "    return \"x\";\n"
                              "}\n"
                              "void A::DoSomething(void) {\n"
                              "    const std::string x = Bar();\n"  // <- warning
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:16:25]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());
    }

    void localvar50() { // #6261, #6542
        // #6261 - ternary operator in function call
        functionVariableUsage("void foo() {\n"
                              "  char buf1[10];\n"
                              "  dostuff(cond?buf1:buf2);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo() {\n"
                              "  char buf1[10];\n"
                              "  dostuff(cond?buf2:buf1);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // #6542 - ternary operator
        functionVariableUsage("void foo(int c) {\n"
                              "  char buf1[10], buf2[10];\n"
                              "  char *p = c ? buf1 : buf2;\n"
                              "  dostuff(p);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar51() { // #8128 FN
        // extracttests.start: struct Token { const Token* next() const; }; const Token* nameToken();
        functionVariableUsage("void foo(const Token *var) {\n"
                              "  const Token *tok = nameToken();\n"
                              "  tok = tok->next();\n"  // read+write
                              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) Variable 'tok' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo() {\n"
                              "  int x = 4;\n"
                              "  x = 15 + x;\n"  // read+write
                              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar52() {
        functionVariableUsage("void foo() {\n"
                              "  std::vector<int> data;\n"
                              "  data[2] = 32;\n"
                              "  return data;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar53() {
        functionVariableUsage("void foo(int a, int loop) {\n"
                              "  bool x = false;\n"
                              "  while (loop) {\n"
                              "    if (a) {\n"
                              "      x = true;\n" // unused value
                              "      continue;\n"
                              "    }\n"
                              "  }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo(int a, int loop) {\n"
                              "  bool x = false;\n"
                              "  while (loop) {\n"
                              "    if (a) {\n"
                              "      x = true;\n"
                              "      continue;\n"
                              "    }\n"
                              "  }\n"
                              "  return x;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar54() {
        functionVariableUsage("Padding fun() {\n"
                              "  Distance d = DISTANCE;\n"
                              "  return (Padding){ d, d, d, d };\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar55() {
        functionVariableUsage("void f(int mode) {\n"
                              "    int x = 0;\n" // <- redundant assignment
                              "\n"
                              "    for (int i = 0; i < 10; i++) {\n"
                              "        if (mode == 0x04)\n"
                              "            x = 0;\n" // <- redundant assignment
                              "        if (mode == 0x0f) {\n"
                              "            x = address;\n"
                              "            data[x] = 0;\n"
                              "        }\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:6:15]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n",
                      errout_str());
    }

    void localvar56() {
        functionVariableUsage("void f()\n"
                              "{\n"
                              "    int x = 31;\n"
                              "    mask[x] |= 123;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar57() {
        functionVariableUsage("void f()\n"
                              "{\n"
                              "    int x = 0;\n"
                              "    x++;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar58() { // #9901 - increment false positive
        functionVariableUsage("void f() {\n"
                              "    int x = 0;\n"
                              "    if (--x > 0) {}\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n"
                              "    int x = 0;\n"
                              "    if (x-- > 0) {}\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar59() { // #9737
        functionVariableUsage("Response foo() {\n"
                              "    const std::vector<char> cmanifest = z;\n"
                              "    return {.a = cmanifest, .b =0};\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar60() {
        functionVariableUsage("void Scale(double scale) {\n" // #10531
                              "    for (int i = 0; i < m_points.size(); ++i) {\n"
                              "        auto& p = m_points[i];\n"
                              "        p += scale;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo(int c[]) {\n" // #10597
                              "    int& cc = c[0];\n"
                              "    cc &= ~0xff;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar61() { // #9407
        functionVariableUsage("void g(int& i);\n"
                              "void f() {\n"
                              "    int var = 0;\n"
                              "    g(var);\n"
                              "    var = 2;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:5:9]: (style) Variable 'var' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar62() {
        functionVariableUsage("void f() {\n" // #10824
                              "    S* s = nullptr;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2:8]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void f() {\n"
                              "    S* s{};\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("int f() {\n"
                              "    int i = 0, j = 1;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:18]: (style) Variable 'j' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("int f() {\n"
                              "    int i = 0, j = 1;\n"
                              "    return j;\n"
                              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:11]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void f() {\n" // #10846
                              "    int i = 1; while (i) { i = g(); }\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar63() { // #6928
        functionVariableUsage("void f(void) {\n"
                              "  int x=3;\n"             // <- set but not used
                              "  goto y;\n"
                              "  y:return;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar64() { // #9997
        functionVariableUsage("class S {\n"
                              "    ~S();\n"
                              "    S* f();\n"
                              "    S* g(int);\n"
                              "};\n"
                              "void h(S* s, bool b) {\n"
                              "    S* p = nullptr;\n"
                              "    S* q = nullptr;\n"
                              "    if (b) {\n"
                              "        p = s;\n"
                              "        q = s->f()->g(-2);\n"
                              "    }\n"
                              "    else\n"
                              "        q = s;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:10:11]: (style) Variable 'p' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:11:11]: (style) Variable 'q' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:14:11]: (style) Variable 'q' is assigned a value that is never used. [unreadVariable]\n",
                      errout_str());
    }

    void localvar65() {
        functionVariableUsage("bool b();\n" // #9876
                              "void f() {\n"
                              "    for (;;) {\n"
                              "        const T* t = tok->next()->link()->next();\n"
                              "        if (!b())\n"
                              "            continue;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4:18]: (style) Variable 't' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void f() {\n" // #10006
                              "    std::string s = \"\";\n"
                              "    try {}\n"
                              "    catch (...) {}\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvar66() { // #11143
        functionVariableUsage("void f() {\n"
                              "    double phi = 42.0;\n"
                              "    std::cout << pow(sin(phi), 2) + pow(cos(phi), 2) << std::endl;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar67() { // #9946
        functionVariableUsage("struct B {\n"
                              "    virtual ~B() {}\n"
                              "    bool operator() () const { return true; }\n"
                              "    virtual bool f() const = 0;\n"
                              "};\n"
                              "class D : B {\n"
                              "public:\n"
                              "    bool f() const override { return false; }\n"
                              "};\n"
                              "void f1() {\n"
                              "    const D d1;\n"
                              "    d1.f();\n"
                              "}\n"
                              "void f2() {\n"
                              "    const D d2;\n"
                              "    d2();\n"
                              "        B() {}\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar68() {
        checkFunctionVariableUsageP("#define X0 int x = 0\n"
                                    "void f() { X0; }\n");
        ASSERT_EQUALS("", errout_str());

        checkFunctionVariableUsageP("#define X0 int (*x)(int) = 0\n"
                                    "void f() { X0; }\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar69() {
        functionVariableUsage("int g();\n" // #11063
                              "int h(int);\n"
                              "int f() {\n"
                              "    int i = g();\n"
                              "    return (::h)(i);\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar70() {
        functionVariableUsage("struct S { int i = 0; };\n" // #12176
                              "void f(S s) {\n"
                              "    S s1;\n"
                              "    if (s == s1) {}\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar71() {
        functionVariableUsage("struct A { explicit A(int i); };\n" // #12363
                              "void f() { A a(0); }\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvar72() {
        functionVariableUsage("void f()\n"
                              "{\n"
                              "  struct S {};\n"
                              "  int S::* mp;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Unused variable: mp [unusedVariable]\n", errout_str());
    }

    void localvarloops() {
        // loops
        functionVariableUsage("void fun(int c) {\n"
                              "  int x;\n"
                              "  while (c) { x=10; }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void dostuff(int x);\n"
                              "void fun(int y, int c) {\n"
                              "  int x = 1;\n"
                              "  while (c) {\n"
                              "    dostuff(x);\n"
                              "    if (y) { x=10; break; }\n"
                              "  }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6:15]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void dostuff(int &x);\n"
                              "void fun() {\n"
                              "  int x = 1;\n"
                              "  while (c) {\n"
                              "    dostuff(x);\n"
                              "    if (y) { x=10; break; }\n"
                              "  }\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:6:15]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void fun() {\n"
                              "  int x = 0;\n"
                              "  while (c) {\n"
                              "    dostuff(x);\n"
                              "    x = 10;\n"
                              "  }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void fun() {\n"
                              "  int x = 0;\n"
                              "  while (x < 10) { x = x + 1; }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void fun()\n"
                              "{\n"
                              "    int   status = 0;\n"
                              "    for (ind = 0; ((ind < nrArgs) && (status < 10)); ind++)\n"
                              "        status = x;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f()\n"
                              "{\n"
                              "    int sum = 0U;\n"
                              "    for (i = 0U; i < 2U; i++)\n"
                              "        sum += 123;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Variable 'sum' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:5:13]: (style) Variable 'sum' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void f(int c) {\n" // #7908
                              "    int b = 0;\n"
                              "    while (g()) {\n"
                              "        int a = c;\n"
                              "        b = a;\n"
                              "        if (a == 4)\n"
                              "            a = 5;\n"
                              "    }\n"
                              "    h(b);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:7:15]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void f(const std::vector<int>& v) {\n"
                              "    while (g()) {\n"
                              "        const std::vector<int>& v2 = h();\n"
                              "        if (std::vector<int>{ 1, 2, 3 }.size() > v2.size()) {}\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(const std::vector<int>& v) {\n"
                              "    while (g()) {\n"
                              "        const std::vector<int>& v2 = h();\n"
                              "        if (std::vector<int>({ 1, 2, 3 }).size() > v2.size()) {}\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(const std::string &c) {\n"
                              "    std::string s = str();\n"
                              "    if (s[0] == '>')\n"
                              "        s[0] = '<';\n"
                              "    if (s == c) {}\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(bool b) {\n"
                              "    std::map<std::string, std::vector<std::string>> m;\n"
                              "    if (b) {\n"
                              "        const std::string n = g();\n"
                              "        std::vector<std::string> c = h();\n"
                              "        m[n] = c;\n"
                              "    }\n"
                              "    j(m);\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct S { int i; };\n"
                              "S f(S s, bool b) {\n"
                              "    if (b)\n"
                              "        s.i = 1;\n"
                              "    return s;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    int *b = &a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:3:9]: (style) Unused variable: a [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:3:9]: (style) Unused variable: a [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = (char *)&a;\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = (char *)(&a);\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = (const char *)&a;\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = (const char *)(&a);\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    char *b = static_cast<char *>(&a);\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a;\n"
                              "    const char *b = static_cast<const char *>(&a);\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        // a is not a local variable and b is aliased to it
        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:12]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        // a is not a local variable and b is aliased to it
        functionVariableUsage("void foo(int a)\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:12]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        // a is not a local variable and b is aliased to it
        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a;\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = &a;\n"
                              "    }\n"
                              "};");
        ASSERT_EQUALS(
            "[test.cpp:6:16]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("int a;\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo(int a)\n"
                              "{\n"
                              "    int *b = &a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a;\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = &a;\n"
                              "        *b = 0;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = (char *)a;\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = (char *)(a);\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = (const char *)a;\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = (const char *)(a);\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    char *b = static_cast<char *>(a);\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    const char *b = static_cast<const char *>(a);\n"
                              "    *b = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: int a[10];
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int c = b[0];\n"
                              "    x(c);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    int c = b[0];\n"
                              "    x(c);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int *b = &a[0];\n"
                              "    a[0] = b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = &a[0];\n"
                              "    a[0] = b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    a[0] = b[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo(int a[10])\n"
                              "{\n"
                              "    int *b = a;\n"
                              "    *b = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("class A\n"
                              "{\n"
                              "    int a[10];\n"
                              "    void foo()\n"
                              "    {\n"
                              "        int *b = a;\n"
                              "        *b = 0;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int *b = a;\n"
                              "    int *c = b;\n"
                              "    *c = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'a' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    int *d = b;\n"
                              "    *d = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (style) Variable 'c' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:3:9]: (style) Unused variable: a [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    c = b;\n"
                              "    *c = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Unused variable: a [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10];\n"
                              "    int b[10];\n"
                              "    int *c = a;\n"
                              "    c = b;\n"
                              "    *c = 0;\n"
                              "    c = a;\n"
                              "    *c = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (style) Variable 'a' is assigned a value that is never used.\n"
                           "[test.cpp:7]: (style) Variable 'b' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    b[-10] = 1;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Variable 'b[-10]' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    b[-10] = 0;\n"
                              "    int * c = b - 10;\n"
                              "}");
        TODO_ASSERT_EQUALS(
            "[test.cpp:4:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
            "[test.cpp:5:13]: (style) Variable 'c' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    c[1] = 3;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'c[1]' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a[10], * b = a + 10;\n"
                              "    int * c = b - 10;\n"
                              "    c[1] = c[0];\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'c[1]' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo() {\n" // #4022 - FP (a is assigned a value that is never used)
                              "    int a[2], *b[2];\n"
                              "    a[0] = 123;\n"
                              "    b[0] = &a[0];\n"
                              "    int *d = b[0];\n"
                              "    return *d;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo() {\n" // #4022 - FP (a is assigned a value that is never used)
                              "    entry a[2], *b[2];\n"
                              "    a[0].value = 123;\n"
                              "    b[0] = &a[0];\n"
                              "    int d = b[0].value;\n"
                              "    return d;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    struct S * s = (struct S *)a;\n"
                              "    s->c[0] = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    struct S * s = (struct S *)a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:18]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:4:10]: (style) Unused variable: a [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    const struct S * s = (const struct S *)a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:24]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:4:10]: (style) Unused variable: a [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    struct S * s = static_cast<struct S *>(a);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:18]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:4:10]: (style) Unused variable: a [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("struct S { char c[100]; };\n"
                              "void foo()\n"
                              "{\n"
                              "    char a[100];\n"
                              "    const struct S * s = static_cast<const struct S *>(a);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:24]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:4:10]: (style) Unused variable: a [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int b[10];\n"
                              "    int c[10];\n"
                              "    int *d;\n"
                              "    d = b;\n"
                              "    d = a;\n"
                              "    d = c;\n"
                              "    *d = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (style) Unused variable: b [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("int a[10];\n"
                              "void foo()\n"
                              "{\n"
                              "    int b[10];\n"
                              "    int c[10];\n"
                              "    int *d;\n"
                              "    d = b; *d = 0;\n"
                              "    d = a; *d = 0;\n"
                              "    d = c; *d = 0;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (style) Variable 'b' is assigned a value that is never used.\n"
                           "[test.cpp:9]: (style) Variable 'c' is assigned a value that is never used.\n",
                           "",
                           errout_str());
    }

    void localvaralias2() { // ticket 1637
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int * a;\n"
                              "    x(a);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias3() { // ticket 1639
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    BROWSEINFO    info;\n"
                              "    char          szDisplayName[MAX_PATH];\n"
                              "    info.pszDisplayName = szDisplayName;\n"
                              "    SHBrowseForFolder(&info);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias4() { // ticket 1643
        functionVariableUsage("struct AB { int a; int b; } ab;\n"
                              "void foo()\n"
                              "{\n"
                              "    int * a = &ab.a;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("struct AB { int a; int b; } ab;\n"
                              "void foo()\n"
                              "{\n"
                              "    int * a = &ab.a;\n"
                              "    *a = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct AB { int a; int b; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct AB ab;\n"
                              "    int * a = &ab.a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:4:15]: (style) Variable 'ab' is not assigned a value. [unassignedVariable]\n",
                      errout_str());

        functionVariableUsage("struct AB { int a; int b; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct AB ab;\n"
                              "    int * a = &ab.a;\n"
                              "    *a = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias5() { // ticket 1647
        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[0];\n"
                              "    *p++ = 0;\n"
                              "    return buf[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[1];\n"
                              "    *p-- = 0;\n"
                              "    return buf[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[0];\n"
                              "    *++p = 0;\n"
                              "    return buf[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("char foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *p = &buf[1];\n"
                              "    *--p = 0;\n"
                              "    return buf[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias6() { // ticket 1729
        // extracttests.start: int a(); void b(const char *);
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "    } else {\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    b(srcdata);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'buf' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "    }\n"
                              "    srcdata = vdata;\n"
                              "    b(srcdata);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (style) Variable 'buf' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo(char *vdata)\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        srcdata = buf;\n"
                              "    }\n"
                              "    srcdata = vdata;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Unused variable: buf [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    if (a()) {\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    srcdata = buf;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "    } else {\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    b(srcdata);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (style) Variable 'buf' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        buf[0] = 1;\n"
                              "        srcdata = buf;\n"
                              "    }\n"
                              "    srcdata = vdata;\n"
                              "    b(srcdata);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (style) Variable 'buf' is assigned a value that is never used.\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        srcdata = buf;\n"
                              "    }\n"
                              "    srcdata = vdata;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Unused variable: buf [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char buf[8];\n"
                              "    char *srcdata;\n"
                              "    char vdata[8];\n"
                              "    if (a()) {\n"
                              "        srcdata = vdata;\n"
                              "    }\n"
                              "    srcdata = buf;\n"
                              "    b(srcdata);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (style) Unused variable: vdata [unusedVariable]\n", errout_str());
    }

    void localvaralias7() { // ticket 1732
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char *c[10];\n"
                              "    char **cp;\n"
                              "    cp = c;\n"
                              "    *cp = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias8() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else if (a == 2)\n"
                              "        pb = b2;\n"
                              "    else if (a == 3)\n"
                              "        pb = b3;\n"
                              "    else\n"
                              "        pb = b4;\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else if (a == 2)\n"
                              "        pb = b2;\n"
                              "    else if (a == 3)\n"
                              "        pb = b3;\n"
                              "    else {\n"
                              "        pb = b1;\n"
                              "        pb = b4;\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else if (a == 2)\n"
                              "        pb = b2;\n"
                              "    else if (a == 3)\n"
                              "        pb = b3;\n"
                              "    else {\n"
                              "        pb = b1;\n"
                              "        pb = b2;\n"
                              "        pb = b3;\n"
                              "        pb = b4;\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());


        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else if (a == 2)\n"
                              "        pb = b2;\n"
                              "    else if (a == 3)\n"
                              "        pb = b3;\n"
                              "    pb = b4;\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Unused variable: b1 [unusedVariable]\n"
                      "[test.cpp:4:10]: (style) Unused variable: b2 [unusedVariable]\n"
                      "[test.cpp:5:10]: (style) Unused variable: b3 [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else {\n"
                              "        if (a == 2)\n"
                              "            pb = b2;\n"
                              "        else {\n"
                              "            if (a == 3)\n"
                              "                pb = b3;\n"
                              "            else\n"
                              "                pb = b4;\n"
                              "        }\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else {\n"
                              "        if (a == 2)\n"
                              "            pb = b2;\n"
                              "        else {\n"
                              "            if (a == 3)\n"
                              "                pb = b3;\n"
                              "            else {\n"
                              "                pb = b1;\n"
                              "                pb = b4;\n"
                              "            }\n"
                              "        }\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else {\n"
                              "        if (a == 2)\n"
                              "            pb = b2;\n"
                              "        else {\n"
                              "            if (a == 3)\n"
                              "                pb = b3;\n"
                              "            else {\n"
                              "                pb = b1;\n"
                              "                pb = b2;\n"
                              "                pb = b3;\n"
                              "                pb = b4;\n"
                              "            }\n"
                              "        }\n"
                              "    }\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char b1[8];\n"
                              "    char b2[8];\n"
                              "    char b3[8];\n"
                              "    char b4[8];\n"
                              "    char *pb;\n"
                              "    if (a == 1)\n"
                              "        pb = b1;\n"
                              "    else {\n"
                              "        if (a == 2)\n"
                              "            pb = b2;\n"
                              "        else {\n"
                              "            if (a == 3)\n"
                              "                pb = b3;\n"
                              "        }\n"
                              "    }\n"
                              "    pb = b4;\n"
                              "    b(pb);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (style) Unused variable: b1 [unusedVariable]\n"
                      "[test.cpp:4:10]: (style) Unused variable: b2 [unusedVariable]\n"
                      "[test.cpp:5:10]: (style) Unused variable: b3 [unusedVariable]\n", errout_str());
    }

    void localvaralias9() { // ticket 1996
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    Foo foo;\n"
                              "    Foo &ref = foo;\n"
                              "    ref[0] = 123;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias10() { // ticket 2004
        functionVariableUsage("void foo(Foo &foo)\n"
                              "{\n"
                              "    Foo &ref = foo;\n"
                              "    int *x = &ref.x();\n"
                              "    *x = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias11() { // #4423 - iterator
        functionVariableUsage("void f(Foo &foo) {\n"
                              "    std::set<int>::iterator x = foo.dostuff();\n"
                              "    *(x) = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias12() { // #4394
        functionVariableUsage("void f(void) {\n"
                              "    int a[4];\n"
                              "    int *b = (int*)((int*)a+1);\n"
                              "    x(b);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int f(void) {\n" // #4628
                              "    int x=1,y;\n"
                              "    y = (x * a) / 100;\n"
                              "    return y;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias13() { // #4487
        functionVariableUsage("void f(char *p) {\n"
                              "    char a[4];\n"
                              "    p = a;\n"
                              "    strcpy(p, \"x\");\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(char *p) {\n"
                              "    char a[4];\n"
                              "    p = a;\n"
                              "    strcpy(p, \"x\");\n"
                              "}");
        TODO_ASSERT_EQUALS("a is assigned value that is never used", "", errout_str());
    }

    void localvaralias14() { // #5619
        functionVariableUsage("char * dostuff(char *p);\n"
                              "void f() {\n"
                              "    char a[4], *p=a;\n"
                              "    p = dostuff(p);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Variable 'p' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("char * dostuff(char *&p);\n"
                              "void f() {\n"
                              "    char a[4], *p=a;\n"
                              "    p = dostuff(p);\n"
                              "}");
        ASSERT_EQUALS("", errout_str()); // TODO: we can warn in this special case; variable is local and there are no function calls after the assignment

        functionVariableUsage("void f() {\n"
                              "    char a[4], *p=a;\n"
                              "    p = dostuff(p);\n"
                              "}");
        ASSERT_EQUALS("", errout_str()); // TODO: we can warn in this special case; variable is local and there are no function calls after the assignment
    }

    void localvaralias15() { // #6315
        functionVariableUsage("void f() {\n"
                              "  int x=3;\n"
                              "  int *p = &x;\n"
                              "  int *p2[1] = {p};\n"
                              "  dostuff(p2);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias16() {
        functionVariableUsage("void f() {\n"
                              "  auto x = dostuff();\n"
                              "  p = x;\n"
                              "  x->data[0] = 9;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias17() {
        functionVariableUsage("void f() {\n"
                              "  int x;\n"
                              "  unknown_type p = &x;\n"
                              "  *p = 9;\n"
                              "}", dinit(FunctionVariableUsageOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias18() {
        functionVariableUsage("void add( std::vector< std::pair< int, double > >& v)\n"
                              "{\n"
                              "    std::vector< std::pair< int, double > >::iterator it;\n"
                              "    for ( it = v.begin(); it != v.end(); ++it )\n"
                              "    {\n"
                              "        if ( x )\n"
                              "        {\n"
                              "            ( *it ).second = 0;\n"
                              "            break;\n"
                              "        }\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(const std::vector<int>& v) {\n" // #13303
                              "    std::vector<int>::const_iterator it = v.cbegin();\n"
                              "    if (*it == 0)\n"
                              "        it = v.cend();\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4:12]: (style) Variable 'it' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvaralias19() { // #9828
        functionVariableUsage("void f() {\n"
                              "    bool b0{}, b1{};\n"
                              "    struct {\n"
                              "        bool* pb;\n"
                              "        int val;\n"
                              "    } Map[] = { {&b0, 0}, {&b1, 1} };\n"
                              "    b0 = true;\n"
                              "    for (auto & m : Map)\n"
                              "        if (m.pb && *m.pb)\n"
                              "            m.val = 1;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias20() { // #10966
        functionVariableUsage("struct A {};\n"
                              "A g();\n"
                              "void f() {\n"
                              "    const auto& a = g();\n"
                              "    const auto& b = a;\n"
                              "    const auto&& c = g();\n"
                              "    auto&& d = c;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:5:19]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:7:14]: (style) Variable 'd' is assigned a value that is never used. [unreadVariable]\n",
                      errout_str());
    }

    void localvaralias21() { // #11728
        functionVariableUsage("void f(int i) {\n"
                              "    bool b = true;\n"
                              "    bool* p = &b;\n"
                              "    int j{};\n"
                              "    if (i)\n"
                              "        b = false;\n"
                              "    if (*p) {}\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Variable 'j' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvaralias22() { // #11139
        functionVariableUsage("int f() {\n"
                              "    int x[1], *p = x;\n"
                              "    x[0] = 42;\n"
                              "    return *p;\n"
                              "}\n"
                              "int g() {\n"
                              "    int x[1], *p{ x };\n"
                              "    x[0] = 42;\n"
                              "    return *p;\n"
                              "}\n"
                              "int h() {\n"
                              "    int x[1], *p(x);\n"
                              "    x[0] = 42;\n"
                              "    return *p;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvaralias23() { // #11817
        functionVariableUsage("void f(int& r, bool a, bool b) {\n"
                              "    int& e = r;\n"
                              "    if (a)\n"
                              "        e = 42;\n"
                              "    else if (b)\n"
                              "        e = 1;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarasm() {

        functionVariableUsage("void foo(int &b)\n"
                              "{\n"
                              "    int a;\n"
                              "    asm();\n"
                              "    b = a;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStruct1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static const struct{ int x, y, w, h; } bounds = {1,2,3,4};\n"
                              "    return bounds.x + bounds.y + bounds.w + bounds.h;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStruct2() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct ABC { int a, b, c; };\n"
                              "    struct ABC abc = { 1, 2, 3 };\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:20]: (style) Variable 'abc' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());
    }

    void localvarStruct3() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 10;\n"
                              "    union { struct { unsigned char x; }; unsigned char z; };\n"
                              "    do {\n"
                              "        func();\n"
                              "    } while(a--);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (style) Unused variable: x\n"
                           "[test.cpp:4]: (style) Unused variable: z\n", "", errout_str());
    }

    void localvarStruct5() {
        // extracttests.disable
        functionVariableUsage("int foo() {\n"
                              "    A a;\n"
                              "    return a.i;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}\n",
                              dinit(FunctionVariableUsageOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:2:7]: (style) Unused variable: a [unusedVariable]\n", errout_str());
        // extracttests.enable

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return a.i;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    a.i = 0;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (style) Variable 'a.i' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a = { 0 };\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:9]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        // extracttests.disable
        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a = { 0 };\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:9]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        // extracttests.enable
        functionVariableUsage("class A { int i; public: A(); { } };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) Unused variable: a [unusedVariable]\n", errout_str());

        functionVariableUsage("class A { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) Unused variable: a [unusedVariable]\n", errout_str());

        functionVariableUsage("class A { int i; public: A(); { } };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("class A { unknown i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("class A : public Fred { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("class Fred {char c;};\n"
                              "class A : public Fred { int i; };\n"
                              "int foo() {\n"
                              "    A a;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:7]: (style) Unused variable: a [unusedVariable]\n", errout_str());
    }

    void localvarStruct6() {
        functionVariableUsage("class Type { };\n"
                              "class A {\n"
                              "public:\n"
                              "    Type & get() { return t; }\n"
                              "private:\n"
                              "    Type t;\n"
                              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStruct7() {
        functionVariableUsage("struct IMAPARG {\n"
                              "  void *text;\n"
                              "};\n"
                              "\n"
                              "void fun() {\n"
                              "  IMAPARG *args, aatt;\n"
                              "  args = &aatt;\n"
                              "  aatt.text = tmp;\n"
                              "  dostuff(args);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: void dostuff(int*);
        functionVariableUsage("struct ARG {\n"
                              "  int a;\n"
                              "  int b;\n"
                              "};\n"
                              "\n"
                              "void fun() {\n"
                              "  ARG aatt;\n"
                              "  int *p = &aatt.b;\n"
                              "  aatt.a = 123;\n"
                              "  dostuff(p);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:9:10]: (style) Variable 'aatt.a' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("struct AB {\n"
                              "  int a;\n"
                              "  int b;\n"
                              "};\n"
                              "\n"
                              "void fun() {\n"
                              "  AB ab;\n"
                              "  int &a = ab.a;\n"
                              "  ab.a = 123;\n"
                              "  dostuff(a);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStruct8() {
        functionVariableUsage("struct s {\n"
                              "     union {\n"
                              "         struct {\n"
                              "             int fld1 : 16;\n"
                              "             int fld2 : 16;\n"
                              "         };\n"
                              "         int raw;\n"
                              "     };\n"
                              "};\n"
                              "\n"
                              "void foo() {\n"
                              "      struct s test;\n"
                              "      test.raw = 0x100;\n"
                              "      dostuff(test.fld1, test.fld2);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStruct9() {
        functionVariableUsage("struct XY { int x; int y; };\n"
                              "\n"
                              "void foo() {\n"
                              "      struct XY xy(get());\n"
                              "      return xy.x + xy.y;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStruct10() { // #6766
        functionVariableUsage("struct S { int x; };\n"
                              "\n"
                              "void foo(const struct S s2) {\n"
                              "    struct S s;\n"
                              "    s.x = 3;\n"
                              "    memcpy (&s, &s2, sizeof (S));\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:9]: (style) Variable 's.x' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvarStruct11() { // #10095
        functionVariableUsage("struct Point { int x; int y; };\n"
                              "Point scale(Point *p);\n"
                              "\n"
                              "int foo() {\n"
                              "    Point p;\n"
                              "    p.x = 42;\n"
                              "    return scale(&p).y;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStruct12() { // #10495
        functionVariableUsage("struct S { bool& Ref(); };\n"
                              "\n"
                              "void Set() {\n"
                              "    S s;\n"
                              "    s.Ref() = true;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStruct13() { // #10398
        functionVariableUsage("int f() {\n"
                              "    std::vector<std::string> Mode;\n"
                              "    Info Block = {\n"
                              "        {\n"
                              "            { &Mode  },\n"
                              "            { &Level }\n"
                              "        }\n"
                              "    };\n"
                              "    Mode.resize(N);\n"
                              "    for (int i = 0; i < N; ++i)\n"
                              "        Mode[i] = \"abc\";\n"
                              "    return Save(&Block);\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStruct14() { // #12142
        functionVariableUsage("struct S { int i; };\n"
                              "int f() {\n"
                              "    S s;\n"
                              "    int S::* p = &S::i;\n"
                              "    s.*p = 123;\n"
                              "    return s.i;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarStructArray() {
        // extracttests.start: struct X {int a;};

        // #3633 - detect that struct array is assigned a value
        functionVariableUsage("void f() {\n"
                              "    struct X x[10];\n"
                              "    x[0].a = 5;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (style) Variable 'x[0].a' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvarUnion1() {
        // #9707
        functionVariableUsage("static short read(FILE *fp) {\n"
                              "    typedef union { short s; unsigned char c[2]; } u;\n"
                              "    u x;\n"
                              "    x.c[0] = fgetuc(fp);\n"
                              "    x.c[1] = fgetuc(fp);\n"
                              "    return x.s;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarOp() {
        constexpr char op[] = "+-*/%&|^";
        for (const char *p = op; *p; ++p) {
            std::string code("int main()\n"
                             "{\n"
                             "    int tmp = 10;\n"
                             "    return 123 " + std::string(1, *p) + " tmp;\n"
                             "}");
            functionVariableUsage(code.c_str());
            ASSERT_EQUALS("", errout_str());
        }
    }

    void localvarInvert() {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    return ~tmp;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarIf() {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    if ( tmp )\n"
                              "        return 1;\n"
                              "    return 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("bool argsMatch(const Token *first, const Token *second) {\n" // #6145
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
        ASSERT_EQUALS("[test.cpp:5:15]: (style) Variable 'first' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvarIfElse() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int tmp1 = 1;\n"
                              "    int tmp2 = 2;\n"
                              "    int tmp3 = 3;\n"
                              "    return tmp1 ? tmp2 : tmp3;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarDeclaredInIf() {
        functionVariableUsage("int foo(int x)\n"
                              "{\n"
                              "    if (int y = x % 2)\n"
                              "        return 2;\n"
                              "    else\n"
                              "        return 1;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:15]: (style) Variable 'y' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("int foo(int x)\n"
                              "{\n"
                              "    if (int y = x % 2)\n"
                              "        return y;\n"
                              "    else\n"
                              "        return 1;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo(int x)\n"
                              "{\n"
                              "    if (int y = x % 2)\n"
                              "        return 2;\n"
                              "    else\n"
                              "        return y;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int f(int i) {\n" // #11788
                              "    if (int x = i) {\n"
                              "        return x;\n"
                              "    }\n"
                              "    else {\n"
                              "        x = 12;\n"
                              "        return x;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(int i) {\n"
                              "    if (int x = i) {\n"
                              "        while (x < 100) {\n"
                              "            if (x % 2 == 0) {\n"
                              "                x += 3;\n"
                              "            }\n"
                              "            else if (x % 3 == 0) {\n"
                              "                x += 5;\n"
                              "            }\n"
                              "            else {\n"
                              "                x += 7;\n"
                              "            }\n"
                              "            x += 6;\n"
                              "        }\n"
                              "        return x;\n"
                              "    }\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarOpAssign() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    int b = 2;\n"
                              "    a |= b;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:5:7]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo() {\n"
                              "    int a = 1;\n"
                              "    (b).x += a;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo() {\n"
                              "    int a=1, b[10];\n"
                              "    b[0] = x;\n"
                              "    a += b[0];\n"
                              "    return a;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(int *start, int *stop) {\n"
                              "  int length = *start - *stop;\n"
                              "  if (length < 10000)\n"
                              "    length = 10000;\n"
                              "  *stop -= length;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(int a) {\n"
                              "  int x = 3;\n"
                              "  a &= ~x;\n"
                              "  return a;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.disable
        functionVariableUsage("void f() {\n"  // unknown class => library configuration is needed
                              "  Fred fred;\n"
                              "  int *a; a = b;\n"
                              "  fred += a;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:8]: (information) --check-library: Provide <type-checks><unusedvar> configuration for Fred [checkLibraryCheckType]\n", errout_str());
        // extracttests.enable

        functionVariableUsage("void f(std::pair<int,int> x) {\n"
                              "  std::pair<int,int> fred;\n"  // class with library configuration
                              "  fred = x;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:8]: (style) Variable 'fred' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvarFor() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    for (;a;);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo() {\n"
                              "    for (int i = 0; (pci = cdi_list_get(pciDevices, i)); i++)\n"
                              "    {}\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(const int* b, int x) {\n" // #11125
                              "    int a[6];\n"
                              "    int i = 0;\n"
                              "    for (int j = 0; j < 6; ++j) {\n"
                              "        if (b[j] != 0) {\n"
                              "            a[i] = j;\n"
                              "            ++i;\n"
                              "        }\n"
                              "    }\n"
                              "    if (i > 1) {\n"
                              "        a[i] = a[0];\n"
                              "        (void)a[x];\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarForEach() { // #4155 - foreach
        functionVariableUsage("void foo() {\n"
                              "    int i = -1;\n"
                              "    int a[] = {1,2,3};\n"
                              "    FOREACH_X (int x, a) {\n"
                              "        if (i==x) return x;\n"
                              "        i = x;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // #5154 - MSVC 'for each'
        functionVariableUsage("void f() {\n"
                              "  std::map<int,int> ints;\n"
                              "  ints[0]= 1;\n"
                              "  for each(std::pair<int,int> i in ints) { x += i.first; }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarShift1() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int var = 1;\n"
                              "    return 1 >> var;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarShift3() {  // #3509
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    QList<int *> ints;\n"
                              "    ints << 1;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo() {\n" // #4320
                              "    int x;\n"
                              "    x << 1;\n"
                              "    return x;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarCast() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    int b = static_cast<int>(a);\n"
                              "    return b;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarClass() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    class B : public A {\n"
                              "        int a;\n"
                              "        int f() { return a; }\n"
                              "    } b;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarUnused() {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool test __attribute__((unused));\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool test __attribute__((unused)) = true;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool __attribute__((unused)) test;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool __attribute__((unused)) test = true;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool test __attribute__((used));\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    bool __attribute__((used)) test;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    char a[1] __attribute__((unused));\n"
                              "    char b[1][2] __attribute__((unused));\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarFunction() {
        functionVariableUsage("void check_dlsym(void*& h)\n"
                              "{\n"
                              "  typedef void (*function_type) (void);\n"
                              "  function_type fn;\n"
                              "  fn = reinterpret_cast<function_type>(dlsym(h, \"try_allocation\"));\n"
                              "  fn();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarstatic() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i;\n"
                              "    static const int ci;\n"
                              "    static std::string s;\n"
                              "    static const std::string cs;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Unused variable: i [unusedVariable]\n"
                      "[test.cpp:4:22]: (style) Unused variable: ci [unusedVariable]\n"
                      "[test.cpp:5:24]: (style) Unused variable: s [unusedVariable]\n"
                      "[test.cpp:6:30]: (style) Unused variable: cs [unusedVariable]\n",
                      errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i = 0;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
                      errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i(0);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3:11]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int j = 0;\n"
                              "    static int i(j);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4:10]: (style) Variable 'i' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("int * foo(int x)\n"
                              "{\n"
                              "    static int a[] = { 3, 4, 5, 6 };\n"
                              "    static int b[] = { 4, 5, 6, 7 };\n"
                              "    static int c[] = { 5, 6, 7, 8 };\n"
                              "    b[1] = 1;\n"
                              "    return x ? a : c;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6:16]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n",
                           "",
                           errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static int i = 0;\n"
                              "    if(i < foo())\n"
                              "        i += 5;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo() {\n"
                              "    static int x = 0;\n"
                              "    print(x);\n"
                              "    if(x > 5)\n"
                              "        x = 0;\n"
                              "    else\n"
                              "        x++;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo(int value) {\n"
                              "    static int array[16] = {0};\n"
                              "    if(array[value]) {}\n"
                              "    array[value] = 1;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int fun() {\n" // #11310
                              "    static int k;\n"
                              "    k++;\n"
                              "    return k;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int& f() {\n" // #12935
                              "    static int i;\n"
                              "    return i;\n"
                              "}\n"
                              "int* g() {\n"
                              "    static int j;\n"
                              "    return &j;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarextern() {
        functionVariableUsage("void foo() {\n"
                              "    extern int i;\n"
                              "    i = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvardynamic1() {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = malloc(16);\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new char[16];\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        // extracttests.disable

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new ( nothrow ) char[16];\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new ( std::nothrow ) char[16];\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        // extracttests.enable

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new char;\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    void* ptr = malloc(16);\n"
                              "    ptr[0] = 123;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = new char[16];\n"
                              "    ptr[0] = 123;\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    Fred* fred = new Fred;\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    delete fred;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct Fred { int a; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* fred = new Fred;\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    delete fred;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Variable 'fred' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("struct Fred { int a; Fred() : a(0) {} };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* fred = new Fred;\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    delete fred;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Variable 'fred' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    Fred* fred = malloc(sizeof(Fred));\n"
                              "    std::cout << \"test\" << std::endl;\n"
                              "    free(fred);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (style) Variable 'fred' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());


        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    char* ptr = names[i];\n"
                              "    delete[] ptr;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvardynamic2() {
        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = (Fred*)malloc(sizeof(Fred));\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:31]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = (Fred*)malloc(sizeof(Fred));\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = (Fred*)malloc(sizeof(Fred));\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:38]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = (Fred*)malloc(sizeof(Fred));\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        // extracttests.disable

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new (nothrow ) Fred();\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new (std::nothrow) Fred();\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        // extracttests.enable

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    ptr->i = 0;\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = new Fred();\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("struct Fred { int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    struct Fred* ptr = new Fred();\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = (Fred*)malloc(sizeof(Fred));\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:31]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = (Fred*)malloc(sizeof(Fred));\n"
                              "    ptr->i = 0;\n"
                              "    free(ptr);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Variable 'ptr' is allocated memory that is never used. [unusedAllocatedMemory]\n", errout_str());

        functionVariableUsage("class Fred { public: int i; };\n"
                              "void foo()\n"
                              "{\n"
                              "    Fred* ptr = new Fred();\n"
                              "    ptr->i = 0;\n"
                              "    delete ptr;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvardynamic3() {
        // Ticket #3467 - False positive that 'data' is not assigned a value
        functionVariableUsage("void foo() {\n"
                              "    int* data = new int[100];\n"
                              "    int* p = data;\n"
                              "    for ( int i = 0; i < 10; ++i )\n"
                              "        p++;\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (style) Variable 'p' is modified but its new value is never used.\n",
                           "",
                           errout_str());
    }

    void localvararray1() {
        functionVariableUsage("void foo() {\n"
                              "    int p[5];\n"
                              "    *p = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvararray2() {
        functionVariableUsage("int foo() {\n"
                              "    int p[5][5];\n"
                              "    p[0][0] = 0;\n"
                              "    return p[0][0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvararray3() {
        functionVariableUsage("int foo() {\n"
                              "    int p[5][5];\n"
                              "    *((int*)p[0]) = 0;\n"
                              "    return p[0][0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvararray4() {
        functionVariableUsage("void foo() {\n"
                              "    int p[1];\n"
                              "    int *pp[0];\n"
                              "    p[0] = 1;\n"
                              "    *pp[0] = p[0];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvararray5() {
        functionVariableUsage("int foo() {\n"
                              "    int p[5][5];\n"
                              "    dostuff(*p);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo() {\n"
                              "    int p[5];\n"
                              "    dostuff(*p);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo() {\n"
                              "    int p[5][5][5];\n"
                              "    dostuff(**p);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n" // #11872
                              "    char v[1][2];\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Unused variable: v [unusedVariable]\n", errout_str());
    }

    void localvararray6() {
        functionVariableUsage("struct S { int* p; };\n" // #11012
                              "void g(struct S* ps);\n"
                              "void f() {\n"
                              "    int i[2];\n"
                              "    struct S s = { i };\n"
                              "    g(&s);\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarstring1() { // ticket #1597
        functionVariableUsage("void foo() {\n"
                              "    std::string s;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Unused variable: s [unusedVariable]\n", errout_str());

        functionVariableUsage("void foo() {\n"
                              "    std::string s;\n"
                              "    s = \"foo\";\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void foo() {\n"
                              "    std::string s = \"foo\";\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:19]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void foo() {\n" // #8901
                              "    const std::string s = \"foo\";\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:25]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("std::string foo() {\n"
                              "    std::string s;\n" // Class instances are initialized. Assignment is not necessary
                              "    return s;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("std::string foo() {\n"
                              "    std::string s = \"foo\";\n"
                              "    return s;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n"
                              "    std::string s(\"foo\");\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void f() {\n"
                              "    std::string s{ \"foo\" };\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvarstring2() { // ticket #2929
        functionVariableUsage("void foo() {\n"
                              "    std::string s;\n"
                              "    int i;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (style) Unused variable: s [unusedVariable]\n"
                      "[test.cpp:3:9]: (style) Unused variable: i [unusedVariable]\n", errout_str());
    }

    void localvarconst1() {
        functionVariableUsage("void foo() {\n"
                              "    const bool b = true;\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:18]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());
    }

    void localvarconst2() {
        functionVariableUsage("void foo() {\n"
                              "    const int N = 10;\n"
                              "    struct X { int x[N]; };\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarreturn() { // ticket #9167
        functionVariableUsage("void foo() {\n"
                              "    const int MyInt = 1;\n"
                              "    class bar {\n"
                              "      public:\n"
                              "        bool operator()(const int &uIndexA, const int &uIndexB) const {\n"
                              "            return true;\n"
                              "        }\n"
                              "        bar() {}\n"
                              "    };\n"
                              "    return MyInt;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarmaybeunused() {
        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] int x;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("[[nodiscard]] int getX() { return 4; }\n"
                              "int main() {\n"
                              "    [[maybe_unused]] int x = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("[[nodiscard]] int getX() { return 4; }\n"
                              "int main() {\n"
                              "    [[maybe_unused]] int x = getX();\n"
                              "    x = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("[[nodiscard]] int getX() { return 4; }\n"
                              "int main() {\n"
                              "    [[maybe_unused]] int x = getX();\n"
                              "    x = getX();\n"
                              "    std::cout << x;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] const int x = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] const int& x = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] const int* x = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] int& x = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] int* x = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] auto x = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] auto&& x = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] int x[] = getX();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] constexpr volatile static int x = 1;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("[[maybe_unused]] inline int x = 1;");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] [[anotherattribute]] const int* = 1;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    [[maybe_unused]] char a[1];\n"
                              "    [[maybe_unused]] char b[1][2];\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    std::string a [[maybe_unused]];\n"
                              "    f();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarrvalue() { // ticket #13977
        functionVariableUsage("void f(void) {\n"
                              "    std::string s;\n"
                              "    std::string&& m = std::move(s);\n"
                              "    cb();\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3:21]: (style) Variable 'm' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvarthrow() { // ticket #3687
        functionVariableUsage("void foo() {\n"
                              "    try {}"
                              "    catch(Foo& bar) {}\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localVarStd() {
        // extracttests.start: struct MyClass {int x;}; std::string foo();

        functionVariableUsage("void f() {\n"
                              "    std::string x = foo();\n"
                              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:19]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n",
            errout_str());

        functionVariableUsage("void f() {\n"
                              "    std::vector<int> x;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (style) Unused variable: x [unusedVariable]\n", errout_str());

        functionVariableUsage("void f() {\n"
                              "    std::vector<int> x(100);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:23]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("void f() {\n"
                              "    std::vector<MyClass> x;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:26]: (style) Unused variable: x [unusedVariable]\n", errout_str());

        functionVariableUsage("void f() {\n"
                              "    std::lock_guard<MyClass> lock(mutex_);\n" // Has a side-effect #4385
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n"
                              "    pLocker = std::shared_ptr<jfxLocker>(new jfxLocker(m_lock, true));\n" // Could have side-effects (#4355)
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n"
                              "    std::mutex m;\n"
                              "    std::unique_lock<std::mutex> lock(m);\n" // #4624
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n" // #7732
                              "    const std::pair<std::string, std::string> p(\"a\", \"b\");\n"
                              "    std::pair<std::string, std::string> q{\"a\", \"b\" };\n"
                              "    auto r = std::pair<std::string, std::string>(\"a\", \"b\");\n"
                              "    auto s = std::pair<std::string, std::string>{ \"a\", \"b\" };\n"
                              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'p' is assigned a value that is never used. [unreadVariable]\n"
                           "[test.cpp:2:48]: (style) Variable 'p' is assigned a value that is never used. [unreadVariable]\n"
                           "[test.cpp:4]: (style) Variable 'r' is assigned a value that is never used. [unreadVariable]\n"
                           "[test.cpp:5:18]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n",
                           "[test.cpp:2:48]: (style) Variable 'p' is assigned a value that is never used. [unreadVariable]\n"
                           "[test.cpp:3:42]: (style) Variable 'q' is assigned a value that is never used. [unreadVariable]\n",
                           errout_str());

        functionVariableUsage("void f(std::span<int> s) {\n" // #11545
                              "    s[0] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct S {\n"
                              "    std::mutex m;\n"
                              "    void f();\n"
                              "};\n"
                              "void S::f() {\n"
                              "    const ::std::lock_guard g(m);\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(const std::string& str, int i) {\n" // #11879
                              "    const std::string s = str;\n"
                              "    switch (i) {\n"
                              "    default:\n"
                              "        break;\n"
                              "    }\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2:23]: (style) Variable 's' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localVarClass() {
        functionVariableUsage("void f() {\n"
                              "    Fred f;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("class C { int x; };\n"
                              "void f() {\n"
                              "    C c;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) Unused variable: c [unusedVariable]\n", errout_str());

        functionVariableUsage("class ExampleClass\n" // #10000
                              "{\n"
                              "public:\n"
                              "   ExampleClass(int xScale, int yScale, int x, int y)\n"
                              "      : XScale(xScale)\n"
                              "      , YScale(yScale)\n"
                              "      , X(x)\n"
                              "      , Y(y)\n"
                              "   {\n"
                              "   }\n"
                              " \n"
                              "   int XScale;\n"
                              "   int YScale;\n"
                              "   int X;\n"
                              "   int Y;\n"
                              "};\n"
                              " \n"
                              "void foo()\n"
                              "{\n"
                              "   ExampleClass ex(1, 2, 3, 4);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:20:19]: (style) Variable 'ex' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("class C { public: C(int); ~C(); };\n"
                              "void f() {\n"
                              "    C c(12);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // #14643
        functionVariableUsage("class S { S(int); };\n"
                              "void f() { S s = 0; }\n");
        ASSERT_EQUALS("", errout_str());

        // #10965
        functionVariableUsage("class A {\n"
                              "public:\n"
                              "    A();\n"
                              "};\n"
                              "extern A cb();\n"
                              "void f() { const A c = cb(); }\n");
        ASSERT_EQUALS("", errout_str());

        // #11704
        functionVariableUsage("class S {\n"
                              "public:\n"
                              "    S();\n"
                              "};\n"
                              "class C {\n"
                              "    S &s();\n"
                              "    void f() {\n"
                              "        const S s1 = s(); // warning\n"
                              "        const S s2; // no warning\n"
                              "    }\n"
                              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localVarSmartPtr() {
        // handling of smart pointers (#9680)
        functionVariableUsage("static int s_i = 0;\n"
                              "\n"
                              "class A {\n"
                              "public:\n"
                              "    ~A() {\n"
                              "        ++s_i;\n"
                              "    }\n"
                              "};\n"
                              "\n"
                              "static void func() {\n"
                              "    auto a = std::make_shared<A>();\n"
                              "    auto a2 = std::unique_ptr<A>(new A());\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("class A {\n"
                              "public:\n"
                              "    std::string x;\n"
                              "};\n"
                              "\n"
                              "static void func() {\n"
                              "    auto a = std::make_shared<A>();\n"
                              "    auto a2 = std::unique_ptr<A>(new A());\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:7:12]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:8:13]: (style) Variable 'a2' is assigned a value that is never used. [unreadVariable]\n",
                      errout_str());

        functionVariableUsage("void g();\n" // #11094
                              "void f() {\n"
                              "    auto p = std::make_unique<S>();\n"
                              "    p = nullptr;\n"
                              "    g();\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int f(int *p) {\n" // #10703
                              "    std::unique_ptr<int> up(p);\n"
                              "    return *p;\n"
                              "}\n"
                              "int g(int* p) {\n"
                              "    auto up = std::unique_ptr<int>(p);\n"
                              "    return *p;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct S { S(); };\n" // #11108
                              "void f(std::unique_ptr<S> p) {\n"
                              "    p = nullptr;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("struct S { S(); };\n" // #14160
                              "void f() {\n"
                              "    auto p = std::make_unique<S>();\n"
                              "    auto q = std::make_unique<U>();\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // ticket #3104 - false positive when variable is read with "if (NOT var)"
    void localvarIfNOT() {
        functionVariableUsage("void f() {\n"
                              "    bool x = foo();\n"
                              "    if (NOT x) { }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarAnd() { // #3672
        functionVariableUsage("int main() {\n"
                              "    unsigned flag = 0x1 << i;\n"
                              "    if (m_errorflags & flag) {\n"
                              "        return 1;\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarSwitch() { // #3744 - false positive when used in switch
        functionVariableUsage("const char *f(int x) {\n"
                              "    const char a[] = \"abc\";\n"
                              "    const char def[] = \"def\";\n"
                              "    const char *ptr;\n"
                              "    switch(x) {\n"
                              "        case 1:  ptr=a; break;\n"
                              "        default: ptr=def; break;\n"
                              "    }\n"
                              "    return ptr;\n"
                              "}");
        ASSERT_EQUALS("", errout_str()); // Don't write an error that "a" is not used

        functionVariableUsage("void x() {\n"
                              "    unsigned char* pcOctet = NULL;\n"
                              "    float fValeur;\n"
                              "    switch (pnodeCurrent->left.pnode->usLen) {\n"
                              "    case 4:\n"
                              "        fValeur = (float)pevalDataLeft->data.fd;\n"
                              "        pcOctet = (unsigned char*)&fValeur;\n"
                              "        break;\n"
                              "    case 8:\n"
                              "        pcOctet = (unsigned char*)&pevalDataLeft->data.fd;\n"
                              "        break;\n"
                              "    }\n"
                              "    for (iIndice = 1; iIndice <= (pnodeCurrent->usLen / 2); iIndice++) {\n"
                              "        *pcData = gacHexChar[(*pcOctet >> 4) & 0x0F];\n"
                              "    }\n"
                              "}");
        ASSERT_EQUALS("", errout_str()); // Don't write an error that "fValeur" is not used
    }

    void localvarNULL() { // #4203 - Setting NULL value is not redundant - it is safe
        functionVariableUsage("void f() {\n"
                              "    char *p = malloc(100);\n"
                              "    foo(p);\n"
                              "    free(p);\n"
                              "    p = NULL;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(Foo *p) {\n"
                              "    free(p);\n"
                              "    p = (Foo *)NULL;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n" // #11079
                              "    std::string s1{ nullptr };\n"
                              "    std::string s2{ NULL };\n"
                              "    std::string s4(nullptr);\n"
                              "    std::string s5(NULL);\n"
                              "}\n"
                              "struct A { A(void*) {} };\n"
                              "static void g() {\n"
                              "    A a1{ nullptr };\n"
                              "    A a2{ NULL };\n"
                              "    A a4(nullptr);\n"
                              "    A a5(NULL);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2:19]: (style) Variable 's1' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:3:19]: (style) Variable 's2' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:4:19]: (style) Variable 's4' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:5:19]: (style) Variable 's5' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:9:9]: (style) Variable 'a1' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:10:9]: (style) Variable 'a2' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:11:9]: (style) Variable 'a4' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:12:9]: (style) Variable 'a5' is assigned a value that is never used. [unreadVariable]\n",
                      errout_str());
    }

    void localvarUnusedGoto() {
        // #4447
        functionVariableUsage("bool f(const int &i) {\n"
                              " int X = i;\n"
                              "label:\n"
                              " if ( X == 0 ) {\n"
                              "    X -= 101;\n"
                              "    return true;\n"
                              " }\n"
                              " if ( X < 1001 )  {\n"
                              "    X += 1;\n"
                              "    goto label;\n"
                              " }\n"
                              " return false;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:5:7]: (style) Variable 'X' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        // #4558
        functionVariableUsage("int f() {\n"
                              " int i,j=0;\n"
                              " start:\n"
                              " i=j;\n"
                              " i++;\n"
                              " j=i;\n"
                              " if (i<3)\n"
                              "     goto start;\n"
                              " return i;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarLambda() {
        functionVariableUsage("int foo() {\n"
                              "    auto f = []{return 1};\n"
                              "    return f();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo() {\n"
                              "    auto f = []{return 1};\n"
                              "    auto g = []{return 1};\n"
                              "    return f() + g();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void foo(std::vector<int>& v) {\n"
                              "    int n = 0;\n"
                              "    std::generate(v.begin(), v.end(), [&n] {\n"
                              "        int r = n;\n"
                              "        n += 2;\n"
                              "        return r;\n"
                              "    });\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int f() {\n" // #8433
                              "    float a;\n"
                              "    auto lambda = []() {};\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Unused variable: a [unusedVariable]\n", errout_str());

        functionVariableUsage("void f() {\n" // #9823
                              "    auto cb = []() {\n"
                              "        int i;\n"
                              "    };\n"
                              "    (void)cb;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3:13]: (style) Unused variable: i [unusedVariable]\n", errout_str());

        functionVariableUsage("void f() {\n" // #9822
                              "    int i;\n"
                              "    auto cb = []() {\n"
                              "        int i;\n"
                              "    };\n"
                              "    (void)cb;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Unused variable: i [unusedVariable]\n"
                      "[test.cpp:4:13]: (style) Unused variable: i [unusedVariable]\n",
                      errout_str());
    }


    void localvarStructuredBinding() {
        functionVariableUsage("void f() {\n" // #10368
                              "    std::map<int, double> m;\n"
                              "    m[2] = 2.0;\n"
                              "    for (auto& [k, v] : m) {}\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarCppInitialization() {
        functionVariableUsage("void foo() {\n"
                              "    int buf[6];\n"
                              "    Data data(buf);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:14]: (information) --check-library: Provide <type-checks><unusedvar> configuration for Data [checkLibraryCheckType]\n", errout_str());
    }

    void localvarCpp11Initialization() {
        // #6160
        functionVariableUsage("void foo() {\n"
                              "    int myNewValue{ 3u };\n"
                              "    myManager.theDummyTable.addRow(UnsignedIndexValue{ myNewValue }, DummyRowData{ false });\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n"
                              "    std::list<std::list<int>>::value_type a{ 1, 2, 3, 4 };\n"
                              "}\n");
        TODO_ASSERT_EQUALS("", "[test.cpp:2:44]: (information) --check-library: Provide <type-checks><unusedvar> configuration for std::list::value_type [checkLibraryCheckType]\n", errout_str());

        functionVariableUsage("void f(int* p) {\n"
                              "    int* q{ p };\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Variable 'q' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void localvarRangeBasedFor() {
        // #7075
        functionVariableUsage("void reset() {\n"
                              "    for (auto & e : array)\n"
                              "        e = 0;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarAssignInWhile() {
        functionVariableUsage("void foo() {\n"
                              "  int a = 0;\n"
                              "  do {\n"
                              "    dostuff(a);\n"
                              "  } while((a += x) < 30);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int foo() {\n"
                              "    int var = 1;\n"
                              "    while (var = var >> 1) { }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarTemplate() {
        functionVariableUsage("template<int A> void f() {}\n"
                              "void foo() {\n"
                              "  const int x = 0;\n"
                              "  f<x>();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n"
                              "  constexpr std::size_t ArraySize(5);\n"
                              "  std::array<int, ArraySize> X; X.dostuff();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n" // #10686
                              "    std::array<int, 1> a;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2:24]: (style) Unused variable: a [unusedVariable]\n", errout_str());

        functionVariableUsage("class A {};\n" // #9471
                              "    namespace std {\n"
                              "    template<>\n"
                              "    struct hash<A> {};\n"
                              "}\n"
                              "char f() {\n"
                              "    std::string hash = \"-\";\n"
                              "    return hash[0];\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("template <int N>\n" // #14199
                              "struct S {\n"
                              "    int i;\n"
                              "};\n"
                              "void f() {\n"
                              "    S<0> s;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:6:10]: (style) Unused variable: s [unusedVariable]\n", errout_str());
    }

    void localvarFuncPtr() {
        functionVariableUsage("int main() {\n"
                              "    void(*funcPtr)(void)(x);\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'funcPtr' is assigned a value never used.\n", "", errout_str());

        functionVariableUsage("int main() {\n"
                              "    void(*funcPtr)(void);\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (style) Unused variable: funcPtr [unusedVariable]\n", errout_str());

        functionVariableUsage("int main() {\n"
                              "    void(*funcPtr)(void)(x);\n"
                              "    funcPtr();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main() {\n"
                              "    void(*funcPtr)(void) = x;\n"
                              "    funcPtr();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        // #14661
        functionVariableUsage("int main() {\n"
                              "    void (*const funcPtr[])(void) = {x};\n"
                              "    funcPtr[0]();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarAddr() { // #7747
        functionVariableUsage("void f() {\n"
                              "  int x = 0;\n"
                              "  dostuff(&x);\n"
                              "  x = 1;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f() {\n"
                              "  int x = 0;\n"
                              "  dostuff(std::ref(x));\n"
                              "  x = 1;\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void localvarDelete() { // #8339
        functionVariableUsage("void reassign(char*& data, int size)"
                              "{"
                              "    char* buf = new char[size];"
                              ""
                              "    char* tmp = data;"
                              "    data = buf;"
                              "    buf = tmp;"
                              ""
                              "    delete [] buf;"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void chainedAssignment() {
        // #5466
        functionVariableUsage("void NotUsed(double* pdD, int n) {\n"
                              "    double sum = 0.0;\n"
                              "    for (int i = 0; i<n; ++i)\n"
                              "        pdD[i] = (sum += pdD[i]);\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void crash1() {
        functionVariableUsage("SAL_WNODEPRECATED_DECLARATIONS_PUSH\n"
                              "void convertToTokenArray() {\n"
                              "}\n"
                              "SAL_WNODEPRECATED_DECLARATIONS_POP"); // #4033
    }

    void crash2() {
        functionVariableUsage("template<unsigned dim>\n"
                              "struct Y: Y<dim-1> { };\n"
                              "template<>\n"
                              "struct Y<0> {};\n"
                              "void f() {\n"
                              "    Y y;\n"
                              "}"); // #4695
    }

    void crash3() {
        functionVariableUsage("void f(int a, int b, const int* p) {\n" // #12531
                              "    const int* s[] = { p, p + 1, p + 2 };\n"
                              "    a = *(s[a] + b);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3:7]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void usingNamespace() {
        functionVariableUsage("int foo() {\n"
                              "   using namespace ::com::sun::star::i18n;\n"
                              "   bool b = false;\n"
                              "   int j = 0;\n"
                              "   for (int i = 0; i < 3; i++) {\n"
                              "          if (!b) {\n"
                              "             j = 3;\n"
                              "             b = true;\n"
                              "          }\n"
                              "   }\n"
                              "   return j;\n"
                              "}"); // #4585
        ASSERT_EQUALS("", errout_str());
    }

    void lambdaFunction() {
        // #7026
        functionVariableUsage("void f() {\n"
                              "  bool first = true;\n"
                              "\n"
                              "  auto do_something = [&first]() {\n"
                              "    if (first) {\n"
                              "      first = false;\n"
                              "    } else {\n"
                              "      dostuff();\n"
                              "    }\n"
                              "  };\n"
                              "  do_something();\n"
                              "  do_something();\n"
                              "}");
        ASSERT_EQUALS("", errout_str());


        // #4956 - assignment in for_each
        functionVariableUsage("void f(std::vector<int> ints) {\n"
                              "  int x = 0;\n"
                              "  std::for_each(ints.begin(), ints.end(), [&x](int i){ dostuff(x); x = i; });\n"
                              "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("void f(std::vector<int> ints) {\n"
                              "  int x = 0;\n"
                              "  std::for_each(ints.begin(), ints.end(), [&x](int i){ x += i; });\n"
                              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3:5]: (style) Variable 'x' is assigned a value that is never used. [unreadVariable]\n", "", errout_str());

        functionVariableUsage("int f(const std::vector<int>& v) {\n"
                              "    auto it = std::find_if(v.begin(), v.end(), [&](int i) { return i > 0 && i < 7; });\n"
                              "    std::unordered_map<std::string, std::vector<int*>> exprs;\n"
                              "    return *it;\n"
                              "}");
        ASSERT_EQUALS("[test.cpp:3:56]: (style) Unused variable: exprs [unusedVariable]\n", errout_str());
    }

    void namespaces() { // #7557
        functionVariableUsage("namespace t { namespace g {\n"
                              "   typedef std::pair<BoostBox, size_t> value;\n"
                              "} }\n"
                              "namespace t { namespace g {} }\n"
                              "namespace t {\n"
                              "  inline double getTime() const {\n"
                              "     iterator it=find();\n"
                              "     double& value=it->second.values[index];\n"
                              "     if(isnan(value)) {\n"
                              "       value=get();\n"
                              "     }\n"
                              "     return value;\n"
                              "  }\n"
                              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void bracesInitCpp11() {
        functionVariableUsage(
            "int fun() {\n"
            " static int fpUnread{0};\n"
            " const int var{fpUnread++};\n"
            " return var;\n"
            "}");
        ASSERT_EQUALS("", errout_str());
    }

    void argument() {
        functionVariableUsage(
            "void fun(Value value) {\n"
            " value[10] = 123;\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage(
            "void fun(std::string s) {\n"
            " s[10] = 123;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2:8]: (style) Variable 's[10]' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage(
            "void fun(short data[2]) {\n"
            "  data[2] = 1;\n"
            "}"
            );
        ASSERT_EQUALS("", errout_str());

        // Unknown argument type
        functionVariableUsage(
            "void A::b(Date& result) {"
            "  result = 12;\n"
            "}"
            );
        ASSERT_EQUALS("", errout_str());

        {
            // #8914
            functionVariableUsage( // assume unknown argument type is reference
                "void fun(Date result) {"
                "  result.x = 12;\n"
                "}"
                );
            ASSERT_EQUALS("", errout_str());

            functionVariableUsage( // there is no reference type in C
                "void fun(Date result) {"
                "  result.x = 12;\n"
                "}",
                dinit(FunctionVariableUsageOptions, $.cpp = false)
                );
            ASSERT_EQUALS("[test.c:1:35]: (style) Variable 'result.x' is assigned a value that is never used. [unreadVariable]\n", errout_str());

            functionVariableUsage(
                "struct Date { int x; };\n"
                "void fun(Date result) {"
                "  result.x = 12;\n"
                "}"
                );
            ASSERT_EQUALS("[test.cpp:2:35]: (style) Variable 'result.x' is assigned a value that is never used. [unreadVariable]\n", errout_str());
        }

        // Unknown struct type
        functionVariableUsage(
            "void fun() {"
            "  struct FOO foo;\n"
            "  foo.x = 123;\n"
            "}"
            );
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Variable 'foo.x' is assigned a value that is never used. [unreadVariable]\n", errout_str());
    }

    void argumentClass() {
        functionVariableUsage(
            "void foo(std::insert_iterator<C> it) {\n"
            "  it = 123;\n"
            "}");
        ASSERT_EQUALS("", errout_str());
    }

    void escapeAlias() {
        functionVariableUsage(
            "struct A {\n"
            "    std::map<int, int> m;\n"
            "    void f(int key, int number) {\n"
            "        auto pos = m.find(key);\n"
            "        if (pos == m.end())\n"
            "            m.insert(std::map<int, int>::value_type(key, number));\n"
            "        else\n"
            "            (*pos).second = number;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout_str());
    }

    void volatileData() {
        functionVariableUsage(
            "struct Data { unsigned int n; };\n"
            "int main() {\n"
            "  (*(volatile struct Data*)0x4200).n = 1;\n"
            "}");
        ASSERT_EQUALS("", errout_str());
    }

    void globalData() {
        // #10276
        functionVariableUsage(
            "void f(void) {\n"
            "    ((uint8_t *) (uint16_t)0x1000)[0] = 0x42;\n"
            "}");
        ASSERT_EQUALS("", errout_str());
    }

    void structuredBinding() { // #13269
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    auto [a, b] = std::make_pair(42, 0.42);\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:3:17]: (style) Variable 'a' is assigned a value that is never used. [unreadVariable]\n"
                      "[test.cpp:3:17]: (style) Variable 'b' is assigned a value that is never used. [unreadVariable]\n", errout_str());

        functionVariableUsage("int main()\n"
                              "{\n"
                              "    auto [a, b] = std::make_pair(42, 0.42);\n"
                              "    (void) a;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main()\n"
                              "{\n"
                              "    auto [a, b] = std::make_pair(42, 0.42);\n"
                              "    (void) b;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main()\n"
                              "{\n"
                              "    auto [a, b, c] = std::make_pair(42, 0.42);\n"
                              "    (void) b;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());

        functionVariableUsage("int main()\n"
                              "{\n"
                              "    [[maybe_unused]] auto [a2, b3] = std::make_pair(42, 0.42);\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void pointerCast1() { // #14535
        functionVariableUsage("void f(int* p)\n"
                              "{\n"
                              "    int* p2 = p;\n"
                              "    ((int *)(p2))[0] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void pointerCast2() {
        functionVariableUsage("void f(int* p)\n"
                              "{\n"
                              "    int* p2 = p;\n"
                              "    static_cast<int*>(p2)[0] = 0;\n"
                              "}\n");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestUnusedVar)
