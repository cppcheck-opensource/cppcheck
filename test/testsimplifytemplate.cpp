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

#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"
#include "standards.h"
#include "templatesimplifier.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <cstring>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class ErrorLogger;

class TestSimplifyTemplate : public TestFixture {
public:
    TestSimplifyTemplate() : TestFixture("TestSimplifyTemplate") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::portability).build();
    const Settings settings1 = settingsBuilder(settings).library("std.cfg").build();
    const Settings settings1_d = settingsBuilder(settings1).debugwarnings().build();

    void run() override {
        TEST_CASE(template1);
        TEST_CASE(template2);
        TEST_CASE(template3);
        TEST_CASE(template4);
        TEST_CASE(template5);
        TEST_CASE(template6);
        TEST_CASE(template7);
        TEST_CASE(template8);
        TEST_CASE(template9);
        TEST_CASE(template10);
        TEST_CASE(template11);
        TEST_CASE(template12);
        TEST_CASE(template13);
        TEST_CASE(template14);
        TEST_CASE(template15);  // recursive templates
        TEST_CASE(template16);
        TEST_CASE(template17);
        TEST_CASE(template18);
        TEST_CASE(template19);
        TEST_CASE(template20);
        TEST_CASE(template21);
        TEST_CASE(template22);
        TEST_CASE(template23);
        TEST_CASE(template24);  // #2648 - using sizeof in template parameter
        TEST_CASE(template25);  // #2648 - another test for sizeof template parameter
        TEST_CASE(template26);  // #2721 - passing 'char[2]' as template parameter
        TEST_CASE(template27);  // #3350 - removing unused template in macro call
        TEST_CASE(template28);
        TEST_CASE(template30);  // #3529 - template < template < ..
        TEST_CASE(template31);  // #4010 - reference type
        TEST_CASE(template32);  // #3818 - mismatching template not handled well
        TEST_CASE(template33);  // #3818,#4544 - inner templates in template instantiation not handled well
        TEST_CASE(template34);  // #3706 - namespace => hang
        TEST_CASE(template35);  // #4074 - A<'x'> a;
        TEST_CASE(template36);  // #4310 - passing unknown template instantiation as template argument
        TEST_CASE(template37);  // #4544 - A<class B> a;
        TEST_CASE(template38);  // #4832 - crash on C++11 right angle brackets
        TEST_CASE(template39);  // #4742 - freeze
        TEST_CASE(template40);  // #5055 - template specialization outside struct
        TEST_CASE(template41);  // #4710 - const in instantiation not handled perfectly
        TEST_CASE(template42);  // #4878 - variadic templates
        TEST_CASE(template43);  // #5097 - assert due to '>>' not treated as end of template instantiation
        TEST_CASE(template44);  // #5297 - TemplateSimplifier::simplifyCalculations not eager enough
        TEST_CASE(template45);  // #5814 - syntax error reported for valid code
        TEST_CASE(template46);  // #5816 - syntax error reported for valid code
        TEST_CASE(template47);  // #6023 - syntax error reported for valid code
        TEST_CASE(template48);  // #6134 - 100% CPU upon invalid code
        TEST_CASE(template49);  // #6237 - template instantiation
        TEST_CASE(template50);  // #4272 - simple partial specialization
        TEST_CASE(template52);  // #6437 - crash upon valid code
        TEST_CASE(template53);  // #4335 - bail out for valid code
        TEST_CASE(template54);  // #6587 - memory corruption upon valid code
        TEST_CASE(template55);  // #6604 - simplify "const const" to "const" in template instantiations
        TEST_CASE(template56);  // #7117 - const ternary operator simplification as template parameter
        TEST_CASE(template57);  // #7891
        TEST_CASE(template58);  // #6021 - use after free (deleted tokens in simplifyCalculations)
        TEST_CASE(template59);  // #8051 - TemplateSimplifier::simplifyTemplateInstantiation failure
        TEST_CASE(template60);  // handling of methods outside template definition
        TEST_CASE(template61);  // daca2, kodi
        TEST_CASE(template62);  // #8314 - inner template instantiation
        TEST_CASE(template63);  // #8576 - qualified type
        TEST_CASE(template64);  // #8683
        TEST_CASE(template65);  // #8321
        TEST_CASE(template66);  // #8725
        TEST_CASE(template67);  // #8122
        TEST_CASE(template68);  // union
        TEST_CASE(template69);  // #8791
        TEST_CASE(template70);  // #5289
        TEST_CASE(template71);  // #8821
        TEST_CASE(template72);
        TEST_CASE(template73);
        TEST_CASE(template74);
        TEST_CASE(template75);
        TEST_CASE(template76);
        TEST_CASE(template77);
        TEST_CASE(template78);
        TEST_CASE(template79); // #5133
        TEST_CASE(template80);
        TEST_CASE(template81);
        TEST_CASE(template82); // #8603
        TEST_CASE(template83); // #8867
        TEST_CASE(template84); // #8880
        TEST_CASE(template85); // #8902 crash
        TEST_CASE(template86); // crash
        TEST_CASE(template87);
        TEST_CASE(template88); // #6183
        TEST_CASE(template89); // #8917
        TEST_CASE(template90); // crash
        TEST_CASE(template91);
        TEST_CASE(template92);
        TEST_CASE(template93); // crash
        TEST_CASE(template94); // #8927 crash
        TEST_CASE(template95); // #7417
        TEST_CASE(template96); // #7854
        TEST_CASE(template97);
        TEST_CASE(template98); // #8959
        TEST_CASE(template99); // #8960
        TEST_CASE(template100); // #8967
        TEST_CASE(template101); // #8968
        TEST_CASE(template102); // #9005
        TEST_CASE(template103);
        TEST_CASE(template104); // #9021
        TEST_CASE(template105); // #9076
        TEST_CASE(template106);
        TEST_CASE(template107); // #8663
        TEST_CASE(template108); // #9109
        TEST_CASE(template109); // #9144
        TEST_CASE(template110);
        TEST_CASE(template111); // crash
        TEST_CASE(template112); // #9146 syntax error
        TEST_CASE(template113);
        TEST_CASE(template114); // #9155
        TEST_CASE(template115); // #9153
        TEST_CASE(template116); // #9178
        TEST_CASE(template117);
        TEST_CASE(template118);
        TEST_CASE(template119); // #9186
        TEST_CASE(template120);
        TEST_CASE(template121); // #9193
        TEST_CASE(template122); // #9147
        TEST_CASE(template123); // #9183
        TEST_CASE(template124); // #9197
        TEST_CASE(template125);
        TEST_CASE(template126); // #9217
        TEST_CASE(template127); // #9225
        TEST_CASE(template128); // #9224
        TEST_CASE(template129);
        TEST_CASE(template130); // #9246
        TEST_CASE(template131); // #9249
        TEST_CASE(template132); // #9250
        TEST_CASE(template133);
        TEST_CASE(template134);
        TEST_CASE(template135);
        TEST_CASE(template136); // #9287
        TEST_CASE(template137); // #9288
        TEST_CASE(template138);
        TEST_CASE(template139);
        TEST_CASE(template140);
        TEST_CASE(template141); // #9337
        TEST_CASE(template142); // #9338
        TEST_CASE(template143);
        TEST_CASE(template144); // #9046
        TEST_CASE(template145); // syntax error
        TEST_CASE(template146); // syntax error
        TEST_CASE(template147); // syntax error
        TEST_CASE(template148); // syntax error
        TEST_CASE(template149); // unknown macro
        TEST_CASE(template150); // syntax error
        TEST_CASE(template151); // crash
        TEST_CASE(template152); // #9467
        TEST_CASE(template153); // #9483
        TEST_CASE(template154); // #9495
        TEST_CASE(template155); // #9539
        TEST_CASE(template156);
        TEST_CASE(template157); // #9854
        TEST_CASE(template158); // daca crash
        TEST_CASE(template159); // #9886
        TEST_CASE(template160);
        TEST_CASE(template161);
        TEST_CASE(template162);
        TEST_CASE(template163); // #9685 syntax error
        TEST_CASE(template164); // #9394
        TEST_CASE(template165); // #10032 syntax error
        TEST_CASE(template166); // #10081 hang
        TEST_CASE(template167);
        TEST_CASE(template168);
        TEST_CASE(template169);
        TEST_CASE(template170); // crash
        TEST_CASE(template171); // crash
        TEST_CASE(template172); // #10258 crash
        TEST_CASE(template173); // #10332 crash
        TEST_CASE(template174); // #10506 hang
        TEST_CASE(template175); // #10908
        TEST_CASE(template176); // #11146
        TEST_CASE(template177);
        TEST_CASE(template178);
        TEST_CASE(template179);
        TEST_CASE(template180);
        TEST_CASE(template181);
        TEST_CASE(template182); // #13770
        TEST_CASE(template183);
        TEST_CASE(template184);
        TEST_CASE(template_specialization_1);  // #7868 - template specialization template <typename T> struct S<C<T>> {..};
        TEST_CASE(template_specialization_2);  // #7868 - template specialization template <typename T> struct S<C<T>> {..};
        TEST_CASE(template_specialization_3);
        TEST_CASE(template_enum);  // #6299 Syntax error in complex enum declaration (including template)
        TEST_CASE(template_unhandled);
        TEST_CASE(template_default_parameter);
        TEST_CASE(template_forward_declared_default_parameter);
        TEST_CASE(template_default_type);
        TEST_CASE(template_typename);
        TEST_CASE(template_constructor);    // #3152 - template constructor is removed
        TEST_CASE(syntax_error_templates_1);
        TEST_CASE(template_member_ptr); // Ticket #5786 - crash upon valid code
        TEST_CASE(template_namespace_1);
        TEST_CASE(template_namespace_2);
        TEST_CASE(template_namespace_3);
        TEST_CASE(template_namespace_4);
        TEST_CASE(template_namespace_5);
        TEST_CASE(template_namespace_6);
        TEST_CASE(template_namespace_7); // #8768
        TEST_CASE(template_namespace_8);
        TEST_CASE(template_namespace_9);
        TEST_CASE(template_namespace_10);
        TEST_CASE(template_namespace_11); // #7145
        TEST_CASE(template_namespace_12);
        TEST_CASE(template_namespace_13);
        TEST_CASE(template_pointer_type);
        TEST_CASE(template_array_type);

        // Test TemplateSimplifier::templateParameters
        TEST_CASE(templateParameters);

        TEST_CASE(templateNamePosition);

        TEST_CASE(findTemplateDeclarationEnd);

        TEST_CASE(getTemplateParametersInDeclaration);

        TEST_CASE(expandSpecialized1);
        TEST_CASE(expandSpecialized2);
        TEST_CASE(expandSpecialized3); // #8671
        TEST_CASE(expandSpecialized4);
        TEST_CASE(expandSpecialized5); // #10494

        TEST_CASE(templateAlias1);
        TEST_CASE(templateAlias2);
        TEST_CASE(templateAlias3); // #8315
        TEST_CASE(templateAlias4); // #9070
        TEST_CASE(templateAlias5);

        // Test TemplateSimplifier::instantiateMatch
        TEST_CASE(instantiateMatchTest);
        TEST_CASE(templateParameterWithoutName); // #8602 Template default parameter without name yields syntax error

        TEST_CASE(templateTypeDeduction1); // #8962
        TEST_CASE(templateTypeDeduction2);
        TEST_CASE(templateTypeDeduction3);
        TEST_CASE(templateTypeDeduction4); // #9983
        TEST_CASE(templateTypeDeduction5);

        TEST_CASE(simplifyTemplateArgs1);
        TEST_CASE(simplifyTemplateArgs2);
        TEST_CASE(simplifyTemplateArgs3);

        TEST_CASE(template_variadic_1); // #9144
        TEST_CASE(template_variadic_2); // #4349
        TEST_CASE(template_variadic_3); // #6172
        TEST_CASE(template_variadic_4);

        TEST_CASE(template_variable_1);
        TEST_CASE(template_variable_2);
        TEST_CASE(template_variable_3);
        TEST_CASE(template_variable_4);

        TEST_CASE(simplifyDecltype);

        TEST_CASE(castInExpansion);

        TEST_CASE(fold_expression_1);
        TEST_CASE(fold_expression_2);
        TEST_CASE(fold_expression_3);
        TEST_CASE(fold_expression_4);

        TEST_CASE(concepts1);
        TEST_CASE(requires1);
        TEST_CASE(requires2);
        TEST_CASE(requires3);
        TEST_CASE(requires4);
        TEST_CASE(requires5);

        TEST_CASE(explicitBool1);
        TEST_CASE(explicitBool2);

        TEST_CASE(templateArgPreserveType); // #13882 - type of template argument

        TEST_CASE(dumpTemplateArgFrom);
    }

    class TemplateSimplifierTest final : public TemplateSimplifier
    {
        friend class TestSimplifyTemplate;
    };

    class TokenizerTest final : public Tokenizer
    {
        friend class TestSimplifyTemplate;
    public:
        TokenizerTest(TokenList tokenList, ErrorLogger &errorLogger)
            : Tokenizer(std::move(tokenList), errorLogger)
        {}
    };

    struct CheckOptions
    {
        bool debugwarnings = false;
    };

#define tok(...) tok_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tok_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings& s = options.debugwarnings ? settings1_d : settings1;
        SimpleTokenizer tokenizer(s, *this);

        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        return tokenizer.tokens()->stringifyList(nullptr, true);
    }

#define dump(...) dump_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string dump_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings& s = options.debugwarnings ? settings1_d : settings1;
        SimpleTokenizer tokenizer(s, *this);

        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        std::ostringstream ostr;
        (tokenizer.dump)(ostr);

        return ostr.str();
    }

    void template1() {
        const char code[] = "template <class T> T f(T val) { T a; }\n"
                            "f<int>(10);";

        const char expected[] = "int f<int> ( int val ) ; "
                                "f<int> ( 10 ) ; "
                                "int f<int> ( int val ) { int a ; }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template2() {
        const char code[] = "template <class T> class Fred { T a; };\n"
                            "Fred<int> fred;";

        const char expected[] = "class Fred<int> ; "
                                "Fred<int> fred ; "
                                "class Fred<int> { int a ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template3() {
        const char code[] = "template <class T, int sz> class Fred { T data[sz]; };\n"
                            "Fred<float,4> fred;";

        const char expected[] = "class Fred<float,4> ; "
                                "Fred<float,4> fred ; "
                                "class Fred<float,4> { float data [ 4 ] ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template4() {
        const char code[] = "template <class T> class Fred { Fred(); };\n"
                            "Fred<float> fred;";

        const char expected[] = "class Fred<float> ; "
                                "Fred<float> fred ; "
                                "class Fred<float> { Fred<float> ( ) ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template5() {
        const char code[] = "template <class T> class Fred { };\n"
                            "template <class T> Fred<T>::Fred() { }\n"
                            "Fred<float> fred;";

        const char expected[] = "class Fred<float> ; "
                                "Fred<float> fred ; "
                                "class Fred<float> { } ; "
                                "Fred<float> :: Fred<float> ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template6() {
        const char code[] = "template <class T> class Fred { };\n"
                            "Fred<float> fred1;\n"
                            "Fred<float> fred2;";

        const char expected[] = "class Fred<float> ; "
                                "Fred<float> fred1 ; "
                                "Fred<float> fred2 ; "
                                "class Fred<float> { } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template7() {
        // A template class that is not used => no simplification
        {
            const char code[] = "template <class T>\n"
                                "class ABC\n"
                                "{\n"
                                "public:\n"
                                "    typedef ABC<T> m;\n"
                                "};\n";

            const char expected[] = "template < class T > class ABC { public: } ;";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <typename T> class ABC {\n"
                                "public:\n"
                                "    typedef std::vector<T> type;\n"
                                "};\n"
                                "int main() {\n"
                                "    ABC<int>::type v;\n"
                                "    v.push_back(4);\n"
                                "    return 0;\n"
                                "}\n";

            const char wanted[] = "class ABC<int> ; "
                                  "int main ( ) { "
                                  "std :: vector < int > v ; "
                                  "v . push_back ( 4 ) ; "
                                  "return 0 ; "
                                  "} "
                                  "class ABC<int> { public: } ;";

            const char current[] = "class ABC<int> ; "
                                   "int main ( ) { "
                                   "ABC<int> :: type v ; "
                                   "v . push_back ( 4 ) ; "
                                   "return 0 ; "
                                   "} "
                                   "class ABC<int> { public: } ;";

            TODO_ASSERT_EQUALS(wanted, current, tok(code));
        }

        {
            const char code[] = "template <typename T> class ABC {\n"
                                "public:\n"
                                "    typedef std::vector<T> type;\n"
                                "    void f()\n"
                                "    {\n"
                                "      ABC<int>::type v;\n"
                                "      v.push_back(4);\n"
                                "    }\n"
                                "};\n";

            const char expected[] = "template < typename T > class ABC { "
                                    "public: void f ( ) { "
                                    "ABC < int > :: type v ; "
                                    "v . push_back ( 4 ) ; "
                                    "} "
                                    "} ;";

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    // Template definitions but no usage => no expansion
    void template8() {
        const char code[] = "template<typename T> class A;\n"
                            "template<typename T> class B;\n"
                            "\n"
                            "typedef A<int> x;\n"
                            "typedef B<int> y;\n"
                            "\n"
                            "template<typename T> class A {\n"
                            "    void f() {\n"
                            "        B<T> a = B<T>::g();\n"
                            "        T b = 0;\n"
                            "        if (b)\n"
                            "            b = 0;\n"
                            "    }\n"
                            "};\n"
                            "\n"
                            "template<typename T> inline B<T> h() { return B<T>(); }\n";

        ASSERT_EQUALS("template < typename T > class A ; "
                      "template < typename T > class B ; "
                      "template < typename T > class A { void f ( ) { B < T > a ; a = B < T > :: g ( ) ; T b ; b = 0 ; if ( b ) { b = 0 ; } } } ; "
                      "template < typename T > B < T > h ( ) { return B < T > ( ) ; }", tok(code));

        ASSERT_EQUALS("class A { template < typename T > int foo ( T d ) ; } ;", tok("class A{ template<typename T> int foo(T d);};"));
    }

    void template9() {
        const char code[] = "template < typename T > class A { } ;\n"
                            "\n"
                            "void f ( ) {\n"
                            "    A < int > a ;\n"
                            "}\n"
                            "\n"
                            "template < typename T >\n"
                            "class B {\n"
                            "    void g ( ) {\n"
                            "        A < T > b = A < T > :: h ( ) ;\n"
                            "    }\n"
                            "} ;\n";

        // The expected result..
        const char expected[] = "class A<int> ; "
                                "void f ( ) { A<int> a ; } "
                                "template < typename T > class B { void g ( ) { A < T > b ; b = A < T > :: h ( ) ; } } ; "
                                "class A<int> { } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template10() {
        const char code[] = "template <int ui, typename T> T * foo()\n"
                            "{ return new T[ui]; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    foo<3,int>();\n"
                            "}\n";

        // The expected result..
        const char expected[] = "int * foo<3,int> ( ) ; "
                                "void f ( ) "
                                "{"
                                " foo<3,int> ( ) ; "
                                "} "
                                "int * foo<3,int> ( ) { return new int [ 3 ] ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template11() {
        const char code[] = "template <int ui, typename T> T * foo()\n"
                            "{ return new T[ui]; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    char * p = foo<3,char>();\n"
                            "}\n";

        // The expected result..
        const char expected[] = "char * foo<3,char> ( ) ; "
                                "void f ( ) "
                                "{"
                                " char * p ; p = foo<3,char> ( ) ; "
                                "} "
                                "char * foo<3,char> ( ) { return new char [ 3 ] ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template12() {
        const char code[] = "template <int x, int y, int z>\n"
                            "class A : public B<x, y, (x - y) ? ((y < z) ? 1 : -1) : 0>\n"
                            "{ };\n"
                            "\n"
                            "void f()\n"
                            "{\n"
                            "    A<12,12,11> a;\n"
                            "}\n";
        const char expected[] = "class A<12,12,11> ; "
                                "void f ( ) "
                                "{"
                                " A<12,12,11> a ; "
                                "} "
                                "class A<12,12,11> : public B < 12 , 12 , 0 > "
                                "{ } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template13() {
        const char code[] = "class BB {};\n"
                            "\n"
                            "template <class T>\n"
                            "class AA {\n"
                            "public:\n"
                            "    static AA<T> create(T* newObject);\n"
                            "    static int size();\n"
                            "};\n"
                            "\n"
                            "class CC { public: CC(AA<BB>, int) {} };\n"
                            "\n"
                            "class XX {\n"
                            "    AA<CC> y;\n"
                            "public:\n"
                            "    XX();\n"
                            "};\n"
                            "\n"
                            "XX::XX():\n"
                            "    y(AA<CC>::create(new CC(AA<BB>(), 0)))\n"
                            "    {}\n"
                            "\n"
                            "int yy[AA<CC>::size()];";
        const char expected[] = "class BB { } ; "
                                "class AA<BB> ; "
                                "class AA<CC> ; "
                                "class CC { public: CC ( AA<BB> , int ) { } } ; "
                                "class XX { "
                                "AA<CC> y ; "
                                "public: "
                                "XX ( ) ; "
                                "} ; "
                                "XX :: XX ( ) : "
                                "y ( AA<CC> :: create ( new CC ( AA<BB> ( ) , 0 ) ) ) "
                                "{ } "
                                "int yy [ AA<CC> :: size ( ) ] ; "
                                "class AA<BB> { "
                                "public: "
                                "static AA<BB> create ( BB * newObject ) ; "
                                "static int size ( ) ; "
                                "} ; "
                                "class AA<CC> { "
                                "public: "
                                "static AA<CC> create ( CC * newObject ) ; "
                                "static int size ( ) ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template14() {
        const char code[] = "template <> void foo<int *>()\n"
                            "{ x(); }\n"
                            "\n"
                            "int main()\n"
                            "{\n"
                            "foo<int*>();\n"
                            "}\n";
        const char expected[] = "void foo<int*> ( ) ; "
                                "void foo<int*> ( ) "
                                "{ x ( ) ; } "
                                "int main ( ) "
                                "{ foo<int*> ( ) ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template15() { // recursive templates  #3130 etc
        const char code[] = "template <unsigned int i> void a()\n"
                            "{\n"
                            "    a<i-1>();\n"
                            "}\n"
                            "\n"
                            "template <> void a<0>()\n"
                            "{ }\n"
                            "\n"
                            "int main()\n"
                            "{\n"
                            "    a<2>();\n"
                            "    return 0;\n"
                            "}\n";

        // The expected result..
        const char expected[] = "void a<0> ( ) ; "
                                "void a<2> ( ) ; "
                                "void a<1> ( ) ; "
                                "void a<0> ( ) { } "
                                "int main ( ) "
                                "{ a<2> ( ) ; return 0 ; } "
                                "void a<2> ( ) { a<1> ( ) ; } "
                                "void a<1> ( ) { a<0> ( ) ; }";

        ASSERT_EQUALS(expected, tok(code));

        // #3130
        const char code2[] = "template <int n> struct vec {\n"
                             "  vec() {}\n"
                             "  vec(const vec<n-1>& v) {}\n" // <- never used don't instantiate
                             "};\n"
                             "\n"
                             "vec<4> v;";
        const char expected2[] = "struct vec<4> ; "
                                 "vec<4> v ; "
                                 "struct vec<4> { "
                                 "vec<4> ( ) { } "
                                 "vec<4> ( const vec < ( int ) 4 - 1 > & v ) { } "
                                 "} ;";

        ASSERT_EQUALS(expected2, tok(code2));
    }

    void template16() {
        const char code[] = "template <unsigned int i> void a()\n"
                            "{ }\n"
                            "\n"
                            "template <unsigned int i> void b()\n"
                            "{ a<i>(); }\n"
                            "\n"
                            "int main()\n"
                            "{\n"
                            "    b<2>();\n"
                            "    return 0;\n"
                            "}\n";

        const char expected[] = "void a<2> ( ) ; "
                                "void b<2> ( ) ; "
                                "int main ( ) { b<2> ( ) ; return 0 ; } "
                                "void b<2> ( ) { a<2> ( ) ; } "
                                "void a<2> ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template17() {
        const char code[] = "template<class T>\n"
                            "class Fred\n"
                            "{\n"
                            "    template<class T>\n"
                            "    static shared_ptr< Fred<T> > CreateFred()\n"
                            "    {\n"
                            "    }\n"
                            "};\n"
                            "\n"
                            "shared_ptr<int> i;\n";
        const char expected[] = "template < class T > "
                                "class Fred "
                                "{ "
                                "template < class T > "
                                "static shared_ptr < Fred < T > > CreateFred ( ) "
                                "{ "
                                "} "
                                "} ; "
                                "shared_ptr < int > i ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template18() {
        const char code[] = "template <class T> class foo { T a; };\n"
                            "foo<int> *f;";

        const char expected[] = "class foo<int> ; "
                                "foo<int> * f ; "
                                "class foo<int> { int a ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template19() {
        const char code[] = "template <typename T> T & foo()\n"
                            "{ static T temp; return temp; }\n"
                            "\n"
                            "void f ( )\n"
                            "{\n"
                            "    char p = foo<char>();\n"
                            "}\n";

        // The expected result..
        const char expected[] = "char & foo<char> ( ) ; "
                                "void f ( ) "
                                "{"
                                " char p ; p = foo<char> ( ) ; "
                                "} "
                                "char & foo<char> ( ) { static char temp ; return temp ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template20() {
        // Ticket #1788 - the destructor implementation is lost
        const char code[] = "template <class T> class A { public:  ~A(); };\n"
                            "template <class T> A<T>::~A() {}\n"
                            "A<int> a;\n";

        // The expected result..
        const char expected[] = "class A<int> ; "
                                "A<int> a ; "
                                "class A<int> { public: ~ A<int> ( ) ; } ; "
                                "A<int> :: ~ A<int> ( ) { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template21() {
        {
            const char code[] = "template <class T> struct Fred { T a; };\n"
                                "Fred<int> fred;";

            const char expected[] = "struct Fred<int> ; "
                                    "Fred<int> fred ; "
                                    "struct Fred<int> { int a ; } ;";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <class T, int sz> struct Fred { T data[sz]; };\n"
                                "Fred<float,4> fred;";

            const char expected[] = "struct Fred<float,4> ; "
                                    "Fred<float,4> fred ; "
                                    "struct Fred<float,4> { float data [ 4 ] ; } ;";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <class T> struct Fred { Fred(); };\n"
                                "Fred<float> fred;";

            const char expected[] = "struct Fred<float> ; "
                                    "Fred<float> fred ; "
                                    "struct Fred<float> { Fred<float> ( ) ; } ;";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "template <class T> struct Fred { };\n"
                                "Fred<float> fred1;\n"
                                "Fred<float> fred2;";

            const char expected[] = "struct Fred<float> ; "
                                    "Fred<float> fred1 ; "
                                    "Fred<float> fred2 ; "
                                    "struct Fred<float> { } ;";

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void template22() {
        const char code[] = "template <class T> struct Fred { T a; };\n"
                            "Fred<std::string> fred;";

        const char expected[] = "struct Fred<std::string> ; "
                                "Fred<std::string> fred ; "
                                "struct Fred<std::string> { std :: string a ; } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template23() {
        const char code[] = "template <class T> void foo() { }\n"
                            "void bar() {\n"
                            "    std::cout << (foo<double>());\n"
                            "}";

        const char expected[] = "void foo<double> ( ) ; "
                                "void bar ( ) {"
                                " std :: cout << ( foo<double> ( ) ) ; "
                                "} "
                                "void foo<double> ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template24() {
        // #2648
        const char code[] = "template<int n> struct B\n"
                            "{\n"
                            "  int a[n];\n"
                            "};\n"
                            "\n"
                            "template<int x> class bitset: B<sizeof(int)>\n"
                            "{};\n"
                            "\n"
                            "bitset<1> z;";
        const char expected[] = "struct B<4> ; "
                                "class bitset<1> ; "
                                "bitset<1> z ; "
                                "class bitset<1> : B<4> { } ; "
                                "struct B<4> { int a [ 4 ] ; } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template25() {
        const char code[] = "template<int n> struct B\n"
                            "{\n"
                            "  int a[n];\n"
                            "};\n"
                            "\n"
                            "template<int x> class bitset: B<((sizeof(int)) ? : 1)>\n"
                            "{};\n"
                            "\n"
                            "bitset<1> z;";
        const char expected[] = "struct B<4> ; "
                                "class bitset<1> ; "
                                "bitset<1> z ; "
                                "class bitset<1> : B<4> { } ; "
                                "struct B<4> { int a [ 4 ] ; } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template26() {
        // #2721
        const char code[] = "template<class T>\n"
                            "class A { public: T x; };\n"
                            "\n"
                            "template<class M>\n"
                            "class C: public A<char[M]> {};\n"
                            "\n"
                            "C<2> a;\n";
        ASSERT_EQUALS("class A<char[2]> ; class C<2> ; C<2> a ; class C<2> : public A<char[2]> { } ; class A<char[2]> { public: char [ 2 ] x ; } ;", tok(code));
    }

    void template27() {
        // #3350 - template inside macro call
        const char code[] = "X(template<class T> class Fred);";
        ASSERT_THROW_INTERNAL(tok(code), SYNTAX);
    }

    void template28() {
        // #3226 - inner template
        const char code[] = "template<class A, class B> class Fred {};\n"
                            "Fred<int,Fred<int,int> > x;\n";
        ASSERT_EQUALS("class Fred<int,int> ; "
                      "class Fred<int,Fred<int,int>> ; "
                      "Fred<int,Fred<int,int>> x ; "
                      "class Fred<int,int> { } ; "
                      "class Fred<int,Fred<int,int>> { } ;", tok(code));
    }

    void template30() {
        // #3529 - template < template < ..
        const char code[] = "template<template<class> class A, class B> void f(){}";
        ASSERT_EQUALS("template < template < class > class A , class B > void f ( ) { }", tok(code));
    }

    void template31() {
        // #4010 - template reference type
        const char code[] = "template<class T> struct A{}; A<int&> a;";
        ASSERT_EQUALS("struct A<int&> ; "
                      "A<int&> a ; "
                      "struct A<int&> { } ;", tok(code));

        // #7409 - rvalue
        const char code2[] = "template<class T> struct A{}; A<int&&> a;";
        ASSERT_EQUALS("struct A<int&&> ; "
                      "A<int&&> a ; "
                      "struct A<int&&> { } ;", tok(code2));
    }

    void template32() {
        // #3818 - mismatching template not handled well
        const char code[] = "template <class T1, class T2, class T3, class T4 > struct A { };\n"
                            "\n"
                            "template <class T>\n"
                            "struct B\n"
                            "{\n"
                            "    public:\n"
                            "        A < int, Pair<T, int>, int > a;\n"  // mismatching parameters => don't instantiate
                            "};\n"
                            "\n"
                            "B<int> b;\n";
        ASSERT_EQUALS("template < class T1 , class T2 , class T3 , class T4 > struct A { } ; "
                      "struct B<int> ; "
                      "B<int> b ; "
                      "struct B<int> { public: A < int , Pair < int , int > , int > a ; } ;", tok(code));
    }

    void template33() {
        {
            // #3818 - inner templates in template instantiation not handled well
            const char code[] = "template<class T> struct A { };\n"
                                "template<class T> struct B { };\n"
                                "template<class T> struct C { A<B<X<T> > > ab; };\n"
                                "C<int> c;";
            ASSERT_EQUALS("struct A<B<X<int>>> ; "
                          "struct B<X<int>> ; "
                          "struct C<int> ; "
                          "C<int> c ; "
                          "struct C<int> { A<B<X<int>>> ab ; } ; "
                          "struct B<X<int>> { } ; "  // <- redundant.. but nevermind
                          "struct A<B<X<int>>> { } ;", tok(code));
        }

        {
            // #4544
            const char code[] = "struct A { };\n"
                                "template<class T> struct B { };\n"
                                "template<class T> struct C { };\n"
                                "C< B<A> > c;";
            ASSERT_EQUALS("struct A { } ; "
                          "template < class T > struct B { } ; "  // <- redundant.. but nevermind
                          "struct C<B<A>> ; "
                          "C<B<A>> c ; "
                          "struct C<B<A>> { } ;",
                          tok(code));
        }
    }

    void template34() {
        // #3706 - namespace => hang
        const char code[] = "namespace abc {\n"
                            "template <typename T> struct X { void f(X<T> &x) {} };\n"
                            "}\n"
                            "template <> int X<int>::Y(0);";
        (void)tok(code);
    }

    void template35() { // #4074 - "A<'x'> a;" is not recognized as template instantiation
        const char code[] = "template <char c> class A {};\n"
                            "A <'x'> a;";
        ASSERT_EQUALS("class A<'x'> ; "
                      "A<'x'> a ; "
                      "class A<'x'> { } ;", tok(code));
    }

    void template36() { // #4310 - Passing unknown template instantiation as template argument
        const char code[] = "template <class T> struct X { T t; };\n"
                            "template <class C> struct Y { Foo < X< Bar<C> > > _foo; };\n" // <- Bar is unknown
                            "Y<int> bar;";
        ASSERT_EQUALS("struct X<Bar<int>> ; "
                      "struct Y<int> ; "
                      "Y<int> bar ; "
                      "struct Y<int> { Foo < X<Bar<int>> > _foo ; } ; "
                      "struct X<Bar<int>> { Bar < int > t ; } ;",
                      tok(code));
    }

    void template37() { // #4544 - A<class B> a;
        {
            const char code[] = "class A { };\n"
                                "template<class T> class B {};\n"
                                "B<class A> b1;\n"
                                "B<A> b2;";
            ASSERT_EQUALS("class A { } ; class B<A> ; B<A> b1 ; B<A> b2 ; class B<A> { } ;",
                          tok(code));
        }
        {
            const char code[] = "struct A { };\n"
                                "template<class T> class B {};\n"
                                "B<struct A> b1;\n"
                                "B<A> b2;";
            ASSERT_EQUALS("struct A { } ; class B<A> ; B<A> b1 ; B<A> b2 ; class B<A> { } ;",
                          tok(code));
        }
        {
            const char code[] = "enum A { };\n"
                                "template<class T> class B {};\n"
                                "B<enum A> b1;\n"
                                "B<A> b2;";
            ASSERT_EQUALS("enum A { } ; class B<A> ; B<A> b1 ; B<A> b2 ; class B<A> { } ;",
                          tok(code));
        }
    }

    void template_unhandled() {
        // An unhandled template usage should not be simplified..
        ASSERT_EQUALS("x < int > ( ) ;", tok("x<int>();"));
    }

    void template38() { // #4832 - Crash on C++11 right angle brackets
        const char code[] = "template <class T> class A {\n"
                            "  T mT;\n"
                            "public:\n"
                            "  void foo() {}\n"
                            "};\n"
                            "\n"
                            "int main() {\n"
                            "    A<A<BLA>>   gna1;\n"
                            "    A<BLA>      gna2;\n"
                            "}\n";
        const char expected[] = "class A<BLA> ; "
                                "class A<A<BLA>> ; "
                                "int main ( ) { "
                                "A<A<BLA>> gna1 ; "
                                "A<BLA> gna2 ; "
                                "} "
                                "class A<BLA> { "
                                "BLA mT ; "
                                "public: "
                                "void foo ( ) { } "
                                "} ; "
                                "class A<A<BLA>> { "
                                "A<BLA> mT ; "
                                "public: "
                                "void foo ( ) { } "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template39() { // #4742 - Used to freeze in 1.60
        const char code[] = "template<typename T> struct vector {"
                            "  operator T() const;"
                            "};"
                            "void f() {"
                            "  vector<vector<int>> v;"
                            "  const vector<int> vi = static_cast<vector<int>>(v);"
                            "}";
        (void)tok(code);
    }

    void template40() { // #5055 - false negatives when there is template specialization outside struct
        const char code[] = "struct A {"
                            "  template<typename T> struct X { T t; };"
                            "};"
                            "template<> struct A::X<int> { int *t; };";
        const char expected[] = "struct A { "
                                "struct X<int> ; "
                                "template < typename T > struct X { T t ; } ; "
                                "} ; "
                                "struct A :: X<int> { int * t ; } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template41() { // #4710 - const in template instantiation not handled perfectly
        const char code1[] = "template<class T> struct X { };\n"
                             "void f(const X<int> x) { }";
        ASSERT_EQUALS("struct X<int> ; "
                      "void f ( const X<int> x ) { } "
                      "struct X<int> { } ;", tok(code1));

        const char code2[] = "template<class T> T f(T t) { return t; }\n"
                             "int x() { return f<int>(123); }";
        ASSERT_EQUALS("int f<int> ( int t ) ; "
                      "int x ( ) { return f<int> ( 123 ) ; } "
                      "int f<int> ( int t ) { return t ; }", tok(code2));
    }

    void template42() { // #4878 cppcheck aborts in ext-blocks.cpp (clang testcode)
        const char code[] = "template<typename ...Args>\n"
                            "int f0(Args ...args) {\n"
                            "  return ^ {\n"
                            "    return sizeof...(Args);\n"
                            "  }() + ^ {\n"
                            "    return sizeof...(args);\n"
                            "  }();\n"
                            "}";
        ASSERT_THROW_INTERNAL(tok(code), SYNTAX);
    }

    void template43() { // #5097 - Assert due to '>>' in 'B<A<C>>' not being treated as end of template instantiation
        const char code[] = "template <typename T> struct E { typedef int Int; };\n"
                            "template <typename T> struct C { };\n"
                            "template <typename T> struct D { static int f() { return C<T>::f(); } };\n"
                            "template <typename T> inline int f2() { return D<T>::f(); }\n"
                            "template <typename T> int f1 (int x, T *) { int id = f2<T>(); return id; }\n"
                            "template <typename T> struct B { void f3(B<T> & other) { } };\n"
                            "struct A { };\n"
                            "template <> struct C<B<A>> {\n"
                            "    static int f() { return f1<B<A>>(0, reinterpret_cast<B<A>*>(E<void*>::Int(-1))); }\n"
                            "};\n"
                            "int main(void) {\n"
                            "    C<A> ca;\n"
                            "    return 0;\n"
                            "}";
        const char expected[] = "struct E<void*> ; "
                                "struct C<B<A>> ; "
                                "struct C<A> ; "
                                "struct D<B<A>> ; "
                                "int f2<B<A>> ( ) ; "
                                "int f1<B<A>> ( int x , B<A> * ) ; "
                                "struct B<A> ; "
                                "struct A { } ; "
                                "struct C<B<A>> { "
                                "static int f ( ) { "
                                "return f1<B<A>> ( 0 , reinterpret_cast < B<A> * > ( E<void*> :: Int ( -1 ) ) ) ; "
                                "} "
                                "} ; "
                                "int main ( ) { "
                                "C<A> ca ; "
                                "return 0 ; "
                                "} "
                                "struct B<A> { "
                                "void f3 ( B<A> & other ) { } "
                                "} ; "
                                "int f1<B<A>> ( int x , B<A> * ) { "
                                "int id ; id = f2<B<A>> ( ) ; "
                                "return id ; "
                                "} "
                                "int f2<B<A>> ( ) { "
                                "return D<B<A>> :: f ( ) ; "
                                "} "
                                "struct D<B<A>> { "
                                "static int f ( ) { "
                                "return C<B<A>> :: f ( ) ; "
                                "} "
                                "} ; "
                                "struct C<A> { } ; struct E<void*> { "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template44() { // #5297
        const char code[] = "template<class T> struct StackContainer {"
                            "  void foo(int i) {"
                            "    if (0 >= 1 && i<0) {}"
                            "  }"
                            "};"
                            "template<class T> class ZContainer : public StackContainer<T> {};"
                            "struct FGSTensor {};"
                            "class FoldedZContainer : public ZContainer<FGSTensor> {};";
        const char expected[] = "struct StackContainer<FGSTensor> ; "
                                "class ZContainer<FGSTensor> ; "
                                "struct FGSTensor { } ; "
                                "class FoldedZContainer : public ZContainer<FGSTensor> { } ; "
                                "class ZContainer<FGSTensor> : public StackContainer<FGSTensor> { } ; "
                                "struct StackContainer<FGSTensor> { "
                                "void foo ( int i ) { "
                                "if ( 0 >= 1 && i < 0 ) { } "
                                "} "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template45() { // #5814
        const char code[] = "namespace Constants { const int fourtytwo = 42; } "
                            "template <class T, int U> struct TypeMath { "
                            "  static const int mult = sizeof(T) * U; "
                            "}; "
                            "template <class T> struct FOO { "
                            "  enum { value = TypeMath<T, Constants::fourtytwo>::mult }; "
                            "}; "
                            "FOO<int> foo;";
        const char expected[] = "namespace Constants { const int fourtytwo = 42 ; } "
                                "struct TypeMath<int,Constants::fourtytwo> ; "
                                "struct FOO<int> ; "
                                "FOO<int> foo ; "
                                "struct FOO<int> { "
                                "enum Anonymous0 { value = TypeMath<int,Constants::fourtytwo> :: mult } ; "
                                "} ; "
                                "struct TypeMath<int,Constants::fourtytwo> { "
                                "static const int mult = sizeof ( int ) * Constants :: fourtytwo ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code, dinit(CheckOptions, $.debugwarnings = true)));
        ASSERT_EQUALS("", errout_str());
    }

    void template46() { // #5816
        ASSERT_NO_THROW(tok("template<class T, class U> struct A { static const int value = 0; }; "
                            "template <class T> struct B { "
                            "  enum { value = A<typename T::type, int>::value }; "
                            "};"));
        ASSERT_EQUALS("", errout_str());
        ASSERT_NO_THROW(tok("template <class T, class U> struct A {}; "
                            "enum { e = sizeof(A<int, int>) }; "
                            "template <class T, class U> struct B {};"));
        ASSERT_EQUALS("", errout_str());
        ASSERT_NO_THROW(tok("template<class T, class U> struct A { static const int value = 0; }; "
                            "template<class T> struct B { typedef int type; }; "
                            "template <class T> struct C { "
                            "  enum { value = A<typename B<T>::type, int>::value }; "
                            "};"));
        ASSERT_EQUALS("", errout_str());
    }

    void template47() { // #6023
        ASSERT_NO_THROW(tok("template <typename T1, typename T2 = T3<T1> > class C1 {}; "
                            "class C2 : public C1<C2> {};"));
        ASSERT_EQUALS("", errout_str());
    }

    void template48() { // #6134 (hang)
        (void)tok("template <int> int f( {  } ); "
                  "int foo = f<1>(0);");
        ASSERT_EQUALS("", errout_str());
    }

    void template49() { // #6237
        const char code[] = "template <class T> class Fred { void f(); void g(); };\n"
                            "template <class T> void Fred<T>::f() { }\n"
                            "template <class T> void Fred<T>::g() { }\n"
                            "template void Fred<float>::f();\n"
                            "template void Fred<int>::g();\n";

        const char expected[] = "class Fred<float> ; "
                                "class Fred<int> ; "
                                "class Fred<float> { void f ( ) ; void g ( ) ; } ; "
                                "void Fred<float> :: f ( ) { } "
                                "void Fred<float> :: g ( ) { } "
                                "class Fred<int> { void f ( ) ; void g ( ) ; } ; "
                                "void Fred<int> :: f ( ) { } "
                                "void Fred<int> :: g ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template50() { // #4272
        const char code[] = "template <class T> class Fred { void f(); };\n"
                            "template <class T> void Fred<T>::f() { }\n"
                            "template<> void Fred<float>::f() { }\n"
                            "template<> void Fred<int>::f() { }\n";

        const char expected[] = "class Fred<float> ; "
                                "class Fred<int> ; "
                                "template < > void Fred<float> :: f ( ) { } "
                                "template < > void Fred<int> :: f ( ) { } "
                                "class Fred<float> { void f ( ) ; } ; "
                                "void Fred<float> :: f ( ) { } "
                                "class Fred<int> { void f ( ) ; } ; "
                                "void Fred<int> :: f ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template52() { // #6437
        const char code[] = "template <int value> int sum() { "
                            "  return value + sum<value/2>(); "
                            "} "
                            "template<int x, int y> int calculate_value() { "
                            "  if (x != y) { "
                            "    return sum<x - y>(); "
                            "  } else { "
                            "    return 0; "
                            "  } "
                            "} "
                            "int value = calculate_value<1,1>();";
        const char expected[] = "int sum<0> ( ) ; "
                                "int calculate_value<1,1> ( ) ; "
                                "int value ; value = calculate_value<1,1> ( ) ; "
                                "int calculate_value<1,1> ( ) { "
                                "if ( ( int ) 1 != ( int ) 1 ) { "
                                "return sum<0> ( ) ; "
                                "} else { "
                                "return 0 ; "
                                "} "
                                "} "
                                "int sum<0> ( ) { "
                                "return 0 + sum<0> ( ) ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template53() { // #4335
        const char code[] = "template<int N> struct Factorial { "
                            "  enum { value = N * Factorial<N - 1>::value }; "
                            "};"
                            "template <> struct Factorial<0> { "
                            "  enum { value = 1 }; "
                            "};"
                            "const int x = Factorial<4>::value;";
        const char expected[] = "struct Factorial<0> ; "
                                "struct Factorial<4> ; "
                                "struct Factorial<3> ; "
                                "struct Factorial<2> ; "
                                "struct Factorial<1> ; "
                                "struct Factorial<0> { "
                                "enum Anonymous1 { value = 1 } ; "
                                "} ; "
                                "const int x = Factorial<4> :: value ; "
                                "struct Factorial<4> { "
                                "enum Anonymous0 { value = ( int ) 4 * Factorial<3> :: value } ; "
                                "} ; "
                                "struct Factorial<3> { "
                                "enum Anonymous0 { value = ( int ) 3 * Factorial<2> :: value } ; "
                                "} ; "
                                "struct Factorial<2> { "
                                "enum Anonymous0 { value = ( int ) 2 * Factorial<1> :: value } ; "
                                "} ; "
                                "struct Factorial<1> { "
                                "enum Anonymous0 { value = ( int ) 1 * Factorial<0> :: value } ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code, dinit(CheckOptions, $.debugwarnings = true)));
        ASSERT_EQUALS("", errout_str());
    }

    void template54() { // #6587 (use-after-free)
        (void)tok("template<typename _Tp> _Tp* fn(); "
                  "template <class T> struct A { "
                  "  template <class U, class S = decltype(fn<T>())> "
                  "  struct B { }; "
                  "}; "
                  "A<int> a;");
    }

    void template55() { // #6604
        // Avoid constconstconst in macro instantiations
        ASSERT_EQUALS(
            "template < class T > class AtSmartPtr : public ConstCastHelper < AtSmartPtr < const T > , T > { "
            "friend struct ConstCastHelper < AtSmartPtr < const T > , T > ; "
            "AtSmartPtr ( const AtSmartPtr < T > & r ) ; "
            "} ;",
            tok("template<class T> class AtSmartPtr : public ConstCastHelper<AtSmartPtr<const T>, T>\n"
                "{\n"
                "    friend struct ConstCastHelper<AtSmartPtr<const T>, T>;\n"
                "    AtSmartPtr(const AtSmartPtr<T>& r);\n"
                "};"));

        // Similar problem can also happen with ...
        ASSERT_EQUALS(
            "struct A<int> ; "
            "struct A<int...> ; "
            "A<int> a ( 0 ) ; "
            "struct A<int> { "
            "A<int> ( int * p ) { ( A<int...> * ) ( p ) ; } "
            "} ; "
            "struct A<int...> { "
            "A<int...> ( int * p ) { "
            "( A<int...> * ) ( p ) ; "
            "} } ;",
            tok("template <typename... T> struct A\n"
                "{\n"
                "    A(T* p) {\n"
                "        (A<T...>*)(p);\n"
                "    }\n"
                "};\n"
                "A<int> a(0);"));
    }

    void template56() { // #7117
        const char code[] = "template<bool B> struct Foo { "
                            "  std::array<int, B ? 1 : 2> mfoo; "
                            "}; "
                            "void foo() { "
                            "  Foo<true> myFoo; "
                            "}";
        const char expected[] = "struct Foo<true> ; "
                                "void foo ( ) { "
                                "Foo<true> myFoo ; "
                                "} struct Foo<true> { "
                                "std :: array < int , 1 > mfoo ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code, dinit(CheckOptions, $.debugwarnings = true)));
        ASSERT_EQUALS("", errout_str());
    }

    void template57() { // #7891
        const char code[] = "template<class T> struct Test { Test(T); };\n"
                            "Test<unsigned long> test( 0 );";
        const char exp[] = "struct Test<unsignedlong> ; "
                           "Test<unsignedlong> test ( 0 ) ; "
                           "struct Test<unsignedlong> { Test<unsignedlong> ( unsigned long ) ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template58() { // #6021
        const char code[] = "template <typename A>\n"
                            "void TestArithmetic() {\n"
                            "  x(1 * CheckedNumeric<A>());\n"
                            "}\n"
                            "void foo() {\n"
                            "  TestArithmetic<int>();\n"
                            "}";
        const char exp[] = "void TestArithmetic<int> ( ) ; "
                           "void foo ( ) {"
                           " TestArithmetic<int> ( ) ; "
                           "} "
                           "void TestArithmetic<int> ( ) {"
                           " x ( 1 * CheckedNumeric < int > ( ) ) ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template59() { // #8051
        const char code[] = "template<int N>\n"
                            "struct Factorial {\n"
                            "    enum FacHelper { value = N * Factorial<N - 1>::value };\n"
                            "};\n"
                            "template <>\n"
                            "struct Factorial<0> {\n"
                            "    enum FacHelper { value = 1 };\n"
                            "};\n"
                            "template<int DiagonalDegree>\n"
                            "int diagonalGroupTest() {\n"
                            "    return Factorial<DiagonalDegree>::value;\n"
                            "}\n"
                            "int main () {\n"
                            "    return diagonalGroupTest<4>();\n"
                            "}";
        const char exp[] = "struct Factorial<0> ; "
                           "struct Factorial<4> ; "
                           "struct Factorial<3> ; "
                           "struct Factorial<2> ; "
                           "struct Factorial<1> ; "
                           "struct Factorial<0> { enum FacHelper { value = 1 } ; } ; "
                           "int diagonalGroupTest<4> ( ) ; "
                           "int main ( ) { return diagonalGroupTest<4> ( ) ; } "
                           "int diagonalGroupTest<4> ( ) { return Factorial<4> :: value ; } "
                           "struct Factorial<4> { enum FacHelper { value = ( int ) 4 * Factorial<3> :: value } ; } ; "
                           "struct Factorial<3> { enum FacHelper { value = ( int ) 3 * Factorial<2> :: value } ; } ; "
                           "struct Factorial<2> { enum FacHelper { value = ( int ) 2 * Factorial<1> :: value } ; } ; "
                           "struct Factorial<1> { enum FacHelper { value = ( int ) 1 * Factorial<0> :: value } ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template60() { // Extracted from Clang testfile
        const char code[] = "template <typename T> struct S { typedef int type; };\n"
                            "template <typename T> void f() {}\n"
                            "template <typename T> void h() { f<typename S<T>::type(0)>(); }\n"
                            "\n"
                            "void j() { h<int>(); }";
        const char exp[] = "struct S<int> ; "
                           "void f<S<int>::type(0)> ( ) ; "
                           "void h<int> ( ) ; "
                           "void j ( ) { h<int> ( ) ; } "
                           "void h<int> ( ) { f<S<int>::type(0)> ( ) ; } "
                           "struct S<int> { } ; "
                           "void f<S<int>::type(0)> ( ) { }";
        const char act[] = "template < typename T > struct S { } ; "
                           "void f<S<int>::type(0)> ( ) ; "
                           "void h<int> ( ) ; "
                           "void j ( ) { h<int> ( ) ; } "
                           "void h<int> ( ) { f<S<int>::type(0)> ( ) ; } "
                           "void f<S<int>::type(0)> ( ) { }";
        TODO_ASSERT_EQUALS(exp, act, tok(code));
    }

    void template61() { // hang in daca, code extracted from kodi
        const char code[] = "template <typename T> struct Foo {};\n"
                            "template <typename T> struct Bar {\n"
                            "  void f1(Bar<T> x) {}\n"
                            "  Foo<Bar<T>> f2() { }\n"
                            "};\n"
                            "Bar<int> c;";
        const char exp[] = "struct Foo<Bar<int>> ; "
                           "struct Bar<int> ; "
                           "Bar<int> c ; "
                           "struct Bar<int> {"
                           " void f1 ( Bar<int> x ) { }"
                           " Foo<Bar<int>> f2 ( ) { } "
                           "} ; "
                           "struct Foo<Bar<int>> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template62() { // #8314
        const char code[] = "template <class T> struct C1 {};\n"
                            "template <class T> void f() { x = y ? C1<int>::allocate(1) : 0; }\n"
                            "template <class T, unsigned S> class C3 {};\n"
                            "template <class T, unsigned S> C3<T, S>::C3(const C3<T, S> &v) { C1<T *> c1; }\n"
                            "C3<int,6> c3;";
        const char exp[] = "struct C1<int*> ; "
                           "template < class T > void f ( ) { x = y ? ( C1 < int > :: allocate ( 1 ) ) : 0 ; } "
                           "class C3<int,6> ; "
                           "C3<int,6> c3 ; "
                           "class C3<int,6> { } ; "
                           "C3<int,6> :: C3<int,6> ( const C3<int,6> & v ) { C1<int*> c1 ; } "
                           "struct C1<int*> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template63() { // #8576
        const char code[] = "template<class T> struct TestClass { T m_hi; };"
                            "TestClass<std::auto_ptr<v>> objTest3;";
        const char exp[] = "struct TestClass<std::auto_ptr<v>> ; "
                           "TestClass<std::auto_ptr<v>> objTest3 ; "
                           "struct TestClass<std::auto_ptr<v>> { std :: auto_ptr < v > m_hi ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template64() { // #8683
        const char code[] = "template <typename T>\n"
                            "bool foo(){return true;}\n"
                            "struct A {\n"
                            "template<int n>\n"
                            "void t_func()\n"
                            "{\n"
                            "     if( n != 0 || foo<int>());\n"
                            "}\n"
                            "void t_caller()\n"
                            "{\n"
                            "    t_func<0>();\n"
                            "    t_func<1>();\n"
                            "}\n"
                            "};";
        const char exp[] = "bool foo<int> ( ) ; "
                           "struct A { "
                           "void t_func<0> ( ) ; "
                           "void t_func<1> ( ) ; "
                           "void t_caller ( ) "
                           "{ "
                           "t_func<0> ( ) ; "
                           "t_func<1> ( ) ; "
                           "} "
                           "} ; "
                           "void A :: t_func<0> ( ) "
                           "{ "
                           "if ( ( int ) 0 != 0 || foo<int> ( ) ) { ; } "
                           "} "
                           "void A :: t_func<1> ( ) "
                           "{ "
                           "if ( ( int ) 1 != 0 || foo<int> ( ) ) { ; } "
                           "} "
                           "bool foo<int> ( ) { return true ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template65() { // #8321 (crash)
        const char code[] = "namespace bpp\n"
                            "{\n"
                            "template<class N, class E, class DAGraphImpl>\n"
                            "class AssociationDAGraphImplObserver :\n"
                            "  public AssociationGraphImplObserver<N, E, DAGraphImpl>\n"
                            "{};\n"
                            "template<class N, class E>\n"
                            "using AssociationDAGlobalGraphObserver =  AssociationDAGraphImplObserver<N, E, DAGlobalGraph>;\n"
                            "}\n"
                            "using namespace bpp;\n"
                            "using namespace std;\n"
                            "int main() {\n"
                            "  AssociationDAGlobalGraphObserver<string,unsigned int> grObs;\n"
                            " return 1;\n"
                            "}";
        const char exp[] = "namespace bpp "
                           "{ "
                           "class AssociationDAGraphImplObserver<string,unsignedint,DAGlobalGraph> ; "
                           "} "
                           "using namespace bpp ; "
                           "int main ( ) { "
                           "bpp :: AssociationDAGraphImplObserver<string,unsignedint,DAGlobalGraph> grObs ; "
                           "return 1 ; "
                           "} class bpp :: AssociationDAGraphImplObserver<string,unsignedint,DAGlobalGraph> : "
                           "public AssociationGraphImplObserver < std :: string , unsigned int , DAGlobalGraph > "
                           "{ } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template66() { // #8725
        const char code[] = "template <class T> struct Fred {\n"
                            "    const int ** foo();\n"
                            "};\n"
                            "template <class T> const int ** Fred<T>::foo() { return nullptr; }\n"
                            "Fred<int> fred;";
        const char exp[] = "struct Fred<int> ; "
                           "Fred<int> fred ; "
                           "struct Fred<int> { "
                           "const int * * foo ( ) ; "
                           "} ; "
                           "const int * * Fred<int> :: foo ( ) { return nullptr ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template67() { // ticket #8122
        const char code[] = "template <class T> struct Container {\n"
                            "  Container();\n"
                            "  Container(const Container &);\n"
                            "  Container & operator = (const Container &);\n"
                            "  ~Container();\n"
                            "  T* mElements;\n"
                            "  const Container * c;\n"
                            "};\n"
                            "template <class T> Container<T>::Container() : mElements(nullptr), c(nullptr) {}\n"
                            "template <class T> Container<T>::Container(const Container & x) { nElements = x.nElements; c = x.c; }\n"
                            "template <class T> Container<T> & Container<T>::operator = (const Container & x) { mElements = x.mElements; c = x.c; return *this; }\n"
                            "template <class T> Container<T>::~Container() {}\n"
                            "Container<int> intContainer;";

        const char expected[] = "struct Container<int> ; "
                                "Container<int> intContainer ; "
                                "struct Container<int> { "
                                "Container<int> ( ) ; "
                                "Container<int> ( const Container<int> & ) ; "
                                "Container<int> & operator= ( const Container<int> & ) ; "
                                "~ Container<int> ( ) ; "
                                "int * mElements ; "
                                "const Container<int> * c ; "
                                "} ; "
                                "Container<int> :: Container<int> ( ) : mElements ( nullptr ) , c ( nullptr ) { } "
                                "Container<int> :: Container<int> ( const Container<int> & x ) { nElements = x . nElements ; c = x . c ; } "
                                "Container<int> & Container<int> :: operator= ( const Container<int> & x ) { mElements = x . mElements ; c = x . c ; return * this ; } "
                                "Container<int> :: ~ Container<int> ( ) { }";

        ASSERT_EQUALS(expected, tok(code));
    }

    void template68() {
        const char code[] = "template <class T> union Fred {\n"
                            "    char dummy[sizeof(T)];\n"
                            "    T value;\n"
                            "};\n"
                            "Fred<int> fred;";
        const char exp[] = "union Fred<int> ; "
                           "Fred<int> fred ; "
                           "union Fred<int> { "
                           "char dummy [ sizeof ( int ) ] ; "
                           "int value ; "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template69() { // #8791
        const char code[] = "class Test {\n"
                            "    int test;\n"
                            "    template <class T> T lookup() { return test; }\n"
                            "    int Fun() { return lookup<int>(); }\n"
                            "};";
        const char exp[] = "class Test { "
                           "int test ; "
                           "int lookup<int> ( ) ; "
                           "int Fun ( ) { return lookup<int> ( ) ; } "
                           "} ; "
                           "int Test :: lookup<int> ( ) { return test ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template70() { // #5289
        const char code[] = "template<typename T, typename V, int KeySize = 0> class Bar;\n"
                            "template<>\n"
                            "class Bar<void, void> {\n"
                            "};\n"
                            "template<typename K, typename V, int KeySize>\n"
                            "class Bar : private Bar<void, void> {\n"
                            "   void foo() { }\n"
                            "};";
        const char exp[] = "template < typename T , typename V , int KeySize = 0 > class Bar ; "
                           "class Bar<void,void> ; "
                           "class Bar<void,void> { "
                           "} ; "
                           "template < typename K , typename V , int KeySize = 0 > "
                           "class Bar : private Bar<void,void> { "
                           "void foo ( ) { } "
                           "} ;";
        const char act[] = "template < typename T , typename V , int KeySize = 0 > class Bar ; "
                           "class Bar<void,void> { "
                           "} ; "
                           "class Bar<void,void> ; "
                           "template < typename K , typename V , int KeySize = 0 > "
                           "class Bar : private Bar<void,void> { "
                           "void foo ( ) { } "
                           "} ;";
        TODO_ASSERT_EQUALS(exp, act, tok(code));
    }

    void template71() { // #8821
        const char code[] = "int f1(int * pInterface, int x) { return 0; }\n"
                            "\n"
                            "template< class interface_type > class Reference {\n"
                            "  template< class interface_type > int i();\n"
                            "  int *pInterface;\n"
                            "};\n"
                            "\n"
                            "template< class interface_type > int Reference< interface_type >::i() {\n"
                            "    return f1(pInterface, interface_type::static_type());\n"
                            "}\n"
                            "\n"
                            "Reference< class XPropertyList > dostuff();";
        const char exp[] = "int f1 ( int * pInterface , int x ) { return 0 ; } "
                           "class Reference<XPropertyList> ; "
                           "Reference<XPropertyList> dostuff ( ) ; "
                           "class Reference<XPropertyList> { template < class XPropertyList > int i ( ) ; int * pInterface ; } ; "
                           "int Reference<XPropertyList> :: i ( ) { return f1 ( pInterface , XPropertyList :: static_type ( ) ) ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template72() {
        const char code[] = "template <typename N, typename P> class Tokenizer;\n"
                            "const Tokenizer<Node, Path> *tokenizer() const;\n"
                            "template <typename N, typename P>\n"
                            "Tokenizer<N, P>::Tokenizer() { }";
        const char exp[] = "template < typename N , typename P > class Tokenizer ; "
                           "const Tokenizer < Node , Path > * tokenizer ( ) const ; "
                           "template < typename N , typename P > "
                           "Tokenizer < N , P > :: Tokenizer ( ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template73() {
        const char code[] = "template<typename T>\n"
                            "void keep_range(T& value, const T mini, const T maxi){}\n"
                            "template void keep_range<float>(float& v, const float l, const float u);\n"
                            "template void keep_range<int>(int& v, const int l, const int u);";
        const char exp[] = "void keep_range<float> ( float & value , const float mini , const float maxi ) ; "
                           "void keep_range<int> ( int & value , const int mini , const int maxi ) ; "
                           "void keep_range<float> ( float & value , const float mini , const float maxi ) { } "
                           "void keep_range<int> ( int & value , const int mini , const int maxi ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template74() {
        const char code[] = "template <class T> class BTlist { };\n"
                            "class PushBackStreamBuf {\n"
                            "public:\n"
                            "    void pushBack(const BTlist<int> &vec);\n"
                            "};";
        const char exp[] = "class BTlist<int> ; "
                           "class PushBackStreamBuf { "
                           "public: "
                           "void pushBack ( const BTlist<int> & vec ) ; "
                           "} ; "
                           "class BTlist<int> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template75() {
        const char code[] = "template<typename T>\n"
                            "T foo(T& value){ return value; }\n"
                            "template std::vector<std::vector<int>> foo<std::vector<std::vector<int>>>(std::vector<std::vector<int>>& v);";
        const char exp[] = "std :: vector < std :: vector < int > > foo<std::vector<std::vector<int>>> ( std :: vector < std :: vector < int > > & value ) ; "
                           "std :: vector < std :: vector < int > > foo<std::vector<std::vector<int>>> ( std :: vector < std :: vector < int > > & value ) { return value ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template76() {
        const char code[] = "namespace NS {\n"
                            "    template<typename T> T foo(T& value) { return value; }\n"
                            "    template std::vector<std::vector<int>> foo<std::vector<std::vector<int>>>(std::vector<std::vector<int>>& v);\n"
                            "}\n"
                            "std::vector<std::vector<int>> v;\n"
                            "v = foo<std::vector<std::vector<int>>>(v);\n";
        const char exp[] = "namespace NS { "
                           "std :: vector < std :: vector < int > > foo<std::vector<std::vector<int>>> ( std :: vector < std :: vector < int > > & value ) ; "
                           "} "
                           "std :: vector < std :: vector < int > > v ; "
                           "v = foo<std::vector<std::vector<int>>> ( v ) ; "
                           "std :: vector < std :: vector < int > > NS :: foo<std::vector<std::vector<int>>> ( std :: vector < std :: vector < int > > & value ) { return value ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template77() {
        const char code[] = "template<typename T>\n"
                            "struct is_void : std::false_type { };\n"
                            "template<>\n"
                            "struct is_void<void> : std::true_type { };\n"
                            "int main() {\n"
                            "    std::cout << is_void<char>::value << std::endl;\n"
                            "    std::cout << is_void<void>::value << std::endl;\n"
                            "}";
        const char exp[] = "struct is_void<void> ; "
                           "struct is_void<char> ; "
                           "struct is_void<void> : std :: true_type { } ; "
                           "int main ( ) { "
                           "std :: cout << is_void<char> :: value << std :: endl ; "
                           "std :: cout << is_void<void> :: value << std :: endl ; "
                           "} "
                           "struct is_void<char> : std :: false_type { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template78() {
        const char code[] = "template <typename>\n"
                            "struct Base { };\n"
                            "struct S : Base <void>::Type { };";
        const char exp[] = "struct Base<void> ; "
                           "struct S : Base<void> :: Type { } ; "
                           "struct Base<void> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template79() { // #5133
        const char code[] = "class Foo {\n"
                            "public:\n"
                            "    template<typename T> void foo() { bar<T>(); }\n"
                            "private:\n"
                            "    template<typename T> void bar() { bazz(); }\n"
                            "    void bazz() { }\n"
                            "};\n"
                            "void some_func() {\n"
                            "    Foo x;\n"
                            "    x.foo<int>();\n"
                            "}";
        const char exp[] = "class Foo { "
                           "public: "
                           "void foo<int> ( ) ; "
                           "private: "
                           "void bar<int> ( ) ; "
                           "void bazz ( ) { } "
                           "} ; "
                           "void some_func ( ) { "
                           "Foo x ; "
                           "x . foo<int> ( ) ; "
                           "} "
                           "void Foo :: foo<int> ( ) { bar<int> ( ) ; } "
                           "void Foo :: bar<int> ( ) { bazz ( ) ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template80() {
        const char code[] = "class Fred {\n"
                            "    template <typename T> T foo(T t) const { return t; }\n"
                            "};\n"
                            "const void * p = Fred::foo<const void *>(nullptr);";
        const char exp[] = "class Fred { "
                           "const void * foo<constvoid*> ( const void * t ) const ; "
                           "} ; "
                           "const void * p ; p = Fred :: foo<constvoid*> ( nullptr ) ; "
                           "const void * Fred :: foo<constvoid*> ( const void * t ) const { return t ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template81() {
        const char code[] = "template <typename Type>\n"
                            "struct SortWith {\n"
                            "    SortWith(Type);\n"
                            "};\n"
                            "template <typename Type>\n"
                            "SortWith<Type>::SortWith(Type) {}\n"
                            "int main() {\n"
                            "    SortWith<int>(0);\n"
                            "}";
        const char exp[] = "template < typename Type > "
                           "struct SortWith { "
                           "SortWith ( Type ) ; "
                           "} ; "
                           "SortWith<int> :: SortWith<int> ( int ) ; "
                           "int main ( ) { "
                           "SortWith<int> ( 0 ) ; "
                           "} "
                           "SortWith<int> :: SortWith<int> ( int ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template82() { // 8603
        const char code[] = "typedef int comp;\n"
                            "const int f16=16;\n"
                            "template<int x>\n"
                            "class tvec2 {};\n"
                            "template<int x>\n"
                            "class tvec3 {};\n"
                            "namespace swizzle {\n"
                            "template <comp> void swizzle(tvec2<f16> v) { }\n"
                            "template <comp x, comp y> void swizzle(tvec3<f16> v) { }\n"
                            "}\n"
                            "void foo() {\n"
                            "  using namespace swizzle;\n"
                            "  tvec2<f16> tt2;\n"
                            "  swizzle<1>(tt2);\n"
                            "  tvec3<f16> tt3;\n"
                            "  swizzle<2,3>(tt3);\n"
                            "}";
        const char exp[] = "const int f16 = 16 ; "
                           "class tvec2<f16> ; "
                           "class tvec3<f16> ; "
                           "namespace swizzle { "
                           "void swizzle<1> ( tvec2<f16> v ) ; "
                           "void swizzle<2,3> ( tvec3<f16> v ) ; "
                           "} "
                           "void foo ( ) { "
                           "using namespace swizzle ; "
                           "tvec2<f16> tt2 ; "
                           "swizzle :: swizzle<1> ( tt2 ) ; "
                           "tvec3<f16> tt3 ; "
                           "swizzle :: swizzle<2,3> ( tt3 ) ; "
                           "} "
                           "void swizzle :: swizzle<2,3> ( tvec3<f16> v ) { } "
                           "void swizzle :: swizzle<1> ( tvec2<f16> v ) { } "
                           "class tvec3<f16> { } ; "
                           "class tvec2<f16> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template83() { // #8867
        const char code[] = "template<typename Task>\n"
                            "class MultiConsumer {\n"
                            "    MultiConsumer();\n"
                            "};\n"
                            "template<typename Task>\n"
                            "MultiConsumer<Task>::MultiConsumer() : sizeBuffer(0) {}\n"
                            "MultiReads::MultiReads() {\n"
                            "    mc = new MultiConsumer<reads_packet>();\n"
                            "}";
        const char exp[] = "template < typename Task > " // TODO: this should be expanded
                           "class MultiConsumer { "
                           "MultiConsumer ( ) ; "
                           "} ; "
                           "MultiConsumer<reads_packet> :: MultiConsumer<reads_packet> ( ) ; "
                           "MultiReads :: MultiReads ( ) { "
                           "mc = new MultiConsumer<reads_packet> ( ) ; "
                           "} "
                           "MultiConsumer<reads_packet> :: MultiConsumer<reads_packet> ( ) : sizeBuffer ( 0 ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template84() { // #8880
        {
            const char code[] = "template <class b, int c, class>\n"
                                "auto d() -> typename a<decltype(b{})>::e {\n"
                                "  d<int, c, int>();\n"
                                "}";
            const char exp[] = "template < class b , int c , class > "
                               "auto d ( ) . a < decltype ( b { } ) > :: e { "
                               "d < int , c , int > ( ) ; "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <class b, int c, class>\n"
                                "auto d() -> typename a<decltype(b{})>::e {\n"
                                "  d<int, c, int>();\n"
                                "}"
                                "void foo() { d<char, 1, int>(); }";
            const char exp[] = "auto d<char,1,int> ( ) . a < char > :: e ; "
                               "auto d<int,1,int> ( ) . a < int > :: e ; "
                               "void foo ( ) { d<char,1,int> ( ) ; } "
                               "auto d<char,1,int> ( ) . a < char > :: e { "
                               "d<int,1,int> ( ) ; "
                               "} "
                               "auto d<int,1,int> ( ) . a < int > :: e { "
                               "d<int,1,int> ( ) ; "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template85() { // #8902 - crash
        const char code[] = "template<typename T>\n"
                            "struct C\n"
                            "{\n"
                            "  template<typename U, typename std::enable_if<(!std::is_fundamental<U>::value)>::type* = nullptr>\n"
                            "  void foo();\n"
                            "};\n"
                            "extern template void C<int>::foo<int, nullptr>();\n"
                            "template<typename T>\n"
                            "template<typename U, typename std::enable_if<(!std::is_fundamental<U>::value)>::type>\n"
                            "void C<T>::foo() {}";
        // @todo the output is very wrong but we are only worried about the crash for now
        (void)tok(code);
    }

    void template86() { // crash
        const char code[] = "struct S {\n"
                            "  S();\n"
                            "};\n"
                            "template <typename T>\n"
                            "struct U {\n"
                            "  static S<T> u;\n"
                            "};\n"
                            "template <typename T>\n"
                            "S<T> U<T>::u;\n"
                            "template S<int> U<int>::u;\n"
                            "S<int> &i = U<int>::u;";
        (void)tok(code);
    }

    void template87() {
        const char code[] = "template<typename T>\n"
                            "T f1(T t) { return t; }\n"
                            "template const char * f1<const char *>(const char *);\n"
                            "template const char & f1<const char &>(const char &);";
        const char exp[] = "const char * f1<constchar*> ( const char * t ) ; "
                           "const char & f1<constchar&> ( const char & t ) ; "
                           "const char * f1<constchar*> ( const char * t ) { return t ; } "
                           "const char & f1<constchar&> ( const char & t ) { return t ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template88() { // #6183.cpp
        const char code[] = "class CTest {\n"
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
                            "	CTest::SayHello();\n"
                            "}\n"
                            "int main() {\n"
                            "    CTest::Greeting<bool>(true);\n"
                            "    return 0;\n"
                            "}";
        const char exp[] = "class CTest { "
                           "public: "
                           "static void Greeting<bool> ( bool ) ; "
                           "template < typename T > "
                           "static void Greeting ( T val ) { "
                           "std :: cout << val << std :: endl ; "
                           "} "
                           "private: "
                           "static void SayHello ( ) { "
                           "std :: cout << \"Hello World!\" << std :: endl ; "
                           "} "
                           "} ; "
                           "void CTest :: Greeting<bool> ( bool ) { "
                           "CTest :: SayHello ( ) ; "
                           "} "
                           "int main ( ) { "
                           "CTest :: Greeting<bool> ( true ) ; "
                           "return 0 ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template89() { // #8917
        const char code[] = "struct Fred {\n"
                            "    template <typename T> static void foo() { }\n"
                            "};\n"
                            "template void Fred::foo<char>();\n"
                            "template void Fred::foo<float>();\n"
                            "template <> void Fred::foo<bool>() { }\n"
                            "template <> void Fred::foo<int>() { }";
        const char exp[] = "struct Fred { "
                           "static void foo<int> ( ) ; "
                           "static void foo<bool> ( ) ; "
                           "static void foo<char> ( ) ; "
                           "static void foo<float> ( ) ; "
                           "} ; "
                           "void Fred :: foo<bool> ( ) { } "
                           "void Fred :: foo<int> ( ) { } "
                           "void Fred :: foo<char> ( ) { } "
                           "void Fred :: foo<float> ( ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template90() { // crash
        const char code[] = "template <typename T> struct S1 {};\n"
                            "void f(S1<double>) {}\n"
                            "template <typename T>\n"
                            "decltype(S1<T>().~S1<T>()) fun1() {};";
        const char exp[] = "struct S1<double> ; "
                           "void f ( S1<double> ) { } "
                           "template < typename T > "
                           "decltype ( S1 < T > ( ) . ~ S1 < T > ( ) ) fun1 ( ) { } ; "
                           "struct S1<double> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template91() {
        {
            const char code[] = "template<typename T> T foo(T t) { return t; }\n"
                                "template<> char foo<char>(char a) { return a; }\n"
                                "template<> int foo<int>(int a) { return a; }\n"
                                "template float foo<float>(float);\n"
                                "template double foo<double>(double);";
            const char exp[] = "int foo<int> ( int a ) ; "
                               "char foo<char> ( char a ) ; "
                               "float foo<float> ( float t ) ; "
                               "double foo<double> ( double t ) ; "
                               "char foo<char> ( char a ) { return a ; } "
                               "int foo<int> ( int a ) { return a ; } "
                               "float foo<float> ( float t ) { return t ; } "
                               "double foo<double> ( double t ) { return t ; }";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "struct Fred {\n"
                                "    template<typename T> T foo(T t) { return t; }\n"
                                "    template<> char foo<char>(char a) { return a; }\n"
                                "    template<> int foo<int>(int a) { return a; }\n"
                                "};\n"
                                "template float Fred::foo<float>(float);\n"
                                "template double Fred::foo<double>(double);";
            const char exp[] = "struct Fred { "
                               "int foo<int> ( int a ) ; "
                               "char foo<char> ( char a ) ; "
                               "float foo<float> ( float t ) ; "
                               "double foo<double> ( double t ) ; "
                               "char foo<char> ( char a ) { return a ; } "
                               "int foo<int> ( int a ) { return a ; } "
                               "} ; "
                               "float Fred :: foo<float> ( float t ) { return t ; } "
                               "double Fred :: foo<double> ( double t ) { return t ; }";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "namespace NS1 {\n"
                                "    namespace NS2 {\n"
                                "        template<typename T> T foo(T t) { return t; }\n"
                                "        template<> char foo<char>(char a) { return a; }\n"
                                "        template<> int foo<int>(int a) { return a; }\n"
                                "        template short NS2::foo<short>(short);\n"
                                "        template long NS1::NS2::foo<long>(long);\n"
                                "    }\n"
                                "    template float NS2::foo<float>(float);\n"
                                "    template bool NS1::NS2::foo<bool>(bool);\n"
                                "}\n"
                                "template double NS1::NS2::foo<double>(double);";
            const char exp[] = "namespace NS1 { "
                               "namespace NS2 { "
                               "int foo<int> ( int a ) ; "
                               "char foo<char> ( char a ) ; "
                               "short foo<short> ( short t ) ; "
                               "long foo<long> ( long t ) ; "
                               "float foo<float> ( float t ) ; "
                               "bool foo<bool> ( bool t ) ; "
                               "double foo<double> ( double t ) ; "
                               "char foo<char> ( char a ) { return a ; } "
                               "int foo<int> ( int a ) { return a ; } "
                               "} "
                               "} "
                               "short NS1 :: NS2 :: foo<short> ( short t ) { return t ; } "
                               "long NS1 :: NS2 :: foo<long> ( long t ) { return t ; } "
                               "float NS1 :: NS2 :: foo<float> ( float t ) { return t ; } "
                               "bool NS1 :: NS2 :: foo<bool> ( bool t ) { return t ; } "
                               "double NS1 :: NS2 :: foo<double> ( double t ) { return t ; }";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "namespace NS1 {\n"
                                "    namespace NS {\n"
                                "        template<typename T> T foo(T t) { return t; }\n"
                                "        template<> char foo<char>(char a) { return a; }\n"
                                "        template<> int foo<int>(int a) { return a; }\n"
                                "        template short NS::foo<short>(short);\n"
                                "        template long NS1::NS::foo<long>(long);\n"
                                "    }\n"
                                "    template float NS::foo<float>(float);\n"
                                "    template bool NS1::NS::foo<bool>(bool);\n"
                                "}\n"
                                "template double NS1::NS::foo<double>(double);";
            const char exp[] = "namespace NS1 { "
                               "namespace NS { "
                               "int foo<int> ( int a ) ; "
                               "char foo<char> ( char a ) ; "
                               "short foo<short> ( short t ) ; "
                               "long foo<long> ( long t ) ; "
                               "float foo<float> ( float t ) ; "
                               "bool foo<bool> ( bool t ) ; "
                               "double foo<double> ( double t ) ; "
                               "char foo<char> ( char a ) { return a ; } "
                               "int foo<int> ( int a ) { return a ; } "
                               "} "
                               "} "
                               "short NS1 :: NS :: foo<short> ( short t ) { return t ; } "
                               "long NS1 :: NS :: foo<long> ( long t ) { return t ; } "
                               "float NS1 :: NS :: foo<float> ( float t ) { return t ; } "
                               "bool NS1 :: NS :: foo<bool> ( bool t ) { return t ; } "
                               "double NS1 :: NS :: foo<double> ( double t ) { return t ; }";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template92() {
        const char code[] = "template<class T> void foo(T const& t) { }\n"
                            "template<> void foo<double>(double const& d) { }\n"
                            "template void foo<float>(float const& f);\n"
                            "int main() {\n"
                            "    foo<int>(2);\n"
                            "    foo<double>(3.14);\n"
                            "    foo<float>(3.14f);\n"
                            "}";
        const char exp[] = "void foo<double> ( const double & d ) ; "
                           "void foo<float> ( const float & t ) ; "
                           "void foo<int> ( const int & t ) ; "
                           "void foo<double> ( const double & d ) { } "
                           "int main ( ) { "
                           "foo<int> ( 2 ) ; "
                           "foo<double> ( 3.14 ) ; "
                           "foo<float> ( 3.14f ) ; "
                           "} "
                           "void foo<float> ( const float & t ) { } "
                           "void foo<int> ( const int & t ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template93() { // crash
        const char code[] = "template <typename Iterator>\n"
                            "void ForEach() { }\n"
                            "template <typename Type>\n"
                            "class Vector2 : public Vector {\n"
                            "    template <typename Iterator>\n"
                            "    void ForEach();\n"
                            "public:\n"
                            "    void process();\n"
                            "};\n"
                            "template <typename Type>\n"
                            "void Vector2<Type>::process() {\n"
                            "    ForEach<iterator>();\n"
                            "}\n"
                            "Vector2<string> c;";
        const char exp[] = "void ForEach<iterator> ( ) ; "
                           "class Vector2<string> ; "
                           "Vector2<string> c ; "
                           "class Vector2<string> : public Vector { "
                           "template < typename Iterator > "
                           "void ForEach ( ) ; "
                           "public: "
                           "void process ( ) ; "
                           "} ; "
                           "void Vector2<string> :: process ( ) { "
                           "ForEach<iterator> ( ) ; "
                           "} "
                           "void ForEach<iterator> ( ) { "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template94() { // #8927 crash
        const char code[] = "template <typename T>\n"
                            "class Array { };\n"
                            "template<typename T>\n"
                            "Array<T> foo() {};\n"
                            "template <> Array<double> foo<double>() { }\n"
                            "template <> Array<std::complex<float>> foo<std::complex<float>>() { }\n"
                            "template <> Array<float> foo<float>() { }\n"
                            "template < typename T >\n"
                            "Array<T> matmul() {\n"
                            "    return foo<T>( );\n"
                            "}\n"
                            "template Array<std::complex<float>> matmul<std::complex<float>>();";
        const char exp[] = "class Array<double> ; "
                           "class Array<std::complex<float>> ; "
                           "class Array<float> ; "
                           "Array<float> foo<float> ( ) ; "
                           "Array<std::complex<float>> foo<std::complex<float>> ( ) ; "
                           "Array<double> foo<double> ( ) ; "
                           "template < typename T > "
                           "Array < T > foo ( ) { } ; "
                           "Array<double> foo<double> ( ) { } "
                           "Array<std::complex<float>> foo<std::complex<float>> ( ) { } "
                           "Array<float> foo<float> ( ) { } "
                           "Array<std::complex<float>> matmul<std::complex<float>> ( ) ; "
                           "Array<std::complex<float>> matmul<std::complex<float>> ( ) { "
                           "return foo<std::complex<float>> ( ) ; "
                           "} "
                           "class Array<double> { } ; "
                           "class Array<std::complex<float>> { } ; "
                           "class Array<float> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template95() { // #7417
        const char code[] = "template <typename T>\n"
                            "T Value = 123;\n"
                            "template<>\n"
                            "int Value<int> = 456;\n"
                            "float f = Value<float>;\n"
                            "int i = Value<int>;";
        const char exp[] = "float Value<float> ; Value<float> = 123 ; "
                           "int Value<int> ; Value<int> = 456 ; "
                           "float f ; f = Value<float> ; "
                           "int i ; i = Value<int> ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template96() { // #7854
        {
            const char code[] = "template<unsigned int n>\n"
                                "  constexpr long fib = fib<n-1> + fib<n-2>;\n"
                                "template<>\n"
                                "  constexpr long fib<0> = 0;\n"
                                "template<>\n"
                                "  constexpr long fib<1> = 1;\n"
                                "long f0 = fib<0>;\n"
                                "long f1 = fib<1>;\n"
                                "long f2 = fib<2>;\n"
                                "long f3 = fib<3>;";
            const char exp[] = "constexpr long fib<2> = fib<1> + fib<0> ; "
                               "constexpr long fib<3> = fib<2> + fib<1> ; "
                               "constexpr long fib<0> = 0 ; "
                               "constexpr long fib<1> = 1 ; "
                               "long f0 ; f0 = fib<0> ; "
                               "long f1 ; f1 = fib<1> ; "
                               "long f2 ; f2 = fib<2> ; "
                               "long f3 ; f3 = fib<3> ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template<unsigned int n>\n"
                                "  constexpr long fib = fib<n-1> + fib<n-2>;\n"
                                "template<>\n"
                                "  constexpr long fib<0> = 0;\n"
                                "template<>\n"
                                "  constexpr long fib<1> = 1;\n"
                                "long f5 = fib<5>;\n";
            const char exp[] = "constexpr long fib<5> = fib<4> + fib<3> ; "
                               "constexpr long fib<4> = fib<3> + fib<2> ; "
                               "constexpr long fib<3> = fib<2> + fib<1> ; "
                               "constexpr long fib<2> = fib<1> + fib<0> ; "
                               "constexpr long fib<0> = 0 ; "
                               "constexpr long fib<1> = 1 ; "
                               "long f5 ; f5 = fib<5> ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template97() {
        const char code[] ="namespace NS1 {\n"
                            "    namespace NS2 {\n"
                            "        namespace NS3 {\n"
                            "            namespace NS4 {\n"
                            "                template<class T>\n"
                            "                class Fred {\n"
                            "                    T * t;\n"
                            "                public:\n"
                            "                    Fred<T>() : t(nullptr) {}\n"
                            "                };\n"
                            "            }\n"
                            "            using namespace NS4;\n"
                            "            Fred<bool> fred_bool;\n"
                            "            NS4::Fred<char> fred_char;\n"
                            "        }\n"
                            "        using namespace NS3;\n"
                            "        NS4::Fred<short> fred_short;\n"
                            "        using namespace NS3::NS4;\n"
                            "        Fred<int> fred_int;\n"
                            "        NS3::NS4::Fred<long> fred_long;\n"
                            "        NS2::NS3::NS4::Fred<float> fred_float;\n"
                            "        NS1::NS2::NS3::NS4::Fred<double> fred_double;\n"
                            "    }\n"
                            "    using namespace NS2;\n"
                            "    NS3::NS4::Fred<float> fred_float1;\n"
                            "    NS2::NS3::NS4::Fred<double> fred_double1;\n"
                            "}\n"
                            "using namespace NS1::NS2::NS3::NS4;\n"
                            "Fred<bool> fred_bool1;\n"
                            "NS1::NS2::NS3::NS4::Fred<int> fred_int1;";
        const char exp[] = "namespace NS1 { "
                           "namespace NS2 { "
                           "namespace NS3 { "
                           "namespace NS4 { "
                           "class Fred<bool> ; "
                           "class Fred<char> ; "
                           "class Fred<short> ; "
                           "class Fred<int> ; "
                           "class Fred<long> ; "
                           "class Fred<float> ; "
                           "class Fred<double> ; "
                           "} "
                           "using namespace NS4 ; "
                           "NS4 :: Fred<bool> fred_bool ; "
                           "NS4 :: Fred<char> fred_char ; "
                           "} "
                           "using namespace NS3 ; "
                           "NS3 :: NS4 :: Fred<short> fred_short ; "
                           "using namespace NS3 :: NS4 ; "
                           "NS3 :: NS4 :: Fred<int> fred_int ; "
                           "NS3 :: NS4 :: Fred<long> fred_long ; "
                           "NS2 :: NS3 :: NS4 :: Fred<float> fred_float ; "
                           "NS1 :: NS2 :: NS3 :: NS4 :: Fred<double> fred_double ; "
                           "} "
                           "using namespace NS2 ; "
                           "NS2 :: NS3 :: NS4 :: Fred<float> fred_float1 ; "
                           "NS2 :: NS3 :: NS4 :: Fred<double> fred_double1 ; "
                           "} "
                           "using namespace NS1 :: NS2 :: NS3 :: NS4 ; "
                           "NS1 :: NS2 :: NS3 :: NS4 :: Fred<bool> fred_bool1 ; "
                           "NS1 :: NS2 :: NS3 :: NS4 :: Fred<int> fred_int1 ; "
                           "class NS1 :: NS2 :: NS3 :: NS4 :: Fred<bool> { "
                           "bool * t ; "
                           "public: "
                           "Fred<bool> ( ) : t ( nullptr ) { } "
                           "} ; "
                           "class NS1 :: NS2 :: NS3 :: NS4 :: Fred<char> { "
                           "char * t ; "
                           "public: "
                           "Fred<char> ( ) : t ( nullptr ) { } "
                           "} ; "
                           "class NS1 :: NS2 :: NS3 :: NS4 :: Fred<short> { "
                           "short * t ; "
                           "public: "
                           "Fred<short> ( ) : t ( nullptr ) { } "
                           "} ; "
                           "class NS1 :: NS2 :: NS3 :: NS4 :: Fred<int> { "
                           "int * t ; "
                           "public: "
                           "Fred<int> ( ) : t ( nullptr ) { } "
                           "} ; "
                           "class NS1 :: NS2 :: NS3 :: NS4 :: Fred<long> { "
                           "long * t ; "
                           "public: "
                           "Fred<long> ( ) : t ( nullptr ) { } "
                           "} ; "
                           "class NS1 :: NS2 :: NS3 :: NS4 :: Fred<float> { "
                           "float * t ; "
                           "public: "
                           "Fred<float> ( ) : t ( nullptr ) { } "
                           "} ; "
                           "class NS1 :: NS2 :: NS3 :: NS4 :: Fred<double> { "
                           "double * t ; "
                           "public: "
                           "Fred<double> ( ) : t ( nullptr ) { } "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template98() { // #8959
        const char code[] = "template <typename T>\n"
                            "using unique_ptr_with_deleter = std::unique_ptr<T, std::function<void(T*)>>;\n"
                            "class A {};\n"
                            "static void func() {\n"
                            "    unique_ptr_with_deleter<A> tmp(new A(), [](A* a) {\n"
                            "        delete a;\n"
                            "    });\n"
                            "}";
        const char exp[] = "class A { } ; "
                           "static void func ( ) { "
                           "std :: unique_ptr < A , std :: function < void ( A * ) > > tmp ( new A ( ) , [ ] ( A * a ) { "
                           "delete a ; "
                           "} ) ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template99() { // #8960
        const char code[] = "template <typename T>\n"
                            "class Base {\n"
                            "public:\n"
                            "    using ArrayType = std::vector<Base<T>>;\n"
                            "};\n"
                            "using A = Base<int>;\n"
                            "static A::ArrayType array;\n";
        const char exp[] = "class Base<int> ; "
                           "static std :: vector < Base<int> > array ; "
                           "class Base<int> { "
                           "public: "
                           "} ;";

        ASSERT_EQUALS(exp, tok(code));
    }

    void template100() { // #8967
        const char code[] = "enum class Device { I2C0, I2C1 };\n"
                            "template <Device D>\n"
                            "const char* deviceFile;\n"
                            "template <>\n"
                            "const char* deviceFile<Device::I2C0> = \"/tmp/i2c-0\";\n";

        const char exp[] = "enum class Device { I2C0 , I2C1 } ; "
                           "template < Device D > "
                           "const char * deviceFile ; "
                           "const char * deviceFile<Device::I2C0> ; deviceFile<Device::I2C0> = \"/tmp/i2c-0\" ;";

        ASSERT_EQUALS(exp, tok(code));
    }

    void template101() { // #8968
        const char code[] = "class A {\n"
                            "public:\n"
                            "    using ArrayType = std::vector<int>;\n"
                            "    void func(typename ArrayType::size_type i) {\n"
                            "    }\n"
                            "};";

        const char exp[] = "class A { "
                           "public: "
                           "void func ( std :: vector < int > :: size_type i ) { "
                           "} "
                           "} ;";

        ASSERT_EQUALS(exp, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void template102() { // #9005
        const char code[] = "namespace ns {\n"
                            "template <class T>\n"
                            "struct is_floating_point\n"
                            ": std::integral_constant<bool, std::is_floating_point<T>::value || true>\n"
                            "{};\n"
                            "}\n"
                            "void f() {\n"
                            "    if(std::is_floating_point<float>::value) {}\n"
                            "}";
        const char exp[] = "namespace ns { "
                           "template < class T > "
                           "struct is_floating_point "
                           ": std :: integral_constant < bool , std :: is_floating_point < T > :: value || true > "
                           "{ } ; "
                           "} "
                           "void f ( ) { "
                           "if ( std :: is_floating_point < float > :: value ) { } "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template103() {
        const char code[] = "namespace sample {\n"
                            "  template <typename T>\n"
                            "  class Sample {\n"
                            "  public:\n"
                            "    T function(T t);\n"
                            "  };\n"
                            "  template <typename T>\n"
                            "  T Sample<T>::function(T t) {\n"
                            "    return t;\n"
                            "  }\n"
                            "}\n"
                            "sample::Sample<int> s1;";
        const char exp[] = "namespace sample { "
                           "class Sample<int> ; "
                           "} "
                           "sample :: Sample<int> s1 ; "
                           "class sample :: Sample<int> { "
                           "public: "
                           "int function ( int t ) ; "
                           "} ; "
                           "int sample :: Sample<int> :: function ( int t ) { "
                           "return t ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template104() { // #9021
        const char code[] = "template < int i >\n"
                            "auto key ( ) { return hana :: test :: ct_eq < i > { } ; }\n"
                            "template < int i >\n"
                            "auto val ( ) { return hana :: test :: ct_eq < - i > { } ; }\n"
                            "template < int i , int j >\n"
                            "auto p ( ) { return :: minimal_product ( key < i > ( ) , val < j > ( ) ) ; }\n"
                            "int main ( ) {\n"
                            "    BOOST_HANA_CONSTANT_CHECK ( hana :: equal (\n"
                            "        hana :: at_key ( hana :: make_map ( p < 0 , 0 > ( ) ) , key < 0 > ( ) ) ,\n"
                            "        val < 0 > ( ) ) ) ;\n"
                            "}";
        const char exp[] = "auto key<0> ( ) ; "
                           "auto val<0> ( ) ; "
                           "auto p<0,0> ( ) ; "
                           "int main ( ) { "
                           "BOOST_HANA_CONSTANT_CHECK ( hana :: equal ( "
                           "hana :: at_key ( hana :: make_map ( p<0,0> ( ) ) , key<0> ( ) ) , "
                           "val<0> ( ) ) ) ; "
                           "} "
                           "auto p<0,0> ( ) { return :: minimal_product ( key<0> ( ) , val<0> ( ) ) ; } "
                           "auto val<0> ( ) { return hana :: test :: ct_eq < - 0 > { } ; } "
                           "auto key<0> ( ) { return hana :: test :: ct_eq < 0 > { } ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template105() { // #9076
        const char code[] = "template <template <typename> class TOUT> class ObjectCache;\n"
                            "template <template <typename> class TOUT>\n"
                            "class ObjectCache { };\n"
                            "template <typename T> class Fred {};\n"
                            "ObjectCache<Fred> _cache;";
        const char exp[] = "class ObjectCache<Fred> ; "
                           "template < typename T > class Fred { } ; "
                           "ObjectCache<Fred> _cache ; class ObjectCache<Fred> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template106() {
        const char code[] = "template<class T, class U> class A {\n"
                            "public:\n"
                            "   int x;\n"
                            "};\n"
                            "template<template<class T, class U> class V> class B {\n"
                            "   V<char, char> i;\n"
                            "};\n"
                            "B<A> c;";
        const char exp[] = "class A<char,char> ; "
                           "class B<A> ; "
                           "B<A> c ; "
                           "class B<A> { "
                           "A<char,char> i ; "
                           "} ; class A<char,char> { "
                           "public: "
                           "int x ; "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template107() { // #8663
        const char code[] = "template <class T1, class T2>\n"
                            "void f() {\n"
                            "  using T3 = typename T1::template T3<T2>;\n"
                            "  T3 t;\n"
                            "}\n"
                            "struct C3 {\n"
                            "  template <typename T>\n"
                            "  class T3\n"
                            "  {};\n"
                            "};\n"
                            "void foo() {\n"
                            "  f<C3, long>();\n"
                            "}";
        const char exp[] = "void f<C3,long> ( ) ; "
                           "struct C3 { "
                           "class T3<long> ; "
                           "} ; "
                           "void foo ( ) { "
                           "f<C3,long> ( ) ; "
                           "} "
                           "void f<C3,long> ( ) { "
                           "C3 :: T3<long> t ; "
                           "} "
                           "class C3 :: T3<long> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template108() { // #9109
        {
            const char code[] = "template <typename> struct a;\n"
                                "template <typename> struct b {};\n"
                                "template <typename> struct c;\n"
                                "template <typename d> struct e {\n"
                                "  using f = a<b<typename c<d>::g>>;\n"
                                "  bool h = f::h;\n"
                                "};\n"
                                "struct i {\n"
                                "  e<int> j();\n"
                                "};\n";
            const char exp[] = "template < typename > struct a ; "
                               "struct b<c<int>::g> ; "
                               "template < typename > struct c ; "
                               "struct e<int> ; "
                               "struct i { e<int> j ( ) ; "
                               "} ; "
                               "struct e<int> { bool h ; "
                               "h = a < b<c<int>::g> > :: h ; "
                               "} ; "
                               "struct b<c<int>::g> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "namespace {\n"
                                "template <typename> struct a;\n"
                                "template <typename> struct b {};\n"
                                "}\n"
                                "namespace {\n"
                                "template <typename> struct c;\n"
                                "template <typename d> struct e {\n"
                                "  using f = a<b<typename c<d>::g>>;\n"
                                "  bool h = f::h;\n"
                                "};\n"
                                "template <typename i> using j = typename e<i>::g;\n"
                                "}";
            const char exp[] = "namespace { "
                               "template < typename > struct a ; "
                               "template < typename > struct b { } ; "
                               "} "
                               "namespace { "
                               "template < typename > struct c ; "
                               "template < typename d > struct e { "
                               "using f = a < b < c < d > :: g > > ; "
                               "bool h ; h = f :: h ; "
                               "} ; "
                               "template < typename i > using j = typename e < i > :: g ; "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "namespace {\n"
                                "template <typename> struct a;\n"
                                "template <typename> struct b {};\n"
                                "}\n"
                                "namespace {\n"
                                "template <typename> struct c;\n"
                                "template <typename d> struct e {\n"
                                "  using f = a<b<typename c<d>::g>>;\n"
                                "  bool h = f::h;\n"
                                "};\n"
                                "template <typename i> using j = typename e<i>::g;\n"
                                "}\n"
                                "j<int> foo;";
            const char exp[] = "namespace { "
                               "template < typename > struct a ; "
                               "struct b<c<int>::g> ; "
                               "} "
                               "namespace { "
                               "template < typename > struct c ; "
                               "struct e<int> ; "
                               "} "
                               "e<int> :: g foo ; "
                               "struct e<int> { "
                               "bool h ; h = a < b<c<int>::g> > :: h ; "
                               "} ; "
                               "struct b<c<int>::g> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template109() { // #9144
        {
            const char code[] = "namespace a {\n"
                                "template <typename b, bool = __is_empty(b) && __is_final(b)> struct c;\n"
                                "}\n"
                                "template <typename...> struct e {};\n"
                                "static_assert(sizeof(e<>) == sizeof(e<c<int>, c<int>, int>), \"\");\n";
            const char exp[] = "namespace a { "
                               "template < typename b , bool > struct c ; "
                               "} "
                               "struct e<> ; "
                               "struct e<c<int>,c<int>,int> ; "
                               "static_assert ( sizeof ( e<> ) == sizeof ( e<c<int>,c<int>,int> ) , \"\" ) ; "
                               "struct e<> { } ; "
                               "struct e<c<int>,c<int>,int> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "namespace a {\n"
                                "template <typename b, bool = __is_empty(b) && __is_final(b)> struct c;\n"
                                "}\n"
                                "template <typename...> struct e {};\n"
                                "static_assert(sizeof(e<>) == sizeof(e<a::c<int>, a::c<int>, int>), \"\");\n";
            const char exp[] = "namespace a { "
                               "template < typename b , bool > struct c ; "
                               "} "
                               "struct e<> ; "
                               "struct e<a::c<int,std::is_empty<int>{}&&std::is_final<int>{}>,a::c<int,std::is_empty<int>{}&&std::is_final<int>{}>,int> ; "
                               "static_assert ( sizeof ( e<> ) == sizeof ( e<a::c<int,std::is_empty<int>{}&&std::is_final<int>{}>,a::c<int,std::is_empty<int>{}&&std::is_final<int>{}>,int> ) , \"\" ) ; "
                               "struct e<> { } ; "
                               "struct e<a::c<int,std::is_empty<int>{}&&std::is_final<int>{}>,a::c<int,std::is_empty<int>{}&&std::is_final<int>{}>,int> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <typename b, bool = __is_empty(b) && __is_final(b)> struct c;\n"
                                "template <typename...> struct e {};\n"
                                "static_assert(sizeof(e<>) == sizeof(e<c<int>, c<int>, int>), \"\");\n";
            const char exp[] = "template < typename b , bool > struct c ; "
                               "struct e<> ; "
                               "struct e<c<int,std::is_empty<int>{}&&std::is_final<int>{}>,c<int,std::is_empty<int>{}&&std::is_final<int>{}>,int> ; "
                               "static_assert ( sizeof ( e<> ) == sizeof ( e<c<int,std::is_empty<int>{}&&std::is_final<int>{}>,c<int,std::is_empty<int>{}&&std::is_final<int>{}>,int> ) , \"\" ) ; "
                               "struct e<> { } ; "
                               "struct e<c<int,std::is_empty<int>{}&&std::is_final<int>{}>,c<int,std::is_empty<int>{}&&std::is_final<int>{}>,int> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <typename b, bool = __is_empty(b) && __is_final(b)> struct c{};\n"
                                "c<int> cc;\n";
            const char exp[] = "struct c<int,std::is_empty<int>{}&&std::is_final<int>{}> ; "
                               "c<int,std::is_empty<int>{}&&std::is_final<int>{}> cc ; "
                               "struct c<int,std::is_empty<int>{}&&std::is_final<int>{}> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <typename b, bool = unknown1(b) && unknown2(b)> struct c{};\n"
                                "c<int> cc;\n";
            const char exp[] = "struct c<int,unknown1(int)&&unknown2(int)> ; "
                               "c<int,unknown1(int)&&unknown2(int)> cc ; "
                               "struct c<int,unknown1(int)&&unknown2(int)> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template110() {
        const char code[] = "template<typename T> using A = int;\n"
                            "template<typename T> using A<T*> = char;\n"
                            "template<> using A<char> = char;\n"
                            "template using A<char> = char;\n"
                            "using A<char> = char;";
        const char exp[] = "template < typename T > using A = int ; "
                           "template < typename T > using A < T * > = char ; "
                           "template < > using A < char > = char ; "
                           "template using A < char > = char ; "
                           "using A < char > = char ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template111() { // crash
        const char code[] = "template<typename T, typename U> struct pair;\n"
                            "template<typename T> using cell = pair<T*, cell<T>*>;";
        const char exp[] = "template < typename T , typename U > struct pair ; "
                           "template < typename T > using cell = pair < T * , cell < T > * > ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template112() { // #9146 syntax error
        const char code[] = "template <int> struct a;\n"
                            "template <class, class b> using c = typename a<int{b::d}>::e;\n"
                            "template <class> struct f;\n"
                            "template <class b> using g = typename f<c<int, b>>::e;";
        const char exp[] = "template < int > struct a ; "
                           "template < class , class b > using c = typename a < int { b :: d } > :: e ; "
                           "template < class > struct f ; "
                           "template < class b > using g = typename f < c < int , b > > :: e ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template113() {
        {
            const char code[] = "template <class> class A { void f(); };\n"
                                "A<int> a;";
            const char exp[] = "class A<int> ; "
                               "A<int> a ; "
                               "class A<int> { void f ( ) ; } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <struct> struct A { void f(); };\n"
                                "A<int> a;";
            const char exp[] = "struct A<int> ; "
                               "A<int> a ; "
                               "struct A<int> { void f ( ) ; } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template114() { // #9155
        {
            const char code[] = "template <typename a, a> struct b {};\n"
                                "template <typename> struct c;\n"
                                "template <typename> struct d : b<bool, std::is_polymorphic<int>{}> {};\n"
                                "template <bool> struct e;\n"
                                "template <typename a> using f = typename e<c<d<a>>::g>::h;";
            const char exp[] =  "template < typename a , a > struct b { } ; "
                               "template < typename > struct c ; "
                               "template < typename > struct d : b < bool , std :: is_polymorphic < int > { } > { } ; "
                               "template < bool > struct e ; "
                               "template < typename a > using f = typename e < c < d < a > > :: g > :: h ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <typename a, a> struct b;\n"
                                "template <bool, typename> struct c;\n"
                                "template <typename a> struct d : b<bool, std::is_empty<a>{}> {};\n"
                                "template <typename a> using e = typename c<std::is_final<a>{}, d<a>>::f;\n";
            const char exp[] =  "template < typename a , a > struct b ; "
                               "template < bool , typename > struct c ; "
                               "template < typename a > struct d : b < bool , std :: is_empty < a > { } > { } ; "
                               "template < typename a > using e = typename c < std :: is_final < a > { } , d < a > > :: f ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template115() { // #9153
        const char code[] = "namespace {\n"
                            "    namespace b {\n"
                            "        template <int c> struct B { using B<c / 2>::d; };\n"
                            "    }\n"
                            "    template <class, class> using e = typename b::B<int{}>;\n"
                            "    namespace b {\n"
                            "        template <class> struct f {};\n"
                            "    }\n"
                            "    template <class c> using g = b::f<e<int, c>>;\n"
                            "}\n"
                            "g<int> g1;";
        const char exp[] = "namespace { "
                           "namespace b { "
                           "struct B<0> ; "
                           "} "
                           "namespace b { "
                           "struct f<b::B<0>> ; "
                           "} "
                           "} "
                           "b :: f<b::B<0>> g1 ; struct b :: B<0> { using d = B<0> :: d ; } ; "
                           "struct b :: f<b::B<0>> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template116() { // #9178
        {
            const char code[] = "template <class, class a> auto b() -> decltype(a{}.template b<void(int, int)>);\n"
                                "template <class, class a> auto b() -> decltype(a{}.template b<void(int, int)>){}";
            const char exp[] = "template < class , class a > auto b ( ) . decltype ( a { } . template b < void ( int , int ) > ) ; "
                               "template < class , class a > auto b ( ) . decltype ( a { } . template b < void ( int , int ) > ) { }";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <class, class a>\n"
                                "auto b() -> decltype(a{}.template b<void(int, int)>()) {}\n"
                                "struct c {\n"
                                "  template <class> void b();\n"
                                "};\n"
                                "void d() { b<c, c>(); }";
            const char exp[] = "auto b<c,c> ( ) . decltype ( c { } . template b < void ( int , int ) > ( ) ) ; "
                               "struct c { "
                               "template < class > void b ( ) ; "
                               "} ; "
                               "void d ( ) { b<c,c> ( ) ; } "
                               "auto b<c,c> ( ) . decltype ( c { } . template b < void ( int , int ) > ( ) ) { }";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template117() {
        const char code[] = "template<typename T = void> struct X {};\n"
                            "X<X<>> x;";
        const char exp[] = "struct X<void> ; "
                           "struct X<X<void>> ; "
                           "X<X<void>> x ; "
                           "struct X<void> { } ; "
                           "struct X<X<void>> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template118() {
        const char code[] = "template<int> struct S { void f(int i); };\n"
                            "S<1> s;";
        const char exp[] = "struct S<1> ; "
                           "S<1> s ; struct S<1> { "
                           "void f ( int i ) ; "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template119() { // #9186
        {
            const char code[] = "template <typename T>\n"
                                "constexpr auto func = [](auto x){ return T(x);};\n"
                                "template <typename T>\n"
                                "constexpr auto funcBraced = [](auto x){ return T{x};};\n"
                                "double f(int x) { return func<double>(x); }\n"
                                "double fBraced(int x) { return funcBraced<int>(x); }";
            const char exp[] = "constexpr auto func<double> = [ ] ( auto x ) { return double ( x ) ; } ; "
                               "constexpr auto funcBraced<int> = [ ] ( auto x ) { return int { x } ; } ; "
                               "double f ( int x ) { return func<double> ( x ) ; } "
                               "double fBraced ( int x ) { return funcBraced<int> ( x ) ; }";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <typename T>\n"
                                "constexpr auto func = [](auto x){ return T(x);};\n"
                                "void foo() {\n"
                                "    func<int>(x);\n"
                                "    func<double>(x);\n"
                                "}";
            const char exp[] = "constexpr auto func<int> = [ ] ( auto x ) { return int ( x ) ; } ; "
                               "constexpr auto func<double> = [ ] ( auto x ) { return double ( x ) ; } ; "
                               "void foo ( ) { "
                               "func<int> ( x ) ; "
                               "func<double> ( x ) ; "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template120() {
        const char code[] = "template<typename Tuple>\n"
                            "struct lambda_context {\n"
                            "    template<typename Sig> struct result;\n"
                            "    template<typename This, typename I>\n"
                            "    struct result<This(terminal, placeholder)> : at<Tuple, I> {};\n"
                            "};\n"
                            "template<typename T>\n"
                            "struct lambda {\n"
                            "    template<typename Sig> struct result;\n"
                            "    template<typename This>\n"
                            "    struct result<This()> : lambda_context<tuple<> > {};\n"
                            "};\n"
                            "lambda<int> l;";
        const char exp[] = "template < typename Tuple > "
                           "struct lambda_context { "
                           "template < typename Sig > struct result ; "
                           "template < typename This , typename I > "
                           "struct result < This ( terminal , placeholder ) > : at < Tuple , I > { } ; "
                           "} ; "
                           "struct lambda<int> ; "
                           "lambda<int> l ; struct lambda<int> { "
                           "template < typename Sig > struct result ; "
                           "template < typename This > "
                           "struct result < This ( ) > : lambda_context < tuple < > > { } ; "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template121() { // #9193
        const char code[] = "template <class VALUE_T, class LIST_T = std::list<VALUE_T>>\n"
                            "class TestList { };\n"
                            "TestList<std::shared_ptr<int>> m_test;";
        const char exp[] = "class TestList<std::shared_ptr<int>,std::list<std::shared_ptr<int>>> ; "
                           "TestList<std::shared_ptr<int>,std::list<std::shared_ptr<int>>> m_test ; "
                           "class TestList<std::shared_ptr<int>,std::list<std::shared_ptr<int>>> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template122() { // #9147
        const char code[] = "template <class...> struct a;\n"
                            "namespace {\n"
                            "template <class, class> struct b;\n"
                            "template <template <class> class c, class... f, template <class...> class d>\n"
                            "struct b<c<f...>, d<>>;\n"
                            "}\n"
                            "void e() { using c = a<>; }";
        const char exp[] = "template < class ... > struct a ; "
                           "namespace { "
                           "template < class , class > struct b ; "
                           "template < template < class > class c , class ... f , template < class ... > class d > "
                           "struct b < c < f ... > , d < > > ; "
                           "} "
                           "void e ( ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template123() { // #9183
        const char code[] = "template <class...> struct a;\n"
                            "namespace {\n"
                            "template <class, class, class, class>\n"
                            "struct b;\n"
                            "template <template <class> class c, class... d, template <class> class e, class... f>\n"
                            "struct b<c<d...>, e<f...>>;\n"
                            "}\n"
                            "void fn1() {\n"
                            "  using c = a<>;\n"
                            "  using e = a<>;\n"
                            "}";
        const char exp[] = "template < class ... > struct a ; "
                           "namespace { "
                           "template < class , class , class , class > "
                           "struct b ; "
                           "template < template < class > class c , class ... d , template < class > class e , class ... f > "
                           "struct b < c < d ... > , e < f ... > > ; "
                           "} "
                           "void fn1 ( ) { "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template124() { // #9197
        const char code[] = "template <bool> struct a;\n"
                            "template <bool b> using c = typename a<b>::d;\n"
                            "template <typename> struct e;\n"
                            "template <typename> struct h {\n"
                            "  template <typename... f, c<h<e<typename f::d...>>::g>> void i();\n"
                            "};";
        const char exp[] = "template < bool > struct a ; "
                           "template < bool b > using c = typename a < b > :: d ; "
                           "template < typename > struct e ; "
                           "template < typename > struct h { "
                           "template < typename ... f , c < h < e < typename f :: d ... > > :: g > > void i ( ) ; "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template125() {
        ASSERT_THROW_INTERNAL(tok("template<int M, int N>\n"
                                  "class GCD {\n"
                                  "public:\n"
                                  "  enum { val = (N == 0) ? M : GCD<N, M % N>::val };\n"
                                  "};\n"
                                  "int main() {\n"
                                  "  GCD< 1, 0 >::val;\n"
                                  "}"), INSTANTIATION);
    }

    void template126() { // #9217
        const char code[] = "template <typename b> using d = a<b>;\n"
                            "static_assert(i<d<l<b>>>{}, \"\");";
        const char exp[] = "static_assert ( i < a < l < b > > > { } , \"\" ) ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template127() { // #9225
        {
            const char code[] = "template <typename> struct a {\n"
                                "  template <typename b> constexpr decltype(auto) operator()(b &&) const;\n"
                                "};\n"
                                "a<int> c;\n"
                                "template <typename d>\n"
                                "template <typename b>\n"
                                "constexpr decltype(auto) a<d>::operator()(b &&) const {}";
            const char exp[] = "struct a<int> ; "
                               "a<int> c ; "
                               "template < typename d > "
                               "template < typename b > "
                               "constexpr decltype ( auto ) a < d > :: operator() ( b && ) const { } "
                               "struct a<int> { "
                               "template < typename b > constexpr decltype ( auto ) operator() ( b && ) const ; "
                               "} ;";
            const char act[] = "struct a<int> ; "
                               "a<int> c ; "
                               "template < typename d > "
                               "template < typename b > "
                               "constexpr decltype ( auto ) a < d > :: operator() ( b && ) const { } "
                               "struct a<int> { "
                               "template < typename b > constexpr decltype ( auto ) operator() ( b && ) const ; "
                               "} ; "
                               "constexpr decltype ( auto ) a<int> :: operator() ( b && ) const { }";
            TODO_ASSERT_EQUALS(exp, act, tok(code));
        }
        {
            const char code[] = "template <typename> struct a {\n"
                                "  template <typename b> static void foo();\n"
                                "};\n"
                                "a<int> c;\n"
                                "template <typename d>\n"
                                "template <typename b>\n"
                                "void a<d>::foo() {}\n"
                                "void bar() { a<int>::foo<char>(); }";
            const char exp[] = "struct a<int> ; "
                               "a<int> c ; "
                               "template < typename d > "
                               "template < typename b > "
                               "void a < d > :: foo ( ) { } "
                               "void bar ( ) { a<int> :: foo < char > ( ) ; } "
                               "struct a<int> { "
                               "template < typename b > static void foo ( ) ; "
                               "static void foo<char> ( ) ; "
                               "} ; "
                               "void a<int> :: foo<char> ( ) { }";
            const char act[] = "struct a<int> ; "
                               "a<int> c ; "
                               "template < typename d > "
                               "template < typename b > "
                               "void a < d > :: foo ( ) { } "
                               "void bar ( ) { a<int> :: foo < char > ( ) ; } "
                               "struct a<int> { "
                               "template < typename b > static void foo ( ) ; "
                               "} ; "
                               "void a<int> :: foo ( ) { }";
            TODO_ASSERT_EQUALS(exp, act, tok(code));
        }
        {
            const char code[] = "template <typename> struct a {\n"
                                "  template <typename b> static void foo();\n"
                                "};\n"
                                "template <typename d>\n"
                                "template <typename b>\n"
                                "void a<d>::foo() {}\n"
                                "void bar() { a<int>::foo<char>(); }";
            const char exp[] = "struct a<int> ; "
                               "template < typename d > "
                               "template < typename b > "
                               "void a < d > :: foo ( ) { } "
                               "void bar ( ) { a<int> :: foo < char > ( ) ; } "
                               "struct a<int> { "
                               "static void foo<char> ( ) ; "
                               "} ; "
                               "void a<int> :: foo<char> ( ) { }";
            const char act[] = "struct a<int> ; "
                               "template < typename d > "
                               "template < typename b > "
                               "void a < d > :: foo ( ) { } "
                               "void bar ( ) { a<int> :: foo < char > ( ) ; } "
                               "struct a<int> { "
                               "template < typename b > static void foo ( ) ; "
                               "} ; "
                               "void a<int> :: foo ( ) { }";
            TODO_ASSERT_EQUALS(exp, act, tok(code));
        }
    }

    void template128() { // #9224
        const char code[] = "template <typename> struct a { };\n"
                            "template <typename j> void h() { k.h<a<j>>; }\n"
                            "void foo() { h<int>(); }";
        const char exp[] = "struct a<int> ; "
                           "void h<int> ( ) ; "
                           "void foo ( ) { h<int> ( ) ; } "
                           "void h<int> ( ) { k . h < a<int> > ; } "
                           "struct a<int> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template129() {
        const char code[] = "class LuaContext {\n"
                            "public:\n"
                            "  template <typename TFunctionType, typename TType>\n"
                            "  void registerFunction(TType fn) { }\n"
                            "};\n"
                            "void setupLuaBindingsDNSQuestion() {\n"
                            "  g_lua.registerFunction<void (DNSQuestion ::*)(std ::string, std ::string)>();\n"
                            "}";
        const char exp[] = "class LuaContext { "
                           "public: "
                           "template < typename TFunctionType , typename TType > "
                           "void registerFunction ( TType fn ) { } "
                           "} ; "
                           "void setupLuaBindingsDNSQuestion ( ) { "
                           "g_lua . registerFunction < void ( DNSQuestion :: * ) ( std :: string , std :: string ) > ( ) ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template130() { // #9246
        const char code[] = "template <typename...> using a = int;\n"
                            "template <typename, typename> using b = a<>;\n"
                            "template <typename, typename> void c();\n"
                            "template <typename d, typename> void e() { c<b<d, int>, int>; }\n"
                            "void f() { e<int(int, ...), int>(); }";
        const char exp[] = "template < typename , typename > void c ( ) ; "
                           "void e<int(int,...),int> ( ) ; "
                           "void f ( ) { e<int(int,...),int> ( ) ; } "
                           "void e<int(int,...),int> ( ) { c < int , int > ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template131() { // #9249
        {
            const char code[] = "template <long a, bool = 0 == a> struct b {};\n"
                                "b<1> b1;";
            const char exp[] = "struct b<1,false> ; "
                               "b<1,false> b1 ; "
                               "struct b<1,false> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <long a, bool = 0 != a> struct b {};\n"
                                "b<1> b1;";
            const char exp[] = "struct b<1,true> ; "
                               "b<1,true> b1 ; "
                               "struct b<1,true> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <long a, bool = a < 0> struct b {};\n"
                                "b<1> b1;";
            const char exp[] = "struct b<1,false> ; "
                               "b<1,false> b1 ; "
                               "struct b<1,false> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <long a, bool = 0 < a> struct b {};\n"
                                "b<1> b1;";
            const char exp[] = "struct b<1,true> ; "
                               "b<1,true> b1 ; "
                               "struct b<1,true> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <long a, bool = 0 <= a> struct b {};\n"
                                "b<1> b1;";
            const char exp[] = "struct b<1,true> ; "
                               "b<1,true> b1 ; "
                               "struct b<1,true> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <long a, bool = a >= 0> struct b {};\n"
                                "b<1> b1;";
            const char exp[] = "struct b<1,true> ; "
                               "b<1,true> b1 ; "
                               "struct b<1,true> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template132() { // #9250
        const char code[] = "struct TrueFalse {\n"
                            "  static constexpr bool v() { return true; }\n"
                            "};\n"
                            "int global;\n"
                            "template<typename T> int foo() {\n"
                            "  __transaction_atomic noexcept(T::v()) { global += 1; }\n"
                            "  return __transaction_atomic noexcept(T::v()) (global + 2);\n"
                            "}\n"
                            "int f1() {\n"
                            "  return foo<TrueFalse>();\n"
                            "}";
        const char exp[] = "struct TrueFalse { "
                           "static constexpr bool v ( ) { return true ; } "
                           "} ; "
                           "int global ; "
                           "int foo<TrueFalse> ( ) ; "
                           "int f1 ( ) { "
                           "return foo<TrueFalse> ( ) ; "
                           "} "
                           "int foo<TrueFalse> ( ) { "
                           "__transaction_atomic noexcept ( TrueFalse :: v ( ) ) { global += 1 ; } "
                           "return __transaction_atomic noexcept ( TrueFalse :: v ( ) ) ( global + 2 ) ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template133() {
        const char code[] = "template <typename a> struct bar {\n"
                            "  template <typename b> static bar foo(const bar<b> &c) {\n"
                            "    return bar();\n"
                            "  }\n"
                            "};\n"
                            "bar<short> bs;\n"
                            "bar<std::array<int,4>> ba;\n"
                            "bar<short> b1 = bar<short>::foo<std::array<int,4>>(ba);\n"
                            "bar<std::array<int,4>> b2 = bar<std::array<int,4>>::foo<short>(bs);";
        const char act[] = "struct bar<short> ; struct bar<std::array<int,4>> ; "
                           "bar<short> bs ; "
                           "bar<std::array<int,4>> ba ; "
                           "bar<short> b1 ; b1 = bar<short> :: foo<std::array<int,4>> ( ba ) ; "
                           "bar<std::array<int,4>> b2 ; b2 = bar<std::array<int,4>> :: foo<short> ( bs ) ; "
                           "struct bar<short> { "
                           "static bar<short> foo<std::array<int,4>> ( const bar < std :: array < int , 4 > > & c ) ; "
                           "} ; "
                           "struct bar<std::array<int,4>> { "
                           "static bar<std::array<int,4>> foo<short> ( const bar < short > & c ) ; "
                           "} ; "
                           "bar<std::array<int,4>> bar<std::array<int,4>> :: foo<short> ( const bar < short > & c ) { "
                           "return bar<std::array<int,4>> ( ) ; "
                           "} "
                           "bar<short> bar<short> :: foo<std::array<int,4>> ( const bar < std :: array < int , 4 > > & c ) { "
                           "return bar<short> ( ) ; "
                           "}";
        const char exp[] = "struct bar<short> ; struct bar<std::array<int,4>> ; "
                           "bar<short> bs ; "
                           "bar<std::array<int,4>> ba ; "
                           "bar<short> b1 ; b1 = bar<short> :: foo<std::array<int,4>> ( ba ) ; "
                           "bar<std::array<int,4>> b2 ; b2 = bar<std::array<int,4>> :: foo<short> ( bs ) ; "
                           "struct bar<short> { "
                           "static bar<short> foo<std::array<int,4>> ( const bar<std::array<int,4>> & c ) ; "
                           "} ; "
                           "struct bar<std::array<int,4>> { "
                           "static bar<std::array<int,4>> foo<short> ( const bar<short> & c ) ; "
                           "} ; "
                           "bar<std::array<int,4>> bar<std::array<int,4>> :: foo<short> ( const bar<short> & c ) { "
                           "return bar<std::array<int,4>> ( ) ; "
                           "} "
                           "bar<short> bar<short> :: foo<std::array<int,4>> ( const bar<std::array<int,4>> & c ) { "
                           "return bar<short> ( ) ; "
                           "}";
        TODO_ASSERT_EQUALS(exp, act, tok(code));
    }

    void template134() {
        const char code[] = "template <int a> class e { };\n"
                            "template <int a> class b { e<(c > a ? 1 : 0)> d; };\n"
                            "b<0> b0;\n"
                            "b<1> b1;";
        const char exp[] = "class e<(c>0)> ; class e<(c>1)> ; "
                           "class b<0> ; class b<1> ; "
                           "b<0> b0 ; "
                           "b<1> b1 ; "
                           "class b<0> { e<(c>0)> d ; } ; class b<1> { e<(c>1)> d ; } ; "
                           "class e<(c>0)> { } ; class e<(c>1)> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template135() {
        const char code[] = "template <int> struct a { template <int b> void c(a<b>); };\n"
                            "a<2> d;";
        const char exp[] = "struct a<2> ; "
                           "a<2> d ; "
                           "struct a<2> { template < int b > void c ( a < b > ) ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template136() { // #9287
        const char code[] = "namespace a {\n"
                            "template <typename> struct b;\n"
                            "template <int> struct c;\n"
                            "template <typename> struct d;\n"
                            "template <typename> struct f;\n"
                            "template <typename> struct g;\n"
                            "template <typename h>\n"
                            "struct i : c<b<f<typename h ::j>>::k && b<g<typename h ::j>>::k> {};\n"
                            "}\n"
                            "namespace hana = a;\n"
                            "using e = int;\n"
                            "void l(hana::d<hana::i<e>>);";
        const char exp[] = "namespace a { "
                           "template < typename > struct b ; "
                           "template < int > struct c ; "
                           "template < typename > struct d ; "
                           "template < typename > struct f ; "
                           "template < typename > struct g ; "
                           "struct i<int> ; "
                           "} "
                           "void l ( a :: d < a :: i<int> > ) ; "
                           "struct a :: i<int> : c < b < f < int :: j > > :: k && b < g < int :: j > > :: k > { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template137() { // #9288
        const char code[] = "template <bool> struct a;\n"
                            "template <bool b, class> using c = typename a<b>::d;\n"
                            "template <class, template <class> class, class> struct e;\n"
                            "template <class f, class g, class... h>\n"
                            "using i = typename e<f, g::template fn, h...>::d;\n"
                            "template <class... j> struct k : c<sizeof...(j), int>::template fn<j...> {};";
        const char exp[] = "template < bool > struct a ; "
                           "template < bool b , class > using c = typename a < b > :: d ; "
                           "template < class , template < class > class , class > struct e ; "
                           "template < class f , class g , class ... h > "
                           "using i = typename e < f , g :: fn , h ... > :: d ; "
                           "template < class ... j > struct k : c < sizeof... ( j ) , int > :: fn < j ... > { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template138() {
        {
            const char code[] = "struct inferior {\n"
                                "  using visitor = int;\n"
                                "  template <typename T>\n"
                                "  bool operator()(const T &a, const T &b) const {\n"
                                "    return 1 < b;\n"
                                "  }\n"
                                "};\n"
                                "int main() {\n"
                                "  return 0;\n"
                                "}";
            const char exp[] = "struct inferior { "
                               "template < typename T > "
                               "bool operator() ( const T & a , const T & b ) const { "
                               "return 1 < b ; "
                               "} "
                               "} ; "
                               "int main ( ) { "
                               "return 0 ; "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "struct inferior {\n"
                                "  template <typename T>\n"
                                "  bool operator()(const T &a, const T &b) const {\n"
                                "    return 1 < b;\n"
                                "  }\n"
                                "};\n"
                                "int main() {\n"
                                "  return 0;\n"
                                "}";
            const char exp[] = "struct inferior { "
                               "template < typename T > "
                               "bool operator() ( const T & a , const T & b ) const { "
                               "return 1 < b ; "
                               "} "
                               "} ; "
                               "int main ( ) { "
                               "return 0 ; "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "struct inferior {\n"
                                "  using visitor = int;\n"
                                "  template <typename T>\n"
                                "  bool operator()(const T &a, const T &b) const {\n"
                                "    return a < b;\n"
                                "  }\n"
                                "};\n"
                                "int main() {\n"
                                "  return 0;\n"
                                "}";
            const char exp[] = "struct inferior { "
                               "template < typename T > "
                               "bool operator() ( const T & a , const T & b ) const { "
                               "return a < b ; "
                               "} "
                               "} ; "
                               "int main ( ) { "
                               "return 0 ; "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "struct inferior {\n"
                                "  template <typename T>\n"
                                "  bool operator()(const T &a, const T &b) const {\n"
                                "    return a < b;\n"
                                "  }\n"
                                "};\n"
                                "int main() {\n"
                                "  return 0;\n"
                                "}";
            const char exp[] = "struct inferior { "
                               "template < typename T > "
                               "bool operator() ( const T & a , const T & b ) const { "
                               "return a < b ; "
                               "} "
                               "} ; "
                               "int main ( ) { "
                               "return 0 ; "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template139() {
        {
            const char code[] = "template<typename T>\n"
                                "struct Foo {\n"
                                "  template<typename> friend struct Foo;\n"
                                "};";
            const char exp[] = "template < typename T > "
                               "struct Foo { "
                               "template < typename > friend struct Foo ; "
                               "} ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template<typename T>\n"
                                "struct Foo {\n"
                                "  template<typename> friend struct Foo;\n"
                                "} ;\n"
                                "Foo<int> foo;";
            const char exp[] = "struct Foo<int> ; "
                               "Foo<int> foo ; "
                               "struct Foo<int> { "
                               "template < typename > friend struct Foo ; "
                               "} ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template140() {
        {
            const char code[] = "template <typename> struct a { };\n"
                                "template <typename b> struct d {\n"
                                "    d();\n"
                                "    d(d<a<b>> e);\n"
                                "};\n"
                                "void foo() { d<char> c; }";
            const char exp[] = "struct a<char> ; "
                               "struct d<char> ; "
                               "void foo ( ) { d<char> c ; } "
                               "struct d<char> { "
                               "d<char> ( ) ; "
                               "d<char> ( d < a<char> > e ) ; "
                               "} ; "
                               "struct a<char> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "namespace a {\n"
                                "template <typename b> using c = typename b ::d;\n"
                                "template <typename> constexpr bool e() { return false; }\n"
                                "template <typename b> class f { f(f<c<b>>); };\n"
                                "static_assert(!e<f<char>>());\n"
                                "}";
            const char exp[] = "namespace a { "
                               "constexpr bool e<f<char>> ( ) ; "
                               "class f<char> ; "
                               "static_assert ( ! e<f<char>> ( ) ) ; } "
                               "class a :: f<char> { f<char> ( a :: f < b :: d > ) ; } ; "
                               "constexpr bool a :: e<f<char>> ( ) { return false ; }";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template141() { // #9337
        const char code[] = "struct a {\n"
                            "  int c;\n"
                            "  template <typename b> void d(b e) const { c < *e; }\n"
                            "};";
        const char exp[] = "struct a { "
                           "int c ; "
                           "template < typename b > void d ( b e ) const { c < * e ; } "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template142() { // #9338
        const char code[] = "template <typename...> struct a;\n"
                            "template <typename b, typename c, typename... d> struct a<b c::*, d...> {\n"
                            "  using typename b ::e;\n"
                            "  static_assert(e::f ? sizeof...(d) : sizeof...(d), \"\");\n"
                            "};";
        const char exp[] = "template < typename ... > struct a ; "
                           "template < typename b , typename c , typename ... d > struct a < b c :: * , d ... > { "
                           "using e = b :: e ; "
                           "static_assert ( e :: f ? sizeof... ( d ) : sizeof... ( d ) , \"\" ) ; "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template143() {
        const char code[] = "template<int N>\n"
                            "using A1 = struct B1 { static auto constexpr value = N; };\n"
                            "A1<0> a1;\n"
                            "template<class T>\n"
                            "using A2 = struct B2 { void f(T){} };\n"
                            "A2<bool> a2;\n"
                            "template<class T>\n"
                            "using A3 = enum B3 {b = 0};\n"
                            "A3<int> a3;";
        const char exp[] = "template < int N > "
                           "using A1 = struct B1 { static auto constexpr value = N ; } ; "
                           "A1 < 0 > a1 ; "
                           "template < class T > "
                           "using A2 = struct B2 { void f ( T ) { } } ; "
                           "A2 < bool > a2 ; "
                           "template < class T > "
                           "using A3 = enum B3 { b = 0 } ; "
                           "A3 < int > a3 ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template144() { // #9046
        const char code[] = "namespace a {\n"
                            "template <typename T, typename enable = void>\n"
                            "struct promote {\n"
                            "  using type = T;\n"
                            "};\n"
                            "template <typename T>\n"
                            "struct promote <T, typename std::enable_if< std::is_integral<T>::value && sizeof(T) < sizeof(int) >::type>{\n"
                            "};\n"
                            "}";
        const char exp[] = "namespace a { "
                           "template < typename T , typename enable = void > "
                           "struct promote { "
                           "using type = T ; "
                           "} ; "
                           "template < typename T > "
                           "struct promote < T , std :: enable_if < std :: is_integral < T > :: value && sizeof ( T ) < sizeof ( int ) > :: type > { "
                           "} ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template145() { // syntax error
        const char code[] = "template<template<typename, Ts = 0> class ...Cs, Cs<Ts> ...Vs> struct B { };";
        const char exp[] = "template < template < typename , Ts = 0 > class ... Cs , Cs < Ts > ... Vs > struct B { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template146() { // syntax error
        const char code[] = "template<class T> struct C { };\n"
                            "template<class T, template<class TT_T0, template<class TT_T1> class TT_TT> class TT, class U = TT<int, C> >\n"
                            "struct S {\n"
                            "  void foo(TT<T, C>);\n"
                            "};";
        const char exp[] = "template < class T > struct C { } ; "
                           "template < class T , template < class TT_T0 , template < class TT_T1 > class TT_TT > class TT , class U = TT < int , C > > "
                           "struct S { "
                           "void foo ( TT < T , C > ) ; "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template147() { // syntax error
        const char code[] = "template <template <typename> class C, typename X, C<X>*> struct b { };";
        const char exp[] = "template < template < typename > class C , typename X , C < X > * > struct b { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template148() { // syntax error
        const char code[] = "static_assert(var<S1<11, 100>> == var<S1<199, 23>> / 2\n"
                            "  && var<S1<50, 120>> == var<S1<150, var<S1<10, 10>>>>\n"
                            "  && var<S1<53, 23>> != 222, \"\");";
        const char exp[] = "static_assert ( var < S1 < 11 , 100 > > == var < S1 < 199 , 23 > > / 2 "
                           "&& var < S1 < 50 , 120 > > == var < S1 < 150 , var < S1 < 10 , 10 > > > > "
                           "&& var < S1 < 53 , 23 > > != 222 , \"\" ) ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template149() { // unknown macro
        const char code[] = "BEGIN_VERSIONED_NAMESPACE_DECL\n"
                            "template<typename T> class Fred { };\n"
                            "END_VERSIONED_NAMESPACE_DECL";
        ASSERT_THROW_INTERNAL_EQUALS(tok(code), UNKNOWN_MACRO, "There is an unknown macro here somewhere. Configuration is required. If BEGIN_VERSIONED_NAMESPACE_DECL is a macro then please configure it.");
    }

    void template150() { // syntax error
        const char code[] = "struct Test {\n"
                            "  template <typename T>\n"
                            "  T &operator[] (T) {}\n"
                            "};\n"
                            "void foo() {\n"
                            "  Test test;\n"
                            "  const string type = test.operator[]<string>(\"type\");\n"
                            "}";
        const char exp[] = "struct Test { "
                           "string & operator[]<string> ( string ) ; "
                           "} ; "
                           "void foo ( ) { "
                           "Test test ; "
                           "const string type = test . operator[]<string> ( \"type\" ) ; "
                           "} string & Test :: operator[]<string> ( string ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template151() { // crash
        {
            const char code[] = "class SimulationComponentGroupGenerator {\n"
                                "  std::list<int, std::allocator<int>> build() const;\n"
                                "};\n"
                                "template <\n"
                                "  class obj_type,\n"
                                "  template<class> class allocator = std::allocator,\n"
                                "  template<class, class> class data_container = std::list>\n"
                                "class GenericConfigurationHandler {\n"
                                "  data_container<int, std::allocator<int>> m_target_configurations;\n"
                                "};\n"
                                "class TargetConfigurationHandler : public GenericConfigurationHandler<int> { };";
            const char exp[] = "class SimulationComponentGroupGenerator { "
                               "std :: list < int , std :: allocator < int > > build ( ) const ; "
                               "} ; "
                               "class GenericConfigurationHandler<int,std::allocator,std::list> ; "
                               "class TargetConfigurationHandler : public GenericConfigurationHandler<int,std::allocator,std::list> { } ; "
                               "class GenericConfigurationHandler<int,std::allocator,std::list> { "
                               "std :: list < int , std :: allocator < int > > m_target_configurations ; "
                               "} ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "std::list<std::allocator<int>> a;\n"
                                "template <class, template <class> class allocator = std::allocator> class b {};\n"
                                "class c : b<int> {};";
            const char exp[] = "std :: list < std :: allocator < int > > a ; "
                               "class b<int,std::allocator> ; "
                               "class c : b<int,std::allocator> { } ; "
                               "class b<int,std::allocator> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <typename> class a {};\n"
                                "template class a<char>;\n"
                                "template <class, template <class> class a = a> class b {};\n"
                                "class c : b<int> {};";
            const char exp[] = "class a<char> ; "
                               "class b<int,a> ; "
                               "class c : b<int,a> { } ; "
                               "class b<int,a> { } ; "
                               "class a<char> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template152() { // #9467
        const char code[] = "class Foo {\n"
                            "  template <unsigned int i>\n"
                            "  bool bar() {\n"
                            "    return true;\n"
                            "  }\n"
                            "};\n"
                            "template <>\n"
                            "bool Foo::bar<9>() {\n"
                            "  return true;\n"
                            "}\n"
                            "int global() {\n"
                            "  int bar = 1;\n"
                            "  return bar;\n"
                            "}";
        const char exp[] = "class Foo { "
                           "bool bar<9> ( ) ; "
                           "template < unsigned int i > "
                           "bool bar ( ) { "
                           "return true ; "
                           "} "
                           "} ; "
                           "bool Foo :: bar<9> ( ) { "
                           "return true ; "
                           "} "
                           "int global ( ) { "
                           "int bar ; bar = 1 ; "
                           "return bar ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template153() { // #9483
        const char code[] = "template <class = b<decltype(a<h>())...>> void i();";
        const char exp[] = "template < class = b < decltype ( a < h > ( ) ) ... > > void i ( ) ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template154() { // #9495
        const char code[] = "template <typename S, enable_if_t<(is_compile_string<S>::value), int>> void i(S s);";
        const char exp[] = "template < typename S , enable_if_t < ( is_compile_string < S > :: value ) , int > > void i ( S s ) ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template155() { // #9539
        const char code[] = "template <int> int a = 0;\n"
                            "struct b {\n"
                            "  void operator[](int);\n"
                            "};\n"
                            "void c() {\n"
                            "  b d;\n"
                            "  d[a<0>];\n"
                            "}";
        const char exp[] = "int a<0> ; "
                           "a<0> = 0 ; "
                           "struct b { "
                           "void operator[] ( int ) ; "
                           "} ; "
                           "void c ( ) { "
                           "b d ; "
                           "d [ a<0> ] ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template156() {
        const char code[] = "template <int a> struct c { static constexpr int d = a; };\n"
                            "template <bool b> using e = c<b>;\n"
                            "using f = e<false>;\n"
                            "template <typename> struct g : f {};\n"
                            "template <bool, class, class> using h = e<g<long>::d>;\n"
                            "template <typename> using i = e<g<double>::d>;\n"
                            "template <typename j> using k = e<i<j>::d>;\n"
                            "template <typename j> using l = h<k<j>::d, e<1 < (j)0>, f>;\n"
                            "template <typename> void m(int, int, int) { l<int> d; }\n"
                            "void n() { m<int>(0, 4, 5); }";
        (void)tok(code); // don't crash
    }

    void template157() { // #9854
        const char code[] = "template <int a, bool c = a == int()> struct b1 { bool d = c; };\n"
                            "template <int a, bool c = a != int()> struct b2 { bool d = c; };\n"
                            "template <int a, bool c = a < int()> struct b3 { bool d = c; };\n"
                            "template <int a, bool c = a <= int()> struct b4 { bool d = c; };\n"
                            "template <int a, bool c = (a > int())> struct b5 { bool d = c; };\n"
                            "template <int a, bool c = a >= int()> struct b6 { bool d = c; };\n"
                            "b1<0> var1;\n"
                            "b2<0> var2;\n"
                            "b3<0> var3;\n"
                            "b4<0> var4;\n"
                            "b5<0> var5;\n"
                            "b6<0> var6;";
        const char exp[] = "struct b1<0,true> ; "
                           "struct b2<0,false> ; "
                           "struct b3<0,false> ; "
                           "struct b4<0,true> ; "
                           "struct b5<0,false> ; "
                           "struct b6<0,true> ; "
                           "b1<0,true> var1 ; "
                           "b2<0,false> var2 ; "
                           "b3<0,false> var3 ; "
                           "b4<0,true> var4 ; "
                           "b5<0,false> var5 ; "
                           "b6<0,true> var6 ; "
                           "struct b6<0,true> { bool d ; d = true ; } ; "
                           "struct b5<0,false> { bool d ; d = false ; } ; "
                           "struct b4<0,true> { bool d ; d = true ; } ; "
                           "struct b3<0,false> { bool d ; d = false ; } ; "
                           "struct b2<0,false> { bool d ; d = false ; } ; "
                           "struct b1<0,true> { bool d ; d = true ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template158() { // daca crash
        const char code[] = "template <typename> class a0{};\n"
                            "template <typename> class a1{};\n"
                            "template <typename> class a2{};\n"
                            "template <typename> class a3{};\n"
                            "template <typename> class a4{};\n"
                            "template <typename> class a5{};\n"
                            "template <typename> class a6{};\n"
                            "template <typename> class a7{};\n"
                            "template <typename> class a8{};\n"
                            "template <typename> class a9{};\n"
                            "template <typename> class a10{};\n"
                            "template <typename> class a11{};\n"
                            "template <typename> class a12{};\n"
                            "template <typename> class a13{};\n"
                            "template <typename> class a14{};\n"
                            "template <typename> class a15{};\n"
                            "template <typename> class a16{};\n"
                            "template <typename> class a17{};\n"
                            "template <typename> class a18{};\n"
                            "template <typename> class a19{};\n"
                            "template <typename> class a20{};\n"
                            "template <typename> class a21{};\n"
                            "template <typename> class a22{};\n"
                            "template <typename> class a23{};\n"
                            "template <typename> class a24{};\n"
                            "template <typename> class a25{};\n"
                            "template <typename> class a26{};\n"
                            "template <typename> class a27{};\n"
                            "template <typename> class a28{};\n"
                            "template <typename> class a29{};\n"
                            "template <typename> class a30{};\n"
                            "template <typename> class a31{};\n"
                            "template <typename> class a32{};\n"
                            "template <typename> class a33{};\n"
                            "template <typename> class a34{};\n"
                            "template <typename> class a35{};\n"
                            "template <typename> class a36{};\n"
                            "template <typename> class a37{};\n"
                            "template <typename> class a38{};\n"
                            "template <typename> class a39{};\n"
                            "template <typename> class a40{};\n"
                            "template <typename> class a41{};\n"
                            "template <typename> class a42{};\n"
                            "template <typename> class a43{};\n"
                            "template <typename> class a44{};\n"
                            "template <typename> class a45{};\n"
                            "template <typename> class a46{};\n"
                            "template <typename> class a47{};\n"
                            "template <typename> class a48{};\n"
                            "template <typename> class a49{};\n"
                            "template <typename> class a50{};\n"
                            "template <typename> class a51{};\n"
                            "template <typename> class a52{};\n"
                            "template <typename> class a53{};\n"
                            "template <typename> class a54{};\n"
                            "template <typename> class a55{};\n"
                            "template <typename> class a56{};\n"
                            "template <typename> class a57{};\n"
                            "template <typename> class a58{};\n"
                            "template <typename> class a59{};\n"
                            "template <typename> class a60{};\n"
                            "template <typename> class a61{};\n"
                            "template <typename> class a62{};\n"
                            "template <typename> class a63{};\n"
                            "template <typename> class a64{};\n"
                            "template <typename> class a65{};\n"
                            "template <typename> class a66{};\n"
                            "template <typename> class a67{};\n"
                            "template <typename> class a68{};\n"
                            "template <typename> class a69{};\n"
                            "template <typename> class a70{};\n"
                            "template <typename> class a71{};\n"
                            "template <typename> class a72{};\n"
                            "template <typename> class a73{};\n"
                            "template <typename> class a74{};\n"
                            "template <typename> class a75{};\n"
                            "template <typename> class a76{};\n"
                            "template <typename> class a77{};\n"
                            "template <typename> class a78{};\n"
                            "template <typename> class a79{};\n"
                            "template <typename> class a80{};\n"
                            "template <typename> class a81{};\n"
                            "template <typename> class a82{};\n"
                            "template <typename> class a83{};\n"
                            "template <typename> class a84{};\n"
                            "template <typename> class a85{};\n"
                            "template <typename> class a86{};\n"
                            "template <typename> class a87{};\n"
                            "template <typename> class a88{};\n"
                            "template <typename> class a89{};\n"
                            "template <typename> class a90{};\n"
                            "template <typename> class a91{};\n"
                            "template <typename> class a92{};\n"
                            "template <typename> class a93{};\n"
                            "template <typename> class a94{};\n"
                            "template <typename> class a95{};\n"
                            "template <typename> class a96{};\n"
                            "template <typename> class a97{};\n"
                            "template <typename> class a98{};\n"
                            "template <typename> class a99{};\n"
                            "template <typename> class a100{};\n"
                            "template <typename> class b {};\n"
                            "b<a0<int>> d0;\n"
                            "b<a1<int>> d1;\n"
                            "b<a2<int>> d2;\n"
                            "b<a3<int>> d3;\n"
                            "b<a4<int>> d4;\n"
                            "b<a5<int>> d5;\n"
                            "b<a6<int>> d6;\n"
                            "b<a7<int>> d7;\n"
                            "b<a8<int>> d8;\n"
                            "b<a9<int>> d9;\n"
                            "b<a10<int>> d10;\n"
                            "b<a11<int>> d11;\n"
                            "b<a12<int>> d12;\n"
                            "b<a13<int>> d13;\n"
                            "b<a14<int>> d14;\n"
                            "b<a15<int>> d15;\n"
                            "b<a16<int>> d16;\n"
                            "b<a17<int>> d17;\n"
                            "b<a18<int>> d18;\n"
                            "b<a19<int>> d19;\n"
                            "b<a20<int>> d20;\n"
                            "b<a21<int>> d21;\n"
                            "b<a22<int>> d22;\n"
                            "b<a23<int>> d23;\n"
                            "b<a24<int>> d24;\n"
                            "b<a25<int>> d25;\n"
                            "b<a26<int>> d26;\n"
                            "b<a27<int>> d27;\n"
                            "b<a28<int>> d28;\n"
                            "b<a29<int>> d29;\n"
                            "b<a30<int>> d30;\n"
                            "b<a31<int>> d31;\n"
                            "b<a32<int>> d32;\n"
                            "b<a33<int>> d33;\n"
                            "b<a34<int>> d34;\n"
                            "b<a35<int>> d35;\n"
                            "b<a36<int>> d36;\n"
                            "b<a37<int>> d37;\n"
                            "b<a38<int>> d38;\n"
                            "b<a39<int>> d39;\n"
                            "b<a40<int>> d40;\n"
                            "b<a41<int>> d41;\n"
                            "b<a42<int>> d42;\n"
                            "b<a43<int>> d43;\n"
                            "b<a44<int>> d44;\n"
                            "b<a45<int>> d45;\n"
                            "b<a46<int>> d46;\n"
                            "b<a47<int>> d47;\n"
                            "b<a48<int>> d48;\n"
                            "b<a49<int>> d49;\n"
                            "b<a50<int>> d50;\n"
                            "b<a51<int>> d51;\n"
                            "b<a52<int>> d52;\n"
                            "b<a53<int>> d53;\n"
                            "b<a54<int>> d54;\n"
                            "b<a55<int>> d55;\n"
                            "b<a56<int>> d56;\n"
                            "b<a57<int>> d57;\n"
                            "b<a58<int>> d58;\n"
                            "b<a59<int>> d59;\n"
                            "b<a60<int>> d60;\n"
                            "b<a61<int>> d61;\n"
                            "b<a62<int>> d62;\n"
                            "b<a63<int>> d63;\n"
                            "b<a64<int>> d64;\n"
                            "b<a65<int>> d65;\n"
                            "b<a66<int>> d66;\n"
                            "b<a67<int>> d67;\n"
                            "b<a68<int>> d68;\n"
                            "b<a69<int>> d69;\n"
                            "b<a70<int>> d70;\n"
                            "b<a71<int>> d71;\n"
                            "b<a72<int>> d72;\n"
                            "b<a73<int>> d73;\n"
                            "b<a74<int>> d74;\n"
                            "b<a75<int>> d75;\n"
                            "b<a76<int>> d76;\n"
                            "b<a77<int>> d77;\n"
                            "b<a78<int>> d78;\n"
                            "b<a79<int>> d79;\n"
                            "b<a80<int>> d80;\n"
                            "b<a81<int>> d81;\n"
                            "b<a82<int>> d82;\n"
                            "b<a83<int>> d83;\n"
                            "b<a84<int>> d84;\n"
                            "b<a85<int>> d85;\n"
                            "b<a86<int>> d86;\n"
                            "b<a87<int>> d87;\n"
                            "b<a88<int>> d88;\n"
                            "b<a89<int>> d89;\n"
                            "b<a90<int>> d90;\n"
                            "b<a91<int>> d91;\n"
                            "b<a92<int>> d92;\n"
                            "b<a93<int>> d93;\n"
                            "b<a94<int>> d94;\n"
                            "b<a95<int>> d95;\n"
                            "b<a96<int>> d96;\n"
                            "b<a97<int>> d97;\n"
                            "b<a98<int>> d98;\n"
                            "b<a99<int>> d99;\n"
                            "b<a100<int>> d100;";
        // don't bother checking the output because this is not instantiated properly
        (void)tok(code); // don't crash

        const char code2[] = "template<typename T> void f();\n" // #11489
                             "template<typename T> void f(int);\n"
                             "void g() {\n"
                             "    f<int>();\n"
                             "    f<char>(1);\n"
                             "}\n"
                             "template<typename T>\n"
                             "void f(int) {}\n"
                             "template<typename T>\n"
                             "void f() {\n"
                             "    f<T>(0);\n"
                             "}\n";
        const char exp2[] = "template < typename T > void f ( ) ; "
                            "void f<char> ( int ) ; "
                            "void f<int> ( int ) ; "
                            "void g ( ) { f<int> ( ) ; f<char> ( 1 ) ; } "
                            "void f<char> ( int ) { } "
                            "void f<int> ( ) { f<int> ( 0 ) ; }";
        ASSERT_EQUALS(exp2, tok(code2));
    }

    void template159() {  // #9886
        const char code[] = "struct impl { template <class T> static T create(); };\n"
                            "template<class T, class U, class = decltype(impl::create<T>()->impl::create<U>())>\n"
                            "struct tester{};\n"
                            "tester<impl*, int> ti;\n"
                            "template<class T, class U, class = decltype(impl::create<T>()->impl::create<U>())>\n"
                            "int test() { return 0; }\n"
                            "int i = test<impl*, int>();";
        const char exp[]  = "struct impl { template < class T > static T create ( ) ; } ; "
                            "struct tester<impl*,int,decltype(impl::create<impl*>().impl::create<int>())> ; "
                            "tester<impl*,int,decltype(impl::create<impl*>().impl::create<int>())> ti ; "
                            "int test<impl*,int,decltype(impl::create<impl*>().impl::create<int>())> ( ) ; "
                            "int i ; i = test<impl*,int,decltype(impl::create<impl*>().impl::create<int>())> ( ) ; "
                            "int test<impl*,int,decltype(impl::create<impl*>().impl::create<int>())> ( ) { return 0 ; } "
                            "struct tester<impl*,int,decltype(impl::create<impl*>().impl::create<int>())> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template160() {
        const char code[] = "struct Fred {\n"
                            "    template <typename T> static void foo() { }\n"
                            "    template <typename T> static void foo(T) { }\n"
                            "};\n"
                            "template void Fred::foo<char>();\n"
                            "template <> void Fred::foo<bool>() { }\n"
                            "template void Fred::foo<float>(float);\n"
                            "template <> void Fred::foo<int>(int) { }";
        const char exp[]  = "struct Fred { "
                            "static void foo<bool> ( ) ; "
                            "static void foo<char> ( ) ; "
                            "static void foo<int> ( int ) ; "
                            "static void foo<float> ( float ) ; "
                            "} ; "
                            "void Fred :: foo<bool> ( ) { } "
                            "void Fred :: foo<int> ( int ) { } "
                            "void Fred :: foo<float> ( float ) { } "
                            "void Fred :: foo<char> ( ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template161() {
        const char code[] = "struct JobEntry { };\n"
                            "template<class T>\n"
                            "struct adapter : public T {\n"
                            "    template<class... Args>\n"
                            "    adapter(Args&&... args) : T{ std::forward<Args>(args)... } {}\n"
                            "};\n"
                            "void foo() {\n"
                            "   auto notifyJob = std::make_shared<adapter<JobEntry>> ();\n"
                            "}";
        const char exp[]  = "???";
        const char act[]  = "struct JobEntry { } ; "
                            "struct adapter<JobEntry> ; "
                            "void foo ( ) { "
                            "auto notifyJob ; notifyJob = std :: make_shared < adapter<JobEntry> > ( ) ; "
                            "} "
                            "struct adapter<JobEntry> : public JobEntry { "
                            "template < class ... Args > "
                            "adapter<JobEntry> ( Args && ... args ) : JobEntry { std :: forward < Args > ( args ) ... } { } "
                            "} ;";
        TODO_ASSERT_EQUALS(exp, act, tok(code));
    }

    void template162() {
        const char code[] = "template <std::size_t N>\n"
                            "struct CountryCode {\n"
                            "    CountryCode(std::string cc);\n"
                            "};"
                            "template <std::size_t N>\n"
                            "CountryCode<N>::CountryCode(std::string cc) : m_String{std::move(cc)} {\n"
                            "}\n"
                            "template class CountryCode<2>;\n"
                            "template class CountryCode<3>;";
        const char exp[]  = "struct CountryCode<2> ; "
                            "struct CountryCode<3> ; "
                            "struct CountryCode<2> { "
                            "CountryCode<2> ( std :: string cc ) ; "
                            "} ; "
                            "CountryCode<2> :: CountryCode<2> ( std :: string cc ) : m_String { std :: move ( cc ) } { "
                            "} "
                            "struct CountryCode<3> { "
                            "CountryCode<3> ( std :: string cc ) ; "
                            "} ; "
                            "CountryCode<3> :: CountryCode<3> ( std :: string cc ) : m_String { std :: move ( cc ) } { "
                            "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template163() { // #9685 syntax error
        const char code[] = "extern \"C++\" template < typename T > T * test ( ) { return nullptr ; }";
        const char expected[] = "template < typename T > T * test ( ) { return nullptr ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template164() {  // #9394
        const char code[] = "template <class TYPE>\n"
                            "struct A {\n"
                            "    A();\n"
                            "    ~A();\n"
                            "    static void f();\n"
                            "};\n"
                            "template <class TYPE>\n"
                            "A<TYPE>::A() { }\n"
                            "template <class TYPE>\n"
                            "A<TYPE>::~A() { }\n"
                            "template <class TYPE>\n"
                            "void A<TYPE>::f() { }\n"
                            "template class A<int>;\n"
                            "template class A<float>;";
        const char exp[]  = "struct A<int> ; "
                            "struct A<float> ; "
                            "struct A<int> { "
                            "A<int> ( ) ; "
                            "~ A<int> ( ) ; "
                            "static void f ( ) ; "
                            "} ; "
                            "A<int> :: A<int> ( ) { } "
                            "A<int> :: ~ A<int> ( ) { } "
                            "void A<int> :: f ( ) { } "
                            "struct A<float> { "
                            "A<float> ( ) ; "
                            "~ A<float> ( ) ; "
                            "static void f ( ) ; "
                            "} ; "
                            "A<float> :: A<float> ( ) { } "
                            "A<float> :: ~ A<float> ( ) { } "
                            "void A<float> :: f ( ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template165() { // #10032 syntax error
        const char code[] = "struct MyStruct {\n"
                            "    template<class T>\n"
                            "    bool operator()(const T& l, const T& r) const {\n"
                            "        return l.first < r.first;\n"
                            "    }\n"
                            "};";
        const char exp[]  = "struct MyStruct { "
                            "template < class T > "
                            "bool operator() ( const T & l , const T & r ) const { "
                            "return l . first < r . first ; "
                            "} "
                            "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template166() { // #10081 hang
        const char code[] = "template <typename T, size_t k = (T::s < 3) ? 0 : 3>\n"
                            "void foo() {}\n"
                            "foo<T>();";
        const char exp[]  = "void foo<T,(T::s<3)?0:3> ( ) ; "
                            "foo<T,(T::s<3)?0:3> ( ) ; "
                            "void foo<T,(T::s<3)?0:3> ( ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template167() {
        const char code[] = "struct MathLib {\n"
                            "    template<class T> static std::string toString(T value) {\n"
                            "        return std::string{};\n"
                            "    }\n"
                            "};\n"
                            "template<> std::string MathLib::toString(double value);\n"
                            "template<> std::string MathLib::toString(double value) {\n"
                            "    return std::string{std::to_string(value)};\n"
                            "}\n"
                            "void foo() {\n"
                            "    std::string str = MathLib::toString(1.0);\n"
                            "}";
        const char exp[]  = "struct MathLib { "
                            "static std :: string toString<double> ( double value ) ; "
                            "template < class T > static std :: string toString ( T value ) { "
                            "return std :: string { } ; "
                            "} "
                            "} ; "
                            "std :: string MathLib :: toString<double> ( double value ) { "
                            "return std :: string { std :: to_string ( value ) } ; "
                            "} "
                            "void foo ( ) { "
                            "std :: string str ; str = MathLib :: toString<double> ( 1.0 ) ; "
                            "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template168() {
        const char code[] = "template < typename T, typename U > struct type { };\n"
                            "template < > struct type < bool, bool > {};\n"
                            "template < > struct type < unsigned char, unsigned char > {};\n"
                            "template < > struct type < char, char > {};\n"
                            "template < > struct type < signed char, signed char > {};\n"
                            "template < > struct type < unsigned short, unsigned short > {};\n"
                            "template < > struct type < short, short > {};\n"
                            "template < > struct type < unsigned int, unsigned int > {};\n"
                            "template < > struct type < int, int > {};\n"
                            "template < > struct type < unsigned long long, unsigned long long > {};\n"
                            "template < > struct type < long long, long long > {};\n"
                            "template < > struct type < double, double > {};\n"
                            "template < > struct type < float, float > {};\n"
                            "template < > struct type < long double, long double > {};";
        const char exp[]  = "struct type<longdouble,longdouble> ; "
                            "struct type<float,float> ; "
                            "struct type<double,double> ; "
                            "struct type<longlong,longlong> ; "
                            "struct type<unsignedlonglong,unsignedlonglong> ; "
                            "struct type<int,int> ; "
                            "struct type<unsignedint,unsignedint> ; "
                            "struct type<short,short> ; "
                            "struct type<unsignedshort,unsignedshort> ; "
                            "struct type<signedchar,signedchar> ; "
                            "struct type<char,char> ; "
                            "struct type<unsignedchar,unsignedchar> ; "
                            "struct type<bool,bool> ; "
                            "template < typename T , typename U > struct type { } ; "
                            "struct type<bool,bool> { } ; "
                            "struct type<unsignedchar,unsignedchar> { } ; "
                            "struct type<char,char> { } ; "
                            "struct type<signedchar,signedchar> { } ; "
                            "struct type<unsignedshort,unsignedshort> { } ; "
                            "struct type<short,short> { } ; "
                            "struct type<unsignedint,unsignedint> { } ; "
                            "struct type<int,int> { } ; "
                            "struct type<unsignedlonglong,unsignedlonglong> { } ; "
                            "struct type<longlong,longlong> { } ; "
                            "struct type<double,double> { } ; "
                            "struct type<float,float> { } ; "
                            "struct type<longdouble,longdouble> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template169() {
        const char code[] = "template < typename T> struct last { T t; };\n"
                            "template < typename T > struct CImgList { T t; };\n"
                            "CImgList < last < bool > > c1;\n"
                            "CImgList < last < signed char > > c2;\n"
                            "CImgList < last < unsigned char > > c3;\n"
                            "CImgList < last < char > > c4;\n"
                            "CImgList < last < unsigned short > > c5;\n"
                            "CImgList < last < short > > c6;\n"
                            "CImgList < last < unsigned int > > c7;\n"
                            "CImgList < last < int > > c8;\n"
                            "CImgList < last < unsigned long > > c9;\n"
                            "CImgList < last < long > > c10;\n"
                            "CImgList < last < unsigned long long > > c11;\n"
                            "CImgList < last < long long > > c12;\n"
                            "CImgList < last < float > > c13;\n"
                            "CImgList < last < double > > c14;\n"
                            "CImgList < last < long double > > c15;";
        const char exp[]  = "struct last<bool> ; "
                            "struct last<signedchar> ; "
                            "struct last<unsignedchar> ; "
                            "struct last<char> ; "
                            "struct last<unsignedshort> ; "
                            "struct last<short> ; "
                            "struct last<unsignedint> ; "
                            "struct last<int> ; "
                            "struct last<unsignedlong> ; "
                            "struct last<long> ; "
                            "struct last<unsignedlonglong> ; "
                            "struct last<longlong> ; "
                            "struct last<float> ; "
                            "struct last<double> ; "
                            "struct last<longdouble> ; "
                            "struct CImgList<last<bool>> ; "
                            "struct CImgList<last<signedchar>> ; "
                            "struct CImgList<last<unsignedchar>> ; "
                            "struct CImgList<last<char>> ; "
                            "struct CImgList<last<unsignedshort>> ; "
                            "struct CImgList<last<short>> ; "
                            "struct CImgList<last<unsignedint>> ; "
                            "struct CImgList<last<int>> ; "
                            "struct CImgList<last<unsignedlong>> ; "
                            "struct CImgList<last<long>> ; "
                            "struct CImgList<last<unsignedlonglong>> ; "
                            "struct CImgList<last<longlong>> ; "
                            "struct CImgList<last<float>> ; "
                            "struct CImgList<last<double>> ; "
                            "struct CImgList<last<longdouble>> ; "
                            "CImgList<last<bool>> c1 ; "
                            "CImgList<last<signedchar>> c2 ; "
                            "CImgList<last<unsignedchar>> c3 ; "
                            "CImgList<last<char>> c4 ; "
                            "CImgList<last<unsignedshort>> c5 ; "
                            "CImgList<last<short>> c6 ; "
                            "CImgList<last<unsignedint>> c7 ; "
                            "CImgList<last<int>> c8 ; "
                            "CImgList<last<unsignedlong>> c9 ; "
                            "CImgList<last<long>> c10 ; "
                            "CImgList<last<unsignedlonglong>> c11 ; "
                            "CImgList<last<longlong>> c12 ; "
                            "CImgList<last<float>> c13 ; "
                            "CImgList<last<double>> c14 ; "
                            "CImgList<last<longdouble>> c15 ; "
                            "struct CImgList<last<bool>> { last<bool> t ; } ; "
                            "struct CImgList<last<signedchar>> { last<signedchar> t ; } ; "
                            "struct CImgList<last<unsignedchar>> { last<unsignedchar> t ; } ; "
                            "struct CImgList<last<char>> { last<char> t ; } ; "
                            "struct CImgList<last<unsignedshort>> { last<unsignedshort> t ; } ; "
                            "struct CImgList<last<short>> { last<short> t ; } ; "
                            "struct CImgList<last<unsignedint>> { last<unsignedint> t ; } ; "
                            "struct CImgList<last<int>> { last<int> t ; } ; "
                            "struct CImgList<last<unsignedlong>> { last<unsignedlong> t ; } ; "
                            "struct CImgList<last<long>> { last<long> t ; } ; "
                            "struct CImgList<last<unsignedlonglong>> { last<unsignedlonglong> t ; } ; "
                            "struct CImgList<last<longlong>> { last<longlong> t ; } ; "
                            "struct CImgList<last<float>> { last<float> t ; } ; "
                            "struct CImgList<last<double>> { last<double> t ; } ; "
                            "struct CImgList<last<longdouble>> { last<longdouble> t ; } ; "
                            "struct last<bool> { bool t ; } ; "
                            "struct last<signedchar> { signed char t ; } ; "
                            "struct last<unsignedchar> { unsigned char t ; } ; "
                            "struct last<char> { char t ; } ; "
                            "struct last<unsignedshort> { unsigned short t ; } ; "
                            "struct last<short> { short t ; } ; "
                            "struct last<unsignedint> { unsigned int t ; } ; "
                            "struct last<int> { int t ; } ; "
                            "struct last<unsignedlong> { unsigned long t ; } ; "
                            "struct last<long> { long t ; } ; "
                            "struct last<unsignedlonglong> { unsigned long long t ; } ; "
                            "struct last<longlong> { long long t ; } ; "
                            "struct last<float> { float t ; } ; "
                            "struct last<double> { double t ; } ; "
                            "struct last<longdouble> { long double t ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template170() { // crash
        const char code[] = "template <int b> int a = 0;\n"
                            "void c() {\n"
                            "  a<1>;\n"
                            "  [](auto b) {};\n"
                            "}";
        const char exp[]  = "int a<1> ; a<1> = 0 ; "
                            "void c ( ) { "
                            "a<1> ; "
                            "[ ] ( auto b ) { } ; "
                            "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template171() { // crash
        const char code[] = "template <int> struct c { enum { b }; };\n"
                            "template <int> struct h { enum { d }; enum { e }; };\n"
                            "template <int f, long = h<f>::d, int g = h<f>::e> class i { enum { e = c<g>::b }; };\n"
                            "void j() { i<2> a; }";
        const char exp[]  = "struct c<h<2>::e> ; "
                            "struct h<2> ; "
                            "class i<2,h<2>::d,h<2>::e> ; "
                            "void j ( ) { i<2,h<2>::d,h<2>::e> a ; } "
                            "class i<2,h<2>::d,h<2>::e> { enum Anonymous3 { e = c<h<2>::e> :: b } ; } ; "
                            "struct h<2> { enum Anonymous1 { d } ; enum Anonymous2 { e } ; } ; "
                            "struct c<h<2>::e> { enum Anonymous0 { b } ; } ;";
        const char act[]  = "struct c<h<2>::e> ; "
                            "template < int > struct h { enum Anonymous1 { d } ; enum Anonymous2 { e } ; } ; "
                            "class i<2,h<2>::d,h<2>::e> ; "
                            "void j ( ) { i<2,h<2>::d,h<2>::e> a ; } "
                            "class i<2,h<2>::d,h<2>::e> { enum Anonymous3 { e = c<h<2>::e> :: b } ; } ; "
                            "struct c<h<2>::e> { enum Anonymous0 { b } ; } ;";
        TODO_ASSERT_EQUALS(exp, act, tok(code));
    }

    void template172() { // #10258 crash
        const char code[] = "template<typename T, typename... Args>\n"
                            "void bar(T t, Args&&... args) { }\n"
                            "void foo() { bar<int>(0, 1); }";
        const char exp[]  = "void bar<int> ( int t , Args && ... args ) ; "
                            "void foo ( ) { bar<int> ( 0 , 1 ) ; } "
                            "void bar<int> ( int t , Args && ... args ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template173() { // #10332 crash
        const char code[] = "namespace a {\n"
                            "template <typename, typename> struct b;\n"
                            "template <template <typename, typename> class = b> class c;\n"
                            "using d = c<>;\n"
                            "template <template <typename, typename = void> class> class c {};\n"
                            "}\n"
                            "namespace std {\n"
                            "template <> void swap<a::d>(a::d &, a::d &) {}\n"
                            "}";
        const char exp[]  = "namespace a { "
                            "template < typename , typename > struct b ; "
                            "template < template < typename , typename > class > class c ; "
                            "class c<b> ; "
                            "} "
                            "namespace std { "
                            "void swap<a::c<b>> ( a :: c<b> & , a :: c<b> & ) ; "
                            "void swap<a::c<b>> ( a :: c<b> & , a :: c<b> & ) { } "
                            "} "
                            "class a :: c<b> { } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template174()
    { // #10506 hang
        const char code[] = "namespace a {\n"
                            "template <typename> using b = int;\n"
                            "template <typename c> c d() { return d<b<c>>(); }\n"
                            "}\n"
                            "void e() { a::d<int>(); }\n";
        const char exp[] = "namespace a { int d<int> ( ) ; } "
                           "void e ( ) { a :: d<int> ( ) ; } "
                           "int a :: d<int> ( ) { return d < int > ( ) ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template175() // #10908
    {
        const char code[] = "template <typename T, int value> T Get() {return value;}\n"
                            "char f() { Get<int,10>(); }\n";
        const char exp[] = "int Get<int,10> ( ) ; "
                           "char f ( ) { Get<int,10> ( ) ; } "
                           "int Get<int,10> ( ) { return 10 ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template176() // #11146 don't crash
    {
        const char code[] = "struct a {\n"
                            "    template <typename> class b {};\n"
                            "};\n"
                            "struct c {\n"
                            "    template <typename> a::b<int> d();\n"
                            "    ;\n"
                            "};\n"
                            "template <typename> a::b<int> c::d() {}\n"
                            "template <> class a::b<int> c::d<int>() { return {}; };\n";
        const char exp[] = "struct a { "
                           "class b<int> c :: d<int> ( ) ; "
                           "template < typename > class b { } ; "
                           "} ; "
                           "struct c { a :: b<int> d<int> ( ) ; } ; "
                           "class a :: b<int> c :: d<int> ( ) { return { } ; } ; "
                           "a :: b<int> c :: d<int> ( ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template177() {
        const char code[] = "template <typename Encoding, typename Allocator>\n"
                            "class C { xyz<Encoding, Allocator> x; };\n"
                            "C<UTF8<>, MemoryPoolAllocator<>> c;";
        const char exp[] = "class C<UTF8<>,MemoryPoolAllocator<>> ; "
                           "C<UTF8<>,MemoryPoolAllocator<>> c ; "
                           "class C<UTF8<>,MemoryPoolAllocator<>> { xyz < UTF8 < > , MemoryPoolAllocator < > > x ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template178() {
        const char code[] = "template<typename T>\n" // #11893
                            "void g() {\n"
                            "    for (T i = 0; i < T{ 3 }; ++i) {}\n"
                            "}\n"
                            "void f() {\n"
                            "    g<int>();\n"
                            "}";
        const char exp[] = "void g<int> ( ) ; void f ( ) { g<int> ( ) ; } void g<int> ( ) { for ( int i = 0 ; i < int { 3 } ; ++ i ) { } }";
        ASSERT_EQUALS(exp, tok(code));

        const char code2[] = "template<typename T>\n"
                             "struct S {\n"
                             "    T c;\n"
                             "    template<typename U>\n"
                             "    void g() {\n"
                             "        for (U i = 0; i < U{ 3 }; ++i) {}\n"
                             "    }\n"
                             "};\n"
                             "void f() {\n"
                             "    S<char> s{};\n"
                             "    s.g<int>();\n"
                             "}";
        const char exp2[] = "struct S<char> ; void f ( ) { S<char> s { } ; s . g<int> ( ) ; } struct S<char> { char c ; void g<int> ( ) ; } ; "
                            "void S<char> :: g<int> ( ) { for ( int i = 0 ; i < int { 3 } ; ++ i ) { } }";
        ASSERT_EQUALS(exp2, tok(code2));
    }

    void template179() {
        const char code[] = "template <typename T, typename C>\n" // #13498
                            "struct B {\n"
                            "    int a;\n"
                            "    int b;\n"
                            "};\n"
                            "template <typename T>\n"
                            "struct B<T, void> {\n"
                            "    int a;\n"
                            "};\n"
                            "void f() {\n"
                            "    B<int, int>{ 0, {} };\n"
                            "}\n";
        const char exp[] = "struct B<int,int> ; "
                           "template < typename T > struct B < T , void > { int a ; } ; "
                           "void f ( ) { B<int,int> { 0 , { } } ; } "
                           "struct B<int,int> { int a ; int b ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template180() {
        const char code[] = "template<int C>\n" // #13680
                            "int f() {\n"
                            "    return dostuff(C);\n"
                            "}\n"
                            "void g() {\n"
                            "    f<static_cast<int>(0U)>();\n"
                            "}\n";
        const char exp[] = "int f<static_cast<int>(0U)> ( ) ; "
                           "void g ( ) { "
                           "f<static_cast<int>(0U)> ( ) ; "
                           "} "
                           "int f<static_cast<int>(0U)> ( ) { "
                           "return dostuff ( static_cast < int > ( 0U ) ) ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template181() {
        const char code[] = "struct K { bool b; };\n" // #13747
                            "template<bool b>\n"
                            "void f(struct K* k) {\n"
                            "    assert(b == k->b);\n"
                            "}\n"
                            "void g(struct K* k) {\n"
                            "    f<false>(k);\n"
                            "}\n";
        const char exp[] = "struct K { bool b ; } ; "
                           "void f<false> ( struct K * k ) ; "
                           "void g ( struct K * k ) { "
                           "f<false> ( k ) ; "
                           "} "
                           "void f<false> ( struct K * k ) { "
                           "assert ( false == k . b ) ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));

        const char code2[] = "namespace N { bool b = false; }\n" // #13759
                             "template<bool b>\n"
                             "void f() {\n"
                             "    assert(b == N::b);\n"
                             "}\n"
                             "void g() {\n"
                             "    f<false>();\n"
                             "}\n";
        const char exp2[] = "namespace N { bool b ; b = false ; } "
                            "void f<false> ( ) ; "
                            "void g ( ) { "
                            "f<false> ( ) ; "
                            "} "
                            "void f<false> ( ) { "
                            "assert ( false == N :: b ) ; "
                            "}";
        ASSERT_EQUALS(exp2, tok(code2));
    }

    void template182() {
        const char code[] = "template <class...>\n"
                            "auto f() {\n"
                            "    return [](auto&&...) {};\n"
                            "}\n";
        const char exp[] = "template < class ... > auto f ( ) { return [ ] ( auto && ... ) { } ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template183() { // #11498
        const char code[] = "template <typename T>\n"
                            "struct S {\n"
                            "    void f();\n"
                            "    using X = decltype(&S<T>::f);\n"
                            "private:\n"
                            "    X x;\n"
                            "};\n"
                            "S<int> s;\n";
        const char exp[] = "struct S<int> ; "
                           "S<int> s ; "
                           "struct S<int> { "
                           "void f ( ) ; "
                           "private: "
                           "decltype ( & S<int> :: f ) x ; "
                           "} ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template184() {
        const char code[] = "template <typename T>\n"
                            "T g(T x) {\n"
                            "  return x;\n"
                            "}\n"
                            "template <>\n"
                            "float g<float>(float x) {\n"
                            "  return x + 1.0f;\n"
                            "}\n"
                            "void f(int i) {\n"
                            "  g(i);\n"
                            "  g(1.0f);\n"
                            "}\n";
        const char exp[] = "float g<float> ( float x ) ; "
                           "template < typename T > "
                           "T g ( T x ) { return x ; } "
                           "float g<float> ( float x ) {"
                           " return x + 1.0f ; "
                           "} "
                           "void f ( int i ) {"
                           " g ( i ) ;"
                           " g<float> ( 1.0f ) ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code)); // TODO: instantiate g<int>(int)
    }

    void template_specialization_1() {  // #7868 - template specialization template <typename T> struct S<C<T>> {..};
        const char code[] = "template <typename T> struct C {};\n"
                            "template <typename T> struct S {a};\n"
                            "template <typename T> struct S<C<T>> {b};\n"
                            "S<int> s;";
        const char exp[]  = "template < typename T > struct C { } ; struct S<int> ; template < typename T > struct S < C < T > > { b } ; S<int> s ; struct S<int> { a } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template_specialization_2() {  // #7868 - template specialization template <typename T> struct S<C<T>> {..};
        const char code[] = "template <typename T> struct C {};\n"
                            "template <typename T> struct S {a};\n"
                            "template <typename T> struct S<C<T>> {b};\n"
                            "S<C<int>> s;";
        const char exp[]  = "template < typename T > struct C { } ; template < typename T > struct S { a } ; struct S<C<int>> ; S<C<int>> s ; struct S<C<int>> { b } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template_specialization_3() {
        const char code[] = "namespace N {\n" // #12312
                            "    template <typename T>\n"
                            "    bool equal(const T&);\n"
                            "    template <>\n"
                            "    bool equal<int>(const int&) { return false; }\n"
                            "}\n"
                            "void f(bool equal, int i) { if (equal) {} }\n";
        const char exp[] = "namespace N { "
                           "bool equal<int> ( const int & ) ; "
                           "template < typename T > "
                           "bool equal ( const T & ) ; "
                           "bool equal<int> ( const int & ) { return false ; } "
                           "} "
                           "void f ( bool equal , int i ) { if ( equal ) { } }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template_enum() {
        const char code[] = "template <class T>\n"
                            "struct Unconst {\n"
                            "    typedef T type;\n"
                            "};\n"
                            "template <class T>\n"
                            "struct Unconst<const T> {\n"
                            "    typedef T type;\n"
                            "};\n"
                            "template <class T>\n"
                            "struct Unconst<const T&> {\n"
                            "    typedef T& type;\n"
                            "};\n"
                            "template <class T>\n"
                            "struct Unconst<T* const> {\n"
                            "    typedef T* type;\n"
                            "};\n"
                            "template <class T1, class T2>\n"
                            "struct type_equal {\n"
                            "    enum {  value = 0   };\n"
                            "};\n"
                            "template <class T>\n"
                            "struct type_equal<T, T> {\n"
                            "    enum {  value = 1   };\n"
                            "};\n"
                            "template<class T>\n"
                            "struct template_is_const\n"
                            "{\n"
                            "    enum {value = !type_equal<T, typename Unconst<T>::type>::value  };\n"
                            "};";
        const char exp[] = "template < class T > struct Unconst { } ; "
                           "template < class T > struct Unconst < const T > { } ; "
                           "template < class T > struct Unconst < const T & > { } ; "
                           "template < class T > struct Unconst < T * const > { } ; "
                           "template < class T1 , class T2 > struct type_equal { enum Anonymous0 { value = 0 } ; } ; "
                           "template < class T > struct type_equal < T , T > { enum Anonymous1 { value = 1 } ; } ; "
                           "template < class T > struct template_is_const { enum Anonymous2 { value = ! type_equal < T , Unconst < T > :: type > :: value } ; } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void template_default_parameter() {
        {
            const char code[] = "template <class T, int n=3>\n"
                                "class A\n"
                                "{ T ar[n]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int,2> a1;\n"
                                "    A<int> a2;\n"
                                "}\n";
            const char expected[] = "class A<int,2> ; "
                                    "class A<int,3> ; "
                                    "void f ( ) "
                                    "{"
                                    " A<int,2> a1 ;"
                                    " A<int,3> a2 ; "
                                    "} "
                                    "class A<int,2> "
                                    "{ int ar [ 2 ] ; } ; "
                                    "class A<int,3> "
                                    "{ int ar [ 3 ] ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <class T, int n1=3, int n2=2>\n"
                                "class A\n"
                                "{ T ar[n1+n2]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int> a1;\n"
                                "    A<int,3> a2;\n"
                                "}\n";

            const char expected[] = "class A<int,3,2> ; "
                                    "void f ( ) "
                                    "{"
                                    " A<int,3,2> a1 ;"
                                    " A<int,3,2> a2 ; "
                                    "} "
                                    "class A<int,3,2> "
                                    "{ int ar [ 3 + 2 ] ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <class T, int n=3>\n"
                                "class A\n"
                                "{ T ar[n]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int,(int)2> a1;\n"
                                "    A<int> a2;\n"
                                "}\n";
            const char expected[] = "class A<int,2> ; "
                                    "class A<int,3> ; "
                                    "void f ( ) "
                                    "{ "
                                    "A<int,2> a1 ; "
                                    "A<int,3> a2 ; "
                                    "} "
                                    "class A<int,2> "
                                    "{ int ar [ 2 ] ; } ; "
                                    "class A<int,3> "
                                    "{ int ar [ 3 ] ; } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "class A { }; "
                                "template<class T> class B { }; "
                                "template<class T1, class T2 = B<T1>> class C { }; "
                                "template<class T1 = A, typename T2 = B<A>> class D { };";
            ASSERT_EQUALS("class A { } ; "
                          "template < class T > class B { } ; "
                          "template < class T1 , class T2 = B < T1 > > class C { } ; "
                          "template < class T1 = A , typename T2 = B < A > > class D { } ;", tok(code));
        }
        {
            // #7548
            const char code[] = "template<class T, class U> class DefaultMemory {}; "
                                "template<class Key, class Val, class Mem=DefaultMemory<Key,Val> > class thv_table_c  {}; "
                                "thv_table_c<void *,void *> id_table_m;";
            const char exp[] = "template < class T , class U > class DefaultMemory { } ; "
                               "class thv_table_c<void*,void*,DefaultMemory<void*,void*>> ; "
                               "thv_table_c<void*,void*,DefaultMemory<void*,void*>> id_table_m ; "
                               "class thv_table_c<void*,void*,DefaultMemory<void*,void*>> { } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            // #8890
            const char code[] = "template <typename = void> struct a {\n"
                                "  void c();\n"
                                "};\n"
                                "void f() {\n"
                                "  a<> b;\n"
                                "  b.a<>::c();\n"
                                "}";
            ASSERT_EQUALS("struct a<void> ; "
                          "void f ( ) { "
                          "a<void> b ; "
                          "b . a<void> :: c ( ) ; "
                          "} "
                          "struct a<void> { "
                          "void c ( ) ; "
                          "} ;", tok(code));
        }
        {
            // #8890
            const char code[] = "template< typename T0 = void > class A;\n"
                                "template<>\n"
                                "class A< void > {\n"
                                "    public:\n"
                                "        A() { }\n"
                                "        ~A() { }\n"
                                "        void Print() { std::cout << \"A\" << std::endl; }\n"
                                "};\n"
                                "class B : public A<> {\n"
                                "    public:\n"
                                "        B() { }\n"
                                "        ~B() { }\n"
                                "};\n"
                                "int main( int argc, char* argv[] ) {\n"
                                "    B b;\n"
                                "    b.A<>::Print();\n"
                                "    return 0;\n"
                                "}";
            ASSERT_EQUALS("class A<void> ; "
                          "template < typename T0 > class A ; "
                          "class A<void> { "
                          "public: "
                          "A<void> ( ) { } "
                          "~ A<void> ( ) { } "
                          "void Print ( ) { std :: cout << \"A\" << std :: endl ; } "
                          "} ; "
                          "class B : public A<void> { "
                          "public: "
                          "B ( ) { } "
                          "~ B ( ) { } "
                          "} ; "
                          "int main ( int argc , char * argv [ ] ) { "
                          "B b ; "
                          "b . A<void> :: Print ( ) ; "
                          "return 0 ; "
                          "}", tok(code));
        }
    }

    void template_forward_declared_default_parameter() {
        {
            const char code[] = "template <class T, int n=3> class A;\n"
                                "template <class T, int n>\n"
                                "class A\n"
                                "{ T ar[n]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int,2> a1;\n"
                                "    A<int> a2;\n"
                                "}\n";

            const char exp[] = "class A<int,2> ; "
                               "class A<int,3> ; "
                               "void f ( ) "
                               "{"
                               " A<int,2> a1 ;"
                               " A<int,3> a2 ; "
                               "} "
                               "class A<int,2> "
                               "{ int ar [ 2 ] ; } ; "
                               "class A<int,3> "
                               "{ int ar [ 3 ] ; } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template <class, int = 3> class A;\n"
                                "template <class T, int n>\n"
                                "class A\n"
                                "{ T ar[n]; };\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "    A<int,2> a1;\n"
                                "    A<int> a2;\n"
                                "}\n";

            const char exp[] = "class A<int,2> ; "
                               "class A<int,3> ; "
                               "void f ( ) "
                               "{"
                               " A<int,2> a1 ;"
                               " A<int,3> a2 ; "
                               "} "
                               "class A<int,2> "
                               "{ int ar [ 2 ] ; } ; "
                               "class A<int,3> "
                               "{ int ar [ 3 ] ; } ;";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "template<typename Lhs, int TriangularPart = (int(Lhs::Flags) & LowerTriangularBit)>\n"
                                "struct ei_solve_triangular_selector;\n"
                                "template<typename Lhs, int UpLo>\n"
                                "struct ei_solve_triangular_selector<Lhs,UpLo> {\n"
                                "};\n"
                                "template<typename Lhs, int TriangularPart>\n"
                                "struct ei_solve_triangular_selector { };";

            const char exp[] = "template < typename Lhs , int TriangularPart = ( int ( Lhs :: Flags ) & LowerTriangularBit ) > "
                               "struct ei_solve_triangular_selector ; "
                               "template < typename Lhs , int UpLo > "
                               "struct ei_solve_triangular_selector < Lhs , UpLo > { "
                               "} ; "
                               "template < typename Lhs , int TriangularPart = ( int ( Lhs :: Flags ) & LowerTriangularBit ) > "
                               "struct ei_solve_triangular_selector { } ;";

            ASSERT_EQUALS(exp, tok(code));
        }
        { // #10432
            const char code[] = "template<int A = 128, class T = wchar_t>\n"
                                "class Foo;\n"
                                "template<int A, class T>\n"
                                "class Foo\n"
                                "{\n"
                                "public:\n"
                                "  T operator[](int Index) const;\n"
                                "};\n"
                                "template<int A, class T>\n"
                                "T Foo<A, T>::operator[](int Index) const\n"
                                "{\n"
                                "  return T{};\n"
                                "}\n"
                                "Foo<> f;";
            const char exp[] = "class Foo<128,wchar_t> ; Foo<128,wchar_t> f ; "
                               "class Foo<128,wchar_t> { public: wchar_t operator[] ( int Index ) const ; } ; "
                               "wchar_t Foo<128,wchar_t> :: operator[] ( int Index ) const { return wchar_t { } ; }";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void template_default_type() {
        const char code[] = "template <typename T, typename U=T>\n"
                            "class A\n"
                            "{\n"
                            "public:\n"
                            "  void foo() {\n"
                            "    int a;\n"
                            "    a = static_cast<U>(a);\n"
                            "  }\n"
                            "};\n"
                            "\n"
                            "template <typename T>\n"
                            "class B\n"
                            "{\n"
                            "protected:\n"
                            "  A<int> a;\n"
                            "};\n"
                            "\n"
                            "class C\n"
                            "  : public B<int>\n"
                            "{\n"
                            "};\n";

        (void)tok(code);

        //ASSERT_EQUALS("[file1.cpp:15]: (error) Internal error: failed to instantiate template. The checking continues anyway.\n", errout_str());
        ASSERT_EQUALS("", errout_str());
    }

    void template_typename() {
        {
            const char code[] = "template <class T>\n"
                                "void foo(typename T::t *)\n"
                                "{ }";

            // The expected result..
            const char expected[] = "template < class T > void foo ( T :: t * ) { }";
            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "void f() {\n"
                                "    x(sizeof typename);\n"
                                "    type = 0;\n"
                                "}";

            ASSERT_EQUALS("void f ( ) { x ( sizeof ( typename ) ) ; type = 0 ; }", tok(code));
        }
    }

    void template_constructor() {
        // #3152 - if template constructor is removed then there might be
        //         "no constructor" false positives
        const char code[] = "class Fred {\n"
                            "    template<class T> explicit Fred(T t) { }\n"
                            "};";
        ASSERT_EQUALS("class Fred { template < class T > explicit Fred ( T t ) { } } ;", tok(code));

        // #3532
        const char code2[] = "class Fred {\n"
                             "    template<class T> Fred(T t) { }\n"
                             "};";
        ASSERT_EQUALS("class Fred { template < class T > Fred ( T t ) { } } ;", tok(code2));
    }

    void syntax_error_templates_1() {
        // ok code.. using ">" for a comparison
        ASSERT_NO_THROW(tok("x<y>z> xyz;"));
        ASSERT_EQUALS("", errout_str());

        // ok code
        ASSERT_NO_THROW(tok("template<class T> operator<(T a, T b) { }"));
        ASSERT_EQUALS("", errout_str());

        // ok code (ticket #1984)
        ASSERT_NO_THROW(tok("void f(a) int a;\n"
                            "{ ;x<y; }"));
        ASSERT_EQUALS("", errout_str());

        // ok code (ticket #1985)
        ASSERT_NO_THROW(tok("void f()\n"
                            "{ try { ;x<y; } }"));
        ASSERT_EQUALS("", errout_str());

        // ok code (ticket #3183)
        ASSERT_NO_THROW(tok("MACRO(({ i < x }))"));
        ASSERT_EQUALS("", errout_str());

        // bad code.. missing ">"
        ASSERT_THROW_INTERNAL(tok("x<y<int> xyz;\n"), SYNTAX);

        // bad code
        ASSERT_THROW_INTERNAL(tok("typedef\n"
                                  "    typename boost::mpl::if_c<\n"
                                  "          _visitableIndex < boost::mpl::size< typename _Visitables::ConcreteVisitables >::value\n"
                                  "          , ConcreteVisitable\n"
                                  "          , Dummy< _visitableIndex >\n"
                                  "    >::type ConcreteVisitableOrDummy;\n"), SYNTAX);

        // code is ok, don't show syntax error
        ASSERT_NO_THROW(tok("struct A {int a;int b};\n"
                            "class Fred {"
                            "public:\n"
                            "    Fred() : a({1,2}) {\n"
                            "        for (int i=0;i<6;i++);\n" // <- no syntax error
                            "    }\n"
                            "private:\n"
                            "    A a;\n"
                            "};"));
        ASSERT_EQUALS("", errout_str());

        //both of these should work but in cppcheck 2.1 only the first option will work (ticket #9843)
        {
            const std::string expected = "template < long Num > constexpr bool foo < bar < Num > > = true ;";
            ASSERT_EQUALS(expected,
                          tok("template <long Num>\n"
                              "constexpr bool foo<bar<Num> > = true;\n"));
            ASSERT_EQUALS("", errout_str());
            ASSERT_EQUALS(expected,
                          tok("template <long Num>\n"
                              "constexpr bool foo<bar<Num>> = true;\n"));
            ASSERT_EQUALS("", errout_str());
        }
    }

    void template_member_ptr() { // Ticket #5786 (segmentation fault)
        (void)tok("struct A {}; "
                  "struct B { "
                  "template <void (A::*)() const> struct BB {}; "
                  "template <bool BT> static bool foo(int) { return true; } "
                  "void bar() { bool b = foo<true>(0); }"
                  "};");
        (void)tok("struct A {}; "
                  "struct B { "
                  "template <void (A::*)() volatile> struct BB {}; "
                  "template <bool BT> static bool foo(int) { return true; } "
                  "void bar() { bool b = foo<true>(0); }"
                  "};");
        (void)tok("struct A {}; "
                  "struct B { "
                  "template <void (A::*)() const volatile> struct BB {}; "
                  "template <bool BT> static bool foo(int) { return true; } "
                  "void bar() { bool b = foo<true>(0); }"
                  "};");
        (void)tok("struct A {}; "
                  "struct B { "
                  "template <void (A::*)() volatile const> struct BB {}; "
                  "template <bool BT> static bool foo(int) { return true; } "
                  "void bar() { bool b = foo<true>(0); }"
                  "};");
    }

    void template_namespace_1() {
        // #6570
        const char code[] = "namespace {\n"
                            "  template<class T> void Fred(T value) { }\n"
                            "}\n"
                            "Fred<int>(123);";
        ASSERT_EQUALS("namespace { "
                      "void Fred<int> ( int value ) ; "
                      "} "
                      "Fred<int> ( 123 ) ; "
                      "void Fred<int> ( int value ) { }", tok(code));
    }

    void template_namespace_2() {
        // #8283
        const char code[] = "namespace X {\n"
                            "  template<class T> struct S { };\n"
                            "}\n"
                            "X::S<int> s;";
        ASSERT_EQUALS("namespace X { "
                      "struct S<int> ; "
                      "} "
                      "X :: S<int> s ; "
                      "struct X :: S<int> { } ;", tok(code));
    }

    void template_namespace_3() {
        const char code[] = "namespace test16 {\n"
                            "  template <class T> struct foo {\n"
                            "    static void *bar();\n"
                            "  };\n"
                            "  void *test() { return foo<int>::bar(); }\n"
                            "}";
        ASSERT_EQUALS("namespace test16 {"
                      " struct foo<int> ;"
                      " void * test ( ) {"
                      " return foo<int> :: bar ( ) ;"
                      " } "
                      "} "
                      "struct test16 :: foo<int> {"
                      " static void * bar ( ) ; "
                      "} ;", tok(code));
    }

    void template_namespace_4() {
        const char code[] = "namespace foo {\n"
                            "  template<class T> class A { void dostuff() {} };\n"
                            "  struct S : public A<int> {\n"
                            "    void f() {\n"
                            "      A<int>::dostuff();\n"
                            "    }\n"
                            "  };\n"
                            "}";
        ASSERT_EQUALS("namespace foo {"
                      " class A<int> ;"
                      " struct S : public A<int> {"
                      " void f ( ) {"
                      " A<int> :: dostuff ( ) ;"
                      " }"
                      " } ; "
                      "} "
                      "class foo :: A<int> { void dostuff ( ) { } } ;", tok(code));
    }

    void template_namespace_5() {
        const char code[] = "template<class C> struct S {};\n"
                            "namespace X { S<int> s; }";
        ASSERT_EQUALS("struct S<int> ; "
                      "namespace X { S<int> s ; } "
                      "struct S<int> { } ;", tok(code));
    }

    void template_namespace_6() {
        const char code[] = "namespace NS {\n"
                            "template <typename T> union C {\n"
                            "  char dummy[sizeof(T)];\n"
                            "  T value;\n"
                            "  C();\n"
                            "  ~C();\n"
                            "  C(const C &);\n"
                            "  C & operator = (const C &);\n"
                            "};\n"
                            "}\n"
                            "NS::C<int> intC;\n"
                            "template <typename T> NS::C<T>::C() {}\n"
                            "template <typename T> NS::C<T>::~C() {}\n"
                            "template <typename T> NS::C<T>::C(const NS::C<T> &) {}\n"
                            "template <typename T> NS::C<T> & NS::C<T>::operator=(const NS::C<T> &) {}";
        ASSERT_EQUALS("namespace NS { "
                      "union C<int> ; "
                      "} "
                      "NS :: C<int> intC ; union NS :: C<int> { "
                      "char dummy [ sizeof ( int ) ] ; "
                      "int value ; "
                      "C<int> ( ) ; "
                      "~ C<int> ( ) ; "
                      "C<int> ( const NS :: C<int> & ) ; "
                      "NS :: C<int> & operator= ( const NS :: C<int> & ) ; "
                      "} ; "
                      "NS :: C<int> :: C<int> ( ) { } "
                      "NS :: C<int> :: ~ C<int> ( ) { } "
                      "NS :: C<int> :: C<int> ( const NS :: C<int> & ) { } "
                      "NS :: C<int> & NS :: C<int> :: operator= ( const NS :: C<int> & ) { }", tok(code));
    }

    void template_namespace_7() { // #8768
        const char code[] = "namespace N1 {\n"
                            "namespace N2 {\n"
                            "    struct C { };\n"
                            "    template <class T> struct CT { };\n"
                            "    C c1;\n"
                            "    CT<int> ct1;\n"
                            "}\n"
                            "N2::C c2;\n"
                            "N2::CT<int> ct2;\n"
                            "}\n"
                            "N1::N2::C c3;\n"
                            "N1::N2::CT<int> ct3;";
        ASSERT_EQUALS("namespace N1 { "
                      "namespace N2 { "
                      "struct C { } ; "
                      "struct CT<int> ; "
                      "C c1 ; "
                      "CT<int> ct1 ; "
                      "} "
                      "N2 :: C c2 ; "
                      "N2 :: CT<int> ct2 ; "
                      "} "
                      "N1 :: N2 :: C c3 ; "
                      "N1 :: N2 :: CT<int> ct3 ; struct N1 :: N2 :: CT<int> { } ;", tok(code));
    }

    void template_namespace_8() { // #8768
        const char code[] = "namespace NS1 {\n"
                            "namespace NS2 {\n"
                            "    template <typename T>\n"
                            "    struct Fred {\n"
                            "        Fred();\n"
                            "        Fred(const Fred &);\n"
                            "        Fred & operator = (const Fred &);\n"
                            "        ~Fred();\n"
                            "    };\n"
                            "    template <typename T>\n"
                            "    Fred<T>::Fred() { }\n"
                            "    template <typename T>\n"
                            "    Fred<T>::Fred(const Fred<T> & f) { }\n"
                            "    template <typename T>\n"
                            "    Fred<T> & Fred<T>::operator = (const Fred<T> & f) { }\n"
                            "    template <typename T>\n"
                            "    Fred<T>::~Fred() { }\n"
                            "}\n"
                            "}\n"
                            "NS1::NS2::Fred<int> fred;";
        ASSERT_EQUALS("namespace NS1 { "
                      "namespace NS2 { "
                      "struct Fred<int> ; "
                      "} "
                      "} "
                      "NS1 :: NS2 :: Fred<int> fred ; struct NS1 :: NS2 :: Fred<int> { "
                      "Fred<int> ( ) ; "
                      "Fred<int> ( const NS1 :: NS2 :: Fred<int> & ) ; "
                      "NS1 :: NS2 :: Fred<int> & operator= ( const NS1 :: NS2 :: Fred<int> & ) ; "
                      "~ Fred<int> ( ) ; "
                      "} ; "
                      "NS1 :: NS2 :: Fred<int> :: Fred<int> ( ) { } "
                      "NS1 :: NS2 :: Fred<int> :: Fred<int> ( const NS1 :: NS2 :: Fred<int> & f ) { } "
                      "NS1 :: NS2 :: Fred<int> & NS1 :: NS2 :: Fred<int> :: operator= ( const NS1 :: NS2 :: Fred<int> & f ) { } "
                      "NS1 :: NS2 :: Fred<int> :: ~ Fred<int> ( ) { }", tok(code));
    }

    void template_namespace_9() {
        const char code[] = "namespace NS {\n"
                            "template<int type> struct Barney;\n"
                            "template<> struct Barney<1> { };\n"
                            "template<int type>\n"
                            "class Fred {\n"
                            "public:\n"
                            "  Fred();\n"
                            "private:\n"
                            "  Barney<type> m_data;\n"
                            "};\n"
                            "template class Fred<1>;\n"
                            "}\n";
        ASSERT_EQUALS("namespace NS { "
                      "struct Barney<1> ; "
                      "template < int type > struct Barney ; "
                      "struct Barney<1> { } ; "
                      "class Fred<1> ; "
                      "} "
                      "class NS :: Fred<1> { "
                      "public: "
                      "Fred<1> ( ) ; "
                      "private: "
                      "Barney<1> m_data ; "
                      "} ;", tok(code));
    }

    void template_namespace_10() {
        const char code[] = "namespace NS1 {\n"
                            "namespace NS2 {\n"
                            "template<class T>\n"
                            "class Fred {\n"
                            "    T * t;\n"
                            "public:\n"
                            "    Fred<T>() : t(nullptr) {}\n"
                            "};\n"
                            "}\n"
                            "}\n"
                            "NS1::NS2::Fred<int> fred;";
        ASSERT_EQUALS("namespace NS1 { "
                      "namespace NS2 { "
                      "class Fred<int> ; "
                      "} "
                      "} "
                      "NS1 :: NS2 :: Fred<int> fred ; class NS1 :: NS2 :: Fred<int> "
                      "{ "
                      "int * t ; "
                      "public: "
                      "Fred<int> ( ) : t ( nullptr ) { } "
                      "} ;", tok(code));
    }

    void template_namespace_11() {// #7145
        const char code[] = "namespace MyNamespace {\n"
                            "class TestClass {\n"
                            "public:\n"
                            "    TestClass() {\n"
                            "        SomeFunction();\n"
                            "        TemplatedMethod< int >( 0 );\n"
                            "    }\n"
                            "    void SomeFunction() { }\n"
                            "private:\n"
                            "    template< typename T > T TemplatedMethod(T);\n"
                            "};\n"
                            "template< typename T > T TestClass::TemplatedMethod(T t) { return t; }\n"
                            "}";
        ASSERT_EQUALS("namespace MyNamespace { "
                      "class TestClass { "
                      "public: "
                      "TestClass ( ) { "
                      "SomeFunction ( ) ; "
                      "TemplatedMethod<int> ( 0 ) ; "
                      "} "
                      "void SomeFunction ( ) { } "
                      "private: "
                      "int TemplatedMethod<int> ( int ) ; "
                      "} ; "
                      "} int MyNamespace :: TestClass :: TemplatedMethod<int> ( int t ) { return t ; }", tok(code));
    }

    void template_namespace_12() {
        const char code[] = "struct S {};\n" // #13444
                            "namespace N {\n"
                            "    template<>\n"
                            "    struct hash<S> {};\n"
                            "}\n"
                            "struct T {\n"
                            "    T(int i) : hash(i) {}\n"
                            "    int hash;\n"
                            "};\n";
        ASSERT_EQUALS("struct S { } ; "
                      "namespace N { "
                      "struct hash<S> { } ; "
                      "} "
                      "struct T { "
                      "T ( int i ) : hash ( i ) { } "
                      "int hash ; "
                      "} ;",
                      tok(code));
    }

    void template_namespace_13() {
        const char code[] = "namespace N {\n" // #13593
                            "    template<typename T>\n"
                            "    struct S {\n"
                            "        using U = T*;\n"
                            "    };\n"
                            "}\n"
                            "::N::S<int>::U u;\n";
        ASSERT_EQUALS("namespace N { "
                      "struct S<int> ; "
                      "} "
                      "int * u ; "
                      "struct N :: S<int> { } ;",
                      tok(code));
    }

    void template_pointer_type() {
        const char code[] = "template<class T> void foo(const T x) {}\n"
                            "void bar() { foo<int*>(0); }";
        ASSERT_EQUALS("void foo<int*> ( int * const x ) ; "
                      "void bar ( ) { foo<int*> ( 0 ) ; } "
                      "void foo<int*> ( int * const x ) { }", tok(code));
    }

    void template_array_type() {
        ASSERT_EQUALS("void foo<int[]> ( int [ ] x ) ; "
                      "void bar ( ) { int y [ 3 ] ; foo<int[]> ( y ) ; } "
                      "void foo<int[]> ( int [ ] x ) { }",
                      tok("template <class T> void foo(T x) {};\n"
                          "void bar() {\n"
                          "  int y[3];\n"
                          "  foo<int[]>(y);\n"
                          "}"));
        ASSERT_EQUALS("struct A<int[2]> ; "
                      "A<int[2]> y ; "
                      "struct A<int[2]> { int [ 2 ] x ; } ;",
                      tok("template <class T> struct A { T x; };\n"
                          "A<int[2]> y;"));

        // Previously resulted in:
        //   test.cpp:2:33: error: Syntax Error: AST broken, binary operator '>' doesn't have two operands. [internalAstError]
        ASSERT_EQUALS("struct A<B<int>[]> ; "
                      "struct B<B<int>> ; "
                      "struct C<B<int>> ; "
                      "C<B<int>> y ; "
                      "struct C<B<int>> : B<B<int>> { } ; "
                      "struct B<B<int>> { A<B<int>[]> x ; } ; "
                      "struct A<B<int>[]> { } ;",
                      tok("template <class  > struct A {};\n"
                          "template <class T> struct B { A<T[]> x; };\n"
                          "template <class T> struct C : B<T> {};\n"
                          "C<B<int>> y;"));
    }

    template<size_t size>
    unsigned int templateParameters(const char (&data)[size]) {
        TokenList tokenlist{settings, Standards::Language::CPP};
        tokenlist.appendFileIfNew("test.cpp");
        if (!tokenlist.createTokensFromString(data))
            return false;
        TokenizerTest tokenizer(std::move(tokenlist), *this);
        tokenizer.createLinks();
        tokenizer.splitTemplateRightAngleBrackets(false);

        for (const Token *tok1 = tokenizer.tokens(); tok1; tok1 = tok1->next()) {
            if (tok1->str() == "var1")
                (const_cast<Token *>(tok1))->varId(1);
        }

        return TemplateSimplifier::templateParameters(tokenizer.tokens()->next());
    }

    void templateParameters() {
        // Test that the function TemplateSimplifier::templateParameters works
        ASSERT_EQUALS(1U, templateParameters("X<struct C> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<union C> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<const int> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<int const *> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<const struct C> x;"));
        ASSERT_EQUALS(0U, templateParameters("X<len>>x;"));
        ASSERT_EQUALS(1U, templateParameters("X<typename> x;"));
        ASSERT_EQUALS(0U, templateParameters("X<...> x;"));
        ASSERT_EQUALS(0U, templateParameters("X<class T...> x;")); // Invalid syntax
        ASSERT_EQUALS(1U, templateParameters("X<class... T> x;"));
        ASSERT_EQUALS(0U, templateParameters("X<class, typename T...> x;")); // Invalid syntax
        ASSERT_EQUALS(2U, templateParameters("X<class, typename... T> x;"));
        ASSERT_EQUALS(2U, templateParameters("X<int(&)(), class> x;"));
        ASSERT_EQUALS(3U, templateParameters("X<char, int(*)(), bool> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<int...> x;"));
        ASSERT_EQUALS(2U, templateParameters("X<class, typename...> x;"));
        ASSERT_EQUALS(2U, templateParameters("X<1, T> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<T[]> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<T[2]> x;"));
        ASSERT_EQUALS(1U, templateParameters("X<i == 0> x;"));
        ASSERT_EQUALS(2U, templateParameters("X<int, i>=0> x;"));
        ASSERT_EQUALS(3U, templateParameters("X<int, i>=0, i - 2> x;"));
        ASSERT_EQUALS(0U, templateParameters("var1<1> x;"));
        ASSERT_EQUALS(0U, templateParameters("X<1>2;"));
        ASSERT_EQUALS(2U, templateParameters("template<typename...B,typename=SameSize<B...>> x;"));
        ASSERT_EQUALS(2U, templateParameters("template<typename...B,typename=SameSize<B...> > x;"));
        ASSERT_EQUALS(1U, templateParameters("template<template<typename>...Foo> x;"));
        ASSERT_EQUALS(1U, templateParameters("template<template<typename>> x;"));
        ASSERT_EQUALS(1U, templateParameters("template<template<template<typename>>> x;"));
        ASSERT_EQUALS(1U, templateParameters("template<template<template<template<typename>>>> x;"));
        ASSERT_EQUALS(1U, templateParameters("template<template<template<template<template<typename>>>>> x;"));
        ASSERT_EQUALS(2U, templateParameters("template<template<typename>,int> x;"));
        ASSERT_EQUALS(2U, templateParameters("template<template<template<typename>>,int> x;"));
        ASSERT_EQUALS(2U, templateParameters("template<template<template<template<typename>>>,int> x;"));
        ASSERT_EQUALS(2U, templateParameters("template<template<template<template<template<typename>>>>,int> x;"));
        ASSERT_EQUALS(2U, templateParameters("template<template<typename>...Foo,template<template<template<typename>>>> x;"));
        ASSERT_EQUALS(3U, templateParameters("template<template<typename>...Foo,int,template<template<template<typename>>>> x;"));
        ASSERT_EQUALS(4U, templateParameters("template<template<typename>...Foo,int,template<template<template<typename>>>,int> x;"));
        ASSERT_EQUALS(2U, templateParameters("template<typename S, enable_if_t<(is_compile_string<S>::value), int>> void i(S s);"));
        ASSERT_EQUALS(2U, templateParameters("template<typename c, b<(c::d), int>> void e();"));
        ASSERT_EQUALS(3U, templateParameters("template <class T, class... Args, class Tup = std::tuple<Args&...>> constexpr void f() {}")); // #11351
        ASSERT_EQUALS(3U, templateParameters("template <class T, class... Args, class Tup = std::tuple<Args&&...>> void f() {}"));
        ASSERT_EQUALS(3U, templateParameters("template <class T, class... Args, class Tup = std::tuple<Args*...>> void f() {}"));
        ASSERT_EQUALS(1U, templateParameters("S<4 < sizeof(uintptr_t)> x;"));
        ASSERT_EQUALS(2U, templateParameters("template <typename... Ts, typename = std::enable_if_t<std::is_same<Ts..., int>::value>> void g() {}")); // #11915
        ASSERT_EQUALS(1U, templateParameters("S<N::M<O<\"A\"_I>>> s;")); // #10837
    }

    // Helper function to unit test TemplateSimplifier::getTemplateNamePosition
    template<size_t size>
    int templateNamePositionHelper(const char (&data)[size], unsigned offset = 0) {
        TokenList tokenlist{settings, Standards::Language::CPP};

        tokenlist.appendFileIfNew("test.cpp");
        if (!tokenlist.createTokensFromString(data))
            return false;
        TokenizerTest tokenizer(std::move(tokenlist), *this);
        tokenizer.createLinks();
        tokenizer.splitTemplateRightAngleBrackets(false);

        const Token *_tok = tokenizer.tokens();
        for (unsigned i = 0; i < offset; ++i)
            _tok = _tok->next();
        return tokenizer.mTemplateSimplifier->getTemplateNamePosition(_tok);
    }

    void templateNamePosition() {
        // Template class
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> class A {};", 4));
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> struct A {};", 4));
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> class A : B {};", 4));
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> struct A : B {};", 4));
        // Template function definitions
        ASSERT_EQUALS(2, templateNamePositionHelper("template<class T> unsigned foo() { return 0; }", 4));
        ASSERT_EQUALS(3, templateNamePositionHelper("template<class T> unsigned* foo() { return 0; }", 4));
        ASSERT_EQUALS(4, templateNamePositionHelper("template<class T> unsigned** foo() { return 0; }", 4));

        ASSERT_EQUALS(3, templateNamePositionHelper("template<class T> const unsigned foo() { return 0; }", 4));
        ASSERT_EQUALS(4, templateNamePositionHelper("template<class T> const unsigned& foo() { return 0; }", 4));
        ASSERT_EQUALS(5, templateNamePositionHelper("template<class T> const unsigned** foo() { return 0; }", 4));

        ASSERT_EQUALS(4, templateNamePositionHelper("template<class T> std::string foo() { static str::string str; return str; }", 4));
        ASSERT_EQUALS(5, templateNamePositionHelper("template<class T> std::string & foo() { static str::string str; return str; }", 4));
        ASSERT_EQUALS(6, templateNamePositionHelper("template<class T> const std::string & foo() { static str::string str; return str; }", 4));

        ASSERT_EQUALS(9, templateNamePositionHelper("template<class T> std::map<int, int> foo() { static std::map<int, int> m; return m; }", 4));
        ASSERT_EQUALS(10, templateNamePositionHelper("template<class T> std::map<int, int> & foo() { static std::map<int, int> m; return m; }", 4));
        ASSERT_EQUALS(11, templateNamePositionHelper("template<class T> const std::map<int, int> & foo() { static std::map<int, int> m; return m; }", 4));
        // Class template members
        ASSERT_EQUALS(4, templateNamePositionHelper(
                          "class A { template<class T> unsigned foo(); }; "
                          "template<class T> unsigned A::foo() { return 0; }", 19));
        ASSERT_EQUALS(5, templateNamePositionHelper(
                          "class A { template<class T> const unsigned foo(); }; "
                          "template<class T> const unsigned A::foo() { return 0; }", 20));
        ASSERT_EQUALS(7, templateNamePositionHelper(
                          "class A { class B { template<class T> const unsigned foo(); }; } ; "
                          "template<class T> const unsigned A::B::foo() { return 0; }", 25));
        ASSERT_EQUALS(8, templateNamePositionHelper(
                          "class A { class B { template<class T> const unsigned * foo(); }; } ; "
                          "template<class T> const unsigned * A::B::foo() { return 0; }", 26));
        ASSERT_EQUALS(9, templateNamePositionHelper(
                          "class A { class B { template<class T> const unsigned ** foo(); }; } ; "
                          "template<class T> const unsigned ** A::B::foo() { return 0; }", 27));
        // Template class member
        ASSERT_EQUALS(6, templateNamePositionHelper(
                          "template<class T> class A { A(); }; "
                          "template<class T> A<T>::A() {}", 18));
        ASSERT_EQUALS(8, templateNamePositionHelper(
                          "template<class T, class U> class A { A(); }; "
                          "template<class T, class U> A<T, U>::A() {}", 24));
        ASSERT_EQUALS(7, templateNamePositionHelper(
                          "template<class T> class A { unsigned foo(); }; "
                          "template<class T> unsigned A<T>::foo() { return 0; }", 19));
        ASSERT_EQUALS(9, templateNamePositionHelper(
                          "template<class T, class U> class A { unsigned foo(); }; "
                          "template<class T, class U> unsigned A<T, U>::foo() { return 0; }", 25));
        ASSERT_EQUALS(12, templateNamePositionHelper(
                          "template<> unsigned A<int, v<char> >::foo() { return 0; }", 2));
    }

    // Helper function to unit test TemplateSimplifier::findTemplateDeclarationEnd
    template<size_t size>
    bool findTemplateDeclarationEndHelper(const char (&data)[size], const char pattern[], unsigned offset = 0) {
        TokenList tokenlist{settings, Standards::Language::CPP};
        if (!TokenListHelper::createTokensFromString(tokenlist, data, "test.cpp"))
            return false;
        TokenizerTest tokenizer(std::move(tokenlist), *this);
        tokenizer.createLinks();
        tokenizer.splitTemplateRightAngleBrackets(false);

        const Token *_tok = tokenizer.tokens();
        for (unsigned i = 0; i < offset; ++i)
            _tok = _tok->next();

        const Token *tok1 = TemplateSimplifier::findTemplateDeclarationEnd(_tok);

        return (tok1 == Token::findsimplematch(tokenizer.list.front(), pattern, strlen(pattern)));
    }

    void findTemplateDeclarationEnd() {
        ASSERT(findTemplateDeclarationEndHelper("template <typename T> class Fred { }; int x;", "; int x ;"));
        ASSERT(findTemplateDeclarationEndHelper("template <typename T> void Fred() { } int x;", "} int x ;"));
        ASSERT(findTemplateDeclarationEndHelper("template <typename T> int Fred = 0; int x;", "; int x ;"));
        ASSERT(findTemplateDeclarationEndHelper("template <typename T> constexpr auto func = [](auto x){ return T(x);}; int x;", "; int x ;"));
        ASSERT(findTemplateDeclarationEndHelper("template <class, class a> auto b() -> decltype(a{}.template b<void(int, int)>); int x;", "; int x ;"));
        ASSERT(findTemplateDeclarationEndHelper("template <class, class a> auto b() -> decltype(a{}.template b<void(int, int)>){} int x;", "} int x ;"));
        ASSERT(findTemplateDeclarationEndHelper("template <typename... f, c<h<e<typename f::d...>>::g>> void i(); int x;", "; int x ;"));
        ASSERT(findTemplateDeclarationEndHelper("template <typename... f, c<h<e<typename f::d...>>::g>> void i(){} int x;", "} int x ;"));
    }

    // Helper function to unit test TemplateSimplifier::getTemplateParametersInDeclaration
    template<size_t size>
    bool getTemplateParametersInDeclarationHelper(const char (&data)[size], const std::vector<std::string> & params) {
        TokenList tokenlist{settings, Standards::Language::CPP};

        if (!TokenListHelper::createTokensFromString(tokenlist, data, "test.cpp"))
            return false;
        TokenizerTest tokenizer(std::move(tokenlist), *this);
        tokenizer.createLinks();
        tokenizer.splitTemplateRightAngleBrackets(false);

        std::vector<const Token *> typeParametersInDeclaration;
        TemplateSimplifierTest::getTemplateParametersInDeclaration(tokenizer.tokens()->tokAt(2), typeParametersInDeclaration);

        if (params.size() != typeParametersInDeclaration.size())
            return false;

        for (size_t i = 0; i < typeParametersInDeclaration.size(); ++i) {
            if (typeParametersInDeclaration[i]->str() != params[i])
                return false;
        }
        return true;
    }

    void getTemplateParametersInDeclaration() {
        ASSERT(getTemplateParametersInDeclarationHelper("template<typename T> class Fred {};", std::vector<std::string> {"T"}));
        ASSERT(getTemplateParametersInDeclarationHelper("template<typename T=int> class Fred {};", std::vector<std::string> {"T"}));
        ASSERT(getTemplateParametersInDeclarationHelper("template<typename T,typename U> class Fred {};", std::vector<std::string> {"T","U"}));
        ASSERT(getTemplateParametersInDeclarationHelper("template<typename T,typename U=int> class Fred {};", std::vector<std::string> {"T","U"}));
        ASSERT(getTemplateParametersInDeclarationHelper("template<typename T=int,typename U=int> class Fred {};", std::vector<std::string> {"T","U"}));
    }

    void expandSpecialized1() {
        ASSERT_EQUALS("class A<int> { } ;", tok("template<> class A<int> {};"));
        ASSERT_EQUALS("class A<int> : public B { } ;", tok("template<> class A<int> : public B {};"));
        ASSERT_EQUALS("class A<int> { A<int> ( ) ; ~ A<int> ( ) ; } ;", tok("template<> class A<int> { A(); ~A(); };"));
        ASSERT_EQUALS("class A<int> { A<int> ( ) { } ~ A<int> ( ) { } } ;", tok("template<> class A<int> { A() {} ~A() {} };"));
        ASSERT_EQUALS("class A<int> { A<int> ( ) ; ~ A<int> ( ) ; } ; A<int> :: A<int> ( ) { } ~ A<int> :: A<int> ( ) { }",
                      tok("template<> class A<int> { A(); ~A(); }; A<int>::A() { } ~A<int>::A() {}"));
        ASSERT_EQUALS("class A<int> { A<int> ( ) ; A<int> ( const A<int> & ) ; A<int> foo ( ) ; } ; A<int> :: A<int> ( ) { } A<int> :: A<int> ( const A<int> & ) { } A<int> A<int> :: foo ( ) { A<int> a ; return a ; }",
                      tok("template<> class A<int> { A(); A(const A &) ; A foo(); }; A<int>::A() { } A<int>::A(const A &) { } A<int> A<int>::foo() { A a; return a; }"));
    }

    void expandSpecialized2() {
        {
            const char code[] = "template <>\n"
                                "class C<float> {\n"
                                "public:\n"
                                "   C() { }\n"
                                "   C(const C &) { }\n"
                                "   ~C() { }\n"
                                "   C & operator=(const C &) { return *this; }\n"
                                "};\n"
                                "C<float> b;\n";
            const char expected[] = "class C<float> { "
                                    "public: "
                                    "C<float> ( ) { } "
                                    "C<float> ( const C<float> & ) { } "
                                    "~ C<float> ( ) { } "
                                    "C<float> & operator= ( const C<float> & ) { return * this ; } "
                                    "} ; "
                                    "C<float> b ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <>\n"
                                "class C<float> {\n"
                                "public:\n"
                                "   C() { }\n"
                                "   C(const C &) { }\n"
                                "   ~C() { }\n"
                                "   C & operator=(const C &) { return *this; }\n"
                                "};";
            const char expected[] = "class C<float> { "
                                    "public: "
                                    "C<float> ( ) { } "
                                    "C<float> ( const C<float> & ) { } "
                                    "~ C<float> ( ) { } "
                                    "C<float> & operator= ( const C<float> & ) { return * this ; } "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <>\n"
                                "class C<float> {\n"
                                "public:\n"
                                "   C();\n"
                                "   C(const C &);\n"
                                "   ~C();\n"
                                "   C & operator=(const C &);\n"
                                "};\n"
                                "C::C() { }\n"
                                "C::C(const C &) { }\n"
                                "C::~C() { }\n"
                                "C & C::operator=(const C &) { return *this; }\n"
                                "C<float> b;\n";
            const char expected[] = "class C<float> { "
                                    "public: "
                                    "C<float> ( ) ; "
                                    "C<float> ( const C<float> & ) ; "
                                    "~ C<float> ( ) ; "
                                    "C<float> & operator= ( const C<float> & ) ; "
                                    "} ; "
                                    "C<float> :: C<float> ( ) { } "
                                    "C<float> :: C<float> ( const C<float> & ) { } "
                                    "C<float> :: ~ C<float> ( ) { } "
                                    "C<float> & C<float> :: operator= ( const C<float> & ) { return * this ; } "
                                    "C<float> b ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <>\n"
                                "class C<float> {\n"
                                "public:\n"
                                "   C();\n"
                                "   C(const C &);\n"
                                "   ~C();\n"
                                "   C & operator=(const C &);\n"
                                "};\n"
                                "C::C() { }\n"
                                "C::C(const C &) { }\n"
                                "C::~C() { }\n"
                                "C & C::operator=(const C &) { return *this; }";
            const char expected[] = "class C<float> { "
                                    "public: "
                                    "C<float> ( ) ; "
                                    "C<float> ( const C<float> & ) ; "
                                    "~ C<float> ( ) ; "
                                    "C<float> & operator= ( const C<float> & ) ; "
                                    "} ; "
                                    "C<float> :: C<float> ( ) { } "
                                    "C<float> :: C<float> ( const C<float> & ) { } "
                                    "C<float> :: ~ C<float> ( ) { } "
                                    "C<float> & C<float> :: operator= ( const C<float> & ) { return * this ; }";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void expandSpecialized3() { // #8671
        const char code[] = "template <> struct OutputU16<unsigned char> final {\n"
                            "    explicit OutputU16(std::basic_ostream<unsigned char> &t) : outputStream_(t) {}\n"
                            "    void operator()(unsigned short) const;\n"
                            "private:\n"
                            "    std::basic_ostream<unsigned char> &outputStream_;\n"
                            "};";
        const char expected[] = "struct OutputU16<unsignedchar> { "
                                "explicit OutputU16<unsignedchar> ( std :: basic_ostream < unsigned char > & t ) : outputStream_ ( t ) { } "
                                "void operator() ( unsigned short ) const ; "
                                "private: "
                                "std :: basic_ostream < unsigned char > & outputStream_ ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void expandSpecialized4() {
        {
            const char code[] = "template<> class C<char> { };\n"
                                "map<int> m;";
            const char expected[] = "class C<char> { } ; "
                                    "map < int > m ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<> class C<char> { };\n"
                                "map<int> m;\n"
                                "C<char> c;";
            const char expected[] = "class C<char> { } ; "
                                    "map < int > m ; "
                                    "C<char> c ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<typename T> class C { };\n"
                                "template<> class C<char> { };\n"
                                "map<int> m;\n";
            const char expected[] = "class C<char> ; "
                                    "template < typename T > class C { } ; "
                                    "class C<char> { } ; "
                                    "map < int > m ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<typename T> class C { };\n"
                                "template<> class C<char> { };\n"
                                "map<int> m;\n"
                                "C<int> i;";
            const char expected[] = "class C<char> ; "
                                    "class C<int> ; "
                                    "class C<char> { } ; "
                                    "map < int > m ; "
                                    "C<int> i ; "
                                    "class C<int> { } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<typename T> class C { };\n"
                                "template<> class C<char> { };\n"
                                "map<int> m;\n"
                                "C<int> i;\n"
                                "C<char> c;";
            const char expected[] = "class C<char> ; "
                                    "class C<int> ; "
                                    "class C<char> { } ; "
                                    "map < int > m ; "
                                    "C<int> i ; "
                                    "C<char> c ; "
                                    "class C<int> { } ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "class A {};\n"
                                "template<typename T> struct B;\n"
                                "template<> struct B<A> {};\n"
                                "int f() {\n"
                                "    int B[1] = {};\n"
                                "    return B[0];\n"
                                "}\n";
            const char expected[] = "class A { } ; "
                                    "struct B<A> ; "
                                    "template < typename T > struct B ; "
                                    "struct B<A> { } ; "
                                    "int f ( ) { "
                                    "int B [ 1 ] = { } ; "
                                    "return B [ 0 ] ; "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void expandSpecialized5() {
        const char code[] = "template<typename T> class hash;\n" // #10494
                            "template<> class hash<int> {};\n"
                            "int f(int i) {\n"
                            "    int hash = i;\n"
                            "    const int a[2]{};\n"
                            "    return a[hash];\n"
                            "}\n";

        const char expected[] = "class hash<int> ; "
                                "template < typename T > class hash ; "
                                "class hash<int> { } ; "
                                "int f ( int i ) { "
                                "int hash ; hash = i ; "
                                "const int a [ 2 ] { } ; "
                                "return a [ hash ] ; "
                                "}";

        ASSERT_EQUALS(expected, tok(code));
    }

    void templateAlias1() {
        const char code[] = "template<class T, int N> struct Foo {};\n"
                            "template<class T> using Bar = Foo<T,3>;\n"
                            "Bar<int> b;\n";

        const char expected[] = "struct Foo<int,3> ; "
                                "Foo<int,3> b ; "
                                "struct Foo<int,3> { } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void templateAlias2() {
        const char code[] = "namespace A { template<class T, int N> struct Foo {}; }\n"
                            "template<class T> using Bar = A::Foo<T,3>;\n"
                            "Bar<int> b;\n";

        const char expected[] = "namespace A { struct Foo<int,3> ; } "
                                "A :: Foo<int,3> b ; "
                                "struct A :: Foo<int,3> { } ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void templateAlias3() { // #8315
        const char code[] = "template <int> struct Tag {};\n"
                            "template <int ID> using SPtr = std::shared_ptr<void(Tag<ID>)>;\n"
                            "SPtr<0> s;";
        const char expected[] = "struct Tag<0> ; "
                                "std :: shared_ptr < void ( Tag<0> ) > s ; "
                                "struct Tag<0> { } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void templateAlias4() { // #9070
        const char code[] = "template <class T>\n"
                            "using IntrusivePtr = boost::intrusive_ptr<T>;\n"
                            "template <class T> class Vertex { };\n"
                            "IntrusivePtr<Vertex<int>> p;";
        const char expected[] = "class Vertex<int> ; "
                                "boost :: intrusive_ptr < Vertex<int> > p ; "
                                "class Vertex<int> { } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void templateAlias5() {
        const char code[] = "template<typename T> using A = int;\n"
                            "template<typename T> using B = T;\n"
                            "A<char> a;\n"
                            "B<char> b;";
        const char expected[] = "int a ; "
                                "char b ;";
        ASSERT_EQUALS(expected, tok(code));
    }

#define instantiateMatch(...) instantiateMatch_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    bool instantiateMatch_(const char* file, int line, const char (&code)[size], const std::size_t numberOfArguments, const char patternAfter[]) {
        SimpleTokenizer tokenizer(settings, *this);

        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        return (TemplateSimplifierTest::instantiateMatch)(tokenizer.tokens(), numberOfArguments, false, patternAfter);
    }

    void instantiateMatchTest() {
        // Ticket #8175
        ASSERT_EQUALS(false,
                      instantiateMatch("ConvertHelper < From, To > c ;",
                                       2, ":: %name% ("));
        ASSERT_EQUALS(true,
                      instantiateMatch("ConvertHelper < From, To > :: Create ( ) ;",
                                       2, ":: %name% ("));
        ASSERT_EQUALS(false,
                      instantiateMatch("integral_constant < bool, sizeof ( ConvertHelper < From, To > :: Create ( ) ) > ;",
                                       2, ":: %name% ("));
        ASSERT_EQUALS(false,
                      instantiateMatch("integral_constant < bool, sizeof ( ns :: ConvertHelper < From, To > :: Create ( ) ) > ;",
                                       2, ":: %name% ("));
    }

    void templateParameterWithoutName() {
        ASSERT_EQUALS(1U, templateParameters("template<typename = void> struct s;"));
        ASSERT_EQUALS(1U, templateParameters("template<template<typename = float> typename T> struct A {\n"
                                             "    void f();\n"
                                             "    void g();\n"
                                             "};n"));
    }

    void templateTypeDeduction1() { // #8962
        const char code[] = "template<typename T>\n"
                            "void f(T n) { (void)n; }\n"
                            "static void func() {\n"
                            "    f(0);\n"
                            "    f(0u);\n"
                            "    f(0U);\n"
                            "    f(0l);\n"
                            "    f(0L);\n"
                            "    f(0ul);\n"
                            "    f(0UL);\n"
                            "    f(0ll);\n"
                            "    f(0LL);\n"
                            "    f(0ull);\n"
                            "    f(0ULL);\n"
                            "    f(0.0);\n"
                            "    f(0.0f);\n"
                            "    f(0.0F);\n"
                            "    f(0.0l);\n"
                            "    f(0.0L);\n"
                            "    f('c');\n"
                            "    f(L'c');\n"
                            "    f(\"string\");\n"
                            "    f(L\"string\");\n"
                            "    f(true);\n"
                            "    f(false);\n"
                            "}";
        const char expected[] = "void f<int> ( int n ) ; "
                                "void f<unsignedint> ( unsigned int n ) ; "
                                "void f<long> ( long n ) ; "
                                "void f<unsignedlong> ( unsigned long n ) ; "
                                "void f<longlong> ( long long n ) ; "
                                "void f<unsignedlonglong> ( unsigned long long n ) ; "
                                "void f<double> ( double n ) ; "
                                "void f<float> ( float n ) ; "
                                "void f<longdouble> ( long double n ) ; "
                                "void f<char> ( char n ) ; "
                                "void f<wchar_t> ( wchar_t n ) ; "
                                "void f<constchar*> ( const char * n ) ; "
                                "void f<constwchar_t*> ( const wchar_t * n ) ; "
                                "void f<bool> ( bool n ) ; "
                                "static void func ( ) { "
                                "f<int> ( 0 ) ; "
                                "f<unsignedint> ( 0u ) ; "
                                "f<unsignedint> ( 0U ) ; "
                                "f<long> ( 0l ) ; "
                                "f<long> ( 0L ) ; "
                                "f<unsignedlong> ( 0ul ) ; "
                                "f<unsignedlong> ( 0UL ) ; "
                                "f<longlong> ( 0ll ) ; "
                                "f<longlong> ( 0LL ) ; "
                                "f<unsignedlonglong> ( 0ull ) ; "
                                "f<unsignedlonglong> ( 0ULL ) ; "
                                "f<double> ( 0.0 ) ; "
                                "f<float> ( 0.0f ) ; "
                                "f<float> ( 0.0F ) ; "
                                "f<longdouble> ( 0.0l ) ; "
                                "f<longdouble> ( 0.0L ) ; "
                                "f<char> ( 'c' ) ; "
                                "f<wchar_t> ( L'c' ) ; "
                                "f<constchar*> ( \"string\" ) ; "
                                "f<constwchar_t*> ( L\"string\" ) ; "
                                "f<bool> ( true ) ; "
                                "f<bool> ( false ) ; "
                                "} "
                                "void f<int> ( int n ) { ( void ) n ; } "
                                "void f<unsignedint> ( unsigned int n ) { ( void ) n ; } "
                                "void f<long> ( long n ) { ( void ) n ; } "
                                "void f<unsignedlong> ( unsigned long n ) { ( void ) n ; } "
                                "void f<longlong> ( long long n ) { ( void ) n ; } "
                                "void f<unsignedlonglong> ( unsigned long long n ) { ( void ) n ; } "
                                "void f<double> ( double n ) { ( void ) n ; } "
                                "void f<float> ( float n ) { ( void ) n ; } "
                                "void f<longdouble> ( long double n ) { ( void ) n ; } "
                                "void f<char> ( char n ) { ( void ) n ; } "
                                "void f<wchar_t> ( wchar_t n ) { ( void ) n ; } "
                                "void f<constchar*> ( const char * n ) { ( void ) n ; } "
                                "void f<constwchar_t*> ( const wchar_t * n ) { ( void ) n ; } "
                                "void f<bool> ( bool n ) { ( void ) n ; }";

        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void templateTypeDeduction2() {
        const char code[] = "template<typename T, typename U>\n"
                            "void f(T t, U u) { }\n"
                            "static void func() {\n"
                            "    f(0, 0.0);\n"
                            "    f(0.0, 0);\n"
                            "}";

        const char expected[] = "void f<int,double> ( int t , double u ) ; "
                                "void f<double,int> ( double t , int u ) ; "
                                "static void func ( ) { "
                                "f<int,double> ( 0 , 0.0 ) ; "
                                "f<double,int> ( 0.0, 0 ) ; "
                                "void f<int,double> ( int t , double u ) { } "
                                "void f<double,int> ( double t , int u ) { } ";

        const char actual[] = "template < typename T , typename U > "
                              "void f ( T t , U u ) { } "
                              "static void func ( ) { "
                              "f ( 0 , 0.0 ) ; "
                              "f ( 0.0 , 0 ) ; "
                              "}";

        TODO_ASSERT_EQUALS(expected, actual, tok(code));
    }

    void templateTypeDeduction3() {  // #9975
        const char code[] = "struct A {\n"
                            "    int a = 1;\n"
                            "    void f() { g(1); }\n"
                            "    template <typename T> void g(T x) { a = 2; }\n"
                            "};\n"
                            "int main() {\n"
                            "    A a;\n"
                            "    a.f();\n"
                            "}";
        const char exp[]  = "struct A { "
                            "int a ; a = 1 ; "
                            "void f ( ) { g<int> ( 1 ) ; } "
                            "void g<int> ( int x ) ; "
                            "} ; "
                            "int main ( ) { "
                            "A a ; "
                            "a . f ( ) ; "
                            "} void A :: g<int> ( int x ) { a = 2 ; }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void templateTypeDeduction4() {  // #9983
        {
            const char code[] = "int a = 1;\n"
                                "template <typename T> void f(T x, T y) { a = x + y; }\n"
                                "void test() { f(0, 0); }";
            const char exp[]  = "int a ; a = 1 ; "
                                "void f<int> ( int x , int y ) ; "
                                "void test ( ) { f<int> ( 0 , 0 ) ; } "
                                "void f<int> ( int x , int y ) { a = x + y ; }";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "int a = 1;\n"
                                "template <typename T> void f(T x, double y) { a = x + y; }\n"
                                "void test() { f(0, 0.0); }";
            const char exp[]  = "int a ; a = 1 ; "
                                "void f<int> ( int x , double y ) ; "
                                "void test ( ) { f<int> ( 0 , 0.0 ) ; } "
                                "void f<int> ( int x , double y ) { a = x + y ; }";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "int a = 1;\n"
                                "template <typename T> void f(double x, T y) { a = x + y; }\n"
                                "void test() { f(0.0, 0); }";
            const char exp[]  = "int a ; a = 1 ; "
                                "void f<int> ( double x , int y ) ; "
                                "void test ( ) { f<int> ( 0.0 , 0 ) ; } "
                                "void f<int> ( double x , int y ) { a = x + y ; }";
            ASSERT_EQUALS(exp, tok(code));
        }

        {
            const char code[] = "int a = 1;\n"
                                "template <typename T> void f(double x, T y) { a = x + y; }\n"
                                "template <typename T> void f(int x, T y) { a = x + y; }\n"
                                "void test() {\n"
                                "    f(0, 0);\n"
                                "    f(0.0, 0);\n"
                                "    f(0, 0.0);\n"
                                "    f(0.0, 0.0);\n"
                                "}";
            const char exp[]  = "int a ; a = 1 ; "
                                "void f<int> ( int x , int y ) ; "
                                "void f<int> ( double x , int y ) ; "
                                "void f<double> ( int x , double y ) ; "
                                "void f<double> ( double x , double y ) ; "
                                "void test ( ) { "
                                "f<int> ( 0 , 0 ) ; "
                                "f<int> ( 0.0 , 0 ) ; "
                                "f<double> ( 0 , 0.0 ) ; "
                                "f<double> ( 0.0 , 0.0 ) ; "
                                "} "
                                "void f<int> ( int x , int y ) { a = x + y ; } "
                                "void f<int> ( double x , int y ) { a = x + y ; } "
                                "void f<double> ( int x , double y ) { a = x + y ; } "
                                "void f<double> ( double x , double y ) { a = x + y ; }";

            const char act[]  = "int a ; a = 1 ; "
                                "template < typename T > void f ( double x , T y ) { a = x + y ; } "
                                "void f<int> ( int x , int y ) ; void f<double> ( int x , double y ) ; "
                                "void test ( ) { "
                                "f<int> ( 0 , 0 ) ; "
                                "f<int> ( 0.0 , 0 ) ; "
                                "f<double> ( 0 , 0.0 ) ; "
                                "f<double> ( 0.0 , 0.0 ) ; "
                                "} "
                                "void f<int> ( int x , int y ) { a = x + y ; } "
                                "void f<double> ( int x , double y ) { a = x + y ; }";
            TODO_ASSERT_EQUALS(exp, act, tok(code));
        }
        {
            const char code[] = "int a = 1;\n"
                                "template <typename T, typename U> void f(T x, U y) { a = x + y; }\n"
                                "void test() { f(0, 0.0); }";
            const char exp[]  = "int a ; a = 1 ; "
                                "void f<int,double> ( int x , double y ) ; "
                                "void test ( ) { f<int,double> ( 0 , 0.0 ) ; } "
                                "void f<int,double> ( int x , double y ) { a = x + y ; }";
            const char act[]  = "int a ; a = 1 ; "
                                "template < typename T , typename U > void f ( T x , U y ) { a = x + y ; } "
                                "void test ( ) { f ( 0 , 0.0 ) ; }";
            TODO_ASSERT_EQUALS(exp, act, tok(code));
        }
    }

    void templateTypeDeduction5() {
        {
            const char code[] = "class Fred {\n"
                                "public:\n"
                                "    template <class T> Fred(T t) { }\n"
                                "};\n"
                                "Fred fred1 = Fred(0);\n"
                                "Fred fred2 = Fred(0.0);\n"
                                "Fred fred3 = Fred(\"zero\");\n"
                                "Fred fred4 = Fred(false);";
            const char exp[]  = "class Fred { "
                                "public: "
                                "Fred<int> ( int t ) ; "
                                "Fred<double> ( double t ) ; "
                                "Fred<constchar*> ( const char * t ) ; "
                                "Fred<bool> ( bool t ) ; "
                                "} ; "
                                "Fred fred1 ; fred1 = Fred<int> ( 0 ) ; "
                                "Fred fred2 ; fred2 = Fred<double> ( 0.0 ) ; "
                                "Fred fred3 ; fred3 = Fred<constchar*> ( \"zero\" ) ; "
                                "Fred fred4 ; fred4 = Fred<bool> ( false ) ; "
                                "Fred :: Fred<int> ( int t ) { } "
                                "Fred :: Fred<double> ( double t ) { } "
                                "Fred :: Fred<constchar*> ( const char * t ) { } "
                                "Fred :: Fred<bool> ( bool t ) { }";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "namespace NS {\n"
                                "class Fred {\n"
                                "public:\n"
                                "    template <class T> Fred(T t) { }\n"
                                "};\n"
                                "Fred fred1 = Fred(0);\n"
                                "Fred fred2 = Fred(0.0);\n"
                                "Fred fred3 = Fred(\"zero\");\n"
                                "Fred fred4 = Fred(false);\n"
                                "}\n"
                                "NS::Fred fred1 = NS::Fred(0);\n"
                                "NS::Fred fred2 = NS::Fred(0.0);\n"
                                "NS::Fred fred3 = NS::Fred(\"zero\");\n"
                                "NS::Fred fred4 = NS::Fred(false);\n";
            const char exp[]  = "namespace NS { "
                                "class Fred { "
                                "public: "
                                "Fred<int> ( int t ) ; "
                                "Fred<double> ( double t ) ; "
                                "Fred<constchar*> ( const char * t ) ; "
                                "Fred<bool> ( bool t ) ; "
                                "} ; "
                                "Fred fred1 ; fred1 = Fred<int> ( 0 ) ; "
                                "Fred fred2 ; fred2 = Fred<double> ( 0.0 ) ; "
                                "Fred fred3 ; fred3 = Fred<constchar*> ( \"zero\" ) ; "
                                "Fred fred4 ; fred4 = Fred<bool> ( false ) ; "
                                "} "
                                "NS :: Fred fred1 ; fred1 = NS :: Fred<int> ( 0 ) ; "
                                "NS :: Fred fred2 ; fred2 = NS :: Fred<double> ( 0.0 ) ; "
                                "NS :: Fred fred3 ; fred3 = NS :: Fred<constchar*> ( \"zero\" ) ; "
                                "NS :: Fred fred4 ; fred4 = NS :: Fred<bool> ( false ) ; "
                                "NS :: Fred :: Fred<int> ( int t ) { } "
                                "NS :: Fred :: Fred<double> ( double t ) { } "
                                "NS :: Fred :: Fred<constchar*> ( const char * t ) { } "
                                "NS :: Fred :: Fred<bool> ( bool t ) { }";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void simplifyTemplateArgs1() {
        ASSERT_EQUALS("foo<2> = 2 ; foo<2> ;", tok("template<int N> foo = N; foo < ( 2 ) >;"));
        ASSERT_EQUALS("foo<2> = 2 ; foo<2> ;", tok("template<int N> foo = N; foo < 1 + 1 >;"));
        ASSERT_EQUALS("foo<2> = 2 ; foo<2> ;", tok("template<int N> foo = N; foo < ( 1 + 1 ) >;"));

        ASSERT_EQUALS("foo<2,2> = 4 ; foo<2,2> ;", tok("template<int N, int M> foo = N * M; foo < ( 2 ), ( 2 ) >;"));
        ASSERT_EQUALS("foo<2,2> = 4 ; foo<2,2> ;", tok("template<int N, int M> foo = N * M; foo < 1 + 1, 1 + 1 >;"));
        ASSERT_EQUALS("foo<2,2> = 4 ; foo<2,2> ;", tok("template<int N, int M> foo = N * M; foo < ( 1 + 1 ), ( 1 + 1 ) >;"));

        ASSERT_EQUALS("foo<true> = true ; foo<true> ;", tok("template<bool N> foo = N; foo < true ? true : false >;"));
        ASSERT_EQUALS("foo<false> = false ; foo<false> ;", tok("template<bool N> foo = N; foo < false ? true : false >;"));
        ASSERT_EQUALS("foo<true> = true ; foo<true> ;", tok("template<bool N> foo = N; foo < 1 ? true : false >;"));
        ASSERT_EQUALS("foo<false> = false ; foo<false> ;", tok("template<bool N> foo = N; foo < 0 ? true : false >;"));
        ASSERT_EQUALS("foo<true> = true ; foo<true> ;", tok("template<bool N> foo = N; foo < (1 + 1 ) ? true : false >;"));
        ASSERT_EQUALS("foo<false> = false ; foo<false> ;", tok("template<bool N> foo = N; foo < ( 1 - 1) ? true : false >;"));
    }

    void simplifyTemplateArgs2() {
        const char code[] = "template<bool T> struct a_t { static const bool t = T; };\n"
                            "typedef a_t<sizeof(void*) == sizeof(char)> a;\n"
                            "void foo() { bool b = a::t; }";
        const char expected[] = "struct a_t<false> ; "
                                "void foo ( ) { bool b ; b = a_t<false> :: t ; } "
                                "struct a_t<false> { static const bool t = false ; } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyTemplateArgs3() { // #11418
        const char code[] = "template <class T> struct S {};\n"
                            "template<typename T>\n"
                            "T f() {}\n"
                            "template<typename T, typename U>\n"
                            "void g() {\n"
                            "    S<decltype(true ? f<T>() : f<U>())> s1;\n"
                            "    S<decltype(false ? f<T>() : f<U>())> s2;\n"
                            "}\n"
                            "void h() {\n"
                            "    g<int, char>();\n"
                            "}\n";
        const char expected[] = "struct S<decltype((f<int>()))> ; "
                                "struct S<decltype(f<char>())> ; "
                                "int f<int> ( ) ; "
                                "char f<char> ( ) ; "
                                "void g<int,char> ( ) ; "
                                "void h ( ) { g<int,char> ( ) ; } "
                                "void g<int,char> ( ) { "
                                "S<decltype((f<int>()))> s1 ; "
                                "S<decltype(f<char>())> s2 ; "
                                "} "
                                "int f<int> ( ) { } "
                                "char f<char> ( ) { } "
                                "struct S<decltype((f<int>()))> { } ; "
                                "struct S<decltype(f<char>())> { } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template_variadic_1() { // #9144
        const char code[] = "template <typename...> struct e {};\n"
                            "static_assert(sizeof(e<>) == sizeof(e<int,int>), \"\");";
        const char expected[] = "struct e<> ; struct e<int,int> ; "
                                "static_assert ( sizeof ( e<> ) == sizeof ( e<int,int> ) , \"\" ) ; "
                                "struct e<> { } ; struct e<int,int> { } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template_variadic_2() { // #4349
        const char code[] = "template<typename T, typename... Args>\n"
                            "void printf(const char *s, T value, Args... args) {}\n"
                            "\n"
                            "int main() {\n"
                            "    printf<int, float>(\"\", foo, bar);\n"
                            "}";
        const char expected[] = "void printf<int,float> ( const char * s , int value , float ) ; "
                                "int main ( ) { printf<int,float> ( \"\" , foo , bar ) ; } "
                                "void printf<int,float> ( const char * s , int value , float ) { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template_variadic_3() { // #6172
        const char code[] = "template<int N, int ... M> struct A { "
                            "  static void foo() { "
                            "    int i = N; "
                            "  } "
                            "}; "
                            "void bar() { "
                            "  A<0>::foo(); "
                            "}";
        const char expected[] = "struct A<0> ; "
                                "void bar ( ) { A<0> :: foo ( ) ; } "
                                "struct A<0> { static void foo ( ) { int i ; i = ( int ) 0 ; } } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void template_variadic_4() { // #11763
        const char code[] = "template <int... N>\n"
                            "class E {\n"
                            "    template <int... I>\n"
                            "    int f(int n, std::integer_sequence<int, I...>) {\n"
                            "        return (((I == n) ? N : 0) + ...);\n"
                            "    }\n"
                            "};\n"
                            "E<1, 3> e;\n";
        const char expected[] = "class E<1,3> ; E<1,3> e ; "
                                "class E<1,3> { "
                                "template < int ... I > "
                                "int f ( int n , std :: integer_sequence < int , I ... > ) { "
                                "return ( ( ( I == n ) ? ( 1 , 3 ) : 0 ) + ... ) ; "  // TODO the simplification is not quite correct
                                "} "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));

        const char code2[] = "template <typename T>\n" // #13929
                             "struct S {};\n"
                             "template <typename T, template<typename...> typename C = S>\n"
                             "struct A {\n"
                             "    using x = C<T>;\n"
                             "};\n"
                             "A<int> a;\n";
        const char expected2[] = "template < typename T > "
                                 "struct S { } ; "
                                 "struct A<int,S> ; "
                                 "A<int,S> a ; "
                                 "struct A<int,S> { } ;";
        ASSERT_EQUALS(expected2, tok(code2));

        const char code3[] = "template <int... N>\n" // #14477
                             "    int f() {\n"
                             "    return (0 | ... | (1, 2, 4));\n"
                             "}\n"
                             "int main() {\n"
                             "    return f<1, 2, 4>();\n"
                             "}\n";
        const char expected3[] = "int f<1,2,4> ( ) ; "
                                 "int main ( ) { return f<1,2,4> ( ) ; } "
                                 "int f<1,2,4> ( ) { return ( 0 | ... | ( 1 , 2 , 4 ) ) ; }";
        ASSERT_EQUALS(expected3, tok(code3));
    }

    void template_variable_1() {
        {
            const char code[] = "template <int N> const int foo = N*N;\n"
                                "int x = foo<7>;";
            const char expected[] = "const int foo<7> = 49 ; "
                                    "int x ; x = foo<7> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <int> const int foo = 7;\n"
                                "int x = foo<7>;";
            const char expected[] = "const int foo<7> = 7 ; "
                                    "int x ; x = foo<7> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <int N = 7> const int foo = N*N;\n"
                                "int x = foo<7>;";
            const char expected[] = "const int foo<7> = 49 ; "
                                    "int x ; x = foo<7> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template <int N = 7> const int foo = N*N;\n"
                                "int x = foo<>;";
            const char expected[] = "const int foo<7> = 49 ; "
                                    "int x ; x = foo<7> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void template_variable_2() {
        {
            const char code[] = "template<class T> constexpr T pi = T(3.1415926535897932385L);\n"
                                "float x = pi<float>;";
            const char expected[] = "constexpr float pi<float> = float ( 3.1415926535897932385L ) ; "
                                    "float x ; x = pi<float> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<class> constexpr float pi = float(3.1415926535897932385L);\n"
                                "float x = pi<float>;";
            const char expected[] = "constexpr float pi<float> = float ( 3.1415926535897932385L ) ; "
                                    "float x ; x = pi<float> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<class T = float> constexpr T pi = T(3.1415926535897932385L);\n"
                                "float x = pi<float>;";
            const char expected[] = "constexpr float pi<float> = float ( 3.1415926535897932385L ) ; "
                                    "float x ; x = pi<float> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<class T = float> constexpr T pi = T(3.1415926535897932385L);\n"
                                "float x = pi<>;";
            const char expected[] = "constexpr float pi<float> = float ( 3.1415926535897932385L ) ; "
                                    "float x ; x = pi<float> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void template_variable_3() {
        {
            const char code[] = "template<class T, int N> constexpr T foo = T(N*N);\n"
                                "float x = foo<float,7>;";
            const char expected[] = "constexpr float foo<float,7> = float ( 49 ) ; "
                                    "float x ; x = foo<float,7> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<class,int> constexpr float foo = float(7);\n"
                                "float x = foo<float,7>;";
            const char expected[] = "constexpr float foo<float,7> = float ( 7 ) ; "
                                    "float x ; x = foo<float,7> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<class T = float, int N = 7> constexpr T foo = T(7);\n"
                                "double x = foo<double, 14>;";
            const char expected[] = "constexpr double foo<double,14> = double ( 7 ) ; "
                                    "double x ; x = foo<double,14> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<class T = float, int N = 7> constexpr T foo = T(7);\n"
                                "float x = foo<>;";
            const char expected[] = "constexpr float foo<float,7> = float ( 7 ) ; "
                                    "float x ; x = foo<float,7> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
        {
            const char code[] = "template<class T = float, int N = 7> constexpr T foo = T(7);\n"
                                "double x = foo<double>;";
            const char expected[] = "constexpr double foo<double,7> = double ( 7 ) ; "
                                    "double x ; x = foo<double,7> ;";
            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void template_variable_4() {
        const char code[] = "template<typename T> void test() { }\n"
                            "template<typename T> decltype(test<T>)* foo = &(test<T>);\n"
                            "void bar() { foo<int>(); }";
        const char expected[] = "void test<int> ( ) ; "
                                "decltype ( test<int> ) * foo<int> = & ( test<int> ) ; "
                                "void bar ( ) { foo<int> ( ) ; } "
                                "void test<int> ( ) { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyDecltype() {
        const char code[] = "template<typename T> class type { };\n"
                            "type<decltype(true)> b;\n"
                            "type<decltype(0)> i;\n"
                            "type<decltype(0U)> ui;\n"
                            "type<decltype(0L)> l;\n"
                            "type<decltype(0UL)> ul;\n"
                            "type<decltype(0LL)> ll;\n"
                            "type<decltype(0ULL)> ull;\n"
                            "type<decltype(0.0)> d;\n"
                            "type<decltype(0.0F)> f;\n"
                            "type<decltype(0.0L)> ld;";
        const char expected[] = "class type<bool> ; "
                                "class type<int> ; "
                                "class type<unsignedint> ; "
                                "class type<long> ; "
                                "class type<unsignedlong> ; "
                                "class type<longlong> ; "
                                "class type<unsignedlonglong> ; "
                                "class type<double> ; "
                                "class type<float> ; "
                                "class type<longdouble> ; "
                                "type<bool> b ; "
                                "type<int> i ; "
                                "type<unsignedint> ui ; "
                                "type<long> l ; "
                                "type<unsignedlong> ul ; "
                                "type<longlong> ll ; "
                                "type<unsignedlonglong> ull ; "
                                "type<double> d ; "
                                "type<float> f ; "
                                "type<longdouble> ld ; "
                                "class type<bool> { } ; "
                                "class type<int> { } ; "
                                "class type<unsignedint> { } ; "
                                "class type<long> { } ; "
                                "class type<unsignedlong> { } ; "
                                "class type<longlong> { } ; "
                                "class type<unsignedlonglong> { } ; "
                                "class type<double> { } ; "
                                "class type<float> { } ; "
                                "class type<longdouble> { } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void castInExpansion() {
        const char code[] = "template <int N> class C { };\n"
                            "template <typename TC> class Base {};\n"
                            "template <typename TC> class Derived : private Base<TC> {};\n"
                            "typedef Derived<C<static_cast<int>(-1)> > C_;\n"
                            "class C3 { C_ c; };";
        const char expected[] = "template < int N > class C { } ; "
                                "class Base<C<static_cast<int>(-1)>> ; "
                                "class Derived<C<static_cast<int>(-1)>> ; "
                                "class C3 { Derived<C<static_cast<int>(-1)>> c ; } ; "
                                "class Derived<C<static_cast<int>(-1)>> : private Base<C<static_cast<int>(-1)>> { } ; "
                                "class Base<C<static_cast<int>(-1)>> { } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void fold_expression_1() {
        const char code[] = "template<typename... Args> bool all(Args... args) { return (... && args); }\n"
                            "x=all(true,false,true,true);";
        const char expected[] = "template < typename ... Args > bool all ( Args ... args ) { return ( __cppcheck_fold_&&__ ( args ... ) ) ; } x = all ( true , false , true , true ) ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void fold_expression_2() {
        const char code[] = "template<typename... Args> bool all(Args... args) { return (args && ...); }\n"
                            "x=all(true,false,true,true);";
        const char expected[] = "template < typename ... Args > bool all ( Args ... args ) { return ( __cppcheck_fold_&&__ ( args ... ) ) ; } x = all ( true , false , true , true ) ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void fold_expression_3() {
        const char code[] = "template<typename... Args> int foo(Args... args) { return (12 * ... * args); }\n"
                            "x=foo(1,2);";
        const char expected[] = "template < typename ... Args > int foo ( Args ... args ) { return ( __cppcheck_fold_*__ ( args ... ) ) ; } x = foo ( 1 , 2 ) ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void fold_expression_4() {
        const char code[] = "template<typename... Args> int foo(Args... args) { return (args * ... * 123); }\n"
                            "x=foo(1,2);";
        const char expected[] = "template < typename ... Args > int foo ( Args ... args ) { return ( __cppcheck_fold_*__ ( args ... ) ) ; } x = foo ( 1 , 2 ) ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void concepts1() {
        const char code[] = "template <my_concept T> void f(T v) {}\n"
                            "f<int>(123);";
        const char expected[] = "void f<int> ( int v ) ; f<int> ( 123 ) ; void f<int> ( int v ) { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void requires1() {
        const char code[] = "template <class T> requires my_concept<T> void f(T v) {}\n"
                            "f<int>(123);";
        const char expected[] = "void f<int> ( int v ) ; f<int> ( 123 ) ; void f<int> ( int v ) { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void requires2() {
        const char code[] = "template<class T> requires (sizeof(T) > 1 && get_value<T>()) void f(T v){}\n"
                            "f<int>(123);";
        const char expected[] = "void f<int> ( int v ) ; f<int> ( 123 ) ; void f<int> ( int v ) { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void requires3() {
        const char code[] = "template<class T> requires c1<T> && c2<T> void f(T v){}\n"
                            "f<int>(123);";
        const char expected[] = "void f<int> ( int v ) ; f<int> ( 123 ) ; void f<int> ( int v ) { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void requires4() {
        const char code[] = "template <class T> void f(T v) requires my_concept<T> {}\n"
                            "f<int>(123);";
        const char expected[] = "void f<int> ( int v ) ; f<int> ( 123 ) ; void f<int> ( int v ) { }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void requires5() {
        const char code[] = "template <class T>\n"
                            "  requires requires (T x) { x + x; }\n"
                            "  T add(T a, T b) { return a + b; }\n"
                            "add<int>(123,456);";
        const char expected[] = "int add<int> ( int a , int b ) ; add<int> ( 123 , 456 ) ; int add<int> ( int a , int b ) { return a + b ; }";
        ASSERT_EQUALS(expected, tok(code));
    }

    void explicitBool1() {
        const char code[] = "class Fred { explicit(true) Fred(int); };";
        ASSERT_EQUALS("class Fred { explicit Fred ( int ) ; } ;", tok(code));
    }

    void explicitBool2() {
        const char code[] = "class Fred { explicit(false) Fred(int); };";
        ASSERT_EQUALS("class Fred { Fred ( int ) ; } ;", tok(code));
    }

    void templateArgPreserveType() { // #13882 - type of template argument
        const char code[] = "template <uint32_t x> class Test {\n"
                            "    uint32_t i = x;\n"
                            "};\n"
                            "Test<64> test;\n";
        ASSERT_EQUALS("class Test<64> ; "
                      "Test<64> test ; "
                      "class Test<64> { uint32_t i ; i = ( uint32_t ) 64 ; } ;",
                      tok(code));
    }

    void dumpTemplateArgFrom() {
        const char code[] = "template<class T> void foo(T t) {}\n"
                            "foo<int>(23);";
        const std::string d = dump(code);
        ASSERT(!d.empty());

        // Assert that first 'int' token has templateArg location info
        const std::string::size_type strpos1 = d.find(" str=\"int\" ");
        ASSERT(strpos1 < d.size());
        const std::string::size_type endpos1 = d.find('>', strpos1);
        const std::string::size_type templateArgPos1 = d.find(" templateArgFileIndex=\"0\" templateArgLineNumber=\"2\" templateArgColumn=\"5\"");
        ASSERT(templateArgPos1 > strpos1 && templateArgPos1 < endpos1);

        // Assert that second 'int' token has templateArg location info
        const std::string::size_type strpos2 = d.find(" str=\"int\" ", endpos1);
        ASSERT(strpos2 < d.size());
        const std::string::size_type endpos2 = d.find('>', strpos2);
        const std::string::size_type templateArgPos2 = d.find(" templateArgFileIndex=\"0\" templateArgLineNumber=\"2\" templateArgColumn=\"5\"", endpos1);
        ASSERT(templateArgPos2 > strpos2 && templateArgPos2 < endpos2);

        // Assert there is no further unexpected templateArg location info
        ASSERT(d.find(" templateArg", endpos2) == std::string::npos);
    }
};

REGISTER_TEST(TestSimplifyTemplate)
