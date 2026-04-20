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

#include "check.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"
#include "token.h"

#include <cstddef>
#include <list>
#include <string>

class TestGarbage : public TestFixture {
public:
    TestGarbage() : TestFixture("TestGarbage") {}

private:
    /*const*/ Settings settings = settingsBuilder().debugwarnings().build();

    void run() override {
        settings.severity.fill();
        settings.certainty.fill();

        mNewTemplate = true;

        // don't freak out when the syntax is wrong

        TEST_CASE(final_class_x);
        TEST_CASE(wrong_syntax1);
        TEST_CASE(wrong_syntax2);
        TEST_CASE(wrong_syntax3); // #3544
        TEST_CASE(wrong_syntax4); // #3618
        TEST_CASE(wrong_syntax_if_macro);  // #2518 - if MACRO()
        TEST_CASE(wrong_syntax_class_x_y); // #3585 - class x y { };
        TEST_CASE(wrong_syntax_anonymous_struct);
        TEST_CASE(syntax_case_default);
        TEST_CASE(garbageCode1);
        TEST_CASE(garbageCode2); // #4300
        TEST_CASE(garbageCode3); // #4869
        TEST_CASE(garbageCode4); // #4887
        TEST_CASE(garbageCode5); // #5168
        TEST_CASE(garbageCode6); // #5214
        TEST_CASE(garbageCode7);
        TEST_CASE(garbageCode8); // #5511
        TEST_CASE(garbageCode9); // #5604
        TEST_CASE(garbageCode10); // #6127
        TEST_CASE(garbageCode12);
        TEST_CASE(garbageCode13); // #2607
        TEST_CASE(garbageCode15); // #5203
        TEST_CASE(garbageCode16);
        TEST_CASE(garbageCode17);
        TEST_CASE(garbageCode18);
        TEST_CASE(garbageCode20);
        TEST_CASE(garbageCode21);
        TEST_CASE(garbageCode22);
        TEST_CASE(garbageCode23);
        TEST_CASE(garbageCode24); // #6361
        TEST_CASE(garbageCode25);
        TEST_CASE(garbageCode26);
        TEST_CASE(garbageCode27);
        TEST_CASE(garbageCode28);
        TEST_CASE(garbageCode30); // #5867
        TEST_CASE(garbageCode31); // #6539
        TEST_CASE(garbageCode33); // #6613
        TEST_CASE(garbageCode34); // #6626
        TEST_CASE(garbageCode35); // #2604
        TEST_CASE(garbageCode36); // #6334
        TEST_CASE(garbageCode37); // #5166
        TEST_CASE(garbageCode38); // #6666
        TEST_CASE(garbageCode40); // #6620
        TEST_CASE(garbageCode41); // #6685
        TEST_CASE(garbageCode42); // #5760
        TEST_CASE(garbageCode43); // #6703
        TEST_CASE(garbageCode44); // #6704
        TEST_CASE(garbageCode45); // #6608
        TEST_CASE(garbageCode46); // #6705
        TEST_CASE(garbageCode47); // #6706
        TEST_CASE(garbageCode48); // #6712
        TEST_CASE(garbageCode49); // #6715
        TEST_CASE(garbageCode51); // #6719
        TEST_CASE(garbageCode53); // #6721
        TEST_CASE(garbageCode54); // #6722
        TEST_CASE(garbageCode55); // #6724
        TEST_CASE(garbageCode56); // #6713
        TEST_CASE(garbageCode57); // #6733
        TEST_CASE(garbageCode58); // #6732
        TEST_CASE(garbageCode59); // #6735
        TEST_CASE(garbageCode60); // #6736
        TEST_CASE(garbageCode61);
        TEST_CASE(garbageCode63);
        TEST_CASE(garbageCode64);
        TEST_CASE(garbageCode65);
        TEST_CASE(garbageCode66);
        TEST_CASE(garbageCode68);
        TEST_CASE(garbageCode69);
        TEST_CASE(garbageCode70);
        TEST_CASE(garbageCode71);
        TEST_CASE(garbageCode72);
        TEST_CASE(garbageCode73);
        TEST_CASE(garbageCode74);
        TEST_CASE(garbageCode76);
        TEST_CASE(garbageCode77);
        TEST_CASE(garbageCode78);
        TEST_CASE(garbageCode79);
        TEST_CASE(garbageCode80);
        TEST_CASE(garbageCode81);
        TEST_CASE(garbageCode82);
        TEST_CASE(garbageCode83);
        TEST_CASE(garbageCode84);
        TEST_CASE(garbageCode85);
        TEST_CASE(garbageCode86);
        TEST_CASE(garbageCode87);
        TEST_CASE(garbageCode88);
        TEST_CASE(garbageCode90);
        TEST_CASE(garbageCode91);
        TEST_CASE(garbageCode92);
        TEST_CASE(garbageCode94);
        TEST_CASE(garbageCode95);
        TEST_CASE(garbageCode96);
        TEST_CASE(garbageCode97);
        TEST_CASE(garbageCode98);
        TEST_CASE(garbageCode99);
        TEST_CASE(garbageCode100);
        TEST_CASE(garbageCode101); // #6835
        TEST_CASE(garbageCode102); // #6846
        TEST_CASE(garbageCode103); // #6824
        TEST_CASE(garbageCode104); // #6847
        TEST_CASE(garbageCode105); // #6859
        TEST_CASE(garbageCode106);
        TEST_CASE(garbageCode107);
        TEST_CASE(garbageCode108);
        TEST_CASE(garbageCode109);
        TEST_CASE(garbageCode110);
        TEST_CASE(garbageCode111);
        TEST_CASE(garbageCode112);
        TEST_CASE(garbageCode114); // #2118
        TEST_CASE(garbageCode115); // #5506
        TEST_CASE(garbageCode116); // #5356
        TEST_CASE(garbageCode117); // #6121
        TEST_CASE(garbageCode118); // #5600
        TEST_CASE(garbageCode119); // #5598
        TEST_CASE(garbageCode120); // #4927
        TEST_CASE(garbageCode121); // #2585
        TEST_CASE(garbageCode122); // #6303
        TEST_CASE(garbageCode123);
        TEST_CASE(garbageCode125); // 6782, 6834
        TEST_CASE(garbageCode126); // #6997
        TEST_CASE(garbageCode127); // #6667
        TEST_CASE(garbageCode128); // #7018
        TEST_CASE(garbageCode129); // #7020
        TEST_CASE(garbageCode130); // #7021
        TEST_CASE(garbageCode131); // #7023
        TEST_CASE(garbageCode132); // #7022
        TEST_CASE(garbageCode133);
        TEST_CASE(garbageCode134);
        TEST_CASE(garbageCode135); // #4994
        TEST_CASE(garbageCode136); // #7033
        TEST_CASE(garbageCode137); // #7034
        TEST_CASE(garbageCode138); // #6660
        TEST_CASE(garbageCode139); // #6659
        TEST_CASE(garbageCode140); // #7035
        TEST_CASE(garbageCode141); // #7043
        TEST_CASE(garbageCode142); // #7050
        TEST_CASE(garbageCode143); // #6922
        TEST_CASE(garbageCode144); // #6865
        TEST_CASE(garbageCode146); // #7081
        TEST_CASE(garbageCode147); // #7082
        TEST_CASE(garbageCode148); // #7090
        TEST_CASE(garbageCode149); // #7085
        TEST_CASE(garbageCode150); // #7089
        TEST_CASE(garbageCode151); // #4911
        TEST_CASE(garbageCode152); // travis after 9c7271a5
        TEST_CASE(garbageCode153);
        TEST_CASE(garbageCode154); // #7112
        TEST_CASE(garbageCode156); // #7120
        TEST_CASE(garbageCode157); // #7131
        TEST_CASE(garbageCode158); // #3238
        TEST_CASE(garbageCode159); // #7119
        TEST_CASE(garbageCode160); // #7190
        TEST_CASE(garbageCode161); // #7200
        TEST_CASE(garbageCode162); // #7208
        TEST_CASE(garbageCode163); // #7228
        TEST_CASE(garbageCode164); // #7234
        TEST_CASE(garbageCode165); // #7235
        TEST_CASE(garbageCode167); // #7237
        TEST_CASE(garbageCode168); // #7246
        TEST_CASE(garbageCode169); // #6731
        TEST_CASE(garbageCode170);
        TEST_CASE(garbageCode171);
        TEST_CASE(garbageCode172);
        TEST_CASE(garbageCode173); // #6781
        TEST_CASE(garbageCode174); // #7356
        TEST_CASE(garbageCode175);
        TEST_CASE(garbageCode176); // #7527
        TEST_CASE(garbageCode181);
        TEST_CASE(garbageCode182); // #4195
        TEST_CASE(garbageCode183); // #7505
        TEST_CASE(garbageCode184); // #7699
        TEST_CASE(garbageCode185); // #6011
        TEST_CASE(garbageCode186); // #8151
        TEST_CASE(garbageCode187);
        TEST_CASE(garbageCode188);
        TEST_CASE(garbageCode189); // #8317
        TEST_CASE(garbageCode190); // #8307
        TEST_CASE(garbageCode191); // #8333
        TEST_CASE(garbageCode192); // #8386 (segmentation fault)
        TEST_CASE(garbageCode193); // #8740
        TEST_CASE(garbageCode194); // #8384
        TEST_CASE(garbageCode195); // #8709
        TEST_CASE(garbageCode196); // #8265
        TEST_CASE(garbageCode197); // #8385
        TEST_CASE(garbageCode198); // #8383
        TEST_CASE(garbageCode199); // #8752
        TEST_CASE(garbageCode200); // #8757
        TEST_CASE(garbageCode201); // #8873
        TEST_CASE(garbageCode202); // #8907
        TEST_CASE(garbageCode203); // #8972
        TEST_CASE(garbageCode204);
        TEST_CASE(garbageCode205);
        TEST_CASE(garbageCode206);
        TEST_CASE(garbageCode207); // #8750
        TEST_CASE(garbageCode208); // #8753
        TEST_CASE(garbageCode209); // #8756
        TEST_CASE(garbageCode210); // #8762
        TEST_CASE(garbageCode211); // #8764
        TEST_CASE(garbageCode212); // #8765
        TEST_CASE(garbageCode213); // #8758
        TEST_CASE(garbageCode214);
        TEST_CASE(garbageCode215); // daca@home script with extension .c
        TEST_CASE(garbageCode216); // #7884
        TEST_CASE(garbageCode217); // #10011
        TEST_CASE(garbageCode218); // #8763
        TEST_CASE(garbageCode219); // #10101
        TEST_CASE(garbageCode220); // #6832
        TEST_CASE(garbageCode221);
        TEST_CASE(garbageCode222); // #10763
        TEST_CASE(garbageCode223); // #11639
        TEST_CASE(garbageCode224);
        TEST_CASE(garbageCode225);
        TEST_CASE(garbageCode226);
        TEST_CASE(garbageCode227);
        TEST_CASE(garbageCode228);
        TEST_CASE(garbageCode229);
        TEST_CASE(garbageCode230);
        TEST_CASE(garbageCode231);

        TEST_CASE(garbageCodeFuzzerClientMode1); // test cases created with the fuzzer client, mode 1

        TEST_CASE(garbageValueFlow);
        TEST_CASE(garbageSymbolDatabase);
        TEST_CASE(garbageAST);
        TEST_CASE(templateSimplifierCrashes);
        TEST_CASE(syntaxErrorFirstToken); // Make sure syntax errors are detected and reported
        TEST_CASE(syntaxErrorLastToken); // Make sure syntax errors are detected and reported
        TEST_CASE(syntaxErrorCase);
        TEST_CASE(syntaxErrorFuzzerCliType1);
        TEST_CASE(cliCode);
        TEST_CASE(enumTrailingComma);

        TEST_CASE(nonGarbageCode1); // #8346
    }

#define checkCodeInternal(...) checkCodeInternal_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string checkCodeInternal_(const char* file, int line, const char (&code)[size], bool cpp) {
        // tokenize..
        SimpleTokenizer tokenizer(settings, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // call all "runChecks" in all registered Check classes
        for (auto it = Check::instances().cbegin(); it != Check::instances().cend(); ++it) {
            (*it)->runChecks(tokenizer, this);
        }

        return tokenizer.tokens()->stringifyList(false, false, false, true, false, nullptr, nullptr);
    }

    template<size_t size>
    std::string checkCode(const char (&code)[size], bool cpp = true) {
        // double the tests - run each example as C as well as C++

        // run alternate check first. It should only ensure stability - so we catch exceptions here.
        try {
            (void)checkCodeInternal(code, !cpp);
        } catch (const InternalError&) {}

        return checkCodeInternal(code, cpp);
    }

#define getSyntaxError(...) getSyntaxError_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string getSyntaxError_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer tokenizer(settings, *this);
        try {
            ASSERT_LOC(tokenizer.tokenize(code), file, line);
        } catch (InternalError& e) {
            if (e.id != "syntaxError")
                return "";
            return "[test.cpp:" + std::to_string(e.token->linenr()) + "] " + e.errorMessage;
        }
        return "";
    }


    void final_class_x() {

        const char code[] = "class __declspec(dllexport) x final { };";
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT(tokenizer.tokenize(code));
        ASSERT_EQUALS("", errout_str());
    }

    void wrong_syntax1() {
        {
            const char code[] ="TR(kvmpio, PROTO(int rw), ARGS(rw), TP_(aa->rw;))";
            ASSERT_THROW_INTERNAL(checkCode(code), UNKNOWN_MACRO);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] ="struct A { template<int> struct { }; };";
            ASSERT_THROW_INTERNAL(checkCode(code), SYNTAX);
        }

        {
            const char code[] ="enum ABC { A,B, typedef enum { C } };";
            ASSERT_THROW_INTERNAL(checkCode(code), SYNTAX);
        }
    }

    void wrong_syntax2() {   // #3504
        const char code[] = "void f() {\n"
                            "    X<int> x;\n"
                            "    Y<int, int, int, int, int, char> y;\n"
                            "}\n"
                            "\n"
                            "void G( template <typename T> class (j) ) {}";

        // don't segfault..
        ASSERT_THROW_INTERNAL(checkCode(code), SYNTAX);
        ignore_errout(); // we are not interested in the output
    }


    void wrong_syntax3() {   // #3544
        const char code[] = "X #define\n"
                            "{\n"
                            " (\n"
                            "  for(  #endif typedef typedef cb[N] )\n"
                            "        ca[N]; =  cb[i]\n"
                            " )\n"
                            "}";

        SimpleTokenizer tokenizer(settings, *this);
        try {
            ASSERT(tokenizer.tokenize(code));
            assertThrowFail(__FILE__, __LINE__);
        } catch (InternalError& e) {
            ASSERT_EQUALS("syntax error", e.errorMessage);
            ASSERT_EQUALS("syntaxError", e.id);
            ASSERT_EQUALS(4, e.token->linenr());
        }
    }

    void wrong_syntax4() {   // #3618
        const char code[] = "typedef void (x) (int);    return x&";

        ASSERT_THROW_INTERNAL(checkCode(code), SYNTAX);
    }

    void wrong_syntax_if_macro() {
        // #2518 #4171
        ASSERT_THROW_INTERNAL(checkCode("void f() { if MACRO(); }"), SYNTAX);

        // #4668 - note there is no semicolon after MACRO()
        ASSERT_THROW_INTERNAL(checkCode("void f() { if (x) MACRO() {} }"), SYNTAX);

        // #4810 - note there is no semicolon after MACRO()
        ASSERT_THROW_INTERNAL(checkCode("void f() { if (x) MACRO() else ; }"), SYNTAX);
    }

    void wrong_syntax_class_x_y() {
        // #3585
        const char code[] = "class x y { };";

        {
            SimpleTokenizer tokenizer(settings, *this, false);
            ASSERT(tokenizer.tokenize(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            SimpleTokenizer tokenizer(settings, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT_EQUALS("[test.cpp:1:1]: (information) The code 'class x y {' is not handled. You can use -I or --include to add handling of this code. [class_X_Y]\n", errout_str());
        }
    }

    void wrong_syntax_anonymous_struct() {
        ASSERT_THROW_INTERNAL(checkCode("struct { int x; } = {0};"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("struct { int x; } * = {0};"), SYNTAX);
    }

    void syntax_case_default() {
        ASSERT_THROW_INTERNAL(checkCode("void f() {switch (n) { case: z(); break;}}"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("void f() {switch (n) { case;: z(); break;}}"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("void f() {switch (n) { case {}: z(); break;}}"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("void f() {switch (n) { case 0?{1}:{2} : z(); break;}}"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("void f() {switch (n) { case 0?1;:{2} : z(); break;}}"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("void f() {switch (n) { case 0?(1?{3:4}):2 : z(); break;}}"), AST);

        //ticket #4234
        ASSERT_THROW_INTERNAL(checkCode("( ) { switch break ; { switch ( x ) { case } y break ; : } }"), SYNTAX);

        //ticket #4267
        ASSERT_THROW_INTERNAL(checkCode("f ( ) { switch break; { switch ( x ) { case } case break; -6: ( ) ; } }"), SYNTAX);

        // Missing semicolon
        ASSERT_THROW_INTERNAL(checkCode("void foo () { switch(0) case 0 : default : }"), SYNTAX);
    }

    void garbageCode1() {
        (void)checkCode("struct x foo_t; foo_t typedef y;");
    }

    void garbageCode2() { //#4300 (segmentation fault)
        TODO_ASSERT_THROW(checkCode("enum { D = 1  struct  { } ; }  s.b = D;"), InternalError);
    }

    void garbageCode3() { //#4849 (segmentation fault in Tokenizer::simplifyStructDecl (invalid code))
        TODO_ASSERT_THROW(checkCode("enum {  D = 2 s ; struct y  { x } ; } { s.a = C ; s.b = D ; }"), InternalError);
    }

    void garbageCode4() { // #4887
        ASSERT_THROW_INTERNAL(checkCode("void f ( ) { = a ; if ( 1 ) if = ( 0 ) ; }"), SYNTAX);
    }

    void garbageCode5() { // #5168 (segmentation fault)
        ASSERT_THROW_INTERNAL(checkCode("( asm : ; void : );"), SYNTAX);
    }

    void garbageCode6() { // #5214
        ASSERT_THROW_INTERNAL(checkCode("int b = ( 0 ? ? ) 1 : 0 ;"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("int a = int b = ( 0 ? ? ) 1 : 0 ;"), SYNTAX);
    }

    void garbageCode7() {
        ASSERT_THROW_INTERNAL(checkCode("1 (int j) { return return (c) * sizeof } y[1];"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("foo(Args&&...) fn void = { } auto template<typename... bar(Args&&...)"), SYNTAX);
    }

    void garbageCode8() { // #5604
        TODO_ASSERT_THROW(checkCode("{ enum struct : };"), InternalError);
        TODO_ASSERT_THROW(checkCode("int ScopedEnum{ template<typename T> { { e = T::error }; };\n"
                                    "ScopedEnum1<int> se1; { enum class E : T { e = 0 = e ScopedEnum2<void*> struct UnscopedEnum3 { T{ e = 4 }; };\n"
                                    "arr[(int) E::e]; }; UnscopedEnum3<int> e2 = f()\n"
                                    "{ { e = e1; T::error } int test1 ue2; g() { enum class E { e = T::error }; return E::e; } int test2 = }\n"
                                    "namespace UnscopedEnum { template<typename T> struct UnscopedEnum1 { E{ e = T::error }; }; UnscopedEnum1<int> { enum E : { e = 0 }; };\n"
                                    "UnscopedEnum2<void*> ue3; template<typename T> struct UnscopedEnum3 { enum { }; }; int arr[E::e]; };\n"
                                    "UnscopedEnum3<int> namespace template<typename T> int f() { enum E { e }; T::error }; return (int) E(); } int test1 int g() { enum E { e = E };\n"
                                    "E::e; } int test2 = g<int>(); }"), InternalError);
    }

    void garbageCode9() {
        TODO_ASSERT_THROW(checkCode("enum { e = { } } ( ) { { enum { } } } { e } "), InternalError);
    }

    void garbageCode10() { // #6127
        ASSERT_THROW_INTERNAL(checkCode("for( rl=reslist; rl!=NULL; rl=rl->next )"), SYNTAX);
    }

    void garbageCode12() { // do not crash
        (void)checkCode("{ g; S (void) { struct } { } int &g; }");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode13() { // Ticket #2607 - crash
        (void)checkCode("struct C {} {} x");
    }

    void garbageCode15() { // Ticket #5203
        ASSERT_THROW_INTERNAL(checkCode("int f ( int* r ) { {  int s[2] ; f ( s ) ; if ( ) } }"), SYNTAX);
    }

    void garbageCode16() { // #6080 (segmentation fault)
        (void)checkCode("{ } A() { delete }"); // #6080
        ignore_errout(); // we do not care about the output
    }

    void garbageCode17() {
        ASSERT_THROW_INTERNAL(checkCode("void h(int l) {\n"
                                        "    while\n" // Don't crash (#3870)
                                        "}"), SYNTAX);
    }

    void garbageCode18() {
        ASSERT_THROW_INTERNAL(checkCode("switch(){case}"), SYNTAX);
    }

    void garbageCode20() {
        // #3953 (valgrind errors on garbage code)
        ASSERT_EQUALS("void f ( 0 * ) ;", checkCode("void f ( 0 * ) ;"));
    }

    void garbageCode21() {
        // Ticket #3486 - Don't crash garbage code
        ASSERT_THROW_INTERNAL(checkCode("void f()\n"
                                        "{\n"
                                        "  (\n"
                                        "    x;\n"
                                        "    int a, a2, a2*x; if () ;\n"
                                        "  )\n"
                                        "}"), SYNTAX);
    }

    void garbageCode22() {
        // Ticket #3480 - Don't crash garbage code
        ASSERT_THROW_INTERNAL(checkCode("int f()\n"
                                        "{\n"
                                        "    return if\n"
                                        "}"), SYNTAX);
    }

    void garbageCode23() {
        //garbage code : don't crash (#3481)
        ASSERT_THROW_INTERNAL_EQUALS(checkCode("{\n"
                                               "    if (1) = x\n"
                                               "    else abort s[2]\n"
                                               "}"),
                                     SYNTAX,
                                     "syntax error");
    }

    void garbageCode24() {
        // don't crash (example from #6361)
        ASSERT_THROW_INTERNAL(checkCode("float buffer[64];\n"
                                        "main (void)\n"
                                        "{\n"
                                        "  char *cptr;\n"
                                        "  cptr = (char *)buffer;\n"
                                        "  cptr += (-(long int) buffer & (16 * sizeof (float) - 1));\n"
                                        "}\n"), SYNTAX);
        ignore_errout(); // we do not care about the output
    }

    void garbageCode25() {
        // Ticket #2386 - Segmentation fault upon strange syntax
        ASSERT_THROW_INTERNAL(checkCode("void f() {\n"
                                        "    switch ( x ) {\n"
                                        "        case struct Tree : break;\n"
                                        "    }\n"
                                        "}"), SYNTAX);
        ignore_errout(); // we do not care about the output
    }

    void garbageCode26() {
        // See tickets #2518 #2555 #4171
        ASSERT_THROW_INTERNAL(checkCode("void f() {\n"
                                        "    switch MAKEWORD(1)\n"
                                        "    {\n"
                                        "    case 0:\n"
                                        "        return;\n"
                                        "    }\n"
                                        "}"), SYNTAX);
    }

    void garbageCode27() {
        ASSERT_THROW_INTERNAL(checkCode("int f() {\n"
                                        "    return if\n"
                                        "}"), SYNTAX);
    }

    void garbageCode28() { // #5702 (segmentation fault)
        // 5702
        (void)checkCode("struct R1 {\n"
                        "  int a;\n"
                        "  R1 () : a { }\n"
                        "};");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode30() {
        // simply survive - a syntax error would be even better (#5867)
        (void)checkCode("void f(int x) {\n"
                        " x = 42\n"
                        "}");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode31() {
        ASSERT_THROW_INTERNAL(checkCode("typedef struct{}x[([],)]typedef e y;(y,x 0){}"), SYNTAX);
    }

    void garbageCode33() { // #6613 (segmentation fault)
        (void)checkCode("main(()B{});");
    }

    // Bug #6626 crash: Token::astOperand2() const ( do while )
    void garbageCode34() {
        const char code[] = "void foo(void) {\n"
                            " do\n"
                            " while (0);\n"
                            "}";
        ASSERT_THROW_INTERNAL(checkCode(code), SYNTAX);
    }

    void garbageCode35() {
        // ticket #2604 segmentation fault
        ASSERT_THROW_INTERNAL(checkCode("sizeof <= A"), AST);
    }

    void garbageCode36() { // #6334
        ASSERT_THROW_INTERNAL(checkCode("{ } < class template < > , { = } ; class... >\n"
                                        "struct Y { }\n"
                                        "class Types { }\n"
                                        "( X < int > \"uses template\" ) ( < ( ) \"uses ;"
                                        "( int int ::primary \"uses template\" ) int double \"uses )"
                                        "::primary , \"uses template\" ;\n"), SYNTAX);
    }

    void garbageCode37() {
        // #5166 segmentation fault (invalid code) in lib/checkother.cpp:329 ( void * f { } void b ( ) { * f } )
        (void)checkCode("void * f { } void b ( ) { * f }");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode38() { // Ticket #6666 (segmentation fault)
        (void)checkCode("{ f2 { } } void f3 () { delete[] } { }");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode40() { // #6620 (segmentation fault)
        (void)checkCode("{ ( ) () { virtual } ; { } E } A { : { } ( ) } * const ( ) const { }");
        // test doesn't seem to work on any platform: ASSERT_THROW(checkCode("{ ( ) () { virtual } ; { } E } A { : { } ( ) } * const ( ) const { }", "test.c"), InternalError);
    }

    void garbageCode41() { // #6685 (segmentation fault)
        (void)checkCode(" { } { return } *malloc(__SIZE_TYPE__ size); *memcpy(void n); static * const () { memcpy (*slot, 3); } { (); } { }");
    }

    void garbageCode42() { // #5760 (segmentation fault)
        (void)checkCode("{  } * const ( ) { }");
    }

    void garbageCode43() { // #6703 (segmentation fault)
        (void)checkCode("int { }; struct A<void> a = { }");
    }

    void garbageCode44() { // #6704
        ASSERT_THROW_INTERNAL(checkCode("{ { }; }; { class A : }; public typedef b;"), SYNTAX);
    }

    void garbageCode45() { // #6608
        ASSERT_THROW_INTERNAL(checkCode("struct true template < > { = } > struct Types \"s\" ; static_assert < int > ;"), SYNTAX);
    }

    void garbageCode46() { // #6705 (segmentation fault)
        (void)checkCode(" { bar(char *x); void foo (int ...) { struct } va_list ap; va_start(ap, size); va_arg(ap, (d)); }");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode47() { // #6706 (segmentation fault)
        (void)checkCode(" { { }; }; * new private: B: B;");
    }

    void garbageCode48() { // #6712 (segmentation fault)
        ASSERT_THROW_INTERNAL(checkCode(" { d\" ) d ...\" } int main ( ) { ( ) catch ( A a ) { { } catch ( ) \"\" } }"), SYNTAX);
    }

    void garbageCode49() { // #6715
        ASSERT_THROW_INTERNAL(checkCode(" ( ( ) ) { } ( { ( __builtin_va_arg_pack ( ) ) ; } ) { ( int { ( ) ( ( ) ) } ( ) { } ( ) ) += ( ) }"), AST);
    }

    void garbageCode51() { // #6719
        ASSERT_THROW_INTERNAL(checkCode(" (const \"C\" ...); struct base { int f2; base (int arg1, int arg2); }; global_base(0x55, 0xff); { ((global_base.f1 0x55) (global_base.f2 0xff)) { } } base::base(int arg1, int arg2) { f2 = }"), SYNTAX);
    }

    void garbageCode53() { // #6721
        ASSERT_THROW_INTERNAL(checkCode("{ { } }; void foo (struct int i) { x->b[i] = = }"), SYNTAX);
    }

    void garbageCode54() { // #6722
        ASSERT_THROW_INTERNAL(checkCode("{ typedef long ((pf) p) (); }"), SYNTAX);
    }

    void garbageCode55() { // #6724
        ASSERT_THROW_INTERNAL(checkCode("() __attribute__((constructor)); { } { }"), SYNTAX);
    }

    void garbageCode56() { // #6713
        ASSERT_THROW_INTERNAL(checkCode("void foo() { int a = 0; int b = ???; }"), AST);
    }

    void garbageCode57() { // #6731
        ASSERT_THROW_INTERNAL(checkCode("{ } if () try { } catch (...) B::~B { }"), SYNTAX);
    }

    void garbageCode58() { // #6732, #6762
        ASSERT_THROW_INTERNAL(checkCode("{ }> {= ~A()^{} }P { }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("{= ~A()^{} }P { } { }> is"), SYNTAX);
    }

    void garbageCode59() { // #6735
        ASSERT_THROW_INTERNAL(checkCode("{ { } }; char font8x8[256][8]"), SYNTAX);
    }

    void garbageCode60() { // #6736
        ASSERT_THROW_INTERNAL(checkCode("{ } { } typedef int int_array[]; int_array &right ="), SYNTAX);
    }

    void garbageCode61() { // #6737
        ASSERT_THROW_INTERNAL(checkCode("{ (const U&) }; { }; { }; struct U : virtual public"), SYNTAX);
    }

    void garbageCode63() { // #6739
        ASSERT_THROW_INTERNAL(checkCode("{ } { } typedef int u_array[]; typedef u_array &u_array_ref; (u_array_ref arg) { } u_array_ref u_array_ref_gbl_obj0"), SYNTAX);
    }

    void garbageCode64() { // #6740
        ASSERT_THROW_INTERNAL(checkCode("{ } foo(void (*bar)(void))"), SYNTAX);
    }

    void garbageCode65() { // #6741 (segmentation fault)
        // TODO write some syntax error
        (void)checkCode("{ } { } typedef int u_array[]; typedef u_array &u_array_ref; (u_array_ref arg) { } u_array_ref");
    }

    void garbageCode66() { // #6742
        ASSERT_THROW_INTERNAL(checkCode("{ { } }; { { } }; { }; class bar : public virtual"), SYNTAX);
    }

    void garbageCode68() { // #6745 (segmentation fault)
        (void)checkCode("(int a[3]); typedef void (*fp) (void); fp");
    }

    void garbageCode69() { // #6746
        ASSERT_THROW_INTERNAL(checkCode("{ (make_mess, aux); } typedef void F(void); aux(void (*x)()) { } (void (*y)()) { } F*"), SYNTAX);
    }

    void garbageCode70() { // #6747
        ASSERT_THROW_INTERNAL(checkCode("{ } __attribute__((constructor)) void"), SYNTAX);
    }

    void garbageCode71() { // #6748
        ASSERT_THROW_INTERNAL(checkCode("( ) { } typedef void noattr_t ( ) ; noattr_t __attribute__ ( )"), SYNTAX);
    }

    void garbageCode72() { // #6749
        ASSERT_THROW_INTERNAL(checkCode("{ } { } typedef void voidfn(void); <voidfn&"), SYNTAX);
    }

    void garbageCode73() { // #6750
        ASSERT_THROW_INTERNAL(checkCode("typedef int IRT[2]; IRT&"), SYNTAX);
    }

    void garbageCode74() { // #6751
        ASSERT_THROW_INTERNAL(checkCode("_lenraw(const char* digits) { } typedef decltype(sizeof(0)) { } operator"), SYNTAX);
    }

    void garbageCode76() { // #6754
        ASSERT_THROW_INTERNAL(checkCode(" ( ) ( ) { ( ) [ ] } TEST ( ) { ( _broadcast_f32x4 ) ( ) ( ) ( ) ( ) if ( ) ( ) ; } E mask = ( ) [ ] ( ) res1.x ="), SYNTAX);
    }

    void garbageCode77() { // #6755
        ASSERT_THROW_INTERNAL(checkCode("void foo (int **p) { { { };>= } } unsigned *d = (b b--) --*d"), SYNTAX);
    }

    void garbageCode78() { // #6756
        ASSERT_THROW_INTERNAL(checkCode("( ) { [ ] } ( ) { } const_array_of_int ( ) { } typedef int A [ ] [ ] ; A a = { { } { } }"), SYNTAX);
        ignore_errout(); // we do not care about the output
    }

    void garbageCode79() { // #6757
        ASSERT_THROW_INTERNAL(checkCode("{ } { } typedef void ( func_type ) ( ) ; func_type & ( )"), SYNTAX);
    }

    void garbageCode80() { // #6759
        ASSERT_THROW_INTERNAL(checkCode("( ) { ; ( ) ; ( * ) [ ] ; [ ] = ( ( ) ( ) h ) ! ( ( ) ) } { ; } { } head heads [ ] = ; = & heads [ 2 ]"), SYNTAX);
    }

    void garbageCode81() { // #6760
        ASSERT_THROW_INTERNAL(checkCode("{ } [ ] { ( ) } { } typedef void ( *fptr1 ) ( ) const"), SYNTAX);
    }

    void garbageCode82() { // #6761
        ASSERT_THROW_INTERNAL(checkCode("p(\"Hello \" 14) _yn(const size_t) typedef bool pfunk (*pfunk)(const size_t)"), SYNTAX);
    }

    void garbageCode83() { // #6771
        ASSERT_THROW_INTERNAL(checkCode("namespace A { class } class A { friend C ; } { } ;"), SYNTAX);
    }

    void garbageCode84() { // #6780
        ASSERT_THROW_INTERNAL(checkCode("int main ( [ ] ) { " " [ ] ; int i = 0 ; do { } ; } ( [ ] ) { }"), SYNTAX); // do not crash
    }

    void garbageCode85() { // #6784 (segmentation fault)
        (void)checkCode("{ } { } typedef void ( *VoidFunc() ) ( ) ; VoidFunc"); // do not crash
    }

    void garbageCode86() { // #6785
        ASSERT_THROW_INTERNAL(checkCode("{ } typedef char ( *( X ) ( void) , char ) ;"), SYNTAX); // do not crash
    }

    void garbageCode87() { // #6788
        ASSERT_THROW_INTERNAL(checkCode("((X (128))) (int a) { v[ = {} (x 42) a] += }"), SYNTAX); // do not crash
    }

    void garbageCode88() { // #6786
        ASSERT_THROW_INTERNAL(checkCode("( ) { ( 0 ) { ( ) } } g ( ) { i( ( false ?) ( ) : 1 ) ; } ;"), SYNTAX); // do not crash
    }

    void garbageCode90() { // #6790
        ASSERT_THROW_INTERNAL(checkCode("{ } { } typedef int u_array [[ ] ; typedef u_array & u_array_ref] ( ) { } u_array_ref_gbl_obj0"), SYNTAX); // do not crash
    }

    void garbageCode91() { // #6791
        ASSERT_THROW_INTERNAL(checkCode("typedef __attribute__((vector_size (16))) { return[ (v2df){ } ;] }"), SYNTAX); // throw syntax error
    }

    void garbageCode92() { // #6792
        ASSERT_THROW_INTERNAL(checkCode("template < typename _Tp ( ( ) ; _Tp ) , decltype > { } { ( ) ( ) }"), SYNTAX); // do not crash
    }

    void garbageCode94() { // #6803
        //checkCode("typedef long __m256i __attribute__ ( ( ( ) ) )[ ; ( ) { } typedef __m256i __attribute__ ( ( ( ) ) ) < ] ( ) { ; }");
        ASSERT_THROW_INTERNAL(checkCode("typedef long __m256i __attribute__ ( ( ( ) ) )[ ; ( ) { } typedef __m256i __attribute__ ( ( ( ) ) ) < ] ( ) { ; }"), SYNTAX);
    }

    void garbageCode95() { // #6804
        ASSERT_THROW_INTERNAL(checkCode("{ } x x ; { } h h [ ] ( ) ( ) { struct x ( x ) ; int __attribute__ ( ) f ( ) { h - > first = & x ; struct x * n = h - > first ; ( ) n > } }"), SYNTAX); // do not crash
    }

    void garbageCode96() { // #6807
        ASSERT_THROW_INTERNAL(checkCode("typedef J J[ ; typedef ( ) ( ) { ; } typedef J J ;] ( ) ( J cx ) { n } ;"), SYNTAX); // throw syntax error
    }

    void garbageCode97() { // #6808
        ASSERT_THROW_INTERNAL(checkCode("namespace A {> } class A{ { }} class A : T< ;"), SYNTAX);
    }

    void garbageCode98() { // #6838
        ASSERT_THROW_INTERNAL(checkCode("for (cocon To::ta@Taaaaaforconst oken aaaaaaaaaaaa5Dl()\n"
                                        "const unsigned in;\n"
                                        "fon *tok = f);.s(Token i = d-)L;"), SYNTAX);
    }

    void garbageCode99() { // #6726
        ASSERT_THROW_INTERNAL_EQUALS(checkCode("{ xs :: i(:) ! ! x/5 ! !\n"
                                               "i, :: a :: b integer, } foo2(x) :: j(:)\n"
                                               "b type(*), d(:), a x :: end d(..), foo end\n"
                                               "foo4 b d(..), a a x type(*), b foo2 b"), SYNTAX, "syntax error");
    }

    void garbageCode100() { // #6840
        ASSERT_THROW_INTERNAL(checkCode("( ) { ( i< ) } int foo ( ) { int i ; ( for ( i => 1 ) ; ) }"), SYNTAX);
    }

    void garbageCode101() { // #6835
        // Reported case
        ASSERT_THROW_INTERNAL(checkCode("template < class , =( , int) X = 1 > struct A { } ( ) { = } [ { } ] ( ) { A < void > 0 }"), SYNTAX);
        // Reduced case
        ASSERT_THROW_INTERNAL(checkCode("template < class =( , ) X = 1> struct A {}; A<void> a;"), SYNTAX);
    }

    void garbageCode102() { // #6846 (segmentation fault)
        ASSERT_THROW_INTERNAL(checkCode("struct Object { ( ) ; Object & operator= ( Object ) { ( ) { } if ( this != & b ) } }"), SYNTAX);
    }

    void garbageCode103() { // #6824
        ASSERT_THROW_INTERNAL(checkCode("a f(r) int * r; { { int s[2]; [f(s); if () ]  } }"), SYNTAX);
    }

    void garbageCode104() { // #6847
        ASSERT_THROW_INTERNAL(checkCode("template < Types > struct S {> ( S < ) S >} { ( ) { } } ( ) { return S < void > ( ) } { ( )> >} { ( ) { } } ( ) { ( ) }"), SYNTAX);
    }

    void garbageCode105() { // #6859
        ASSERT_THROW_INTERNAL(checkCode("void foo (int i) { int a , for (a 1; a( < 4; a++) if (a) (b b++) (b);) n++; }"), SYNTAX);
    }

    void garbageCode106() { // #6880
        ASSERT_THROW_INTERNAL(checkCode("[ ] typedef typedef b_array b_array_ref [ ; ] ( ) b_array_ref b_array_ref_gbl_obj0 { ; { b_array_ref b_array_ref_gbl_obj0 } }"), SYNTAX);
    }

    void garbageCode107() { // #6881
        TODO_ASSERT_THROW(checkCode("enum { val = 1{ }; { const} }; { } Bar { const int A = val const } ;"), InternalError);
    }

    void garbageCode108() { //  #6895 "segmentation fault (invalid code) in CheckCondition::isOppositeCond"
        ASSERT_THROW_INTERNAL(checkCode("A( ) { } bool f( ) { ( ) F; ( ) { ( == ) if ( !=< || ( !A( ) && r[2] ) ) ( !A( ) ) ( ) } }"), SYNTAX);
    }

    void garbageCode109() { //  #6900 "segmentation fault (invalid code) in CheckStl::runSimplifiedChecks"
        (void)checkCode("( *const<> (( ) ) { } ( *const ( ) ( ) ) { } ( * const<> ( size_t )) ) { } ( * const ( ) ( ) ) { }");
    }

    void garbageCode110() { //  #6902 "segmentation fault (invalid code) in CheckStl::string_c_str"
        ASSERT_THROW_INTERNAL(checkCode("( *const<> ( size_t ) ; foo ) { } * ( *const ( size_t ) ( ) ;> foo )< { }"), SYNTAX);
    }

    void garbageCode111() { //  #6907
        TODO_ASSERT_THROW(checkCode("enum { FOO = 1( ,) } {{ FOO }} ;"), InternalError);
    }

    void garbageCode112() { //  #6909
        TODO_ASSERT_THROW(checkCode("enum { FOO = ( , ) } {{ }}>> enum { FOO< = ( ) } { { } } ;"), InternalError);
    }

    void garbageCode114() { // #2118
        ASSERT_NO_THROW(checkCode("Q_GLOBAL_STATIC_WITH_INITIALIZER(Qt4NodeStaticData, qt4NodeStaticData, {\n"
                                  "    for (unsigned i = 0 ; i < count; i++) {\n"
                                  "    }\n"
                                  "});"));
    }

    void garbageCode115() { // #5506
        ASSERT_THROW_INTERNAL(checkCode("A template < int { int = -1 ; } template < int N > struct B { int [ A < N > :: zero ] ;  } ; B < 0 > b ;"), UNKNOWN_MACRO);
    }

    void garbageCode116() { // #5356
        ASSERT_THROW_INTERNAL(checkCode("struct template<int { = }; > struct B { }; B < 0 > b;"), SYNTAX);
    }

    void garbageCode117() { // #6121
        TODO_ASSERT_THROW(checkCode("enum E { f = {} };\n"
                                    "int a = f;"), InternalError);
    }

    void garbageCode118() { // #5600 - missing include causes invalid enum
        ASSERT_THROW_INTERNAL(checkCode("enum {\n"
                                        "    NUM_OPCODES =\n"
                                        // #include "definition"
                                        "};\n"
                                        "struct bytecode {};\n"
                                        "jv jq_next() { opcode = ((opcode) +NUM_OPCODES);\n"
                                        "}"), SYNTAX);
    }

    void garbageCode119() { // #5598 (segmentation fault)
        (void)checkCode("{ { void foo() { struct }; template <typename> struct S { Used x; void bar() } auto f = [this] { }; } };");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode120() { // #4927 (segmentation fault)
        (void)checkCode("int main() {\n"
                        "   return 0\n"
                        "}");
        ASSERT_EQUALS("", errout_str());
    }

    void garbageCode121() { // #2585
        ASSERT_THROW_INTERNAL(checkCode("abcdef?" "?<"
                                        "123456?" "?>"
                                        "+?" "?="), SYNTAX);
    }

    void garbageCode122() { // #6303 (segmentation fault)
        (void)checkCode("void foo() {\n"
                        "char *a = malloc(10);\n"
                        "a[0]\n"
                        "}");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode123() {
        (void)checkCode("namespace pr16989 {\n"
                        "    class C {\n"
                        "        C tpl_mem(T *) { return }\n"
                        "    };\n"
                        "}");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode125() {
        ASSERT_THROW_INTERNAL(checkCode("{ T struct B : T valueA_AA ; } T : [ T > ( ) { B } template < T > struct A < > : ] { ( ) { return valueA_AC struct { : } } b A < int > AC ( ) a_aa.M ; ( ) ( ) }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("template < Types > struct S :{ ( S < ) S >} { ( ) { } } ( ) { return S < void > ( ) }"),
                              SYNTAX);
    }

    void garbageCode126() {
        ASSERT_THROW_INTERNAL(checkCode("{ } float __ieee754_sinhf ( float x ) { float t , , do { gf_u ( jx ) { } ( 0 ) return ; ( ) { } t } ( 0x42b17180 ) { } }"),
                              SYNTAX);
    }

    void garbageCode127() { // #6667 (segmentation fault)
        (void)checkCode("extern \"C\" int printf(const char* fmt, ...);\n"
                        "class A {\n"
                        "public:\n"
                        "  int Var;\n"
                        "  A(int arg) { Var = arg; }\n"
                        "  ~A() { printf(\"A d'tor\\n\"); }\n"
                        "};\n"
                        " const A& foo(const A& arg) { return arg; }\n"
                        " foo(A(12)).Var");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode128() {
        TODO_ASSERT_THROW(checkCode("enum { FOO = ( , ) } {{ }} enum {{ FOO << = } ( ) } {{ }} ;"),
                          InternalError);
    }

    void garbageCode129() {
        ASSERT_THROW_INTERNAL(checkCode("operator - ( { } typedef typename x ; ( ) ) { ( { { ( ( ) ) } ( { } ) } ) }"),
                              SYNTAX);
    }

    void garbageCode130() {
        TODO_ASSERT_THROW(checkCode("enum { FOO = ( , ){ } { { } } { { FOO} = } ( ) } { { } } enumL\" ( enumL\" { { FOO } ( ) } { { } } ;"),
                          InternalError);
    }

    void garbageCode131() {
        ASSERT_THROW_INTERNAL(checkCode("( void ) { ( ) } ( ) / { ( ) }"), SYNTAX);
        // actually the invalid code should trigger an syntax error...
    }

    void garbageCode132() { // #7022
        ASSERT_THROW_INTERNAL(checkCode("() () { } { () () ({}) i() } void i(void(*ptr) ()) { ptr(!) () }"), SYNTAX);
    }

    void garbageCode133() {
        ASSERT_THROW_INTERNAL(checkCode("void f() {{}"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("void f()) {}"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("void f()\n"
                                        "{\n"
                                        " foo(;\n"
                                        "}\n"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("void f()\n"
                                        "{\n"
                                        " for(;;){ foo();\n"
                                        "}\n"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("void f()\n"
                                        "{\n"
                                        " a[10;\n"
                                        "}\n"), SYNTAX);

        {
            const char code[] = "{\n"
                                "   a(\n" // <- error
                                "}\n"
                                "{\n"
                                "   b());\n"
                                "}\n";
            ASSERT_EQUALS("[test.cpp:2] Unmatched '('. Configuration: ''.", getSyntaxError(code));
        }

        {
            const char code[] = "void f() {\n"
                                "   int x = 3) + 0;\n" // <- error: unmatched )
                                "}\n";
            ASSERT_EQUALS("[test.cpp:2] Unmatched ')'. Configuration: ''.", getSyntaxError(code));
        }

        {
            const char code[] = "void f() {\n"
                                "   int x = (3] + 0;\n" // <- error: unmatched ]
                                "}\n";
            ASSERT_EQUALS("[test.cpp:2] Unmatched ']'. Configuration: ''.", getSyntaxError(code));
        }

        {
            const char code[] = "void f() {\n" // <- error: unmatched {
                                "   {\n"
                                "}\n";
            ASSERT_EQUALS("[test.cpp:1] Unmatched '{'. Configuration: ''.", getSyntaxError(code));
        }
    }

    void garbageCode134() {
        // Ticket #5605, #5759, #5762, #5774, #5823, #6059
        ASSERT_THROW_INTERNAL(checkCode("foo() template<typename T1 = T2 = typename = unused, T5 = = unused> struct tuple Args> tuple<Args...> { } main() { foo<int,int,int,int,int,int>(); }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("( ) template < T1 = typename = unused> struct Args { } main ( ) { foo < int > ( ) ; }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("() template < T = typename = x > struct a {} { f <int> () }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("template < T = typename = > struct a { f <int> }"), SYNTAX);
        (void)checkCode("struct S { int i, j; }; "
                        "template<int S::*p, typename U> struct X {}; "
                        "X<&S::i, int> x = X<&S::i, int>(); "
                        "X<&S::j, int> y = X<&S::j, int>();");
        ignore_errout(); // we are not interested in the output
        (void)checkCode("template <typename T> struct A {}; "
                        "template <> struct A<void> {}; "
                        "void foo(const void* f = 0) {}");
        (void)checkCode("template<typename... T> struct A { "
                        "  static const int s = 0; "
                        "}; "
                        "A<int> a;");
        (void)checkCode("template<class T, class U> class A {}; "
                        "template<class T = A<int, int> > class B {}; "
                        "template<class T = B<int> > class C { "
                        "    C() : _a(0), _b(0) {} "
                        "    int _a, _b; "
                        "};");
        (void)checkCode("template<class... T> struct A { "
                        "  static int i; "
                        "}; "
                        "void f() { A<int>::i = 0; }");
        ignore_errout(); // we are not interested in the output
    }

    void garbageCode135() { // #4994 (segmentation fault)
        (void)checkCode("long f () {\n"
                        "  return a >> extern\n"
                        "}\n"
                        "long a = 1 ;\n"
                        "long b = 2 ;");
        ignore_errout(); // we are not interested in the output
    }

    void garbageCode136() { // #7033
        ASSERT_THROW_INTERNAL(checkCode("{ } () { void f() { node_t * n; for (; -n) {} } } { }"),
                              SYNTAX);
    }

    void garbageCode137() { // #7034
        ASSERT_THROW_INTERNAL(checkCode("\" \" typedef signed char f; \" \"; void a() { f * s = () &[]; (; ) (; ) }"), SYNTAX);
    }

    void garbageCode138() { // #6660 (segmentation fault)
        (void)checkCode("CS_PLUGIN_NAMESPACE_BEGIN(csparser)\n"
                        "{\n"
                        "    struct foo\n"
                        "    {\n"
                        "      union\n"
                        "      {};\n"
                        "    } halo;\n"
                        "}\n"
                        "CS_PLUGIN_NAMESPACE_END(csparser)");
        ignore_errout(); // we are not interested in the output
    }

    void garbageCode139() { // #6659 heap user after free: kernel: sm750_accel.c
        ASSERT_THROW_INTERNAL(checkCode("void hw_copyarea() {\n"
                                        "   de_ctrl = (nDirection == RIGHT_TO_LEFT) ?\n"
                                        "    ( (0 & ~(((1 << (1 - (0 ? DE_CONTROL_DIRECTION))) - 1) << (0 ? DE_CONTROL_DIRECTION))) )\n"
                                        "    : 42;\n"
                                        "}"), SYNTAX);
    }

    void garbageCode140() { // #7035
        ASSERT_THROW_INTERNAL(checkCode("int foo(int align) { int off(= 0 % align;  return off) ? \\ align - off  :  0;  \\ }"), SYNTAX);
    }

    void garbageCode141() { // #7043
        TODO_ASSERT_THROW(checkCode("enum { X = << { X } } enum { X = X } = X ;"), InternalError);
    }

    void garbageCode142() { // #7050
        ASSERT_THROW_INTERNAL(checkCode("{ } (  ) { void mapGraphs ( ) { node_t * n ; for (!oid n ) { } } } { }"), SYNTAX);
    }

    void garbageCode143() { // #6922
        ASSERT_THROW_INTERNAL(checkCode("void neoProgramShadowRegs() {\n"
                                        "    int i;\n"
                                        "    Bool noProgramShadowRegs;\n"
                                        "    if (noProgramShadowRegs) {\n"
                                        "    } else {\n"
                                        "        switch (nPtr->NeoPanelWidth) {\n"
                                        "        case 1280:\n"
                                        "            VGAwCR(0x64,0x?? );\n"
                                        "        }\n"
                                        "    }\n"
                                        "}"), AST);
    }

    void garbageCode144() { // #6865
        ASSERT_THROW_INTERNAL(checkCode("template < typename > struct A { } ; template < typename > struct A < INVALID > : A < int[ > { }] ;"), SYNTAX);
    }

    void garbageCode146() { // #7081
        ASSERT_THROW_INTERNAL(checkCode("void foo() {\n"
                                        "    ? std::cout << pow((, 1) << std::endl;\n"
                                        "    double <ip = NUO ip) << std::end;\n"
                                        "}"), SYNTAX);
    }

    void garbageCode147() { // #7082
        ASSERT_THROW_INTERNAL(checkCode("free(3();\n"
                                        "$  vWrongAllocp1) test1<int, -!>() ^ {\n"
                                        "    int *p<ynew int[n];\n"
                                        "    delete[]p;\n"
                                        "    int *p1 = (int*)malloc(n*sizeof(int));\n"
                                        "    free(p1);\n"
                                        "}\n"
                                        "void est2() {\n"
                                        "    for (int ui = 0; ui < 1z; ui++)\n"
                                        "        ;\n"
                                        "}"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("; void f ^ { return } int main ( ) { }"), SYNTAX); // #4941
    }

    void garbageCode148() { // #7090
        ASSERT_THROW_INTERNAL(checkCode("void f_1() {\n"
                                        "    typedef S0 b[][1][1] != 0\n"
                                        "};\n"
                                        "b[K][0] S0 b[][1][1] != 4{ 0 };\n"
                                        "b[0][0]"), UNKNOWN_MACRO);
    }

    void garbageCode149() { // #7085 (segmentation fault)
        (void)checkCode("int main() {\n"
                        "    for (j = 0; j < 1; j)\n"
                        "        j6;\n"
                        "}");
        ignore_errout(); // we do not care about the output
    }

    void garbageCode150() { // #7089
        ASSERT_THROW_INTERNAL(checkCode("class A {\n"
                                        "    pl vFoo() {\n"
                                        "        A::\n"
                                        "    };\n"
                                        "    A::\n"
                                        "}\n"), SYNTAX);
    }

    void garbageCode151() { // #4911 - bad simplification => don't crash
        (void)checkCode("void f() {\n"
                        "    int a;\n"
                        "    do { a=do_something() } while (a);\n"
                        "}");
    }

    void garbageCode152() { // happened in travis, originally from llvm clang code (segmentation fault)
        const char code[] = "template <bool foo = std::value &&>\n"
                            "static std::string foo(char *Bla) {\n"
                            "    while (Bla[1] && Bla[1] != ',') }\n";
        ASSERT_THROW_INTERNAL(checkCode(code), SYNTAX);
    }

    void garbageCode153() {
        TODO_ASSERT_THROW(checkCode("enum { X = << { X } } { X X } enum { X = << { ( X ) } } { } X */"), InternalError);
    }

    void garbageCode154() { // #7112 (segmentation fault)
        (void)checkCode("\"abc\"[];");
    }

    void garbageCode156() { // #7120
        ASSERT_THROW_INTERNAL(checkCode("struct {}a; d f() { c ? : } {}a.p"), AST);
    }

    void garbageCode157() { // #7131
        ASSERT_THROW_INTERNAL(checkCode("namespace std {\n"
                                        "  template < typename >\n"
                                        "  void swap();\n"
                                        "}"
                                        "template std::swap\n"), SYNTAX);
    }

    void garbageCode158() { // #3238 (segmentation fault)
        (void)checkCode("__FBSDID(\"...\");");
    }

    void garbageCode159() { // #7119
        ASSERT_THROW_INTERNAL(checkCode("({}typedef typename x;typename x!){({{}()})}"), SYNTAX);
    }

    void garbageCode160() { // #7190
        ASSERT_THROW_INTERNAL(checkCode("f(a,b,c,d)float [  a[],d;int ]  b[],c;{} "), SYNTAX); // don't hang
    }


    void garbageCodeFuzzerClientMode1() {
        ASSERT_THROW_INTERNAL(checkCode("void f() { x= name2 & name3 name2 = | 0.1 , | 0.1 , | 0.1 name4 <= >( ); }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("void f() { x = , * [ | + 0xff | > 0xff]; }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("void f() {  x = , | 0xff , 0.1 < ; }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("void f() { x = [ 1 || ] ; }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("void f1() { x = name6 1 || ? name3 [  ( 1 || +) ] ; }"), SYNTAX);
    }

    void garbageValueFlow() {
        { // #6089
            const char code[] = "{} int foo(struct, x1, struct x2, x3, int, x5, x6, x7)\n"
                                "{\n"
                                "    (foo(s, , 2, , , 5, , 7)) abort()\n"
                                "}\n";
            ASSERT_THROW_INTERNAL(checkCode(code), SYNTAX);
        }

        { // 6122 survive garbage code
            const char code[] = "; { int i ; for ( i = 0 ; = 123 ; ) - ; }";
            ASSERT_THROW_INTERNAL(checkCode(code), SYNTAX);
        }

        {
            const char code[] = "void f1() { for (int n = 0 n < 10 n++); }";
            ASSERT_THROW_INTERNAL(checkCode(code), SYNTAX);
        }
    }

    void garbageSymbolDatabase() {
        (void)checkCode("void f( { u = 1 ; } ) { }");

        ASSERT_THROW_INTERNAL(checkCode("{ }; void namespace A::f; { g() { int } }"), SYNTAX);

        ASSERT_THROW_INTERNAL(checkCode("class Foo {}; class Bar : public Foo"), SYNTAX);

        // #5663 (segmentation fault)
        (void)checkCode("YY_DECL { switch (yy_act) {\n"
                        "    case 65: YY_BREAK\n"
                        "    case YY_STATE_EOF(block):\n"
                        "        yyterminate();\n"
                        "} }");
        ignore_errout(); // we are not interested in the output
    }

    void garbageAST() {
        ASSERT_THROW_INTERNAL(checkCode("N 1024 float a[N], b[N + 3], c[N]; void N; (void) i;\n"
                                        "int #define for (i = avx_test i < c[i]; i++)\n"
                                        "b[i + 3] = a[i] * {}"), SYNTAX); // Don't hang (#5787)

        (void)checkCode("START_SECTION([EXTRA](bool isValid(const String &filename)))"); // Don't crash (#5991)

        // #8352
        ASSERT_THROW_INTERNAL(checkCode("else return % name5 name2 - =name1 return enum | { - name3 1 enum != >= 1 >= ++ { { || "
                                        "{ return return { | { - name3 1 enum != >= 1 >= ++ { name6 | ; ++}}}}}}}"), UNKNOWN_MACRO);
        ASSERT_THROW_INTERNAL(checkCode("else return % name5 name2 - =name1 return enum | { - name3 1 enum != >= 1 >= ++ { { || "
                                        "{ return return { | { - name3 1 enum != >= 1 >= ++ { { || ; ++}}}}}}}}"), UNKNOWN_MACRO);
    }

    void templateSimplifierCrashes() {
        (void)checkCode( // #5950 (segmentation fault)
            "struct A {\n"
            "  template <class T> operator T*();\n"
            "};\n"
            "\n"
            "template <> A::operator char*(){ return 0; } // specialization\n"
            "\n"
            "int main() {\n"
            "  A a;\n"
            "  int *ip = a.operator int*();\n"
            "}\n"
            "\n"
            "namespace PR5742 {\n"
            "  template <class T> struct A { };\n"
            "  struct S {\n"
            "    template <class T> operator T();\n"
            "  } s;\n"
            "  void f() {\n"
            "    s.operator A<A<int> >();\n"
            "  }\n"
            "}");

        ASSERT_NO_THROW(checkCode( // #6034
                            "template<template<typename...> class T, typename... Args>\n"
                            "struct foo<T<Args...> > {\n"
                            "    const bool value = true;\n"
                            "};\n"
                            "\n"
                            "template<int I>\n"
                            "struct int_\n"
                            "{};\n"
                            "\n"
                            "int main() {\n"
                            "  foo<int_<0> >::value;\n"
                            "}"));

        (void)checkCode( // #6117 (segmentation fault)
            "template <typename ...> struct something_like_tuple\n"
            "{};\n"
            "template <typename, typename> struct is_last {\n"
            "  static const bool value = false;\n"
            "};\n"
            "template <typename T, template <typename ...> class Tuple, typename ... Head>\n"
            "struct is_last<T, Tuple<Head ..., T>>\n"
            "{\n"
            "  static const bool value = true;\n"
            "};\n"
            "\n"
            "#define SA(X) static_assert (X, #X)\n"
            "\n"
            "typedef something_like_tuple<char, int, float> something_like_tuple_t;\n"
            "SA ((is_last<float, something_like_tuple_t>::value == false));\n"
            "SA ((is_last<int, something_like_tuple_t>::value == false));");
        ignore_errout(); // we are not interested in the output

        (void)checkCode( // #6225 (use-after-free)
            "template <typename...>\n"
            "void templ_fun_with_ty_pack() {}\n"
            "\n"
            "namespace PR20047 {\n"
            "        template <typename T>\n"
            "        struct A {};\n"
            "        using AliasA = A<T>;\n"
            "}");

        // #3449
        ASSERT_EQUALS("template < typename T > struct A ;\n"
                      "struct B { template < typename T > struct C } ;\n"
                      "{ } ;",
                      checkCode("template<typename T> struct A;\n"
                                "struct B { template<typename T> struct C };\n"
                                "{};"));
    }
    void garbageCode161() {
        //7200
        ASSERT_THROW_INTERNAL(checkCode("{ }{ if () try { } catch (...)} B : : ~B { }"), SYNTAX);
    }

    void garbageCode162() {
        //7208
        ASSERT_THROW_INTERNAL(checkCode("return <<  >>  x return <<  >>  x ", false), UNKNOWN_MACRO);
    }

    void garbageCode163() {
        //7228
        ASSERT_THROW_INTERNAL(checkCode("typedef s f[](){typedef d h(;f)}", false), SYNTAX);
    }

    void garbageCode164() {
        //7234
        ASSERT_THROW_INTERNAL(checkCode("class d{k p;}(){d::d():B<()}"), SYNTAX);
    }

    void garbageCode165() {
        //7235
        ASSERT_THROW_INTERNAL(checkCode("for(;..)", false),SYNTAX);
    }

    void garbageCode167() {
        //7237
        ASSERT_THROW_INTERNAL(checkCode("class D00i000{:D00i000::}i"),SYNTAX);
    }

    void garbageCode168() {
        // #7246 (segmentation fault)
        (void)checkCode("long foo(void) { return *bar; }", false);
        ignore_errout(); // we do not care about the output
    }

    void garbageCode169() {
        // 6713
        ASSERT_THROW_INTERNAL(checkCode("( ) { ( ) ; { return } switch ( ) i\n"
                                        "set case break ; default: ( ) }", false), SYNTAX);
    }

    void garbageCode170() {
        // 7255
        ASSERT_THROW_INTERNAL(checkCode("d i(){{f*s=typeid(()0,)}}", false), SYNTAX);
    }

    void garbageCode171() {
        // 7270
        ASSERT_THROW_INTERNAL(checkCode("(){case()?():}:", false), SYNTAX);
    }

    void garbageCode172() {
        // #7357
        ASSERT_THROW_INTERNAL(checkCode("p<e T=l[<]<>>,"), SYNTAX);
    }

    void garbageCode173() {
        // #6781  heap corruption ;  TemplateSimplifier::simplifyTemplateInstantiations
        ASSERT_THROW_INTERNAL(checkCode(" template < Types > struct S : >( S < ...Types... > S <) > { ( ) { } } ( ) { return S < void > ( ) }"), SYNTAX);
    }

    void garbageCode174() { // #7356
        ASSERT_THROW_INTERNAL(checkCode("{r e() { w*constD = (())D = cast< }}"), SYNTAX);
    }

    void garbageCode175() { // #7027
        ASSERT_THROW_INTERNAL(checkCode("int f() {\n"
                                        "  int i , j;\n"
                                        "  for ( i = t3 , i < t1 ; i++ )\n"
                                        "    for ( j = 0 ; j < = j++ )\n"
                                        "        return t1 ,\n"
                                        "}"), SYNTAX);
    }

    void garbageCode176() { // #7257 (segmentation fault)
        (void)checkCode("class t { { struct } enum class f : unsigned { q } b ; operator= ( T ) { switch ( b ) { case f::q: } } { assert ( b ) ; } } { ; & ( t ) ( f::t ) ; } ;");
        ignore_errout(); // we are not interested in the output
    }

    void garbageCode181() {
        ASSERT_THROW_INTERNAL(checkCode("int test() { int +; }"), SYNTAX);
    }

    // #4195 - segfault for "enum { int f ( ) { return = } r = f ( ) ; }"
    void garbageCode182() {
        ASSERT_THROW_INTERNAL(checkCode("enum { int f ( ) { return = } r = f ( ) ; }"), SYNTAX);
    }
    // #7505 - segfault
    void garbageCode183() {
        ASSERT_THROW_INTERNAL(checkCode("= { int } enum return { r = f() f(); }"), SYNTAX);
    }

    void garbageCode184() { // #7699
        ASSERT_THROW_INTERNAL(checkCode("unsigned int AquaSalSystem::GetDisplayScreenCount() {\n"
                                        "    NSArray* pScreens = [NSScreen screens];\n"
                                        "    return pScreens ? [pScreens count] : 1;\n"
                                        "}"), SYNTAX);
    }

    void garbageCode185() { // #6011 crash in libreoffice failure to create proper AST
        (void)checkCode(
            "namespace binfilter\n"
            "{\n"
            "       BOOL EnhWMFReader::ReadEnhWMF()\n"
            "       {\n"
            "               pOut->CreateObject( nIndex, GDI_BRUSH, new WinMtfFillStyle( ReadColor(), ( nStyle == BS_HOLLOW ) ? TRUE : FALSE ) );\n"
            "               return bStatus;\n"
            "       };\n"
            "}");
        ignore_errout(); // we are not interested in the output
    }

    // #8151 - segfault due to incorrect template syntax
    void garbageCode186() {
        ASSERT_THROW_INTERNAL(checkCode("A<B<><>C"), SYNTAX);
    }

    void garbageCode187() { // # 8152 - segfault in handling
        const char inp[] = "0|\0|0>;\n";
        ASSERT_THROW_INTERNAL(checkCode(inp), SYNTAX);

        (void)checkCode("template<class T> struct S : A< B<T> || C<T> > {};"); // No syntax error: #8390
        (void)checkCode("static_assert(A<x> || B<x>, ab);");
    }

    void garbageCode188() { // #8255
        ASSERT_THROW_INTERNAL(checkCode("{z r(){(){for(;<(x);){if(0==0)}}}}"), SYNTAX);
    }

    void garbageCode189() { // #8317 (segmentation fault)
        (void)checkCode("t&n(){()()[](){()}}$");
    }

    void garbageCode190() { // #8307
        ASSERT_THROW_INTERNAL(checkCode("void foo() {\n"
                                        "    int i;\n"
                                        "    i *= 0;\n"
                                        "    !i <;\n"
                                        "}"),
                              AST);
    }

    void garbageCode191() { // #8333
        ASSERT_THROW_INTERNAL(checkCode("struct A { int f(const); };"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("struct A { int f(int, const, char); };"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("struct A { int f(struct); };"), SYNTAX);

        // The following code is valid and should not trigger any error
        ASSERT_NO_THROW(checkCode("struct A { int f ( char ) ; } ;"));
    }

    void garbageCode192() { // #8386 (segmentation fault)
        ASSERT_THROW_INTERNAL(checkCode("{(()[((0||0xf||))]0[])}"), SYNTAX);
    }

    // #8740
    void garbageCode193() {
        ASSERT_THROW_INTERNAL(checkCode("d f(){!=[]&&0()!=0}"), SYNTAX);
    }

    // #8384
    void garbageCode194() {
        ASSERT_THROW_INTERNAL(checkCode("{((()))(return 1||);}"), SYNTAX);
    }

    // #8709 - no garbage but to avoid stability regression (segmentation fault)
    void garbageCode195() {
        (void)checkCode("a b;\n"
                        "void c() {\n"
                        "  switch (d) { case b:; }\n"
                        "  double e(b);\n"
                        "  if(e <= 0) {}\n"
                        "}");
        ignore_errout(); // we do not care about the output
    }

    // #8265
    void garbageCode196() {
        ASSERT_THROW_INTERNAL(checkCode("0|,0<<V"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode(";|4|<0;"), SYNTAX);
    }

    // #8385
    void garbageCode197() {
        ASSERT_THROW_INTERNAL(checkCode("(){e break,{(case)|{e:[()]}}}"), SYNTAX);
    }

    // #8383
    void garbageCode198() {
        ASSERT_THROW_INTERNAL(checkCode("void f(){\n"
                                        "x= ={(continue continue { ( struct continue { ( ++ name5 name5 ) ( name5 name5 n\n"
                                        "ame5 ( name5 struct ( name5 name5 < ) ) ( default ) { name4 != name5 name5 name5\n"
                                        " ( name5 name5 name5 ( { 1 >= void { ( ()) } 1 name3 return >= >= ( ) >= name5 (\n"
                                        " name5 name6 :nam00 [ ()])}))})})})};\n"
                                        "}"), SYNTAX);
    }

    // #8752 (segmentation fault)
    void garbageCode199() {
        ASSERT_THROW_INTERNAL(checkCode("d f(){e n00e0[]n00e0&" "0+f=0}"), SYNTAX);
    }

    // #8757
    void garbageCode200() {
        ASSERT_THROW_INTERNAL(checkCode("(){e break,{(case)!{e:[]}}}"), SYNTAX);
    }

    // #8873
    void garbageCode201() {
        ASSERT_THROW_INTERNAL(checkCode("void f() { std::string s=\"abc\"; return s + }"), SYNTAX);
    }

    // #8907
    void garbageCode202() {
        ASSERT_THROW_INTERNAL(checkCode("void f() { UNKNOWN_MACRO(return); }"), UNKNOWN_MACRO);
        ASSERT_THROW_INTERNAL(checkCode("void f() { UNKNOWN_MACRO(throw); }"), UNKNOWN_MACRO);
        ignore_errout();
    }

    void garbageCode203() { // #8972 (segmentation fault)
        ASSERT_THROW_INTERNAL(checkCode("{ > () {} }"), SYNTAX);
        (void)checkCode("template <> a > ::b();");
    }

    void garbageCode204() {
        ASSERT_THROW_INTERNAL(checkCode("template <a, = b<>()> c; template <a> a as() {} as<c<>>();"), SYNTAX);
    }

    void garbageCode205() {
        ASSERT_THROW_INTERNAL(checkCode("class CodeSnippetsEvent : public wxCommandEvent {\n"
                                        "public :\n"
                                        "    CodeSnippetsEvent ( wxEventType commandType =  wxEventType , int id = 0 ) ;\n"
                                        "    CodeSnippetsEvent ( const CodeSnippetsEvent & event ) ;\n"
                                        "virtual wxEvent * Clone ( ) const { return new CodeSnippetsEvent ( * this ) ; }\n"
                                        "private :\n"
                                        "    int m_SnippetID ;\n"
                                        "} ;\n"
                                        "const  wxEventType wxEVT_CODESNIPPETS_GETFILELINKS  =  wxNewEventType  (  )\n"
                                        "CodeSnippetsEvent :: CodeSnippetsEvent ( wxEventType commandType , int id )\n"
                                        ": wxCommandEvent ( commandType , id ) {\n"
                                        "}\n"
                                        "CodeSnippetsEvent :: CodeSnippetsEvent ( const CodeSnippetsEvent & Event )\n"
                                        ": wxCommandEvent ( Event )\n"
                                        ", m_SnippetID ( 0 ) {\n"
                                        "}"),
                              INTERNAL);
    }

    void garbageCode206() {
        ASSERT_EQUALS("[test.cpp:1] syntax error: operator", getSyntaxError("void foo() { for (auto operator new : int); }"));
        ASSERT_EQUALS("[test.cpp:1] syntax error", getSyntaxError("void foo() { for (a operator== :) }"));
    }

    void garbageCode207() { // #8750
        ASSERT_THROW_INTERNAL(checkCode("d f(){(.n00e0(return%n00e0''('')));}"), SYNTAX);
    }

    void garbageCode208() { // #8753
        ASSERT_THROW_INTERNAL(checkCode("d f(){(for(((((0{t b;((((((((()))))))))}))))))}"), SYNTAX);
    }

    void garbageCode209() { // #8756
        ASSERT_THROW_INTERNAL(checkCode("{(- -##0xf/-1 0)[]}"), SYNTAX);
    }

    void garbageCode210() { // #8762
        ASSERT_THROW_INTERNAL(checkCode("{typedef typedef c n00e0[]c000(;n00e0&c000)}"), SYNTAX);
    }

    void garbageCode211() { // #8764
        ASSERT_THROW_INTERNAL(checkCode("{typedef f typedef[]({typedef e e,>;typedef(((typedef<typedef|)))})}"), SYNTAX);
    }

    void garbageCode212() { // #8765
        ASSERT_THROW_INTERNAL(checkCode("{(){}[]typedef r n00e0[](((n00e0 0((;()))))){(0 typedef n00e0 bre00 n00e0())}[]();typedef n n00e0()[],(bre00)}"), SYNTAX);
    }

    void garbageCode213() { // #8758
        ASSERT_THROW_INTERNAL(checkCode("{\"\"[(1||)];}"), SYNTAX);
    }

    void garbageCode214() { // segmentation fault
        (void)checkCode("THIS FILE CONTAINS VARIOUS TEXT");
    }

    void garbageCode215() { // daca@home script with extension .c
        ASSERT_THROW_INTERNAL(checkCode("a = [1,2,3];"), SYNTAX);
    }

    void garbageCode216() { // #7884 (out-of-memory)
        (void)checkCode("template<typename> struct A {};\n"
                        "template<typename...T> struct A<T::T...> {}; \n"
                        "A<int> a;");
    }

    void garbageCode217() { // #10011
        ASSERT_THROW_INTERNAL(checkCode("void f() {\n"
                                        "    auto p;\n"
                                        "    if (g(p)) {}\n"
                                        "    assert();\n"
                                        "}"), AST);
    }

    void garbageCode218() { // #8763
        ASSERT_THROW_INTERNAL(checkCode("d f(){t n0000 const[]n0000+0!=n0000,(0)}"), SYNTAX);
    }
    void garbageCode219() { // #10101
        (void)checkCode("typedef void (*func) (addr) ;\n"
                        "void bar(void) {\n"
                        "    func f;\n"
                        "    f & = (func)42;\n"
                        "}\n"); // don't crash
        ignore_errout(); // we are not interested in the output
    }
    void garbageCode220() { // #6832
        ASSERT_THROW_INTERNAL(checkCode("(){(){{()}}return;{switch()0 case(){}break;l:()}}\n"), SYNTAX);  // don't crash
    }
    void garbageCode221() {
        ASSERT_THROW_INTERNAL(checkCode("struct A<0<;\n"), SYNTAX);  // don't crash
    }
    void garbageCode222() { // #10763
        ASSERT_THROW_INTERNAL(checkCode("template<template<class>\n"), SYNTAX);  // don't crash
    }
    void garbageCode223() { // #11639
        ASSERT_THROW_INTERNAL(checkCode("struct{}*"), SYNTAX);  // don't crash
    }
    void garbageCode224() {
        ASSERT_THROW_INTERNAL(checkCode("void f(){ auto* b = dynamic_cast<const }"), SYNTAX);  // don't crash
        ASSERT_EQUALS("", errout_str());
        ASSERT_THROW_INTERNAL(checkCode("void f(){ auto* b = dynamic_cast x; }"), SYNTAX);
        ignore_errout();
    }
    void garbageCode225() {
        ASSERT_THROW_INTERNAL(checkCode("int n() { c * s0, 0 s0 = c(sizeof = ) }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("int n() { c * s0, 0 s0 = c(sizeof |= ) }"), SYNTAX);
    }
    void garbageCode226() {
        ASSERT_THROW_INTERNAL(checkCode("int a() { (b((c)`)) } {}"), SYNTAX); // #11638
        ASSERT_THROW_INTERNAL(checkCode("int a() { (b((c)\\)) } {}"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("int a() { (b((c)@)) } {}"), SYNTAX);
    }
    void garbageCode227() { // #12615
        ASSERT_NO_THROW(checkCode("f(&S::operator=);"));
    }
    void garbageCode228() {
        ASSERT_NO_THROW(checkCode("void f() { enum { A = [=]() mutable { return 0; }() }; }"));
        ASSERT_NO_THROW(checkCode("enum { A = [=](void) mutable -> int { return 0; }() };"));
    }
    void garbageCode229() { // #14126
        ASSERT_THROW_INTERNAL(checkCode("void f() {} [[maybe_unused]]"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("void f() {} [[unused]]"), SYNTAX);
    }
    void garbageCode230() { // #14432
        ASSERT_THROW_INTERNAL(checkCode("e U U,i"), SYNTAX);
    }
    void garbageCode231() {
        ASSERT_THROW_INTERNAL(checkCode("char char* [] = {\"a\" \"b\"}"), SYNTAX);
    }


    void syntaxErrorFirstToken() {
        ASSERT_THROW_INTERNAL(checkCode("&operator(){[]};"), SYNTAX); // #7818
        ASSERT_THROW_INTERNAL(checkCode("*(*const<> (size_t); foo) { } *(*const (size_t)() ; foo) { }"), SYNTAX); // #6858
        ASSERT_THROW_INTERNAL(checkCode(">{ x while (y) z int = }"), SYNTAX); // #4175
        ASSERT_THROW_INTERNAL(checkCode("&p(!{}e x){({(0?:?){({})}()})}"), SYNTAX); // #7118
        ASSERT_THROW_INTERNAL(checkCode("<class T> { struct { typename D4:typename Base<T*> }; };"), SYNTAX); // #3533
        ASSERT_THROW_INTERNAL(checkCode(" > template < . > struct Y < T > { = } ;\n"), SYNTAX); // #6108
    }

    void syntaxErrorLastToken() {
        ASSERT_THROW_INTERNAL(checkCode("int *"), SYNTAX); // #7821
        ASSERT_THROW_INTERNAL(checkCode("x[y]"), SYNTAX); // #2986
        ASSERT_THROW_INTERNAL(checkCode("( ) &"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("|| #if #define <="), SYNTAX); // #2601
        ASSERT_THROW_INTERNAL(checkCode("f::y:y : <x::"), SYNTAX); // #6613
        ASSERT_THROW_INTERNAL(checkCode("a \"b\" not_eq \"c\""), SYNTAX); // #6720
        ASSERT_THROW_INTERNAL(checkCode("(int arg2) { } { } typedef void (func_type) (int, int); typedef func_type&"), SYNTAX); // #6738
        ASSERT_THROW_INTERNAL(checkCode("&g[0]; { (g[0] 0) } =", false), SYNTAX); // #6744
        ASSERT_THROW_INTERNAL(checkCode("{ { void foo() { struct }; { }; } }; struct S { } f =", false), SYNTAX); // #6753
        ASSERT_THROW_INTERNAL(checkCode("{ { ( ) } P ( ) ^ { } { } { } ( ) } 0"), SYNTAX); // #6772
        ASSERT_THROW_INTERNAL(checkCode("+---+"), SYNTAX); // #6948
        ASSERT_THROW_INTERNAL(checkCode("template<>\n"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("++4++ +  + E++++++++++ + ch " "tp.oed5[.]"), SYNTAX); // #7074
        ASSERT_THROW_INTERNAL(checkCode("d a(){f s=0()8[]s?():0}*()?:0", false), SYNTAX); // #7236
        ASSERT_THROW_INTERNAL(checkCode("!2 : #h2 ?:", false), SYNTAX); // #7769
        ASSERT_THROW_INTERNAL(checkCode("--"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("volatile true , test < test < #ifdef __ppc__ true ,"), SYNTAX); // #4169
        ASSERT_THROW_INTERNAL(checkCode("a,b--\n"), SYNTAX); // #2847
        ASSERT_THROW_INTERNAL(checkCode("x a[0] ="), SYNTAX); // #2682
        ASSERT_THROW_INTERNAL(checkCode("auto_ptr<x>\n"), SYNTAX); // #2967
        ASSERT_THROW_INTERNAL(checkCode("char a[1]\n"), SYNTAX); // #2865
        ASSERT_THROW_INTERNAL(checkCode("<><<"), SYNTAX); // #2612
        ASSERT_THROW_INTERNAL(checkCode("z<y<x>"), SYNTAX); // #2831
        ASSERT_THROW_INTERNAL(checkCode("><,f<i,"), SYNTAX); // #2835
        ASSERT_THROW_INTERNAL(checkCode("0; (a) < (a)"), SYNTAX); // #2875
        ASSERT_THROW_INTERNAL(checkCode(" ( * const ( size_t ) ; foo )"), SYNTAX); // #6135
        ASSERT_THROW_INTERNAL(checkCode("({ (); strcat(strcat(() ()) ()) })"), SYNTAX); // #6686
        ASSERT_THROW_INTERNAL(checkCode("%: return ; ()"), SYNTAX); // #3441
        ASSERT_THROW_INTERNAL(checkCode("__attribute__((destructor)) void"), SYNTAX); // #7816
        ASSERT_THROW_INTERNAL(checkCode("1 *p = const"), SYNTAX); // #3512
        ASSERT_THROW_INTERNAL(checkCode("sizeof"), SYNTAX); // #2599
        ASSERT_THROW_INTERNAL(checkCode(" enum struct"), SYNTAX); // #6718
        ASSERT_THROW_INTERNAL(checkCode("{(){(())}}r&const"), SYNTAX); // #7321
        ASSERT_THROW_INTERNAL(checkCode("int"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("struct A :\n"), SYNTAX); // #2591
        ASSERT_THROW_INTERNAL(checkCode("{} const const\n"), SYNTAX); // #2637
        ASSERT_THROW_INTERNAL(checkCode("re2c: error: line 14, column 4: can only difference char sets"), SYNTAX);

        // ASSERT_THROW(  , InternalError)
    }

    void syntaxErrorCase() {
        // case must be inside switch block
        ASSERT_THROW_INTERNAL(checkCode("void f() { switch (a) {}; case 1: }"), SYNTAX); // #8184
        ASSERT_THROW_INTERNAL(checkCode("struct V : { public case {} ; struct U : U  void { V *f (int x) (x) } }"), SYNTAX); // #5120
        ASSERT_THROW_INTERNAL(checkCode("void f() { 0 0; }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("void f() { true 0; }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("void f() { 'a' 0; }"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("void f() { 1 \"\"; }"), SYNTAX);
    }

    void syntaxErrorFuzzerCliType1() {
        ASSERT_THROW_INTERNAL(checkCode("void f(){x=0,return return''[]()}"), SYNTAX);
        ASSERT_THROW_INTERNAL(checkCode("void f(){x='0'++'0'(return)[];}"), SYNTAX); // #9063
        (void)checkCode("void f(){*(int *)42=0;}"); // no syntax error
        ignore_errout(); // we are not interested in the output
        ASSERT_THROW_INTERNAL(checkCode("void f() { x= 'x' > typedef name5 | ( , ;){ } (); }"), SYNTAX); // #9067
        ASSERT_THROW_INTERNAL(checkCode("void f() { x= {}( ) ( 'x')[ ] (); }"), SYNTAX); // #9068
        ASSERT_THROW_INTERNAL(checkCode("void f() { x= y{ } name5 y[ ] + y ^ name5 ^ name5 for ( ( y y y && y y y && name5 ++ int )); }"), SYNTAX); // #9069
    }

    void cliCode() {
        // #8913
        ASSERT_NO_THROW(checkCode(
                            "public ref class LibCecSharp : public CecCallbackMethods {\n"
                            "array<CecAdapter ^> ^ FindAdapters(String ^ path) {}\n"
                            "bool GetDeviceInformation(String ^ port, LibCECConfiguration ^configuration, uint32_t timeoutMs) {\n"
                            "bool bReturn(false);\n"
                            "}\n"
                            "};"));
        ignore_errout(); // we are not interested in the output
    }

    void enumTrailingComma() {
        ASSERT_THROW_INTERNAL(checkCode("enum ssl_shutdown_t {ssl_shutdown_none = 0,ssl_shutdown_close_notify = , } ;"), SYNTAX); // #8079
    }

    void nonGarbageCode1() {
        ASSERT_NO_THROW(checkCode("template <class T> class List {\n"
                                  "public:\n"
                                  "   List();\n"
                                  "   virtual ~List();\n"
                                  "   template< class Predicate > u_int DeleteIf( const Predicate &pred );\n"
                                  "};\n"
                                  "template< class T >\n"
                                  "template< class Predicate > int\n"
                                  "List<T>::DeleteIf( const Predicate &pred )\n"
                                  "{}"));
        ignore_errout(); // we are not interested in the output

        // #8749
        ASSERT_NO_THROW(checkCode(
                            "struct A {\n"
                            "    void operator+=(A&) && = delete;\n"
                            "};"));

        // #8788
        ASSERT_NO_THROW(checkCode(
                            "struct foo;\n"
                            "void f() {\n"
                            "    auto fn = []() -> foo* { return new foo(); };\n"
                            "}"));
        ignore_errout(); // we do not care about the output

        // #13892
        ASSERT_NO_THROW(checkCode("void foovm(int x[const *]);"));
    }
};

REGISTER_TEST(TestGarbage)
