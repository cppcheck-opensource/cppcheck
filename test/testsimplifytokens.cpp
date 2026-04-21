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

#include "errortypes.h"
#include "helpers.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "fixture.h"
#include "token.h"

#include <cstddef>
#include <string>

class TestSimplifyTokens : public TestFixture {
public:
    TestSimplifyTokens() : TestFixture("TestSimplifyTokens") {}


private:
    const Settings settings0 = settingsBuilder().severity(Severity::portability).build();
    const Settings settings1 = settingsBuilder().severity(Severity::style).build();
    const Settings settings_std = settingsBuilder().library("std.cfg").build();
    const Settings settings_windows = settingsBuilder().library("windows.cfg").severity(Severity::portability).build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(combine_strings);
        TEST_CASE(combine_wstrings);
        TEST_CASE(combine_ustrings);
        TEST_CASE(combine_Ustrings);
        TEST_CASE(combine_u8strings);
        TEST_CASE(combine_mixedstrings);

        TEST_CASE(double_plus);
        TEST_CASE(redundant_plus);
        TEST_CASE(redundant_plus_numbers);
        TEST_CASE(declareVar);

        TEST_CASE(declareArray);

        TEST_CASE(dontRemoveIncrement);

        TEST_CASE(elseif1);

        TEST_CASE(namespaces);

        // Simplify "not" to "!" (#345)
        TEST_CASE(not1);

        // Simplify "and" to "&&" (#620)
        TEST_CASE(and1);

        // Simplify "or" to "||"
        TEST_CASE(or1);

        TEST_CASE(cAlternativeTokens);

        TEST_CASE(comma_keyword);

        TEST_CASE(simplifyOperator1);
        TEST_CASE(simplifyOperator2);

        TEST_CASE(simplifyArrayAccessSyntax);

        TEST_CASE(pointeralias1);
        TEST_CASE(pointeralias3);

        // struct ABC { } abc; => struct ABC { }; ABC abc;
        TEST_CASE(simplifyStructDecl1);
        TEST_CASE(simplifyStructDecl2); // ticket #2579
        TEST_CASE(simplifyStructDecl3);
        TEST_CASE(simplifyStructDecl4);
        TEST_CASE(simplifyStructDecl6); // ticket #3732
        TEST_CASE(simplifyStructDecl7); // ticket #476 (static anonymous struct array)
        TEST_CASE(simplifyStructDecl8); // ticket #7698
        TEST_CASE(simplifyStructDecl9);

        // register int var; => int var;
        // inline int foo() {} => int foo() {}
        TEST_CASE(removeUnwantedKeywords);

        // remove calling convention __cdecl, __stdcall, ...
        TEST_CASE(simplifyCallingConvention);

        // remove __attribute, __attribute__
        TEST_CASE(simplifyAttribute);

        TEST_CASE(simplifyFunctorCall);

        TEST_CASE(simplifyFunctionPointer); // ticket #5339 (simplify function pointer after comma)

        TEST_CASE(simplifyFunctionReturn);

        TEST_CASE(consecutiveBraces);

        TEST_CASE(simplifyOverride); // ticket #5069
        TEST_CASE(simplifyNestedNamespace);
        TEST_CASE(simplifyNamespaceAliases1);
        TEST_CASE(simplifyNamespaceAliases2); // ticket #10281
        TEST_CASE(simplifyNamespaceAliases3);

        TEST_CASE(simplifyKnownVariables2);
        TEST_CASE(simplifyKnownVariables3);
        TEST_CASE(simplifyKnownVariables4);
        TEST_CASE(simplifyKnownVariables5);
        TEST_CASE(simplifyKnownVariables13);
        TEST_CASE(simplifyKnownVariables14);
        TEST_CASE(simplifyKnownVariables16);
        TEST_CASE(simplifyKnownVariables17);
        TEST_CASE(simplifyKnownVariables18);
        TEST_CASE(simplifyKnownVariables19);
        TEST_CASE(simplifyKnownVariables21);
        TEST_CASE(simplifyKnownVariables25);
        // FIXME Does expression id handle these? TEST_CASE(simplifyKnownVariables29);    // ticket #1811
        TEST_CASE(simplifyKnownVariables30);
        TEST_CASE(simplifyKnownVariables34);
        TEST_CASE(simplifyKnownVariables36);    // ticket #5972
        TEST_CASE(simplifyKnownVariables42);    // ticket #2031 - known string value after strcpy
        TEST_CASE(simplifyKnownVariables43);
        TEST_CASE(simplifyKnownVariables44);    // ticket #3117 - don't simplify static variables
        TEST_CASE(simplifyKnownVariables46);    // ticket #3587 - >>
        TEST_CASE(simplifyKnownVariables47);    // ticket #3627 - >>
        TEST_CASE(simplifyKnownVariables48);    // ticket #3754 - wrong simplification in for loop header
        TEST_CASE(simplifyKnownVariables49);    // #3691 - continue in switch
        TEST_CASE(simplifyKnownVariables50);    // #4066 sprintf changes
        TEST_CASE(simplifyKnownVariables51);    // #4409 hang
        TEST_CASE(simplifyKnownVariables54);    // #4913 'x' is not 0 after *--x=0;
        TEST_CASE(simplifyKnownVariables56);    // ticket #5301 - >>
        TEST_CASE(simplifyKnownVariables58);    // ticket #5268
        TEST_CASE(simplifyKnownVariables59);    // skip for header
        TEST_CASE(simplifyKnownVariables61);    // #7805
        TEST_CASE(simplifyKnownVariables62);    // #5666 - p=&str[0]
        TEST_CASE(simplifyKnownVariables63);    // #10798
        TEST_CASE(simplifyKnownVariablesBailOutAssign1);
        TEST_CASE(simplifyKnownVariablesBailOutAssign2);
        TEST_CASE(simplifyKnownVariablesBailOutFor1);
        TEST_CASE(simplifyKnownVariablesBailOutFor2);
        TEST_CASE(simplifyKnownVariablesBailOutFor3);
        TEST_CASE(simplifyKnownVariablesBailOutMemberFunction);
        TEST_CASE(simplifyKnownVariablesBailOutConditionalIncrement);
        TEST_CASE(simplifyKnownVariablesBailOutSwitchBreak); // ticket #2324
        TEST_CASE(simplifyKnownVariablesClassMember);  // #2815 - value of class member may be changed by function call
        TEST_CASE(simplifyKnownVariablesFunctionCalls); // Function calls (don't assume pass by reference)
        TEST_CASE(simplifyKnownVariablesGlobalVars);
        TEST_CASE(simplifyKnownVariablesNamespace); // #10059

        TEST_CASE(simplify_constants6);     // Ticket #5625: Ternary operator as template parameter
        TEST_CASE(simplifyVarDeclInitLists);
    }

    struct TokOptions
    {
        bool cpp = true;
        Platform::Type type = Platform::Type::Native;
    };

#define tok(...) tok_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tok_(const char* file, int line, const char (&code)[size], const TokOptions& options = make_default_obj()) {
        const Settings settings = settingsBuilder(settings0).platform(options.type).build();
        SimpleTokenizer tokenizer(settings, *this, options.cpp);

        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        return tokenizer.tokens()->stringifyList(nullptr, false);
    }

    struct TokenizeAndStringifyOptions
    {
        Platform::Type platform = Platform::Type::Native;
        bool cpp = true;
    };

#define tokenizeAndStringify(...) tokenizeAndStringify_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tokenizeAndStringify_(const char* file, int linenr, const char (&code)[size], const TokenizeAndStringifyOptions& options = make_default_obj()) {
        const Settings settings = settingsBuilder(settings1).debugwarnings().platform(options.platform).cpp(Standards::CPP03).build();

        // tokenize..
        SimpleTokenizer tokenizer(settings, *this, options.cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, linenr);

        if (tokenizer.tokens())
            return tokenizer.tokens()->stringifyList(false, true, false, true, false, nullptr, nullptr);
        return "";
    }

#define tokenizeDebugListing(...) tokenizeDebugListing_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tokenizeDebugListing_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer tokenizer(settings0, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // result..
        return tokenizer.tokens()->stringifyList(true, false, true, true, true);
    }



    void combine_strings() {
        const char code1[] =  "void foo()\n"
                             "{\n"
                             "const char *a =\n"
                             "{\n"
                             "\"hello \"\n"
                             "\"world\"\n"
                             "};\n"
                             "}\n";

        const char code2[] =  "void foo()\n"
                             "{\n"
                             "const char *a =\n"
                             "{\n"
                             "\"hello world\"\n"
                             "};\n"
                             "}\n";
        ASSERT_EQUALS(tok(code2), tok(code1));

        const char code3[] = "x = L\"1\" TEXT(\"2\") L\"3\";";
        ASSERT_EQUALS("x = L\"123\" ;", tok(code3, dinit(TokOptions, $.type = Platform::Type::Win64)));

        const char code4[] = "x = TEXT(\"1\") L\"2\";";
        ASSERT_EQUALS("x = L\"1\" L\"2\" ;", tok(code4, dinit(TokOptions, $.type = Platform::Type::Win64)));
    }

    void combine_wstrings() {
        const char code[] =  "a = L\"hello \"  L\"world\" ;\n";

        const char expected[] =  "a = L\"hello world\" ;";

        SimpleTokenizer tokenizer(settings0, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_ustrings() {
        const char code[] =  "abcd = u\"ab\" u\"cd\";";

        const char expected[] =  "abcd = u\"abcd\" ;";

        SimpleTokenizer tokenizer(settings0, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_Ustrings() {
        const char code[] =  "abcd = U\"ab\" U\"cd\";";

        const char expected[] =  "abcd = U\"abcd\" ;";

        SimpleTokenizer tokenizer(settings0, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_u8strings() {
        const char code[] =  "abcd = u8\"ab\" u8\"cd\";";

        const char expected[] =  "abcd = u8\"abcd\" ;";

        SimpleTokenizer tokenizer(settings0, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void combine_mixedstrings() {
        const char code[] = "abcdef = \"ab\" L\"cd\" \"ef\";";

        const char expected[] =  "abcdef = L\"abcdef\" ;";

        SimpleTokenizer tokenizer(settings0, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
    }

    void double_plus() {
        {
            const char code1[] =  "void foo( int a )\n"
                                 "{\n"
                                 "a++;\n"
                                 "a--;\n"
                                 "++a;\n"
                                 "--a;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a ++ ; a -- ; ++ a ; -- a ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                 "{\n"
                                 "a=a+a;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + a ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a+++b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a ++ + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a---b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a -- - b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a--+b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a -- + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a++-b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a ++ - b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a+--b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + -- b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a-++b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a - ++ b ; }", tok(code1));
        }
    }

    void redundant_plus() {
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a + + b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a + + + b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a + - b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a - b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a - + b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a - b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a - - b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a - + - b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a + b ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a, int b )\n"
                                 "{\n"
                                 "a=a - - - b;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a , int b ) { a = a - b ; }", tok(code1));
        }
    }

    void redundant_plus_numbers() {
        {
            const char code1[] =  "void foo( int a )\n"
                                 "{\n"
                                 "a=a + + 1;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                 "{\n"
                                 "a=a + + + 1;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                 "{\n"
                                 "a=a + - 1;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a - 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                 "{\n"
                                 "a=a - + 1;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a - 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                 "{\n"
                                 "a=a - - 1;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                 "{\n"
                                 "a=a - + - 1;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a + 1 ; }", tok(code1));
        }
        {
            const char code1[] =  "void foo( int a )\n"
                                 "{\n"
                                 "a=a - - - 1;\n"
                                 "}\n";
            ASSERT_EQUALS("void foo ( int a ) { a = a - 1 ; }", tok(code1));
        }
    }


    void declareVar() {
        const char code[] = "void f ( ) { char str [ 100 ] = \"100\" ; }";
        ASSERT_EQUALS(code, tok(code));
    }

    void declareArray() {
        const char code1[] = "void f ( ) { char str [ ] = \"100\" ; }";
        const char expected1[] = "void f ( ) { char str [ 4 ] = \"100\" ; }";
        ASSERT_EQUALS(expected1, tok(code1));

        const char code2[] = "char str [ ] = \"\\x00\";";
        const char expected2[] = "char str [ 2 ] = \"\\0\" ;";
        ASSERT_EQUALS(expected2, tok(code2));

        const char code3[] = "char str [ ] = \"\\0\";";
        const char expected3[] = "char str [ 2 ] = \"\\0\" ;";
        ASSERT_EQUALS(expected3, tok(code3));

        const char code4[] = "char str [ ] = \"\\n\\n\";";
        const char expected4[] = "char str [ 3 ] = \"\\n\\n\" ;";
        ASSERT_EQUALS(expected4, tok(code4));
    }

    void dontRemoveIncrement() {
        {
            const char code[] = "void f(int a)\n"
                                "{\n"
                                "    if (a > 10)\n"
                                "        a = 5;\n"
                                "    else\n"
                                "        a = 10;\n"
                                "    a++;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( int a ) { if ( a > 10 ) { a = 5 ; } else { a = 10 ; } a ++ ; }", tok(code));
        }

        {
            const char code[] = "void f(int a)\n"
                                "{\n"
                                "    if (a > 10)\n"
                                "        a = 5;\n"
                                "    else\n"
                                "        a = 10;\n"
                                "    ++a;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( int a ) { if ( a > 10 ) { a = 5 ; } else { a = 10 ; } ++ a ; }", tok(code));
        }
    }


    void elseif1() {
        const char code[] = "void f(){ if(x) {} else if(ab) { cd } else { ef }gh; }";
        ASSERT_EQUALS("\n\n##file 0\n1: void f ( ) { if ( x ) { } else { if ( ab ) { cd } else { ef } } gh ; }\n", tokenizeDebugListing(code));

        // syntax error: assert there is no segmentation fault
        ASSERT_EQUALS("\n\n##file 0\n1: void f ( ) { if ( x ) { } else { if ( x ) { } } }\n", tokenizeDebugListing("void f(){ if(x) {} else if (x) { } }"));

        {
            const char src[] =  "void f(int g,int f) {\n"
                               "if(g==1) {poo();}\n"
                               "else if( g == 2 )\n"
                               "{\n"
                               " if( f == 0 ){coo();}\n"
                               " else if( f==1)\n"
                               "  goo();\n"
                               "}\n"
                               "}";

            const char expected[] = "void f ( int g , int f ) "
                                    "{ "
                                    "if ( g == 1 ) { poo ( ) ; } "
                                    "else { "
                                    "if ( g == 2 ) "
                                    "{ "
                                    "if ( f == 0 ) { coo ( ) ; } "
                                    "else { "
                                    "if ( f == 1 ) "
                                    "{ "
                                    "goo ( ) ; "
                                    "} "
                                    "} "
                                    "} "
                                    "} "
                                    "}";
            ASSERT_EQUALS(tok(expected), tok(src));
        }

        // Ticket #6860 - lambdas
        {
            const char src[] = "( []{if (ab) {cd}else if(ef) { gh } else { ij }kl}() );";
            const char expected[] = "\n\n##file 0\n1: ( [ ] { if ( ab ) { cd } else { if ( ef ) { gh } else { ij } } kl } ( ) ) ;\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(src));
        }
        {
            const char src[] = "[ []{if (ab) {cd}else if(ef) { gh } else { ij }kl}() ];";
            const char expected[] = "\n\n##file 0\n1: [ [ ] { if ( ab ) { cd } else { if ( ef ) { gh } else { ij } } kl } ( ) ] ;\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(src));
        }
        {
            const char src[] = "= { []{if (ab) {cd}else if(ef) { gh } else { ij }kl}() }";
            const char expected[] = "\n\n##file 0\n1: = { [ ] { if ( ab ) { cd } else { if ( ef ) { gh } else { ij } } kl } ( ) }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(src));
        }
    }


    void namespaces() {
        {
            const char code[] = "namespace std { }";

            ASSERT_EQUALS(";", tok(code));
        }

        {
            const char code[] = "; namespace std { }";

            ASSERT_EQUALS(";", tok(code));
        }

        {
            const char code[] = "using namespace std; namespace a{ namespace b{ void f(){} } }";

            const char expected[] = "namespace a { namespace b { void f ( ) { } } }";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "namespace b{ void f(){} }";

            const char expected[] = "namespace b { void f ( ) { } }";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f(int namespace) { }";

            ASSERT_THROW_INTERNAL(tok(code), SYNTAX);
        }
    }

    void not1() {
        ASSERT_EQUALS("void f ( ) { if ( ! p ) { ; } }", tok("void f() { if (not p); }", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( p && ! q ) { ; } }", tok("void f() { if (p && not q); }", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { a = ! ( p && q ) ; }", tok("void f() { a = not(p && q); }", dinit(TokOptions, $.cpp = false)));
        // Don't simplify 'not' or 'compl' if they are defined as a type;
        // in variable declaration and in function declaration/definition
        ASSERT_EQUALS("struct not { int x ; } ;", tok("struct not { int x; };", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { not p ; compl c ; }", tok(" void f() { not p; compl c; }", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("void foo ( not i ) ;", tok("void foo(not i);", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("int foo ( not i ) { return g ( i ) ; }", tok("int foo(not i) { return g(i); }", dinit(TokOptions, $.cpp = false)));
    }

    void and1() {
        ASSERT_EQUALS("void f ( ) { if ( p && q ) { ; } }",
                      tok("void f() { if (p and q) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) && q ) { ; } }",
                      tok("void f() { if (foo() and q) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) && bar ( ) ) { ; } }",
                      tok("void f() { if (foo() and bar()) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { if ( p && bar ( ) ) { ; } }",
                      tok("void f() { if (p and bar()) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { if ( p && ! q ) { ; } }",
                      tok("void f() { if (p and not q) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { r = a && b ; }",
                      tok("void f() { r = a and b; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { r = ( a || b ) && ( c || d ) ; }",
                      tok("void f() { r = (a || b) and (c || d); }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { if ( test1 [ i ] == 'A' && test2 [ i ] == 'C' ) { } }",
                      tok("void f() { if (test1[i] == 'A' and test2[i] == 'C') {} }", dinit(TokOptions, $.cpp = false)));
    }

    void or1() {
        ASSERT_EQUALS("void f ( ) { if ( p || q ) { ; } }",
                      tok("void f() { if (p or q) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) || q ) { ; } }",
                      tok("void f() { if (foo() or q) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { if ( foo ( ) || bar ( ) ) { ; } }",
                      tok("void f() { if (foo() or bar()) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { if ( p || bar ( ) ) { ; } }",
                      tok("void f() { if (p or bar()) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { if ( p || ! q ) { ; } }",
                      tok("void f() { if (p or not q) ; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { r = a || b ; }",
                      tok("void f() { r = a or b; }", dinit(TokOptions, $.cpp = false)));

        ASSERT_EQUALS("void f ( ) { r = ( a && b ) || ( c && d ) ; }",
                      tok("void f() { r = (a && b) or (c && d); }", dinit(TokOptions, $.cpp = false)));
    }

    void cAlternativeTokens() {
        ASSERT_EQUALS("void f ( ) { err |= ( ( r & s ) && ! t ) ; }",
                      tok("void f() { err or_eq ((r bitand s) and not t); }", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) const { r = f ( a [ 4 ] | 0x0F , ~ c , ! d ) ; }",
                      tok("void f() const { r = f(a[4] bitor 0x0F, compl c, not d) ; }", dinit(TokOptions, $.cpp = false)));

    }

    void comma_keyword() {
        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b;\n"
                                "    delete a, delete b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; delete a , delete b ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    struct A *a, *b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { struct A * a ; struct A * b ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    struct A **a, **b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { struct A * * a ; struct A * * b ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b;\n"
                                "    delete a, b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; delete a , b ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b, *c;\n"
                                "    delete a, b, c;\n"
                                "}\n";
            // delete a; b; c; would be better but this will do too
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; char * c ; delete a , b , c ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b;\n"
                                "    if (x)\n"
                                "        delete a, b;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; if ( x ) { delete a , b ; } }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char *a, *b, *c;\n"
                                "    if (x) \n"
                                "        delete a, b, c;\n"
                                "}\n";
            // delete a; b; c; would be better but this will do too
            ASSERT_EQUALS("void foo ( ) { char * a ; char * b ; char * c ; if ( x ) { delete a , b , c ; } }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    char **a, **b, **c;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { char * * a ; char * * b ; char * * c ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    delete [] a, a = 0;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { delete [ ] a , a = 0 ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    delete a, a = 0;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { delete a , a = 0 ; }", tok(code));
        }

        {
            const char code[] = "void foo()\n"
                                "{\n"
                                "    if( x ) delete a, a = 0;\n"
                                "}\n";
            ASSERT_EQUALS("void foo ( ) { if ( x ) { delete a , a = 0 ; } }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    for(int a,b; a < 10; a = a + 1, b = b + 1);\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { for ( int a , b ; a < 10 ; a = a + 1 , b = b + 1 ) { ; } }", tok(code));
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    char buf[BUFSIZ], **p;\n"
                                "    char *ptrs[BUFSIZ], **pp;\n"
                                "}\n";
            ASSERT_EQUALS("void f ( ) { char buf [ BUFSIZ ] ; char * * p ; char * ptrs [ BUFSIZ ] ; char * * pp ; }", tok(code));
        }

        {
            // #4786 - don't replace , with ; in ".. : public B, C .." code
            const char code[] = "template < class T = X > class A : public B , C { } ;";
            ASSERT_EQUALS(code, tok(code));
        }
    }

    void simplifyOperator1() {
        // #3237 - error merging namespaces with operators
        const char code[] = "class c {\n"
                            "public:\n"
                            "    operator std::string() const;\n"
                            "    operator string() const;\n"
                            "};\n";
        const char expected[] = "class c { "
                                "public: "
                                "operatorstd::string ( ) const ; "
                                "operatorstring ( ) const ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyOperator2() {
        // #6576
        ASSERT_EQUALS("template < class T > class SharedPtr { "
                      "SharedPtr & operator= ( const SharedPtr < Y > & r ) ; "
                      "} ; "
                      "class TClass { "
                      "public: TClass & operator= ( const TClass & rhs ) ; "
                      "} ; "
                      "TClass :: TClass ( const TClass & other ) { operator= ( other ) ; }",
                      tok("template<class T>\n"
                          "    class SharedPtr {\n"
                          "    SharedPtr& operator=(SharedPtr<Y> const & r);\n"
                          "};\n"
                          "class TClass {\n"
                          "public:\n"
                          "    TClass& operator=(const TClass& rhs);\n"
                          "};\n"
                          "TClass::TClass(const TClass &other) {\n"
                          "    operator=(other);\n"
                          "}"));
    }

    void simplifyArrayAccessSyntax() {
        ASSERT_EQUALS("\n\n##file 0\n"
                      "1: int a@1 ; a@1 [ 13 ] ;\n", tokenizeDebugListing("int a; 13[a];"));
    }


    void pointeralias1() {
        {
            const char code[] = "void f(char *p1)\n"
                                "{\n"
                                "    char *p = p1;\n"
                                "    p1 = 0;"
                                "    x(p);\n"
                                "}\n";

            const char expected[] = "void f ( char * p1 ) "
                                    "{ "
                                    "char * p ; p = p1 ; "
                                    "p1 = 0 ; "
                                    "x ( p ) ; "
                                    "}";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void foo(Result* ptr)\n"
                                "{\n"
                                "    Result* obj = ptr;\n"
                                "    ++obj->total;\n"
                                "}\n";

            const char expected[] = "void foo ( Result * ptr ) "
                                    "{ "
                                    "Result * obj ; obj = ptr ; "
                                    "++ obj . total ; "
                                    "}";

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void pointeralias3() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int i, j, *p;\n"
                            "    if (ab) p = &i;\n"
                            "    else p = &j;\n"
                            "    *p = 0;\n"
                            "}\n";
        const char expected[] = "void f ( ) "
                                "{"
                                " int i ; int j ; int * p ;"
                                " if ( ab ) { p = & i ; }"
                                " else { p = & j ; }"
                                " * p = 0 ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyStructDecl1() {
        {
            const char code[] = "struct ABC { } abc;";
            const char expected[] = "struct ABC { } ; struct ABC abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct ABC { } * pabc;";
            const char expected[] = "struct ABC { } ; struct ABC * pabc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct ABC { } abc[4];";
            const char expected[] = "struct ABC { } ; struct ABC abc [ 4 ] ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct ABC { } abc, def;";
            const char expected[] = "struct ABC { } ; struct ABC abc ; struct ABC def ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct ABC { } abc, * pabc;";
            const char expected[] = "struct ABC { } ; struct ABC abc ; struct ABC * pabc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct ABC { struct DEF {} def; } abc;";
            const char expected[] = "struct ABC { struct DEF { } ; struct DEF def ; } ; struct ABC abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { } abc;";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { } * pabc;";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 * pabc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { } abc[4];";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 abc [ 4 ] ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct {int a;} const array[3] = {0};";
            const char expected[] = "struct Anonymous0 { int a ; } ; struct Anonymous0 const array [ 3 ] = { 0 } ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "static struct {int a;} const array[3] = {0};";
            const char expected[] = "struct Anonymous0 { int a ; } ; static struct Anonymous0 const array [ 3 ] = { 0 } ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { } abc, def;";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 abc ; struct Anonymous0 def ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { } abc, * pabc;";
            const char expected[] = "struct Anonymous0 { } ; struct Anonymous0 abc ; struct Anonymous0 * pabc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { struct DEF {} def; } abc;";
            const char expected[] = "struct Anonymous0 { struct DEF { } ; struct DEF def ; } ; struct Anonymous0 abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct ABC { struct {} def; } abc;";
            const char expected[] = "struct ABC { struct Anonymous0 { } ; struct Anonymous0 def ; } ; struct ABC abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { struct {} def; } abc;";
            const char expected[] = "struct Anonymous0 { struct Anonymous1 { } ; struct Anonymous1 def ; } ; struct Anonymous0 abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "union ABC { int i; float f; } abc;";
            const char expected[] = "union ABC { int i ; float f ; } ; union ABC abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct ABC { struct {} def; };";
            const char expected[] = "struct ABC { struct Anonymous0 { } ; struct Anonymous0 def ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct ABC : public XYZ { struct {} def; };";
            const char expected[] = "struct ABC : public XYZ { struct Anonymous0 { } ; struct Anonymous0 def ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { int x; }; int y;";
            const char expected[] = "int x ; int y ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { int x; };";
            const char expected[] = "int x ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { };";
            const char expected[] = ";";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { struct { struct { } ; } ; };";
            const char expected[] = ";";
            ASSERT_EQUALS(expected, tok(code));
        }

        // ticket 2464
        {
            const char code[] = "static struct ABC { } abc ;";
            const char expected[] = "struct ABC { } ; static struct ABC abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        // ticket #980
        {
            const char code[] = "void f() { int A(1),B(2),C=3,D,E(5),F=6; }";
            const char expected[] = "void f ( ) { int A ( 1 ) ; int B ( 2 ) ; int C ; C = 3 ; int D ; int E ( 5 ) ; int F ; F = 6 ; }";
            ASSERT_EQUALS(expected, tok(code));
        }

        // ticket #8284
        {
            const char code[] = "void f() { class : foo<int> { } abc; }";
            const char expected[] = "void f ( ) { class Anonymous0 : foo < int > { } ; Anonymous0 abc ; }";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void simplifyStructDecl2() { // ticket #2479 (segmentation fault)
        const char code[] = "struct { char c; }";
        const char expected[] = "struct { char c ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyStructDecl3() {
        {
            const char code[] = "class ABC { } abc;";
            const char expected[] = "class ABC { } ; ABC abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class ABC { } * pabc;";
            const char expected[] = "class ABC { } ; ABC * pabc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class ABC { } abc[4];";
            const char expected[] = "class ABC { } ; ABC abc [ 4 ] ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class ABC { } abc, def;";
            const char expected[] = "class ABC { } ; ABC abc ; ABC def ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class ABC { } abc, * pabc;";
            const char expected[] = "class ABC { } ; ABC abc ; ABC * pabc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class ABC { class DEF {} def; } abc;";
            const char expected[] = "class ABC { class DEF { } ; DEF def ; } ; ABC abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class { } abc;";
            const char expected[] = "class { } abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class { } * pabc;";
            const char expected[] = "class { } * pabc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class { } abc[4];";
            const char expected[] = "class { } abc [ 4 ] ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class { } abc, def;";
            const char expected[] = "class { } abc , def ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class { } abc, * pabc;";
            const char expected[] = "class { } abc , * pabc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "struct { class DEF {} def; } abc;";
            const char expected[] = "struct Anonymous0 { class DEF { } ; DEF def ; } ; struct Anonymous0 abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class ABC { struct {} def; } abc;";
            const char expected[] = "class ABC { struct Anonymous0 { } ; struct Anonymous0 def ; } ; ABC abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class { class {} def; } abc;";
            const char expected[] = "class { class { } def ; } abc ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class ABC { struct {} def; };";
            const char expected[] = "class ABC { struct Anonymous0 { } ; struct Anonymous0 def ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class ABC : public XYZ { struct {} def; };";
            const char expected[] = "class ABC : public XYZ { struct Anonymous0 { } ; struct Anonymous0 def ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class { int x; }; int y;";
            const char expected[] = "class { int x ; } ; int y ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class { int x; };";
            const char expected[] = "class { int x ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "class { };";
            const char expected[] = "class { } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void simplifyStructDecl4() {
        const char code[] = "class ABC {\n"
                            "    void foo() {\n"
                            "        union {\n"
                            "            int i;\n"
                            "            float f;\n"
                            "        };\n"
                            "        struct Fee { } fee;\n"
                            "    }\n"
                            "    union {\n"
                            "        long long ll;\n"
                            "        double d;\n"
                            "    };\n"
                            "} abc;\n";
        const char expected[] = "class ABC { "
                                "void foo ( ) { "
                                "union { int i ; float f ; } ; "
                                "struct Fee { } ; struct Fee fee ; "
                                "} "
                                "union { "
                                "long ll ; "
                                "double d ; "
                                "} ; "
                                "} ; ABC abc ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyStructDecl6() {
        ASSERT_EQUALS("struct A { "
                      "char integers [ X ] ; "
                      "} ; struct A arrays ; arrays = { { 0 } } ;",
                      tok("struct A {\n"
                          "    char integers[X];\n"
                          "} arrays = {{0}};"));
    }

    void simplifyStructDecl7() {
        ASSERT_EQUALS("struct Anonymous0 { char x ; } ; struct Anonymous0 a [ 2 ] ;",
                      tok("struct { char x; } a[2];"));
        ASSERT_EQUALS("struct Anonymous0 { char x ; } ; static struct Anonymous0 a [ 2 ] ;",
                      tok("static struct { char x; } a[2];"));
    }

    void simplifyStructDecl8() {
        ASSERT_EQUALS("enum A { x , y , z } ; enum A a ; a = x ;", tok("enum A { x, y, z } a(x);"));
        ASSERT_EQUALS("enum B { x , y , z } ; enum B b ; b = x ;", tok("enum B { x , y, z } b{x};"));
        ASSERT_EQUALS("struct C { int i ; } ; struct C c ; c = C { 0 } ;", tok("struct C { int i; } c{0};"));
        ASSERT_EQUALS("enum Anonymous0 { x , y , z } ; enum Anonymous0 d ; d = x ;", tok("enum { x, y, z } d(x);"));
        ASSERT_EQUALS("enum Anonymous0 { x , y , z } ; enum Anonymous0 e ; e = x ;", tok("enum { x, y, z } e{x};"));
        ASSERT_EQUALS("struct Anonymous0 { int i ; } ; struct Anonymous0 f ; f = Anonymous0 { 0 } ;", tok("struct { int i; } f{0};"));
        ASSERT_EQUALS("struct Anonymous0 { } ; struct Anonymous0 x ; x = { 0 } ;", tok("struct {} x = {0};"));
        ASSERT_EQUALS("enum G : short { x , y , z } ; enum G g ; g = x ;", tok("enum G : short { x, y, z } g(x);"));
        ASSERT_EQUALS("enum H : short { x , y , z } ; enum H h ; h = x ;", tok("enum H : short { x, y, z } h{x};"));
        ASSERT_EQUALS("enum class I : short { x , y , z } ; enum I i ; i = x ;", tok("enum class I : short { x, y, z } i(x);"));
        ASSERT_EQUALS("enum class J : short { x , y , z } ; enum J j ; j = x ;", tok("enum class J : short { x, y, z } j{x};"));
    }

    void simplifyStructDecl9() {
        const char *exp{};
        {
            const char code[] = "enum E : int;\n" // #12588
                                "void f() {}\n"
                                "namespace {\n"
                                "    struct S {\n"
                                "        explicit S(int i) : m(i) {}\n"
                                "        int m;\n"
                                "    };\n"
                                "}\n"
                                "void g() { S s(0); }\n";
            exp = "enum E : int ; "
                  "void f ( ) { } "
                  "namespace { "
                  "struct S { "
                  "explicit S ( int i ) : m ( i ) { } "
                  "int m ; "
                  "} ; "
                  "} "
                  "void g ( ) { S s ( 0 ) ; }";
            ASSERT_EQUALS(exp, tok(code));
        }

        {
            const char code[] = "struct S {\n" // 12595
                                "    explicit S() {\n"
                                "        e = E1;\n"
                                "    }\n"
                                "    enum { E1 } e = E1;\n"
                                "};\n";
            exp = "struct S { "
                  "explicit S ( ) { e = E1 ; } "
                  "enum Anonymous0 { E1 } ; "
                  "enum Anonymous0 e ; "
                  "e = E1 ; "
                  "} ;";
            ASSERT_EQUALS(exp, tok(code));
        }

        {
            const char code[] = "struct S {\n"
                                "    explicit S() {\n"
                                "        e = E1;\n"
                                "    }\n"
                                "    enum : std::uint8_t { E1 } e = E1;\n"
                                "};\n";
            exp = "struct S { "
                  "explicit S ( ) { e = E1 ; } "
                  "enum Anonymous0 : std :: uint8_t { E1 } ; "
                  "enum Anonymous0 e ; "
                  "e = E1 ; "
                  "} ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void removeUnwantedKeywords() {
        ASSERT_EQUALS("int var ;", tok("register int var ;"));
        ASSERT_EQUALS("short var ;", tok("register short int var ;"));
        ASSERT_EQUALS("int foo ( ) { }", tok("inline int foo ( ) { }"));
        ASSERT_EQUALS("int foo ( ) { }", tok("__inline int foo ( ) { }"));
        ASSERT_EQUALS("int foo ( ) { }", tok("__forceinline int foo ( ) { }"));
        ASSERT_EQUALS("constexpr int foo ( ) { }", tok("constexpr int foo() { }"));
        ASSERT_EQUALS("constexpr int foo ( ) { }", tok("consteval int foo() { }"));
        ASSERT_EQUALS("int x ; x = 0 ;", tok("constinit int x = 0;"));
        ASSERT_EQUALS("void f ( ) { int final [ 10 ] ; }", tok("void f() { int final[10]; }"));
        ASSERT_EQUALS("int * p ;", tok("int * __restrict p;", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("int * * p ;", tok("int * __restrict__ * p;", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("void foo ( float * a , float * b ) ;", tok("void foo(float * __restrict__ a, float * __restrict__ b);", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("int * p ;", tok("int * restrict p;", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("int * * p ;", tok("int * restrict * p;", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("void foo ( float * a , float * b ) ;", tok("void foo(float * restrict a, float * restrict b);", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("void foo ( int restrict ) ;", tok("void foo(int restrict);"));
        ASSERT_EQUALS("int * p ;", tok("typedef int * __restrict__ rint; rint p;", dinit(TokOptions, $.cpp = false)));

        // don't remove struct members:
        ASSERT_EQUALS("a = b . _inline ;", tok("a = b._inline;"));

        ASSERT_EQUALS("int i ; i = 0 ;", tok("auto int i = 0;", dinit(TokOptions, $.cpp = false)));
        ASSERT_EQUALS("auto i ; i = 0 ;", tok("auto i = 0;"));
    }

    void simplifyCallingConvention() {
        ASSERT_EQUALS("int f ( ) ;", tok("int __cdecl f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __stdcall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __fastcall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __clrcall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __thiscall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __syscall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __pascal f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __fortran f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __cdecl f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __stdcall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __fastcall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __clrcall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __thiscall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __syscall f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __pascal f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int __far __fortran f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("int WINAPI f();", dinit(TokOptions, $.type = Platform::Type::Win32A)));
        ASSERT_EQUALS("int f ( ) ;", tok("int APIENTRY f();", dinit(TokOptions, $.type = Platform::Type::Win32A)));
        ASSERT_EQUALS("int f ( ) ;", tok("int CALLBACK f();", dinit(TokOptions, $.type = Platform::Type::Win32A)));

        // don't simplify Microsoft defines in unix code (#7554)
        ASSERT_EQUALS("enum E { CALLBACK } ;", tok("enum E { CALLBACK } ;", dinit(TokOptions, $.type = Platform::Type::Unix32)));
    }

    void simplifyAttribute() {
        ASSERT_EQUALS("int f ( ) ;", tok("__attribute__ ((visibility(\"default\"))) int f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("__attribute__((visibility(\"default\"))) int f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("__attribute ((visibility(\"default\"))) int f();"));
        ASSERT_EQUALS("int f ( ) ;", tok("__attribute__ ((visibility(\"default\"))) __attribute__ ((warn_unused_result)) int f();"));
        ASSERT_EQUALS("blah :: blah f ( ) ;", tok("__attribute__ ((visibility(\"default\"))) blah::blah f();"));
        ASSERT_EQUALS("template < T > Result < T > f ( ) ;", tok("template<T> __attribute__ ((warn_unused_result)) Result<T> f();"));
        ASSERT_EQUALS("template < T , U > Result < T , U > f ( ) ;", tok("template<T, U> __attribute__ ((warn_unused_result)) Result<T, U> f();"));
        ASSERT_EQUALS("void ( * fp ) ( ) ; fp = nullptr ;", tok("typedef void (*fp_t)() __attribute__((noreturn)); fp_t fp = nullptr;")); // #12137
    }

    void simplifyFunctorCall() {
        ASSERT_EQUALS("IncrementFunctor ( ) ( a ) ;", tok("IncrementFunctor()(a);"));
    }

    // #ticket #5339 (simplify function pointer after comma)
    void simplifyFunctionPointer() {
        ASSERT_EQUALS("f ( double x , double ( * y ) ( ) ) ;", tok("f (double x, double (*y) ());"));
    }

    void simplifyFunctionReturn() {
        {
            const char code[] = "typedef void (*testfp)();\n"
                                "struct Fred\n"
                                "{\n"
                                "    testfp get1() { return 0; }\n"
                                "    void ( * get2 ( ) ) ( ) { return 0 ; }\n"
                                "    testfp get3();\n"
                                "    void ( * get4 ( ) ) ( );\n"
                                "};";
            const char expected[] = "struct Fred "
                                    "{ "
                                    "void * get1 ( ) { return 0 ; } "
                                    "void * get2 ( ) { return 0 ; } "
                                    "void * get3 ( ) ; "
                                    "void * get4 ( ) ; "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "class Fred {\n"
                                "    std::string s;\n"
                                "    const std::string & foo();\n"
                                "};\n"
                                "const std::string & Fred::foo() { return \"\"; }";
            const char expected[] = "class Fred { "
                                    "std :: string s ; "
                                    "const std :: string & foo ( ) ; "
                                    "} ; "
                                    "const std :: string & Fred :: foo ( ) { return \"\" ; }";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            // Ticket #7916
            // Tokenization would include "int fact < 2 > ( ) { return 2 > ( ) ; }" and generate
            // a parse error (and use after free)
            const char code[] = "extern \"C\" void abort ();\n"
                                "template <int a> inline int fact2 ();\n"
                                "template <int a> inline int fact () {\n"
                                "  return a * fact2<a-1> ();\n"
                                "}\n"
                                "template <> inline int fact<1> () {\n"
                                "  return 1;\n"
                                "}\n"
                                "template <int a> inline int fact2 () {\n"
                                "  return a * fact<a-1>();\n"
                                "}\n"
                                "template <> inline int fact2<1> () {\n"
                                "  return 1;\n"
                                "}\n"
                                "int main() {\n"
                                "  fact2<3> ();\n"
                                "  fact2<2> ();\n"
                                "}";
            (void)tok(code);
        }
    }

    void consecutiveBraces() {
        ASSERT_EQUALS("void f ( ) { }", tok("void f(){{}}"));
        ASSERT_EQUALS("void f ( ) { }", tok("void f(){{{}}}"));
        ASSERT_EQUALS("void f ( ) { for ( ; ; ) { } }", tok("void f () { for(;;){} }"));
        ASSERT_EQUALS("void f ( ) { { scope_lock lock ; foo ( ) ; } { scope_lock lock ; bar ( ) ; } }", tok("void f () { {scope_lock lock; foo();} {scope_lock lock; bar();} }"));
    }

    void simplifyOverride() { // ticket #5069
        const char code[] = "void fun() {\n"
                            "    unsigned char override[] = {0x01, 0x02};\n"
                            "    doSomething(override, sizeof(override));\n"
                            "}\n";
        ASSERT_EQUALS("void fun ( ) { char override [ 2 ] = { 0x01 , 0x02 } ; doSomething ( override , sizeof ( override ) ) ; }",
                      tok(code));
    }

    void simplifyNestedNamespace() {
        ASSERT_EQUALS("namespace A { namespace B { namespace C { int i ; } } }", tok("namespace A::B::C { int i; }"));
    }

    void simplifyNamespaceAliases1() {
        ASSERT_EQUALS(";",
                      tok("namespace ios = boost::iostreams;"));
        ASSERT_EQUALS("boost :: iostreams :: istream foo ( \"foo\" ) ;",
                      tok("namespace ios = boost::iostreams; ios::istream foo(\"foo\");"));
        ASSERT_EQUALS("boost :: iostreams :: istream foo ( \"foo\" ) ;",
                      tok("using namespace std; namespace ios = boost::iostreams; ios::istream foo(\"foo\");"));
        ASSERT_EQUALS(";",
                      tok("using namespace std; namespace ios = boost::iostreams;"));
        ASSERT_EQUALS("namespace NS { boost :: iostreams :: istream foo ( \"foo\" ) ; }",
                      tok("namespace NS { using namespace std; namespace ios = boost::iostreams; ios::istream foo(\"foo\"); }"));

        // duplicate namespace aliases
        ASSERT_EQUALS(";",
                      tok("namespace ios = boost::iostreams;\nnamespace ios = boost::iostreams;"));
        ASSERT_EQUALS(";",
                      tok("namespace ios = boost::iostreams;\nnamespace ios = boost::iostreams;\nnamespace ios = boost::iostreams;"));
        ASSERT_EQUALS("namespace A { namespace B { void foo ( ) { bar ( A :: B :: ab ( ) ) ; } } }",
                      tok("namespace A::B {"
                          "namespace AB = A::B;"
                          "void foo() {"
                          "    namespace AB = A::B;" // duplicate declaration
                          "    bar(AB::ab());"
                          "}"
                          "namespace AB = A::B;" //duplicate declaration
                          "}"));
        ASSERT_EQUALS(";",
                      tok("namespace p = boost::python;\n"
                          "namespace np = boost::numpy;\n"));

        // redeclared nested namespace aliases
        TODO_ASSERT_EQUALS("namespace A { namespace B { void foo ( ) { bar ( A :: B :: ab ( ) ) ; { baz ( A :: a ( ) ) ; } bar ( A :: B :: ab ( ) ) ; } } }",
                           "namespace A { namespace B { void foo ( ) { bar ( A :: B :: ab ( ) ) ; { baz ( A :: B :: a ( ) ) ; } bar ( A :: B :: ab ( ) ) ; } } }",
                           tok("namespace A::B {"
                               "namespace AB = A::B;"
                               "void foo() {"
                               "    namespace AB = A::B;" // duplicate declaration
                               "    bar(AB::ab());"
                               "    {"
                               "         namespace AB = A;"
                               "         baz(AB::a());" // redeclaration OK
                               "    }"
                               "    bar(AB::ab());"
                               "}"
                               "namespace AB = A::B;" //duplicate declaration
                               "}"));

        // variable and namespace alias with same name
        ASSERT_EQUALS("namespace external { namespace ns { "
                      "class A { "
                      "public: "
                      "static void f ( const std :: string & json ) ; "
                      "} ; "
                      "} } "
                      "namespace external { namespace ns { "
                      "void A :: f ( const std :: string & json ) { } "
                      "} }",
                      tok("namespace external::ns {"
                          "    class A {"
                          "    public:"
                          "        static void f(const std::string& json);"
                          "    };"
                          "}"
                          "namespace json = rapidjson;"
                          "namespace external::ns {"
                          "    void A::f(const std::string& json) { }"
                          "}"));
    }

    void simplifyNamespaceAliases2() {
        ASSERT_EQUALS("void foo ( ) "
                      "{ "
                      "int maxResults ; maxResults = :: a :: b :: c :: d :: ef :: MAX ; "
                      "}",
                      tok("namespace ef = ::a::b::c::d::ef;"
                          "void foo()"
                          "{"
                          "  int maxResults = ::a::b::c::d::ef::MAX;"
                          "}"));
    }

    void simplifyNamespaceAliases3() {
        ASSERT_EQUALS("namespace N { namespace O { int g ( ) ; } } " // #12586
                      "void f ( ) { "
                      "int s ; s = 3 * :: N :: O :: g ( ) ; "
                      "}",
                      tok("namespace N { namespace O { int g(); } }\n"
                          "namespace _n = ::N::O;\n"
                          "void f() {\n"
                          "    int s = 3 * ::_n::g();\n"
                          "}\n"));
    }

#define simplifyKnownVariables(...) simplifyKnownVariables_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string simplifyKnownVariables_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer tokenizer(settings0, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        return tokenizer.tokens()->stringifyList(nullptr, false);
    }

    void simplifyKnownVariables2() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 10;\n"
                            "    a = g();\n"
                            "    if (a);\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 10 ; a = g ( ) ; if ( a ) { ; } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables3() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    while(true){\n"
                            "    break;\n"
                            "    a = 10;\n"
                            "    }\n"
                            "    if (a);\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 4 ; while ( true ) { break ; a = 10 ; } if ( a ) { ; } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables4() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    if ( g(a));\n"
                            "}\n";

        // TODO: if a is passed by value is is ok to simplify..
        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 4 ; if ( g ( a ) ) { ; } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables5() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int a = 4;\n"
                            "    if ( a = 5 );\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int a ; a = 4 ; if ( a = 5 ) { ; } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables13() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    int i = 10;\n"
                            "    while(--i) {}\n"
                            "}\n";

        ASSERT_EQUALS(
            "void f ( ) { int i ; i = 10 ; while ( -- i ) { } }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables14() {
        // ticket #753
        const char code[] = "void f ( ) { int n ; n = 1 ; do { ++ n ; } while ( n < 10 ) ; }";
        ASSERT_EQUALS(code, simplifyKnownVariables(code));
    }

    void simplifyKnownVariables16() {
        // ticket #807 - segmentation fault when macro isn't found
        const char code[] = "void f ( ) { int n = 1; DISPATCH(while); }";
        ASSERT_THROW_INTERNAL(simplifyKnownVariables(code), UNKNOWN_MACRO);
    }

    void simplifyKnownVariables17() {
        // ticket #807 - segmentation fault when macro isn't found
        const char code[] = "void f ( ) { char *s = malloc(100);mp_ptr p = s; p++; }";
        ASSERT_EQUALS(
            "void f ( ) { char * s ; s = malloc ( 100 ) ; mp_ptr p ; p = s ; p ++ ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables18() {
        const char code[] = "void f ( ) { char *s = malloc(100);mp_ptr p = s; ++p; }";
        ASSERT_EQUALS(
            "void f ( ) { char * s ; s = malloc ( 100 ) ; mp_ptr p ; p = s ; ++ p ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables19() {
        const char code[] = "void f ( ) { int i=0; do { if (i>0) { a(); } i=b(); } while (i != 12); }";
        ASSERT_EQUALS(
            "void f ( ) { int i ; i = 0 ; do { if ( i > 0 ) { a ( ) ; } i = b ( ) ; } while ( i != 12 ) ; }",
            simplifyKnownVariables(code));
    }

    void simplifyKnownVariables21() {
        ASSERT_EQUALS(
            "void foo ( int i ) { int n ; n = i ; for ( i = 0 ; i < n ; ++ i ) { } }",
            simplifyKnownVariables("void foo(int i) { int n = i; for (i = 0; i < n; ++i) { } }"));
    }

    void simplifyKnownVariables25() {
        {
            // This testcase is related to ticket #1646
            const char code[] = "void foo(char *str)\n"
                                "{\n"
                                "    int i;\n"
                                "    for (i=0;i<10;++i) {\n"
                                "        if (*str == 0) goto label;\n"
                                "    }\n"
                                "    return;\n"
                                "label:\n"
                                "    str[i] = 0;\n"
                                "}\n";

            // Current result
            ASSERT_EQUALS(
                "void foo ( char * str ) "
                "{"
                " int i ;"
                " for ( i = 0 ; i < 10 ; ++ i ) {"
                " if ( * str == 0 ) { goto label ; }"
                " }"
                " return ;"
                " label : ;"
                " str [ i ] = 0 ; "
                "}",
                simplifyKnownVariables(code));
        }

        {
            // This testcase is related to ticket #1646
            const char code[] = "void foo(char *str)\n"
                                "{\n"
                                "    int i;\n"
                                "    for (i=0;i<10;++i) { }\n"
                                "    return;\n"
                                "    str[i] = 0;\n"
                                "}\n";

            // Current result
            ASSERT_EQUALS(
                "void foo ( char * str ) "
                "{"
                " int i ;"
                " for ( i = 0 ; i < 10 ; ++ i ) { }"
                " return ;"
                " str [ i ] = 0 ; "
                "}",
                simplifyKnownVariables(code));
        }
    }

    // cppcheck-suppress unusedPrivateFunction
    void simplifyKnownVariables29() { // ticket #1811
        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h + i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 + v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h - i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 - v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h * i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 * v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h / i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 / v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h & i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 & v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h | i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 | v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h ^ i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 ^ v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h % i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 % v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h >> i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 >> v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "int foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h << i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: int foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 << v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h == i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 == v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h != i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 != v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h > i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 > v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h >= i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 >= v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h < i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 < v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h <= i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 <= v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h && i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 && v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }

        {
            const char code[] = "bool foo(int u, int v)\n"
                                "{\n"
                                "  int h = u;\n"
                                "  int i = v;\n"
                                "  return h || i;\n"
                                "}\n";
            const char expected[] = "\n\n"
                                    "##file 0\n"
                                    "1: bool foo ( int u@1 , int v@2 )\n"
                                    "2: {\n"
                                    "3:\n"
                                    "4:\n"
                                    "5: return u@1 || v@2 ;\n"
                                    "6: }\n";
            ASSERT_EQUALS(expected, tokenizeDebugListing(code));
        }
    }

    void simplifyKnownVariables30() {
        const char code[] = "int foo() {\n"
                            "  iterator it1 = ints.begin();\n"
                            "  iterator it2 = it1;\n"
                            "  for (++it2;it2!=ints.end();++it2);\n"
                            "}\n";
        const char expected[] = "int foo ( ) {\n"
                                "iterator it1 ; it1 = ints . begin ( ) ;\n"
                                "iterator it2 ; it2 = it1 ;\n"
                                "for ( ++ it2 ; it2 != ints . end ( ) ; ++ it2 ) { ; }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void simplifyKnownVariables34() {
        const char code[] = "void f() {\n"
                            "    int x = 10;\n"
                            "    do { cin >> x; } while (x > 5);\n"
                            "    a[x] = 0;\n"
                            "}\n";
        const char expected[] = "void f ( ) {\n"
                                "int x ; x = 10 ;\n"
                                "do { cin >> x ; } while ( x > 5 ) ;\n"
                                "a [ x ] = 0 ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:10]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable cin [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void simplifyKnownVariables36() {

        // Ticket #5972
        const char code2[] = "void f() {"
                             "  char buf[10] = \"ab\";"
                             "    memset(buf, 0, 10);"
                             "}";
        const char expected2[] = "void f ( ) { char buf [ 10 ] = \"ab\" ; memset ( buf , 0 , 10 ) ; }";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2));
    }

    void simplifyKnownVariables42() {
        {
            const char code[] = "void f() {\n"
                                "   char a[10];\n"
                                "   strcpy(a, \"hello\");\n"
                                "   strcat(a, \"!\");\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "char a [ 10 ] ;\n"
                                    "strcpy ( a , \"hello\" ) ;\n"
                                    "strcat ( a , \"!\" ) ;\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeAndStringifyOptions, $.cpp = false)));
        }

        {
            const char code[] = "void f() {"
                                "    char *s = malloc(10);"
                                "    strcpy(s, \"\");"
                                "    free(s);"
                                "}";
            const char expected[] = "void f ( ) {"
                                    " char * s ; s = malloc ( 10 ) ;"
                                    " strcpy ( s , \"\" ) ;"
                                    " free ( s ) ; "
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }

        {
            const char code[] = "void f(char *p, char *q) {"
                                "    strcpy(p, \"abc\");"
                                "    q = p;"
                                "}";
            const char expected[] = "void f ( char * p , char * q ) {"
                                    " strcpy ( p , \"abc\" ) ;"
                                    " q = p ; "
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }

        // 3538
        {
            const char code[] = "void f() {\n"
                                "    char s[10];\n"
                                "    strcpy(s, \"123\");\n"
                                "    if (s[6] == ' ');\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "char s [ 10 ] ;\n"
                                    "strcpy ( s , \"123\" ) ;\n"
                                    "if ( s [ 6 ] == ' ' ) { ; }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
            ASSERT_EQUALS("", filter_valueflow(errout_str()));
        }
    }

    void simplifyKnownVariables43() {
        {
            const char code[] = "void f() {\n"
                                "    int a, *p; p = &a;\n"
                                "    { int a = *p; }\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "int a ; int * p ; p = & a ;\n"
                                    "{ int a ; a = * p ; }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }

        {
            const char code[] = "void f() {\n"
                                "    int *a, **p; p = &a;\n"
                                "    { int *a = *p; }\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "int * a ; int * * p ; p = & a ;\n"
                                    "{ int * a ; a = * p ; }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
    }

    void simplifyKnownVariables44() {
        const char code[] = "void a() {\n"
                            "    static int i = 10;\n"
                            "    b(i++);\n"
                            "}";
        const char expected[] = "void a ( ) {\n"
                                "static int i = 10 ;\n"
                                "b ( i ++ ) ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void simplifyKnownVariables46() {
        const char code[] = "void f() {\n"
                            "    int x = 0;\n"
                            "    cin >> x;\n"
                            "    return x;\n"
                            "}";

        const char expected[] = "void f ( ) {\n"
                                "int x ; x = 0 ;\n"
                                "cin >> x ;\n"
                                "return x ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:5]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable cin [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void simplifyKnownVariables47() {
        // #3621
        const char code[] = "void f() {\n"
                            "    int x = 0;\n"
                            "    cin >> std::hex >> x;\n"
                            "}";
        const char expected[] = "void f ( ) {\n"
                                "int x ; x = 0 ;\n"
                                "cin >> std :: hex >> x ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:5]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable cin [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void simplifyKnownVariables48() {
        // #3754
        const char code[] = "void f(int sz) {\n"
                            "    int i;\n"
                            "    for (i = 0; ((i<sz) && (sz>3)); ++i) { }\n"
                            "}";
        const char expected[] = "void f ( int sz ) {\n"
                                "int i ;\n"
                                "for ( i = 0 ; ( i < sz ) && ( sz > 3 ) ; ++ i ) { }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeAndStringifyOptions, $.cpp = false)));
        ignore_errout();
    }

    void simplifyKnownVariables49() { // #3691
        const char code[] = "void f(int sz) {\n"
                            "    switch (x) {\n"
                            "    case 1: sz = 2; continue;\n"
                            "    case 2: x = sz; break;\n"
                            "    }\n"
                            "}";
        const char expected[] = "void f ( int sz ) {\n"
                                "switch ( x ) {\n"
                                "case 1 : ; sz = 2 ; continue ;\n"
                                "case 2 : ; x = sz ; break ;\n"
                                "}\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeAndStringifyOptions, $.cpp = false)));
        ASSERT_EQUALS(
            "[test.c:2:13]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable x [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void simplifyKnownVariables50() { // #4066
        {
            //don't simplify '&x'!
            const char code[] = "const char * foo ( ) {\n"
                                "const char x1 = 'b' ;\n"
                                "f ( & x1 ) ;\n"
                                "const char x2 = 'b' ;\n"
                                "f ( y , & x2 ) ;\n"
                                "const char x3 = 'b' ;\n"
                                "t = & x3 ;\n"
                                "const char x4 = 'b' ;\n"
                                "t = y + & x4 ;\n"
                                "const char x5 = 'b' ;\n"
                                "z [ & x5 ] = y ;\n"
                                "const char x6 = 'b' ;\n"
                                "v = { & x6 } ;\n"
                                "const char x7 = 'b' ;\n"
                                "return & x7 ;\n"
                                "}";
            ASSERT_EQUALS(code, tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:5:5]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable y [valueFlowBailoutIncompleteVar]\n",
                errout_str());
        }
        {
            //don't simplify '&x'!
            const char code[] = "const int * foo ( ) {\n"
                                "const int x1 = 1 ;\n"
                                "f ( & x1 ) ;\n"
                                "const int x2 = 1 ;\n"
                                "f ( y , & x2 ) ;\n"
                                "const int x3 = 1 ;\n"
                                "t = & x3 ;\n"
                                "const int x4 = 1 ;\n"
                                "t = y + & x4 ;\n"
                                "const int x5 = 1 ;\n"
                                "z [ & x5 ] = y ;\n"
                                "const int x6 = 1 ;\n"
                                "v = { & x6 } ;\n"
                                "const int x7 = 1 ;\n"
                                "return & x7 ;\n"
                                "}";
            ASSERT_EQUALS(code, tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:5:5]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable y [valueFlowBailoutIncompleteVar]\n",
                errout_str());
        }
    }

    void simplifyKnownVariables51() { // #4409 hang
        const char code[] = "void mhz_M(int enough) {\n"
                            "  TYPE *x=&x, **p=x, **q = NULL;\n"
                            "  BENCH1(q = _mhz_M(n); n = 1;)\n"
                            "  use_pointer(q);\n"
                            "}";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code), UNKNOWN_MACRO);
    }

    void simplifyKnownVariables54() { // #4913
        ASSERT_EQUALS("void f ( int * p ) { * -- p = 0 ; * p = 0 ; }", tokenizeAndStringify("void f(int*p) { *--p=0; *p=0; }"));
    }

    void simplifyKnownVariables56() { // ticket #5301 - >>
        ASSERT_EQUALS("void f ( ) { int a ; a = 0 ; int b ; b = 0 ; * p >> a >> b ; return a / b ; }",
                      tokenizeAndStringify("void f() { int a=0,b=0; *p>>a>>b; return a/b; }"));
        ASSERT_EQUALS(
            "[test.cpp:1:26]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable p [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void simplifyKnownVariables58() { // #5268
        const char code[] = "enum e { VAL1 = 1, VAL2 }; "
                            "typedef char arr_t[VAL2]; "
                            "int foo(int) ; "
                            "void bar () { "
                            "  throw foo (VAL1); "
                            "} "
                            "int baz() { "
                            "  return sizeof(arr_t); "
                            "}";
        ASSERT_EQUALS("enum e { VAL1 = 1 , VAL2 } ; "
                      "int foo ( int ) ; "
                      "void bar ( ) { "
                      "throw foo ( VAL1 ) ; "
                      "} "
                      "int baz ( ) { "
                      "return sizeof ( char [ VAL2 ] ) ; "
                      "}", tokenizeAndStringify(code));
    }

    void simplifyKnownVariables59() { // #5062 - for head
        const char code[] = "void f() {\n"
                            "  int a[3], i, j;\n"
                            "  for(i = 0, j = 1; i < 3, j < 12; i++,j++) {\n"
                            "    a[i] = 0;\n"
                            "  }\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "int a [ 3 ] ; int i ; int j ;\n"
                      "for ( i = 0 , j = 1 ; i < 3 , j < 12 ; i ++ , j ++ ) {\n"
                      "a [ i ] = 0 ;\n"
                      "}\n"
                      "}", tokenizeAndStringify(code));
    }

    void simplifyKnownVariables61() { // #7805
        ASSERT_NO_THROW(tokenizeAndStringify("static const int XX = 0;\n"
                                             "enum E { XX };\n"
                                             "struct s {\n"
                                             "  enum Bar {\n"
                                             "    XX,\n"
                                             "    Other\n"
                                             "  };\n"
                                             "  enum { XX };\n"
                                             "};"));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyKnownVariables62() { // #5666
        ASSERT_EQUALS("void foo ( std :: string str ) {\n"
                      "char * p ; p = & str [ 0 ] ;\n"
                      "* p = 0 ;\n"
                      "}",
                      tokenizeAndStringify("void foo(std::string str) {\n"
                                           "  char *p = &str[0];\n"
                                           "  *p = 0;\n"
                                           "}"));
    }

    void simplifyKnownVariables63() { // #10798
        ASSERT_NO_THROW(tokenizeAndStringify("typedef void (*a)();\n"
                                             "enum class E { a };\n"));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyKnownVariablesBailOutAssign1() {
        const char code[] = "int foo() {\n"
                            "    int i; i = 0;\n"
                            "    if (x) { i = 10; }\n"
                            "    return i;\n"
                            "}\n";
        const char expected[] = "int foo ( ) {\n"
                                "int i ; i = 0 ;\n"
                                "if ( x ) { i = 10 ; }\n"
                                "return i ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:9]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable x [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str()));
    }

    void simplifyKnownVariablesBailOutAssign2() {
        // ticket #3032 - assignment in condition
        const char code[] = "void f(struct ABC *list) {\n"
                            "    struct ABC *last = NULL;\n"
                            "    nr = (last = list->prev)->nr;\n"  // <- don't replace "last" with 0
                            "}\n";
        const char expected[] = "void f ( struct ABC * list ) {\n"
                                "struct ABC * last ; last = NULL ;\n"
                                "nr = ( last = list . prev ) . nr ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:5]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable nr [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void simplifyKnownVariablesBailOutFor1() {
        const char code[] = "void foo() {\n"
                            "    for (int i = 0; i < 10; ++i) { }\n"
                            "}\n";
        const char expected[] = "void foo ( ) {\n"
                                "for ( int i = 0 ; i < 10 ; ++ i ) { }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());    // debug warnings
    }

    void simplifyKnownVariablesBailOutFor2() {
        const char code[] = "void foo() {\n"
                            "    int i = 0;\n"
                            "    while (i < 10) { ++i; }\n"
                            "}\n";
        const char expected[] = "void foo ( ) {\n"
                                "int i ; i = 0 ;\n"
                                "while ( i < 10 ) { ++ i ; }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());    // debug warnings
    }

    void simplifyKnownVariablesBailOutFor3() {
        const char code[] = "void foo() {\n"
                            "    for (std::string::size_type pos = 0; pos < 10; ++pos)\n"
                            "    { }\n"
                            "}\n";
        const char expected[] = "void foo ( ) {\n"
                                "for ( std :: string :: size_type pos = 0 ; pos < 10 ; ++ pos )\n"
                                "{ }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());    // debug warnings
    }

    void simplifyKnownVariablesBailOutMemberFunction() {
        const char code[] = "void foo(obj a) {\n"
                            "    obj b = a;\n"
                            "    b.f();\n"
                            "}\n";
        const char expected[] = "void foo ( obj a ) {\n"
                                "obj b ; b = a ;\n"
                                "b . f ( ) ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void simplifyKnownVariablesBailOutConditionalIncrement() {
        const char code[] = "int f() {\n"
                            "    int a = 0;\n"
                            "    if (x) {\n"
                            "        ++a;\n" // conditional increment
                            "    }\n"
                            "    return a;\n"
                            "}\n";
        (void)tokenizeAndStringify(code);
        ASSERT_EQUALS(
            "[test.cpp:3:9]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable x [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str())); // no debug warnings
    }

    void simplifyKnownVariablesBailOutSwitchBreak() {
        // Ticket #2324
        const char code[] = "int f(char *x) {\n"
                            "    char *p;\n"
                            "    char *q;\n"
                            "\n"
                            "    switch (x & 0x3)\n"
                            "    {\n"
                            "        case 1:\n"
                            "            p = x;\n"
                            "            x = p;\n"
                            "            break;\n"
                            "        case 2:\n"
                            "            q = x;\n" // x is not equal with p
                            "            x = q;\n"
                            "            break;\n"
                            "    }\n"
                            "}\n";

        const char expected[] = "int f ( char * x ) {\n"
                                "char * p ;\n"
                                "char * q ;\n"
                                "\n"
                                "switch ( x & 0x3 )\n"
                                "{\n"
                                "case 1 : ;\n"
                                "p = x ;\n"
                                "x = p ;\n"
                                "break ;\n"
                                "case 2 : ;\n"
                                "q = x ;\n"
                                "x = q ;\n"
                                "break ;\n"
                                "}\n"
                                "}";

        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void simplifyKnownVariablesFunctionCalls() {
        {
            const char code[] = "void a(int &x);" // <- x is passed by reference
                                "void b() {"
                                "    int x = 123;"
                                "    a(x);"       // <- don't replace with a(123);
                                "}";
            const char expected[] = "void a ( int & x ) ; void b ( ) { int x ; x = 123 ; a ( x ) ; }";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
    }

    void simplifyKnownVariablesGlobalVars() {
        // #8054
        const char code[] = "static int x;"
                            "void f() {"
                            "    x = 123;"
                            "    while (!x) { dostuff(); }"
                            "}";
        ASSERT_EQUALS("static int x ; void f ( ) { x = 123 ; while ( ! x ) { dostuff ( ) ; } }", tokenizeAndStringify(code));
        ASSERT_EQUALS("", filter_valueflow(errout_str()));
    }

    void simplifyKnownVariablesNamespace() {
        { // #10059
            const char code[] = "namespace N {\n"
                                "    const int n = 0;\n"
                                "    namespace M { const int m = 0; }\n"
                                "}\n"
                                "using namespace N;\n"
                                "int i(n);\n"
                                "int j(M::m);\n"
                                "using namespace N::M;\n"
                                "int k(m);\n"
                                "int l(N::M::m);\n";
            const char exp[]  = "\n\n##file 0\n"
                                "1: namespace N {\n"
                                "2: const int n@1 = 0 ;\n"
                                "3: namespace M { const int m@2 = 0 ; }\n"
                                "4: }\n"
                                "5: using namespace N ;\n"
                                "6: int i ( n@1 ) ;\n"
                                "7: int j ( M :: m@2 ) ;\n"
                                "8: using namespace N :: M ;\n"
                                "9: int k ( m@2 ) ;\n"
                                "10: int l ( N :: M :: m@2 ) ;\n";
            ASSERT_EQUALS(exp, tokenizeDebugListing(code));
        }
        { // #10835
            const char code[] = "using namespace X;\n"
                                "namespace N {\n"
                                "    struct A {\n"
                                "        static int i;\n"
                                "        struct B {\n"
                                "            double x;\n"
                                "            void f();\n"
                                "        };\n"
                                "    };\n"
                                "}\n"
                                "namespace N {\n"
                                "    int A::i = 0;\n"
                                "    void A::B::f() {\n"
                                "        x = 0;\n"
                                "    }\n"
                                "}\n";
            const char exp[]  = "\n\n##file 0\n"
                                "1: using namespace X ;\n"
                                "2: namespace N {\n"
                                "3: struct A {\n"
                                "4: static int i@1 ;\n"
                                "5: struct B {\n"
                                "6: double x@2 ;\n"
                                "7: void f ( ) ;\n"
                                "8: } ;\n"
                                "9: } ;\n"
                                "10: }\n"
                                "11: namespace N {\n"
                                "12: int A :: i@1 = 0 ;\n"
                                "13: void A :: B :: f ( ) {\n"
                                "14: x@2 = 0 ;\n"
                                "15: }\n"
                                "16: }\n";
            ASSERT_EQUALS(exp, tokenizeDebugListing(code));
        }
    }

    void simplifyKnownVariablesClassMember() {
        // Ticket #2815
        {
            const char code[] = "char *a;\n"
                                "void f(const char *s) {\n"
                                "    a = NULL;\n"
                                "    x();\n"
                                "    memcpy(a, s, 10);\n"   // <- don't simplify "a" here
                                "}\n";

            const std::string s(tokenizeAndStringify(code));
            ASSERT_EQUALS(true, s.find("memcpy ( a , s , 10 ) ;") != std::string::npos);
        }

        // If the variable is local then perform simplification..
        {
            const char code[] = "void f(const char *s) {\n"
                                "    char *a = NULL;\n"
                                "    x();\n"
                                "    memcpy(a, s, 10);\n"   // <- simplify "a"
                                "}\n";

            const std::string s(tokenizeAndStringify(code));
            TODO_ASSERT_EQUALS(true, false, s.find("memcpy ( 0 , s , 10 ) ;") != std::string::npos);
        }
    }

    void simplify_constants6() { // Ticket #5625
        {
            const char code[] = "template < class T > struct foo ;\n"
                                "void bar ( ) {\n"
                                "foo < 1 ? 0 ? 1 : 6 : 2 > x ;\n"
                                "foo < 1 ? 0 : 2 > y ;\n"
                                "}";
            const char exp[] = "template < class T > struct foo ; "
                               "void bar ( ) { "
                               "foo < 6 > x ; "
                               "foo < 0 > y ; "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "bool b = true ? false : 1 > 2 ;";
            const char exp[] = "bool b ; b = true ? false : 1 > 2 ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void simplifyVarDeclInitLists()
    {
        {
            const char code[] = "std::vector<int> v{a * b, 1};";
            const char exp[] = "std :: vector < int > v { a * b , 1 } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "enum E { E0 } e{};";
            const char exp[] = "enum E { E0 } ; enum E e ; e = { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }
};

REGISTER_TEST(TestSimplifyTokens)
