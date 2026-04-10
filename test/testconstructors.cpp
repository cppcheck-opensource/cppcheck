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
#include "fixture.h"
#include "helpers.h"
#include "standards.h"
#include "settings.h"

#include <cstddef>

class TestConstructors : public TestFixture {
public:
    TestConstructors() : TestFixture("TestConstructors") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::style).severity(Severity::warning).build();
    const Settings settings_i = settingsBuilder(settings).certainty(Certainty::inconclusive).build();

    struct CheckOptions
    {
        bool inconclusive = false;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings settings1 = options.inconclusive ? settings_i : settings;

        check_(file, line, code, settings1);
    }

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const Settings& settings1) {
        SimpleTokenizer tokenizer(settings1, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check class constructors..
        CheckClass checkClass(&tokenizer, &settings1, this);
        checkClass.constructors();
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(simple1);
        TEST_CASE(simple2);
        TEST_CASE(simple3);
        TEST_CASE(simple4);
        TEST_CASE(simple5); // ticket #2560
        TEST_CASE(simple6); // ticket #4085 - uninstantiated template class
        TEST_CASE(simple7); // ticket #4531
        TEST_CASE(simple8);
        TEST_CASE(simple9); // ticket #4574
        TEST_CASE(simple10); // ticket #4388
        TEST_CASE(simple11); // ticket #4536, #6214
        TEST_CASE(simple12); // ticket #4620
        TEST_CASE(simple13); // #5498 - no constructor, c++11 assignments
        TEST_CASE(simple14); // #6253 template base
        TEST_CASE(simple15); // #8942 multiple arguments, decltype
        TEST_CASE(simple16); // copy members with c++11 init
        TEST_CASE(simple17); // #10360
        TEST_CASE(simple18);

        TEST_CASE(noConstructor1);
        TEST_CASE(noConstructor2);
        TEST_CASE(noConstructor3);
        TEST_CASE(noConstructor4);
        TEST_CASE(noConstructor5);
        TEST_CASE(noConstructor6); // ticket #4386
        TEST_CASE(noConstructor7); // ticket #4391
        TEST_CASE(noConstructor8); // ticket #4404
        TEST_CASE(noConstructor9); // ticket #4419
        TEST_CASE(noConstructor10); // ticket #6614
        TEST_CASE(noConstructor11); // ticket #3552
        TEST_CASE(noConstructor12); // #8951 - member initialization
        TEST_CASE(noConstructor13); // #9998
        TEST_CASE(noConstructor14); // #10770
        TEST_CASE(noConstructor15); // #5499
        TEST_CASE(noConstructor16);

        TEST_CASE(forwardDeclaration); // ticket #4290/#3190

        TEST_CASE(initvar_with_this);       // BUG 2190300
        TEST_CASE(initvar_if);              // BUG 2190290
        TEST_CASE(initvar_operator_eq1);     // BUG 2190376
        TEST_CASE(initvar_operator_eq2);     // BUG 2190376
        TEST_CASE(initvar_operator_eq3);
        TEST_CASE(initvar_operator_eq4);     // ticket #2204
        TEST_CASE(initvar_operator_eq5);     // ticket #4119
        TEST_CASE(initvar_operator_eq6);
        TEST_CASE(initvar_operator_eq7);
        TEST_CASE(initvar_operator_eq8);
        TEST_CASE(initvar_operator_eq9);
        TEST_CASE(initvar_operator_eq10);
        TEST_CASE(initvar_operator_eq11);
        TEST_CASE(initvar_same_classname);      // BUG 2208157
        TEST_CASE(initvar_chained_assign);      // BUG 2270433
        TEST_CASE(initvar_2constructors);       // BUG 2270353
        TEST_CASE(initvar_constvar);
        TEST_CASE(initvar_mutablevar);
        TEST_CASE(initvar_staticvar);
        TEST_CASE(initvar_brace_init);
        TEST_CASE(initvar_union);
        TEST_CASE(initvar_delegate);       // ticket #4302
        TEST_CASE(initvar_delegate2);
        TEST_CASE(initvar_derived_class);
        TEST_CASE(initvar_derived_pod_struct_with_union); // #11101

        TEST_CASE(initvar_private_constructor);     // BUG 2354171 - private constructor
        TEST_CASE(initvar_derived_private_constructor);
        TEST_CASE(initvar_copy_constructor); // ticket #1611
        TEST_CASE(initvar_nested_constructor); // ticket #1375
        TEST_CASE(initvar_nocopy1);            // ticket #2474
        TEST_CASE(initvar_nocopy2);            // ticket #2484
        TEST_CASE(initvar_nocopy3);            // ticket #3611
        TEST_CASE(initvar_nocopy4);            // ticket #9247
        TEST_CASE(initvar_with_member_function_this); // ticket #4824

        TEST_CASE(initvar_destructor);      // No variables need to be initialized in a destructor
        TEST_CASE(initvar_func_ret_func_ptr); // ticket #4449

        TEST_CASE(initvar_alias); // #6921

        TEST_CASE(initvar_templateMember); // #7205
        TEST_CASE(initvar_smartptr); // #10237

        TEST_CASE(operatorEqSTL);

        TEST_CASE(uninitVar1);
        TEST_CASE(uninitVar2);
        TEST_CASE(uninitVar3);
        TEST_CASE(uninitVar4);
        TEST_CASE(uninitVar5);
        TEST_CASE(uninitVar6);
        TEST_CASE(uninitVar7);
        TEST_CASE(uninitVar8);
        TEST_CASE(uninitVar9);  // ticket #1730
        TEST_CASE(uninitVar10); // ticket #1993
        TEST_CASE(uninitVar11);
        TEST_CASE(uninitVar12); // ticket #2078
        TEST_CASE(uninitVar13); // ticket #1195
        TEST_CASE(uninitVar14); // ticket #2149
        TEST_CASE(uninitVar15);
        TEST_CASE(uninitVar16);
        TEST_CASE(uninitVar17);
        TEST_CASE(uninitVar18); // ticket #2465
        TEST_CASE(uninitVar19); // ticket #2792
        TEST_CASE(uninitVar20); // ticket #2867
        TEST_CASE(uninitVar21); // ticket #2947
        TEST_CASE(uninitVar22); // ticket #3043
        TEST_CASE(uninitVar23); // ticket #3702
        TEST_CASE(uninitVar24); // ticket #3190
        TEST_CASE(uninitVar25); // ticket #4789
        TEST_CASE(uninitVar26);
        TEST_CASE(uninitVar27); // ticket #5170 - rtl::math::setNan(&d)
        TEST_CASE(uninitVar28); // ticket #6258
        TEST_CASE(uninitVar29);
        TEST_CASE(uninitVar30); // ticket #6417
        TEST_CASE(uninitVar31); // ticket #8271
        TEST_CASE(uninitVar32); // ticket #8835
        TEST_CASE(uninitVar33); // ticket #10295
        TEST_CASE(uninitVar34); // ticket #10841
        TEST_CASE(uninitVar35);
        TEST_CASE(uninitVar36);
        TEST_CASE(uninitVar37);
        TEST_CASE(uninitVarEnum1);
        TEST_CASE(uninitVarEnum2); // ticket #8146
        TEST_CASE(uninitVarStream);
        TEST_CASE(uninitVarTypedef);
        TEST_CASE(uninitVarMemset);
        TEST_CASE(uninitVarArray1);
        TEST_CASE(uninitVarArray2);
        TEST_CASE(uninitVarArray3);
        TEST_CASE(uninitVarArray4);
        TEST_CASE(uninitVarArray5);
        TEST_CASE(uninitVarArray6);
        TEST_CASE(uninitVarArray7);
        TEST_CASE(uninitVarArray8);
        TEST_CASE(uninitVarArray9); // ticket #6957, #6959
        TEST_CASE(uninitVarArray10);
        TEST_CASE(uninitVarArray11);
        TEST_CASE(uninitVarArray2D);
        TEST_CASE(uninitVarArray3D);
        TEST_CASE(uninitVarCpp11Init1);
        TEST_CASE(uninitVarCpp11Init2);
        TEST_CASE(uninitVarStruct1);       // ticket #2172
        TEST_CASE(uninitVarStruct2);       // ticket #838
        TEST_CASE(uninitVarUnion1);        // ticket #3196
        TEST_CASE(uninitVarUnion2);
        TEST_CASE(uninitMissingFuncDef);   // can't expand function in constructor
        TEST_CASE(privateCtor1);           // If constructor is private..
        TEST_CASE(privateCtor2);           // If constructor is private..
        TEST_CASE(function);               // Function is not variable
        TEST_CASE(uninitVarPublished);     // Borland C++: Variables in the published section are auto-initialized
        TEST_CASE(uninitVarInheritClassInit); // Borland C++: if class inherits from TObject, all variables are initialized
        TEST_CASE(uninitOperator);         // No FP about uninitialized 'operator[]'
        TEST_CASE(uninitFunction1);        // No FP when initialized in function
        TEST_CASE(uninitFunction2);        // No FP when initialized in function
        TEST_CASE(uninitFunction3);        // No FP when initialized in function
        TEST_CASE(uninitFunction4);
        TEST_CASE(uninitFunction5);
        TEST_CASE(uninitSameClassName);    // No FP when two classes have the same name
        TEST_CASE(uninitFunctionOverload); // No FP when there are overloaded functions
        TEST_CASE(uninitVarOperatorEqual); // ticket #2415
        TEST_CASE(uninitVarPointer);       // ticket #3801
        TEST_CASE(uninitConstVar);
        TEST_CASE(constructors_crash1);    // ticket #5641
        TEST_CASE(classWithOperatorInName);// ticket #2827
        TEST_CASE(templateConstructor);    // ticket #7942
        TEST_CASE(typedefArray);           // ticket #5766

        TEST_CASE(uninitAssignmentWithOperator);  // ticket #7429
        TEST_CASE(uninitCompoundAssignment);      // ticket #7429
        TEST_CASE(uninitComparisonAssignment);    // ticket #7429

        TEST_CASE(uninitTemplate1); // ticket #7372

        TEST_CASE(unknownTemplateType);
    }


    void simple1() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:1:1]: (style) The class 'Fred' does not declare a constructor although it has private member variables which likely require initialization. [noConstructor]\n", errout_str());

        check("struct Fred\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:1:1]: (style) The struct 'Fred' does not declare a constructor although it has private member variables which likely require initialization. [noConstructor]\n", errout_str());
    }


    void simple2() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() : i(0) { }\n"
              "    Fred(Fred const & other) : i(other.i) {}\n"
              "    Fred(Fred && other) : i(other.i) {}\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { i = 0; }\n"
              "    Fred(Fred const & other) {i=other.i}\n"
              "    Fred(Fred && other) {i=other.i}\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { }\n"
              "    Fred(Fred const & other) {}\n"
              "    Fred(Fred && other) {}\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:5:5]: (warning) Member variable 'Fred::i' is not initialized in the copy constructor. [uninitMemberVar]\n"
                      "[test.cpp:6:5]: (warning) Member variable 'Fred::i' is not initialized in the move constructor. [uninitMemberVar]\n", errout_str());

        check("struct Fred\n"
              "{\n"
              "    Fred() { }\n"
              "    Fred(Fred const & other) {}\n"
              "    Fred(Fred && other) {}\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:4:5]: (warning) Member variable 'Fred::i' is not initialized in the copy constructor. [uninitMemberVar]\n"
                      "[test.cpp:5:5]: (warning) Member variable 'Fred::i' is not initialized in the move constructor. [uninitMemberVar]\n", errout_str());
    }


    void simple3() {
        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred() :i(0)\n"
              "{ }");
        ASSERT_EQUALS("", errout_str());

        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ i = 0; }");
        ASSERT_EQUALS("", errout_str());

        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }");
        ASSERT_EQUALS("[test.cpp:6:7]: (warning) Member variable 'Fred::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }


    void simple4() {
        check("struct Fred\n"
              "{\n"
              "    Fred();\n"
              "    explicit Fred(int _i);\n"
              "    Fred(Fred const & other);\n"
              "    int i;\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }\n"
              "Fred::Fred(int _i)\n"
              "{\n"
              "    i = _i;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:8:7]: (warning, inconclusive) Member variable 'Fred::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void simple5() { // ticket #2560
        check("namespace Nsp\n"
              "{\n"
              "    class B { };\n"
              "}\n"
              "class Altren : public Nsp::B\n"
              "{\n"
              "public:\n"
              "    Altren () : Nsp::B(), mValue(0)\n"
              "    {\n"
              "    }\n"
              "private:\n"
              "    int mValue;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void simple6() { // ticket #4085 - uninstantiated template class
        check("template <class T> struct A {\n"
              "    A<T>() { x = 0; }\n"
              "    A<T>(const T & t) { x = t.x; }\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("template <class T> struct A {\n"
              "    A<T>() : x(0) { }\n"
              "    A<T>(const T & t) : x(t.x) { }\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("template <class T> struct A {\n"
              "    A<T>() : x(0) { }\n"
              "    A<T>(const T & t) : x(t.x) { }\n"
              "private:\n"
              "    int x;\n"
              "    int y;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Member variable 'A::y' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:3:5]: (warning) Member variable 'A::y' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void simple7() { // ticket #4531
        check("class Fred;\n"
              "struct Fred {\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void simple8() {
        check("struct Fred { int x; };\n"
              "class Barney { Fred fred; };\n"
              "class Wilma { struct Betty { int x; } betty; };");
        ASSERT_EQUALS("[test.cpp:2:1]: (style) The class 'Barney' does not declare a constructor although it has private member variables which likely require initialization. [noConstructor]\n"
                      "[test.cpp:3:1]: (style) The class 'Wilma' does not declare a constructor although it has private member variables which likely require initialization. [noConstructor]\n", errout_str());
    }

    void simple9() { // ticket #4574
        check("class Unknown::Fred {\n"
              "public:\n"
              "    Fred() : x(0) { }\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void simple10() {
        check("class Fred {\n" // ticket #4388
              "public:\n"
              "    Fred() = default;\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Member variable 'Fred::x' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("struct S {\n" // #9391
              "    S() = default;\n"
              "    ~S() = default;\n"
              "    S(const S&) = default;\n"
              "    S(S&&) = default;\n"
              "    S& operator=(const S&) = default;\n"
              "    S& operator=(S&&) = default;\n"
              "    int i;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Member variable 'S::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void simple11() { // ticket #4536, #6214
        check("class Fred {\n"
              "public:\n"
              "    Fred() {}\n"
              "private:\n"
              "    int x = 0;\n"
              "    int y = f();\n"
              "    int z{0};\n"
              "    int (*pf[2])(){nullptr, nullptr};\n"
              "    int a[2][3] = {{1,2,3},{4,5,6}};\n"
              "    int b{1}, c{2};\n"
              "    int d, e{3};\n"
              "    int f{4}, g;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Member variable 'Fred::d' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:3:5]: (warning) Member variable 'Fred::g' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void simple12() { // ticket #4620
        check("class Fred {\n"
              "    int x;\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "    void Init(int i = 0);\n"
              "};\n"
              "void Fred::Init(int i) { x = i; }");
        ASSERT_EQUALS("", errout_str());

        check("class Fred {\n"
              "    int x;\n"
              "    int y;\n"
              "public:\n"
              "    Fred() { Init(0); }\n"
              "    void Init(int i, int j = 0);\n"
              "};\n"
              "void Fred::Init(int i, int j) { x = i; y = j; }");
        ASSERT_EQUALS("", errout_str());
    }

    void simple13() { // #5498
        check("class Fred {\n"
              "    int x=1;\n"
              "    int *y=0;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void simple14() { // #6253 template base
        check("class Fred : public Base<A, B> {"
              "public:"
              "    Fred()\n"
              "    :Base<A, B>(1),\n"
              "     x(1)\n"
              "    {}\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred : public Base<A, B> {"
              "public:"
              "    Fred()\n"
              "    :Base<A, B>{1},\n"
              "     x{1}\n"
              "    {}\n"
              "private:\n"
              "    int x;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void simple15() { // #8942
        check("class A\n"
              "{\n"
              "public:\n"
              "  int member;\n"
              "};\n"
              "class B\n"
              "{\n"
              "public:\n"
              "  B(const decltype(A::member)& x, const decltype(A::member)& y) : x(x), y(y) {}\n"
              "private:\n"
              "  const decltype(A::member)& x;\n"
              "  const decltype(A::member)& y;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void simple16() {
        check("struct S {\n"
              "    int i{};\n"
              "    S() = default;\n"
              "    S(const S& s) {}\n"
              "    S& operator=(const S& s) { return *this; }\n"
              "};\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:5]: (warning, inconclusive) Member variable 'S::i' is not assigned in the copy constructor. Should it be copied? [missingMemberCopy]\n"
                      "[test.cpp:5:8]: (warning) Member variable 'S::i' is not assigned a value in 'S::operator='. [operatorEqVarError]\n",
                      errout_str());

        check("struct S {\n"
              "    int i;\n"
              "    S() : i(0) {}\n"
              "    S(const S& s) {}\n"
              "    S& operator=(const S& s) { return *this; }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'S::i' is not initialized in the copy constructor. [uninitMemberVar]\n"
                      "[test.cpp:5:8]: (warning) Member variable 'S::i' is not assigned a value in 'S::operator='. [operatorEqVarError]\n",
                      errout_str());
    }

    void simple17() { // #10360
        check("class Base {\n"
              "public:\n"
              "    virtual void Copy(const Base& Src) = 0;\n"
              "};\n"
              "class Derived : public Base {\n"
              "public:\n"
              "    Derived() : i(0) {}\n"
              "    Derived(const Derived& Src);\n"
              "    void Copy(const Base& Src) override;\n"
              "    int i;\n"
              "};\n"
              "Derived::Derived(const Derived& Src) {\n"
              "    Copy(Src);\n"
              "}\n"
              "void Derived::Copy(const Base& Src) {\n"
              "    auto d = dynamic_cast<const Derived&>(Src);\n"
              "    i = d.i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void simple18() { // #13302
        check("struct S {};\n"
              "class C1 { S& r; };\n"
              "class C2 { S* p; };\n");
        ASSERT_EQUALS("[test.cpp:2:1]: (style) The class 'C1' does not declare a constructor although it has private member variables which likely require initialization. [noConstructor]\n"
                      "[test.cpp:3:1]: (style) The class 'C2' does not declare a constructor although it has private member variables which likely require initialization. [noConstructor]\n",
                      errout_str());
    }

    void noConstructor1() {
        // There are nonstatic member variables - constructor is needed
        check("class Fred\n"
              "{\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:1:1]: (style) The class 'Fred' does not declare a constructor although it has private member variables which likely require initialization. [noConstructor]\n", errout_str());
    }

    void noConstructor2() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    static void foobar();\n"
              "};\n"
              "\n"
              "void Fred::foobar()\n"
              "{ }");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor3() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    static int foobar;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor4() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int foobar;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor5() {
        check("namespace Foo\n"
              "{\n"
              "    int i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor6() {
        // ticket #4386
        check("class Ccpucycles {\n"
              "    friend class foo::bar;\n"
              "    Ccpucycles() :\n"
              "    m_v(0), m_b(true)\n"
              "    {}\n"
              "private:\n"
              "    cpucyclesT m_v;\n"
              "    bool m_b;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor7() {
        // ticket #4391
        check("short bar;\n"
              "class foo;");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor8() {
        // ticket #4404
        check("class LineSegment;\n"
              "class PointArray  { };\n"
              "void* tech_ = NULL;");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor9() {
        // ticket #4419
        check("class CGreeting : public CGreetingBase<char> {\n"
              "public:\n"
              " CGreeting() : CGreetingBase<char>(), MessageSet(false) {}\n"
              "private:\n"
              " bool MessageSet;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor10() {
        // ticket #6614
        check("class A : public wxDialog\n"
              "{\n"
              "private:\n"
              "    DECLARE_EVENT_TABLE()\n"
              "public:\n"
              "    A(wxWindow *parent,\n"
              "      wxWindowID id = 1,\n"
              "      const wxString &title = wxT(" "),\n"
              "      const wxPoint& pos = wxDefaultPosition,\n"
              "      const wxSize& size = wxDefaultSize,\n"
              "      long style = wxDIALOG_NO_PARENT | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX);\n"
              "    virtual ~A();\n"
              "private:\n"
              "    wxTimer *WxTimer1;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }


    void noConstructor11() { // #3552
        check("class Fred { int x; };\n"
              "union U { int y; Fred fred; };");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor12() { // #8951
        check("class Fred { int x{0}; };");
        ASSERT_EQUALS("", errout_str());

        check("class Fred { int x=0; };");
        ASSERT_EQUALS("", errout_str());

        check("class Fred { int x[1]={0}; };"); // #8850
        ASSERT_EQUALS("", errout_str());

        check("class Fred { int x[1]{0}; };");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor13() { // #9998
        check("struct C { int v; };\n"
              "struct B { C c[5] = {}; };\n"
              "struct A {\n"
              "    A() {}\n"
              "    B b;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor14() { // #10770
        check("typedef void (*Func)();\n"
              "class C {\n"
              "public:\n"
              "    void f();\n"
              "private:\n"
              "    Func fp = nullptr;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void noConstructor15() { // #5499
        check("class C {\n"
              "private:\n"
              "    int i1 = 0;\n"
              "    int i2;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'C::i2' is not initialized. [uninitMemberVarPrivate]\n", errout_str());

        check("class C {\n"
              "private:\n"
              "    int i1;\n"
              "    int i2;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:1:1]: (style) The class 'C' does not declare a constructor although it has private member variables which likely require initialization. [noConstructor]\n", errout_str());

        check("class C {\n"
              "public:\n"
              "    C(int i) : i1(i) {}\n"
              "private:\n"
              "    int i1;\n"
              "    int i2;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Member variable 'C::i2' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void noConstructor16() {
        check("struct S {\n" // #14546
              "    int a = 0, b;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:1:10]: (warning) Member variable 'S::b' has no initializer. [uninitMemberVarNoCtor]\n", errout_str());

        check("struct S {\n"
              "    int a, b;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    // ticket #4290 "False Positive: style (noConstructor): The class 'foo' does not have a constructor."
    // ticket #3190 "SymbolDatabase: Parse of sub class constructor fails"
    void forwardDeclaration() {
        check("class foo;\n"
              "int bar;");
        ASSERT_EQUALS("", errout_str());

        check("class foo;\n"
              "class foo;");
        ASSERT_EQUALS("", errout_str());

        check("class foo{};\n"
              "class foo;");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_with_this() {
        check("struct Fred\n"
              "{\n"
              "    Fred()\n"
              "    { this->i = 0; }\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_if() {
        check("struct Fred\n"
              "{\n"
              "    Fred()\n"
              "    {\n"
              "        if (true)\n"
              "            i = 0;\n"
              "        else\n"
              "            i = 1;\n"
              "    }\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_operator_eq1() {
        // Bug 2190376 and #3820 - False positive, Uninitialized member variable with operator=

        check("struct Fred\n"
              "{\n"
              "    int i;\n"
              "\n"
              "    Fred()\n"
              "    { i = 0; }\n"
              "\n"
              "    Fred(const Fred &fred)\n"
              "    { *this = fred; }\n"
              "\n"
              "    const Fred & operator=(const Fred &fred)\n"
              "    { i = fred.i; return *this; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct Fred {\n"
              "    int i;\n"
              "\n"
              "    Fred(const Fred &fred)\n"
              "    { (*this) = fred; }\n"
              "\n"
              "    const Fred & operator=(const Fred &fred)\n"
              "    { i = fred.i; return *this; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct A\n"
              "{\n"
              "  A() : i(0), j(0) {}\n"
              "\n"
              "  A &operator=(const int &value)\n"
              "  {\n"
              "    i = value;\n"
              "    return (*this);\n"
              "  }\n"
              "\n"
              "  int i;\n"
              "  int j;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }


    void initvar_operator_eq2() {
        check("struct Fred\n"
              "{\n"
              "    Fred() { i = 0; }\n"
              "    void operator=(const Fred &fred) { }\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:10]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='. [operatorEqVarError]\n", errout_str());
    }

    void initvar_operator_eq3() {
        check("struct Fred\n"
              "{\n"
              "    Fred() { Init(); }\n"
              "    void operator=(const Fred &fred) { Init(); }\n"
              "private:\n"
              "    void Init() { i = 0; }\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_operator_eq4() {
        check("class Fred\n"
              "{\n"
              "    int i;\n"
              "public:\n"
              "    Fred() : i(5) { }\n"
              "    Fred & operator=(const Fred &fred)\n"
              "    {\n"
              "        if (&fred != this)\n"
              "        {\n"
              "        }\n"
              "        return *this\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:12]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='. [operatorEqVarError]\n", errout_str());

        check("class Fred\n"
              "{\n"
              "    int * i;\n"
              "public:\n"
              "    Fred() : i(NULL) { }\n"
              "    Fred & operator=(const Fred &fred)\n"
              "    {\n"
              "        if (&fred != this)\n"
              "        {\n"
              "        }\n"
              "        return *this\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:12]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='. [operatorEqVarError]\n", errout_str());

        check("class Fred\n"
              "{\n"
              "    const int * i;\n"
              "public:\n"
              "    Fred() : i(NULL) { }\n"
              "    Fred & operator=(const Fred &fred)\n"
              "    {\n"
              "        if (&fred != this)\n"
              "        {\n"
              "        }\n"
              "        return *this\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:12]: (warning) Member variable 'Fred::i' is not assigned a value in 'Fred::operator='. [operatorEqVarError]\n", errout_str());

        check("class Fred\n"
              "{\n"
              "    const int i;\n"
              "public:\n"
              "    Fred() : i(5) { }\n"
              "    Fred & operator=(const Fred &fred)\n"
              "    {\n"
              "        if (&fred != this)\n"
              "        {\n"
              "        }\n"
              "        return *this\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_operator_eq5() { // #4119 - false positive when using swap to assign variables
        check("class Fred {\n"
              "    int i;\n"
              "public:\n"
              "    Fred() : i(5) { }\n"
              "    ~Fred() { }\n"
              "    Fred(const Fred &fred) : i(fred.i) { }\n"
              "    Fred & operator=(const Fred &rhs) {\n"
              "        Fred(rhs).swap(*this);\n"
              "        return *this;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_operator_eq6() { // std::vector
        check("struct Fred {\n"
              "    uint8_t data;\n"
              "    Fred & operator=(const Fred &rhs) {\n"
              "        return *this;\n"
              "    }\n"
              "};",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:12]: (warning, inconclusive) Member variable 'Fred::data' is not assigned a value in 'Fred::operator='. [operatorEqVarError]\n", errout_str());

        check("struct Fred {\n"
              "    std::vector<int> ints;\n"
              "    Fred & operator=(const Fred &rhs) {\n"
              "        return *this;\n"
              "    }\n"
              "};",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:12]: (warning, inconclusive) Member variable 'Fred::ints' is not assigned a value in 'Fred::operator='. [operatorEqVarError]\n", errout_str());

        check("struct Fred {\n"
              "    Data data;\n"
              "    Fred & operator=(const Fred &rhs) {\n"
              "        return *this;\n"
              "    }\n"
              "};",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:12]: (warning, inconclusive) Member variable 'Fred::data' is not assigned a value in 'Fred::operator='. [operatorEqVarError]\n", errout_str());
    }

    void initvar_operator_eq7() {
        check("struct B {\n"
              "    virtual void CopyImpl(const B& Src) = 0;\n"
              "    void Copy(const B& Src);\n"
              "};\n"
              "struct D : B {};\n"
              "struct DD : D {\n"
              "    void CopyImpl(const B& Src) override;\n"
              "    DD& operator=(const DD& Src);\n"
              "    int i{};\n"
              "};\n"
              "DD& DD::operator=(const DD& Src) {\n"
              "    if (this != &Src)\n"
              "        Copy(Src);\n"
              "    return *this;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_operator_eq8() {
        check("struct B {\n"
              "    int b;\n"
              "};\n"
              "struct D1 : B {\n"
              "    D1& operator=(const D1& src);\n"
              "    int d1;\n"
              "};\n"
              "struct D2 : D1 {\n"
              "    D2& operator=(const D2& src);\n"
              "    int d2;\n"
              "};\n"
              "struct D3 : D2 {\n"
              "    D3& operator=(const D3& src) {\n"
              "        D1::operator=(src);\n"
              "        d3_1 = src.d3_1;\n"
              "    }\n"
              "    int d3_1;\n"
              "    int d3_2;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:13:9]: (warning) Member variable 'D3::d3_2' is not assigned a value in 'D3::operator='. [operatorEqVarError]\n"
                      "[test.cpp:13:9]: (warning) Member variable 'D3::d2' is not assigned a value in 'D3::operator='. [operatorEqVarError]\n", errout_str());
    }

    void initvar_operator_eq9() {  // ticket #13203
        check("struct S {\n"
              "    int* m_data;\n"
              "    S() : m_data(new int()) {}\n"
              "    S& operator=(const S& s) {\n"
              "        if (&s != this) {\n"
              "            *(m_data) = *(s.m_data);\n"
              "        }\n"
              "        return *this;\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_operator_eq10() {
        check("struct S {\n"
              "    int* m_data;\n"
              "    S() : m_data(new int()) {}\n"
              "    S& operator=(const S& s) {\n"
              "        if (&s != this) {\n"
              "            (*m_data) = *(s.m_data);\n"
              "        }\n"
              "        return *this;\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_operator_eq11() {
        check("struct S {\n"
              "    int* m_data;\n"
              "    S() : m_data(new int()) {}\n"
              "    S& operator=(const S& s) {\n"
              "        return *this;\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:4:8]: (warning) Member variable 'S::m_data' is not assigned a value in 'S::operator='. [operatorEqVarError]\n", errout_str());
    }


    void initvar_same_classname() {
        // Bug 2208157 - False positive: Uninitialized variable, same class name

        check("void func1()\n"
              "{\n"
              "    class Fred\n"
              "    {\n"
              "        int a;\n"
              "        Fred() { a = 0; }\n"
              "    };\n"
              "}\n"
              "\n"
              "void func2()\n"
              "{\n"
              "    class Fred\n"
              "    {\n"
              "        int b;\n"
              "        Fred() { b = 0; }\n"
              "    };\n"
              "}");

        ASSERT_EQUALS("", errout_str());

        check("void func1()\n"
              "{\n"
              "    struct Fred\n"
              "    {\n"
              "        int a;\n"
              "        Fred() { a = 0; }\n"
              "    };\n"
              "}\n"
              "\n"
              "void func2()\n"
              "{\n"
              "    class Fred\n"
              "    {\n"
              "        int b;\n"
              "        Fred() { b = 0; }\n"
              "    };\n"
              "}");

        ASSERT_EQUALS("", errout_str());

        check("void func1()\n"
              "{\n"
              "    struct Fred\n"
              "    {\n"
              "        int a;\n"
              "        Fred() { a = 0; }\n"
              "    };\n"
              "}\n"
              "\n"
              "void func2()\n"
              "{\n"
              "    struct Fred\n"
              "    {\n"
              "        int b;\n"
              "        Fred() { b = 0; }\n"
              "    };\n"
              "}");

        ASSERT_EQUALS("", errout_str());

        check("class Foo {\n"
              "    void func1()\n"
              "    {\n"
              "        struct Fred\n"
              "        {\n"
              "            int a;\n"
              "            Fred() { a = 0; }\n"
              "        };\n"
              "    }\n"
              "\n"
              "    void func2()\n"
              "    {\n"
              "        struct Fred\n"
              "        {\n"
              "            int b;\n"
              "            Fred() { b = 0; }\n"
              "        };\n"
              "    }\n"
              "};");

        ASSERT_EQUALS("", errout_str());

        check("class Foo {\n"
              "    void func1()\n"
              "    {\n"
              "        struct Fred\n"
              "        {\n"
              "            int a;\n"
              "            Fred() { }\n"
              "        };\n"
              "    }\n"
              "\n"
              "    void func2()\n"
              "    {\n"
              "        struct Fred\n"
              "        {\n"
              "            int b;\n"
              "            Fred() { }\n"
              "        };\n"
              "    }\n"
              "};");

        ASSERT_EQUALS("[test.cpp:7:13]: (warning) Member variable 'Fred::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:16:13]: (warning) Member variable 'Fred::b' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void initvar_chained_assign() {
        // Bug 2270433 - Uninitialized variable false positive on chained assigns

        check("struct c\n"
              "{\n"
              "    c();\n"
              "\n"
              "    int m_iMyInt1;\n"
              "    int m_iMyInt2;\n"
              "}\n"
              "\n"
              "c::c()\n"
              "{\n"
              "    m_iMyInt1 = m_iMyInt2 = 0;\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }


    void initvar_2constructors() {
        check("struct c\n"
              "{\n"
              "    c();\n"
              "    explicit c(bool b);"
              "\n"
              "    void InitInt();\n"
              "\n"
              "    int m_iMyInt;\n"
              "    int m_bMyBool;\n"
              "}\n"
              "\n"
              "c::c()\n"
              "{\n"
              "    m_bMyBool = false;\n"
              "    InitInt();"
              "}\n"
              "\n"
              "c::c(bool b)\n"
              "{\n"
              "    m_bMyBool = b;\n"
              "    InitInt();\n"
              "}\n"
              "\n"
              "void c::InitInt()\n"
              "{\n"
              "    m_iMyInt = 0;\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }


    void initvar_constvar() {
        check("struct Fred\n"
              "{\n"
              "    const char *s;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred() : s(NULL)\n"
              "{ }");
        ASSERT_EQUALS("", errout_str());

        check("struct Fred\n"
              "{\n"
              "    const char *s;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{ s = NULL; }");
        ASSERT_EQUALS("", errout_str());

        check("struct Fred\n"
              "{\n"
              "    const char *s;\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{ }");
        ASSERT_EQUALS("[test.cpp:6:7]: (warning) Member variable 'Fred::s' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }


    void initvar_mutablevar() {
        check("class Foo {\n"
              "public:\n"
              "    Foo() { update(); }\n"
              "private:\n"
              "    void update() const;\n"
              "    mutable int x;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }


    void initvar_staticvar() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { }\n"
              "    static void *p;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }


    void initvar_brace_init() { // #10142
        check("class C\n"
              "{\n"
              "public:\n"
              "  C() {}\n"
              "\n"
              "private:\n"
              "  std::map<int, double> * values_{};\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }


    void initvar_union() {
        check("class Fred\n"
              "{\n"
              "    union\n"
              "    {\n"
              "        int a;\n"
              "        char b[4];\n"
              "    } U;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        U.a = 0;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "    union\n"
              "    {\n"
              "        int a;\n"
              "        char b[4];\n"
              "    } U;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9:5]: (warning) Member variable 'Fred::U' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }


    void initvar_delegate() {
        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) { }\n"
              "    A() : A(42) {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'A::number' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:5:5]: (warning) Member variable 'A::number' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) { number = n; }\n"
              "    A() : A(42) {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) : A() { }\n"
              "    A() {}\n"
              "};", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'A::number' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:5:5]: (warning, inconclusive) Member variable 'A::number' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) : A() { }\n"
              "    A() { number = 42; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) { }\n"
              "    A() : A{42} {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'A::number' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:5:5]: (warning) Member variable 'A::number' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) { number = n; }\n"
              "    A() : A{42} {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) : A{} { }\n"
              "    A() {}\n"
              "};", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'A::number' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:5:5]: (warning, inconclusive) Member variable 'A::number' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class A {\n"
              "    int number;\n"
              "public:\n"
              "    A(int n) : A{} { }\n"
              "    A() { number = 42; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        // Ticket #6675
        check("struct Foo {\n"
              "  Foo();\n"
              "  Foo(int foo);\n"
              "  int foo_;\n"
              "};\n"
              "Foo::Foo() : Foo(0) {}\n"
              "Foo::Foo(int foo) : foo_(foo) {}");
        ASSERT_EQUALS("", errout_str());

        // Noexcept ctors
        check("class A {\n"
              "private:\n"
              "    int _a;\n"
              "public:\n"
              "    A(const int a) noexcept : _a{a} {}\n"
              "    A() noexcept;\n"
              "};\n"
              "\n"
              "A::A() noexcept: A(0) {}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #8581
        check("class A {\n"
              "private:\n"
              "    int _a;\n"
              "public:\n"
              "    A(int a) : _a(a) {}\n"
              "    A(float a) : A(int(a)) {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        // Ticket #8258
        check("struct F{};\n"
              "struct Foo {\n"
              "    Foo(int a, F&& f, int b = 21) : _a(a), _b(b), _f(f) {}\n"
              "    Foo(int x, const char* value) : Foo(x, F(), 42) {}\n"
              "    Foo(int x, int* value) : Foo(x, F()) {}\n"
              "    int _a;\n"
              "    int _b;\n"
              "    F _f;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_delegate2() {
        check("class Foo {\n"
              "public:\n"
              "    explicit Foo(const Bar bar);\n"
              "    Foo(const std::string& id);\n"
              "    virtual ~RtpSession() { }\n"
              "protected:\n"
              "    bool a;\n"
              "    uint16_t b;\n"
              "};\n"
              "\n"
              "Foo::Foo(const Bar var)\n"
              "    : Foo(bar->getId())\n"
              "{\n"
              "}\n"
              "\n"
              "Foo::Foo(const std::string& id)\n"
              "    : a(true)\n"
              "    , b(0)\n"
              "{\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_derived_class() {
        check("class Base {\n" // #10161
              "public:\n"
              "  virtual void foo() = 0;\n"
              "  int x;\n" // <- uninitialized
              "};\n"
              "\n"
              "class Derived: public Base {\n"
              "public:\n"
              "  Derived() {}\n"
              "  void foo() override;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9:3]: (warning) Member variable 'Base::x' is not initialized in the constructor. Maybe it should be initialized directly in the class Base? [uninitDerivedMemberVar]\n", errout_str());

        check("struct A {\n" // #3462
              "    char ca;\n"
              "    A& operator=(const A& a) {\n"
              "        ca = a.ca;\n"
              "        return *this;\n"
              "    }\n"
              "};\n"
              "struct B : public A {\n"
              "    char cb;\n"
              "    B& operator=(const B& b) {\n"
              "        A::operator=(b);\n"
              "        return *this;\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:10:8]: (warning) Member variable 'B::cb' is not assigned a value in 'B::operator='. [operatorEqVarError]\n", errout_str());

        check("struct A {\n"
              "    char ca;\n"
              "    A& operator=(const A& a) {\n"
              "        return *this;\n"
              "    }\n"
              "};\n"
              "struct B : public A {\n"
              "    char cb;\n"
              "    B& operator=(const B& b) {\n"
              "        A::operator=(b);\n"
              "        return *this;\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:3:8]: (warning) Member variable 'A::ca' is not assigned a value in 'A::operator='. [operatorEqVarError]\n"
                      "[test.cpp:9:8]: (warning) Member variable 'B::cb' is not assigned a value in 'B::operator='. [operatorEqVarError]\n"
                      "[test.cpp:9:8]: (warning) Member variable 'B::ca' is not assigned a value in 'B::operator='. [operatorEqVarError]\n",
                      errout_str());

        check("class C : B {\n" // don't crash
              "    virtual C& operator=(C& c);\n"
              "};\n"
              "class D : public C {\n"
              "    virtual C& operator=(C& c) { return C::operator=(c); };\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct B;\n" // don't crash
              "struct D : B { D& operator=(const D&); };\n"
              "struct E : D { E& operator=(const E& rhs); };\n"
              "E& E::operator=(const E& rhs) {\n"
              "    if (this != &rhs)\n"
              "        D::operator=(rhs);\n"
              "    return *this;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("template <class T>\n" // #12128
              "struct B {\n"
              "    T x;\n"
              "};\n"
              "struct D : B<double> {\n"
              "    D(double x) : B{ x } {}\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct B {\n"
              "    int x;\n"
              "};\n"
              "struct D : B {\n"
              "    D(int i) : B{ i } {}\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_derived_pod_struct_with_union() {
        check("struct S {\n"
              "    union {\n"
              "        unsigned short       all;\n"
              "        struct {\n"
              "            unsigned char    flag1;\n"
              "            unsigned char    flag2;\n"
              "        };\n"
              "    };\n"
              "};\n"
              "\n"
              "class C : public S {\n"
              "public:\n"
              "    C() { all = 0; tick = 0; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    union {\n"
              "        unsigned short       all;\n"
              "        struct {\n"
              "            unsigned char    flag1;\n"
              "            unsigned char    flag2;\n"
              "        };\n"
              "    };\n"
              "};\n"
              "\n"
              "class C : public S {\n"
              "public:\n"
              "    C() { flag1 = flag2 = 0; tick = 0; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    union {\n"
              "        unsigned short       all;\n"
              "        struct {\n"
              "            unsigned char    flag1;\n"
              "            unsigned char    flag2;\n"
              "        };\n"
              "    };\n"
              "};\n"
              "\n"
              "class C : public S {\n"
              "public:\n"
              "    C() {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:13:5]: (warning) Member variable 'S::all' is not initialized in the constructor. Maybe it should be initialized directly in the class S? [uninitDerivedMemberVar]\n"
                      "[test.cpp:13:5]: (warning) Member variable 'S::flag1' is not initialized in the constructor. Maybe it should be initialized directly in the class S? [uninitDerivedMemberVar]\n"
                      "[test.cpp:13:5]: (warning) Member variable 'S::flag2' is not initialized in the constructor. Maybe it should be initialized directly in the class S? [uninitDerivedMemberVar]\n", errout_str());
    }

    void initvar_private_constructor() {
        {
            const Settings s = settingsBuilder(settings).cpp( Standards::CPP11).build();
            check("class Fred\n"
                  "{\n"
                  "private:\n"
                  "    int var;\n"
                  "    Fred();\n"
                  "};\n"
                  "Fred::Fred()\n"
                  "{ }", s);
            ASSERT_EQUALS("[test.cpp:7:7]: (warning) Member variable 'Fred::var' is not initialized in the constructor. [uninitMemberVarPrivate]\n", errout_str());
        }

        {
            const Settings s = settingsBuilder(settings).cpp(Standards::CPP03).build();
            check("class Fred\n"
                  "{\n"
                  "private:\n"
                  "    int var;\n"
                  "    Fred();\n"
                  "};\n"
                  "Fred::Fred()\n"
                  "{ }", s);
            ASSERT_EQUALS("", errout_str());
        }
    }
    void initvar_derived_private_constructor() {
        check("class B { int i; };\n"
              "class D : B {\n"
              "    explicit D(int) {}\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:1:1]: (style) The class 'B' does not declare a constructor although it has private member variables which likely require initialization. [noConstructor]\n"
                      "[test.cpp:3:14]: (warning) Member variable 'B::i' is not initialized in the constructor. Maybe it should be initialized directly in the class B? [uninitDerivedMemberVarPrivate]\n",
                      errout_str());
    }

    void initvar_copy_constructor() { // ticket #1611
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    std::string var;\n"
              "public:\n"
              "    Fred() { };\n"
              "    Fred(const Fred &) { };\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    std::string var;\n"
              "public:\n"
              "    Fred() { };\n"
              "    Fred(const Fred &) { };\n"
              "};", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:7:5]: (warning, inconclusive) Member variable 'Fred::var' is not assigned in the copy constructor. Should it be copied? [missingMemberCopy]\n", errout_str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    std::string var;\n"
              "public:\n"
              "    Fred();\n"
              "    Fred(const Fred &);\n"
              "};\n"
              "Fred::Fred() { };\n"
              "Fred::Fred(const Fred &) { };\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:10:7]: (warning, inconclusive) Member variable 'Fred::var' is not assigned in the copy constructor. Should it be copied? [missingMemberCopy]\n", errout_str());

        check("class Baz {};\n" // #8496
              "class Bar {\n"
              "public:\n"
              "    explicit Bar(Baz* pBaz = NULL) : i(0) {}\n"
              "    Bar(const Bar& bar) {}\n"
              "    int i;\n"
              "};\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:5]: (warning) Member variable 'Bar::i' is not initialized in the copy constructor. [uninitMemberVar]\n", errout_str());
    }

    void initvar_nested_constructor() { // ticket #1375
        check("class A {\n"
              "public:\n"
              "    A();\n"
              "    struct B {\n"
              "        explicit B(int x);\n"
              "        struct C {\n"
              "            explicit C(int y);\n"
              "            struct D {\n"
              "                int d;\n"
              "                explicit D(int z);\n"
              "            };\n"
              "            int c;\n"
              "        };\n"
              "        int b;\n"
              "    };\n"
              "private:\n"
              "    int a;\n"
              "    B b;\n"
              "};\n"
              "A::A(){}\n"
              "A::B::B(int x){}\n"
              "A::B::C::C(int y){}\n"
              "A::B::C::D::D(int z){}");
        // Note that the example code is not compilable. The A constructor must
        // explicitly initialize A::b. A warning for A::b is not necessary.
        ASSERT_EQUALS("[test.cpp:20:4]: (warning) Member variable 'A::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:21:7]: (warning) Member variable 'B::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:22:10]: (warning) Member variable 'C::c' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:23:13]: (warning) Member variable 'D::d' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class A {\n"
              "public:\n"
              "    A();\n"
              "    struct B {\n"
              "        explicit B(int x);\n"
              "        struct C {\n"
              "            explicit C(int y);\n"
              "            struct D {\n"
              "                D(const D &);\n"
              "                int d;\n"
              "            };\n"
              "            int c;\n"
              "        };\n"
              "        int b;\n"
              "    };\n"
              "private:\n"
              "    int a;\n"
              "    B b;\n"
              "};\n"
              "A::A(){}\n"
              "A::B::B(int x){}\n"
              "A::B::C::C(int y){}\n"
              "A::B::C::D::D(const A::B::C::D & d){}");
        // Note that the example code is not compilable. The A constructor must
        // explicitly initialize A::b. A warning for A::b is not necessary.
        ASSERT_EQUALS("[test.cpp:20:4]: (warning) Member variable 'A::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:21:7]: (warning) Member variable 'B::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:22:10]: (warning) Member variable 'C::c' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:23:13]: (warning) Member variable 'D::d' is not initialized in the copy constructor. [uninitMemberVar]\n", errout_str());

        check("class A {\n"
              "public:\n"
              "    A();\n"
              "    struct B {\n"
              "        explicit B(int x);\n"
              "        struct C {\n"
              "            explicit C(int y);\n"
              "            struct D {\n"
              "                struct E { int e; };\n"
              "                struct E d;\n"
              "                explicit D(const E &);\n"
              "            };\n"
              "            int c;\n"
              "        };\n"
              "        int b;\n"
              "    };\n"
              "private:\n"
              "    int a;\n"
              "    B b;\n"
              "};\n"
              "A::A(){}\n"
              "A::B::B(int x){}\n"
              "A::B::C::C(int y){}\n"
              "A::B::C::D::D(const A::B::C::D::E & e){}");
        // Note that the example code is not compilable. The A constructor must
        // explicitly initialize A::b. A warning for A::b is not necessary.
        ASSERT_EQUALS("[test.cpp:21:4]: (warning) Member variable 'A::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:22:7]: (warning) Member variable 'B::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:23:10]: (warning) Member variable 'C::c' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:24:13]: (warning) Member variable 'D::d' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void initvar_nocopy1() { // ticket #2474
        check("class B\n"
              "{\n"
              "    B (const B & Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class B\n"
              "{\n"
              "    B (B && Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class B\n"
              "{\n"
              "    B & operator= (const B & Var);\n"
              "public:\n"
              "    B ();\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B (const B & Var);\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A : public std::vector<int>\n"
              "{\n"
              "public:\n"
              "    A(const A &a);\n"
              "};\n"
              "class B\n"
              "{\n"
              "    A a;\n"
              "public:\n"
              "    B(){}\n"
              "    B(const B&){}\n"
              "    B(B &&){}\n"
              "    const B& operator=(const B&){return *this;}\n"
              "};", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:11:5]: (warning, inconclusive) Member variable 'B::a' is not assigned in the copy constructor. Should it be copied? [missingMemberCopy]\n"
                      "[test.cpp:12:5]: (warning, inconclusive) Member variable 'B::a' is not assigned in the move constructor. Should it be moved? [missingMemberCopy]\n"
                      "[test.cpp:13:14]: (warning, inconclusive) Member variable 'B::a' is not assigned a value in 'B::operator='. [operatorEqVarError]\n",
                      errout_str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B (B && Var);\n"
              "    int data;\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:13:5]: (warning) Member variable 'A::m_SemVar' is not initialized in the move constructor. [uninitMemberVar]\n", errout_str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B ();\n"
              "    B & operator= (const B & Var);\n"
              "    int data;\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    A(A &&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_nocopy2() { // ticket #2484
        check("class B\n"
              "{\n"
              "    B (B & Var);\n"
              "    B & operator= (const B & Var);\n"
              "    int data;\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B (B & Var);\n"
              "    B & operator= (const B & Var);\n"
              "    int data;\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B m_SemVar;\n"
              "public:\n"
              "    A(){}\n"
              "    A(const A&){}\n"
              "    const A& operator=(const A&){return *this;}\n"
              "};", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:12:5]: (warning) Member variable 'A::m_SemVar' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:13:5]: (warning) Member variable 'A::m_SemVar' is not initialized in the copy constructor. [uninitMemberVar]\n"
                      "[test.cpp:14:14]: (warning) Member variable 'A::m_SemVar' is not assigned a value in 'A::operator='. [operatorEqVarError]\n", errout_str());
    }

    void initvar_nocopy3() { // #3611 - unknown type is non-copyable
        check("struct A {\n"
              "    B b;\n"
              "    A() {}\n"
              "    A(const A& rhs) {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    B b;\n"
              "    A() {}\n"
              "    A(const A& rhs) {}\n"
              "};", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:5]: (warning, inconclusive) Member variable 'A::b' is not assigned in the copy constructor. Should it be copied? [missingMemberCopy]\n", errout_str());
    }

    void initvar_nocopy4() { // #9247
        check("struct S {\n"
              "    S(const S & s);\n"
              "    void S::Set(const T& val);\n"
              "    void S::Set(const U& val);\n"
              "    T t;\n"
              "};\n"
              "S::S(const S& s) {\n"
              "    Set(s.t);\n"
              "}\n"
              "void S::Set(const T& val) {\n"
              "    t = val;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_with_member_function_this() {
        check("struct Foo {\n"
              "  Foo(int m) { this->setMember(m); }\n"
              "  void setMember(int m) { member = m; }\n"
              "  int member;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_destructor() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    int var;\n"
              "public:\n"
              "    Fred() : var(0) {}\n"
              "    ~Fred() {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_func_ret_func_ptr() { // ticket #4449 (segmentation fault)
        check("class something {\n"
              "    int * ( something :: * process()) () { return 0; }\n"
              "    something() { process(); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_alias() { // #6921
        check("struct S {\n"
              "    int a;\n"
              "    S() {\n"
              "        int& pa = a;\n"
              "        pa = 4;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    int a;\n"
              "    S() {\n"
              "        int* pa = &a;\n"
              "        *pa = 4;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    int a[2];\n"
              "    S() {\n"
              "        int* pa = a;\n"
              "        for (int i = 0; i < 2; i++)\n"
              "            *pa++ = i;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    int* a[2];\n"
              "    S() {\n"
              "        int* pa = a[1];\n"
              "        *pa = 0;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Member variable 'S::a' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("struct S {\n"
              "    int a;\n"
              "    S() {\n"
              "        int pa = a;\n"
              "        pa = 4;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Member variable 'S::a' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void initvar_templateMember() {
        check("template<int n_>\n"
              "struct Wrapper {\n"
              "    static void foo(int * x) {\n"
              "        for (int i(0); i <= n_; ++i)\n"
              "            x[i] = 5;\n"
              "    }\n"
              "};\n"
              "class A {\n"
              "public:\n"
              "    static constexpr int dim = 5;\n"
              "    int x[dim + 1];\n"
              "    A() {\n"
              "        Wrapper<dim>::foo(x);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void initvar_smartptr() { // #10237
        // TODO: test should probably not pass without library
        const Settings s = settingsBuilder() /*.library("std.cfg")*/.build();
        check("struct S {\n"
              "    explicit S(const std::shared_ptr<S>& sp) {\n"
              "        set(*sp);\n"
              "    }\n"
              "    double get() const {\n"
              "        return d;\n"
              "    }\n"
              "    void set(const S& rhs) {\n"
              "        d = rhs.get();\n"
              "    }\n"
              "    double d;\n"
              "};", s);
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #8485
              "    explicit S(const T& rhs) { set(*rhs); }\n"
              "    void set(const S& v) {\n"
              "        d = v.d;\n"
              "    }\n"
              "    double d; \n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void operatorEqSTL() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    std::vector<int> ints;\n"
              "public:\n"
              "    Fred();\n"
              "    void operator=(const Fred &f);\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{ }\n"
              "\n"
              "void Fred::operator=(const Fred &f)\n"
              "{ }", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:13:12]: (warning, inconclusive) Member variable 'Fred::ints' is not assigned a value in 'Fred::operator='. [operatorEqVarError]\n", errout_str());

        const Settings s = settingsBuilder().certainty(Certainty::inconclusive).severity(Severity::style).severity(Severity::warning).library("std.cfg").build();
        check("struct S {\n"
              "    S& operator=(const S& s) { return *this; }\n"
              "    std::mutex m;\n"
              "};\n", s);
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar1() {
        check("enum ECODES\n"
              "{\n"
              "    CODE_1 = 0,\n"
              "    CODE_2 = 1\n"
              "};\n"
              "\n"
              "class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() {}\n"
              "\n"
              "private:\n"
              "    ECODES _code;\n"
              "};");

        ASSERT_EQUALS("[test.cpp:10:5]: (warning) Member variable 'Fred::_code' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());


        check("class A{};\n"
              "\n"
              "class B : public A\n"
              "{\n"
              "public:\n"
              "  B() {}\n"
              "private:\n"
              "  float f;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:3]: (warning) Member variable 'B::f' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class C\n"
              "{\n"
              "    FILE *fp;\n"
              "\n"
              "public:\n"
              "    explicit C(FILE *fp);\n"
              "};\n"
              "\n"
              "C::C(FILE *fp) {\n"
              "    C::fp = fp;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar2() {
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { (*this).i = 0; }\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar3() {
        // No FP when struct has constructor
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo() { }\n"
              "private:\n"
              "    struct Bar {\n"
              "        Bar();\n"
              "    };\n"
              "    Bar bars[2];\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        // Using struct that doesn't have constructor
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo() { }\n"
              "private:\n"
              "    struct Bar {\n"
              "        int x;\n"
              "    };\n"
              "    Bar bars[2];\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Foo::bars' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar4() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo() { bar.x = 0; }\n"
              "private:\n"
              "    struct Bar {\n"
              "        int x;\n"
              "    };\n"
              "    struct Bar bar;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar5() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo() { }\n"
              "    Foo &operator=(const Foo &)\n"
              "    { return *this; }\n"
              "    static int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar6() {
        check("class Foo : public Bar\n"
              "{\n"
              "public:\n"
              "    explicit Foo(int i) : Bar(mi=i) { }\n"
              "private:\n"
              "    int mi;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Foo : public Bar\n"
              "{\n"
              "public:\n"
              "    explicit Foo(int i) : Bar{mi=i} { }\n"
              "private:\n"
              "    int mi;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar7() {
        check("class Foo {\n"
              "    int a;\n"
              "public:\n"
              "    Foo() : a(0) {}\n"
              "    Foo& operator=(const Foo&);\n"
              "    void Swap(Foo& rhs);\n"
              "};\n"
              "\n"
              "void Foo::Swap(Foo& rhs) {\n"
              "    std::swap(a,rhs.a);\n"
              "}\n"
              "\n"
              "Foo& Foo::operator=(const Foo& rhs) {\n"
              "    Foo copy(rhs);\n"
              "    copy.Swap(*this);\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar8() {
        check("class Foo {\n"
              "    int a;\n"
              "public:\n"
              "    Foo() : a(0) {}\n"
              "    Foo& operator=(const Foo&);\n"
              "};\n"
              "\n"
              "Foo& Foo::operator=(const Foo& rhs) {\n"
              "    if (&rhs != this)\n"
              "    {\n"
              "    }\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:11]: (warning) Member variable 'Foo::a' is not assigned a value in 'Foo::operator='. [operatorEqVarError]\n", errout_str());
    }

    void uninitVar9() { // ticket #1730
        check("class Prefs {\n"
              "private:\n"
              "    int xasd;\n"
              "public:\n"
              "    explicit Prefs(wxSize size);\n"
              "};\n"
              "Prefs::Prefs(wxSize size)\n"
              "{\n"
              "    SetMinSize( wxSize( 48,48 ) );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:8]: (warning) Member variable 'Prefs::xasd' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar10() { // ticket #1993
        check("class A {\n"
              "public:\n"
              "        A();\n"
              "private:\n"
              "        int var1;\n"
              "        int var2;\n"
              "};\n"
              "A::A() : var1(0) { }");
        ASSERT_EQUALS("[test.cpp:8:4]: (warning) Member variable 'A::var2' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar11() {
        check("class A {\n"
              "public:\n"
              "        explicit A(int a = 0);\n"
              "private:\n"
              "        int var;\n"
              "};\n"
              "A::A(int a) { }");
        ASSERT_EQUALS("[test.cpp:7:4]: (warning) Member variable 'A::var' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar12() { // ticket #2078
        check("class Point\n"
              "{\n"
              "public:\n"
              "    Point()\n"
              "    {\n"
              "        Point(0, 0);\n"
              "    }\n"
              "    Point(int x, int y)\n"
              "        : x(x), y(y)\n"
              "    {}\n"
              "    int x, y;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Point::x' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:4:5]: (warning) Member variable 'Point::y' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar13() { // ticket #1195
        check("class A {\n"
              "private:\n"
              "    std::vector<int> *ints;\n"
              "public:\n"
              "    A()\n"
              "    {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:5]: (warning) Member variable 'A::ints' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar14() { // ticket #2149
        // no namespace
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo();\n"
              "private:\n"
              "    bool mMember;\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:6]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // single namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "    Foo::Foo()\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:10]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // constructor outside namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "Foo::Foo()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // constructor outside namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "Output::Foo::Foo()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11:14]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // constructor outside namespace with using, #4792
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "using namespace Output;"
              "Foo::Foo()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11:29]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // constructor in separate namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "namespace Output\n"
              "{\n"
              "    Foo::Foo()\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:13:10]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // constructor in different separate namespace
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "namespace Input\n"
              "{\n"
              "    Foo::Foo()\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // constructor in different separate namespace (won't compile)
        check("namespace Output\n"
              "{\n"
              "    class Foo\n"
              "    {\n"
              "    public:\n"
              "        Foo();\n"
              "    private:\n"
              "        bool mMember;\n"
              "    };\n"
              "}\n"
              "namespace Input\n"
              "{\n"
              "    Output::Foo::Foo()\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // constructor in nested separate namespace
        check("namespace A\n"
              "{\n"
              "    namespace Output\n"
              "    {\n"
              "        class Foo\n"
              "        {\n"
              "        public:\n"
              "            Foo();\n"
              "        private:\n"
              "            bool mMember;\n"
              "        };\n"
              "    }\n"
              "    namespace Output\n"
              "    {\n"
              "        Foo::Foo()\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:15:14]: (warning) Member variable 'Foo::mMember' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // constructor in nested different separate namespace
        check("namespace A\n"
              "{\n"
              "    namespace Output\n"
              "    {\n"
              "        class Foo\n"
              "        {\n"
              "        public:\n"
              "            Foo();\n"
              "        private:\n"
              "            bool mMember;\n"
              "        };\n"
              "    }\n"
              "    namespace Input\n"
              "    {\n"
              "        Foo::Foo()\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // constructor in nested different separate namespace
        check("namespace A\n"
              "{\n"
              "    namespace Output\n"
              "    {\n"
              "        class Foo\n"
              "        {\n"
              "        public:\n"
              "            Foo();\n"
              "        private:\n"
              "            bool mMember;\n"
              "        };\n"
              "    }\n"
              "    namespace Input\n"
              "    {\n"
              "        Output::Foo::Foo()\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar15() {
        check("class Fred\n"
              "{\n"
              "    int a;\n"
              "public:\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "Fred::~Fred()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar16() {
        check("struct Foo\n"
              "{\n"
              "    int a;\n"
              "    void set(int x) { a = x; }\n"
              "};\n"
              "class Bar\n"
              "{\n"
              "    Foo foo;\n"
              "public:\n"
              "    Bar()\n"
              "    {\n"
              "        foo.set(0);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct Foo\n"
              "{\n"
              "    int a;\n"
              "    void set(int x) { a = x; }\n"
              "};\n"
              "class Bar\n"
              "{\n"
              "    Foo foo;\n"
              "public:\n"
              "    Bar()\n"
              "    {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:10:5]: (warning) Member variable 'Bar::foo' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar17() {
        check("struct Foo\n"
              "{\n"
              "    int a;\n"
              "};\n"
              "class Bar\n"
              "{\n"
              "    Foo foo[10];\n"
              "public:\n"
              "    Bar()\n"
              "    {\n"
              "        foo[0].a = 0;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct Foo\n"
              "{\n"
              "    int a;\n"
              "};\n"
              "class Bar\n"
              "{\n"
              "    Foo foo[10];\n"
              "public:\n"
              "    Bar()\n"
              "    {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9:5]: (warning) Member variable 'Bar::foo' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar18() { // ticket #2465
        check("struct Altren\n"
              "{\n"
              "    explicit Altren(int _a = 0) : value(0) { }\n"
              "    int value;\n"
              "};\n"
              "class A\n"
              "{\n"
              "public:\n"
              "    A() { }\n"
              "private:\n"
              "    Altren value;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar19() { // ticket #2792
        check("class mystring\n"
              "{\n"
              "    char* m_str;\n"
              "    int m_len;\n"
              "public:\n"
              "    explicit mystring(const char* str)\n"
              "    {\n"
              "        m_len = strlen(str);\n"
              "        m_str = (char*) malloc(m_len+1);\n"
              "        memcpy(m_str, str, m_len+1);\n"
              "    }\n"
              "    mystring& operator=(const mystring& copy)\n"
              "    {\n"
              "        return (*this = copy.m_str);\n"
              "    }\n"
              "    ~mystring()\n"
              "    {\n"
              "        free(m_str);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar20() { // ticket #2867
        check("Object::MemFunc() {\n"
              "    class LocalClass {\n"
              "    public:\n"
              "        LocalClass() : dataLength_(0) {}\n"
              "        std::streamsize dataLength_;\n"
              "        double bitsInData_;\n"
              "    } obj;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:9]: (warning) Member variable 'LocalClass::bitsInData_' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("struct copy_protected;\n"
              "Object::MemFunc() {\n"
              "    class LocalClass : public copy_protected {\n"
              "    public:\n"
              "        LocalClass() : copy_protected(1), dataLength_(0) {}\n"
              "        std::streamsize dataLength_;\n"
              "        double bitsInData_;\n"
              "    } obj;\n"
              "};");
        ASSERT_EQUALS(
            "[test.cpp:5:9]: (warning) Member variable 'LocalClass::bitsInData_' is not initialized in the constructor. [uninitMemberVar]\n",
            errout_str());

        check("struct copy_protected;\n"
              "Object::MemFunc() {\n"
              "    class LocalClass : ::copy_protected {\n"
              "    public:\n"
              "        LocalClass() : copy_protected(1), dataLength_(0) {}\n"
              "        std::streamsize dataLength_;\n"
              "        double bitsInData_;\n"
              "    } obj;\n"
              "};");
        ASSERT_EQUALS(
            "[test.cpp:5:9]: (warning) Member variable 'LocalClass::bitsInData_' is not initialized in the constructor. [uninitMemberVar]\n",
            errout_str());
    }

    void uninitVar21() { // ticket #2947
        check("class Fred {\n"
              "private:\n"
              "    int a[23];\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred() {\n"
              "    a[x::y] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar22() { // ticket #3043
        check("class Fred {\n"
              "  public:\n"
              "    Fred & operator=(const Fred &);\n"
              "    virtual Fred & clone(const Fred & other);\n"
              "    int x;\n"
              "};\n"
              "Fred & Fred::operator=(const Fred & other) {\n"
              "    return clone(other);\n"
              "}\n"
              "Fred & Fred::clone(const Fred & other) {\n"
              "    x = 0;\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class Fred {\n"
              "  public:\n"
              "    Fred & operator=(const Fred &);\n"
              "    virtual Fred & clone(const Fred & other);\n"
              "    int x;\n"
              "};\n"
              "Fred & Fred::operator=(const Fred & other) {\n"
              "    return clone(other);\n"
              "}\n"
              "Fred & Fred::clone(const Fred & other) {\n"
              "    return *this;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:14]: (warning) Member variable 'Fred::x' is not assigned a value in 'Fred::operator='. [operatorEqVarError]\n", errout_str());
    }

    void uninitVar23() { // ticket #3702
        check("class Fred {\n"
              "    int x;\n"
              "public:\n"
              "    Fred(struct A a, struct B b);\n"
              "    Fred(C c, struct D d);\n"
              "    Fred(struct E e, F f);\n"
              "    Fred(struct G, struct H);\n"
              "    Fred(I, J);\n"
              "};\n"
              "Fred::Fred(A a, B b) { }\n"
              "Fred::Fred(struct C c, D d) { }\n"
              "Fred::Fred(E e, struct F f) { }\n"
              "Fred::Fred(G g, H h) { }\n"
              "Fred::Fred(struct I i, struct J j) { }");
        ASSERT_EQUALS("[test.cpp:10:7]: (warning) Member variable 'Fred::x' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:11:7]: (warning) Member variable 'Fred::x' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:12:7]: (warning) Member variable 'Fred::x' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:13:7]: (warning) Member variable 'Fred::x' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:14:7]: (warning) Member variable 'Fred::x' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar24() { // ticket #3190
        check("class Foo;\n"
              "class Bar;\n"
              "class Sub;\n"
              "class Foo { class Sub; };\n"
              "class Bar { class Sub; };\n"
              "class Bar::Sub {\n"
              "    int b;\n"
              "public:\n"
              "    Sub() { }\n"
              "    Sub(int);\n"
              "};\n"
              "Bar::Sub::Sub(int) { };\n"
              "class ::Foo::Sub {\n"
              "    int f;\n"
              "public:\n"
              "    ~Sub();\n"
              "    Sub();\n"
              "};\n"
              "::Foo::Sub::~Sub() { }\n"
              "::Foo::Sub::Sub() { }\n"
              "class Foo;\n"
              "class Bar;\n"
              "class Sub;\n", dinit(CheckOptions, $.inconclusive = true));

        ASSERT_EQUALS("[test.cpp:9:5]: (warning, inconclusive) Member variable 'Sub::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:12:11]: (warning) Member variable 'Sub::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:20:13]: (warning) Member variable 'Sub::f' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar25() { // ticket #4789
        check("struct A {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    A(int x = 0, int y = 0, int z = 0);\n"
              "};\n"
              "A::A(int x = 0, int y = 0, int z = 0) { }\n"
              "struct B {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    B(int x = 0, int y = 0, int z = 0);\n"
              "};\n"
              "B::B(int x, int y, int z) { }\n"
              "struct C {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    C(int, int, int);\n"
              "};\n"
              "C::C(int x = 0, int y = 0, int z = 0) { }\n"
              "struct D {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    D(int, int, int);\n"
              "};\n"
              "D::D(int x, int y, int z) { }\n"
              "struct E {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    E(int x, int y, int z);\n"
              "};\n"
              "E::E(int, int, int) { }\n"
              "struct F {\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "    F(int x = 0, int y = 0, int z = 0);\n"
              "};\n"
              "F::F(int, int, int) { }\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:7:4]: (warning) Member variable 'A::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:7:4]: (warning) Member variable 'A::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:7:4]: (warning) Member variable 'A::c' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:14:4]: (warning) Member variable 'B::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:14:4]: (warning) Member variable 'B::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:14:4]: (warning) Member variable 'B::c' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:21:4]: (warning) Member variable 'C::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:21:4]: (warning) Member variable 'C::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:21:4]: (warning) Member variable 'C::c' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:28:4]: (warning) Member variable 'D::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:28:4]: (warning) Member variable 'D::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:28:4]: (warning) Member variable 'D::c' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:35:4]: (warning) Member variable 'E::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:35:4]: (warning) Member variable 'E::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:35:4]: (warning) Member variable 'E::c' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:42:4]: (warning) Member variable 'F::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:42:4]: (warning) Member variable 'F::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:42:4]: (warning) Member variable 'F::c' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar26() {
        check("class A {\n"
              "    int * v;\n"
              "    int sz;\n"
              "public:\n"
              "    A(int s) {\n"
              "        v = new int [sz = s];\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar27() {
        check("class A {\n"
              "    double d;\n"
              "public:\n"
              "    A() {\n"
              "        rtl::math::setNan(&d);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
        check("class A {\n"
              "    double d;\n"
              "public:\n"
              "    A() {\n"
              "        ::rtl::math::setNan(&d);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar28() {
        check("class Fred {\n"
              "    int i;\n"
              "    float f;\n"
              "public:\n"
              "    Fred() {\n"
              "        foo(1);\n"
              "        foo(1.0f);\n"
              "    }\n"
              "    void foo(int a) { i = a; }\n"
              "    void foo(float a) { f = a; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar29() {
        check("class A {\n"
              "    int i;\n"
              "public:\n"
              "    A() { foo(); }\n"
              "    void foo() const { };\n"
              "    void foo() { i = 0; }\n"
              "};\n"
              "class B {\n"
              "    int i;\n"
              "public:\n"
              "    B() { foo(); }\n"
              "    void foo() { i = 0; }\n"
              "    void foo() const { }\n"
              "};\n"
              "class C {\n"
              "    int i;\n"
              "public:\n"
              "    C() { foo(); }\n"
              "    void foo() const { i = 0; }\n"
              "    void foo() { }\n"
              "};\n"
              "class D {\n"
              "    int i;\n"
              "public:\n"
              "    D() { foo(); }\n"
              "	void foo() { }\n"
              "	void foo() const { i = 0; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:18:5]: (warning) Member variable 'C::i' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:25:5]: (warning) Member variable 'D::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar30() { // ticket #6417
        check("namespace NS {\n"
              "    class MyClass {\n"
              "    public:\n"
              "        MyClass();\n"
              "        ~MyClass();\n"
              "    private:\n"
              "        bool SomeVar;\n"
              "    };\n"
              "}\n"
              "using namespace NS;\n"
              "MyClass::~MyClass() { }\n"
              "MyClass::MyClass() : SomeVar(false) { }");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar31() { // ticket #8271
        check("void bar();\n"
              "class MyClass {\n"
              "public:\n"
              "    MyClass();\n"
              "    void Restart();\n"
              "protected:\n"
              "    int m_retCode;\n"
              "};\n"
              "MyClass::MyClass() {\n"
              "    bar(),Restart();\n"
              "}\n"
              "void MyClass::Restart() {\n"
              "    m_retCode = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar32() { // ticket #8835
        check("class Foo {\n"
              "   friend class Bar;\n"
              "   int member;\n"
              "public:\n"
              "   Foo()\n"
              "   {\n"
              "      if (1) {}\n"
              "   }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:4]: (warning) Member variable 'Foo::member' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
        check("class Foo {\n"
              "   friend class Bar;\n"
              "   int member;\n"
              "public:\n"
              "   Foo()\n"
              "   {\n"
              "      while (1) {}\n"
              "   }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:4]: (warning) Member variable 'Foo::member' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
        check("class Foo {\n"
              "   friend class Bar;\n"
              "   int member;\n"
              "public:\n"
              "   Foo()\n"
              "   {\n"
              "      for (;;) {}\n"
              "   }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:4]: (warning) Member variable 'Foo::member' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar33() { // ticket #10295
        check("namespace app {\n"
              "    class B {\n"
              "    public:\n"
              "        B(void);\n"
              "        int x;\n"
              "    };\n"
              "};\n"
              "app::B::B(void){}");
        ASSERT_EQUALS("[test.cpp:8:9]: (warning) Member variable 'B::x' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar34() { // ticket #10841
        check("struct A { void f() {} };\n"
              "struct B {\n"
              "    B() { a->f(); }\n"
              "    A* a;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Member variable 'B::a' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar35() {
        check("struct S {\n" // #12908
              "    int a, b;\n"
              "    explicit S(int h = 0);\n"
              "    S(S&& s) : a(s.a), b(a.b) {\n"
              "        s.a = 0;\n"
              "    }\n"
              "};\n"
              "S::S(int h) : a(h), b(0) {}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVar36() {
        check("struct S {\n"
              "    unsigned int a : 16;\n"
              "    unsigned int   : 8;\n"
              "    unsigned int b : 8;\n"
              "    S() : a(0) {}\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:5:5]: (warning) Member variable 'S::b' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVar37() {
        const Settings s = settingsBuilder(settings).library("std.cfg").build();
        check("struct C {\n" // #13989
              "    C() = default;\n"
              "    std::list<int>::const_iterator it;\n"
              "};\n", s);
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Member variable 'C::it' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVarArray1() {
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() {}\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'John::name' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class John\n"
              "{\n"
              "public:\n"
              "    John() {John::name[0] = '\\0';}\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { strcpy(name, \"\"); }\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { }\n"
              "\n"
              "    double  operator[](const unsigned long i);\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A;\n"
              "class John\n"
              "{\n"
              "public:\n"
              "    John() { }\n"
              "    A a[5];\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A;\n"
              "class John\n"
              "{\n"
              "public:\n"
              "    John() { }\n"
              "    A (*a)[5];\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:5]: (warning) Member variable 'John::a' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVarArray2() {
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { *name = 0; }\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        // #5754
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() {*this->name = '\\0';}\n"
              "\n"
              "private:\n"
              "    char name[255];\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarArray3() {
        check("class John\n"
              "{\n"
              "private:\n"
              "    int a[100];\n"
              "    int b[100];\n"
              "\n"
              "public:\n"
              "    John()\n"
              "    {\n"
              "        memset(a,0,sizeof(a));\n"
              "        memset(b,0,sizeof(b));\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarArray4() {
        check("class John\n"
              "{\n"
              "private:\n"
              "    int a[100];\n"
              "    int b[100];\n"
              "\n"
              "public:\n"
              "    John()\n"
              "    {\n"
              "        if (snprintf(a,10,\"a\")) { }\n"
              "        if (snprintf(b,10,\"b\")) { }\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarArray5() {
        check("class Foo\n"
              "{\n"
              "private:\n"
              "    Bar bars[10];\n"
              "public:\n"
              "    Foo()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarArray6() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    Foo();\n"
              "    static const char STR[];\n"
              "};\n"
              "const char Foo::STR[] = \"abc\";\n"
              "Foo::Foo() { }");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarArray7() {
        check("class Foo\n"
              "{\n"
              "    int array[10];\n"
              "public:\n"
              "    Foo() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:5]: (warning) Member variable 'Foo::array' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class Foo\n"
              "{\n"
              "    int array[10];\n"
              "public:\n"
              "    Foo() { memset(array, 0, sizeof(array)); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Foo\n"
              "{\n"
              "    int array[10];\n"
              "public:\n"
              "    Foo() { ::memset(array, 0, sizeof(array)); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarArray8() {
        check("class Foo {\n"
              "    char a[10];\n"
              "public:\n"
              "    Foo() { ::ZeroMemory(a); }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarArray9() { // #6957
        check("class BaseGDL;\n"
              "struct IxExprListT {\n"
              "private:\n"
              "    BaseGDL* eArr[3];\n"
              "public:\n"
              "    IxExprListT() {}\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:5]: (warning) Member variable 'IxExprListT::eArr' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
        check("struct sRAIUnitDefBL {\n"
              "  sRAIUnitDefBL();\n"
              "  ~sRAIUnitDefBL();\n"
              "};\n"
              "struct sRAIUnitDef {\n"
              "  sRAIUnitDef() {}\n"
              "  sRAIUnitDefBL *List[35];\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:3]: (warning) Member variable 'sRAIUnitDef::List' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVarArray10() { // #11650
        const Settings s = settingsBuilder(settings).library("std.cfg").build();
        check("struct T { int j; };\n"
              "struct U { int k{}; };\n"
              "struct S {\n"
              "    std::array<int, 2> a;\n"
              "    std::array<T, 2> b;\n"
              "    std::array<std::size_t, 2> c;\n"
              "    std::array<clock_t, 2> d;\n"
              "    std::array<std::string, 2> e;\n"
              "    std::array<U, 2> f;\n"
              "S() {}\n"
              "};\n", s);

        ASSERT_EQUALS("[test.cpp:10:1]: (warning) Member variable 'S::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:10:1]: (warning) Member variable 'S::b' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:10:1]: (warning) Member variable 'S::c' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:10:1]: (warning) Member variable 'S::d' is not initialized in the constructor. [uninitMemberVar]\n",
                      errout_str());
    }

    void uninitVarArray11() {
        check("class C {\n" // #12150
              "private:\n"
              "    int buf[10];\n"
              "public:\n"
              "    C() {\n"
              "        for (int& i : buf)\n"
              "            i = 0;\n"
              "    }\n"
              "};\n");

        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarArray2D() {
        check("class John\n"
              "{\n"
              "public:\n"
              "    John() { a[0][0] = 0; }\n"
              "\n"
              "private:\n"
              "    char a[2][2];\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarArray3D() {
        check("class John\n"
              "{\n"
              "private:\n"
              "    char a[2][2][2];\n"
              "public:\n"
              "    John() { a[0][0][0] = 0; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarCpp11Init1() {
        check("class Foo {\n"
              "    std::vector<std::string> bar;\n"
              "public:\n"
              "    Foo()\n"
              "        : bar({\"a\", \"b\"})\n"
              "    {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarCpp11Init2() {
        check("class Fred {\n"
              "    struct Foo {\n"
              "        int a;\n"
              "        bool b;\n"
              "    };\n"
              "    Foo f;\n"
              "    float g;\n"
              "public:\n"
              "    Fred() : f{0, true} { }\n"
              "    float get() const;\n"
              "};\n"
              "float Fred::get() const { return g; }");
        ASSERT_EQUALS("[test.cpp:9:5]: (warning) Member variable 'Fred::g' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVarStruct1() { // ticket #2172
        check("class A\n"
              "{\n"
              "private:\n"
              "    struct B {\n"
              "        std::string str1;\n"
              "        std::string str2;\n"
              "    }\n"
              "    struct B b;\n"
              "public:\n"
              "    A() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    struct B {\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    }\n"
              "    struct B b;\n"
              "public:\n"
              "    A() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:10:5]: (warning) Member variable 'A::b' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    struct B {\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "        B() : str1(NULL), str2(NULL) { }\n"
              "    }\n"
              "    struct B b;\n"
              "public:\n"
              "    A() {\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarStruct2() { // ticket #838
        check("struct POINT\n"
              "{\n"
              "    int x;\n"
              "    int y;\n"
              "};\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    POINT p;\n"
              "public:\n"
              "    Fred()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:11:5]: (warning) Member variable 'Fred::p' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("struct POINT\n"
              "{\n"
              "    int x;\n"
              "    int y;\n"
              "    POINT();\n"
              "};\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    POINT p;\n"
              "public:\n"
              "    Fred()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct POINT\n"
              "{\n"
              "    int x;\n"
              "    int y;\n"
              "    POINT() :x(0), y(0) { }\n"
              "};\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    POINT p;\n"
              "public:\n"
              "    Fred()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        // non static data-member initialization
        check("struct POINT\n"
              "{\n"
              "    int x=0;\n"
              "    int y=0;\n"
              "};\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    POINT p;\n"
              "public:\n"
              "    Fred()\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarUnion1() {
        check("class Fred\n" // ticket #3196
              "{\n"
              "private:\n"
              "    union { int a; int b; };\n"
              "public:\n"
              "    Fred()\n"
              "    { a = 0; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred {\n"
              "private:\n"
              "    union { int a{}; int b; };\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarUnion2() {
        check("union Data { int id; int *ptr; };\n"
              "\n"
              "class Fred {\n"
              "private:\n"
              "    int data_type;\n"
              "    Data data;\n"
              "public:\n"
              "    Fred() : data_type(0)\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:8:5]: (warning) Member variable 'Fred::data' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitMissingFuncDef() {
        // Unknown member function
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    void Init();"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        // Unknown non-member function (friend class)
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    friend ABC;\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        // Unknown non-member function (is Init a virtual function?)
        check("class Fred : private ABC\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // Unknown non-member function
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // Unknown non-member function
        check("class ABC { };\n"
              "class Fred : private ABC\n"
              "{\n"
              "public:\n"
              "    Fred() { Init(); }\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // Unknown member functions and unknown static functions
        check("class ABC {\n"
              "  static void static_base_func();\n"
              "  void const_base_func() const;\n"
              "};\n"
              "class Fred : private ABC {\n"
              "public:\n"
              "    Fred() {\n"
              "        const_func();\n"
              "        static_func();\n"
              "        const_base_func();\n"
              "        ABC::static_base_func();\n"
              "    }\n"
              "    void const_func() const;\n"
              "    static void static_f();\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // Unknown overloaded member functions
        check("class Fred : private ABC {\n"
              "public:\n"
              "    Fred() {\n"
              "        func();\n"
              "    }\n"
              "    void func() const;\n"
              "    void func();\n"
              "private:\n"
              "    int i;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarEnum1() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    enum abc {a,b,c};\n"
              "    Fred() {}\n"
              "private:\n"
              "    unsigned int i;\n"
              "};");

        ASSERT_EQUALS("[test.cpp:5:5]: (warning) Member variable 'Fred::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitVarEnum2() { // ticket #8146
        check("enum E { E1 };\n"
              "struct X { E e = E1; };\n"
              "struct Y {\n"
              "    Y() {}\n"
              "    X x;\n"
              "};");

        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarStream() {
        check("class Foo\n"
              "{\n"
              "private:\n"
              "    int foo;\n"
              "public:\n"
              "    explicit Foo(std::istream &in)\n"
              "    {\n"
              "        if(!(in >> foo))\n"
              "            throw 0;\n"
              "    }\n"
              "};");

        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarTypedef() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    typedef int * pointer;\n"
              "    Foo() : a(0) {}\n"
              "    pointer a;\n"
              "};");

        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarMemset() {
        check("class Foo\n"
              "{\n"
              "public:\n"
              "    int * pointer;\n"
              "    Foo() { memset(this, 0, sizeof(*this)); }\n"
              "};");

        ASSERT_EQUALS("", errout_str());

        check("class Foo\n"
              "{\n"
              "public:\n"
              "    int * pointer;\n"
              "    Foo() { ::memset(this, 0, sizeof(*this)); }\n"
              "};");

        ASSERT_EQUALS("", errout_str());

        // Ticket #7068
        check("struct Foo {\n"
              "    int * p;\n"
              "    char c;\n"
              "    Foo() { memset(p, 0, sizeof(int)); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Foo::c' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
        check("struct Foo {\n"
              "    int i;\n"
              "    char c;\n"
              "    Foo() { memset(&i, 0, sizeof(int)); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Foo::c' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
        check("struct Foo { int f; };\n"
              "struct Bar { int b; };\n"
              "struct FooBar {\n"
              "  FooBar() {\n"
              "     memset(&foo, 0, sizeof(foo));\n"
              "  }\n"
              "  Foo foo;\n"
              "  Bar bar;\n"
              "};\n"
              "int main() {\n"
              "  FooBar foobar;\n"
              "  return foobar.foo.f + foobar.bar.b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:3]: (warning) Member variable 'FooBar::bar' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
        check("struct Foo { int f; };\n"
              "struct Bar { int b; };\n"
              "struct FooBar {\n"
              "  FooBar() {\n"
              "     memset(&this->foo, 0, sizeof(this->foo));\n"
              "  }\n"
              "  Foo foo;\n"
              "  Bar bar;\n"
              "};\n"
              "int main() {\n"
              "  FooBar foobar;\n"
              "  return foobar.foo.f + foobar.bar.b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:3]: (warning) Member variable 'FooBar::bar' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // #7755
        check("struct A {\n"
              "  A() {\n"
              "    memset(this->data, 0, 42);\n"
              "  }\n"
              "  char data[42];\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void privateCtor1() {
        {
            const Settings s = settingsBuilder(settings).cpp(Standards::CPP03).build();
            check("class Foo {\n"
                  "    int foo;\n"
                  "    Foo() { }\n"
                  "};", s);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const Settings s = settingsBuilder(settings).cpp(Standards::CPP11).build();
            check("class Foo {\n"
                  "    int foo;\n"
                  "    Foo() { }\n"
                  "};", s);
            ASSERT_EQUALS("[test.cpp:3:5]: (warning) Member variable 'Foo::foo' is not initialized in the constructor. [uninitMemberVarPrivate]\n", errout_str());
        }
    }

    void privateCtor2() {
        check("class Foo\n"
              "{\n"
              "private:\n"
              "    int foo;\n"
              "    Foo() { }\n"
              "public:\n"
              "    explicit Foo(int _i) { }\n"
              "};");

        ASSERT_EQUALS("[test.cpp:7:14]: (warning) Member variable 'Foo::foo' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }


    void function() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    A();\n"
              "    int* f(int*);\n"
              "};\n"
              "\n"
              "A::A()\n"
              "{\n"
              "}\n"
              "\n"
              "int* A::f(int* p)\n"
              "{\n"
              "    return p;\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }


    // Borland C++: No FP for published pointers - they are automatically initialized
    void uninitVarPublished() {
        check("class Fred\n"
              "{\n"
              "__published:\n"
              "    int *i;\n"
              "public:\n"
              "    Fred() { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarInheritClassInit() {
        // TODO: test should probably not pass without library
        const Settings s = settingsBuilder() /*.library("vcl.cfg")*/.build();

        check("class Fred: public TObject\n"
              "{\n"
              "public:\n"
              "    Fred() { }\n"
              "private:\n"
              "    int x;\n"
              "};", s);
        ASSERT_EQUALS("", errout_str());
    }

    void uninitOperator() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { }\n"
              "    int *operator [] (int index) { return 0; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitFunction1() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { init(*this); }\n"
              "\n"
              "    static void init(Fred &f)\n"
              "    { f.d = 0; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { init(*this); }\n"
              "\n"
              "    static void init(Fred &f)\n"
              "    { }\n"
              "\n"
              "    double d;\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Fred::d' is not initialized in the constructor. [uninitMemberVar]\n", "", errout_str());
    }

    void uninitFunction2() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { if (!init(*this)); }\n"
              "\n"
              "    static bool init(Fred &f)\n"
              "    { f.d = 0; return true; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { if (!init(*this)); }\n"
              "\n"
              "    static bool init(Fred &f)\n"
              "    { return true; }\n"
              "\n"
              "    double d;\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Fred::d' is not initialized in the constructor. [uninitMemberVar]\n", "",  errout_str());
    }

    void uninitFunction3() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { if (!init()); }\n"
              "\n"
              "    bool init()\n"
              "    { d = 0; return true; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { if (!init()); }\n"
              "\n"
              "    bool init()\n"
              "    { return true; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Fred::d' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitFunction4() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { init(this); }\n"
              "\n"
              "    init(Fred *f)\n"
              "    { f.d = 0; }\n"
              "\n"
              "    double d;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { init(this); }\n"
              "\n"
              "    init(Fred *f)\n"
              "    { }\n"
              "\n"
              "    double d;\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'Fred::d' is not initialized in the constructor. [uninitMemberVar]\n", "", errout_str());
    }

    void uninitFunction5() { // #4072 - FP about struct that is initialized in function
        check("struct Structure {\n"
              "    int C;\n"
              "};\n"
              "\n"
              "class A {\n"
              "    Structure B;\n"
              "public:\n"
              "    A() { Init( B ); };\n"
              "    void Init( Structure& S ) { S.C = 0; };\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct Structure {\n"
              "    int C;\n"
              "};\n"
              "\n"
              "class A {\n"
              "    Structure B;\n"
              "public:\n"
              "    A() { Init( B ); };\n"
              "    void Init(const Structure& S) { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:8:5]: (warning) Member variable 'A::B' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());
    }

    void uninitSameClassName() {
        check("class B\n"
              "{\n"
              "public:\n"
              "    B();\n"
              "    int j;\n"
              "};\n"
              "\n"
              "class A\n"
              "{\n"
              "    class B\n"
              "    {\n"
              "    public:\n"
              "        B();\n"
              "        int i;\n"
              "    };\n"
              "};\n"
              "\n"
              "A::B::B()\n"
              "{\n"
              "    i = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class B\n"
              "{\n"
              "public:\n"
              "    B();\n"
              "    int j;\n"
              "};\n"
              "\n"
              "class A\n"
              "{\n"
              "    class B\n"
              "    {\n"
              "    public:\n"
              "        B();\n"
              "        int i;\n"
              "    };\n"
              "};\n"
              "\n"
              "B::B()\n"
              "{\n"
              "}\n"
              "\n"
              "A::B::B()\n"
              "{\n"
              "}");
        ASSERT_EQUALS("[test.cpp:18:4]: (warning) Member variable 'B::j' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:22:7]: (warning) Member variable 'B::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        // Ticket #1700
        check("namespace n1\n"
              "{\n"
              "class Foo {"
              "public:\n"
              "    Foo() : i(0) { }\n"
              "private:\n"
              "    int i;\n"
              "};\n"
              "}\n"
              "\n"
              "namespace n2\n"
              "{\n"
              "class Foo {"
              "public:\n"
              "    Foo() { }\n"
              "};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("namespace n1\n"
              "{\n"
              "class Foo {\n"
              "public:\n"
              "    Foo();\n"
              "private:\n"
              "    int i;\n"
              "};\n"
              "}\n"
              "\n"
              "n1::Foo::Foo()\n"
              "{ }\n"
              "\n"
              "namespace n2\n"
              "{\n"
              "class Foo {\n"
              "public:\n"
              "    Foo() { }\n"
              "};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11:10]: (warning) Member variable 'Foo::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("namespace n1\n"
              "{\n"
              "class Foo {"
              "public:\n"
              "    Foo();\n"
              "private:\n"
              "    int i;\n"
              "};\n"
              "}\n"
              "\n"
              "n1::Foo::Foo() : i(0)\n"
              "{ }\n"
              "\n"
              "namespace n2\n"
              "{\n"
              "class Foo {"
              "public:\n"
              "    Foo() { }\n"
              "};\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitFunctionOverload() {
        // Ticket #1783 - overloaded "init" functions
        check("class A\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "\n"
              "public:\n"
              "    A()\n"
              "    {\n"
              "        init();\n"
              "    }\n"
              "\n"
              "    void init() { init(0); }\n"
              "\n"
              "    void init(int value)\n"
              "    { i = value; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    int i;\n"
              "\n"
              "public:\n"
              "    A()\n"
              "    {\n"
              "        init();\n"
              "    }\n"
              "\n"
              "    void init() { init(0); }\n"
              "\n"
              "    void init(int value)\n"
              "    { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7:5]: (warning) Member variable 'A::i' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class bar {\n" // #9887
              "    int length;\n"
              "    bar() { length = 0; }\n"
              "};\n"
              "class foo {\n"
              "    int x;\n"
              "    foo() { Set(bar()); }\n"
              "    void Set(int num) { x = 1; }\n"
              "    void Set(bar num) { x = num.length; }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitVarOperatorEqual() { // ticket #2415
        check("struct A {\n"
              "    int a;\n"
              "    A() { a=0; }\n"
              "    A(A const &a) { operator=(a); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int a;\n"
              "    A() { a=0; }\n"
              "    A(A const &a) { operator=(a); }\n"
              "    A & operator = (const A & rhs) {\n"
              "        a = rhs.a;\n"
              "        return *this;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int a;\n"
              "    A() { a=0; }\n"
              "    A(A const &a) { operator=(a); }\n"
              "    A & operator = (const A & rhs) {\n"
              "        return *this;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'A::a' is not initialized in the copy constructor. [uninitMemberVar]\n"
                      "[test.cpp:5:9]: (warning) Member variable 'A::a' is not assigned a value in 'A::operator='. [operatorEqVarError]\n", errout_str());
    }

    void uninitVarPointer() { // #3801
        check("struct A {\n"
              "    int a;\n"
              "};\n"
              "struct B {\n"
              "    A* a;\n"
              "    B() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:5]: (warning) Member variable 'B::a' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("struct A;\n"
              "struct B {\n"
              "    A* a;\n"
              "    B() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'B::a' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("struct A;\n"
              "struct B {\n"
              "    const A* a;\n"
              "    B() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'B::a' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("struct A;\n"
              "struct B {\n"
              "    A* const a;\n"
              "    B() { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'B::a' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("class Test {\n" // #8498
              "public:\n"
              "    Test() {}\n"
              "    std::map<int, int>* pMap = nullptr;\n"
              "    std::string* pStr = nullptr;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("template <typename U>\n"
              "class C1 {}; \n"
              "template <typename U, typename V>\n"
              "class C2 {};\n"
              "namespace A {\n"
              "    template <typename U>\n"
              "    class D1 {};\n"
              "    template <typename U, typename V>\n"
              "    class D2 {};\n"
              "}\n"
              "class Test {\n"
              "public:\n"
              "    Test() {}\n"
              "    C1<int>* c1 = nullptr;\n"
              "    C2<int, int >* c2 = nullptr;\n"
              "    A::D1<int>* d1 = nullptr;\n"
              "    A::D2<int, int >* d2 = nullptr;\n"
              "    std::map<int, int>* pMap = nullptr;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #6294
              "    S() : r(new int) {\n"
              "        *p = 0;\n"
              "        *(q) = 1;\n"
              "        *r = 2;\n"
              "    }\n"
              "    int *p, *q, *r;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Member variable 'S::p' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:2:5]: (warning) Member variable 'S::q' is not initialized in the constructor. [uninitMemberVar]\n",
                      errout_str());
    }

    void uninitConstVar() {
        check("struct A;\n"
              "struct B {\n"
              "    A* const a;\n"
              "    B() { }\n"
              "    B(B& b) { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Member variable 'B::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:5:5]: (warning) Member variable 'B::a' is not initialized in the copy constructor. [uninitMemberVar]\n", errout_str());

        check("struct A;\n"
              "struct B {\n"
              "    A* const a;\n"
              "    B& operator=(const B& r) { }\n"
              "};");
        ASSERT_EQUALS("", errout_str()); // #3804

        check("struct B {\n"
              "    const int a;\n"
              "    B() { }\n"
              "    B(B& b) { }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning) Member variable 'B::a' is not initialized in the constructor. [uninitMemberVar]\n"
                      "[test.cpp:4:5]: (warning) Member variable 'B::a' is not initialized in the copy constructor. [uninitMemberVar]\n", errout_str());

        check("struct B {\n"
              "    const int a;\n"
              "    B& operator=(const B& r) { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    // Ticket #5641 "Regression. Crash for 'C() _STLP_NOTHROW {}'"
    void constructors_crash1() {
        check("class C {\n"
              "public:\n"
              "  C() _STLP_NOTHROW {}\n"
              "  C(const C&) _STLP_NOTHROW {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void classWithOperatorInName() { // ticket #2827
        check("class operatorX {\n"
              "  int mValue;\n"
              "public:\n"
              "  operatorX() : mValue(0) {}\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void templateConstructor() { // ticket #7942
        check("template <class T> struct Container {\n"
              "  Container();\n"
              "  T* mElements;\n"
              "};\n"
              "template <class T> Container<T>::Container() : mElements(nullptr) {}\n"
              "Container<int> intContainer;");
        ASSERT_EQUALS("", errout_str());
    }

    void typedefArray() { // ticket #5766
        check("typedef float    rvec[3];\n"
              "class SelectionPosition {\n"
              "public:\n"
              "    SelectionPosition() {}\n"
              "    const rvec &x() const;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitAssignmentWithOperator() {
        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = false;\n"
              "        b = b && SetValue();\n"
              "    }\n"
              "    bool SetValue() {\n"
              "        x = 1;\n"
              "        return true;\n"
              "    }\n"
              "};", dinit(CheckOptions, $.inconclusive = true));
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Member variable 'C::x' is not initialized in the constructor.\n",
                           "[test.cpp:3:5]: (warning) Member variable 'C::x' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = false;\n"
              "        b = true || SetValue();\n"
              "    }\n"
              "    bool SetValue() {\n"
              "        x = 1;\n"
              "        return true;\n"
              "    }\n"
              "};", dinit(CheckOptions, $.inconclusive = true));
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) Member variable 'C::x' is not initialized in the constructor.\n",
                           "[test.cpp:3:5]: (warning) Member variable 'C::x' is not initialized in the constructor. [uninitMemberVar]\n", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = true;\n"
              "        b = b & SetValue();\n"
              "    }\n"
              "    bool SetValue() {\n"
              "        x = 1;\n"
              "        return true;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = false;\n"
              "        b = true | SetValue();\n"
              "    }\n"
              "    bool SetValue() {\n"
              "        x = 1;\n"
              "        return true;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i = i * SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i = i / SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i = i % SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i = i + SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i = i - SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i = i << SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i = i >> SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i = i ^ SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitCompoundAssignment() {
        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = true;\n"
              "        b &= SetValue();\n"
              "    }\n"
              "    bool SetValue() {\n"
              "        x = 1;\n"
              "        return true;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = false;\n"
              "        b |= SetValue();\n"
              "    }\n"
              "    bool SetValue() {\n"
              "        x = 1;\n"
              "        return true;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i *= SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i /= SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i %= SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i += SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i -= SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i <<= SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i >>= SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        int i = 0;\n"
              "        i ^= SetValue();\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitComparisonAssignment() {
        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = true;\n"
              "        b = (true == SetValue());\n"
              "    }\n"
              "    bool SetValue() {\n"
              "        x = 1;\n"
              "        return true;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = false;\n"
              "        b |= (true != SetValue());\n"
              "    }\n"
              "    bool SetValue() {\n"
              "        x = 1;\n"
              "        return true;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = (0 < SetValue());\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = (0 <= SetValue());\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = (0 > SetValue());\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n"
              "    int x;\n"
              "    C() {\n"
              "        bool b = (0 >= SetValue());\n"
              "    }\n"
              "    int SetValue() { return x = 1; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void uninitTemplate1() {
        check("template <class A, class T> class C;\n"
              "template <class A>\n"
              "class C<A, void> {\n"
              "  public:\n"
              "    C() : b(0) { }\n"
              "    C(A* a) : b(a) { }\n"
              "  private:\n"
              "    A* b;\n"
              "};\n"
              "template <class A, class T>\n"
              "class C {\n"
              "  private:\n"
              "    A* b;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("template<class T> class A{};\n"
              "template<class T1, class T2> class B{};\n"
              "template<class T1, class T2>\n"
              "class A<B<T1, T2>> {\n"
              "  public:\n"
              "    A();\n"
              "    bool m_value;\n"
              "};\n"
              "template<class T1, class T2>\n"
              "A<B<T1, T2>>::A() : m_value(false) {}");
        ASSERT_EQUALS("", errout_str());

        check("template <typename T> struct S;\n" // #11177
              "template <> struct S<void> final {\n"
              "    explicit S(int& i);\n"
              "    int& m;\n"
              "};\n"
              "S<void>::S(int& i) : m(i) {}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void unknownTemplateType() {
        check("template <typename T> class A {\n"
              "private:\n"
              "    T m;\n"
              "public:\n"
              "    A& operator=() { return *this; }\n"
              "};\n"
              "A<decltype(SOMETHING)> a;");
        ASSERT_EQUALS("", errout_str());
    }

};

REGISTER_TEST(TestConstructors)
