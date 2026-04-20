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

#include "helpers.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "fixture.h"
#include "token.h"

#include <cstddef>
#include <string>

class TestVarID : public TestFixture {
public:
    TestVarID() : TestFixture("TestVarID") {}

private:
    const Settings settings = settingsBuilder().c(Standards::C89).platform(Platform::Type::Unix64).build();
    void run() override {
        TEST_CASE(varid1);
        TEST_CASE(varid2);
        TEST_CASE(varid3);
        TEST_CASE(varid4);
        TEST_CASE(varid5);
        TEST_CASE(varid6);
        TEST_CASE(varid7);
        TEST_CASE(varidReturn1);
        TEST_CASE(varidReturn2);
        TEST_CASE(varidReturn3);
        TEST_CASE(varid8);
        TEST_CASE(varid9);
        TEST_CASE(varid10);
        TEST_CASE(varid11);
        TEST_CASE(varid12);
        TEST_CASE(varid13);
        TEST_CASE(varid14);
        TEST_CASE(varid15);
        TEST_CASE(varid16);
        TEST_CASE(varid17);   // ticket #1810
        TEST_CASE(varid18);
        TEST_CASE(varid19);
        TEST_CASE(varid20);
        TEST_CASE(varid24);
        TEST_CASE(varid25);
        TEST_CASE(varid26);   // ticket #1967 (list of function pointers)
        TEST_CASE(varid27);   // Ticket #2280 (same name for namespace and variable)
        TEST_CASE(varid28);   // ticket #2630
        TEST_CASE(varid29);   // ticket #1974
        TEST_CASE(varid30);   // ticket #2614
        TEST_CASE(varid34);   // ticket #2825
        TEST_CASE(varid35);   // function declaration inside function body
        TEST_CASE(varid36);   // ticket #2980 (segmentation fault)
        TEST_CASE(varid37);   // ticket #3092 (varid for 'Bar bar(*this);')
        TEST_CASE(varid38);   // ticket #3272 (varid for 'FOO class C;')
        TEST_CASE(varid39);   // ticket #3279 (varid for 'FOO::BAR const')
        TEST_CASE(varid40);   // ticket #3279
        TEST_CASE(varid41);   // ticket #3340 (varid for union type)
        TEST_CASE(varid42);   // ticket #3316 (varid for array)
        TEST_CASE(varid43);
        TEST_CASE(varid44);
        TEST_CASE(varid45); // #3466
        TEST_CASE(varid46); // struct varname
        TEST_CASE(varid47); // function parameters
        TEST_CASE(varid48); // #3785 - return (a*b)
        TEST_CASE(varid49); // #3799 - void f(std::vector<int>)
        TEST_CASE(varid50); // #3760 - explicit
        TEST_CASE(varid51); // don't set varid for template function
        TEST_CASE(varid52); // Set varid for nested templates
        TEST_CASE(varid53); // #4172 - Template instantiation: T<&functionName> list[4];
        TEST_CASE(varid54); // hang
        TEST_CASE(varid55); // #5868: Function::addArgument with varid 0 for argument named the same as a typedef
        TEST_CASE(varid56); // function with a throw()
        TEST_CASE(varid57); // #6636: new scope by {}
        TEST_CASE(varid58); // #6638: for loop in for condition
        TEST_CASE(varid59); // #6696
        TEST_CASE(varid60); // #7267 cast '(unsigned x)10'
        TEST_CASE(varid61); // #4988 inline function
        TEST_CASE(varid62);
        TEST_CASE(varid63);
        TEST_CASE(varid64); // #9922 - extern const char (*x[256])
        TEST_CASE(varid65); // #10936
        TEST_CASE(varid66);
        TEST_CASE(varid67); // #11711 - NOT function pointer
        TEST_CASE(varid68); // #11740 - switch (str_chars(&strOut)[0])
        TEST_CASE(varid69);
        TEST_CASE(varid70); // #12660 - function
        TEST_CASE(varid71); // #12676 - wrong varid in uninstantiated templated constructor
        TEST_CASE(varid72);
        TEST_CASE(varid_for_1);
        TEST_CASE(varid_for_2);
        TEST_CASE(varid_for_3);
        TEST_CASE(varid_cpp_keywords_in_c_code);
        TEST_CASE(varid_cpp_keywords_in_c_code2); // #5373: varid=0 for argument called "delete"
        TEST_CASE(varid_cpp_keywords_in_c_code3);
        TEST_CASE(varid_cpp_keywords_in_c_code4);
        TEST_CASE(varidFunctionCall1);
        TEST_CASE(varidFunctionCall2);
        TEST_CASE(varidFunctionCall3);
        TEST_CASE(varidFunctionCall4);  // ticket #3280
        TEST_CASE(varidFunctionCall5);
        TEST_CASE(varidStl);
        TEST_CASE(varidStl2);
        TEST_CASE(varid_newauto);       // not declaration: new const auto(0);
        TEST_CASE(varid_delete);
        TEST_CASE(varid_functions);
        TEST_CASE(varid_sizeof);
        TEST_CASE(varid_reference_to_containers);
        TEST_CASE(varid_in_class1);
        TEST_CASE(varid_in_class2);
        TEST_CASE(varid_in_class3);     // #3092 - shadow variable in member function
        TEST_CASE(varid_in_class4);     // #3271 - public: class C;
        TEST_CASE(varid_in_class5);     // #3584 - std::vector<::FOO::B> b;
        TEST_CASE(varid_in_class6);     // #3755
        TEST_CASE(varid_in_class7);     // set variable id for struct members
        TEST_CASE(varid_in_class8);     // unknown macro in class
        TEST_CASE(varid_in_class9);     // #4291 - id for variables accessed through 'this'
        TEST_CASE(varid_in_class10);
        TEST_CASE(varid_in_class11);    // #4277 - anonymous union
        TEST_CASE(varid_in_class12);    // #4637 - method
        TEST_CASE(varid_in_class13);    // #4637 - method
        TEST_CASE(varid_in_class14);
        TEST_CASE(varid_in_class15);    // #5533 - functions
        TEST_CASE(varid_in_class16);
        TEST_CASE(varid_in_class17);    // #6056 - no varid for member functions
        TEST_CASE(varid_in_class18);    // #7127
        TEST_CASE(varid_in_class19);
        TEST_CASE(varid_in_class20);    // #7267
        TEST_CASE(varid_in_class21);    // #7788
        TEST_CASE(varid_in_class22);    // #10872
        TEST_CASE(varid_in_class23);    // #11293
        TEST_CASE(varid_in_class24);
        TEST_CASE(varid_in_class25);
        TEST_CASE(varid_in_class26);
        TEST_CASE(varid_in_class27);
        TEST_CASE(varid_in_class28);
        TEST_CASE(varid_in_class29);
        TEST_CASE(varid_in_class30);
        TEST_CASE(varid_namespace_1);   // #7272
        TEST_CASE(varid_namespace_2);   // #7000
        TEST_CASE(varid_namespace_3);   // #8627
        TEST_CASE(varid_namespace_4);
        TEST_CASE(varid_namespace_5);
        TEST_CASE(varid_namespace_6);
        TEST_CASE(varid_initList);
        TEST_CASE(varid_initListWithBaseTemplate);
        TEST_CASE(varid_initListWithScope);
        TEST_CASE(varid_operator);
        TEST_CASE(varid_throw);
        TEST_CASE(varid_unknown_macro);     // #2638 - unknown macro is not type
        TEST_CASE(varid_using);  // ticket #3648
        TEST_CASE(varid_catch);
        TEST_CASE(varid_functionPrototypeTemplate);
        TEST_CASE(varid_templatePtr); // #4319
        TEST_CASE(varid_templateNamespaceFuncPtr); // #4172
        TEST_CASE(varid_templateArray);
        TEST_CASE(varid_templateParameter); // #7046 set varid for "X":  std::array<int,X> Y;
        TEST_CASE(varid_templateParameterFunctionPointer); // #11050
        TEST_CASE(varid_templateUsing); // #5781 #7273
        TEST_CASE(varid_templateSpecializationFinal);
        TEST_CASE(varid_not_template_in_condition); // #7988
        TEST_CASE(varid_cppcast); // #6190
        TEST_CASE(varid_variadicFunc);
        TEST_CASE(varid_typename); // #4644
        TEST_CASE(varid_rvalueref);
        TEST_CASE(varid_arrayFuncPar); // #5294
        TEST_CASE(varid_sizeofPassed); // #5295
        TEST_CASE(varid_classInFunction); // #5293
        TEST_CASE(varid_pointerToArray); // #2645
        TEST_CASE(varid_cpp11initialization); // #4344
        TEST_CASE(varid_inheritedMembers); // #4101
        TEST_CASE(varid_header); // #6386
        TEST_CASE(varid_rangeBasedFor);
        TEST_CASE(varid_structinit); // #6406
        TEST_CASE(varid_arrayinit); // #7579
        TEST_CASE(varid_lambda_arg);
        TEST_CASE(varid_lambda_mutable);
        TEST_CASE(varid_trailing_return1); // #8889
        TEST_CASE(varid_trailing_return2); // #9066
        TEST_CASE(varid_trailing_return3); // #11423
        TEST_CASE(varid_parameter_pack); // #9383
        TEST_CASE(varid_for_auto_cpp17);
        TEST_CASE(varid_not); // #9689 'not x'
        TEST_CASE(varid_declInIfCondition);
        TEST_CASE(varid_globalScope);
        TEST_CASE(varid_function_pointer_args);
        TEST_CASE(varid_alignas);

        TEST_CASE(varidclass1);
        TEST_CASE(varidclass2);
        TEST_CASE(varidclass3);
        TEST_CASE(varidclass4);
        TEST_CASE(varidclass5);
        TEST_CASE(varidclass6);
        TEST_CASE(varidclass7);
        TEST_CASE(varidclass8);
        TEST_CASE(varidclass9);
        TEST_CASE(varidclass10);  // variable declaration below usage
        TEST_CASE(varidclass11);  // variable declaration below usage
        TEST_CASE(varidclass12);
        TEST_CASE(varidclass13);
        TEST_CASE(varidclass14);
        TEST_CASE(varidclass15);  // initializer list
        TEST_CASE(varidclass16);  // #4577
        TEST_CASE(varidclass17);  // #6073
        TEST_CASE(varidclass18);
        TEST_CASE(varidclass19);  // initializer list
        TEST_CASE(varidclass20);   // #7578: int (*p)[2]
        TEST_CASE(varid_classnameshaddowsvariablename); // #3990
        TEST_CASE(varid_classnametemplate); // #10221

        TEST_CASE(varidenum1);
        TEST_CASE(varidenum2);
        TEST_CASE(varidenum3);
        TEST_CASE(varidenum4);
        TEST_CASE(varidenum5);
        TEST_CASE(varidenum6); // #9180
        TEST_CASE(varidenum7); // #8991

        TEST_CASE(varidnamespace1);
        TEST_CASE(varidnamespace2);
        TEST_CASE(usingNamespace1);
        TEST_CASE(usingNamespace2);
        TEST_CASE(usingNamespace3);

        TEST_CASE(setVarIdStructMembers1);

        TEST_CASE(decltype1);
        TEST_CASE(decltype2);

        TEST_CASE(typeof1);
        TEST_CASE(typeof2);
        TEST_CASE(typeof3);
        TEST_CASE(typeof4);

        TEST_CASE(exprid1);
        TEST_CASE(exprid2);
        TEST_CASE(exprid3);
        TEST_CASE(exprid4);
        TEST_CASE(exprid5);
        TEST_CASE(exprid6);
        TEST_CASE(exprid7);
        TEST_CASE(exprid8);
        TEST_CASE(exprid9);
        TEST_CASE(exprid10);
        TEST_CASE(exprid11);
        TEST_CASE(exprid12);
        TEST_CASE(exprid13);
        TEST_CASE(exprid14);

        TEST_CASE(structuredBindings);
    }

    struct TokenizeOptions
    {
        bool cpp = true;
    };

#define tokenize(...) tokenize_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tokenize_(const char* file, int line, const char (&code)[size], const TokenizeOptions& options = make_default_obj()) {
        return tokenize_(file, line, code, settings, options.cpp);
    }

    template<size_t size>
    std::string tokenize_(const char* file, int line, const char (&code)[size], const Settings& settings1, bool cpp = true) {
        SimpleTokenizer tokenizer(settings1, *this, cpp);
        ASSERT_LOC((tokenizer.tokenize)(code), file, line);

        // result..
        Token::stringifyOptions str_options = Token::stringifyOptions::forDebugVarId();
        str_options.files = false;
        return tokenizer.tokens()->stringifyList(str_options);
    }

#define tokenizeHeader(...) tokenizeHeader_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tokenizeHeader_(const char* file, int line, const char (&code)[size], const char filename[]) {
        SimpleTokenizer tokenizer{settings, *this, std::string(filename)};
        ASSERT_LOC((tokenizer.tokenize)(code), file, line);

        // result..
        Token::stringifyOptions options = Token::stringifyOptions::forDebugVarId();
        options.files = false;
        return tokenizer.tokens()->stringifyList(options);
    }

#define tokenizeExpr(...) tokenizeExpr_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tokenizeExpr_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer2 tokenizer(settings, *this, code, "test.cpp");

        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // result..
        Token::stringifyOptions options = Token::stringifyOptions::forDebugExprId();
        options.files = false;
        return tokenizer.tokens()->stringifyList(options);
    }

#define compareVaridsForVariable(...) compareVaridsForVariable_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string compareVaridsForVariable_(const char* file, int line, const char (&code)[size], const char varname[]) {
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC((tokenizer.tokenize)(code), file, line);

        unsigned int varid = ~0U;
        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            if (tok->str() == varname) {
                if (varid == ~0U)
                    varid = tok->varId();
                else if (varid != tok->varId())
                    return std::string("Variable ") + varname + " has different varids:\n" + tokenizer.tokens()->stringifyList(true,true,true,true,false);
            }
        }

        return "same varid";
    }

    void varid1() {
        {
            const std::string actual = tokenize(
                "static int i = 1;\n"
                "void f()\n"
                "{\n"
                "    int i = 2;\n"
                "    for (int i = 0; i < 10; ++i)\n"
                "        i = 3;\n"
                "    i = 4;\n"
                "}\n", dinit(TokenizeOptions, $.cpp = false));

            const char expected[] = "1: static int i@1 = 1 ;\n"
                                    "2: void f ( )\n"
                                    "3: {\n"
                                    "4: int i@2 ; i@2 = 2 ;\n"
                                    "5: for ( int i@3 = 0 ; i@3 < 10 ; ++ i@3 ) {\n"
                                    "6: i@3 = 3 ; }\n"
                                    "7: i@2 = 4 ;\n"
                                    "8: }\n";

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                "static int i = 1;\n"
                "void f()\n"
                "{\n"
                "    int i = 2;\n"
                "    for (int i = 0; i < 10; ++i)\n"
                "    {\n"
                "      i = 3;\n"
                "    }\n"
                "    i = 4;\n"
                "}\n", dinit(TokenizeOptions, $.cpp = false));

            const char expected[] = "1: static int i@1 = 1 ;\n"
                                    "2: void f ( )\n"
                                    "3: {\n"
                                    "4: int i@2 ; i@2 = 2 ;\n"
                                    "5: for ( int i@3 = 0 ; i@3 < 10 ; ++ i@3 )\n"
                                    "6: {\n"
                                    "7: i@3 = 3 ;\n"
                                    "8: }\n"
                                    "9: i@2 = 4 ;\n"
                                    "10: }\n";

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid2() {
        const std::string actual = tokenize(
            "void f()\n"
            "{\n"
            "    struct ABC abc;\n"
            "    abc.a = 3;\n"
            "    i = abc.a;\n"
            "}\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: void f ( )\n"
                                "2: {\n"
                                "3: struct ABC abc@1 ;\n"
                                "4: abc@1 . a@2 = 3 ;\n"
                                "5: i = abc@1 . a@2 ;\n"
                                "6: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid3() {
        const std::string actual = tokenize(
            "static char str[4];\n"
            "void f()\n"
            "{\n"
            "    char str[10];\n"
            "    str[0] = 0;\n"
            "}\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: static char str@1 [ 4 ] ;\n"
                                "2: void f ( )\n"
                                "3: {\n"
                                "4: char str@2 [ 10 ] ;\n"
                                "5: str@2 [ 0 ] = 0 ;\n"
                                "6: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid4() {
        const std::string actual = tokenize(
            "void f(const unsigned int a[])\n"
            "{\n"
            "    int i = *(a+10);\n"
            "}\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: void f ( const unsigned int a@1 [ ] )\n"
                                "2: {\n"
                                "3: int i@2 ; i@2 = * ( a@1 + 10 ) ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid5() {
        const std::string actual = tokenize(
            "void f()\n"
            "{\n"
            "    int a,b;\n"
            "}\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: void f ( )\n"
                                "2: {\n"
                                "3: int a@1 ; int b@2 ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, actual);
    }


    void varid6() {
        const std::string actual = tokenize(
            "int f(int a, int b)\n"
            "{\n"
            "    return a+b;\n"
            "}\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: int f ( int a@1 , int b@2 )\n"
                                "2: {\n"
                                "3: return a@1 + b@2 ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, actual);
    }


    void varid7() {
        const std::string actual = tokenize(
            "void func() {\n"
            "    char a[256] = \"test\";\n"
            "    {\n"
            "        char b[256] = \"test\";\n"
            "    }\n"
            "}\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: void func ( ) {\n"
                                "2: char a@1 [ 256 ] = \"test\" ;\n"
                                "3: {\n"
                                "4: char b@2 [ 256 ] = \"test\" ;\n"
                                "5: }\n"
                                "6: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varidReturn1() {
        const std::string actual = tokenize(
            "int f()\n"
            "{\n"
            "    int a;\n"
            "    return a;\n"
            "}\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: int f ( )\n"
                                "2: {\n"
                                "3: int a@1 ;\n"
                                "4: return a@1 ;\n"
                                "5: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varidReturn2() {
        const std::string actual = tokenize(
            "void foo()\n"
            "{\n"
            "    unsigned long mask = (1UL << size_) - 1;\n"
            "    return (abits_val_ & mask);\n"
            "}\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: void foo ( )\n"
                                "2: {\n"
                                "3: unsigned long mask@1 ; mask@1 = ( 1UL << size_ ) - 1 ;\n"
                                "4: return ( abits_val_ & mask@1 ) ;\n"
                                "5: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varidReturn3() {
        const std::string actual = tokenize(
            "struct S { int i; };\n"
            "int f(S s) {\n"
            "    return (&s)->i;\n"
            "}\n");

        const char expected[] = "1: struct S { int i@1 ; } ;\n"
                                "2: int f ( S s@2 ) {\n"
                                "3: return s@2 . i@3 ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid8() {
        const std::string actual = tokenize(
            "void func()\n"
            "{\n"
            "    std::string str(\"test\");\n"
            "    str.clear();\n"
            "}");

        const char expected[] = "1: void func ( )\n"
                                "2: {\n"
                                "3: std :: string str@1 ( \"test\" ) ;\n"
                                "4: str@1 . clear ( ) ;\n"
                                "5: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid9() {
        const std::string actual = tokenize(
            "typedef int INT32;\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: ;\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid10() {
        const std::string actual = tokenize(
            "void foo()\n"
            "{\n"
            "    int abc;\n"
            "    struct abc abc1;\n"
            "}", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: void foo ( )\n"
                                "2: {\n"
                                "3: int abc@1 ;\n"
                                "4: struct abc abc1@2 ;\n"
                                "5: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid11() {
        const std::string actual = tokenize("class Foo;");

        const char expected[] = "1: class Foo ;\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid12() {
        const std::string actual = tokenize(
            "static void a()\n"
            "{\n"
            "    class Foo *foo;\n"
            "}");

        const char expected[] = "1: static void a ( )\n"
                                "2: {\n"
                                "3: class Foo * foo@1 ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid13() {
        const std::string actual = tokenize(
            "void f()\n"
            "{\n"
            "    int a; int b;\n"
            "    a = a;\n"
            "}\n", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: void f ( )\n"
                                "2: {\n"
                                "3: int a@1 ; int b@2 ;\n"
                                "4: a@1 = a@1 ;\n"
                                "5: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid14() {
        // Overloaded operator*
        const std::string actual = tokenize(
            "void foo()\n"
            "{\n"
            "A a;\n"
            "B b;\n"
            "b * a;\n"
            "}", dinit(TokenizeOptions, $.cpp = false));

        const char expected[] = "1: void foo ( )\n"
                                "2: {\n"
                                "3: A a@1 ;\n"
                                "4: B b@2 ;\n"
                                "5: b@2 * a@1 ;\n"
                                "6: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid15() {
        {
            const std::string actual = tokenize(
                "struct S {\n"
                "    struct T {\n"
                "    } t;\n"
                "} s;", dinit(TokenizeOptions, $.cpp = false));

            const char expected[] = "1: struct S {\n"
                                    "2: struct T {\n"
                                    "3: } ; struct T t@1 ;\n"
                                    "4: } ; struct S s@2 ;\n";

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                "struct S {\n"
                "    struct T {\n"
                "    } t;\n"
                "};", dinit(TokenizeOptions, $.cpp = false));

            const char expected[] = "1: struct S {\n"
                                    "2: struct T {\n"
                                    "3: } ; struct T t@1 ;\n"
                                    "4: } ;\n";

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid16() {
        const char code[] ="void foo()\n"
                            "{\n"
                            "    int x = 1;\n"
                            "    y = (z * x);\n"
                            "}\n";

        const char expected[] = "1: void foo ( )\n"
                                "2: {\n"
                                "3: int x@1 ; x@1 = 1 ;\n"
                                "4: y = z * x@1 ;\n"
                                "5: }\n";

        ASSERT_EQUALS(expected, tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid17() { // ticket #1810
        const char code[] ="char foo()\n"
                            "{\n"
                            "    char c('c');\n"
                            "    return c;\n"
                            "}\n";

        const char expected[] = "1: char foo ( )\n"
                                "2: {\n"
                                "3: char c@1 ( 'c' ) ;\n"
                                "4: return c@1 ;\n"
                                "5: }\n";

        ASSERT_EQUALS(expected, tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid18() {
        const char code[] ="char foo(char c)\n"
                            "{\n"
                            "    bar::c = c;\n"
                            "}\n";

        const char expected[] = "1: char foo ( char c@1 )\n"
                                "2: {\n"
                                "3: bar :: c = c@1 ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid19() {
        const char code[] ="void foo()\n"
                            "{\n"
                            "    std::pair<std::vector<double>, int> x;\n"
                            "}\n";

        const char expected[] = "1: void foo ( )\n"
                                "2: {\n"
                                "3: std :: pair < std :: vector < double > , int > x@1 ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid20() {
        const char code[] ="void foo()\n"
                            "{\n"
                            "    pair<vector<int>, vector<double> > x;\n"
                            "}\n";

        const char expected[] = "1: void foo ( )\n"
                                "2: {\n"
                                "3: pair < vector < int > , vector < double > > x@1 ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid24() {
        const char code[] ="class foo()\n"
                            "{\n"
                            "public:\n"
                            "    ;\n"
                            "private:\n"
                            "    static int i;\n"
                            "};\n";

        const char expected[] = "1: class foo ( )\n"
                                "2: {\n"
                                "3: public:\n"
                                "4: ;\n"
                                "5: private:\n"
                                "6: static int i@1 ;\n"
                                "7: } ;\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid25() {
        const char code[] ="class foo()\n"
                            "{\n"
                            "public:\n"
                            "    ;\n"
                            "private:\n"
                            "    mutable int i;\n"
                            "};\n";

        const char expected[] = "1: class foo ( )\n"
                                "2: {\n"
                                "3: public:\n"
                                "4: ;\n"
                                "5: private:\n"
                                "6: mutable int i@1 ;\n"
                                "7: } ;\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid26() {
        const char code[] ="list<int (*)()> functions;\n";
        const char expected[] = "1: list < int ( * ) ( ) > functions@1 ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid27() {
        const char code[] ="int fooled_ya;\n"
                            "fooled_ya::iterator iter;\n";
        const char expected[] = "1: int fooled_ya@1 ;\n"
                                "2: fooled_ya :: iterator iter@2 ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid28() { // ticket #2630 (segmentation fault)
        ASSERT_THROW_INTERNAL(tokenize("template <typedef A>\n"), SYNTAX);
    }

    void varid29() {
        const char code[] ="class A {\n"
                            "    B<C<1>,1> b;\n"
                            "};\n";
        const char expected[] = "1: class A {\n"
                                "2: B < C < 1 > , 1 > b@1 ;\n"
                                "3: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid30() { // ticket #2614
        const char code1[] = "void f(EventPtr *eventP, ActionPtr **actionsP)\n"
                             "{\n"
                             "    EventPtr event = *eventP;\n"
                             "    *actionsP = &event->actions;\n"
                             "}\n";
        const char expected1[] = "1: void f ( EventPtr * eventP@1 , ActionPtr * * actionsP@2 )\n"
                                 "2: {\n"
                                 "3: EventPtr event@3 ; event@3 = * eventP@1 ;\n"
                                 "4: * actionsP@2 = & event@3 . actions@4 ;\n"
                                 "5: }\n";
        ASSERT_EQUALS(expected1, tokenize(code1, dinit(TokenizeOptions, $.cpp = false)));

        const char code2[] = "void f(int b, int c) {\n"
                             "    x(a*b*c,10);\n"
                             "}\n";
        const char expected2[] = "1: void f ( int b@1 , int c@2 ) {\n"
                                 "2: x ( a * b@1 * c@2 , 10 ) ;\n"
                                 "3: }\n";
        ASSERT_EQUALS(expected2, tokenize(code2, dinit(TokenizeOptions, $.cpp = false)));

        const char code3[] = "class Nullpointer : public ExecutionPath\n"
                             " {\n"
                             "    Nullpointer(Check *c, const unsigned int id, const std::string &name)\n"
                             "        : ExecutionPath(c, id)\n"
                             "    {\n"
                             "    }\n"
                             "};\n";
        const char expected3[] = "1: class Nullpointer : public ExecutionPath\n"
                                 "2: {\n"
                                 "3: Nullpointer ( Check * c@1 , const unsigned int id@2 , const std :: string & name@3 )\n"
                                 "4: : ExecutionPath ( c@1 , id@2 )\n"
                                 "5: {\n"
                                 "6: }\n"
                                 "7: } ;\n";
        ASSERT_EQUALS(expected3, tokenize(code3));
    }

    void varid34() { // ticket #2825
        const char code[] ="class Fred : public B1, public B2\n"
                            "{\n"
                            "public:\n"
                            "    Fred() { a = 0; }\n"
                            "private:\n"
                            "    int a;\n"
                            "};\n";
        const char expected[] = "1: class Fred : public B1 , public B2\n"
                                "2: {\n"
                                "3: public:\n"
                                "4: Fred ( ) { a@1 = 0 ; }\n"
                                "5: private:\n"
                                "6: int a@1 ;\n"
                                "7: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
        ASSERT_EQUALS("", errout_str());
    }

    void varid35() { // function declaration inside function body
        // #2937
        const char code[] ="int foo() {\n"
                            "    int f(x);\n"
                            "    return f;\n"
                            "}\n";
        const char expected[] = "1: int foo ( ) {\n"
                                "2: int f@1 ( x ) ;\n"
                                "3: return f@1 ;\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenize(code));

        // #4627
        const char code2[] = "void f() {\n"
                             "  int  *p;\n"
                             "  void bar(int *p);\n"
                             "}";
        const char expected2[] = "1: void f ( ) {\n"
                                 "2: int * p@1 ;\n"
                                 "3: void bar ( int * p ) ;\n"
                                 "4: }\n";
        ASSERT_EQUALS(expected2, tokenize(code2));

        // #7740
        const char code3[] = "Float f(float scale) {\n"
                             "    return Float(val * scale);\n"
                             "}\n";
        const char expected3[] = "1: Float f ( float scale@1 ) {\n"
                                 "2: return Float ( val * scale@1 ) ;\n"
                                 "3: }\n";
        ASSERT_EQUALS(expected3, tokenize(code3));
    }

    void varid36() { // ticket #2980 (segmentation fault)
        const char code[] ="#elif A\n"
                            "A,a<b<x0\n";
        (void)tokenize(code);
        ASSERT_EQUALS("", errout_str());
    }

    void varid37() {
        {
            const char code[] = "void blah() {"
                                "    Bar bar(*x);"
                                "}";
            ASSERT_EQUALS("1: void blah ( ) { Bar bar@1 ( * x ) ; }\n",
                          tokenize(code));
        }
        {
            const char code[] = "void blah() {"
                                "    Bar bar(&x);"
                                "}";
            ASSERT_EQUALS("1: void blah ( ) { Bar bar@1 ( & x ) ; }\n",
                          tokenize(code));
        }
    }

    void varid38() {
        const char code[] = "FOO class C;\n";
        ASSERT_EQUALS("1: FOO class C ;\n",
                      tokenize(code));
    }

    void varid39() {
        // const..
        {
            const char code[] = "void f(FOO::BAR const);\n";
            ASSERT_EQUALS("1: void f ( const FOO :: BAR ) ;\n",
                          tokenize(code));
        }
        {
            const char code[] = "static int const SZ = 22;\n";
            ASSERT_EQUALS("1: static const int SZ@1 = 22 ;\n",
                          tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
        }
    }

    void varid40() {
        const char code[] ="extern \"C\" int (*a())();";
        ASSERT_EQUALS("1: int * a ( ) ;\n",
                      tokenize(code));
    }

    void varid41() {
        const char code1[] = "union evt; void f(const evt & event);";
        ASSERT_EQUALS("1: union evt ; void f ( const evt & event@1 ) ;\n",
                      tokenize(code1));
        ASSERT_THROW_INTERNAL(tokenize(code1, dinit(TokenizeOptions, $.cpp = false)), SYNTAX);

        const char code2[] = "struct evt; void f(const evt & event);";
        ASSERT_EQUALS("1: struct evt ; void f ( const evt & event@1 ) ;\n",
                      tokenize(code2));
        ASSERT_THROW_INTERNAL(tokenize(code2, dinit(TokenizeOptions, $.cpp = false)), SYNTAX);
    }

    void varid42() {
        const char code[] ="namespace fruit { struct banana {}; };\n"
                            "class Fred {\n"
                            "public:\n"
                            "     struct fruit::banana Bananas[25];\n"
                            "};";
        ASSERT_EQUALS("1: namespace fruit { struct banana { } ; } ;\n"
                      "2: class Fred {\n"
                      "3: public:\n"
                      "4: struct fruit :: banana Bananas@1 [ 25 ] ;\n"
                      "5: } ;\n",
                      tokenize(code));
    }

    void varid43() {
        const char code[] ="int main(int flag) { if(a & flag) { return 1; } }";
        ASSERT_EQUALS("1: int main ( int flag@1 ) { if ( a & flag@1 ) { return 1 ; } }\n",
                      tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid44() {
        const char code[] ="class A:public B,public C,public D {};";
        ASSERT_EQUALS("1: class A : public B , public C , public D { } ;\n",
                      tokenize(code));
    }

    void varid45() { // #3466
        const char code[] ="void foo() { B b(this); A a(this, b); }";
        ASSERT_EQUALS("1: void foo ( ) { B b@1 ( this ) ; A a@2 ( this , b@1 ) ; }\n",
                      tokenize(code));
    }

    void varid46() { // #3756
        const char code[] ="void foo() { int t; x = (struct t *)malloc(); f(t); }";
        ASSERT_EQUALS("1: void foo ( ) { int t@1 ; x = ( struct t * ) malloc ( ) ; f ( t@1 ) ; }\n",
                      tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid47() { // function parameters
        // #3768
        {
            const char code[] ="void f(std::string &string, std::string &len) {}";
            ASSERT_EQUALS("1: void f ( std :: string & string@1 , std :: string & len@2 ) { }\n",
                          tokenize(code));
        }

        // #4729
        {
            const char code[] = "int x;\n"
                                "void a(int x);\n"
                                "void b() { x = 0; }\n";
            ASSERT_EQUALS("1: int x@1 ;\n"
                          "2: void a ( int x@2 ) ;\n"
                          "3: void b ( ) { x@1 = 0 ; }\n",
                          tokenize(code));
        }
    }

    void varid48() {  // #3785 - return (a*b)
        const char code[] ="int X::f(int b) const { return(a*b); }";
        ASSERT_EQUALS("1: int X :: f ( int b@1 ) const { return ( a * b@1 ) ; }\n",
                      tokenize(code));
    }

    void varid49() {  // #3799 - void f(std::vector<int>)
        const char code[] ="void f(std::vector<int>)";
        ASSERT_EQUALS("1: void f ( std :: vector < int > )\n",
                      tokenize(code));
    }

    void varid50() {  // #3760 - explicit
        const char code[] ="class A { explicit A(const A&); };";
        ASSERT_EQUALS("1: class A { explicit A ( const A & ) ; } ;\n",
                      tokenize(code));
    }

    void varid51() {  // don't set varid on template function
        const char code[] ="T t; t.x<0>();";
        ASSERT_EQUALS("1: T t@1 ; t@1 . x < 0 > ( ) ;\n",
                      tokenize(code));
    }

    void varid52() {
        const char code[] ="A<B<C>::D> e;\n"
                            "B< C<> > b[10];\n"
                            "B<C<>> c[10];";
        ASSERT_EQUALS("1: A < B < C > :: D > e@1 ;\n"
                      "2: B < C < > > b@2 [ 10 ] ;\n"
                      "3: B < C < > > c@3 [ 10 ] ;\n",
                      tokenize(code));
    }

    void varid53() { // #4172 - Template instantiation: T<&functionName> list[4];
        ASSERT_EQUALS("1: A < & f > list@1 [ 4 ] ;\n",
                      tokenize("A<&f> list[4];"));
    }

    void varid54() { // hang
        // Original source code: libgc
        (void)tokenize("STATIC ptr_t GC_approx_sp(void) { word sp; sp = (word)&sp; return((ptr_t)sp); }");
    }

    void varid55() { // Ticket #5868
        const char code[] =     "typedef struct foo {} foo; "
                            "void bar1(struct foo foo) {} "
                            "void baz1(foo foo) {} "
                            "void bar2(struct foo& foo) {} "
                            "void baz2(foo& foo) {} "
                            "void bar3(struct foo* foo) {} "
                            "void baz3(foo* foo) {}";
        const char expected[] = "1: "
                                "struct foo { } ; "
                                "void bar1 ( struct foo foo@1 ) { } "
                                "void baz1 ( struct foo foo@2 ) { } "
                                "void bar2 ( struct foo & foo@3 ) { } "
                                "void baz2 ( struct foo & foo@4 ) { } "
                                "void bar3 ( struct foo * foo@5 ) { } "
                                "void baz3 ( struct foo * foo@6 ) { }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid56() { // Ticket #6548 - function with a throw()
        const char code1[] = "void fred(int x) throw() {}"
                             "void wilma() { x++; }";
        const char expected1[] = "1: "
                                 "void fred ( int x@1 ) throw ( ) { } "
                                 "void wilma ( ) { x ++ ; }\n";
        ASSERT_EQUALS(expected1, tokenize(code1));

        const char code2[] = "void fred(int x) const throw(EXCEPT) {}"
                             "void wilma() { x++; }";
        const char expected2[] = "1: "
                                 "void fred ( int x@1 ) const throw ( EXCEPT ) { } "
                                 "void wilma ( ) { x ++ ; }\n";
        ASSERT_EQUALS(expected2, tokenize(code2));

        const char code3[] = "void fred(int x) throw() ABCD {}"
                             "void wilma() { x++; }";
        const char expected3[] = "1: "
                                 "void fred ( int x@1 ) throw ( ) { } "
                                 "void wilma ( ) { x ++ ; }\n";
        ASSERT_EQUALS(expected3, tokenize(code3));

        const char code4[] = "void fred(int x) noexcept() {}"
                             "void wilma() { x++; }";
        const char expected4[] = "1: "
                                 "void fred ( int x@1 ) noexcept ( ) { } "
                                 "void wilma ( ) { x ++ ; }\n";
        ASSERT_EQUALS(expected4, tokenize(code4));

        const char code5[] = "void fred(int x) noexcept {}"
                             "void wilma() { x++; }";
        const char expected5[] = "1: "
                                 "void fred ( int x@1 ) noexcept ( true ) { } "
                                 "void wilma ( ) { x ++ ; }\n";
        ASSERT_EQUALS(expected5, tokenize(code5));

        const char code6[] = "void fred(int x) noexcept ( false ) {}"
                             "void wilma() { x++; }";
        const char expected6[] = "1: "
                                 "void fred ( int x@1 ) noexcept ( false ) { } "
                                 "void wilma ( ) { x ++ ; }\n";
        ASSERT_EQUALS(expected6, tokenize(code6));
    }

    void varid57() { // #6636: new scope by {}
        const char code1[] = "void SmoothPath() {\n"
                             "    {\n" // new scope
                             "        float dfx = (p2p0.x > 0.0f)?\n"
                             "                    ((n0->xmax() * SQUARE_SIZE) - p0.x):\n"
                             "                    ((n0->xmin() * SQUARE_SIZE) - p0.x);\n"
                             "        float tx = dfx / dx;\n"
                             "        if (hEdge) {\n"
                             "        }\n"
                             "        if (vEdge) {\n"
                             "            pi.z = tx;\n"
                             "        }\n"
                             "    }\n"
                             "}\n";
        const char expected1[] = "1: void SmoothPath ( ) {\n"
                                 "2:\n"
                                 "3: float dfx@1 ; dfx@1 = ( p2p0 . x > 0.0f ) ?\n"
                                 "4: ( ( n0 . xmax ( ) * SQUARE_SIZE ) - p0 . x ) :\n"
                                 "5: ( ( n0 . xmin ( ) * SQUARE_SIZE ) - p0 . x ) ;\n"
                                 "6: float tx@2 ; tx@2 = dfx@1 / dx ;\n"
                                 "7: if ( hEdge ) {\n"
                                 "8: }\n"
                                 "9: if ( vEdge ) {\n"
                                 "10: pi . z = tx@2 ;\n"
                                 "11: }\n"
                                 "12:\n"
                                 "13: }\n";
        ASSERT_EQUALS(expected1, tokenize(code1));
    }

    void varid58() { // #6638: for loop in for condition
        const char code1[] = "void f() {\n"
                             "    for (int i;\n"
                             "         ({for(int i;i;++i){i++;}i++;}),i;\n"
                             "         ({for(int i;i;++i){i++;}i++;}),i++) {\n"
                             "         i++;\n"
                             "    }\n"
                             "}\n";
        const char expected1[] = "1: void f ( ) {\n"
                                 "2: for ( int i@1 ;\n"
                                 "3: ( { for ( int i@2 ; i@2 ; ++ i@2 ) { i@2 ++ ; } i@1 ++ ; } ) , i@1 ;\n"
                                 "4: ( { for ( int i@3 ; i@3 ; ++ i@3 ) { i@3 ++ ; } i@1 ++ ; } ) , i@1 ++ ) {\n"
                                 "5: i@1 ++ ;\n"
                                 "6: }\n"
                                 "7: }\n";
        ASSERT_EQUALS(expected1, tokenize(code1));
    }

    void varid59() { // #6696
        const char code[] = "class DLLSYM B;\n"
                            "struct B {\n"
                            "    ~B() {}\n"
                            "};";
        const char expected[] = "1: class DLLSYM B@1 ;\n" // In this line, we cannot really do better...
                                "2: struct B {\n"
                                "3: ~ B@1 ( ) { }\n" // ...but here we could
                                "4: } ;\n";
        const char wanted[] = "1: class DLLSYM B@1 ;\n"
                              "2: struct B {\n"
                              "3: ~ B ( ) { }\n"
                              "4: } ;\n";
        TODO_ASSERT_EQUALS(wanted, expected, tokenize(code));
    }

    void varid60() { // #7267 - cast
        ASSERT_EQUALS("1: a = ( x y ) 10 ;\n",
                      tokenize("a=(x y)10;"));
    }

    void varid61() {
        const char code[] = "void foo(int b) {\n"
                            "  void bar(int a, int b) {}\n"
                            "}";
        const char expected[] = "1: void foo ( int b@1 ) {\n"
                                "2: void bar ( int a@2 , int b@3 ) { }\n"
                                "3: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid62() {
        const char code[] = "void bar(int,int);\n"
                            "void f() {\n"
                            "    for (size_t c = 0; c < 42; ++c) {\n"
                            "        int x;\n"
                            "        bar(r, r * x);\n"
                            "    }\n"
                            "}";
        // Ensure that there is only one variable id for "x"
        ASSERT_EQUALS("same varid", compareVaridsForVariable(code, "x"));
    }

    void varid63() {
        const char code[] = "void f(boost::optional<int> const& x) {}";
        const char expected[] = "1: void f ( const boost :: optional < int > & x@1 ) { }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid64() {
        const char code[] = "extern const char (*x[256]);";
        const char expected[] = "1: extern const char ( * x@1 [ 256 ] ) ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid65() { // #10936
        {
            const char code[] = "extern int (*p);";
            const char expected[] = "1: extern int ( * p@1 ) ;\n";
            ASSERT_EQUALS(expected, tokenize(code));
        }
        {
            const char code[] = "extern int (i);";
            const char expected[] = "1: extern int ( i@1 ) ;\n";
            ASSERT_EQUALS(expected, tokenize(code));
        }
        {
            const char code[] = "int (*p);";
            const char expected[] = "1: int ( * p@1 ) ;\n";
            ASSERT_EQUALS(expected, tokenize(code));
        }
        {
            const char code[] = "int (i);";
            const char expected[] = "1: int ( i@1 ) ;\n";
            ASSERT_EQUALS(expected, tokenize(code));
        }
    }

    void varid66() {
        {
            const char code[] = "std::string g();\n"
                                "const std::string s(g() + \"abc\");\n";
            const char expected[] = "1: std :: string g ( ) ;\n"
                                    "2: const std :: string s@1 ( g ( ) + \"abc\" ) ;\n";
            ASSERT_EQUALS(expected, tokenize(code));
        }
        {
            const char code[] = "enum E {};\n"
                                "typedef E(*fp_t)();\n"
                                "E f(fp_t fp);\n";
            const char expected[] = "1: enum E { } ;\n"
                                    "2:\n"
                                    "3: E f ( E ( * fp@1 ) ( ) ) ;\n";
            ASSERT_EQUALS(expected, tokenize(code));
        }
    }

    void varid67() { // #11711
        const char code1[] = "int *x;\n"
                             "_Generic(*x, int: foo, default: bar)();";
        const char expected1[] = "1: int * x@1 ;\n"
                                 "2: _Generic ( * x@1 , int : foo , default : bar ) ( ) ;\n";
        ASSERT_EQUALS(expected1, tokenize(code1, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid68() { // #11740
        const char code1[] = "struct S {};\n"
                             "char* str_chars(struct S* s);\n"
                             "void f(struct S strOut) {\n"
                             "	 switch (str_chars(&strOut)[0])  {}\n"
                             "}";
        const char expected1[] = "1: struct S { } ;\n"
                                 "2: char * str_chars ( struct S * s@1 ) ;\n"
                                 "3: void f ( struct S strOut@2 ) {\n"
                                 "4: switch ( str_chars ( & strOut@2 ) [ 0 ] ) { }\n"
                                 "5: }\n";
        ASSERT_EQUALS(expected1, tokenize(code1, dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS(expected1, tokenize(code1));
    }

    void varid69() {
        const char code1[] = "void f() {\n"
                             "    auto g = [](int&, int& r, int i) {};\n"
                             "}";
        const char expected1[] = "1: void f ( ) {\n"
                                 "2: auto g@1 ; g@1 = [ ] ( int & , int & r@2 , int i@3 ) { } ;\n"
                                 "3: }\n";
        ASSERT_EQUALS(expected1, tokenize(code1));
    }

    void varid70() {
        // isFunctionHead
        const char code1[] = "int x = 1 ? (1 << 0) : 0;\n"
                             "void foo(bool init);\n"
                             "void init();\n";
        const char expected1[] = "1: int x@1 ; x@1 = 1 ? ( 1 << 0 ) : 0 ;\n"
                                 "2: void foo ( bool init@2 ) ;\n"
                                 "3: void init ( ) ;\n";
        ASSERT_EQUALS(expected1, tokenize(code1));

        const char code2[] = "int x = 1 ? f(1 << 0) : 0;\n"
                             "void foo(bool init);\n"
                             "void init();\n";
        const char expected2[] = "1: int x@1 ; x@1 = 1 ? f ( 1 << 0 ) : 0 ;\n"
                                 "2: void foo ( bool init@2 ) ;\n"
                                 "3: void init ( ) ;\n";
        ASSERT_EQUALS(expected2, tokenize(code2));

        const char code3[] = "extern void (*arr[10])(uint32_t some);\n";
        const char expected3[] = "1: extern void ( * arr@1 [ 10 ] ) ( uint32_t some@2 ) ;\n";
        ASSERT_EQUALS(expected3, tokenize(code3));

        const char code4[] = "_Static_assert(sizeof((struct S){0}.i) == 4);\n"; // #12729
        const char expected4[] = "1: _Static_assert ( sizeof ( ( struct S ) { 0 } . i ) == 4 ) ;\n";
        ASSERT_EQUALS(expected4, tokenize(code4, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid71() {
        const char code[] = "namespace myspace {\n"
                            "\n"
                            "template <typename T>\n"
                            "class CounterTest {\n"
                            "public:\n"
                            "  CounterTest(T _obj);\n"
                            "  template <typename T2>\n"
                            "  CounterTest(const CounterTest<T2>& ptr);\n"
                            "  T obj;\n"
                            "  int count;\n"
                            "};\n"
                            "\n"
                            "template <typename T>\n"
                            "CounterTest<T>::CounterTest(T _obj) : obj(_obj) {\n"
                            "  count = 0;\n"
                            "}\n"
                            "\n"
                            "template <typename T>\n"
                            "template <typename T2>\n"
                            "CounterTest<T>::CounterTest(const CounterTest<T2>& p) : obj(0) {\n"
                            "  count = p.count;\n"
                            "}\n"
                            "\n"
                            "}\n"
                            "\n"
                            "using namespace myspace;\n"
                            "CounterTest<int> myobj(0);\n";
        const char expected[] = "1: namespace myspace {\n"
                                "2:\n"
                                "3: class CounterTest<int> ;\n"
                                "4:\n"
                                "|\n"
                                "17:\n"
                                "18: template < typename T >\n"
                                "19: template < typename T2 >\n"
                                "20: CounterTest < T > :: CounterTest ( const CounterTest < T2 > & p@1 ) : obj ( 0 ) {\n"
                                "21: count = p@1 . count@2 ;\n"
                                "22: }\n"
                                "23:\n"
                                "24: }\n"
                                "25:\n"
                                "26: using namespace myspace ;\n"
                                "27: myspace :: CounterTest<int> myobj@3 ( 0 ) ;\n"
                                "4: class myspace :: CounterTest<int> {\n"
                                "5: public:\n"
                                "6: CounterTest<int> ( int _obj@4 ) ;\n"
                                "7: template < typename T2 >\n"
                                "8: CounterTest<int> ( const myspace :: CounterTest < T2 > & ptr@5 ) ;\n"
                                "9: int obj@6 ;\n"
                                "10: int count@7 ;\n"
                                "11: } ;\n"
                                "12:\n"
                                "13:\n"
                                "14: myspace :: CounterTest<int> :: CounterTest<int> ( int _obj@8 ) : obj@6 ( _obj@8 ) {\n"
                                "15: count@7 = 0 ;\n"
                                "16: }\n"
                                "17:\n"
                                "18:\n"
                                "19:\n"
                                "20: myspace :: CounterTest<int> :: CounterTest<int> ( const myspace :: CounterTest < T2 > & p@9 ) : obj@6 ( 0 ) {\n"
                                "21: count@7 = p@9 . count@10 ;\n"
                                "22: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid72() {
        const char code1[] = "static_assert(true && true);\n"; // #14386
        const char expected1[] = "1: static_assert ( true && true ) ;\n";
        ASSERT_EQUALS(expected1, tokenize(code1));
    }

    void varid_for_1() {
        const char code[] = "void foo(int a, int b) {\n"
                            "  for (int a=1,b=2;;) {}\n"
                            "}";
        const char expected[] = "1: void foo ( int a@1 , int b@2 ) {\n"
                                "2: for ( int a@3 = 1 , b@4 = 2 ; ; ) { }\n"
                                "3: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_for_2() {
        const char code[] = "void foo(int a, int b) {\n"
                            "  for (int a=f(x,y,z),b=2;;) {}\n"
                            "}";
        const char expected[] = "1: void foo ( int a@1 , int b@2 ) {\n"
                                "2: for ( int a@3 = f ( x , y , z ) , b@4 = 2 ; ; ) { }\n"
                                "3: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_for_3() {
        const char code[] = "void f(int w, int h) {\n" // #13670
                            "    for (int a[2] = { 2, w / 2 }, b = 2; a[1] && a[0] <= h; a[1] /= 2, a[0] *= 2) {}\n"
                            "}\n";
        const char expected[] = "1: void f ( int w@1 , int h@2 ) {\n"
                                "2: for ( int a@3 [ 2 ] = { 2 , w@1 / 2 } , b@4 = 2 ; a@3 [ 1 ] && a@3 [ 0 ] <= h@2 ; a@3 [ 1 ] /= 2 , a@3 [ 0 ] *= 2 ) { }\n"
                                "3: }\n";
        ASSERT_EQUALS(expected, tokenize(code));

        const char code2[] = "void f() {\n"
                             "    for (int a(1), b{ 2 }, c = 3; a < b + c; ++a) {}\n"
                             "    for (int a{ 1 }, b(2), c = 3; a < b + c; ++a) {}\n"
                             "    for (int a = 1, b{ 2 }, c(3); a < b + c; ++a) {}\n"
                             "}\n";
        const char expected2[] = "1: void f ( ) {\n"
                                 "2: for ( int a@1 ( 1 ) , b@2 { 2 } , c@3 = 3 ; a@1 < b@2 + c@3 ; ++ a@1 ) { }\n"
                                 "3: for ( int a@4 { 1 } , b@5 ( 2 ) , c@6 = 3 ; a@4 < b@5 + c@6 ; ++ a@4 ) { }\n"
                                 "4: for ( int a@7 = 1 , b@8 { 2 } , c@9 ( 3 ) ; a@7 < b@8 + c@9 ; ++ a@7 ) { }\n"
                                 "5: }\n";
        ASSERT_EQUALS(expected2, tokenize(code2));
    }

    void varid_cpp_keywords_in_c_code() {
        const char code[] = "void f() {\n"
                            "    delete d;\n"
                            "    throw t;\n"
                            "}";

        const char expected[] = "1: void f ( ) {\n"
                                "2: delete d@1 ;\n"
                                "3: throw t@2 ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid_cpp_keywords_in_c_code2() { // #5373
        const char code[] = "int clear_extent_bit(struct extent_io_tree *tree, u64 start, u64 end, "
                            "unsigned long bits, int wake, int delete, struct extent_state **cached_state, "
                            "gfp_t mask) {\n"
                            "  struct extent_state *state;\n"
                            "}"
                            "int clear_extent_dirty() {\n"
                            "  return clear_extent_bit(tree, start, end, EXTENT_DIRTY | EXTENT_DELALLOC | "
                            "                          EXTENT_DO_ACCOUNTING, 0, 0, NULL, mask);\n"
                            "}";
        ASSERT_NO_THROW(tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid_cpp_keywords_in_c_code3() { // #12120
        const char code[] = "const struct class *p;";
        const char expected[] = "1: const struct class * p@1 ;\n";
        ASSERT_EQUALS(expected, tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid_cpp_keywords_in_c_code4() { // #12120
        {
            const char code[] = "int false = 0;";
            ASSERT_THROW_INTERNAL_EQUALS(tokenize(code, dinit(TokenizeOptions, $.cpp = true)), INTERNAL, "Internal error. VarId set for bool literal.");
        }
        {
            const char code[] = "int false = 0;";
            ASSERT_EQUALS("1: int false@1 ; false@1 = 0 ;\n", tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
        }
    }

    void varidFunctionCall1() {
        const char code[] ="void f() {\n"
                            "    int x;\n"
                            "    x = a(y*x,10);\n"
                            "}";
        const char expected[] = "1: void f ( ) {\n"
                                "2: int x@1 ;\n"
                                "3: x@1 = a ( y * x@1 , 10 ) ;\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varidFunctionCall2() {
        // #2491
        const char code[] ="void f(int b) {\n"
                            "    x(a*b,10);\n"
                            "}";
        const std::string expected1("1: void f ( int b@1 ) {\n"
                                    "2: x ( a * b");
        const std::string expected2(" , 10 ) ;\n"
                                    "3: }\n");
        ASSERT_EQUALS(expected1+"@1"+expected2, tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varidFunctionCall3() {
        // Ticket #2339
        const char code[] ="void f() {\n"
                            "    int a = 0;\n"
                            "    int b = c - (foo::bar * a);\n"
                            "}";

        const char expected[] = "1: void f ( ) {\n"
                                "2: int a@1 ; a@1 = 0 ;\n"
                                "3: int b@2 ; b@2 = c - ( foo :: bar * a@1 ) ;\n"
                                "4: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidFunctionCall4() {
        // Ticket #3280
        const char code1[] = "void f() { int x; fun(a,b*x); }";
        ASSERT_EQUALS("1: void f ( ) { int x@1 ; fun ( a , b * x@1 ) ; }\n",
                      tokenize(code1, dinit(TokenizeOptions, $.cpp = false)));
        const char code2[] = "void f(int a) { int x; fun(a,b*x); }";
        ASSERT_EQUALS("1: void f ( int a@1 ) { int x@2 ; fun ( a@1 , b * x@2 ) ; }\n",
                      tokenize(code2, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varidFunctionCall5() {
        const char code[] = "void foo() { (f(x[2]))(x[2]); }";
        ASSERT_EQUALS("1: void foo ( ) { f ( x [ 2 ] ) ( x [ 2 ] ) ; }\n",
                      tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varidStl() {
        const std::string actual = tokenize(
            "list<int> ints;\n"
            "list<int>::iterator it;\n"
            "std::vector<std::string> dirs;\n"
            "std::map<int, int> coords;\n"
            "std::tr1::unordered_map<int, int> xy;\n"
            "std::list<boost::wave::token_id> tokens;\n"
            "static std::vector<CvsProcess*> ex1;\n"
            "extern std::vector<CvsProcess*> ex2;\n"
            "std::map<int, 1> m;");

        const char expected[] = "1: list < int > ints@1 ;\n"
                                "2: list < int > :: iterator it@2 ;\n"
                                "3: std :: vector < std :: string > dirs@3 ;\n"
                                "4: std :: map < int , int > coords@4 ;\n"
                                "5: std :: tr1 :: unordered_map < int , int > xy@5 ;\n"
                                "6: std :: list < boost :: wave :: token_id > tokens@6 ;\n"
                                "7: static std :: vector < CvsProcess * > ex1@7 ;\n"
                                "8: extern std :: vector < CvsProcess * > ex2@8 ;\n"
                                "9: std :: map < int , 1 > m@9 ;\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varidStl2() {
        const std::string actual = tokenize("std::bitset<static_cast<int>(2)> x;");

        const char expected[] = "1: std :: bitset < static_cast < int > ( 2 ) > x@1 ;\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid_newauto() {
        ASSERT_EQUALS("1: void f ( ) { const new auto ( 0 ) ; }\n",
                      tokenize("void f(){new const auto(0);}"));
    }

    void varid_delete() {
        const std::string actual = tokenize(
            "void f()\n"
            "{\n"
            "  int *a;\n"
            "  delete a;\n"
            "}");

        const char expected[] = "1: void f ( )\n"
                                "2: {\n"
                                "3: int * a@1 ;\n"
                                "4: delete a@1 ;\n"
                                "5: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid_functions() {
        {
            const std::string actual = tokenize(
                "void f();\n"
                "void f(){}\n", dinit(TokenizeOptions, $.cpp = false));

            const char expected[] = "1: void f ( ) ;\n"
                                    "2: void f ( ) { }\n";

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                "A f(3);\n"
                "A f2(true);\n"
                "A g();\n"
                "A e(int c);\n", dinit(TokenizeOptions, $.cpp = false));

            const char expected[] = "1: A f@1 ( 3 ) ;\n"
                                    "2: A f2@2 ( true ) ;\n"
                                    "3: A g ( ) ;\n"
                                    "4: A e ( int c@3 ) ;\n";

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                "void f1(int &p)\n"
                "{\n"
                "    p = 0;\n"
                "}\n"
                "void f2(std::string &str)\n"
                "{\n"
                "   str.clear();\n"
                "}\n"
                "void f3(const std::string &s)\n"
                "{\n"
                "    s.size();\n"
                "}");

            const char expected[] = "1: void f1 ( int & p@1 )\n"
                                    "2: {\n"
                                    "3: p@1 = 0 ;\n"
                                    "4: }\n"
                                    "5: void f2 ( std :: string & str@2 )\n"
                                    "6: {\n"
                                    "7: str@2 . clear ( ) ;\n"
                                    "8: }\n"
                                    "9: void f3 ( const std :: string & s@3 )\n"
                                    "10: {\n"
                                    "11: s@3 . size ( ) ;\n"
                                    "12: }\n";

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize("void f(struct foobar);", dinit(TokenizeOptions, $.cpp = false));
            const char expected[] = "1: void f ( struct foobar ) ;\n";
            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize("bool f(X x, int=3);");
            const char expected[] = "1: bool f ( X x@1 , int = 3 ) ;\n";
            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize("int main() {\n"
                                                "    int a[2];\n"
                                                "    extern void f(int a[2]);\n"
                                                "    f(a);\n"
                                                "    a[0] = 0;\n"
                                                "}\n");
            const char expected[] = "1: int main ( ) {\n"
                                    "2: int a@1 [ 2 ] ;\n"
                                    "3: extern void f ( int a [ 2 ] ) ;\n"
                                    "4: f ( a@1 ) ;\n"
                                    "5: a@1 [ 0 ] = 0 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize("void f(int n) {\n" // #12537
                                                "    int n1;\n"
                                                "    void g(int is, int n1);\n"
                                                "    n1 = n - 1;\n"
                                                "}\n");
            const char expected[] = "1: void f ( int n@1 ) {\n"
                                    "2: int n1@2 ;\n"
                                    "3: void g ( int is , int n1 ) ;\n"
                                    "4: n1@2 = n@1 - 1 ;\n"
                                    "5: }\n";
            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid_sizeof() {
        const char code[] = "x = sizeof(a*b);";
        const char expected[] = "1: x = sizeof ( a * b ) ;\n";
        ASSERT_EQUALS(expected, tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid_reference_to_containers() {
        const std::string actual = tokenize(
            "void f()\n"
            "{\n"
            "    std::vector<int> b;\n"
            "    std::vector<int> &a = b;\n"
            "    std::vector<int> *c = &b;\n"
            "}");

        const char expected[] = "1: void f ( )\n"
                                "2: {\n"
                                "3: std :: vector < int > b@1 ;\n"
                                "4: std :: vector < int > & a@2 = b@1 ;\n"
                                "5: std :: vector < int > * c@3 ; c@3 = & b@1 ;\n"
                                "6: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid_in_class1() {
        {
            const std::string actual = tokenize(
                "class Foo\n"
                "{\n"
                "public:\n"
                "    std::string name1;\n"
                "    std::string name2;\n"
                "};");

            const char expected[] = "1: class Foo\n"
                                    "2: {\n"
                                    "3: public:\n"
                                    "4: std :: string name1@1 ;\n"
                                    "5: std :: string name2@2 ;\n"
                                    "6: } ;\n";

            ASSERT_EQUALS(expected, actual);
        }

        {
            const std::string actual = tokenize(
                "class foo\n"
                "{\n"
                "public:\n"
                "    void do_something(const int x, const int y);\n"
                "    void bar();\n"
                "};\n"
                "\n"
                "void foo::bar()\n"
                "{\n"
                "    POINT pOutput = { 0 , 0 };\n"
                "    int x = pOutput.x;\n"
                "    int y = pOutput.y;\n"
                "}");

            const char expected[] = "1: class foo\n"
                                    "2: {\n"
                                    "3: public:\n"
                                    "4: void do_something ( const int x@1 , const int y@2 ) ;\n"
                                    "5: void bar ( ) ;\n"
                                    "6: } ;\n"
                                    "7:\n"
                                    "8: void foo :: bar ( )\n"
                                    "9: {\n"
                                    "10: POINT pOutput@3 ; pOutput@3 = { 0 , 0 } ;\n"
                                    "11: int x@4 ; x@4 = pOutput@3 . x@5 ;\n"
                                    "12: int y@6 ; y@6 = pOutput@3 . y@7 ;\n"
                                    "13: }\n";

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid_in_class2() {
        const std::string actual = tokenize(
            "struct Foo {\n"
            "    int x;\n"
            "};\n"
            "\n"
            "struct Bar {\n"
            "    Foo foo;\n"
            "    int x;\n"
            "    void f();\n"
            "};\n"
            "\n"
            "void Bar::f()\n"
            "{\n"
            "    foo.x = x;\n"
            "}");
        const char expected[] = "1: struct Foo {\n"
                                "2: int x@1 ;\n"
                                "3: } ;\n"
                                "4:\n"
                                "5: struct Bar {\n"
                                "6: Foo foo@2 ;\n"
                                "7: int x@3 ;\n"
                                "8: void f ( ) ;\n"
                                "9: } ;\n"
                                "10:\n"
                                "11: void Bar :: f ( )\n"
                                "12: {\n"
                                "13: foo@2 . x@4 = x@3 ;\n"
                                "14: }\n";
        ASSERT_EQUALS(expected, actual);
    }

    void varid_in_class3() {
        const char code[] = "class Foo {\n"
                            "    void blah() {\n"
                            "        Bar x(*this);\n"  // <- ..
                            "    }\n"
                            "    int x;\n"   // <- .. don't assign same varid
                            "};";
        ASSERT_EQUALS("1: class Foo {\n"
                      "2: void blah ( ) {\n"
                      "3: Bar x@1 ( * this ) ;\n"
                      "4: }\n"
                      "5: int x@2 ;\n"
                      "6: } ;\n", tokenize(code));
    }

    void varid_in_class4() {
        const char code[] = "class Foo {\n"
                            "public: class C;\n"
                            "};";
        ASSERT_EQUALS("1: class Foo {\n"
                      "2: public: class C ;\n"
                      "3: } ;\n",
                      tokenize(code));
    }

    void varid_in_class5() {
        const char code[] = "struct Foo {\n"
                            "    std::vector<::X> v;\n"
                            "}";
        ASSERT_EQUALS("1: struct Foo {\n"
                      "2: std :: vector < :: X > v@1 ;\n"
                      "3: }\n",
                      tokenize(code));
    }

    void varid_in_class6() {
        const char code[] = "class A {\n"
                            "    void f(const char *str) const {\n"
                            "        std::stringstream sst;\n"
                            "        sst.str();\n"
                            "    }\n"
                            "};";
        ASSERT_EQUALS("1: class A {\n"
                      "2: void f ( const char * str@1 ) const {\n"
                      "3: std :: stringstream sst@2 ;\n"
                      "4: sst@2 . str ( ) ;\n"
                      "5: }\n"
                      "6: } ;\n",
                      tokenize(code));
    }

    void varid_in_class7() {
        const char code[] = "class A {\n"
                            "    void f() {\n"
                            "        abc.a = 0;\n"
                            "    }\n"
                            "    struct ABC abc;\n"
                            "};";
        ASSERT_EQUALS("1: class A {\n"
                      "2: void f ( ) {\n"
                      "3: abc@1 . a@2 = 0 ;\n"
                      "4: }\n"
                      "5: struct ABC abc@1 ;\n"
                      "6: } ;\n",
                      tokenize(code));
    }

    void varid_in_class8() {  // #3776 - unknown macro
        const char code[] = "class A {\n"
                            "  UNKNOWN_MACRO(A)\n"
                            "private:\n"
                            "  int x;\n"
                            "};";
        ASSERT_EQUALS("1: class A {\n"
                      "2: UNKNOWN_MACRO ( A )\n"
                      "3: private:\n"
                      "4: int x@1 ;\n"
                      "5: } ;\n",
                      tokenize(code));
    }

    void varid_in_class9() {  // #4291 - id for variables accessed through 'this'
        const char code1[] = "class A {\n"
                             "  int var;\n"
                             "public:\n"
                             "  void setVar();\n"
                             "};\n"
                             "void A::setVar() {\n"
                             "  this->var = var;\n"
                             "}";
        ASSERT_EQUALS("1: class A {\n"
                      "2: int var@1 ;\n"
                      "3: public:\n"
                      "4: void setVar ( ) ;\n"
                      "5: } ;\n"
                      "6: void A :: setVar ( ) {\n"
                      "7: this . var@1 = var@1 ;\n"
                      "8: }\n",
                      tokenize(code1));

        const char code2[] = "class Foo : public FooBase {\n"
                             "    void Clone(FooBase& g);\n"
                             "    short m_bar;\n"
                             "};\n"
                             "void Foo::Clone(FooBase& g) {\n"
                             "    g->m_bar = m_bar;\n"
                             "}";
        ASSERT_EQUALS("1: class Foo : public FooBase {\n"
                      "2: void Clone ( FooBase & g@1 ) ;\n"
                      "3: short m_bar@2 ;\n"
                      "4: } ;\n"
                      "5: void Foo :: Clone ( FooBase & g@3 ) {\n"
                      "6: g@3 . m_bar@4 = m_bar@2 ;\n"
                      "7: }\n",
                      tokenize(code2)); // #4311
    }

    void varid_in_class10() {
        const char code[] = "class Foo : public FooBase {\n"
                            "    void Clone(FooBase& g);\n"
                            "    short m_bar;\n"
                            "};\n"
                            "void Foo::Clone(FooBase& g) {\n"
                            "    ((FooBase)g)->m_bar = m_bar;\n"
                            "}";
        ASSERT_EQUALS("1: class Foo : public FooBase {\n"
                      "2: void Clone ( FooBase & g@1 ) ;\n"
                      "3: short m_bar@2 ;\n"
                      "4: } ;\n"
                      "5: void Foo :: Clone ( FooBase & g@3 ) {\n"
                      "6: ( ( FooBase ) g@3 ) . m_bar@4 = m_bar@2 ;\n"
                      "7: }\n",
                      tokenize(code));
    }

    void varid_in_class11() { // #4277 - anonymous union
        const char code1[] = "class Foo {\n"
                             "    union { float a; int b; };\n"
                             "    void f() { a=0; }\n"
                             "};";
        ASSERT_EQUALS("1: class Foo {\n"
                      "2: union { float a@1 ; int b@2 ; } ;\n"
                      "3: void f ( ) { a@1 = 0 ; }\n"
                      "4: } ;\n",
                      tokenize(code1));

        const char code2[] = "class Foo {\n"
                             "    void f() { a=0; }\n"
                             "    union { float a; int b; };\n"
                             "};";
        ASSERT_EQUALS("1: class Foo {\n"
                      "2: void f ( ) { a@1 = 0 ; }\n"
                      "3: union { float a@1 ; int b@2 ; } ;\n"
                      "4: } ;\n",
                      tokenize(code2));

        const char code3[] = "void f() {\n"
                             "    union {\n"
                             "        struct {\n"
                             "            char a, b, c, d;\n"
                             "        };\n"
                             "        int abcd;\n"
                             "    };\n"
                             "    g(abcd);\n"
                             "    h(a, b, c, d);\n"
                             "}";
        ASSERT_EQUALS("1: void f ( ) {\n"
                      "2: union {\n"
                      "3: struct {\n"
                      "4: char a@1 ; char b@2 ; char c@3 ; char d@4 ;\n"
                      "5: } ;\n"
                      "6: int abcd@5 ;\n"
                      "7: } ;\n"
                      "8: g ( abcd@5 ) ;\n"
                      "9: h ( a@1 , b@2 , c@3 , d@4 ) ;\n"
                      "10: }\n",
                      tokenize(code3));

        // #7444
        const char code4[] = "class Foo {\n"
                             "    void f(float a) { this->a = a; }\n"
                             "    union { float a; int b; };\n"
                             "};";
        ASSERT_EQUALS("1: class Foo {\n"
                      "2: void f ( float a@1 ) { this . a@2 = a@1 ; }\n"
                      "3: union { float a@2 ; int b@3 ; } ;\n"
                      "4: } ;\n",
                      tokenize(code4));
    }

    void varid_in_class12() { // #4637 - method
        const char code[] = "class Foo {\n"
                            "private:\n"
                            "    void f(void);\n"
                            "};";
        ASSERT_EQUALS("1: class Foo {\n"
                      "2: private:\n"
                      "3: void f ( ) ;\n"
                      "4: } ;\n",
                      tokenize(code));
    }

    void varid_in_class13() {
        const char code1[] = "struct a { char typename; };";
        ASSERT_EQUALS("1: struct a { char typename@1 ; } ;\n",
                      tokenize(code1, dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("1: struct a { char typename ; } ;\n",  // not valid C++ code
                      tokenize(code1));

        const char code2[] = "struct a { char typename[2]; };";
        ASSERT_EQUALS("1: struct a { char typename@1 [ 2 ] ; } ;\n",
                      tokenize(code2, dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("1: struct a { char typename [ 2 ] ; } ;\n",  // not valid C++ code
                      tokenize(code2));
    }

    void varid_in_class14() {
        const char code[] = "class Tokenizer { TokenList list; };\n"
                            "\n"
                            "void Tokenizer::f() {\n"
                            "  std::list<int> x;\n"               // <- not member variable
                            "  list.do_something();\n"            // <- member variable
                            "  Tokenizer::list.do_something();\n" // <- redundant scope info
                            "}\n";
        ASSERT_EQUALS("1: class Tokenizer { TokenList list@1 ; } ;\n"
                      "2:\n"
                      "3: void Tokenizer :: f ( ) {\n"
                      "4: std :: list < int > x@2 ;\n"
                      "5: list@1 . do_something ( ) ;\n"
                      "6: Tokenizer :: list@1 . do_something ( ) ;\n"
                      "7: }\n", tokenize(code));
    }

    void varid_in_class15() { // #5533 - functions
        const char code[] = "class Fred {\n"
                            "  void x(int a) const;\n"
                            "  void y() { a=0; }\n" // <- unknown variable
                            "}\n";
        ASSERT_EQUALS("1: class Fred {\n"
                      "2: void x ( int a@1 ) const ;\n"
                      "3: void y ( ) { a = 0 ; }\n"
                      "4: }\n", tokenize(code));
    }

    void varid_in_class16() { // Set varId for inline member functions
        {
            const char code[] = "class Fred {\n"
                                "    int x;\n"
                                "    void foo(int x) { this->x = x; }\n"
                                "};\n";
            ASSERT_EQUALS("1: class Fred {\n"
                          "2: int x@1 ;\n"
                          "3: void foo ( int x@2 ) { this . x@1 = x@2 ; }\n"
                          "4: } ;\n", tokenize(code));
        }
        {
            const char code[] = "class Fred {\n"
                                "    void foo(int x) { this->x = x; }\n"
                                "    int x;\n"
                                "};\n";
            ASSERT_EQUALS("1: class Fred {\n"
                          "2: void foo ( int x@1 ) { this . x@2 = x@1 ; }\n"
                          "3: int x@2 ;\n"
                          "4: } ;\n", tokenize(code));
        }
        {
            const char code[] = "class Fred {\n"
                                "    void foo(int x) { (*this).x = x; }\n"
                                "    int x;\n"
                                "};\n";
            ASSERT_EQUALS("1: class Fred {\n"
                          "2: void foo ( int x@1 ) { ( * this ) . x@2 = x@1 ; }\n"
                          "3: int x@2 ;\n"
                          "4: } ;\n", tokenize(code));
        }
    }

    void varid_in_class17() { // #6056 - Set no varid for member functions
        const char code1[] = "class Fred {\n"
                             "    int method_with_internal(X&);\n"
                             "    int method_with_internal(X*);\n"
                             "    int method_with_internal(int&);\n"
                             "    int method_with_internal(A* b, X&);\n"
                             "    int method_with_internal(X&, A* b);\n"
                             "    int method_with_internal(const B &, int);\n"
                             "    void Set(BAR);\n"
                             "    FOO Set(BAR);\n"
                             "    int method_with_class(B<B> b);\n"
                             "    bool function(std::map<int, int, MYless> & m);\n"
                             "};";
        ASSERT_EQUALS("1: class Fred {\n"
                      "2: int method_with_internal ( X & ) ;\n"
                      "3: int method_with_internal ( X * ) ;\n"
                      "4: int method_with_internal ( int & ) ;\n"
                      "5: int method_with_internal ( A * b@1 , X & ) ;\n"
                      "6: int method_with_internal ( X & , A * b@2 ) ;\n"
                      "7: int method_with_internal ( const B & , int ) ;\n"
                      "8: void Set ( BAR ) ;\n"
                      "9: FOO Set ( BAR ) ;\n"
                      "10: int method_with_class ( B < B > b@3 ) ;\n"
                      "11: bool function ( std :: map < int , int , MYless > & m@4 ) ;\n"
                      "12: } ;\n", tokenize(code1));

        const char code2[] = "int i;\n"
                             "SomeType someVar1(i, i);\n"
                             "SomeType someVar2(j, j);\n"
                             "SomeType someVar3(j, 1);\n"
                             "SomeType someVar4(new bar);";
        ASSERT_EQUALS("1: int i@1 ;\n"
                      "2: SomeType someVar1@2 ( i@1 , i@1 ) ;\n"
                      "3: SomeType someVar2 ( j , j ) ;\n" // This one could be a function
                      "4: SomeType someVar3@3 ( j , 1 ) ;\n"
                      "5: SomeType someVar4@4 ( new bar ) ;\n", tokenize(code2));
    }

    void varid_in_class18() {
        const char code[] = "class A {\n"
                            "    class B;\n"
                            "};\n"
                            "class A::B {\n"
                            "    B();\n"
                            "    int* i;\n"
                            "};\n"
                            "A::B::B() :\n"
                            "    i(0)\n"
                            "{}";
        ASSERT_EQUALS("1: class A {\n"
                      "2: class B ;\n"
                      "3: } ;\n"
                      "4: class A :: B {\n"
                      "5: B ( ) ;\n"
                      "6: int * i@1 ;\n"
                      "7: } ;\n"
                      "8: A :: B :: B ( ) :\n"
                      "9: i@1 ( 0 )\n"
                      "10: { }\n", tokenize(code));
    }

    void varid_in_class19() {
        const char code[] = "class Fred {\n"
                            "    char *str1;\n"
                            "    ~Fred();\n"
                            "};\n"
                            "Fred::~Fred() {\n"
                            "    free(str1);\n"
                            "}";
        ASSERT_EQUALS("1: class Fred {\n"
                      "2: char * str1@1 ;\n"
                      "3: ~ Fred ( ) ;\n"
                      "4: } ;\n"
                      "5: Fred :: ~ Fred ( ) {\n"
                      "6: free ( str1@1 ) ;\n"
                      "7: }\n", tokenize(code));
    }

    void varid_in_class20() {
        const char code[] = "template<class C> class cacheEntry {\n"
                            "protected:\n"
                            "    int m_key;\n"
                            "public:\n"
                            "    cacheEntry();\n"
                            "};\n"
                            "\n"
                            "template<class C> cacheEntry<C>::cacheEntry() : m_key() {}";

        ASSERT_EQUALS("1: template < class C > class cacheEntry {\n"
                      "2: protected:\n"
                      "3: int m_key@1 ;\n"
                      "4: public:\n"
                      "5: cacheEntry ( ) ;\n"
                      "6: } ;\n"
                      "7:\n"
                      "8: template < class C > cacheEntry < C > :: cacheEntry ( ) : m_key@1 ( ) { }\n", tokenize(code));
    }

    void varid_in_class21() {
        const char code[] = "template <typename t1,typename t2>\n"
                            "class A::B {\n"
                            "    B();\n"
                            "    int x;\n"
                            "};\n"
                            "\n"
                            "template <typename t1,typename t2>\n"
                            "A::B<t1,t2>::B() : x(9) {}";

        const char expected[] = "1: template < typename t1 , typename t2 >\n"
                                "2: class A :: B {\n"
                                "3: B ( ) ;\n"
                                "4: int x@1 ;\n"
                                "5: } ;\n"
                                "6:\n"
                                "7: template < typename t1 , typename t2 >\n"
                                "8: A :: B < t1 , t2 > :: B ( ) : x@1 ( 9 ) { }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_in_class22() {
        const char code[] = "struct data {};\n"
                            "    struct S {\n"
                            "    std::vector<data> std;\n"
                            "    void f();\n"
                            "};\n"
                            "void S::f() {\n"
                            "    std::vector<data>::const_iterator end = std.end();\n"
                            "    for (std::vector<data>::const_iterator i = std.begin(); i != end; ++i) {}\n"
                            "}\n";

        const char expected[] = "1: struct data { } ;\n"
                                "2: struct S {\n"
                                "3: std :: vector < data > std@1 ;\n"
                                "4: void f ( ) ;\n"
                                "5: } ;\n"
                                "6: void S :: f ( ) {\n"
                                "7: std :: vector < data > :: const_iterator end@2 ; end@2 = std@1 . end ( ) ;\n"
                                "8: for ( std :: vector < data > :: const_iterator i@3 = std@1 . begin ( ) ; i@3 != end@2 ; ++ i@3 ) { }\n"
                                "9: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_in_class23() { // #11293
        const char code[] = "struct A {\n"
                            "    struct S {\n"
                            "        bool b;\n"
                            "    };\n"
                            "};\n"
                            "struct B : A::S {\n"
                            "    void f() { b = false; }\n"
                            "};\n";

        const char expected[] = "1: struct A {\n"
                                "2: struct S {\n"
                                "3: bool b@1 ;\n"
                                "4: } ;\n"
                                "5: } ;\n"
                                "6: struct B : A :: S {\n"
                                "7: void f ( ) { b@1 = false ; }\n"
                                "8: } ;\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_in_class24() {
        const char *expected{};

        {
            const char code[] = "class A {\n"
                                "    Q_OBJECT\n"
                                "public:\n"
                                "    using QPtr = QPointer<A>;\n"
                                "};\n";
            expected = "1: class A {\n"
                       "2: Q_OBJECT\n"
                       "3: public:\n"
                       "4:\n"
                       "5: } ;\n";
            ASSERT_EQUALS(expected, tokenize(code));
        }

        {
            const char code[] = "class A {\n"
                                "    Q_OBJECT\n"
                                "    using QPtr = QPointer<A>;\n"
                                "};\n";
            expected = "1: class A {\n"
                       "2: Q_OBJECT\n"
                       "3:\n"
                       "4: } ;\n";
            ASSERT_EQUALS(expected, tokenize(code));
        }
    }

    void varid_in_class25() {
        const Settings s = settingsBuilder(settings).library("std.cfg").build();

        const char *expected{};
        {
            const char code[] = "struct F {\n" // #11497
                                "    int i;\n"
                                "    void f(const std::vector<F>&v) {\n"
                                "        if (v.front().i) {}\n"
                                "    }\n"
                                "};\n";
            expected = "1: struct F {\n"
                       "2: int i@1 ;\n"
                       "3: void f ( const std :: vector < F > & v@2 ) {\n"
                       "4: if ( v@2 . front ( ) . i@3 ) { }\n"
                       "5: }\n"
                       "6: } ;\n";
            ASSERT_EQUALS(expected, tokenize(code, s));
        }

        {
            const char code[] = "struct T { };\n" // 11533
                                "struct U { T t; };\n"
                                "std::vector<U*>* g();\n"
                                "void f() {\n"
                                "    std::vector<U*>* p = g();\n"
                                "    auto t = p->front()->t;\n"
                                "}\n";
            expected = "1: struct T { } ;\n"
                       "2: struct U { T t@1 ; } ;\n"
                       "3: std :: vector < U * > * g ( ) ;\n"
                       "4: void f ( ) {\n"
                       "5: std :: vector < U * > * p@2 ; p@2 = g ( ) ;\n"
                       "6: auto t@3 ; t@3 = p@2 . front ( ) . t@4 ;\n"
                       "7: }\n";
            ASSERT_EQUALS(expected, tokenize(code, s));
        }
    }

    void varid_in_class26() {
        const char *expected{}; // #11334
        const char code[] = "struct S {\n"
                            "    union {\n"
                            "        uint8_t u8[4];\n"
                            "        uint32_t u32;\n"
                            "    };\n"
                            "    void f();\n"
                            "};\n"
                            "void S::f() {\n"
                            "    u8[0] = 0;\n"
                            "}\n";
        expected = "1: struct S {\n"
                   "2: union {\n"
                   "3: uint8_t u8@1 [ 4 ] ;\n"
                   "4: uint32_t u32@2 ;\n"
                   "5: } ;\n"
                   "6: void f ( ) ;\n"
                   "7: } ;\n"
                   "8: void S :: f ( ) {\n"
                   "9: u8@1 [ 0 ] = 0 ;\n"
                   "10: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_in_class27() {
        const char code[] = "struct S {\n" // #13291
                            "    int** pp;\n"
                            "    void f() { int x(*pp[0]); }\n"
                            "};\n";
        const char expected[] = "1: struct S {\n"
                                "2: int * * pp@1 ;\n"
                                "3: void f ( ) { int x@2 ( * pp@1 [ 0 ] ) ; }\n"
                                "4: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_in_class28() {
        const char code[] = "struct S {\n" // 13725
                            "    struct T {\n"
                            "        bool b();\n"
                            "    };\n"
                            "    bool b = false;\n"
                            "};\n";
        const char expected[] = "1: struct S {\n"
                                "2: struct T {\n"
                                "3: bool b ( ) ;\n"
                                "4: } ;\n"
                                "5: bool b@1 ; b@1 = false ;\n"
                                "6: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_in_class29() {
        const char code[] = "struct S {\n" // #14486
                            "    const int& r;\n"
                            "    explicit S(const int& r) : r(r) {}\n"
                            "    static void f() {\n"
                            "        struct T : std::vector<int> {\n"
                            "            bool g() const { return empty(); }\n"
                            "        };\n"
                            "    }\n"
                            "};\n";
        const char expected[] = "1: struct S {\n"
                                "2: const int & r@1 ;\n"
                                "3: explicit S ( const int & r@2 ) : r@1 ( r@2 ) { }\n"
                                "4: static void f ( ) {\n"
                                "5: struct T : std :: vector < int > {\n"
                                "6: bool g ( ) const { return empty ( ) ; }\n"
                                "7: } ;\n"
                                "8: }\n"
                                "9: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_in_class30() {
        const char code[] = "void* p;\n" // #13443
                            "struct S {\n"
                            "    void p();\n"
                            "};\n";
        const char expected[] = "1: void * p@1 ;\n"
                                "2: struct S {\n"
                                "3: void p ( ) ;\n"
                                "4: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_namespace_1() { // #7272
        const char code[] = "namespace Blah {\n"
                            "  struct foo { int x;};\n"
                            "  struct bar {\n"
                            "    int x;\n"
                            "    union { char y; };\n"
                            "  };\n"
                            "}";
        ASSERT_EQUALS("1: namespace Blah {\n"
                      "2: struct foo { int x@1 ; } ;\n"
                      "3: struct bar {\n"
                      "4: int x@2 ;\n"
                      "5: union { char y@3 ; } ;\n"
                      "6: } ;\n"
                      "7: }\n", tokenize(code));
    }

    void varid_namespace_2() { // #7000
        const char code[] = "namespace Ui {\n"
                            "    class C { int X; };\n"  // X@1
                            "}\n"
                            "\n"
                            "class C {\n"
                            "   void dostuff();\n"
                            "   int X;\n"  // X@2
                            "};\n"
                            "\n"
                            "void C::dostuff() {\n"
                            "   X = 0;\n"  // X@2
                            "}";

        const std::string actual = tokenize(code);

        ASSERT(actual.find("X@2 = 0") != std::string::npos);
    }

    static std::string getLine(const std::string &code, int lineNumber) {
        std::string nr = std::to_string(lineNumber);
        const std::string::size_type pos1 = code.find('\n' + nr + ": ");
        if (pos1 == std::string::npos)
            return "";
        const std::string::size_type pos2 = code.find('\n', pos1+1);
        if (pos2 == std::string::npos)
            return "";
        return code.substr(pos1+1, pos2-pos1-1);
    }

    void varid_namespace_3() { // #8627
        const char code[] = "namespace foo {\n"
                            "struct Bar {\n"
                            "  explicit Bar(int type);\n"
                            "  void f();\n"
                            "  int type;\n"  // <- Same varid here ...
                            "};\n"
                            "\n"
                            "Bar::Bar(int type) : type(type) {}\n"
                            "\n"
                            "void Bar::f() {\n"
                            "  type = 0;\n"  // <- ... and here
                            "}\n"
                            "}";

        const std::string actual = tokenize(code);

        ASSERT_EQUALS("5: int type@2 ;", getLine(actual,5));
        ASSERT_EQUALS("11: type@2 = 0 ;", getLine(actual,11));
    }

    void varid_namespace_4() {
        const char code[] = "namespace X {\n"
                            "  struct foo { int x;};\n"
                            "  struct bar: public foo {\n"
                            "    void dostuff();\n"
                            "  };\n"
                            "  void bar::dostuff() { int x2 = x * 2; }\n"
                            "}";
        ASSERT_EQUALS("1: namespace X {\n"
                      "2: struct foo { int x@1 ; } ;\n"
                      "3: struct bar : public foo {\n"
                      "4: void dostuff ( ) ;\n"
                      "5: } ;\n"
                      "6: void bar :: dostuff ( ) { int x2@2 ; x2@2 = x@1 * 2 ; }\n"
                      "7: }\n", tokenize(code));
    }

    void varid_namespace_5() {
        const char code[] = "namespace X {\n"
                            "  struct foo { int x;};\n"
                            "  namespace Y {\n"
                            "    struct bar: public foo {\n"
                            "      void dostuff();\n"
                            "    };\n"
                            "    void bar::dostuff() { int x2 = x * 2; }\n"
                            "  }\n"
                            "}";
        ASSERT_EQUALS("1: namespace X {\n"
                      "2: struct foo { int x@1 ; } ;\n"
                      "3: namespace Y {\n"
                      "4: struct bar : public foo {\n"
                      "5: void dostuff ( ) ;\n"
                      "6: } ;\n"
                      "7: void bar :: dostuff ( ) { int x2@2 ; x2@2 = x@1 * 2 ; }\n"
                      "8: }\n"
                      "9: }\n", tokenize(code));
    }

    void varid_namespace_6() {
        const char code[] = "namespace N {\n" // #12077
                            "    namespace O {\n"
                            "        U::U(int* map) : id(0) {\n"
                            "            this->p = map;\n"
                            "        }\n"
                            "        void U::f() {\n"
                            "            std::map<Vec2i, int>::iterator iter;\n"
                            "        }\n"
                            "    }\n"
                            "}";
        ASSERT_EQUALS("1: namespace N {\n"
                      "2: namespace O {\n"
                      "3: U :: U ( int * map@1 ) : id ( 0 ) {\n"
                      "4: this . p = map@1 ;\n"
                      "5: }\n"
                      "6: void U :: f ( ) {\n"
                      "7: std :: map < Vec2i , int > :: iterator iter@2 ;\n"
                      "8: }\n"
                      "9: }\n"
                      "10: }\n", tokenize(code));
    }

    void varid_initList() {
        const char code1[] = "class A {\n"
                             "  A() : x(0) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("1: class A {\n"
                      "2: A ( ) : x@1 ( 0 ) { }\n"
                      "3: int x@1 ;\n"
                      "4: } ;\n",
                      tokenize(code1));

        const char code2[] = "class A {\n"
                             "  A(int x) : x(x) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("1: class A {\n"
                      "2: A ( int x@1 ) : x@2 ( x@1 ) { }\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n",
                      tokenize(code2));

        const char code3[] = "class A {\n"
                             "  A(int x);\n"
                             "  int x;\n"
                             "};\n"
                             "A::A(int x) : x(x) {}";
        ASSERT_EQUALS("1: class A {\n"
                      "2: A ( int x@1 ) ;\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n"
                      "5: A :: A ( int x@3 ) : x@2 ( x@3 ) { }\n",
                      tokenize(code3));

        const char code4[] = "struct A {\n"
                             "  int x;\n"
                             "  A(int x) : x(x) {}\n"
                             "};\n";
        ASSERT_EQUALS("1: struct A {\n"
                      "2: int x@1 ;\n"
                      "3: A ( int x@2 ) : x@1 ( x@2 ) { }\n"
                      "4: } ;\n",
                      tokenize(code4));

        const char code5[] = "class A {\n"
                             "  A(int x) noexcept : x(x) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("1: class A {\n"
                      "2: A ( int x@1 ) noexcept ( true ) : x@2 ( x@1 ) { }\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n",
                      tokenize(code5));

        const char code6[] = "class A {\n"
                             "  A(int x) noexcept(true) : x(x) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("1: class A {\n"
                      "2: A ( int x@1 ) noexcept ( true ) : x@2 ( x@1 ) { }\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n",
                      tokenize(code6));

        const char code7[] = "class A {\n"
                             "  A(int x) noexcept(false) : x(x) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("1: class A {\n"
                      "2: A ( int x@1 ) noexcept ( false ) : x@2 ( x@1 ) { }\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n",
                      tokenize(code7));

        const char code8[] = "class Foo : public Bar {\n"
                             "  explicit Foo(int i) : Bar(mi = i) { }\n"
                             "  int mi;\n"
                             "};";
        ASSERT_EQUALS("1: class Foo : public Bar {\n"
                      "2: explicit Foo ( int i@1 ) : Bar ( mi@2 = i@1 ) { }\n"
                      "3: int mi@2 ;\n"
                      "4: } ;\n",
                      tokenize(code8));

        // #6520
        const char code9[] = "class A {\n"
                             "  A(int x) : y(a?0:1), x(x) {}\n"
                             "  int x, y;\n"
                             "};";
        ASSERT_EQUALS("1: class A {\n"
                      "2: A ( int x@1 ) : y@3 ( a ? 0 : 1 ) , x@2 ( x@1 ) { }\n"
                      "3: int x@2 ; int y@3 ;\n"
                      "4: } ;\n",
                      tokenize(code9));

        // #7123
        const char code10[] = "class A {\n"
                              "  double *work;\n"
                              "  A(const Matrix &m) throw (e);\n"
                              "};\n"
                              "A::A(const Matrix &m) throw (e) : work(0)\n"
                              "{}";
        ASSERT_EQUALS("1: class A {\n"
                      "2: double * work@1 ;\n"
                      "3: A ( const Matrix & m@2 ) throw ( e ) ;\n"
                      "4: } ;\n"
                      "5: A :: A ( const Matrix & m@3 ) throw ( e ) : work@1 ( 0 )\n"
                      "6: { }\n",
                      tokenize(code10));

        const char code11[] = "struct S {\n" // #12733
                              "    explicit S(int& r) : a{ int{ 1 } }, b{ r } {}\n"
                              "    int a, &b;\n"
                              "};";
        ASSERT_EQUALS("1: struct S {\n"
                      "2: explicit S ( int & r@1 ) : a@2 { int { 1 } } , b@3 { r@1 } { }\n"
                      "3: int a@2 ; int & b@3 ;\n"
                      "4: } ;\n",
                      tokenize(code11));

        const char code12[] = "template<typename... T>\n" // # 13070
                              "    struct S1 : T... {\n"
                              "    constexpr S1(const T& ... p) : T{ p } {}\n"
                              "};\n"
                              "namespace p { struct S2 {}; }\n"
                              "struct S3 {\n"
                              "    S3() {}\n"
                              "    bool f(p::S2& c);\n"
                              "};\n";
        ASSERT_EQUALS("1: template < typename ... T >\n"
                      "2: struct S1 : T ... {\n"
                      "3: constexpr S1 ( const T & ... p@1 ) : T { p@1 } { }\n"
                      "4: } ;\n"
                      "5: namespace p { struct S2 { } ; }\n"
                      "6: struct S3 {\n"
                      "7: S3 ( ) { }\n"
                      "8: bool f ( p :: S2 & c@2 ) ;\n"
                      "9: } ;\n",
                      tokenize(code12));

        const char code13[] = "template <typename... T>\n" // #13088
                              "struct B {};\n"
                              "template <typename... T>\n"
                              "struct D : B<T>... {\n"
                              "    template <typename... P>\n"
                              "    D(P&&... p) : B<T>(std::forward<P>(p))... {}\n"
                              "};\n"
                              "template <typename... T>\n"
                              "struct S {\n"
                              "    D<T...> d;\n"
                              "};\n";
        ASSERT_EQUALS("1: template < typename ... T >\n"
                      "2: struct B { } ;\n"
                      "3: template < typename ... T >\n"
                      "4: struct D : B < T > ... {\n"
                      "5: template < typename ... P >\n"
                      "6: D ( P && ... p@1 ) : B < T > ( std :: forward < P > ( p@1 ) ) ... { }\n"
                      "7: } ;\n"
                      "8: template < typename ... T >\n"
                      "9: struct S {\n"
                      "10: D < T ... > d@2 ;\n"
                      "11: } ;\n",
                      tokenize(code13));
    }

    void varid_initListWithBaseTemplate() {
        const char code1[] = "class A : B<C,D> {\n"
                             "  A() : B<C,D>(), x(0) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("1: class A : B < C , D > {\n"
                      "2: A ( ) : B < C , D > ( ) , x@1 ( 0 ) { }\n"
                      "3: int x@1 ;\n"
                      "4: } ;\n",
                      tokenize(code1));

        const char code2[] = "class A : B<C,D> {\n"
                             "  A(int x) : x(x) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("1: class A : B < C , D > {\n"
                      "2: A ( int x@1 ) : x@2 ( x@1 ) { }\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n",
                      tokenize(code2));

        const char code3[] = "class A : B<C,D> {\n"
                             "  A(int x);\n"
                             "  int x;\n"
                             "};\n"
                             "A::A(int x) : x(x) {}";
        ASSERT_EQUALS("1: class A : B < C , D > {\n"
                      "2: A ( int x@1 ) ;\n"
                      "3: int x@2 ;\n"
                      "4: } ;\n"
                      "5: A :: A ( int x@3 ) : x@2 ( x@3 ) { }\n",
                      tokenize(code3));

        const char code4[] = "struct A : B<C,D> {\n"
                             "  int x;\n"
                             "  A(int x) : x(x) {}\n"
                             "};\n";
        ASSERT_EQUALS("1: struct A : B < C , D > {\n"
                      "2: int x@1 ;\n"
                      "3: A ( int x@2 ) : x@1 ( x@2 ) { }\n"
                      "4: } ;\n",
                      tokenize(code4));

        const char code5[] = "class BCLass : public Ticket<void> {\n"
                             "  BCLass();\n"
                             "  PClass* member;\n"
                             "};\n"
                             "BCLass::BCLass() : Ticket<void>() {\n"
                             "  member = 0;\n"
                             "}";
        ASSERT_EQUALS("1: class BCLass : public Ticket < void > {\n"
                      "2: BCLass ( ) ;\n"
                      "3: PClass * member@1 ;\n"
                      "4: } ;\n"
                      "5: BCLass :: BCLass ( ) : Ticket < void > ( ) {\n"
                      "6: member@1 = 0 ;\n"
                      "7: }\n",
                      tokenize(code5));
    }

    void varid_initListWithScope() {
        const char code1[] = "class A : public B::C {\n"
                             "  A() : B::C(), x(0) {}\n"
                             "  int x;\n"
                             "};";
        ASSERT_EQUALS("1: class A : public B :: C {\n"
                      "2: A ( ) : B :: C ( ) , x@1 ( 0 ) { }\n"
                      "3: int x@1 ;\n"
                      "4: } ;\n",
                      tokenize(code1));
    }

    void varid_operator() {
        {
            const std::string actual = tokenize(
                "class Foo\n"
                "{\n"
                "public:\n"
                "    void operator=(const Foo &);\n"
                "};");

            const char expected[] = "1: class Foo\n"
                                    "2: {\n"
                                    "3: public:\n"
                                    "4: void operator= ( const Foo & ) ;\n"
                                    "5: } ;\n";

            ASSERT_EQUALS(expected, actual);
        }
        {
            const std::string actual = tokenize(
                "struct Foo {\n"
                "    void * operator new [](int);\n"
                "};");
            const char expected[] = "1: struct Foo {\n"
                                    "2: void * operatornew[] ( int ) ;\n"
                                    "3: } ;\n";

            ASSERT_EQUALS(expected, actual);
        }
    }

    void varid_throw() {  // ticket #1723
        const std::string actual = tokenize(
            "UserDefinedException* pe = new UserDefinedException();\n"
            "throw pe;");

        const char expected[] = "1: UserDefinedException * pe@1 ; pe@1 = new UserDefinedException ( ) ;\n"
                                "2: throw pe@1 ;\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varid_unknown_macro() {
        // #2638 - unknown macro
        const char code[] = "void f() {\n"
                            "    int a[10];\n"
                            "    AAA\n"
                            "    a[0] = 0;\n"
                            "}";
        const char expected[] = "1: void f ( ) {\n"
                                "2: int a@1 [ 10 ] ;\n"
                                "3: AAA\n"
                                "4: a@1 [ 0 ] = 0 ;\n"
                                "5: }\n";
        ASSERT_EQUALS(expected, tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid_using() {
        // #3648
        const char code[] = "using std::size_t;";
        const char expected[] = "1: using unsigned long ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_catch() {
        const char code[] = "void f() {\n"
                            "    try { dostuff(); }\n"
                            "    catch (exception &e) { }\n"
                            "}";
        const char expected[] = "1: void f ( ) {\n"
                                "2: try { dostuff ( ) ; }\n"
                                "3: catch ( exception & e@1 ) { }\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_functionPrototypeTemplate() {
        ASSERT_EQUALS("1: function < void ( ) > fptr@1 ;\n", tokenize("function<void(void)> fptr;"));
    }

    void varid_templatePtr() {
        ASSERT_EQUALS("1: std :: map < int , FooTemplate < int > * > dummy_member@1 [ 1 ] ;\n", tokenize("std::map<int, FooTemplate<int>*> dummy_member[1];"));
    }

    void varid_templateNamespaceFuncPtr() {
        ASSERT_EQUALS("1: KeyListT < float , & NIFFile :: getFloat > mKeyList@1 [ 4 ] ;\n", tokenize("KeyListT<float, &NIFFile::getFloat> mKeyList[4];"));
    }

    void varid_templateArray() {
        ASSERT_EQUALS("1: VertexArrayIterator < float [ 2 ] > attrPos@1 ; attrPos@1 = m_AttributePos . GetIterator < float [ 2 ] > ( ) ;\n",
                      tokenize("VertexArrayIterator<float[2]> attrPos = m_AttributePos.GetIterator<float[2]>();"));
    }

    void varid_templateParameter() {
        {
            const char code[] = "const int X = 0;\n" // #7046 set varid for "X":  std::array<int,X> Y;
                                "std::array<int,X> Y;\n";

            ASSERT_EQUALS("1: const int X@1 = 0 ;\n"
                          "2: std :: array < int , X@1 > Y@2 ;\n",
                          tokenize(code));
        }
        {
            const char code[] = "std::optional<N::Foo<A>> Foo;\n"; // #11003

            ASSERT_EQUALS("1: std :: optional < N :: Foo < A > > Foo@1 ;\n",
                          tokenize(code));
        }
    }

    void varid_templateParameterFunctionPointer() {
        {
            const char code[] = "template <class, void (*F)()>\n"
                                "struct a;\n";

            ASSERT_EQUALS("1: template < class , void ( * F ) ( ) >\n"
                          "2: struct a ;\n",
                          tokenize(code));
        }
    }

    void varid_templateUsing() { // #5781 #7273
        const char code[] = "template<class T> using X = Y<T,4>;\n"
                            "X<int> x;";
        ASSERT_EQUALS("2: Y < int , 4 > x@1 ;\n",
                      tokenize(code));
    }

    void varid_templateSpecializationFinal() {
        const char code[] = "template <typename T>\n"
                            "struct S;\n"
                            "template <>\n"
                            "struct S<void> final {};\n";
        ASSERT_EQUALS("4: struct S<void> ;\n"
                      "1: template < typename T >\n"
                      "2: struct S ;\n"
                      "3:\n"
                      "4: struct S<void> { } ;\n",
                      tokenize(code));
    }

    void varid_not_template_in_condition() {
        const char code1[] = "void f() { if (x<a||x>b); }";
        ASSERT_EQUALS("1: void f ( ) { if ( x < a || x > b ) { ; } }\n", tokenize(code1));

        const char code2[] = "void f() { if (1+x<a||x>b); }";
        ASSERT_EQUALS("1: void f ( ) { if ( 1 + x < a || x > b ) { ; } }\n", tokenize(code2));

        const char code3[] = "void f() { if (x<a||x>b+1); }";
        ASSERT_EQUALS("1: void f ( ) { if ( x < a || x > b + 1 ) { ; } }\n", tokenize(code3));

        const char code4[] = "void f() { if ((x==13) && (x<a||x>b)); }";
        ASSERT_EQUALS("1: void f ( ) { if ( ( x == 13 ) && ( x < a || x > b ) ) { ; } }\n", tokenize(code4));
    }

    void varid_cppcast() {
        ASSERT_EQUALS("1: const_cast < int * > ( code ) [ 0 ] = 0 ;\n",
                      tokenize("const_cast<int *>(code)[0] = 0;"));

        ASSERT_EQUALS("1: dynamic_cast < int * > ( code ) [ 0 ] = 0 ;\n",
                      tokenize("dynamic_cast<int *>(code)[0] = 0;"));

        ASSERT_EQUALS("1: reinterpret_cast < int * > ( code ) [ 0 ] = 0 ;\n",
                      tokenize("reinterpret_cast<int *>(code)[0] = 0;"));

        ASSERT_EQUALS("1: static_cast < int * > ( code ) [ 0 ] = 0 ;\n",
                      tokenize("static_cast<int *>(code)[0] = 0;"));
    }

    void varid_variadicFunc() {
        ASSERT_EQUALS("1: int foo ( ... ) ;\n", tokenize("int foo(...);"));
    }

    void varid_typename() {
        ASSERT_EQUALS("1: template < int d , class A , class B > struct S { } ;\n", tokenize("template<int d, class A, class B> struct S {};"));

        ASSERT_EQUALS("1: template < int d , typename A , typename B > struct S { } ;\n", tokenize("template<int d, typename A, typename B> struct S {};"));

        ASSERT_EQUALS("1: A a@1 ;\n", tokenize("typename A a;"));
    }

    void varid_rvalueref() {
        ASSERT_EQUALS("1: int && a@1 ;\n", tokenize("int&& a;"));

        ASSERT_EQUALS("1: void foo ( int && a@1 ) { }\n", tokenize("void foo(int&& a) {}"));

        ASSERT_EQUALS("1: class C {\n"
                      "2: C ( int && a@1 ) ;\n"
                      "3: } ;\n",
                      tokenize("class C {\n"
                               "    C(int&& a);\n"
                               "};"));

        ASSERT_EQUALS("1: void foo ( int && ) ;\n", tokenize("void foo(int&&);"));
    }

    void varid_arrayFuncPar() {
        ASSERT_EQUALS("1: void check ( const char fname@1 [ ] = 0 ) { }\n", tokenize("void check( const char fname[] = 0) { }"));
    }

    void varid_sizeofPassed() {
        ASSERT_EQUALS("1: void which_test ( ) {\n"
                      "2: const char * argv@1 [ 2 ] = { \"./test_runner\" , \"TestClass\" } ;\n"
                      "3: options args@2 ( sizeof ( argv@1 ) / sizeof ( argv@1 [ 0 ] ) , argv@1 ) ;\n"
                      "4: args@2 . which_test ( ) ;\n"
                      "5: }\n",
                      tokenize("void which_test() {\n"
                               "    const char* argv[] = { \"./test_runner\", \"TestClass\" };\n"
                               "    options args(sizeof argv / sizeof argv[0], argv);\n"
                               "    args.which_test();\n"
                               "}"));
    }

    void varid_classInFunction() {
        ASSERT_EQUALS("1: void AddSuppression ( ) {\n"
                      "2: class QErrorLogger {\n"
                      "3: void reportErr ( ErrorLogger :: ErrorMessage & msg@1 ) {\n"
                      "4: }\n"
                      "5: } ;\n"
                      "6: }\n",
                      tokenize("void AddSuppression() {\n"
                               "    class QErrorLogger {\n"
                               "        void reportErr(ErrorLogger::ErrorMessage &msg) {\n"
                               "        }\n"
                               "    };\n"
                               "}"));
    }

    void varid_pointerToArray() {
        ASSERT_EQUALS("1: int ( * a1@1 ) [ 10 ] ;\n"
                      "2: void f1 ( ) {\n"
                      "3: int ( * a2@2 ) [ 10 ] ;\n"
                      "4: int ( & a3@3 ) [ 10 ] ;\n"
                      "5: }\n"
                      "6: struct A {\n"
                      "7: int ( & a4@4 ) [ 10 ] ;\n"
                      "8: int f2 ( int i@5 ) { return a4@4 [ i@5 ] ; }\n"
                      "9: int f3 ( int ( & a5@6 ) [ 10 ] , int i@7 ) { return a5@6 [ i@7 ] ; }\n"
                      "10: } ;\n"
                      "11: int f4 ( int ( & a6@8 ) [ 10 ] , int i@9 ) { return a6@8 [ i@9 ] ; }\n",
                      tokenize("int (*a1)[10];\n" // pointer to array of 10 ints
                               "void f1() {\n"
                               "    int(*a2)[10];\n"
                               "    int(&a3)[10];\n"
                               "}\n"
                               "struct A {\n"
                               "    int(&a4)[10];\n"
                               "    int f2(int i) { return a4[i]; }\n"
                               "    int f3(int(&a5)[10], int i) { return a5[i]; }\n"
                               "};\n"
                               "int f4(int(&a6)[10], int i) { return a6[i]; }"));
    }

    void varid_cpp11initialization() {
        ASSERT_EQUALS("1: int i@1 { 1 } ;\n"
                      "2: std :: vector < int > vec@2 { 1 , 2 , 3 } ;\n"
                      "3: namespace n { int z@3 ; } ;\n"
                      "4: int & j@4 { i@1 } ;\n"
                      "5: int k@5 { 1 } ; int l@6 { 2 } ;\n",
                      tokenize("int i{1};\n"
                               "std::vector<int> vec{1, 2, 3};\n"
                               "namespace n { int z; };\n"
                               "int& j{i};\n"
                               "int k{1}, l{2};"));

        // #6030
        ASSERT_EQUALS("1: struct S3 : public S1 , public S2 { } ;\n",
                      tokenize("struct S3 : public S1, public S2 { };"));

        // #6058
        ASSERT_EQUALS("1: class Scope { } ;\n",
                      tokenize("class CPPCHECKLIB Scope { };"));

        // #6073 #6253
        ASSERT_EQUALS("1: class A : public B , public C :: D , public E < F > :: G < H > {\n"
                      "2: int i@1 ;\n"
                      "3: A ( int i@2 ) : B { i@2 } , C :: D { i@2 } , E < F > :: G < H > { i@2 } , i@1 { i@2 } {\n"
                      "4: int j@3 { i@2 } ;\n"
                      "5: }\n"
                      "6: } ;\n",
                      tokenize("class A: public B, public C::D, public E<F>::G<H> {\n"
                               "    int i;\n"
                               "    A(int i): B{i}, C::D{i}, E<F>::G<H>{i} ,i{i} {\n"
                               "        int j{i};\n"
                               "    }\n"
                               "};"));
    }

    void varid_inheritedMembers() {
        ASSERT_EQUALS("1: class A {\n"
                      "2: int a@1 ;\n"
                      "3: } ;\n"
                      "4: class B : public A {\n"
                      "5: void f ( ) ;\n"
                      "6: } ;\n"
                      "7: void B :: f ( ) {\n"
                      "8: a@1 = 0 ;\n"
                      "9: }\n",
                      tokenize("class A {\n"
                               "    int a;\n"
                               "};\n"
                               "class B : public A {\n"
                               "    void f();\n"
                               "};\n"
                               "void B::f() {\n"
                               "    a = 0;\n"
                               "}"));

        ASSERT_EQUALS("1: class A {\n"
                      "2: int a@1 ;\n"
                      "3: } ;\n"
                      "4: class B : A {\n"
                      "5: void f ( ) ;\n"
                      "6: } ;\n"
                      "7: void B :: f ( ) {\n"
                      "8: a@1 = 0 ;\n"
                      "9: }\n",
                      tokenize("class A {\n"
                               "    int a;\n"
                               "};\n"
                               "class B : A {\n"
                               "    void f();\n"
                               "};\n"
                               "void B::f() {\n"
                               "    a = 0;\n"
                               "}"));

        ASSERT_EQUALS("1: class A {\n"
                      "2: int a@1 ;\n"
                      "3: } ;\n"
                      "4: class B : protected B , public A {\n"
                      "5: void f ( ) ;\n"
                      "6: } ;\n"
                      "7: void B :: f ( ) {\n"
                      "8: a@1 = 0 ;\n"
                      "9: }\n",
                      tokenize("class A {\n"
                               "    int a;\n"
                               "};\n"
                               "class B : protected B, public A {\n"
                               "    void f();\n"
                               "};\n"
                               "void B::f() {\n"
                               "    a = 0;\n"
                               "}"));

        ASSERT_EQUALS("1: class A {\n"
                      "2: int a@1 ;\n"
                      "3: } ;\n"
                      "4: class B : public A {\n"
                      "5: void f ( ) {\n"
                      "6: a@1 = 0 ;\n"
                      "7: }\n"
                      "8: } ;\n",
                      tokenize("class A {\n"
                               "    int a;\n"
                               "};\n"
                               "class B : public A {\n"
                               "    void f() {\n"
                               "        a = 0;\n"
                               "    }\n"
                               "};"));
    }

    void varid_header() {
        ASSERT_EQUALS("1: class A@1 ;\n"
                      "2: struct B {\n"
                      "3: void setData ( const A@1 & a ) ;\n"
                      "4: } ;\n",
                      tokenizeHeader("class A;\n"
                                     "struct B {\n"
                                     "    void setData(const A & a);\n"
                                     "}; ", "test.h"));
        ASSERT_EQUALS("1: class A ;\n"
                      "2: struct B {\n"
                      "3: void setData ( const A & a@1 ) ;\n"
                      "4: } ;\n",
                      tokenizeHeader("class A;\n"
                                     "struct B {\n"
                                     "    void setData(const A & a);\n"
                                     "}; ", "test.hpp"));
        ASSERT_EQUALS("1: void f ( )\n"
                      "2: {\n"
                      "3: int class@1 ;\n"
                      "4: }\n",
                      tokenizeHeader("void f()\n"
                                     "{\n"
                                     "    int class;\n"
                                     "}", "test.h"));
    }

    void varid_rangeBasedFor() {
        ASSERT_EQUALS("1: void reset ( Foo array@1 ) {\n"
                      "2: for ( auto & e@2 : array@1 ) {\n"
                      "3: foo ( e@2 ) ; }\n"
                      "4: } ;\n",
                      tokenize("void reset(Foo array) {\n"
                               "    for (auto& e : array)\n"
                               "        foo(e);\n"
                               "};"));

        ASSERT_EQUALS("1: void reset ( Foo array@1 ) {\n"
                      "2: for ( auto e@2 : array@1 ) {\n"
                      "3: foo ( e@2 ) ; }\n"
                      "4: } ;\n",
                      tokenize("void reset(Foo array) {\n"
                               "    for (auto e : array)\n"
                               "        foo(e);\n"
                               "};"));

        // Labels are no variables
        ASSERT_EQUALS("1: void foo ( ) {\n"
                      "2: switch ( event . key . keysym . sym ) {\n"
                      "3: case SDLK_LEFT : ;\n"
                      "4: break ;\n"
                      "5: case SDLK_RIGHT : ;\n"
                      "6: delta = 1 ;\n"
                      "7: break ;\n"
                      "8: }\n"
                      "9: }\n",
                      tokenize("void foo() {\n"
                               "    switch (event.key.keysym.sym) {\n"
                               "    case SDLK_LEFT:\n"
                               "        break;\n"
                               "    case SDLK_RIGHT:\n"
                               "        delta = 1;\n"
                               "        break;\n"
                               "    }\n"
                               "}", dinit(TokenizeOptions, $.cpp = false)));

        ASSERT_EQUALS("1: int * f ( ) {\n" // #11838
                      "2: int * label@1 ; label@1 = 0 ;\n"
                      "3: label : ;\n"
                      "4: return label@1 ;\n"
                      "5: }\n",
                      tokenize("int* f() {\n"
                               "    int* label = 0;\n"
                               "label:\n"
                               "    return label;\n"
                               "}"));
    }

    void varid_structinit() { // #6406
        ASSERT_EQUALS("1: void foo ( ) {\n"
                      "2: struct ABC abc@1 ; abc@1 = { . a@2 = 0 , . b@3 = 1 } ;\n"
                      "3: }\n",
                      tokenize("void foo() {\n"
                               "  struct ABC abc = {.a=0,.b=1};\n"
                               "}"));

        ASSERT_EQUALS("1: void foo ( ) {\n"
                      "2: struct ABC abc@1 ; abc@1 = { . a@2 = abc@1 . a@2 , . b@3 = abc@1 . b@3 } ;\n"
                      "3: }\n",
                      tokenize("void foo() {\n"
                               "  struct ABC abc = {.a=abc.a,.b=abc.b};\n"
                               "}"));

        ASSERT_EQUALS("1: void foo ( ) {\n"
                      "2: struct ABC abc@1 ; abc@1 = { . a@2 { abc@1 . a@2 } , . b@3 = { abc@1 . b@3 } } ;\n"
                      "3: }\n",
                      tokenize("void foo() {\n"
                               "  struct ABC abc = {.a { abc.a },.b= { abc.b } };\n"
                               "}"));

        ASSERT_EQUALS("1: struct T { int a@1 ; } ;\n" // #13123
                      "2: void f ( int a@2 ) {\n"
                      "3: struct T t@3 ; t@3 = { . a@4 = 1 } ;\n"
                      "4: }\n",
                      tokenize("struct T { int a; };\n"
                               "void f(int a) {\n"
                               "    struct T t = { .a = 1 };\n"
                               "}\n"));

        ASSERT_EQUALS("1: struct S { int x@1 ; } ;\n" // TODO: set some varid for designated initializer?
                      "2: void f0 ( S s@2 ) ;\n"
                      "3: void f1 ( int n@3 ) {\n"
                      "4: if ( int x@4 = n@3 ) {\n"
                      "5: f0 ( S { . x = x@4 } ) ;\n"
                      "6: }\n"
                      "7: }\n",
                      tokenize("struct S { int x; };\n"
                               "void f0(S s);\n"
                               "void f1(int n) {\n"
                               "    if (int x = n) {\n"
                               "        f0(S{ .x = x });\n"
                               "    }\n"
                               "}\n"));
    }

    void varid_arrayinit() {
        // #7579 - no variable declaration in rhs
        ASSERT_EQUALS("1: void foo ( int * a@1 ) { int b@2 [ 1 ] = { x * a@1 [ 0 ] } ; }\n", tokenize("void foo(int*a) { int b[] = { x*a[0] }; }"));

        // #12402
        ASSERT_EQUALS("1: void f ( ) { void ( * p@1 [ 1 ] ) ( int ) = { [ ] ( int i@2 ) { } } ; }\n", tokenize("void f() { void (*p[1])(int) = { [](int i) {} }; }"));
    }

    void varid_lambda_arg() {
        // #8664
        {
            const char code[] = "static void func(int ec) {\n"
                                "    func2([](const std::error_code& ec) { return ec; });\n"
                                "}";
            const char exp[] = "1: static void func ( int ec@1 ) {\n"
                               "2: func2 ( [ ] ( const std :: error_code & ec@2 ) { return ec@2 ; } ) ;\n"
                               "3: }\n";
            ASSERT_EQUALS(exp, tokenize(code));
        }
        {
            const char code[] = "static void func(int ec) {\n"
                                "    func2([](int x, const std::error_code& ec) { return x + ec; });\n"
                                "}";
            const char exp[] = "1: static void func ( int ec@1 ) {\n"
                               "2: func2 ( [ ] ( int x@2 , const std :: error_code & ec@3 ) { return x@2 + ec@3 ; } ) ;\n"
                               "3: }\n";
            ASSERT_EQUALS(exp, tokenize(code));
        }
        // #9384
        {
            const char code[] = "auto g = [](const std::string& s) -> std::string { return {}; };\n";
            const char exp[] = "1: auto g@1 ; g@1 = [ ] ( const std :: string & s@2 ) . std :: string { return { } ; } ;\n";
            ASSERT_EQUALS(exp, tokenize(code));
        }
        {
            const char code[] = "auto g = [](std::function<void()> p) {};\n";
            const char exp[] = "1: auto g@1 ; g@1 = [ ] ( std :: function < void ( ) > p@2 ) { } ;\n";
            ASSERT_EQUALS(exp, tokenize(code));
        }
        // # 10849
        {
            const char code[] = "class T {};\n"
                                "auto g = [](const T* t) -> int {\n"
                                "    const T* u{}, *v{};\n"
                                "    return 0;\n"
                                "};\n";
            const char exp[] = "1: class T { } ;\n"
                               "2: auto g@1 ; g@1 = [ ] ( const T * t@2 ) . int {\n"
                               "3: const T * u@3 { } ; const T * v@4 { } ;\n"
                               "4: return 0 ;\n"
                               "5: } ;\n";
            ASSERT_EQUALS(exp, tokenize(code));
        }
        // # 11332
        {
            const char code[] = "auto a() {\n"
                                "    return [](int, int b) {};\n"
                                "}\n";
            const char exp[] = "1: auto a ( ) {\n"
                               "2: return [ ] ( int , int b@1 ) { } ;\n"
                               "3: }\n";
            ASSERT_EQUALS(exp, tokenize(code));
        }
    }

    void varid_lambda_mutable() {
        // #8957
        {
            const char code[] = "static void func() {\n"
                                "    auto x = []() mutable {};\n"
                                "    dostuff(x);\n"
                                "}";
            const char exp[] = "1: static void func ( ) {\n"
                               "2: auto x@1 ; x@1 = [ ] ( ) mutable { } ;\n"
                               "3: dostuff ( x@1 ) ;\n"
                               "4: }\n";
            ASSERT_EQUALS(exp, tokenize(code));
        }
        // #9384
        {
            const char code[] = "auto g = [](int i) mutable {};\n";
            const char exp[] = "1: auto g@1 ; g@1 = [ ] ( int i@2 ) mutable { } ;\n";
            ASSERT_EQUALS(exp, tokenize(code));
        }
    }

    void varid_trailing_return1() { // #8889
        const char code[] = "struct Fred {\n"
                            "    auto foo(const Fred & other) -> Fred &;\n"
                            "    auto bar(const Fred & other) -> Fred & {\n"
                            "        return *this;\n"
                            "    }\n"
                            "};\n"
                            "auto Fred::foo(const Fred & other) -> Fred & {\n"
                            "    return *this;\n"
                            "}";
        const char exp[] = "1: struct Fred {\n"
                           "2: auto foo ( const Fred & other@1 ) . Fred & ;\n"
                           "3: auto bar ( const Fred & other@2 ) . Fred & {\n"
                           "4: return * this ;\n"
                           "5: }\n"
                           "6: } ;\n"
                           "7: auto Fred :: foo ( const Fred & other@3 ) . Fred & {\n"
                           "8: return * this ;\n"
                           "9: }\n";
        ASSERT_EQUALS(exp, tokenize(code));
    }

    void varid_trailing_return2() { // #9066
        const char code[] = "auto func(int arg) -> bar::quux {}";
        const char exp[] = "1: auto func ( int arg@1 ) . bar :: quux { }\n";
        ASSERT_EQUALS(exp, tokenize(code));
    }

    void varid_trailing_return3() { // #11423
        const char code[] = "void f(int a, int b) {\n"
                            "    auto g = [](int& a, const int b) -> void {};\n"
                            "    auto h = [&a, &b]() { std::swap(a, b); };\n"
                            "}\n";
        const char exp[] = "1: void f ( int a@1 , int b@2 ) {\n"
                           "2: auto g@3 ; g@3 = [ ] ( int & a@4 , const int b@5 ) . void { } ;\n"
                           "3: auto h@6 ; h@6 = [ & a@1 , & b@2 ] ( ) { std :: swap ( a@1 , b@2 ) ; } ;\n"
                           "4: }\n";
        ASSERT_EQUALS(exp, tokenize(code));
    }

    void varid_parameter_pack() { // #9383
        const char code[] = "template <typename... Rest>\n"
                            "void func(Rest... parameters) {\n"
                            "    foo(parameters...);\n"
                            "}\n";
        const char exp[] = "1: template < typename ... Rest >\n"
                           "2: void func ( Rest ... parameters@1 ) {\n"
                           "3: foo ( parameters@1 ... ) ;\n"
                           "4: }\n";
        ASSERT_EQUALS(exp, tokenize(code));
    }

    void varid_for_auto_cpp17() {
        const char code[] = "void f() {\n"
                            "  for (auto [x,y,z]: xyz) {\n"
                            "    x+y+z;\n"
                            "  }\n"
                            "  x+y+z;\n"
                            "}";
        const char exp[] = "1: void f ( ) {\n"
                           "2: for ( auto [ x@1 , y@2 , z@3 ] : xyz ) {\n"
                           "3: x@1 + y@2 + z@3 ;\n"
                           "4: }\n"
                           "5: x + y + z ;\n"
                           "6: }\n";
        ASSERT_EQUALS(exp, tokenize(code));
    }

    void varid_not() { // #9689 'not x'
        const char code[] = "void foo(int x) const {\n"
                            "  if (not x) {}\n"
                            "}";
        const char exp[] = "1: void foo ( int x@1 ) const {\n"
                           "2: if ( ! x@1 ) { }\n"
                           "3: }\n";
        ASSERT_EQUALS(exp, tokenize(code));
    }

    void varid_declInIfCondition() {
        // if
        ASSERT_EQUALS("1: void f ( int x@1 ) {\n"
                      "2: if ( int x@2 = 0 ) { x@2 ; }\n"
                      "3: x@1 ;\n"
                      "4: }\n",
                      tokenize("void f(int x) {\n"
                               "  if (int x = 0) { x; }\n"
                               "  x;\n"
                               "}"));
        // if, else
        ASSERT_EQUALS("1: void f ( int x@1 ) {\n"
                      "2: if ( int x@2 = 0 ) { x@2 ; }\n"
                      "3: else { x@2 ; }\n"
                      "4: x@1 ;\n"
                      "5: }\n",
                      tokenize("void f(int x) {\n"
                               "  if (int x = 0) { x; }\n"
                               "  else { x; }\n"
                               "  x;\n"
                               "}"));
        // if, else if
        ASSERT_EQUALS("1: void f ( int x@1 ) {\n"
                      "2: if ( int x@2 = 0 ) { x@2 ; }\n"
                      "3: else { if ( void * x@3 = & x@3 ) { x@3 ; } }\n"
                      "4: x@1 ;\n"
                      "5: }\n",
                      tokenize("void f(int x) {\n"
                               "  if (int x = 0) x;\n"
                               "  else if (void* x = &x) x;\n"
                               "  x;\n"
                               "}"));
        // if, else if, else
        ASSERT_EQUALS("1: void f ( int x@1 ) {\n"
                      "2: if ( int x@2 = 0 ) { x@2 ; }\n"
                      "3: else { if ( void * x@3 = & x@3 ) { x@3 ; }\n"
                      "4: else { x@3 ; } }\n"
                      "5: x@1 ;\n"
                      "6: }\n",
                      tokenize("void f(int x) {\n"
                               "  if (int x = 0) x;\n"
                               "  else if (void* x = &x) x;\n"
                               "  else x;\n"
                               "  x;\n"
                               "}"));

        ASSERT_EQUALS("1: const char * f ( int * ) ;\n" // #12924
                      "2: void g ( int i@1 ) {\n"
                      "3: if ( f ( & i@1 ) [ 0 ] == 'm' ) { }\n"
                      "4: }\n",
                      tokenize("const char *f(int*);\n"
                               "void g(int i) {\n"
                               "    if (f(&i)[0] == 'm') {}\n"
                               "}\n", dinit(TokenizeOptions, $.cpp = false)));
    }

    void varid_globalScope() {
        const char code[] = "int a[5];\n"
                            "namespace Z { struct B { int a[5]; } b; }\n"
                            "void f() {\n"
                            "  int a[5];\n"
                            "  memset(a, 123, 5);\n"
                            "  memset(::a, 123, 5);\n"
                            "  memset(Z::b.a, 123, 5);\n"
                            "  memset(::Z::b.a, 123, 5);\n"
                            "}";

        const char exp[] = "1: int a@1 [ 5 ] ;\n"
                           "2: namespace Z { struct B { int a@2 [ 5 ] ; } ; struct B b@3 ; }\n"
                           "3: void f ( ) {\n"
                           "4: int a@4 [ 5 ] ;\n"
                           "5: memset ( a@4 , 123 , 5 ) ;\n"
                           "6: memset ( :: a@1 , 123 , 5 ) ;\n"
                           "7: memset ( Z :: b@3 . a , 123 , 5 ) ;\n"
                           "8: memset ( :: Z :: b@3 . a , 123 , 5 ) ;\n"
                           "9: }\n";
        ASSERT_EQUALS(exp, tokenize(code));
    }

    void varid_function_pointer_args() {
        const char code1[] = "void foo() {\n"
                             "    char *text;\n"
                             "    void (*cb)(char* text);\n"
                             "}\n";
        ASSERT_EQUALS("1: void foo ( ) {\n"
                      "2: char * text@1 ;\n"
                      "3: void ( * cb@2 ) ( char * ) ;\n"
                      "4: }\n", tokenize(code1));

        const char code2[] = "void foo() {\n"
                             "    char *text;\n"
                             "    void (*f)(int (*arg)(char* text));\n"
                             "}\n";
        ASSERT_EQUALS("1: void foo ( ) {\n"
                      "2: char * text@1 ;\n"
                      "3: void ( * f@2 ) ( int ( * arg ) ( char * ) ) ;\n"
                      "4: }\n", tokenize(code2));

        const char code3[] = "void f (void (*g) (int i, IN int n)) {}\n";
        ASSERT_EQUALS("1: void f ( void ( * g@1 ) ( int , IN int ) ) { }\n", tokenize(code3));

        const char code4[] = "void f() {\n" // #14439
                             "    int* p;\n"
                             "    void (*a[1])(int* p) = { 0 };\n"
                             "}\n";
        ASSERT_EQUALS("1: void f ( ) {\n"
                      "2: int * p@1 ;\n"
                      "3: void ( * a@2 [ 1 ] ) ( int * p ) = { 0 } ;\n"
                      "4: }\n",
                      tokenize(code4));
    }

    void varid_alignas() {
        const char code[] = "extern alignas(16) int x;\n"
                            "alignas(16) int x;";
        const char expected[] = "1: extern alignas ( 16 ) int x@1 ;\n"
                                "2: alignas ( 16 ) int x@2 ;\n";
        ASSERT_EQUALS(expected, tokenize(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void varidclass1() {
        const std::string actual = tokenize(
            "class Fred\n"
            "{\n"
            "private:\n"
            "    int i;\n"
            "\n"
            "    void foo1();\n"
            "    void foo2()\n"
            "    {\n"
            "        ++i;\n"
            "    }\n"
            "}\n"
            "\n"
            "Fred::foo1()\n"
            "{\n"
            "    i = 0;\n"
            "}");

        const char expected[] = "1: class Fred\n"
                                "2: {\n"
                                "3: private:\n"
                                "4: int i@1 ;\n"
                                "5:\n"
                                "6: void foo1 ( ) ;\n"
                                "7: void foo2 ( )\n"
                                "8: {\n"
                                "9: ++ i@1 ;\n"
                                "10: }\n"
                                "11: }\n"
                                "12:\n"
                                "13: Fred :: foo1 ( )\n"
                                "14: {\n"
                                "15: i@1 = 0 ;\n"
                                "16: }\n";

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass2() {
        const std::string actual = tokenize(
            "class Fred\n"
            "{ void f(); };\n"
            "\n"
            "void A::foo1()\n"
            "{\n"
            "    int i = 0;\n"
            "}\n"
            "\n"
            "void Fred::f()\n"
            "{\n"
            "    i = 0;\n"
            "}");

        const char expected[] = "1: class Fred\n"
                                "2: { void f ( ) ; } ;\n"
                                "3:\n"
                                "4: void A :: foo1 ( )\n"
                                "5: {\n"
                                "6: int i@1 ; i@1 = 0 ;\n"
                                "7: }\n"
                                "8:\n"
                                "9: void Fred :: f ( )\n"
                                "10: {\n"
                                "11: i = 0 ;\n"
                                "12: }\n";

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass3() {
        const std::string actual = tokenize(
            "class Fred\n"
            "{ int i; void f(); };\n"
            "\n"
            "void Fred::f()\n"
            "{\n"
            "    i = 0;\n"
            "}\n"
            "\n"
            "void A::f()\n"
            "{\n"
            "    i = 0;\n"
            "}");

        const char expected[] = "1: class Fred\n"
                                "2: { int i@1 ; void f ( ) ; } ;\n"
                                "3:\n"
                                "4: void Fred :: f ( )\n"
                                "5: {\n"
                                "6: i@1 = 0 ;\n"
                                "7: }\n"
                                "8:\n"
                                "9: void A :: f ( )\n"
                                "10: {\n"
                                "11: i = 0 ;\n"
                                "12: }\n";

        ASSERT_EQUALS(expected, actual);
    }


    void varidclass4() {
        const std::string actual = tokenize(
            "class Fred\n"
            "{ int i; void f(); };\n"
            "\n"
            "void Fred::f()\n"
            "{\n"
            "    if (i) { }\n"
            "    i = 0;\n"
            "}");

        const char expected[] = "1: class Fred\n"
                                "2: { int i@1 ; void f ( ) ; } ;\n"
                                "3:\n"
                                "4: void Fred :: f ( )\n"
                                "5: {\n"
                                "6: if ( i@1 ) { }\n"
                                "7: i@1 = 0 ;\n"
                                "8: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass5() {
        const std::string actual = tokenize(
            "class A { };\n"
            "class B\n"
            "{\n"
            "    A *a;\n"
            "    B() : a(new A)\n"
            "    { }\n"
            "};");

        const char expected[] = "1: class A { } ;\n"
                                "2: class B\n"
                                "3: {\n"
                                "4: A * a@1 ;\n"
                                "5: B ( ) : a@1 ( new A )\n"
                                "6: { }\n"
                                "7: } ;\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass6() {
        const std::string actual = tokenize(
            "class A\n"
            "{\n"
            "  public:\n"
            "  static char buf[20];\n"
            "};\n"
            "char A::buf[20];\n"
            "int main()\n"
            "{\n"
            "  char buf[2];\n"
            "  A::buf[10] = 0;\n"
            "}");

        const char expected[] = "1: class A\n"
                                "2: {\n"
                                "3: public:\n"
                                "4: static char buf@1 [ 20 ] ;\n"
                                "5: } ;\n"
                                "6: char A :: buf@1 [ 20 ] ;\n"
                                "7: int main ( )\n"
                                "8: {\n"
                                "9: char buf@2 [ 2 ] ;\n"
                                "10: A :: buf@1 [ 10 ] = 0 ;\n"
                                "11: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass7() {
        const std::string actual = tokenize(
            "int main()\n"
            "{\n"
            "  char buf[2];\n"
            "  A::buf[10] = 0;\n"
            "}");

        const char expected[] = "1: int main ( )\n"
                                "2: {\n"
                                "3: char buf@1 [ 2 ] ;\n"
                                "4: A :: buf [ 10 ] = 0 ;\n"
                                "5: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void varidclass8() {
        const char code[] ="class Fred {\n"
                            "public:\n"
                            "    void foo(int d) {\n"
                            "        int i = bar(x * d);\n"
                            "    }\n"
                            "    int x;\n"
                            "}\n";

        const char expected[] = "1: class Fred {\n"
                                "2: public:\n"
                                "3: void foo ( int d@1 ) {\n"
                                "4: int i@2 ; i@2 = bar ( x@3 * d@1 ) ;\n"
                                "5: }\n"
                                "6: int x@3 ;\n"
                                "7: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass9() {
        const char code[] ="typedef char Str[10];"
                            "class A {\n"
                            "public:\n"
                            "    void f(Str &cl);\n"
                            "    void g(Str cl);\n"
                            "}\n"
                            "void Fred::f(Str &cl) {\n"
                            "    sizeof(cl);\n"
                            "}";

        const char expected[] = "1: class A {\n"
                                "2: public:\n"
                                "3: void f ( char ( & cl@1 ) [ 10 ] ) ;\n"
                                "4: void g ( char cl@2 [ 10 ] ) ;\n"
                                "5: }\n"
                                "6: void Fred :: f ( char ( & cl@3 ) [ 10 ] ) {\n"
                                "7: sizeof ( cl@3 ) ;\n"
                                "8: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass10() {
        const char code[] ="class A {\n"
                            "    void f() {\n"
                            "        a = 3;\n"
                            "    }\n"
                            "    int a;\n"
                            "};\n";

        const char expected[] = "1: class A {\n"
                                "2: void f ( ) {\n"
                                "3: a@1 = 3 ;\n"
                                "4: }\n"
                                "5: int a@1 ;\n"
                                "6: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass11() {
        const char code[] ="class Fred {\n"
                            "    int a;\n"
                            "    void f();\n"
                            "};\n"
                            "class Wilma {\n"
                            "    int a;\n"
                            "    void f();\n"
                            "};\n"
                            "void Fred::f() { a = 0; }\n"
                            "void Wilma::f() { a = 0; }\n";

        const char expected[] = "1: class Fred {\n"
                                "2: int a@1 ;\n"
                                "3: void f ( ) ;\n"
                                "4: } ;\n"
                                "5: class Wilma {\n"
                                "6: int a@2 ;\n"
                                "7: void f ( ) ;\n"
                                "8: } ;\n"
                                "9: void Fred :: f ( ) { a@1 = 0 ; }\n"
                                "10: void Wilma :: f ( ) { a@2 = 0 ; }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass12() {
        const char code[] ="class Fred {\n"
                            "    int a;\n"
                            "    void f() { Fred::a = 0; }\n"
                            "};\n";

        const char expected[] = "1: class Fred {\n"
                                "2: int a@1 ;\n"
                                "3: void f ( ) { Fred :: a@1 = 0 ; }\n"
                                "4: } ;\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass13() {
        const char code[] ="class Fred {\n"
                            "    int a;\n"
                            "    void f() { Foo::Fred::a = 0; }\n"
                            "};\n";

        const char expected[] = "1: class Fred {\n"
                                "2: int a@1 ;\n"
                                "3: void f ( ) { Foo :: Fred :: a = 0 ; }\n"
                                "4: } ;\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass14() {
        // don't give friend classes varid
        {
            const char code[] ="class A {\n"
                                "friend class B;\n"
                                "}";

            const char expected[] = "1: class A {\n"
                                    "2: friend class B ;\n"
                                    "3: }\n";

            ASSERT_EQUALS(expected, tokenize(code));
        }

        {
            const char code[] ="class A {\n"
                                "private: friend class B;\n"
                                "}";

            const char expected[] = "1: class A {\n"
                                    "2: private: friend class B ;\n"
                                    "3: }\n";

            ASSERT_EQUALS(expected, tokenize(code));
        }
    }

    void varidclass15() {
        const char code[] = "class A {\n"
                            "    int a;\n"
                            "    int b;\n"
                            "    A();\n"
                            "};\n"
                            "A::A() : a(0) { b = 1; }";
        const char expected[] = "1: class A {\n"
                                "2: int a@1 ;\n"
                                "3: int b@2 ;\n"
                                "4: A ( ) ;\n"
                                "5: } ;\n"
                                "6: A :: A ( ) : a@1 ( 0 ) { b@2 = 1 ; }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass16() {
        const char code[] = "struct A;\n"
                            "typedef bool (A::* FuncPtr)();\n"
                            "struct A {\n"
                            "    FuncPtr pFun;\n"
                            "    void setPFun(int mode);\n"
                            "    bool funcNorm();\n"
                            "};\n"
                            "void A::setPFun(int mode) {\n"
                            "    pFun = &A::funcNorm;\n"
                            "}";
        const char expected[] = "1: struct A ;\n"
                                "2:\n"
                                "3: struct A {\n"
                                "4: bool ( * pFun@1 ) ( ) ;\n"
                                "5: void setPFun ( int mode@2 ) ;\n"
                                "6: bool funcNorm ( ) ;\n"
                                "7: } ;\n"
                                "8: void A :: setPFun ( int mode@3 ) {\n"
                                "9: pFun@1 = & A :: funcNorm ;\n"
                                "10: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass17() {
        const char code[] = "class A: public B, public C::D {\n"
                            "    int i;\n"
                            "    A(int i): B(i), C::D(i), i(i) {\n"
                            "        int j(i);\n"
                            "    }\n"
                            "};";
        const char expected[] = "1: class A : public B , public C :: D {\n"
                                "2: int i@1 ;\n"
                                "3: A ( int i@2 ) : B ( i@2 ) , C :: D ( i@2 ) , i@1 ( i@2 ) {\n"
                                "4: int j@3 ( i@2 ) ;\n"
                                "5: }\n"
                                "6: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));

        const char code2[] = "struct S {\n" // #11411
                             "    std::vector<int> v;\n"
                             "    int i;\n"
                             "    S(int i) : v({ 0 }), i(i) {}\n"
                             "};";
        const char expected2[] = "1: struct S {\n"
                                 "2: std :: vector < int > v@1 ;\n"
                                 "3: int i@2 ;\n"
                                 "4: S ( int i@3 ) : v@1 ( { 0 } ) , i@2 ( i@3 ) { }\n"
                                 "5: } ;\n";
        ASSERT_EQUALS(expected2, tokenize(code2));
    }

    void varidclass18() {
        const char code[] = "class A {\n"
                            "    int a;\n"
                            "    int b;\n"
                            "    A();\n"
                            "};\n"
                            "A::A() : a{0} { b = 1; }";
        const char expected[] = "1: class A {\n"
                                "2: int a@1 ;\n"
                                "3: int b@2 ;\n"
                                "4: A ( ) ;\n"
                                "5: } ;\n"
                                "6: A :: A ( ) : a@1 { 0 } { b@2 = 1 ; }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass19() {
        const char code[] = "class A : public ::B {\n"
                            "  int a;\n"
                            "  A();\n"
                            "};\n"
                            "A::A() : ::B(), a(0) {}";
        const char expected[] = "1: class A : public :: B {\n"
                                "2: int a@1 ;\n"
                                "3: A ( ) ;\n"
                                "4: } ;\n"
                                "5: A :: A ( ) : :: B ( ) , a@1 ( 0 ) { }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidclass20() { // #7578: int (*p)[2]
        const char code[] = "struct S {\n"
                            "  int (*p)[2];\n"
                            "  S();\n"
                            "};\n"
                            "S::S() { p[0] = 0; }";
        const char expected[] = "1: struct S {\n"
                                "2: int ( * p@1 ) [ 2 ] ;\n"
                                "3: S ( ) ;\n"
                                "4: } ;\n"
                                "5: S :: S ( ) { p@1 [ 0 ] = 0 ; }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidenum1() {
        const char code[] = "const int eStart = 6;\n"
                            "enum myEnum {\n"
                            "  A = eStart\n"
                            "};\n";
        const char expected[] = "1: const int eStart@1 = 6 ;\n"
                                "2: enum myEnum {\n"
                                "3: A = eStart@1\n"
                                "4: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidenum2() {
        const char code[] = "const int eStart = 6;\n"
                            "enum myEnum {\n"
                            "  A = f(eStart)\n"
                            "};\n";
        const char expected[] = "1: const int eStart@1 = 6 ;\n"
                                "2: enum myEnum {\n"
                                "3: A = f ( eStart@1 )\n"
                                "4: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidenum3() {
        const char code[] = "const int eStart = 6;\n"
                            "enum myEnum {\n"
                            "  A = f(eStart, x)\n"
                            "};\n";
        const char expected[] = "1: const int eStart@1 = 6 ;\n"
                                "2: enum myEnum {\n"
                                "3: A = f ( eStart@1 , x )\n"
                                "4: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidenum4() {
        const char code[] = "const int eStart = 6;\n"
                            "enum myEnum {\n"
                            "  A = f(x, eStart)\n"
                            "};\n";
        const char expected[] = "1: const int eStart@1 = 6 ;\n"
                                "2: enum myEnum {\n"
                                "3: A = f ( x , eStart@1 )\n"
                                "4: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidenum5() {
        const char code[] = "const int eStart = 6;\n"
                            "enum myEnum {\n"
                            "  A = f(x, eStart, y)\n"
                            "};\n";
        const char expected[] = "1: const int eStart@1 = 6 ;\n"
                                "2: enum myEnum {\n"
                                "3: A = f ( x , eStart@1 , y )\n"
                                "4: } ;\n";
        const char current[] = "1: const int eStart@1 = 6 ;\n"
                               "2: enum myEnum {\n"
                               "3: A = f ( x , eStart , y )\n"
                               "4: } ;\n";
        TODO_ASSERT_EQUALS(expected, current, tokenize(code));
    }

    void varidenum6() { // #9180
        const char code[] = "const int IDL1 = 13;\n"
                            "enum class E { IDL1 = 16, };\n";
        const char expected[] = "1: const int IDL1@1 = 13 ;\n"
                                "2: enum class E { IDL1 = 16 , } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidenum7() { // #8991
        const char code[] = "namespace N1 { const int c = 42; }\n"
                            "namespace N2 { const int c = 24; }\n"
                            "struct S {\n"
                            "    enum { v1 = N1::c, v2 = N2::c };\n"
                            "};\n";
        const char expected[] = "1: namespace N1 { const int c@1 = 42 ; }\n"
                                "2: namespace N2 { const int c@2 = 24 ; }\n"
                                "3: struct S {\n"
                                "4: enum Anonymous0 { v1 = N1 :: c@1 , v2 = N2 :: c@2 } ;\n"
                                "5: } ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varid_classnameshaddowsvariablename() {
        const char code[] = "class Data;\n"
                            "void strange_declarated(const Data& Data);\n"
                            "void handleData(const Data& data) {\n"
                            "    strange_declarated(data);\n"
                            "}\n";
        const char expected[] = "1: class Data ;\n"
                                "2: void strange_declarated ( const Data & Data@1 ) ;\n"
                                "3: void handleData ( const Data & data@2 ) {\n"
                                "4: strange_declarated ( data@2 ) ;\n"
                                "5: }\n";
        ASSERT_EQUALS(expected, tokenize(code));

    }

    void varid_classnametemplate() {
        const char code[] = "template <typename T>\n"
                            "struct BBB {\n"
                            "  struct inner;\n"
                            "};\n"
                            "\n"
                            "template <typename T>\n"
                            "struct BBB<T>::inner {\n"
                            "  inner(int x);\n"
                            "  int x;\n"
                            "};\n"
                            "\n"
                            "template <typename T>\n"
                            "BBB<T>::inner::inner(int x): x(x) {}\n";
        const char expected[] = "1: template < typename T >\n"
                                "2: struct BBB {\n"
                                "3: struct inner ;\n"
                                "4: } ;\n"
                                "5:\n"
                                "6: template < typename T >\n"
                                "7: struct BBB < T > :: inner {\n"
                                "8: inner ( int x@1 ) ;\n"
                                "9: int x@2 ;\n"
                                "10: } ;\n"
                                "11:\n"
                                "12: template < typename T >\n"
                                "13: BBB < T > :: inner :: inner ( int x@3 ) : x@2 ( x@3 ) { }\n";
        ASSERT_EQUALS(expected, tokenize(code));

    }

    void varidnamespace1() {
        const char code[] = "namespace A {\n"
                            "    char buf[20];\n"
                            "}\n"
                            "int main() {\n"
                            "    return foo(A::buf);\n"
                            "}";

        const char expected[] = "1: namespace A {\n"
                                "2: char buf@1 [ 20 ] ;\n"
                                "3: }\n"
                                "4: int main ( ) {\n"
                                "5: return foo ( A :: buf@1 ) ;\n"
                                "6: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void varidnamespace2() {
        const char code[] = "namespace A {\n"
                            "  namespace B {\n"
                            "    char buf[20];\n"
                            "  }\n"
                            "}\n"
                            "int main() {\n"
                            "  return foo(A::B::buf);\n"
                            "}";

        const char expected[] = "1: namespace A {\n"
                                "2: namespace B {\n"
                                "3: char buf@1 [ 20 ] ;\n"
                                "4: }\n"
                                "5: }\n"
                                "6: int main ( ) {\n"
                                "7: return foo ( A :: B :: buf@1 ) ;\n"
                                "8: }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void usingNamespace1() {
        const char code[] = "namespace NS {\n"
                            "  class A { int x; void dostuff(); };\n"
                            "}\n"
                            "using namespace NS;\n"
                            "void A::dostuff() { x = 0; }\n";
        const char expected[] = "1: namespace NS {\n"
                                "2: class A { int x@1 ; void dostuff ( ) ; } ;\n"
                                "3: }\n"
                                "4: using namespace NS ;\n"
                                "5: void A :: dostuff ( ) { x@1 = 0 ; }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void usingNamespace2() {
        const char code[] = "class A { int x; void dostuff(); };\n"
                            "using namespace NS;\n"
                            "void A::dostuff() { x = 0; }\n";
        const char expected[] = "1: class A { int x@1 ; void dostuff ( ) ; } ;\n"
                                "2: using namespace NS ;\n"
                                "3: void A :: dostuff ( ) { x@1 = 0 ; }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void usingNamespace3() {
        const char code[] = "namespace A {\n"
                            "    namespace B {\n"
                            "        class C {\n"
                            "            double m;\n"
                            "            C();\n"
                            "        };\n"
                            "    }\n"
                            "}\n"
                            "using namespace A::B;\n"
                            "C::C() : m(42) {}";

        const char expected[] = "1: namespace A {\n"
                                "2: namespace B {\n"
                                "3: class C {\n"
                                "4: double m@1 ;\n"
                                "5: C ( ) ;\n"
                                "6: } ;\n"
                                "7: }\n"
                                "8: }\n"
                                "9: using namespace A :: B ;\n"
                                "10: C :: C ( ) : m@1 ( 42 ) { }\n";

        ASSERT_EQUALS(expected, tokenize(code));
    }

    void setVarIdStructMembers1() {
        const char code[] = "void f(Foo foo)\n"
                            "{\n"
                            "    foo.size = 0;\n"
                            "    return ((uint8_t)(foo).size);\n"
                            "}";
        const char expected[] = "1: void f ( Foo foo@1 )\n"
                                "2: {\n"
                                "3: foo@1 . size@2 = 0 ;\n"
                                "4: return ( ( uint8_t ) ( foo@1 ) . size@2 ) ;\n"
                                "5: }\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void decltype1() {
        const char code[] = "void foo(int x, decltype(A::b) *p);";
        const char expected[] = "1: void foo ( int x@1 , decltype ( A :: b ) * p@2 ) ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void decltype2() {
        const char code[] = "int x; decltype(x) y;";
        const char expected[] = "1: int x@1 ; decltype ( x@1 ) y@2 ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void typeof1() {
        const char code[] = "int x; typeof(x) y;";
        const char expected[] = "1: int x@1 ; typeof ( x@1 ) y@2 ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void typeof2() {
        const char code[] = "int x; typeof(x) *y;";
        const char expected[] = "1: int x@1 ; typeof ( x@1 ) * y@2 ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void typeof3() {
        const char code[] = "int x; const typeof(x) *const y;";
        const char expected[] = "1: int x@1 ; const typeof ( x@1 ) * const y@2 ;\n";
        ASSERT_EQUALS(expected, tokenize(code));
    }

    void typeof4() {
        const char code[] = "int x; __typeof(x) y;";
        const char expected[] = "1: int x@1 ; __typeof ( x@1 ) y@2 ;\n";
        TokenizeOptions options;
        options.cpp = false;
        ASSERT_EQUALS(expected, tokenize(code, options));
    }

    void exprid1() {
        const std::string actual = tokenizeExpr(
            "struct A {\n"
            "    int x, y;\n"
            "};\n"
            "int f(A a, A b) {\n"
            "    int x = a.x + b.x;\n"
            "    int y = b.x + a.x;\n"
            "    return x + y + a.y + b.y;\n"
            "}\n");

        const char expected[] =
            "1: struct A {\n"
            "2: int x ; int y ;\n"
            "3: } ;\n"
            "4: int f ( A a , A b ) {\n"
            "5: int x@5 ; x@5 =@UNIQUE a@3 .@11 x@6 +@13 b@4 .@12 x@7 ;\n"
            "6: int y@8 ; y@8 =@UNIQUE b@4 .@12 x@7 +@13 a@3 .@11 x@6 ;\n"
            "7: return x@5 +@UNIQUE y@8 +@UNIQUE a@3 .@UNIQUE y@9 +@UNIQUE b@4 .@UNIQUE y@10 ;\n"
            "8: }\n";

        ASSERT_EQUALS(expected, actual);
    }

    void exprid2() {
        const std::string actual = tokenizeExpr( // #11739
            "struct S { std::unique_ptr<int> u; };\n"
            "auto f = [](const S& s) -> std::unique_ptr<int> {\n"
            "    if (auto p = s.u.get())\n"
            "        return std::make_unique<int>(*p);\n"
            "    return nullptr;\n"
            "};\n");

        const char expected[] = "1: struct S { std :: unique_ptr < int > u ; } ;\n"
                                "2: auto f ; f = [ ] ( const S & s ) . std :: unique_ptr < int > {\n"
                                "3: if ( auto p@4 =@UNIQUE s@3 .@UNIQUE u@5 .@UNIQUE get (@UNIQUE ) ) {\n"
                                "4: return std ::@UNIQUE make_unique < int > (@UNIQUE *@UNIQUE p@4 ) ; }\n"
                                "5: return nullptr ;\n"
                                "6: } ;\n";

        ASSERT_EQUALS(expected, actual);
    }

    void exprid3() {
        const char code[] = "void f(bool b, int y) {\n"
                            "    if (b && y > 0) {}\n"
                            "    while (b && y > 0) {}\n"
                            "}\n";
        const char expected[] = "1: void f ( bool b , int y ) {\n"
                                "2: if ( b@1 &&@5 y@2 >@4 0 ) { }\n"
                                "3: while ( b@1 &&@5 y@2 >@4 0 ) { }\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenizeExpr(code));
    }

    void exprid4() {
        // expanded macro..
        const char code[] = "#define ADD(x,y)  x+y\n"
                            "int f(int a, int b) {\n"
                            "    return ADD(a,b) + ADD(a,b);\n"
                            "}\n";
        const char expected[] = "2: int f ( int a , int b ) {\n"
                                "3: return a@1 $+@UNIQUE b@2 +@UNIQUE a@1 $+@UNIQUE b@2 ;\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenizeExpr(code));
    }

    void exprid5() {
        // references..
        const char code[] = "int foo(int a) {\n"
                            "    int& r = a;\n"
                            "    return (a+a)*(r+r);\n"
                            "}\n";
        const char expected[] = "1: int foo ( int a ) {\n"
                                "2: int & r@2 =@UNIQUE a@1 ;\n"
                                "3: return ( a@1 +@4 a@1 ) *@UNIQUE ( r@2 +@4 r@2 ) ;\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenizeExpr(code));
    }

    void exprid6() {
        // ++ and -- should have UNIQUE exprid
        const char code[] = "void foo(int *a) {\n"
                            "    *a++ = 0;\n"
                            "    if (*a++ == 32) {}\n"
                            "}\n";
        const char expected[] = "1: void foo ( int * a ) {\n"
                                "2: *@UNIQUE a@1 ++@UNIQUE = 0 ;\n"
                                "3: if ( *@UNIQUE a@1 ++@UNIQUE ==@UNIQUE 32 ) { }\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenizeExpr(code));
    }

    void exprid7() {
        // different casts
        const char code[] = "void foo(int a) {\n"
                            "    if ((char)a == (short)a) {}\n"
                            "    if ((char)a == (short)a) {}\n"
                            "}\n";
        const char expected[] = "1: void foo ( int a ) {\n"
                                "2: if ( (@2 char ) a@1 ==@4 (@3 short ) a@1 ) { }\n"
                                "3: if ( (@2 char ) a@1 ==@4 (@3 short ) a@1 ) { }\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenizeExpr(code));
    }

    void exprid8() {
        const char code[] = "void f() {\n" // #12249
                            "    std::string s;\n"
                            "    (((s += \"--\") += std::string()) += \"=\");\n"
                            "}\n";
        const char expected[] = "1: void f ( ) {\n"
                                "2: std ::@UNIQUE string s@1 ;\n"
                                "3: ( ( s@1 +=@UNIQUE \"--\"@UNIQUE ) +=@UNIQUE std ::@UNIQUE string (@UNIQUE ) ) +=@UNIQUE \"=\"@UNIQUE ;\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenizeExpr(code));

        const char code2[] = "struct S { std::function<void()>* p; };\n"
                             "S f() { return S{ std::make_unique<std::function<void()>>([]() {}).release()}; }";
        const char expected2[] = "1: struct S { std :: function < void ( ) > * p ; } ;\n"
                                 "2: S f ( ) { return S@UNIQUE {@UNIQUE std ::@UNIQUE make_unique < std :: function < void ( ) > > (@UNIQUE [ ] ( ) { } ) .@UNIQUE release (@UNIQUE ) } ; }\n";
        ASSERT_EQUALS(expected2, tokenizeExpr(code2));

        const char code3[] = "struct S { int* p; };\n"
                             "S f() { return S{ std::make_unique<int>([]() { return 4; }()).release()}; }\n";
        const char expected3[] = "1: struct S { int * p ; } ;\n"
                                 "2: S f ( ) { return S@UNIQUE {@UNIQUE std ::@UNIQUE make_unique < int > (@UNIQUE [ ] ( ) { return 4 ; } ( ) ) .@UNIQUE release (@UNIQUE ) } ; }\n";
        ASSERT_EQUALS(expected3, tokenizeExpr(code3));

        const char code4[] = "std::unique_ptr<int> g(int i) { return std::make_unique<int>(i); }\n"
                             "void h(int*);\n"
                             "void f() {\n"
                             "    h(g({}).get());\n"
                             "}\n";
        const char expected4[] = "1: std :: unique_ptr < int > g ( int i ) { return std ::@UNIQUE make_unique < int > (@UNIQUE i@1 ) ; }\n"
                                 "2: void h ( int * ) ;\n"
                                 "3: void f ( ) {\n"
                                 "4: h (@UNIQUE g (@UNIQUE { } ) .@UNIQUE get (@UNIQUE ) ) ;\n"
                                 "5: }\n";
        ASSERT_EQUALS(expected4, tokenizeExpr(code4));

        const char code5[] = "int* g(std::map<int, std::unique_ptr<int>>& m) {\n"
                             "    return m[{0}].get();\n"
                             "}\n";
        const char expected5[] = "1: int * g ( std :: map < int , std :: unique_ptr < int > > & m ) {\n"
                                 "2: return m@1 [@UNIQUE { 0 } ] .@UNIQUE get (@UNIQUE ) ;\n"
                                 "3: }\n";
        ASSERT_EQUALS(expected5, tokenizeExpr(code5));
    }

    void exprid9()
    {
        const char code[] = "void f(const std::type_info& type) {\n" // #12340
                            "    if (type == typeid(unsigned int)) {}\n"
                            "    else if (type == typeid(int)) {}\n"
                            "}\n";
        const char expected[] = "1: void f ( const std :: type_info & type ) {\n"
                                "2: if ( type@1 ==@UNIQUE typeid (@UNIQUE unsigned int@UNIQUE ) ) { }\n"
                                "3: else { if ( type@1 ==@UNIQUE typeid (@UNIQUE int@UNIQUE ) ) { } }\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenizeExpr(code));
    }

    void exprid10()
    {
        const char code[] = "void f(const std::string& p) {\n" // #12350
                            "    std::string s;\n"
                            "    ((s = \"abc\") += p) += \"def\";\n"
                            "}\n";
        const char expected[] = "1: void f ( const std :: string & p ) {\n"
                                "2: std ::@UNIQUE string s@2 ;\n"
                                "3: ( ( s@2 =@UNIQUE \"abc\" ) +=@UNIQUE p@1 ) +=@UNIQUE \"def\"@UNIQUE ;\n"
                                "4: }\n";
        ASSERT_EQUALS(expected, tokenizeExpr(code));
    }

    void exprid11()
    {
        const char code[] = "struct S { void f(); };\n" // #12713
                            "int g(int, S*);\n"
                            "int h(void (*)(), S*);\n"
                            "void S::f() {\n"
                            "    std::make_unique<int>(g({}, this)).release();\n"
                            "    std::make_unique<int>(h([]() {}, this)).release();\n"
                            "}\n";
        const char* exp = "1: struct S { void f ( ) ; } ;\n"
                          "2: int g ( int , S * ) ;\n"
                          "3: int h ( void ( * ) ( ) , S * ) ;\n"
                          "4: void S :: f ( ) {\n"
                          "5: std ::@UNIQUE make_unique < int > (@UNIQUE g (@UNIQUE { } ,@6 this ) ) .@UNIQUE release (@UNIQUE ) ;\n"
                          "6: std ::@UNIQUE make_unique < int > (@UNIQUE h (@UNIQUE [ ] ( ) { } ,@6 this ) ) .@UNIQUE release (@UNIQUE ) ;\n"
                          "7: }\n";
        ASSERT_EQUALS(exp, tokenizeExpr(code));
    }

    void exprid12()
    {
        const char code[] = "struct S { std::unique_ptr<int> p; };\n" // #12765
                            "namespace N {\n"
                            "    struct T { void (*f)(S*); };\n"
                            "    const T t = {\n"
                            "        [](S* s) { s->p.release(); }\n"
                            "    };\n"
                            "}\n";
        const char* exp = "1: struct S { std :: unique_ptr < int > p ; } ;\n"
                          "2: namespace N {\n"
                          "3: struct T { void ( * f@2 ) ( S * ) ; } ;\n"
                          "4: const T t@3 = {\n"
                          "5: [ ] ( S * s@4 ) { s@4 .@UNIQUE p@5 .@UNIQUE release (@UNIQUE ) ; }\n"
                          "6: } ;\n"
                          "7: }\n";
        ASSERT_EQUALS(exp, tokenizeExpr(code));
    }

    void exprid13()
    {
        const char code[] = "struct S { int s; };\n" // #11488
                            "struct T { struct S s; };\n"
                            "struct U { struct T t; };\n"
                            "void f() {\n"
                            "    struct U u = { .t = { .s = { .s = 1 } } };\n"
                            "}\n";
        const char* exp = "1: struct S { int s ; } ;\n"
                          "2: struct T { struct S s ; } ;\n"
                          "3: struct U { struct T t ; } ;\n"
                          "4: void f ( ) {\n"
                          "5: struct U u@4 ; u@4 = { .@UNIQUE t@5 = { .@UNIQUE s@6 = { .@UNIQUE s@7 = 1 } } } ;\n"
                          "6: }\n";
        ASSERT_EQUALS(exp, tokenizeExpr(code));
    }

    void exprid14()
    {
        const Settings s = settingsBuilder(settings).library("std.cfg").build();

        const char code[] = "int f(double a, double b, double c) {\n" // #13578
                            "    return static_cast<int>(std::ceil((std::min)(a, (std::min)(b, c))));\n"
                            "}\n";
        const char* exp = "1: int f ( double a@1 , double b@2 , double c@3 ) {\n"
                          "2: return static_cast < int > ( std :: ceil ( std :: min ( a@1 , std :: min ( b@2 , c@3 ) ) ) ) ;\n"
                          "3: }\n";
        ASSERT_EQUALS(exp, tokenize(code, s)); // don't crash
    }

    void structuredBindings() {
        const char code[] = "int foo() { auto [x,y] = xy(); return x+y; }";
        ASSERT_EQUALS("1: int foo ( ) { auto [ x@1 , y@2 ] = xy ( ) ; return x@1 + y@2 ; }\n",
                      tokenize(code));
    }
};

REGISTER_TEST(TestVarID)
