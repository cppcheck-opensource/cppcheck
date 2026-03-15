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
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "token.h"
#include "utils.h"

#include <cstddef>
#include <string>

class TestSimplifyUsing : public TestFixture {
public:
    TestSimplifyUsing() : TestFixture("TestSimplifyUsing") {}


private:
    const Settings settings0 = settingsBuilder().severity(Severity::style).build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(simplifyUsing1);
        TEST_CASE(simplifyUsing2);
        TEST_CASE(simplifyUsing3);
        TEST_CASE(simplifyUsing4);
        TEST_CASE(simplifyUsing5);
        TEST_CASE(simplifyUsing6);
        TEST_CASE(simplifyUsing7);
        TEST_CASE(simplifyUsing8);
        TEST_CASE(simplifyUsing9);
        TEST_CASE(simplifyUsing10);
        TEST_CASE(simplifyUsing11);
        TEST_CASE(simplifyUsing12);
        TEST_CASE(simplifyUsing13);
        TEST_CASE(simplifyUsing14);
        TEST_CASE(simplifyUsing15);
        TEST_CASE(simplifyUsing16);
        TEST_CASE(simplifyUsing17);
        TEST_CASE(simplifyUsing18);
        TEST_CASE(simplifyUsing19);
        TEST_CASE(simplifyUsing20);
        TEST_CASE(simplifyUsing21);
        TEST_CASE(simplifyUsing22);
        TEST_CASE(simplifyUsing23);
        TEST_CASE(simplifyUsing24);
        TEST_CASE(simplifyUsing25);
        TEST_CASE(simplifyUsing26); // #11090
        TEST_CASE(simplifyUsing27);
        TEST_CASE(simplifyUsing28);
        TEST_CASE(simplifyUsing29);
        TEST_CASE(simplifyUsing30);
        TEST_CASE(simplifyUsing31);
        TEST_CASE(simplifyUsing32);
        TEST_CASE(simplifyUsing33);
        TEST_CASE(simplifyUsing34);
        TEST_CASE(simplifyUsing35);
        TEST_CASE(simplifyUsing36);
        TEST_CASE(simplifyUsing37);
        TEST_CASE(simplifyUsing38);
        TEST_CASE(simplifyUsing39);

        TEST_CASE(simplifyUsing8970);
        TEST_CASE(simplifyUsing8971);
        TEST_CASE(simplifyUsing8976);
        TEST_CASE(simplifyUsing9040);
        TEST_CASE(simplifyUsing9042);
        TEST_CASE(simplifyUsing9191);
        TEST_CASE(simplifyUsing9381);
        TEST_CASE(simplifyUsing9385);
        TEST_CASE(simplifyUsing9388);
        TEST_CASE(simplifyUsing9518);
        TEST_CASE(simplifyUsing9757);
        TEST_CASE(simplifyUsing10008);
        TEST_CASE(simplifyUsing10054);
        TEST_CASE(simplifyUsing10136);
        TEST_CASE(simplifyUsing10171);
        TEST_CASE(simplifyUsing10172);
        TEST_CASE(simplifyUsing10173);
        TEST_CASE(simplifyUsing10335);
        TEST_CASE(simplifyUsing10720);
        TEST_CASE(simplifyUsing13873); // function declaration

        TEST_CASE(scopeInfo1);
        TEST_CASE(scopeInfo2);
    }

    struct TokOptions
    {
        Platform::Type type = Platform::Type::Native;
        bool debugwarnings = true;
        bool preprocess = false;
        Standards::cppstd_t cppstd = Standards::CPPLatest;
    };

#define tok(...) tok_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tok_(const char* file, int line, const char (&code)[size], const TokOptions& options = make_default_obj()) {
        const Settings settings = settingsBuilder(settings0).certainty(Certainty::inconclusive).debugwarnings(options.debugwarnings)
                                  .platform(options.type).cpp(options.cppstd).build();

        if (options.preprocess) {
            SimpleTokenizer2 tokenizer(settings, *this, code, "test.cpp");

            ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);
            return tokenizer.tokens()->stringifyList(nullptr);
        }

        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        return tokenizer.tokens()->stringifyList(nullptr);
    }

    void simplifyUsing1() {
        const char code[] = "class A\n"
                            "{\n"
                            "public:\n"
                            " using duplicate = wchar_t;\n"
                            " void foo() {}\n"
                            "};\n"
                            "using duplicate = A;\n"
                            "int main()\n"
                            "{\n"
                            " duplicate a;\n"
                            " a.foo();\n"
                            " A::duplicate c = 0;\n"
                            "}";

        const char expected[] =
            "class A "
            "{ "
            "public: "
            ""
            "void foo ( ) { } "
            "} ; "
            "int main ( ) "
            "{ "
            "A a ; "
            "a . foo ( ) ; "
            "wchar_t c ; c = 0 ; "
            "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing2() {
        const char code[] = "class A;\n"
                            "using duplicate = A;\n"
                            "class A\n"
                            "{\n"
                            "public:\n"
                            "using duplicate = wchar_t;\n"
                            "duplicate foo() { wchar_t b; return b; }\n"
                            "};";

        const char expected[] =
            "class A ; "
            "class A "
            "{ "
            "public: "
            ""
            "wchar_t foo ( ) { wchar_t b ; return b ; } "
            "} ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing3() {
        const char code[] = "class A {};\n"
                            "using duplicate = A;\n"
                            "wchar_t foo()\n"
                            "{\n"
                            "using duplicate = wchar_t;\n"
                            "duplicate b;\n"
                            "return b;\n"
                            "}\n"
                            "int main()\n"
                            "{\n"
                            "duplicate b;\n"
                            "}";

        const char expected[] =
            "class A { } ; "
            "wchar_t foo ( ) "
            "{ "
            ""
            "wchar_t b ; "
            "return b ; "
            "} "
            "int main ( ) "
            "{ "
            "A b ; "
            "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing4() {
        const char code[] = "using s32 = int;\n"
                            "using u32 = unsigned int;\n"
                            "void f()\n"
                            "{\n"
                            "    s32 ivar = -2;\n"
                            "    u32 uvar = 2;\n"
                            "    return uvar / ivar;\n"
                            "}";

        const char expected[] =
            "void f ( ) "
            "{ "
            "int ivar ; ivar = -2 ; "
            "unsigned int uvar ; uvar = 2 ; "
            "return uvar / ivar ; "
            "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing5() {
        const char code[] =
            "using YY_BUFFER_STATE = struct yy_buffer_state *;\n"
            "void f()\n"
            "{\n"
            "    YY_BUFFER_STATE state;\n"
            "}";

        const char expected[] =
            "void f ( ) "
            "{ "
            "yy_buffer_state * state ; "
            "}";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing6() {
        const char code[] =
            "namespace VL {\n"
            "    using float_t = float;\n"
            "    inline VL::float_t fast_atan2(VL::float_t y, VL::float_t x){}\n"
            "}";

        const char expected[] =
            "namespace VL { "
            ""
            "float fast_atan2 ( float y , float x ) { } "
            "}";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing7() {
        const char code[] = "using abc = int; "
                            "Fred :: abc f ;";
        const char expected[] = "Fred :: abc f ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing8() {
        const char code[] = "using INT = int;\n"
                            "using UINT = unsigned int;\n"
                            "using PINT = int *;\n"
                            "using PUINT = unsigned int *;\n"
                            "using RINT = int &;\n"
                            "using RUINT = unsigned int &;\n"
                            "using RCINT = const int &;\n"
                            "using RCUINT = const unsigned int &;\n"
                            "INT ti;\n"
                            "UINT tui;\n"
                            "PINT tpi;\n"
                            "PUINT tpui;\n"
                            "RINT tri;\n"
                            "RUINT trui;\n"
                            "RCINT trci;\n"
                            "RCUINT trcui;";

        const char expected[] =
            "int ti ; "
            "unsigned int tui ; "
            "int * tpi ; "
            "unsigned int * tpui ; "
            "int & tri ; "
            "unsigned int & trui ; "
            "const int & trci ; "
            "const unsigned int & trcui ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing9() {
        const char code[] = "using S = struct s;\n"
                            "using PS = S *;\n"
                            "using T = struct t { int a; };\n"
                            "using TP = T *;\n"
                            "using U = struct { int a; };\n"
                            "using V = U *;\n"
                            "using W = struct { int a; } *;\n"
                            "S s;\n"
                            "PS ps;\n"
                            "T t;\n"
                            "TP tp;\n"
                            "U u;\n"
                            "V v;\n"
                            "W w;";

        const char expected[] =
            "struct t { int a ; } ; "
            "struct U { int a ; } ; "
            "struct Unnamed0 { int a ; } ; "
            "s s ; "
            "s * ps ; "
            "t t ; "
            "t * tp ; "
            "U u ; "
            "U * v ; "
            "Unnamed0 * w ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing10() {
        const char code[] = "using S = union s;\n"
                            "using PS = S *;\n"
                            "using T = union t { int a; float b ; };\n"
                            "using TP = T *;\n"
                            "using U = union { int a; float b; };\n"
                            "using V = U *;\n"
                            "using W = union { int a; float b; } *;\n"
                            "S s;\n"
                            "PS ps;\n"
                            "T t;\n"
                            "TP tp;\n"
                            "U u;\n"
                            "V v;\n"
                            "W w;";

        const char expected[] =
            "union t { int a ; float b ; } ; "
            "union U { int a ; float b ; } ; "
            "union Unnamed0 { int a ; float b ; } ; "
            "s s ; "
            "s * ps ; "
            "t t ; "
            "t * tp ; "
            "U u ; "
            "U * v ; "
            "Unnamed0 * w ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing11() {
        const char code[] = "using abc = enum { a = 0 , b = 1 , c = 2 };\n"
                            "using XYZ = enum xyz { x = 0 , y = 1 , z = 2 };\n"
                            "abc e1;\n"
                            "XYZ e2;";

        const char expected[] = "enum abc { a = 0 , b = 1 , c = 2 } ; "
                                "enum xyz { x = 0 , y = 1 , z = 2 } ; "
                                "abc e1 ; "
                                "xyz e2 ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing12() {
        const char code[] = "using V1 = vector<int>;\n"
                            "using V2 = std::vector<int>;\n"
                            "using V3 = std::vector<std::vector<int> >;\n"
                            "using IntListIterator = std::list<int>::iterator;\n"
                            "V1 v1;\n"
                            "V2 v2;\n"
                            "V3 v3;\n"
                            "IntListIterator iter;";

        const char expected[] = "vector < int > v1 ; "
                                "std :: vector < int > v2 ; "
                                "std :: vector < std :: vector < int > > v3 ; "
                                "std :: list < int > :: iterator iter ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing13() {
        const char code[] = "using Func = std::pair<int(*)(void*), void*>;\n"
                            "using CallQueue = std::vector<Func>;\n"
                            "int main() {\n"
                            " CallQueue q;\n"
                            "}";

        const char expected[] = "int main ( ) { "
                                "std :: vector < std :: pair < int ( * ) ( void * ) , void * > > q ; "
                                "}";

        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing14() {
        const char code[] = "template <typename F, unsigned int N> struct E"
                            "{"
                            "    using v = E<F,(N>0)?(N-1):0>;"
                            "    using val = typename add<v,v>::val;"
                            "    FP_M(val);"
                            "};"
                            "template <typename F> struct E <F,0>"
                            "{"
                            "    using nal = typename D<1>::val;"
                            "    FP_M(val);"
                            "};";

        TODO_ASSERT_THROW(tok(code, dinit(TokOptions, $.debugwarnings = false)), InternalError); // TODO: Do not throw AST validation exception
        //ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing15() {
        {
            const char code[] = "using frame = char [10];\n"
                                "frame f;";

            const char expected[] = "char f [ 10 ] ;";

            ASSERT_EQUALS(expected, tok(code));
        }

        {
            const char code[] = "using frame = unsigned char [10];\n"
                                "frame f;";

            const char expected[] = "unsigned char f [ 10 ] ;";

            ASSERT_EQUALS(expected, tok(code));
        }
    }

    void simplifyUsing16() {
        const char code[] = "using MOT8 = char;\n"
                            "using CHFOO = MOT8 [4096];\n"
                            "using STRFOO = struct {\n"
                            "   CHFOO freem;\n"
                            "};\n"
                            "STRFOO s;";

        const char expected[] = "struct STRFOO { "
                                "char freem [ 4096 ] ; "
                                "} ; "
                                "STRFOO s ;";

        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing17() {
        const char code[] = "class C1 {};\n"
                            "using S1 = class S1 {};\n"
                            "using S2 = class S2 : public C1 {};\n"
                            "using S3 = class {};\n"
                            "using S4 = class : public C1 {};\n"
                            "S1 s1;\n"
                            "S2 s2;\n"
                            "S3 s3;\n"
                            "S4 s4;";

        const char expected[] = "class C1 { } ; "
                                "class S1 { } ; "
                                "class S2 : public C1 { } ; "
                                "class S3 { } ; "
                                "class S4 : public C1 { } ; "
                                "S1 s1 ; "
                                "S2 s2 ; "
                                "S3 s3 ; "
                                "S4 s4 ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing18() {
        const char code[] = "{ { { using a = a; using a; } } }";
        (void)tok(code); // don't crash
    }

    void simplifyUsing19() {
        const char code[] = "namespace a {\n"
                            "using b = int;\n"
                            "void foo::c() { }\n"
                            "void foo::d() { }\n"
                            "void foo::e() {\n"
                            "   using b = float;\n"
                            "}\n"
                            "}";
        (void)tok(code); // don't hang
        ignore_errout(); // we are not interested in the output
    }

    void simplifyUsing20() {
        const char code[] = "namespace a {\n"
                            "namespace b {\n"
                            "namespace c {\n"
                            "namespace d {\n"
                            "namespace e {\n"
                            "namespace f {\n"
                            "namespace g {\n"
                            "using Changeset = ::IAdaptionCallback::Changeset;\n"
                            "using EProperty = searches::EProperty;\n"
                            "using ChangesetValueType = Changeset::value_type;\n"
                            "namespace {\n"
                            "   template <class T>\n"
                            "   auto modify(std::shared_ptr<T>& p) -> boost::optional<decltype(p->modify())> {\n"
                            "      return nullptr;\n"
                            "   }\n"
                            "   template <class T>\n"
                            "   std::list<T> getValidElements() {\n"
                            "      return nullptr;\n"
                            "   }\n"
                            "}\n"
                            "std::shared_ptr<ResourceConfiguration>\n"
                            "foo::getConfiguration() {\n"
                            "   return nullptr;\n"
                            "}\n"
                            "void\n"
                            "foo::doRegister(const Input & Input) {\n"
                            "   UNUSED( Input );\n"
                            "}\n"
                            "foo::MicroServiceReturnValue\n"
                            "foo::post(SearchesPtr element, const Changeset& changeset)\n"
                            "{\n"
                            "   using EProperty = ab::ep;\n"
                            "   static std::map<EProperty, std::pair<ElementPropertyHandler, CheckComponentState>> updateHandlers =\n"
                            "   {\n"
                            "      {EProperty::Needle, {&RSISearchesResource::updateNeedle,         &RSISearchesResource::isSearcherReady}},\n"
                            "      {EProperty::SortBy, {&RSISearchesResource::updateResultsSorting, &RSISearchesResource::isSearcherReady}}\n"
                            "   };\n"
                            "   return nullptr;\n"
                            "}\n"
                            "}}}}}}}";
        (void)tok(code); // don't hang
        ignore_errout(); // we do not care about the output
    }

    void simplifyUsing21() {
        const char code[] = "using a = b;\n"
                            "enum {}";
        (void)tok(code); // don't crash
    }

    void simplifyUsing22() {
        const char code[] = "namespace aa { namespace bb { namespace cc { namespace dd { namespace ee {\n"
                            "class fff {\n"
                            "public:\n"
                            "    using icmsp   = std::shared_ptr<aa::bb::ee::cm::api::icm>;\n"
                            "private:\n"
                            "    using Connection = boost::signals2::connection;\n"
                            "    using ESdk = sdk::common::api::Sdk::ESdk;\n"
                            "    using co = aa::bb::ee::com::api::com2;\n"
                            "};\n"
                            "fff::fff() : m_icm(icm) {\n"
                            "    using ESdk = aa::bb::sdk::common::api::Sdk::ESdk;\n"
                            "}\n"
                            "}}}}}";
        const char expected[] = "namespace aa { namespace bb { namespace cc { namespace dd { namespace ee { "
                                "class fff { "
                                "public: "
                                "private: "
                                "} ; "
                                "fff :: fff ( ) : m_icm ( icm ) { "
                                "} "
                                "} } } } }";
        ASSERT_EQUALS(expected, tok(code)); // don't hang
        ignore_errout(); // we do not care about the output
    }

    void simplifyUsing23() {
        const char code[] = "class cmcch {\n"
                            "public:\n"
                            "   cmcch(icmsp const& icm, Rtnf&& rtnf = {});\n"
                            "private:\n"
                            "    using escs = aa::bb::cc::dd::ee;\n"
                            "private:\n"
                            "   icmsp m_icm;\n"
                            "   mutable std::atomic<rt> m_rt;\n"
                            "};\n"
                            "cmcch::cmcch(cmcch::icmsp const& icm, Rtnf&& rtnf)\n"
                            "   : m_icm(icm)\n"
                            "   , m_rt{rt::UNKNOWN_} {\n"
                            "  using escs = yy::zz::aa::bb::cc::dd::ee;\n"
                            "}";
        const char expected[] = "class cmcch { "
                                "public: "
                                "cmcch ( const icmsp & icm , Rtnf && rtnf = { } ) ; "
                                "private: "
                                "private: "
                                "icmsp m_icm ; "
                                "mutable std :: atomic < rt > m_rt ; "
                                "} ; "
                                "cmcch :: cmcch ( const cmcch :: icmsp & icm , Rtnf && rtnf ) "
                                ": m_icm ( icm ) "
                                ", m_rt { rt :: UNKNOWN_ } { "
                                "}";
        ASSERT_EQUALS(expected, tok(code)); // don't hang
    }

    void simplifyUsing24() {
        const char code[] = "using value_type = const ValueFlow::Value;\n"
                            "value_type vt;";
        const char expected[] = "const ValueFlow :: Value vt ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing25() {
        const char code[] = "struct UnusualType {\n"
                            "  using T = vtkm::Id;\n"
                            "  T X;\n"
                            "};\n"
                            "namespace vtkm {\n"
                            "template <>\n"
                            "struct VecTraits<UnusualType> : VecTraits<UnusualType::T> { };\n"
                            "}";
        const char expected[] = "struct UnusualType { "
                                "vtkm :: Id X ; "
                                "} ; "
                                "namespace vtkm { "
                                "struct VecTraits<UnusualType> : VecTraits < vtkm :: Id > { } ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing26() { // #11090
        const char code[] = "namespace M {\n"
                            "    struct A;\n"
                            "    struct B;\n"
                            "    struct C;\n"
                            "    template<typename T>\n"
                            "    struct F {};\n"
                            "    template<>\n"
                            "    struct F<B> : F<A> {};\n"
                            "    template<>\n"
                            "    struct F<C> : F<A> {};\n"
                            "}\n"
                            "namespace N {\n"
                            "    using namespace M;\n"
                            "    using A = void;\n"
                            "}\n";
        const char expected[] = "namespace M { "
                                "struct A ; struct B ; struct C ; "
                                "struct F<C> ; struct F<B> ; struct F<A> ; "
                                "struct F<B> : F<A> { } ; struct F<C> : F<A> { } ; "
                                "} "
                                "namespace N { "
                                "using namespace M ; "
                                "} "
                                "struct M :: F<A> { } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing27() { // #11670
        const char code[] = "namespace N {\n"
                            "    template <class T>\n"
                            "    struct S {\n"
                            "        using iterator = T*;\n"
                            "        iterator begin();\n"
                            "    };\n"
                            "}\n"
                            "using I = N::S<int>;\n"
                            "void f() {\n"
                            "    I::iterator iter;\n"
                            "}\n";
        const char expected[] = "namespace N { struct S<int> ; } "
                                "void f ( ) { int * iter ; } "
                                "struct N :: S<int> { int * begin ( ) ; } ;";
        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing28() { // #11795
        const char code[] = "void f() {\n"
                            "    using T = int;\n"
                            "    T* p{ new T };\n"
                            "}\n";
        const char expected[] = "void f ( ) { int * p { new int } ; }";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing29() { // #11981
        const char code[] = "using T = int*;\n"
                            "void f(T = T()) {}\n";
        const char expected[] = "void f ( int * = ( int * ) 0 ) { }";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing30() {
        {
            const char code[] = "using std::to_string;\n" // #8454
                                "void f() {\n"
                                "    std::string str = to_string(1);\n"
                                "}\n";
            const char expected[] = "void f ( ) { std :: string str ; str = std :: to_string ( 1 ) ; }";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "using std::cout, std::endl, std::cerr, std::ostringstream;\n"
                                "cerr << \"abc\";\n";
            const char expected[] = "std :: cerr << \"abc\" ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "using std::string_view_literals::operator\"\"sv;\n";
            const char expected[] = "using std :: string_view_literals :: operator\"\"sv ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "template <typename T>\n"
                                "class vector : public ::std::vector<T> {\n"
                                "public:\n"
                                "    using ::std::vector<T>::vector;\n"
                                "    vector() {}\n"
                                "};\n"
                                "vector <int> v;\n";
            const char expected[] = "class vector<int> ; "
                                    "vector<int> v ; "
                                    "class vector<int> : public :: std :: vector<int> { "
                                    "public: "
                                    "using vector<int> = :: std :: vector<int> :: vector<int> ; "
                                    "vector<int> ( ) { } "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "using namespace ::std;\n"
                                "void f(const char* c) {\n"
                                "    cout << std::string(c) << \"abc\";\n"
                                "}\n";
            const char expected[] = "using namespace :: std ; " // TODO: simplify cout?
                                    "void f ( const char * c ) { "
                                    "cout << std :: string ( c ) << \"abc\" ; "
                                    "}";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS(
                "[test.cpp:3:5]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable cout [valueFlowBailoutIncompleteVar]\n",
                errout_str());
        }
        {
            const char code[] = "class T : private std::vector<std::pair<std::string, const int*>> {\n" // #12521
                                "    using std::vector<std::pair<std::string, const int*>>::empty;\n"
                                "};\n";
            const char expected[] = "class T : private std :: vector < std :: pair < std :: string , const int * > > { "
                                    "using empty = std :: vector < std :: pair < std :: string , const int * > > :: empty ; "
                                    "} ;";
            ASSERT_EQUALS(expected, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
    }

    void simplifyUsing31() { // #11899
        const char code[] = "struct B {\n"
                            "    B();\n"
                            "    void f();\n"
                            "};\n"
                            "struct D : B {\n"
                            "    using B::B;\n"
                            "    void g() {\n"
                            "        B::f();\n"
                            "    }\n"
                            "    B b;\n"
                            "};\n";
        const char expected[] = "struct B { "
                                "B ( ) ; "
                                "void f ( ) ; "
                                "} ; "
                                "struct D : B { "
                                "using B = B :: B ; "
                                "void g ( ) { "
                                "B :: f ( ) ; "
                                "} "
                                "B b ; "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing32() {
        const char code[] = "using T = int*;\n" // #11430
                            "T f() { return T{}; }\n"
                            "T g() { return T(malloc(4)); }\n";
        const char expected[] = "int * f ( ) { return ( int * ) 0 ; } "
                                "int * g ( ) { return ( int * ) ( malloc ( 4 ) ) ; }";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());

        const char code2[] = "struct S {\n" // #13095
                             "    using reference_type = int&;\n"
                             "    reference_type get();\n"
                             "    int i;\n"
                             "};\n"
                             "auto S::get() -> reference_type {\n"
                             "    return i;\n"
                             "}\n";
        const char expected2[] = "struct S { "
                                 "int & get ( ) ; "
                                 "int i ; "
                                 "} ; "
                                 "auto S :: get ( ) . int & { return i ; }";
        ASSERT_EQUALS(expected2, tok(code2));
        TODO_ASSERT_EQUALS("",
                           "[test.cpp:6:1]: (debug) auto token with no type. [autoNoType]\n"
                           "", errout_str());

        const char code3[] = "using V = int*;\n"
                             "auto g() -> const volatile V { return {}; }\n";
        const char expected3[] = "auto g ( ) . const volatile int * { return { } ; }";
        ASSERT_EQUALS(expected3, tok(code3));
        TODO_ASSERT_EQUALS("",
                           "[test.cpp:2:1]: (debug) auto token with no type. [autoNoType]\n"
                           "", errout_str());
    }

    void simplifyUsing33() { // #13090
        const char code[] = "namespace N {\n"
                            "    using T = int;\n"
                            "    T f() { return (T)0; }\n"
                            "}\n"
                            "struct S {\n"
                            "    using U = int;\n"
                            "    U g() { return (U)0; }\n"
                            "};\n";
        const char expected[] = "namespace N { "
                                "int f ( ) { return ( int ) 0 ; } "
                                "} "
                                "struct S { "
                                "int g ( ) { return ( int ) 0 ; } "
                                "} ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing34() { // #13295
        const char code[] = "struct A {\n"
                            "    static const int a = 1;\n"
                            "};\n"
                            "using B = struct A;\n"
                            "void g(int);\n"
                            "void f() {\n"
                            "    g({ B::a });\n"
                            "}\n";
        const char expected[] = "struct A { "
                                "static const int a = 1 ; "
                                "} ; "
                                "void g ( int ) ; "
                                "void f ( ) { "
                                "g ( { A :: a } ) ; "
                                "}";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing35() { // #13492
        const char code[] = "using a = b;\n"
                            "using c = d;\n";
        const char expected[] = ";";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing36() {
        const char code[] = "using A::a;\n"
                            "int c = B<int>::a;\n";
        const char expected[] = "int c ; c = B < int > :: a ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing37() {
        const char code1[] = "using fp1_t = int(*)(int);\n"
                             "using fp2_t = int(* const)(int);\n"
                             "fp1_t fp1;\n"
                             "fp2_t fp2;\n";
        const char expected1[] = "int ( * fp1 ) ( int ) ; int ( * const fp2 ) ( int ) ;";
        ASSERT_EQUALS(expected1, tok(code1));
        ASSERT_EQUALS("", errout_str());

        const char code2[] = "using f_t = int(int);\n"
                             "f_t* fp1;\n"
                             "f_t* const fp2;\n";
        const char expected2[] = "int ( * fp1 ) ( int ) ; int ( * const fp2 ) ( int ) ;";
        ASSERT_EQUALS(expected2, tok(code2));
        ASSERT_EQUALS("", errout_str());

        const char code3[] = "using FP = std::string (*)();\n" // #14421
                             "using FPC = std::string (*const)();\n"
                             "FP fp;\n"
                             "FPC fpc{};\n";
        const char expected3[] = "std :: string ( * fp ) ( ) ; std :: string ( * const fpc ) ( ) { } ;";
        ASSERT_EQUALS(expected3, tok(code3));
        ASSERT_EQUALS("", errout_str());

        const char code4[] = "using F = void(*)(char);\n" // #14429
                             "F f(int);\n";
        const char expected4[] = "void * f ( char ) ;";
        ASSERT_EQUALS(expected4, tok(code4));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing38() {
        const char code[] = "using std::begin;\n" // #14424
                            "using std::end;\n"
                            "Unknown begin;\n"
                            "int end;\n";
        const char expected[] = "Unknown begin ; int end ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing39() {
        const char code[] = "using std::wstring;\n" // #14578
                            "wstring ws;";
        const char expected[] = "std :: wstring ws ;";
        ASSERT_EQUALS(expected, tok(code));
        ASSERT_EQUALS("", errout_str());
        ASSERT_EQUALS(expected, tok(code, dinit(TokOptions, $.cppstd = Standards::CPP03)));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing8970() {
        const char code[] = "using V = std::vector<int>;\n"
                            "struct A {\n"
                            "    V p;\n"
                            "};";

        const char expected[] = "struct A { "
                                "std :: vector < int > p ; "
                                "} ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing8971() {
        const char code[] = "class A {\n"
                            "public:\n"
                            "    using V = std::vector<double>;\n"
                            "};\n"
                            "using V = std::vector<int>;\n"
                            "class I {\n"
                            "private:\n"
                            "    A::V v_;\n"
                            "    V v2_;\n"
                            "};";

        const char expected[] = "class A { "
                                "public: "
                                "} ; "
                                "class I { "
                                "private: "
                                "std :: vector < double > v_ ; "
                                "std :: vector < int > v2_ ; "
                                "} ;";

        ASSERT_EQUALS(expected, tok(code));
    }

    void simplifyUsing8976() {
        const char code[] = "using mystring = std::string;";

        const char exp[] = ";";

        ASSERT_EQUALS(exp, tok(code));
    }

    void simplifyUsing9040() {
        const char code[] = "using BOOL = unsigned; int i;";

        const char exp[] = "int i ;";

        ASSERT_EQUALS(exp, tok(code, dinit(TokOptions, $.type = Platform::Unix32)));
        ASSERT_EQUALS(exp, tok(code, dinit(TokOptions, $.type = Platform::Unix64)));
        ASSERT_EQUALS(exp, tok(code, dinit(TokOptions, $.type = Platform::Win32A)));
        ASSERT_EQUALS(exp, tok(code, dinit(TokOptions, $.type = Platform::Win32W)));
        ASSERT_EQUALS(exp, tok(code, dinit(TokOptions, $.type = Platform::Win64)));
    }

    void simplifyUsing9042() {
        const char code[] = "template <class T>\n"
                            "class c {\n"
                            "    int i = 0;\n"
                            "    c() { i--; }\n"
                            "};\n"
                            "template <class T>\n"
                            "class s {};\n"
                            "using BOOL = char;";

        const char exp[] = "template < class T > "
                           "class c { "
                           "int i ; i = 0 ; "
                           "c ( ) { i -- ; } "
                           "} ; "
                           "template < class T > class s { } ;";

        ASSERT_EQUALS(exp, tok(code, dinit(TokOptions, $.type = Platform::Win64)));
    }

    void simplifyUsing9191() {
        const char code[] = "namespace NS1 {\n"
                            "  namespace NS2 {\n"
                            "    using _LONG = signed long long;\n"
                            "  }\n"
                            "}\n"
                            "void f1() {\n"
                            "  using namespace NS1;\n"
                            "  NS2::_LONG A;\n"
                            "}\n"
                            "void f2() {\n"
                            "  using namespace NS1::NS2;\n"
                            "  _LONG A;\n"
                            "}";

        const char exp[] = "void f1 ( ) { "
                           "using namespace NS1 ; "
                           "signed long long A ; "
                           "} "
                           "void f2 ( ) { "
                           "using namespace NS1 :: NS2 ; "
                           "signed long long A ; "
                           "}";

        ASSERT_EQUALS(exp, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing9381() {
        const char code[] = "namespace ns {\n"
                            "    class Result;\n"
                            "    using UniqueResultPtr = std::unique_ptr<Result>;\n"
                            "    class A {\n"
                            "    public:\n"
                            "        void func(UniqueResultPtr);\n"
                            "    };\n"
                            "    void A::func(UniqueResultPtr) {\n"
                            "    }\n"
                            "}";
        const char exp[] = "namespace ns { "
                           "class Result ; "
                           "class A { "
                           "public: "
                           "void func ( std :: unique_ptr < Result > ) ; "
                           "} ; "
                           "void A :: func ( std :: unique_ptr < Result > ) { "
                           "} "
                           "}";

        ASSERT_EQUALS(exp, tok(code));
    }

    void simplifyUsing9385() {
        {
            const char code[] = "class A {\n"
                                "public:\n"
                                "    using Foo = int;\n"
                                "    A(Foo foo);\n"
                                "    ~A();\n"
                                "    void func(Foo foo);\n"
                                "};\n"
                                "A::A(Foo) { }\n"
                                "A::~A() { Foo foo; }\n"
                                "void A::func(Foo) { }";
            const char exp[] = "class A { "
                               "public: "
                               "A ( int foo ) ; "
                               "~ A ( ) ; "
                               "void func ( int foo ) ; "
                               "} ; "
                               "A :: A ( int ) { } "
                               "A :: ~ A ( ) { int foo ; } "
                               "void A :: func ( int ) { }";
            ASSERT_EQUALS(exp, tok(code));
        }
        {
            const char code[] = "class A {\n"
                                "public:\n"
                                "    struct B {\n"
                                "        using Foo = int;\n"
                                "        B(Foo foo);\n"
                                "        ~B();\n"
                                "        void func(Foo foo);\n"
                                "    };\n"
                                "};\n"
                                "A::B::B(Foo) { }\n"
                                "A::B::~B() { Foo foo; }\n"
                                "void A::B::func(Foo) { }";
            const char exp[] = "class A { "
                               "public: "
                               "struct B { "
                               "B ( int foo ) ; "
                               "~ B ( ) ; "
                               "void func ( int foo ) ; "
                               "} ; "
                               "} ; "
                               "A :: B :: B ( int ) { } "
                               "A :: B :: ~ B ( ) { int foo ; } "
                               "void A :: B :: func ( int ) { }";
            ASSERT_EQUALS(exp, tok(code));
        }
    }

    void simplifyUsing9388() {
        const char code[] = "class A {\n"
                            "public:\n"
                            "    using Type = int;\n"
                            "    A(Type&);\n"
                            "    Type& t_;\n"
                            "};\n"
                            "A::A(Type& t) : t_(t) { }";
        const char exp[] = "class A { "
                           "public: "
                           "A ( int & ) ; "
                           "int & t_ ; "
                           "} ; "
                           "A :: A ( int & t ) : t_ ( t ) { }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void simplifyUsing9518() {
        const char code[] = "namespace a {\n"
                            "using a = enum {};\n"
                            "}";
        const char exp[] = "namespace a { "
                           "enum a { } ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void simplifyUsing9757() {
        const char code[] = "enum class Type_t { Nil = 0 };\n"
                            "template<Type_t type> class MappedType { };\n"
                            "template<> class MappedType<Type_t::Nil> { using type = void; };\n"
                            "std::string to_string (Example::Type_t type) {\n"
                            "   switch (type) {}\n"
                            "}";
        const char exp[] = "enum class Type_t { Nil = 0 } ; "
                           "class MappedType<Type_t::Nil> ; "
                           "template < Type_t type > class MappedType { } ; "
                           "class MappedType<Type_t::Nil> { } ; "
                           "std :: string to_string ( Example :: Type_t type ) { "
                           "switch ( type ) { } }";
        ASSERT_EQUALS(exp, tok(code));
    }

    void simplifyUsing10008() {
        const char code[] = "namespace ns {\n"
                            "    using ArrayType = std::vector<int>;\n"
                            "}\n"
                            "using namespace ns;\n"
                            "static void f() {\n"
                            "    const ArrayType arr;\n"
                            "}";
        const char exp[] = "using namespace ns ; "
                           "static void f ( ) { "
                           "const std :: vector < int > arr ; "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
    }

    void simplifyUsing10054() { // debug: Executable scope 'x' with unknown function.
        {
            // original example: using "namespace external::ns1;" but redundant qualification
            const char code[] = "namespace external {\n"
                                "    namespace ns1 {\n"
                                "        template <int size> struct B { };\n"
                                "        using V = B<sizeof(bool)>;\n"
                                "    }\n"
                                "}\n"
                                "namespace ns {\n"
                                "    struct A {\n"
                                "        void f(external::ns1::V);\n"
                                "    };\n"
                                "}\n"
                                "using namespace external::ns1;\n"
                                "namespace ns {\n"
                                "    void A::f(external::ns1::V) {}\n"
                                "}";
            const char exp[]  = "namespace external { "
                                "namespace ns1 { "
                                "struct B<1> ; "
                                "} "
                                "} "
                                "namespace ns { "
                                "struct A { "
                                "void f ( external :: ns1 :: B<1> ) ; "
                                "} ; "
                                "} "
                                "using namespace external :: ns1 ; "
                                "namespace ns { "
                                "void A :: f ( external :: ns1 :: B<1> ) { } "
                                "} "
                                "struct external :: ns1 :: B<1> { } ;";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            // no using "namespace external::ns1;"
            const char code[] = "namespace external {\n"
                                "    namespace ns1 {\n"
                                "        template <int size> struct B { };\n"
                                "        using V = B<sizeof(bool)>;\n"
                                "    }\n"
                                "}\n"
                                "namespace ns {\n"
                                "    struct A {\n"
                                "        void f(external::ns1::V);\n"
                                "    };\n"
                                "}\n"
                                "namespace ns {\n"
                                "    void A::f(external::ns1::V) {}\n"
                                "}";
            const char exp[]  = "namespace external { "
                                "namespace ns1 { "
                                "struct B<1> ; "
                                "} "
                                "} "
                                "namespace ns { "
                                "struct A { "
                                "void f ( external :: ns1 :: B<1> ) ; "
                                "} ; "
                                "} "
                                "namespace ns { "
                                "void A :: f ( external :: ns1 :: B<1> ) { } "
                                "} "
                                "struct external :: ns1 :: B<1> { } ;";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            // using "namespace external::ns1;" without redundant qualification
            const char code[] = "namespace external {\n"
                                "    namespace ns1 {\n"
                                "        template <int size> struct B { };\n"
                                "        using V = B<sizeof(bool)>;\n"
                                "    }\n"
                                "}\n"
                                "namespace ns {\n"
                                "    struct A {\n"
                                "        void f(external::ns1::V);\n"
                                "    };\n"
                                "}\n"
                                "using namespace external::ns1;\n"
                                "namespace ns {\n"
                                "    void A::f(V) {}\n"
                                "}";
            const char exp[]  = "namespace external { "
                                "namespace ns1 { "
                                "struct B<1> ; "
                                "} "
                                "} "
                                "namespace ns { "
                                "struct A { "
                                "void f ( external :: ns1 :: B<1> ) ; "
                                "} ; "
                                "} "
                                "using namespace external :: ns1 ; "
                                "namespace ns { "
                                "void A :: f ( external :: ns1 :: B<1> ) { } "
                                "} "
                                "struct external :: ns1 :: B<1> { } ;";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            // using "namespace external::ns1;" without redundant qualification on declaration and definition
            const char code[] = "namespace external {\n"
                                "    namespace ns1 {\n"
                                "        template <int size> struct B { };\n"
                                "        using V = B<sizeof(bool)>;\n"
                                "    }\n"
                                "}\n"
                                "using namespace external::ns1;\n"
                                "namespace ns {\n"
                                "    struct A {\n"
                                "        void f(V);\n"
                                "    };\n"
                                "}\n"
                                "namespace ns {\n"
                                "    void A::f(V) {}\n"
                                "}";
            const char exp[]  = "namespace external { "
                                "namespace ns1 { "
                                "struct B<1> ; "
                                "} "
                                "} "
                                "using namespace external :: ns1 ; "
                                "namespace ns { "
                                "struct A { "
                                "void f ( external :: ns1 :: B<1> ) ; "
                                "} ; "
                                "} "
                                "namespace ns { "
                                "void A :: f ( external :: ns1 :: B<1> ) { } "
                                "} "
                                "struct external :: ns1 :: B<1> { } ;";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "namespace external {\n"
                                "    template <int size> struct B { };\n"
                                "    namespace ns1 {\n"
                                "        using V = B<sizeof(bool)>;\n"
                                "    }\n"
                                "}\n"
                                "namespace ns {\n"
                                "    struct A {\n"
                                "        void f(external::ns1::V);\n"
                                "    };\n"
                                "}\n"
                                "namespace ns {\n"
                                "    void A::f(external::ns1::V) {}\n"
                                "}";
            const char exp[]  = "namespace external { "
                                "struct B<1> ; "
                                "} "
                                "namespace ns { "
                                "struct A { "
                                "void f ( external :: B<1> ) ; "
                                "} ; "
                                "} "
                                "namespace ns { "
                                "void A :: f ( external :: B<1> ) { } "
                                "} "
                                "struct external :: B<1> { } ;";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "template <int size> struct B { };\n"
                                "namespace external {\n"
                                "    namespace ns1 {\n"
                                "        using V = B<sizeof(bool)>;\n"
                                "    }\n"
                                "}\n"
                                "namespace ns {\n"
                                "    struct A {\n"
                                "        void f(external::ns1::V);\n"
                                "    };\n"
                                "}\n"
                                "namespace ns {\n"
                                "    void A::f(external::ns1::V) {}\n"
                                "}";
            const char exp[]  = "struct B<1> ; "
                                "namespace ns { "
                                "struct A { "
                                "void f ( B<1> ) ; "
                                "} ; "
                                "} "
                                "namespace ns { "
                                "void A :: f ( B<1> ) { } "
                                "} "
                                "struct B<1> { } ;";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
    }

    void simplifyUsing10136() {
        {
            const char code[] = "class B {\n"
                                "public:\n"
                                "    using V = std::vector<char>;\n"
                                "    virtual void f(const V&) const = 0;\n"
                                "};\n"
                                "class A final : public B {\n"
                                "public:\n"
                                "    void f(const V&) const override;\n"
                                "};\n"
                                "void A::f(const std::vector<char>&) const { }";
            const char exp[]  = "class B { "
                                "public: "
                                "virtual void f ( const std :: vector < char > & ) const = 0 ; "
                                "} ; "
                                "class A : public B { "
                                "public: "
                                "void f ( const std :: vector < char > & ) const override ; "
                                "} ; "
                                "void A :: f ( const std :: vector < char > & ) const { }";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "namespace NS1 {\n"
                                "    class B {\n"
                                "    public:\n"
                                "        using V = std::vector<char>;\n"
                                "        virtual void f(const V&) const = 0;\n"
                                "    };\n"
                                "}\n"
                                "namespace NS2 {\n"
                                "    class A : public NS1::B {\n"
                                "    public:\n"
                                "        void f(const V&) const override;\n"
                                "    };\n"
                                "    namespace NS3 {\n"
                                "        class C : public A {\n"
                                "        public:\n"
                                "            void f(const V&) const override;\n"
                                "        };\n"
                                "        void C::f(const V&) const { }\n"
                                "    }\n"
                                "    void A::f(const V&) const { }\n"
                                "}\n"
                                "void foo() {\n"
                                "    NS2::A a;\n"
                                "    NS2::NS3::C c;\n"
                                "    NS1::B::V v;\n"
                                "    a.f(v);\n"
                                "    c.f(v);\n"
                                "}";
            const char exp[]  = "namespace NS1 { "
                                "class B { "
                                "public: "
                                "virtual void f ( const std :: vector < char > & ) const = 0 ; "
                                "} ; "
                                "} "
                                "namespace NS2 { "
                                "class A : public NS1 :: B { "
                                "public: "
                                "void f ( const std :: vector < char > & ) const override ; "
                                "} ; "
                                "namespace NS3 { "
                                "class C : public A { "
                                "public: "
                                "void f ( const std :: vector < char > & ) const override ; "
                                "} ; "
                                "void C :: f ( const std :: vector < char > & ) const { } "
                                "} "
                                "void A :: f ( const std :: vector < char > & ) const { } "
                                "} "
                                "void foo ( ) { "
                                "NS2 :: A a ; "
                                "NS2 :: NS3 :: C c ; "
                                "std :: vector < char > v ; "
                                "a . f ( v ) ; "
                                "c . f ( v ) ; "
                                "}";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "foo::ResultCodes_e\n"
                                "GemImpl::setR(const ::foo::s _ipSource)\n"
                                "{\n"
                                "   M3_LOG_EE_DEBUG();\n"
                                "   using MLSource = foo::s::Literal;\n"
                                "   auto ret = foo::ResultCodes_e::NO_ERROR;\n"
                                "   M3_LOG_INFO(\"foo(\" << static_cast<int>(_ipSource) << \")\");\n"
                                "   return ret;\n"
                                "}\n"
                                "foo::ResultCodes_e\n"
                                "GemImpl::getF(::foo::s &_ipSource)\n"
                                "{\n"
                                "   M3_LOG_EE_DEBUG();\n"
                                "   auto ret = foo::ResultCodes_e::NO_ERROR;\n"
                                "   return ret;\n"
                                "}\n"
                                "foo::ResultCodes_e\n"
                                "GemImpl::setF(const ::foo::s _ipSource)\n"
                                "{\n"
                                "   M3_LOG_EE_DEBUG();\n"
                                "   using MLSource = foo::s::Literal;\n"
                                "   auto ret = foo::ResultCodes_e::NO_ERROR;\n"
                                "   return ret;\n"
                                "}";
            (void)tok(code); // don't crash
            ignore_errout(); // we do not care about the output
        }
    }

    void simplifyUsing10171() {
        const char code[] = "namespace ns {\n"
                            "    class A {\n"
                            "    public:\n"
                            "        using V = std::vector<unsigned char>;\n"
                            "        virtual void f(const V&) const = 0;\n"
                            "    };\n"
                            "    class B : public A {\n"
                            "    public:\n"
                            "        void f(const V&) const override;\n"
                            "    };\n"
                            "}\n"
                            "namespace ns {\n"
                            "    void B::f(const std::vector<unsigned char>&) const { }\n"
                            "}";
        const char exp[] = "namespace ns { "
                           "class A { "
                           "public: "
                           "virtual void f ( const std :: vector < unsigned char > & ) const = 0 ; "
                           "} ; "
                           "class B : public A { "
                           "public: "
                           "void f ( const std :: vector < unsigned char > & ) const override ; "
                           "} ; "
                           "} "
                           "namespace ns { "
                           "void B :: f ( const std :: vector < unsigned char > & ) const { } "
                           "}";
        ASSERT_EQUALS(exp, tok(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyUsing10172() {
        {
            const char code[] = "namespace ns {\n"
                                "    class A {\n"
                                "    public:\n"
                                "        using h = std::function<void()>;\n"
                                "    };\n"
                                "    class B : public A {\n"
                                "        void f(h);\n"
                                "    };\n"
                                "}\n"
                                "namespace ns {\n"
                                "    void B::f(h) { }\n"
                                "}";
            const char exp[] = "namespace ns { "
                               "class A { "
                               "public: "
                               "} ; "
                               "class B : public A { "
                               "void f ( std :: function < void ( ) > ) ; "
                               "} ; "
                               "} "
                               "namespace ns { "
                               "void B :: f ( std :: function < void ( ) > ) { } "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "namespace ns {\n"
                                "namespace external {\n"
                                "class A {\n"
                                "public: \n"
                                "  using h = std::function<void()>;\n"
                                "};\n"
                                "class B : public A {\n"
                                "  void f(h);\n"
                                "};\n"
                                "}\n"
                                "}\n"
                                "namespace ns {\n"
                                "namespace external {\n"
                                "void B::f(h) {}\n"
                                "}\n"
                                "}";
            const char exp[] = "namespace ns { "
                               "namespace external { "
                               "class A { "
                               "public: "
                               "} ; "
                               "class B : public A { "
                               "void f ( std :: function < void ( ) > ) ; "
                               "} ; "
                               "} "
                               "} "
                               "namespace ns { "
                               "namespace external { "
                               "void B :: f ( std :: function < void ( ) > ) { } "
                               "} "
                               "}";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
    }

    void simplifyUsing10173() {
        {
            const char code[] = "std::ostream & operator<<(std::ostream &s, const Pr<st> p) {\n"
                                "    return s;\n"
                                "}\n"
                                "void foo() {\n"
                                "    using Pr = d::Pr<st>;\n"
                                "    Pr p;\n"
                                "}\n"
                                "void bar() {\n"
                                "   Pr<st> p;\n"
                                "}";
            const char exp[]  = "std :: ostream & operator<< ( std :: ostream & s , const Pr < st > p ) { "
                                "return s ; "
                                "} "
                                "void foo ( ) { "
                                "d :: Pr < st > p ; "
                                "} "
                                "void bar ( ) { "
                                "Pr < st > p ; "
                                "}";
            ASSERT_EQUALS(exp, tok(code));
            ASSERT_EQUALS("", errout_str());
        }
        {
            const char code[] = "namespace defsa {\n"
                                "void xxx::foo() {\n"
                                "   using NS1 = v1::l;\n"
                                "}\n"
                                "void xxx::bar() {\n"
                                "   using NS1 = v1::l;\n"
                                "}\n"
                                "void xxx::foobar() {\n"
                                "   using NS1 = v1::l;\n"
                                "}\n"
                                "}";
            const char exp[]  = "namespace defsa { "
                                "void xxx :: foo ( ) { "
                                "} "
                                "void xxx :: bar ( ) { "
                                "} "
                                "void xxx :: foobar ( ) { "
                                "} "
                                "}";
            ASSERT_EQUALS(exp, tok(code));
            ignore_errout(); // we do not care about the output
        }
    }

    void simplifyUsing10335() {
        const char code[] = "using uint8_t = unsigned char;\n"
                            "enum E : uint8_t { E0 };";
        const char exp[]  = "enum E : unsigned char { E0 } ;";
        ASSERT_EQUALS(exp, tok(code));
    }

    void simplifyUsing10720() { // hang/segmentation fault
        const char code[] = "template <typename... Ts>\n"
                            "struct S {};\n"
                            "#define STAMP(thiz, prev) using thiz = S<prev, prev, prev, prev, prev, prev, prev, prev, prev, prev>;\n"
                            "STAMP(A, int);\n"
                            "STAMP(B, A);\n"
                            "STAMP(C, B);\n";
        (void)tok(code, dinit(TokOptions, $.preprocess = true));
        TODO_ASSERT(startsWith(errout_str(), "[test.cpp:6]: (debug) Failed to parse 'using C = S < S < S < int"));
    }

    void simplifyUsing13873() { // function declaration
        const char code1[] = "using NS1::f;\n"
                             "namespace NS1 { void f(); }\n";
        ASSERT_EQUALS("namespace NS1 { void f ( ) ; }", tok(code1));

        const char code2[] = "using NS1::f;\n"
                             "void bar() { f(); }\n";
        ASSERT_EQUALS("void bar ( ) { NS1 :: f ( ) ; }", tok(code2));

        const char code3[] = "using NS1::f;\n"
                             "namespace NS1 { void* f(); }\n";
        ASSERT_EQUALS("namespace NS1 { void * f ( ) ; }", tok(code3));
    }

    void scopeInfo1() {
        const char code[] = "struct A {\n"
                            "    enum class Mode { UNKNOWN, ENABLED, NONE, };\n"
                            "};\n"
                            "\n"
                            "namespace spdlog { class logger; }\n"
                            "using LoggerPtr = std::shared_ptr<spdlog::logger>;";
        (void)tok(code);
        ASSERT_EQUALS("", errout_str());
    }

    void scopeInfo2() {
        const char code[] = "struct A {\n"
                            "    using Map = std::map<int, int>;\n"
                            "    Map values;\n"
                            "};\n"
                            "\n"
                            "static void getInitialProgramState(const A::Map& vars = A::Map {})\n"
                            "{}\n";
        (void)tok(code);
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestSimplifyUsing)
