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

#include "checkautovariables.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>

class TestAutoVariables : public TestFixture {
public:
    TestAutoVariables() : TestFixture("TestAutoVariables") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::warning).severity(Severity::style).library("std.cfg").library("qt.cfg").build();
    const Settings settings_i = settingsBuilder(settings).certainty(Certainty::inconclusive).build();

    struct CheckOptions
    {
        bool inconclusive = true;
        bool cpp = true;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings &settings1 = options.inconclusive ? settings_i : settings;

        // Tokenize..
        SimpleTokenizer tokenizer(settings1, *this, options.cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        runChecks<CheckAutoVariables>(tokenizer, this);
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(testautovar1);
        TEST_CASE(testautovar2);
        TEST_CASE(testautovar3); // ticket #2925
        TEST_CASE(testautovar4); // ticket #2928
        TEST_CASE(testautovar5); // ticket #2926
        TEST_CASE(testautovar6); // ticket #2931
        TEST_CASE(testautovar7); // ticket #3066
        TEST_CASE(testautovar8);
        TEST_CASE(testautovar9);
        TEST_CASE(testautovar10); // ticket #2930 - void f(char *p) { p = '\0'; }
        TEST_CASE(testautovar11); // ticket #4641 - fp, assign local struct member address to function parameter
        TEST_CASE(testautovar12); // ticket #5024 - crash
        TEST_CASE(testautovar13); // ticket #5537 - crash
        TEST_CASE(testautovar14); // ticket #4776 - assignment of function parameter, goto
        TEST_CASE(testautovar15); // ticket #6538
        TEST_CASE(testautovar16); // ticket #8114
        TEST_CASE(testautovar17);
        TEST_CASE(testautovar18);
        TEST_CASE(testautovar_array1);
        TEST_CASE(testautovar_array2);
        TEST_CASE(testautovar_array3);
        TEST_CASE(testautovar_normal); // "normal" token list that does not remove casts etc
        TEST_CASE(testautovar_ptrptr); // ticket #6956
        TEST_CASE(testautovar_return1);
        TEST_CASE(testautovar_return2);
        TEST_CASE(testautovar_return3);
        TEST_CASE(testautovar_return4);
        TEST_CASE(testautovar_return5);
        TEST_CASE(testautovar_return6);
        TEST_CASE(testautovar_extern);
        TEST_CASE(testautovar_reassigned);
        TEST_CASE(testinvaliddealloc);
        TEST_CASE(testinvaliddealloc_input); // Ticket #10600
        TEST_CASE(testinvaliddealloc_string);
        TEST_CASE(testinvaliddealloc_C);
        TEST_CASE(testassign1);  // Ticket #1819
        TEST_CASE(testassign2);  // Ticket #2765

        TEST_CASE(assignAddressOfLocalArrayToGlobalPointer);
        TEST_CASE(assignAddressOfLocalVariableToGlobalPointer);
        TEST_CASE(assignAddressOfLocalVariableToMemberVariable);

        TEST_CASE(returnLocalVariable1);
        TEST_CASE(returnLocalVariable2);
        TEST_CASE(returnLocalVariable3); // &x[0]
        TEST_CASE(returnLocalVariable4); // x+y
        TEST_CASE(returnLocalVariable5); // cast
        TEST_CASE(returnLocalVariable6); // valueflow

        // return reference..
        TEST_CASE(returnReference1);
        TEST_CASE(returnReference2);
        TEST_CASE(returnReference3);
        TEST_CASE(returnReference4);
        TEST_CASE(returnReference5);
        TEST_CASE(returnReference6);
        TEST_CASE(returnReference7);
        TEST_CASE(returnReference8);
        TEST_CASE(returnReference9);
        TEST_CASE(returnReference10);
        TEST_CASE(returnReference11);
        TEST_CASE(returnReference12);
        TEST_CASE(returnReference13);
        TEST_CASE(returnReference14);
        TEST_CASE(returnReference15); // #9432
        TEST_CASE(returnReference16); // #9433
        TEST_CASE(returnReference16); // #9433
        TEST_CASE(returnReference17); // #9461
        TEST_CASE(returnReference18); // #9482
        TEST_CASE(returnReference19); // #9597
        TEST_CASE(returnReference20); // #9536
        TEST_CASE(returnReference21); // #9530
        TEST_CASE(returnReference22);
        TEST_CASE(returnReference23);
        TEST_CASE(returnReference24); // #10098
        TEST_CASE(returnReference25); // #10983
        TEST_CASE(returnReference26);
        TEST_CASE(returnReference27);
        TEST_CASE(returnReference28);
        TEST_CASE(returnReference29);
        TEST_CASE(returnReferenceFunction);
        TEST_CASE(returnReferenceContainer);
        TEST_CASE(returnReferenceLiteral);
        TEST_CASE(returnReferenceCalculation);
        TEST_CASE(returnReferenceLambda);
        TEST_CASE(returnReferenceInnerScope);
        TEST_CASE(returnReferenceRecursive);
        TEST_CASE(extendedLifetime);

        TEST_CASE(danglingReference);
        TEST_CASE(danglingTempReference);

        // global namespace
        TEST_CASE(testglobalnamespace);

        TEST_CASE(returnParameterAddress);

        TEST_CASE(testconstructor); // ticket #5478 - crash

        TEST_CASE(variableIsUsedInScope); // ticket #5599 crash in variableIsUsedInScope()

        TEST_CASE(danglingLifetimeLambda);
        TEST_CASE(danglingLifetimeContainer);
        TEST_CASE(danglingLifetimeContainerView);
        TEST_CASE(danglingLifetimeUniquePtr);
        TEST_CASE(danglingLifetime);
        TEST_CASE(danglingLifetimeFunction);
        TEST_CASE(danglingLifetimeUserConstructor);
        TEST_CASE(danglingLifetimeAggregateConstructor);
        TEST_CASE(danglingLifetimeInitList);
        TEST_CASE(danglingLifetimeImplicitConversion);
        TEST_CASE(danglingTemporaryLifetime);
        TEST_CASE(danglingLifetimeBorrowedMembers);
        TEST_CASE(danglingLifetimeClassMemberFunctions);
        TEST_CASE(invalidLifetime);
        TEST_CASE(deadPointer);
        TEST_CASE(splitNamespaceAuto); // crash #10473
        TEST_CASE(incompleteTypeArray);
    }



    void testautovar1() {
        check("void func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    *res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("void func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it? [uselessAssignmentPtrArg]\n", errout_str());

        check("void func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    foo.res = &num;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar2() {
        check("class Fred {\n"
              "    void func1(int **res);\n"
              "}\n"
              "void Fred::func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    *res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("class Fred {\n"
              "    void func1(int **res);\n"
              "}\n"
              "void Fred::func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:5]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it? [uselessAssignmentPtrArg]\n", errout_str());

        check("class Fred {\n"
              "    void func1(int **res);\n"
              "}\n"
              "void Fred::func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    foo.res = &num;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar3() { // ticket #2925
        check("void foo(int **p)\n"
              "{\n"
              "    int x[100];\n"
              "    *p = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());
    }

    void testautovar4() { // ticket #2928
        check("void foo(int **p)\n"
              "{\n"
              "    static int x[100];\n"
              "    *p = x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar5() { // ticket #2926
        check("void foo(struct AB *ab)\n"
              "{\n"
              "    char a;\n"
              "    ab->a = &a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error, inconclusive) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());
    }

    void testautovar6() { // ticket #2931
        check("void foo(struct X *x)\n"
              "{\n"
              "    char a[10];\n"
              "    x->str = a;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("void foo(struct X *x)\n"
              "{\n"
              "    char a[10];\n"
              "    x->str = a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error, inconclusive) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());
    }

    void testautovar7() { // ticket #3066
        check("struct txt_scrollpane_s * TXT_NewScrollPane(struct txt_widget_s * target)\n"
              "{\n"
              "    struct txt_scrollpane_s * scrollpane;\n"
              "    target->parent = &scrollpane->widget;\n"
              "    return scrollpane;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar8() {
        check("void foo(int*& p) {\n"
              "    int i = 0;\n"
              "    p = &i;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("void foo(std::string& s) {\n"
              "    s = foo;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar9() {
        check("struct FN {int i;};\n"
              "struct FP {FN* f};\n"
              "void foo(int*& p, FN* p_fp) {\n"
              "    FN fn;\n"
              "    FP fp;\n"
              "    p = &fn.i;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("struct FN {int i;};\n"
              "struct FP {FN* f};\n"
              "void foo(int*& p, FN* p_fp) {\n"
              "    FN fn;\n"
              "    FP fp;\n"
              "    p = &p_fp->i;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("struct FN {int i;};\n"
              "struct FP {FN* f};\n"
              "void foo(int*& p, FN* p_fp) {\n"
              "    FN fn;\n"
              "    FP fp;\n"
              "    p = &fp.f->i;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar10() { // #2930 - assignment of function parameter
        check("void foo(char* p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it? [uselessAssignmentPtrArg]\n", errout_str());

        check("void foo(int b) {\n"
              "    b = foo(b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Assignment of function parameter has no effect outside the function. [uselessAssignmentArg]\n", errout_str());

        check("void foo(int b) {\n"
              "    b += 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Assignment of function parameter has no effect outside the function. [uselessAssignmentArg]\n", errout_str());

        check("void foo(std::string s) {\n"
              "    s = foo(b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Assignment of function parameter has no effect outside the function. [uselessAssignmentArg]\n", errout_str());

        check("void foo(char* p) {\n" // don't warn for self assignment, there is another warning for this
              "  p = p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char* p) {\n"
              "    if (!p) p = buf;\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char* p) {\n"
              "    if (!p) p = buf;\n"
              "    do_something(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char* p) {\n"
              "    while (!p) p = buf;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char* p) {\n"
              "    p = 0;\n"
              "    asm(\"somecmd\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(Foo* p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it? [uselessAssignmentPtrArg]\n", errout_str());

        check("class Foo {};\n"
              "void foo(Foo p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Assignment of function parameter has no effect outside the function. [uselessAssignmentArg]\n", errout_str());

        check("void foo(Foo p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int& p) {\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("double foo(double d) {\n" // #5005
              "    int i = d;\n"
              "    d = i;\n"
              "    return d;"
              "}",dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());

        check("void foo(int* ptr) {\n" // #4793
              "    ptr++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it? [uselessAssignmentPtrArg]\n", errout_str());

        check("void foo(int* ptr) {\n" // #3177
              "    --ptr;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (warning) Assignment of function parameter has no effect outside the function. Did you forget dereferencing it? [uselessAssignmentPtrArg]\n", errout_str());

        check("void foo(struct S* const x) {\n" // #7839
              "    ++x->n;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar11() { // #4641 - fp, assign local struct member address to function parameter
        check("struct A {\n"
              "    char (*data)[10];\n"
              "};\n"
              "void foo(char** p) {\n"
              "    struct A a = bar();\n"
              "    *p = &(*a.data)[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    char data[10];\n"
              "};\n"
              "void foo(char** p) {\n"
              "    struct A a = bar();\n"
              "    *p = &a.data[0];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("void f(char **out) {\n"
              "  struct S *p = glob;\n"
              "  *out = &p->data;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #4998
        check("void f(s8**out) {\n"
              "  s8 *p;\n"  // <- p is pointer => no error
              "  *out = &p[1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(s8**out) {\n"
              "  s8 p[10];\n"  // <- p is array => error
              "  *out = &p[1];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());
    }

    void testautovar12() { // Ticket #5024, #5050 - Crash on invalid input
        ASSERT_THROW_INTERNAL(check("void f(int* a) { a = }"), SYNTAX);
        check("struct custom_type { custom_type(int) {} };\n"
              "void func(int) {}\n"
              "int var;\n"
              "void init() { func(var); }\n"
              "UNKNOWN_MACRO_EXPANDING_TO_SIGNATURE { custom_type a(var); }");
    }

    void testautovar13() { // Ticket #5537
        check("class FileManager {\n"
              "  FileManager() : UniqueRealDirs(*new UniqueDirContainer())\n"
              "  {}\n"
              "  ~FileManager() {\n"
              "    delete &UniqueRealDirs;\n"
              "   }\n"
              "};");
    }

    void testautovar14() { // Ticket #4776
        check("void f(int x) {\n"
              "label:"
              "  if (x>0) {\n"
              "    x = x >> 1;\n"
              "    goto label;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar15() { // Ticket #6538
        check("static const float4  darkOutline(0.05f, 0.05f, 0.05f, 0.95f);\n"
              "static const float darkLuminosity = 0.05 +\n"
              "                                    0.0722f * math::powf(darkOutline[2], 2.2);\n"
              "const float4* ChooseOutlineColor(const float4& textColor)  {\n"
              "    const float lumdiff = something;\n"
              "    if (lumdiff > 5.0f)\n"
              "        return &darkOutline;\n"
              "    return 0;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar16() { // Ticket #8114
        check("void f(const void* ptr, bool* result) {\n"
              "  int dummy;\n"
              "  *result = (&dummy < ptr);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar17() {
        check("struct S { int* p; };\n" // #14257
              "int a[10];\n"
              "void f(int** q) {\n"
              "    S s;\n"
              "    s.p = a;\n"
              "    *q = &s.p[0];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar18() {
        check("struct S { int* p; };\n"
              "void foo(struct S* s) {\n"
              "    int x;\n"
              "    s[2].p = &x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());
    }

    void testautovar_array1() {
        check("void func1(int* arr[2])\n"
              "{\n"
              "    int num=2;"
              "    arr[0]=&num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());
    }

    void testautovar_array2() {
        check("class Fred {\n"
              "    void func1(int* arr[2]);\n"
              "}\n"
              "void Fred::func1(int* arr[2])\n"
              "{\n"
              "    int num=2;"
              "    arr[0]=&num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:19]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());
    }

    void testautovar_array3() {
        check("int main(int argc, char* argv[]) {\n" // #11732
              "    std::string a = \"abc\";\n"
              "    argv[0] = &a[0];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(char* c[]) {\n"
              "    char a[] = \"abc\";\n"
              "    c[0] = a;\n"
              "}\n"
              "void g(char* c[]) {\n"
              "    std::string a = \"abc\";\n"
              "    c[0] = a.data();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n"
                      "[test.cpp:7:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n",
                      errout_str());

        check("struct String {\n"
              "    String& operator=(const char* c) { m = c; return *this; }\n"
              "    std::string m;\n"
              "};\n"
              "struct T { String s; };\n"
              "void f(T* t) {\n"
              "    char a[] = \"abc\";\n"
              "    t->s = &a[0];\n"
              "}\n"
              "void g(std::string* s) {\n"
              "    char a[] = \"abc\";\n"
              "    *s = &a[0];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int* p[]) {\n" // #7167
              "    int a[4] = { 1, 2, 3 };\n"
              "    p[0] = a;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n",
                      errout_str());
    }

    void testautovar_normal() {
        check("void f(XmDestinationCallbackStruct *ds)\n"
              "{\n"
              "    XPoint DropPoint;\n"
              "    ds->location_data = (XtPointer *)&DropPoint;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error, inconclusive) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());
    }

    void testautovar_ptrptr() { // #6596
        check("void remove_duplicate_matches (char **matches) {\n"
              "  char dead_slot;\n"
              "  matches[0] = (char *)&dead_slot;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());
    }

    void testautovar_return1() {
        check("int* func1()\n"
              "{\n"
              "    int num=2;"
              "    return &num;"
              "}");
        ASSERT_EQUALS("[test.cpp:3:26] -> [test.cpp:3:9] -> [test.cpp:3:26]: (error) Returning pointer to local variable 'num' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());
    }

    void testautovar_return2() {
        check("class Fred {\n"
              "    int* func1();\n"
              "}\n"
              "int* Fred::func1()\n"
              "{\n"
              "    int num=2;"
              "    return &num;"
              "}");
        ASSERT_EQUALS("[test.cpp:6:26] -> [test.cpp:6:9] -> [test.cpp:6:26]: (error) Returning pointer to local variable 'num' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());
    }

    void testautovar_return3() {
        // #2975 - FP
        check("void** f()\n"
              "{\n"
              "    void *&value = tls[id];"
              "    return &value;"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar_return4() {
        // #8058 - FP ignore return in lambda
        check("void foo() {\n"
              "  int cond2;\n"
              "  dostuff([&cond2]() { return &cond2; });\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar_return5() { // #11465
        check("struct S {};\n"
              "const std::type_info* f() {\n"
              "    return &typeid(S);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar_return6() { // #14005
        check("struct S;\n"
              "struct T { const struct S *s; };\n"
              "extern struct T factory();\n"
              "const struct S* f() {\n"
              "    struct T t = factory();\n"
              "    return &t.s[0];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S;\n"
              "struct T { std::vector<struct S> v; };\n"
              "extern struct T factory();\n"
              "const struct S* f() {\n"
              "    struct T t = factory();\n"
              "    return &t.v[0];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:12] -> [test.cpp:5:14] -> [test.cpp:6:12]: (error) Returning pointer to local variable 't' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());
    }

    void testautovar_extern() {
        check("struct foo *f()\n"
              "{\n"
              "    extern struct foo f;\n"
              "    return &f;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testautovar_reassigned() {
        check("void foo(cb* pcb) {\n"
              "  int root0;\n"
              "  pcb->root0 = &root0;\n"
              "  dostuff(pcb);\n"
              "  pcb->root0 = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(cb* pcb) {\n"
              "  int root0;\n"
              "  pcb->root0 = &root0;\n"
              "  dostuff(pcb);\n"
              "  if (condition) return;\n" // <- not reassigned => error
              "  pcb->root0 = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3]: (error, inconclusive) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("void foo(cb* pcb) {\n"
              "  int root0;\n"
              "  pcb->root0 = &root0;\n"
              "  dostuff(pcb);\n"
              "  if (condition)\n"
              "    pcb->root0 = 0;\n"  // <- conditional reassign => error
              "}");
        ASSERT_EQUALS("[test.cpp:3:3]: (error, inconclusive) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("struct S { int *p; };\n"
              "void g(struct S* s) {\n"
              "    int a[10];\n"
              "    s->p = a;\n"
              "    a[0] = 0;\n"
              "}\n"
              "void f() {\n"
              "    struct S s;\n"
              "    g(&s);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n",
            errout_str());
    }

    void testinvaliddealloc() {
        check("void func1() {\n"
              "    char tmp1[256];\n"
              "    free(tmp1);\n"
              "    char tmp2[256];\n"
              "    delete tmp2;\n"
              "    char tmp3[256];\n"
              "    delete tmp3;\n"
              "    char tmp4[256];\n"
              "    delete[] (tmp4);\n"
              "    char tmp5[256];\n"
              "    delete[] tmp5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Deallocation of an auto-variable results in undefined behaviour. [autovarInvalidDeallocation]\n"
                      "[test.cpp:5:12]: (error) Deallocation of an auto-variable results in undefined behaviour. [autovarInvalidDeallocation]\n"
                      "[test.cpp:7:12]: (error) Deallocation of an auto-variable results in undefined behaviour. [autovarInvalidDeallocation]\n"
                      "[test.cpp:9:15]: (error) Deallocation of an auto-variable results in undefined behaviour. [autovarInvalidDeallocation]\n"
                      "[test.cpp:11:14]: (error) Deallocation of an auto-variable results in undefined behaviour. [autovarInvalidDeallocation]\n", errout_str());

        check("void func1(char * ptr) {\n"
              "    free(ptr);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void func1() {\n"
              "    char* tmp1[256];\n"
              "    init(tmp1);\n"
              "    delete tmp1[34];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void func1() {\n"
              "    static char tmp1[256];\n"
              "    char *p = tmp1;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Deallocation of a static variable (tmp1) results in undefined behaviour. [autovarInvalidDeallocation]\n", errout_str());

        check("char tmp1[256];\n"
              "void func1() {\n"
              "    char *p; if (x) p = tmp1;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Deallocation of a global variable (tmp1) results in undefined behaviour. [autovarInvalidDeallocation]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    char psz_title[10];\n"
              "    {\n"
              "        char *psz_title = 0;\n"
              "        abc(0, psz_title);\n"
              "        free(psz_title);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #2298 new check: passing stack-address to free()
        check("int main() {\n"
              "   int *p = malloc(4);\n"
              "   free(&p);\n"
              "   return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Deallocation of an auto-variable results in undefined behaviour. [autovarInvalidDeallocation]\n", errout_str());
        check("int main() {\n"
              "   int i;\n"
              "   free(&i);\n"
              "   return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Deallocation of an auto-variable results in undefined behaviour. [autovarInvalidDeallocation]\n", errout_str());

        // #5732
        check("int main() {\n"
              "   long (*pKoeff)[256] = new long[9][256];\n"
              "   delete[] pKoeff;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int main() {\n"
              "   long *pKoeff[256];\n"
              "   delete[] pKoeff;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Deallocation of an auto-variable results in undefined behaviour. [autovarInvalidDeallocation]\n", errout_str());

        check("int main() {\n"
              "   long *pKoeff[256];\n"
              "   free (pKoeff);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Deallocation of an auto-variable results in undefined behaviour. [autovarInvalidDeallocation]\n", errout_str());

        check("void foo() {\n"
              "   const intPtr& intref = Getter();\n"
              "   delete intref;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void test() {\n"
              "   MyObj& obj = *new MyObj;\n"
              "   delete &obj;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6506
        check("struct F {\n"
              "  void free(void*) {}\n"
              "};\n"
              "void foo() {\n"
              "  char c1[1];\n"
              "  F().free(c1);\n"
              "  char *c2 = 0;\n"
              "  F().free(&c2);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class foo {\n"
              "  void free(void* );\n"
              "  void someMethod() {\n"
              "    char **dst_copy = NULL;\n"
              "    free(&dst_copy);\n"
              "  }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        // #6551
        check("bool foo( ) {\n"
              "  SwTxtFld * pTxtFld = GetFldTxtAttrAt();\n"
              "  delete static_cast<SwFmtFld*>(&pTxtFld->GetAttr());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8910
        check("void f() {\n"
              "    char stack[512];\n"
              "    RGNDATA *data;\n"
              "    if (data_size > sizeof (stack)) data = malloc (data_size);\n"
              "    else data = (RGNDATA *)stack;\n"
              "    if ((char *)data != stack) free (data);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8923
        check("void f(char **args1, char *args2[]) {\n"
              "    free((char **)args1);\n"
              "    free((char **)args2);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #10097
        check("struct Array {\n"
              "    ~Array() { delete m_Arr; }\n"
              "    std::array<long, 256>* m_Arr{};\n"
              "};\n"
              "Array arr;\n");
        ASSERT_EQUALS("", errout_str());

        // #8174
        check("struct S {};\n"
              "void f() {\n"
              "    S s;\n"
              "    S* p = &s;\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Deallocation of an auto-variable (s) results in undefined behaviour. [autovarInvalidDeallocation]\n", errout_str());

        check("void f(bool b, int* q) {\n"
              "    int i;\n"
              "    int* p = b ? &i : q;\n"
              "    if (!b)\n"
              "        free(p);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct E { int* i; };\n" // #11768
              "struct C { E e; };\n"
              "int foo(C* cin) {\n"
              "    E* e = &cin->e;\n"
              "    e->i = new int[42];\n"
              "    delete[] e->i;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    S* p = &g();\n"
              "    delete p;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void testinvaliddealloc_input() {
        // #10600
        check("void f(int* a[]) {\n"
              "    free(a);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a[]) {\n"
              "    free(a);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int* a[]) {\n"
              "    int * p = *a;\n"
              "    free(p);\n"
              "    int ** q = a;\n"
              "    free(q);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a[]) {\n"
              "    int * p = a;\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void testinvaliddealloc_string() {
        // #7341
        check("void f() {\n"
              "    char *ptr = \"a\";\n"
              "    free(\"a\");\n"
              "    delete \"a\";\n"
              "    free(ptr);\n"
              "    delete ptr;\n"
              "    char * p = malloc(1000);\n"
              "    p = \"abc\";\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Deallocation of a string literal results in undefined behaviour. [autovarInvalidDeallocation]\n"
                      "[test.cpp:4:12]: (error) Deallocation of a string literal results in undefined behaviour. [autovarInvalidDeallocation]\n"
                      "[test.cpp:5:10]: (error) Deallocation of a pointer pointing to a string literal (\"a\") results in undefined behaviour. [autovarInvalidDeallocation]\n"
                      "[test.cpp:6:12]: (error) Deallocation of a pointer pointing to a string literal (\"a\") results in undefined behaviour. [autovarInvalidDeallocation]\n"
                      "[test.cpp:9:10]: (error) Deallocation of a pointer pointing to a string literal (\"abc\") results in undefined behaviour. [autovarInvalidDeallocation]\n",
                      errout_str());

        check("void f() {\n"
              "   char *ptr = malloc(10);\n"
              "   char *empty_str = \"\";\n"
              "   if (ptr == NULL)\n"
              "      ptr = empty_str;\n"
              "   if (ptr != empty_str)\n"
              "      free(ptr);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void testinvaliddealloc_C() {
        // #5691
        check("void svn_repos_dir_delta2() {\n"
              "  struct context c;\n"
              "      SVN_ERR(delete(&c, root_baton, src_entry, pool));\n"
              "}\n", dinit(CheckOptions, $.inconclusive = false, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
    }

    void testassign1() { // Ticket #1819
        check("void f(EventPtr *eventP, ActionPtr **actionsP) {\n"
              "    EventPtr event = *eventP;\n"
              "    *actionsP = &event->actions;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void testassign2() { // Ticket #2765
        check("static void function(unsigned long **datap) {\n"
              "    struct my_s *mr = global_structure_pointer;\n"
              "    *datap = &mr->value;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assignAddressOfLocalArrayToGlobalPointer() {
        check("int *p;\n"
              "void f() {\n"
              "  int x[10];\n"
              "  p = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7] -> [test.cpp:3:7] -> [test.cpp:4:3]: (error) Non-local variable 'p' will use pointer to local variable 'x'. [danglingLifetime]\n", errout_str());

        check("int *p;\n"
              "void f() {\n"
              "  int x[10];\n"
              "  p = x;\n"
              "  p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assignAddressOfLocalVariableToGlobalPointer() {
        check("int *p;\n"
              "void f() {\n"
              "  int x;\n"
              "  p = &x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:7] -> [test.cpp:3:7] -> [test.cpp:4:3]: (error) Non-local variable 'p' will use pointer to local variable 'x'. [danglingLifetime]\n", errout_str());

        check("int *p;\n"
              "void f() {\n"
              "  int x;\n"
              "  p = &x;\n"
              "  p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assignAddressOfLocalVariableToMemberVariable() {
        check("struct A {\n"
              "  void f() {\n"
              "    int x;\n"
              "    ptr = &x;\n"
              "  }\n"
              "  int *ptr;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:11] -> [test.cpp:3:9] -> [test.cpp:4:5]: (error) Non-local variable 'ptr' will use pointer to local variable 'x'. [danglingLifetime]\n", errout_str());

        check("struct A {\n"
              "  void f() {\n"
              "    int x;\n"
              "    ptr = &x;\n"
              "    ptr = 0;\n"
              "  }\n"
              "  int *ptr;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct R {\n" // #11393
              "    void pop() { p = nullptr; };\n"
              "    int* p;\n"
              "};\n"
              "struct T {\n"
              "    void f();\n"
              "    R* r;\n"
              "};\n"
              "void T::f() {\n"
              "    int i = 0;\n"
              "    r->p = &i;\n"
              "    r->pop();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void returnLocalVariable1() {
        check("char *foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:12] -> [test.cpp:3:10] -> [test.cpp:4:12]: (error) Returning pointer to local variable 'str' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("char *foo()\n" // use ValueFlow
              "{\n"
              "    char str[100] = {0};\n"
              "    char *p = str;\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:15] -> [test.cpp:3:10] -> [test.cpp:5:12]: (error) Returning pointer to local variable 'str' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("class Fred {\n"
              "    char *foo();\n"
              "};\n"
              "char *Fred::foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:7:12] -> [test.cpp:6:10] -> [test.cpp:7:12]: (error) Returning pointer to local variable 'str' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("char * format_reg(char *outbuffer_start) {\n"
              "    return outbuffer_start;\n"
              "}\n"
              "void print_with_operands() {\n"
              "    char temp[42];\n"
              "    char *tp = temp;\n"
              "    tp = format_reg(tp);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnLocalVariable2() {
        check("std::string foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class Fred {\n"
              "    std::string foo();\n"
              "};\n"
              "std::string Fred::foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    void returnLocalVariable3() { // &x[..]
        // #3030
        check("char *foo() {\n"
              "    char q[] = \"AAAAAAAAAAAA\";\n"
              "    return &q[1];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12] -> [test.cpp:2:10] -> [test.cpp:3:12]: (error) Returning pointer to local variable 'q' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("char *foo()\n"
              "{\n"
              "    static char q[] = \"AAAAAAAAAAAA\";\n"
              "    return &q[1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("char *foo()\n"
              "{\n"
              "char q[] = \"AAAAAAAAAAAA\";\n"
              "char *p;\n"
              "p = &q[1];\n"
              "return p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5] -> [test.cpp:3:6] -> [test.cpp:6:8]: (error) Returning pointer to local variable 'q' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());
    }

    void returnLocalVariable4() { // x+y
        check("char *foo() {\n"
              "    char x[10] = {0};\n"
              "    return x+5;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:2:10] -> [test.cpp:3:13]: (error) Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("char *foo(int y) {\n"
              "    char x[10] = {0};\n"
              "    return (x+8)-y;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:13] -> [test.cpp:2:10] -> [test.cpp:3:17]: (error) Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());
    }

    void returnLocalVariable5() { // cast
        check("char *foo() {\n"
              "    int x[10] = {0};\n"
              "    return (char *)x;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:20] -> [test.cpp:2:9] -> [test.cpp:3:12]: (error) Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());
    }

    void returnLocalVariable6() { // valueflow
        check("int *foo() {\n"
              "    int x = 123;\n"
              "    int p = &x;\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13] -> [test.cpp:2:9] -> [test.cpp:4:12]: (error) Returning object that points to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());
    }

    void returnReference1() {
        check("int &foo()\n"
              "{\n"
              "    int s = 0;\n"
              "    int& x = s;\n"
              "    return x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12] -> [test.cpp:5:12]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("std::string &foo()\n"
              "{\n"
              "    std::string s;\n"
              "    return s;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("std::vector<int> &foo()\n"
              "{\n"
              "    std::vector<int> v;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("std::vector<int> &foo()\n"
              "{\n"
              "    static std::vector<int> v;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<int> &foo()\n"
              "{\n"
              "    thread_local std::vector<int> v;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "\n"
              "std::string &f()\n"
              "{\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:17]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        // make sure scope is used in function lookup
        check("class Fred {\n"
              "    std::string hello() {\n"
              "        return std::string();\n"
              "    }\n"
              "};\n"
              "std::string &f() {\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string hello() {\n"
              "     return std::string();\n"
              "}\n"
              "\n"
              "std::string &f() {\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:17]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("std::string hello() {\n"
              "     return \"foo\";\n"
              "}\n"
              "\n"
              "std::string &f() {\n"
              "    return hello().substr(1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:26]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("class Foo;\n"
              "Foo hello() {\n"
              "     return Foo();\n"
              "}\n"
              "\n"
              "Foo& f() {\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:17]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        // make sure function overloads are handled properly
        check("class Foo;\n"
              "Foo & hello(bool) {\n"
              "     static Foo foo;\n"
              "     return foo;\n"
              "}\n"
              "Foo hello() {\n"
              "     return Foo();\n"
              "}\n"
              "\n"
              "Foo& f() {\n"
              "    return hello(true);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("Foo hello() {\n"
              "     return Foo();\n"
              "}\n"
              "\n"
              "Foo& f() {\n" // Unknown type - might be a reference
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference2() {
        check("class Fred {\n"
              "    std::string &foo();\n"
              "}\n"
              "std::string &Fred::foo()\n"
              "{\n"
              "    std::string s;\n"
              "    return s;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:12]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("class Fred {\n"
              "    std::vector<int> &foo();\n"
              "};\n"
              "std::vector<int> &Fred::foo()\n"
              "{\n"
              "    std::vector<int> v;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:12]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("class Fred {\n"
              "    std::vector<int> &foo();\n"
              "};\n"
              "std::vector<int> &Fred::foo()\n"
              "{\n"
              "    static std::vector<int> v;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class Fred {\n"
              "    std::string &f();\n"
              "};\n"
              "std::string hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "std::string &Fred::f()\n"
              "{\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:17]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("class Fred {\n"
              "    std::string hello();\n"
              "    std::string &f();\n"
              "};\n"
              "std::string Fred::hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "std::string &Fred::f()\n"
              "{\n"
              "    return hello();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11:17]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("class Bar;\n"
              "Bar foo() {\n"
              "     return something;\n"
              "}\n"
              "Bar& bar() {\n"
              "    return foo();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:15]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("std::map<int, string> foo() {\n"
              "     return something;\n"
              "}\n"
              "std::map<int, string>& bar() {\n"
              "    return foo();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:15]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("Bar foo() {\n"
              "     return something;\n"
              "}\n"
              "Bar& bar() {\n" // Unknown type - might be a typedef to a reference type
              "    return foo();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        // Don't crash with function in unknown scope (#4076)
        check("X& a::Bar() {}"
              "X& foo() {"
              "    return Bar();"
              "}");
    }

    void returnReference3() {
        check("double & f(double & rd) {\n"
              "    double ret = getValue();\n"
              "    rd = ret;\n"
              "    return rd;\n"
              "}", dinit(CheckOptions, $.inconclusive = false));
        ASSERT_EQUALS("", errout_str());
    }

    // Returning reference to global variable
    void returnReference4() {
        check("double a;\n"
              "double & f() {\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference5() {
        check("struct A {\n"
              "    int i;\n"
              "};\n"

              "struct B {\n"
              "    A a;\n"
              "};\n"

              "struct C {\n"
              "    B *b;\n"
              "    const A& a() const {\n"
              "        const B *pb = b;\n"
              "        const A &ra = pb->a;\n"
              "        return ra;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference6() {
        check("Fred & create() {\n"
              "    Fred &fred(*new Fred);\n"
              "    return fred;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference7() {  // 3791 - false positive for overloaded function
        check("std::string a();\n"
              "std::string &a(int);\n"
              "std::string &b() {\n"
              "    return a(12);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string &a(int);\n"
              "std::string a();\n"
              "std::string &b() {\n"
              "    return a(12);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference8() {
        check("int& f(std::vector<int> &v) {\n"
              "    std::vector<int>::iterator it = v.begin();\n"
              "    int& value = *it;\n"
              "    return value;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference9() {
        check("int& f(bool b, int& x, int& y) {\n"
              "    return b ? x : y;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference10() {
        check("class A { int f() const; };\n"
              "int& g() {\n"
              "    A a;\n"
              "    return a.f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:15]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("class A { int& f() const; };\n"
              "int& g() {\n"
              "    A a;\n"
              "    return a.f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference11() {
        check("class A { static int f(); };\n"
              "int& g() {\n"
              "    return A::f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("class A { static int& f(); };\n"
              "int& g() {\n"
              "    return A::f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("namespace A { int& f(); }\n"
              "int& g() {\n"
              "    return A::f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference12() {
        check("class A { static int& f(); };\n"
              "auto g() {\n"
              "    return &A::f;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class A { static int& f(); };\n"
              "auto g() {\n"
              "    auto x = &A::f;\n"
              "    return x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference13() {
        check("std::vector<int> v;\n"
              "void* vp = &v;\n"
              "int& foo(size_t i) {\n"
              "    return ((std::vector<int>*)vp)->at(i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<int> v;\n"
              "void* vp = &v;\n"
              "int& foo(size_t i) {\n"
              "    return static_cast<std::vector<int>*>(vp)->at(i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference14() {
        check("struct C { void* m; };\n"
              "struct A { void* &f(); };\n"
              "C* g() {\n"
              "    static C c;\n"
              "    return &c;\n"
              "}\n"
              "void* &A::f() {\n"
              "    return g()->m;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference15() {
        check("template <class T>\n"
              "const int& f() {\n"
              "    static int s;\n"
              "    return s;\n"
              "}\n"
              "template <class T>\n"
              "const int& f(const T&) {\n"
              "    return f<T>();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("template <class T>\n"
              "int g();\n"
              "template <class T>\n"
              "const int& f(const T&) {\n"
              "    return g<T>();\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

    void returnReference16() {
        check("int& f(std::tuple<int>& x) {\n"
              "    return std::get<0>(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int& f(int x) {\n"
              "    return std::get<0>(std::make_tuple(x));\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

    void returnReference17() {
        check("auto g() -> int&;\n"
              "int& f() {\n"
              "    return g();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference18() {
        check("template<class T>\n"
              "auto f(T& x) -> decltype(x);\n"
              "int& g(int* x) {\n"
              "    return f(*x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // #9597
    void returnReference19() {
        check("struct C : B {\n"
              "    const B &f() const { return (const B &)*this; }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // #9536
    void returnReference20() {
        check("struct a {\n"
              "    int& operator()() const;\n"
              "};\n"
              "int& b() {\n"
              "    return a()();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("auto a() {\n"
              "    return []() -> int& {\n"
              "        static int b;\n"
              "        return b;\n"
              "    };\n"
              "}\n"
              "const int& c() {\n"
              "    return a()();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::function<int&()> a();\n"
              "int& b() {\n"
              "    return a()();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9889
        check("int f(std::vector<std::function<int&()>>& v, int i) {\n"
              "    auto& j = v[i]();\n"
              "    return j;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // #9530
    void returnReference21() {
        check("int& f(int& x) {\n"
              "    return {x};\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference22() {
        check("int& f() {\n"
              "    std::unique_ptr<int> p = std::make_unique<int>(1);\n"
              "    return *p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("void g(const std::unique_ptr<int>&);\n"
              "int& f() {\n"
              "    std::unique_ptr<int> p = std::make_unique<int>(1);\n"
              "    g(p);\n"
              "    return *p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("void g(std::shared_ptr<int>);\n"
              "int& f() {\n"
              "    std::shared_ptr<int> p = std::make_shared<int>(1);\n"
              "    g(p);\n"
              "    return *p;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::shared_ptr<int> g();\n"
              "int& f() {\n"
              "    return *g();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::unique_ptr<int> g();\n"
              "int& f() {\n"
              "    return *g();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("struct A { int x; };\n"
              "int& f() {\n"
              "    std::unique_ptr<A> p = std::make_unique<A>();\n"
              "    return p->x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Reference to local variable returned. [returnReference]\n", errout_str());
    }

    void returnReference23() {
        check("const std::vector<int> * g();\n"
              "const std::vector<int>& f() {\n"
              "    return *g();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference24()
    {
        check("struct A {\n"
              "    A() {}\n"
              "};\n"
              "const A& a() {\n"
              "    return A();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());
    }

    void returnReference25()
    {
        check("int& f();\n" // #10983
              "auto g() -> decltype(f()) {\n"
              "    return f();\n"
              "}\n"
              "int& h() {\n"
              "    return g();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference26()
    {
        check("const int& f() {\n" // #12153
              "    int x = 234;\n"
              "    int& s{ x };\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:4:12]: (error) Reference to local variable returned. [returnReference]\n", errout_str());
    }

    void returnReference27()
    {
        check("struct S {\n" // #12607
              "    const std::string & f(const std::string& a, const std::string& b) {\n"
              "        auto status = m.emplace(a, b);\n"
              "        return status.first->second;\n"
              "    }\n"
              "    std::map<std::string, std::string> m;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference28()
    {
        check("struct S {\n" // #13373
              "    int& r;\n"
              "};\n"
              "int& f(S s) {\n"
              "    return s.r;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReference29()
    {
        check("const std::string& f() {\n" // #12548
              "    return std::string{};\n"
              "}\n"
              "const std::string& g() {\n"
              "    return {};\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:23]: (error) Reference to temporary returned. [returnTempReference]\n"
                      "[test.cpp:5:12]: (error) Reference to temporary returned. [returnTempReference]\n",
                      errout_str());
    }

    void returnReferenceFunction() {
        check("int& f(int& a) {\n"
              "    return a;\n"
              "}\n"
              "int& hello() {\n"
              "    int x = 0;\n"
              "    return f(x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:1:14] -> [test.cpp:2:12] -> [test.cpp:6:12] -> [test.cpp:6:13]: (error) Reference to local variable returned. [returnReference]\n",
            errout_str());

        check("int& f(int& a) {\n"
              "    return a;\n"
              "}\n"
              "int* hello() {\n"
              "    int x = 0;\n"
              "    return &f(x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:1:14] -> [test.cpp:2:12] -> [test.cpp:6:13] -> [test.cpp:6:12] -> [test.cpp:5:9] -> [test.cpp:6:12]: (error) Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("int* f(int * x) {\n"
              "    return x;\n"
              "}\n"
              "int * g(int x) {\n"
              "    return f(&x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:14] -> [test.cpp:5:14] -> [test.cpp:4:13] -> [test.cpp:5:13]: (error) Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("int* f(int * x) {\n"
              "    x = nullptr;\n"
              "    return x;\n"
              "}\n"
              "int * g(int x) {\n"
              "    return f(&x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int& a) {\n"
              "    return a;\n"
              "}\n"
              "int& hello() {\n"
              "    int x = 0;\n"
              "    return f(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:13]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("int& f(int a) {\n"
              "    return a;\n"
              "}\n"
              "int& hello() {\n"
              "    int x = 0;\n"
              "    return f(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("int f(int a) {\n"
              "    return a;\n"
              "}\n"
              "int& hello() {\n"
              "    int x = 0;\n"
              "    return f(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:13]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("template<class T>\n"
              "int& f(int& x, T y) {\n"
              "    x += y;\n"
              "    return x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReferenceContainer() {
        check("auto& f() {\n"
              "    std::vector<int> x;\n"
              "    return x[0];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("auto& f() {\n"
              "    std::vector<int> x;\n"
              "    return x.front();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:14] -> [test.cpp:3:19]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("std::vector<int> g();\n"
              "auto& f() {\n"
              "    return g().front();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:16] -> [test.cpp:3:21]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("auto& f() {\n"
              "    return std::vector<int>{1}.front();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:32] -> [test.cpp:2:37]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("struct A { int foo; };\n"
              "int& f(std::vector<A> v) {\n"
              "    auto it = v.begin();\n"
              "    return it->foo;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:22] -> [test.cpp:4:14]: (error) Reference to local variable returned. [returnReference]\n", errout_str());

        check("template <class T, class K, class V>\n"
              "const V& get_default(const T& t, const K& k, const V& v) {\n"
              "    auto it = t.find(k);\n"
              "    if (it == t.end()) return v;\n"
              "    return it->second;\n"
              "}\n"
              "const int& bar(const std::unordered_map<int, int>& m, int k) {\n"
              "    auto x = 0;\n"
              "    return get_default(m, k, x);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:56] -> [test.cpp:4:31] -> [test.cpp:9:12] -> [test.cpp:9:23]: (error, inconclusive) Reference to local variable returned. [returnReference]\n",
            errout_str());

        check("template <class T, class K, class V>\n"
              "const V& get_default(const T& t, const K& k, const V& v) {\n"
              "    auto it = t.find(k);\n"
              "    if (it == t.end()) return v;\n"
              "    return it->second;\n"
              "}\n"
              "const int& bar(const std::unordered_map<int, int>& m, int k) {\n"
              "    return get_default(m, k, 0);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:56] -> [test.cpp:4:31] -> [test.cpp:8:12] -> [test.cpp:8:23]: (error, inconclusive) Reference to temporary returned. [returnTempReference]\n",
            errout_str());

        check("struct A { int foo; };\n"
              "int& f(std::vector<A>& v) {\n"
              "    auto it = v.begin();\n"
              "    return it->foo;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("static std::vector<int> A[2];\n"
              "static std::vector<int> B;\n"
              "std::vector<int>& g(int i) {\n"
              "    return i ? A[i] : B;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReferenceLiteral() {
        check("const std::string &a() {\n"
              "    return \"foo\";\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("const std::string a() {\n"
              "    return \"foo\";\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("const std::string& f(const std::string& x) { return x; }\n"
              "const std::string &a() {\n"
              "    return f(\"foo\");\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:1:42] -> [test.cpp:1:53] -> [test.cpp:3:12] -> [test.cpp:3:13]: (error) Reference to temporary returned. [returnTempReference]\n",
            errout_str());

        check("const char * f(const char * x) { return x; }\n"
              "const std::string &a() {\n"
              "    return f(\"foo\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());
    }

    void returnReferenceCalculation() {
        check("const std::string &a(const std::string& str) {\n"
              "    return \"foo\" + str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:18]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("int& operator<<(int out, int path) {\n"
              "    return out << path;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("std::ostream& operator<<(std::ostream& out, const std::string& path) {\n"
              "    return out << path;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::ostream& operator<<(std::ostream* out, const std::string& path) {\n"
              "    return *out << path;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("Unknown1& operator<<(Unknown1 out, Unknown2 path) {\n"
              "    return out << path;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int& a(int b) {\n"
              "    return 2*(b+1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("const std::string &a(const std::string& str) {\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("const std::string &a(int bar) {\n"
              "    return foo(bar + 1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("const std::string a(const std::string& str) {\n"
              "    return \"foo\" + str;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int& incValue(int& value) {\n"
              "    return ++value;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReferenceLambda() {
        // #6787
        check("const Item& foo(const Container& items) const {\n"
              "    return bar(items.begin(), items.end(),\n"
              "    [](const Item& lhs, const Item& rhs) {\n"
              "        return false;\n"
              "    });\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5844
        check("map<string,string> const &getVariableTable() {\n"
              "static map<string,string> const s_var = []{\n"
              "    map<string,string> var;\n"
              "    return var;\n"
              "  }();\n"
              "return s_var;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7583
        check("Command& foo() {\n"
              "  return f([]() -> int { return 1; });\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReferenceInnerScope() {
        // #6951
        check("const Callback& make() {\n"
              "    struct _Wrapper {\n"
              "        static ulong call(void* o, const void* f, const void*[]) {\n"
              "            return 1;\n"
              "        }\n"
              "    };\n"
              "    return _make(_Wrapper::call, pmf);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void returnReferenceRecursive() {
        check("int& f() { return f(); }");
        ASSERT_EQUALS("", errout_str());

        check("int& g(int& i) { return i; }\n"
              "int& f() { return g(f()); }");
        ASSERT_EQUALS("", errout_str());
    }

    void extendedLifetime() {
        check("void g(int*);\n"
              "int h();\n"
              "auto f() {\n"
              "    const int& x = h();\n"
              "    return [&] { return x; };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18] -> [test.cpp:5:25] -> [test.cpp:4:16] -> [test.cpp:5:12]: (error) Returning lambda that captures local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("void g(int*);\n"
              "int h();\n"
              "int* f() {\n"
              "    const int& x = h();\n"
              "    return &x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18] -> [test.cpp:5:12] -> [test.cpp:4:16] -> [test.cpp:5:12]: (error) Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("void g(int*);\n"
              "int h();\n"
              "void f() {\n"
              "    int& x = h();\n"
              "    g(&x);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:12] -> [test.cpp:5:7] -> [test.cpp:4:15] -> [test.cpp:5:7]: (error) Using pointer that is a temporary. [danglingTemporaryLifetime]\n"
            "[test.cpp:4:12] -> [test.cpp:5:8]: (error) Using reference to dangling temporary. [danglingTempReference]\n",
            errout_str());

        check("void g(int*);\n"
              "int h();\n"
              "void f() {\n"
              "    const int& x = h();\n"
              "    g(&x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Data {\n"
              "    std::string path;\n"
              "};\n"
              "const char* foo() {\n"
              "    const Data& data = getData();\n"
              "    return data.path.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void danglingReference() {
        check("int f( int k )\n"
              "{\n"
              "    static int &r = k;\n"
              "    return r;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:19] -> [test.cpp:3:17]: (error) Non-local reference variable 'r' to local variable 'k' [danglingReference]\n",
                      errout_str());

        check("int &f( int & k )\n"
              "{\n"
              "    static int &r = k;\n"
              "    return r;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void danglingTempReference() {
        check("const std::string& g(const std::string& str_cref) {\n"
              "    return str_cref;\n"
              "}\n"
              "void f() {\n"
              "    const auto& str_cref2 = g(std::string(\"hello\"));\n"
              "    std::cout << str_cref2 << std::endl;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:27] -> [test.cpp:1:49] -> [test.cpp:2:12] -> [test.cpp:5:29] -> [test.cpp:6:18]: (error) Using reference to dangling temporary. [danglingTempReference]\n", errout_str());

        // Lifetime extended
        check("std::string g(const std::string& str_cref) {\n"
              "    return str_cref;\n"
              "}\n"
              "void f() {\n"
              "    const auto& str_cref2 = g(std::string(\"hello\"));\n"
              "    std::cout << str_cref2 << std::endl;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("char f() {\n"
              "    char c = 0;\n"
              "    char&& cr = std::move(c);\n"
              "    return cr;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9987
        check("void g(std::vector<int>);\n"
              "void f() {\n"
              "    std::vector<int>&& v = {};\n"
              "    g(std::move(v));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(std::vector<int>);\n"
              "std::vector<int> h();\n"
              "void f() {\n"
              "    std::vector<int>&& v = h();\n"
              "    g(std::move(v));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11087
        check("struct S1 {\n"
              "    int& get() { return val; }\n"
              "    int val{42};\n"
              "};\n"
              "void f() {\n"
              "    int& v = S1().get();\n"
              "    v += 1;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:6:12] -> [test.cpp:2:25] -> [test.cpp:6:19] -> [test.cpp:7:5]: (error) Using reference to dangling temporary. [danglingTempReference]\n",
            errout_str());

        check("struct A {\n"
              "    const int& g() const { return i; }\n"
              "    int i;\n"
              "};\n"
              "A* a();\n"
              "int f() {\n"
              "    const int& i = a()->g();\n"
              "    return i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    const int& g() const { return i; }\n"
              "    int i;\n"
              "};\n"
              "std::unique_ptr<A> a();\n"
              "int f() {\n"
              "    const int& i = a()->g();\n"
              "    return i;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:7:18] -> [test.cpp:2:35] -> [test.cpp:7:25] -> [test.cpp:8:12]: (error) Using reference to dangling temporary. [danglingTempReference]\n",
            errout_str());

        check("struct S1 {\n"
              "    auto get() -> auto& { return val; }\n"
              "    int val{42};\n"
              "};\n"
              "struct S2 {\n"
              "    auto get() -> S1 { return s; }\n"
              "    S1 s;\n"
              "};\n"
              "auto main() -> int {\n"
              "    S2 c{};\n"
              "    auto& v = c.get().get();\n"
              "    v += 1;\n"
              "    return c.s.val;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:11:13] -> [test.cpp:2:34] -> [test.cpp:11:23] -> [test.cpp:12:5]: (error) Using reference to dangling temporary. [danglingTempReference]\n",
            errout_str());

        check("struct C {\n"
              "  std::vector<std::vector<int>> v;\n"
              "};\n"
              "struct P {\n"
              "  std::vector<C*>::const_iterator find() const { return pv.begin(); }\n"
              "  std::vector<C*> pv;\n"
              "};\n"
              "struct M {\n"
              "  const P* get() const { return p; }\n"
              "  P* p;\n"
              "};\n"
              "void f(const M* m) {\n"
              "  auto it = m->get()->find();\n"
              "  auto e = (*it)->v.begin();\n"
              "  const int& x = (*e)[1];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int* g();\n" // #11188
              "void f() {\n"
              "    const auto& p = g();\n"
              "    if (p != nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("template<typename S, typename T>\n"
              "void f(const std::vector<S>& v) {\n"
              "    T a;\n"
              "    for (typename std::vector<S>::iterator it = v.begin(); it != v.end(); ++it) {\n"
              "        const T& b = static_cast<const T&>(it->find(1));\n"
              "        a = b;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("using namespace std;\n" // #10971
              "struct S { int i = 3; };\n"
              "unique_ptr<S> g() {\n"
              "    auto tp = make_unique<S>();\n"
              "    return tp;\n"
              "}\n"
              "void f() {\n"
              "    const S& s = *g();\n"
              "    (void)s.i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:16] -> [test.cpp:9:11]: (error) Using reference to dangling temporary. [danglingTempReference]\n", errout_str());

        check("std::string f() {\n" // #12173
              "    std::string s;\n"
              "    for (auto& c : { \"a\", \"b\", \"c\" }) {\n"
              "        s += c;\n"
              "    }\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void testglobalnamespace() {
        check("class SharedPtrHolder\n"
              "{\n"
              "   ::std::tr1::shared_ptr<int> pNum;\n"
              "public:\n"
              "   void SetNum(const ::std::tr1::shared_ptr<int> & apNum)\n"
              "   {\n"
              "      pNum = apNum;\n"
              "   }\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void returnParameterAddress() {
        check("int* foo(int y)\n"
              "{\n"
              "  return &y;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:1:14] -> [test.cpp:3:10]: (error) Returning pointer to local variable 'y' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("int ** foo(int * y)\n"
              "{\n"
              "  return &y;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:1:18] -> [test.cpp:3:10]: (error) Returning pointer to local variable 'y' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("const int * foo(const int & y)\n"
              "{\n"
              "  return &y;\n"
              "}");

        ASSERT_EQUALS("", errout_str());

        check("int * foo(int * y)\n"
              "{\n"
              "  return y;\n"
              "}");

        ASSERT_EQUALS("", errout_str());

        check("struct s { void *p; };\n"
              "extern struct s* f(void);\n"
              "void g(void **q)\n"
              "{\n"
              "    struct s *r = f();\n"
              "    *q = &r->p;\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void testconstructor() { // Ticket #5478 - crash while checking a constructor
        check("class const_tree_iterator {\n"
              "  const_tree_iterator(bool (*_incream)(node_type*&)) {}\n"
              "  const_tree_iterator& parent() {\n"
              "    return const_tree_iterator(foo);\n"
              "  }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:31]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());
    }

    void variableIsUsedInScope() {
        check("void removed_cb (GList *uids) {\n"
              "for (; uids; uids = uids->next) {\n"
              "}\n"
              "}\n"
              "void opened_cb () {\n"
              "	g_signal_connect (G_CALLBACK (removed_cb));\n"
              "}");
    }

    void danglingLifetimeLambda() {
        check("auto f() {\n"
              "    int a = 1;\n"
              "    auto l = [&](){ return a; };\n"
              "    return l;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:28] -> [test.cpp:2:9] -> [test.cpp:4:12]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f() {\n"
              "    int a = 1;\n"
              "    return [&](){ return a; };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:26] -> [test.cpp:2:9] -> [test.cpp:3:12]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f(int a) {\n"
              "    return [&](){ return a; };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:26] -> [test.cpp:1:12] -> [test.cpp:2:12]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f(int a) {\n"
              "    auto p = &a;\n"
              "    return [=](){ return p; };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14] -> [test.cpp:3:26] -> [test.cpp:1:12] -> [test.cpp:3:12]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto g(int& a) {\n"
              "    int p = a;\n"
              "    return [&](){ return p; };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:26] -> [test.cpp:2:9] -> [test.cpp:3:12]: (error) Returning lambda that captures local variable 'p' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f() {\n"
              "    return [=](){\n"
              "        int a = 1;\n"
              "        return [&](){ return a; };\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:30] -> [test.cpp:3:13] -> [test.cpp:4:16]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f(int b) {\n"
              "    return [=](int a){\n"
              "        a += b;\n"
              "        return [&](){ return a; };\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:30] -> [test.cpp:2:20] -> [test.cpp:4:16]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto g(int& a) {\n"
              "    return [&](){ return a; };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("auto g(int a) {\n"
              "    auto p = a;\n"
              "    return [=](){ return p; };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("auto g(int& a) {\n"
              "    auto p = a;\n"
              "    return [=](){ return p; };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("auto g(int& a) {\n"
              "    int& p = a;\n"
              "    return [&](){ return p; };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("template<class F>\n"
              "void g(F);\n"
              "auto f() {\n"
              "    int x;\n"
              "    return g([&]() { return x; });\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("auto f() {\n"
              "    int i = 0;\n"
              "    return [&i] {};\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:14] -> [test.cpp:2:9] -> [test.cpp:3:12]: (error) Returning lambda that captures local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f() {\n"
              "    int i = 0;\n"
              "    return [i] {};\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("auto f() {\n"
              "    int i = 0;\n"
              "    return [=, &i] {};\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:17] -> [test.cpp:2:9] -> [test.cpp:3:12]: (error) Returning lambda that captures local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f() {\n"
              "    int i = 0;\n"
              "    int j = 0;\n"
              "    return [=, &i] { return j; };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:17] -> [test.cpp:2:9] -> [test.cpp:4:12]: (error) Returning lambda that captures local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f() {\n"
              "    int i = 0;\n"
              "    return [&, i] {};\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("auto f() {\n"
              "    int i = 0;\n"
              "    int j = 0;\n"
              "    return [&, i] { return j; };\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:28] -> [test.cpp:3:9] -> [test.cpp:4:12]: (error) Returning lambda that captures local variable 'j' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f(int& i) {\n"
              "    int j = 0;\n"
              "    return [=, &i] { return j; };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int*);\n"
              "auto g(int y) {\n"
              "    int x = y;\n"
              "    return [=] {\n"
              "        g(&x);\n"
              "    };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int x;\n"
              "};\n"
              "auto f() {\n"
              "    A a;\n"
              "    return [=] {\n"
              "        const A* ap = &a;\n"
              "        ap->x;\n"
              "    };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void danglingLifetimeContainer() {
        check("auto f(const std::vector<int>& x) {\n"
              "    auto it = x.begin();\n"
              "    return it;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<int>::iterator f() {\n"
              "    std::vector<int> x;\n"
              "    auto it = x.begin();\n"
              "    return it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:22] -> [test.cpp:2:22] -> [test.cpp:4:12]: (error) Returning iterator to local container 'x' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto it = std::begin(x);\n"
              "    return it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:25] -> [test.cpp:2:22] -> [test.cpp:4:12]: (error) Returning iterator to local container 'x' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("int* f() {\n"
              "    std::vector<int> x;\n"
              "    auto p = x.data();\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:20] -> [test.cpp:2:22] -> [test.cpp:4:12]: (error) Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto p = &x[0];\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:14] -> [test.cpp:2:22] -> [test.cpp:4:12]: (error) Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A { int foo; };\n"
              "int* f(std::vector<A> v) {\n"
              "    auto it = v.begin();\n"
              "    return &it->foo;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:22] -> [test.cpp:4:12] -> [test.cpp:2:23] -> [test.cpp:4:12]: (error) Returning pointer to local variable 'v' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::vector<int>::iterator f(std::vector<int> x) {\n"
              "    auto it = x.begin();\n"
              "    return it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22] -> [test.cpp:1:47] -> [test.cpp:3:12]: (error) Returning iterator to local container 'x' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto it = x.begin();\n"
              "    return std::next(it);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:22] -> [test.cpp:2:22] -> [test.cpp:4:21]: (error) Returning iterator to local container 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto it = x.begin();\n"
              "    return it + 1;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:22] -> [test.cpp:2:22] -> [test.cpp:4:15]: (error) Returning iterator to local container 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("auto f() {\n"
              "    std::vector<int> x;\n"
              "    auto it = x.begin();\n"
              "    return std::next(it + 1);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:22] -> [test.cpp:4:25] -> [test.cpp:2:22] -> [test.cpp:4:21]: (error) Returning object that points to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::vector<int*> f() {\n"
              "    int i = 0;\n"
              "    std::vector<int*> v;\n"
              "    v.push_back(&i);\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:17] -> [test.cpp:4:17] -> [test.cpp:2:9] -> [test.cpp:5:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::vector<int*> f() {\n"
              "    std::vector<int*> r;\n"
              "    int i = 0;\n"
              "    std::vector<int*> v;\n"
              "    v.push_back(&i);\n"
              "    r.assign(v.begin(), v.end());\n"
              "    return r;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:5:17] -> [test.cpp:5:17] -> [test.cpp:5:5] -> [test.cpp:3:9] -> [test.cpp:7:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    std::vector<int*> v;\n"
              "    void f() {\n"
              "        int i;\n"
              "        v.push_back(&i);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS(
            "[test.cpp:5:21] -> [test.cpp:5:21] -> [test.cpp:4:13] -> [test.cpp:5:9]: (error) Non-local variable 'v' will use object that points to local variable 'i'. [danglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    std::vector<int*> v;\n"
              "    void f() {\n"
              "        int i;\n"
              "        int * p = &i;\n"
              "        v.push_back(p);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS(
            "[test.cpp:5:19] -> [test.cpp:6:21] -> [test.cpp:4:13] -> [test.cpp:6:9]: (error) Non-local variable 'v' will use object that points to local variable 'i'. [danglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    std::vector<int*> m;\n"
              "    void f() {\n"
              "        int x;\n"
              "        std::vector<int*> v;\n"
              "        v.push_back(&x);\n"
              "        m.insert(m.end(), v.begin(), v.end());\n"
              "    }\n"
              "};");
        ASSERT_EQUALS(
            "[test.cpp:6:21] -> [test.cpp:6:21] -> [test.cpp:6:9] -> [test.cpp:4:13] -> [test.cpp:7:9]: (error) Non-local variable 'm' will use object that points to local variable 'x'. [danglingLifetime]\n"
            "[test.cpp:6:21] -> [test.cpp:6:21] -> [test.cpp:6:9] -> [test.cpp:4:13] -> [test.cpp:7:18]: (error) Non-local variable 'm' will use object that points to local variable 'x'. [danglingLifetime]\n", // duplicate
            errout_str());

        check("std::vector<int>::iterator f(std::vector<int> v) {\n"
              "    for(auto it = v.begin();it != v.end();it++) {\n"
              "        return it;\n"
              "    }\n"
              "    return {};\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:26] -> [test.cpp:1:47] -> [test.cpp:3:16]: (error) Returning iterator to local container 'v' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("const char * f() {\n"
              "   std::string ba(\"hello\");\n"
              "   return ba.c_str();\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:19] -> [test.cpp:2:16] -> [test.cpp:3:19]: (error) Returning pointer to local variable 'ba' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("template <class T, class K, class V>\n"
              "const V* get_default(const T& t, const K& k, const V* v) {\n"
              "    auto it = t.find(k);\n"
              "    if (it == t.end()) return v;\n"
              "    return &it->second;\n"
              "}\n"
              "const int* bar(const std::unordered_map<int, int>& m, int k) {\n"
              "    auto x = 0;\n"
              "    return get_default(m, k, &x);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:30] -> [test.cpp:9:30] -> [test.cpp:8:10] -> [test.cpp:9:23]: (error, inconclusive) Returning pointer to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::vector<int> g();\n"
              "auto f() {\n"
              "    return g().begin();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:21] -> [test.cpp:3:21]: (error) Returning iterator that will be invalid when returning. [returnDanglingLifetime]\n",
                      errout_str());

        check("std::vector<int> g();\n"
              "std::vector<int>::iterator f() {\n"
              "    auto it = g().begin();\n"
              "    return it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:24] -> [test.cpp:3:16] -> [test.cpp:4:5]: (error) Using iterator that is a temporary. [danglingTemporaryLifetime]\n",
                      errout_str());

        check("std::vector<int> g();\n"
              "int& f() {\n"
              "    return *g().begin();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:22] -> [test.cpp:3:12]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("struct A {\n"
              "    std::vector<std::string> v;\n"
              "    void f() {\n"
              "        char s[3];\n"
              "        v.push_back(s);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<std::string> f() {\n"
              "    const char * s = \"hello\";\n"
              "    std::vector<std::string> v;\n"
              "    v.push_back(s);\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("auto f() {\n"
              "  static std::vector<int> x;\n"
              "  return x.begin();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string g() {\n"
              "    std::vector<char> v;\n"
              "    return v.data();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<int>::iterator f(std::vector<int>* v) {\n"
              "    return v->begin();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<int>::iterator f(std::vector<int>* v) {\n"
              "    std::vector<int>* v = new std::vector<int>();\n"
              "    return v->begin();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(std::vector<int> v) {\n"
              "    return *v.begin();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(std::vector<int> v) {\n"
              "    return v.end() - v.begin();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("auto g() {\n"
              "    std::vector<char> v;\n"
              "    return {v, [v]() { return v.data(); }};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("template<class F>\n"
              "void g(F);\n"
              "auto f() {\n"
              "    std::vector<char> v;\n"
              "    return g([&]() { return v.data(); });\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<int> g();\n"
              "struct A {\n"
              "    std::vector<int> m;\n"
              "    void f() {\n"
              "        std::vector<int> v = g();\n"
              "        m.insert(m.end(), v.begin(), v.end());\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n"
              "    std::vector<int> v = {1};\n"
              "    if (b) {\n"
              "        int a[] = {0};\n"
              "        v.insert(a, a+1);\n"
              "    }\n"
              "    return v.back() == 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class A {\n"
              "    int f( P p ) {\n"
              "        std::vector< S > maps;\n"
              "        m2.insert( m1.begin(), m1.end() );\n"
              "    }\n"
              "    struct B {};\n"
              "    std::map< S, B > m1;\n"
              "    std::map< S, B > m2;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    std::vector<int*> v;\n"
              "    int x;\n"
              "    void f() {\n"
              "        v.push_back(&x);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("size_t f(const std::string& x) {\n"
              "    std::string y = \"x\";\n"
              "    return y.find(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string* f();\n"
              "const char* g() {\n"
              "    std::string* var = f();\n"
              "    return var->c_str();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string f() {\n"
              "    std::vector<char> data{};\n"
              "    data.push_back('a');\n"
              "    return std::string{ data.data(), data.size() };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<char*> f() {\n"
              "    char a = 0;\n"
              "    return std::vector<char*>{&a};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:31] -> [test.cpp:3:31] -> [test.cpp:2:10] -> [test.cpp:3:30]: (error) Returning object that points to local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("std::vector<int>* g();\n"
              "int& f() {\n"
              "    auto* p = g();\n"
              "    return p->front();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<std::vector<int>> g();\n"
              "void f() {\n"
              "    for(auto& x:g())\n"
              "        std::sort(x.begin(), x.end());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    std::vector<int*> v;\n"
              "    void add(int* i) {\n"
              "        v.push_back(i);\n"
              "    }\n"
              "    void f() {\n"
              "        int i = 0;\n"
              "        add(&i);\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS(
            "[test.cpp:8:13] -> [test.cpp:8:13] -> [test.cpp:4:21] -> [test.cpp:7:13] -> [test.cpp:4:9]: (error) Non-local variable 'v' will use object that points to local variable 'i'. [danglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    std::vector<int*> v;\n"
              "    void add(int* i) {\n"
              "        v.push_back(i);\n"
              "    }\n"
              "};\n"
              "void f() {\n"
              "    A a;\n"
              "    int i = 0;\n"
              "    a.add(&i);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    std::vector<int*> v;\n"
              "    void add(int* i) {\n"
              "        v.push_back(i);\n"
              "    }\n"
              "    void f() {\n"
              "        A a;\n"
              "        int i = 0;\n"
              "        a.add(&i);\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    int i;\n"
              "    {\n"
              "        std::vector<int> vec;\n"
              "        const auto iter = vec.begin();\n"
              "        i = (int)(iter - vec.begin());\n"
              "    }\n"
              "    return i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int* get(std::vector<int>& container) {\n"
              "    Sequence seq(container);\n"
              "    for (auto& r : seq) {\n"
              "        return &r;\n"
              "    }\n"
              "    return &*seq.begin();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(std::string Str, int first, int last) {\n"
              "    return { Str.begin() + first, Str.begin() + last + 1 };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(std::string s) {\n"
              "    std::string r = { s.begin(), s.end() };\n"
              "    return r;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    std::vector<std::unique_ptr<int>> mA;\n"
              "    void f(std::unique_ptr<int> a) {\n"
              "        auto x = a.get();\n"
              "        mA.push_back(std::move(a));\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    std::map<std::string, int> m;\n"
              "    int* f(std::string s) {\n"
              "        auto r = m.emplace(name, name);\n"
              "        return &(r.first->second);\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "	std::queue<int> q;\n"
              "	auto& h = q.emplace();\n"
              "    h = 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(std::string s) {\n"
              "    std::string ss = (\":\" + s).c_str();\n"
              "    return ss;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #12568
              "    std::string str;\n"
              "    {\n"
              "        std::unique_ptr<char[]> b(new char[1]{});\n"
              "        str.assign(b.get());\n"
              "    }\n"
              "    std::cerr << str;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("auto f() {\n" // #11420
              "    std::vector<int> x;\n"
              "    std::vector<int>::iterator it = x.begin();\n"
              "    return it;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:44] -> [test.cpp:2:22] -> [test.cpp:4:12]: (error) Returning iterator to local container 'x' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());
    }

    void danglingLifetimeContainerView()
    {
        check("std::string_view f() {\n"
              "    std::string s = \"\";\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:2:17] -> [test.cpp:3:12]: (error) Returning object that points to local variable 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::string_view f() {\n"
              "    std::string s = \"\";\n"
              "    std::string_view sv = s;\n"
              "    return sv;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:27] -> [test.cpp:2:17] -> [test.cpp:4:12]: (error) Returning object that points to local variable 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::string_view f() {\n"
              "    std::string s = \"\";\n"
              "    return std::string_view{s};\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:29] -> [test.cpp:2:17] -> [test.cpp:3:28]: (error) Returning object that points to local variable 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::string_view f(std::string_view s) {\n"
              "    return s;\n"
              "}\n"
              "std::string_view g() {\n"
              "    std::string s = \"\";\n"
              "    return f(s);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:6:14] -> [test.cpp:6:14] -> [test.cpp:5:17] -> [test.cpp:6:13]: (error) Returning object that points to local variable 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("const char * f() {\n"
              "    std::string s;\n"
              "    std::string_view sv = s;\n"
              "    return sv.begin();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:20] -> [test.cpp:2:17] -> [test.cpp:4:20]: (error) Returning iterator to local container 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("const char * f() {\n"
              "    std::string s;\n"
              "    return std::string_view{s}.begin();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:37] -> [test.cpp:2:17] -> [test.cpp:3:37]: (error) Returning iterator to local container 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("const char * f() {\n"
              "    std::string s;\n"
              "    return std::string_view(s).begin();\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:3] -> [test.cpp:2] -> [test.cpp:3]: (error) Returning iterator to local container 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            "",
            errout_str());

        check("const char * f(std::string_view sv) {\n"
              "    return sv.begin();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("const char * f(std::string s) {\n"
              "    std::string_view sv = s;\n"
              "    return sv.begin();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:20] -> [test.cpp:1:28] -> [test.cpp:3:20]: (error) Returning iterator to local container 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::string_view f(std::string s) {\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:12] -> [test.cpp:1:32] -> [test.cpp:2:12]: (error) Returning object that points to local variable 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("const char * f(const std::string& s) {\n"
              "    std::string_view sv = s;\n"
              "    return sv.begin();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::string_view f(const std::string_view& sv) {\n"
              "    return sv;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #10993
              "    std::string_view v = std::string();\n"
              "    v.data();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:37] -> [test.cpp:2:37] -> [test.cpp:3:5]: (error) Using object that is a temporary. [danglingTemporaryLifetime]\n", errout_str());

        check("std::string convert(std::string_view sv) { return std::string{ sv }; }\n" // #11374
              "auto f() {\n"
              "    std::vector<std::string> v;\n"
              "    v.push_back(convert(\"foo\"));\n"
              "    return v[0];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10532
        check("std::string f(std::string ss) {\n"
              "  std::string_view sv = true ? \"\" : ss;\n"
              "  std::string s = sv;\n"
              "  return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:30] -> [test.cpp:2:30] -> [test.cpp:3:17]: (error) Using object that is a temporary. [danglingTemporaryLifetime]\n",
                      errout_str());

        // #10833
        check("struct A { std::string s; };\n"
              "const std::string& f(A* a) {\n"
              "    return a ? a->s : \"\";\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("std::span<int> f() {\n"
              "    std::vector<int> v{};\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:2:22] -> [test.cpp:3:12]: (error) Returning object that points to local variable 'v' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::span<int> f() {\n"
              "    std::vector<int> v;\n"
              "    std::span sp = v;\n"
              "    return sp;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:20] -> [test.cpp:2:22] -> [test.cpp:4:12]: (error) Returning object that points to local variable 'v' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::span<int> f() {\n"
              "    std::vector<int> v;\n"
              "    return std::span{v};\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:22] -> [test.cpp:2:22] -> [test.cpp:3:21]: (error) Returning object that points to local variable 'v' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("int f() {\n"
              "    std::span<int> s;\n"
              "    {\n"
              "        std::vector<int> v(1);"
              "        s = v;\n"
              "    }\n"
              "return s.back()\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:43] -> [test.cpp:4:26] -> [test.cpp:6:8]: (error) Using object that points to local variable 'v' that is out of scope. [invalidLifetime]\n",
            errout_str());

        check("int f() {\n"
              "    std::span<int> s;\n"
              "    {\n"
              "        std::vector<int> v(1);"
              "        s = v;\n"
              "    }\n"
              "return s.back()\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:43] -> [test.cpp:4:26] -> [test.cpp:6:8]: (error) Using object that points to local variable 'v' that is out of scope. [invalidLifetime]\n",
            errout_str());

        check("int f() {\n"
              "    std::span<int> s;\n"
              "    {\n"
              "        std::vector<int> v(1);"
              "        s = v;\n"
              "    }\n"
              "return s.front()\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:43] -> [test.cpp:4:26] -> [test.cpp:6:8]: (error) Using object that points to local variable 'v' that is out of scope. [invalidLifetime]\n",
            errout_str());

        check("int f() {\n"
              "    std::span<int> s;\n"
              "    {\n"
              "        std::vector<int> v(1);"
              "        s = v;\n"
              "    }\n"
              "return s.last(1)\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:43] -> [test.cpp:4:26] -> [test.cpp:6:8]: (error) Using object that points to local variable 'v' that is out of scope. [invalidLifetime]\n",
            errout_str());

        check("int f() {\n"
              "    std::span<int> s;\n"
              "    {\n"
              "        std::vector<int> v(1);"
              "        s = v;\n"
              "    }\n"
              "return s.first(1)\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:43] -> [test.cpp:4:26] -> [test.cpp:6:8]: (error) Using object that points to local variable 'v' that is out of scope. [invalidLifetime]\n",
            errout_str());

        check("std::span<const int> f() {\n" // #12966
              "    int a[10]{};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:2:9] -> [test.cpp:3:12]: (error) Returning pointer to local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::string_view f() {\n" // #10995
              "    char a[10]{};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:2:10] -> [test.cpp:3:12]: (error) Returning pointer to local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::string_view f() {\n" // #14465
              "    std::vector<std::string> v;\n"
              "    return *v.begin();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:3:12]: (error) Returning object that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());
    }

    void danglingLifetimeUniquePtr()
    {
        check("int* f(std::unique_ptr<int> p) {\n"
              "    int * rp = p.get();\n"
              "    return rp;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:21] -> [test.cpp:1:29] -> [test.cpp:3:12]: (error) Returning pointer to local variable 'p' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("int* f();\n" // #11406
              "bool g() {\n"
              "    std::unique_ptr<int> ptr(f());\n"
              "    return ptr.get();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int* f();\n"
              "int* g() {\n"
              "    std::unique_ptr<int> ptr(f());\n"
              "    return ptr.get();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:19] -> [test.cpp:3:26] -> [test.cpp:4:19]: (error) Returning object that points to local variable 'ptr' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        // #12600
        check("struct S { std::unique_ptr<int> p; };\n"
              "int* f(const S* s) {\n"
              "    return s[0].p.get();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }
    void danglingLifetime() {
        check("auto f() {\n"
              "    std::vector<int> a;\n"
              "    auto it = a.begin();\n"
              "    return [=](){ return it; };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:22] -> [test.cpp:4:26] -> [test.cpp:2:22] -> [test.cpp:4:12]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("auto f(std::vector<int> a) {\n"
              "    auto it = a.begin();\n"
              "    return [=](){ return it; };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22] -> [test.cpp:3:26] -> [test.cpp:1:25] -> [test.cpp:3:12]: (error) Returning lambda that captures local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n", errout_str());

        check("struct e {};\n"
              "e * j() {\n"
              "    e c[20];\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:12] -> [test.cpp:3:7] -> [test.cpp:4:12]: (error) Returning pointer to local variable 'c' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("auto f(std::vector<int>& a) {\n"
              "    auto it = a.begin();\n"
              "    return [=](){ return it; };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int * f(int a[]) {\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    struct b {\n"
              "        uint32_t f[6];\n"
              "    } d;\n"
              "    uint32_t *a = d.f;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Don't decay std::array
        check("std::array<char, 1> f() {\n"
              "    std::array<char, 1> x;\n"
              "    return x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Make sure we don't hang
        check("struct A;\n"
              "void f() {\n"
              "    using T = A[3];\n"
              "    A &&a = T{1, 2, 3}[1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Make sure we don't hang
        check("struct A;\n"
              "void f() {\n"
              "    using T = A[3];\n"
              "    A &&a = T{1, 2, 3}[1]();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Make sure we don't hang
        check("struct A;\n"
              "void f() {\n"
              "    using T = A[3];\n"
              "    A &&a = T{1, 2, 3}[1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Make sure we don't hang
        check("struct A;\n"
              "void f() {\n"
              "    using T = A[3];\n"
              "    A &&a = T{1, 2, 3}[1]();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Crash #8872
        check("struct a {\n"
              "  void operator()(b c) override {\n"
              "    d(c, [&] { c->e });\n"
              "  }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct a {\n"
              "  void operator()(b c) override {\n"
              "    d(c, [=] { c->e });\n"
              "  }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("struct a {\n"
              "    a(char* b) {}\n"
              "};\n"
              "a f() {\n"
              "    char c[20];\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct a {\n"
              "    a(char* b) {}\n"
              "};\n"
              "a g() {\n"
              "    char c[20];\n"
              "    a d = c;\n"
              "    return d;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    struct a {\n"
              "        std::vector<int> v;\n"
              "        auto g() { return v.end(); }\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int * f(std::vector<int>& v) {\n"
              "    for(int & x : v)\n"
              "        return &x;\n"
              "    return nullptr;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9275
        check("struct S {\n"
              "   void f();\n"
              "   std::string m;\n"
              "}\n"
              "void S::f() {\n"
              "    char buf[1024];\n"
              "    const char* msg = buf;\n"
              "    m = msg;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9201
        check("int* f() {\n"
              "    struct a { int m; };\n"
              "    static a b{0};\n"
              "    return &b.m;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9453
        check("int *ptr;\n"
              "void foo(int arr[]) {\n"
              "    ptr = &arr[2];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9639
        check("struct Fred {\n"
              "    std::string s;\n"
              "};\n"
              "const Fred &getFred();\n"
              "const char * f() {\n"
              "  const Fred &fred = getFred();\n"
              "  return fred.s.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9534
        check("struct A {\n"
              "    int* x;\n"
              "};\n"
              "int* f(int i, std::vector<A>& v) {\n"
              "    A& y = v[i];\n"
              "    return &y.x[i];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9712
        check("std::string f(const char *str) {\n"
              "    char value[256];\n"
              "    return value;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9770
        check("class C {\n"
              "  std::string f(const char*);\n"
              "};\n"
              "std::string C::f(const char*) {\n"
              "  const char data[] = \"x\";\n"
              "  return data;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9899
        check("struct A {\n"
              "    std::vector<int> v;\n"
              "    void f(std::vector<int> w) {\n"
              "        v = std::move(w);\n"
              "    }\n"
              "    void g(std::vector<int> w) {\n"
              "        f(std::move(w));\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        //Make sure we can still take the address of a reference without warning
        check("int* foo() {\n"
              "  int& x = getX();\n"
              "  return &x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
        check("struct C {\n"
              "  int* m_x;\n"
              "  void foo() {\n"
              "    const int& x = getX();\n"
              "    m_x = &x;\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10090
        check("struct a {\n"
              "    int b{};\n"
              "};\n"
              "struct c {\n"
              "    int* c{};\n"
              "    a* d{};\n"
              "};\n"
              "a* f();\n"
              "c g() {\n"
              "    c e;\n"
              "    e.d = f();\n"
              "    if (e.d)\n"
              "        e.c = &e.d->b;\n"
              "    return e;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10214
        check("struct A {\n"
              "  std::string key;\n"
              "  const char *value;\n"
              "};\n"
              "const char *f(const std::string &key, const std::vector<A> &lookup) {\n"
              "  const auto &entry =\n"
              "      std::find_if(lookup.begin(), lookup.end(),\n"
              "                   [key](const auto &v) { return v.key == key; });\n"
              "  return (entry == lookup.end()) ? \"\" : entry->value;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #9811
        check("struct Base {\n"
              "    virtual auto get() -> int & = 0;\n"
              "};\n"
              "struct A : public Base {\n"
              "    int z = 42;\n"
              "    auto get() -> int & override { return z; }\n"
              "    auto getMore() -> int & { return get(); }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #10575
        check("struct Data {\n"
              "    int x=0;\n"
              "    int y=0;\n"
              "};\n"
              "struct MoreData {\n"
              "    Data *data1;\n"
              "};\n"
              "struct Fred {\n"
              "    Fred() {\n"
              "        Data data;\n"
              "        mMoreData.data1 = &data;\n"
              "    }\n"
              "    MoreData mMoreData;\n"
              "};\n");
        ASSERT_EQUALS(
            "[test.cpp:11:27] -> [test.cpp:10:14] -> [test.cpp:11:18]: (error) Non-local variable 'mMoreData.data1' will use pointer to local variable 'data'. [danglingLifetime]\n",
            errout_str());

        // #10784
        check("template <class... Ts>\n"
              "auto f(int i, Ts&... xs) {\n"
              "    return std::tie(xs[i]...);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11362
        check("int* f() {\n"
              "    static struct { int x; } a[] = { { 1 } };\n"
              "    return &a[0].x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11666
        check("template <class T, int M, int N>\n"
              "struct Matrix;\n"
              "template <class T, int N>\n"
              "struct Matrix<T, 1, N> {\n"
              "    std::array<T, N> x;\n"
              "private:\n"
              "    static constexpr decltype(&Matrix::x) members[] = {&Matrix::x};\n"
              "};\n"
              "template <class T, int N>\n"
              "struct Matrix<T, 2, N> {\n"
              "    std::array<T, N> x;\n"
              "    std::array<T, N> y;\n"
              "private:\n"
              "    static constexpr decltype(&Matrix::x) members[] = {&Matrix::x, &Matrix::y};\n"
              "};\n"
              "template <class T>\n"
              "Matrix<T, 2, 2> O() {\n"
              "    return { {}, {} };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11729
        check("struct T {\n"
              "    void add(int* i) {\n"
              "        v.push_back(i);\n"
              "    }\n"
              "    void f() {\n"
              "        static int val = 1;\n"
              "        add(&val);\n"
              "    }\n"
              "    std::vector<int*> v;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #13560
        check("void f() {\n"
              "    int a[] = { 1 };\n"
              "    static int* p = nullptr;\n"
              "    if (!p) {\n"
              "        p = &a[0];\n"
              "    }\n"
              "    *p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:13] -> [test.cpp:2:9] -> [test.cpp:7:6]: (error) Static variable 'p' will use pointer to local variable 'a'. [danglingLifetime]\n", errout_str());

        // #10902
        check("void f() {\n"
              "    static int* x;\n"
              "    int y;\n"
              "    x = &y;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:9] -> [test.cpp:3:9] -> [test.cpp:4:5]: (error) Static variable 'x' will use pointer to local variable 'y'. [danglingLifetime]\n", errout_str());
    }

    void danglingLifetimeFunction() {
        check("auto f() {\n"
              "    int a;\n"
              "    return std::ref(a);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:21] -> [test.cpp:2:9] -> [test.cpp:3:20]: (error) Returning object that points to local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("auto f() {\n"
              "    int a;\n"
              "    return std::make_tuple(std::ref(a));\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:37] -> [test.cpp:3:36] -> [test.cpp:2:9] -> [test.cpp:3:27]: (error) Returning object that points to local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("template<class T>\n"
              "auto by_value(T x) {\n"
              "    return [=] { return x; };\n"
              "}\n"
              "auto g() {\n"
              "    std::vector<int> v;\n"
              "    return by_value(v.begin());\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:7:28] -> [test.cpp:7:28] -> [test.cpp:3:12] -> [test.cpp:3:25] -> [test.cpp:6:22] -> [test.cpp:7:20]: (error) Returning object that points to local variable 'v' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("template<class T>\n"
              "auto by_value(const T& x) {\n"
              "    return [=] { return x; };\n"
              "}\n"
              "auto g() {\n"
              "    std::vector<int> v;\n"
              "    return by_value(v.begin());\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:7:28] -> [test.cpp:7:28] -> [test.cpp:3:12] -> [test.cpp:3:25] -> [test.cpp:6:22] -> [test.cpp:7:20]: (error) Returning object that points to local variable 'v' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("auto by_ref(int& x) {\n"
              "    return [&] { return x; };\n"
              "}\n"
              "auto f() {\n"
              "    int i = 0;\n"
              "    return by_ref(i);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:12] -> [test.cpp:1:19] -> [test.cpp:2:25] -> [test.cpp:6:19] -> [test.cpp:5:9] -> [test.cpp:6:18]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("auto by_ref(const int& x) {\n"
              "    return [=] { return x; };\n"
              "}\n"
              "auto f() {\n"
              "    int i = 0;\n"
              "    return by_ref(i);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("auto f(int x) {\n"
              "    int a;\n"
              "    std::tie(a) = x;\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::pair<std::string, std::string>\n"
              "str_pair(std::string const & a, std::string const & b) {\n"
              "    return std::make_pair(a, b);\n"
              "}\n"
              "std::vector<std::pair<std::string, std::string> > create_parameters() {\n"
              "    std::vector<std::pair<std::string, std::string> > par;\n"
              "    par.push_back(str_pair(\"param1\", \"prop_a\"));\n"
              "    par.push_back(str_pair(\"param2\", \"prop_b\"));\n"
              "    par.push_back(str_pair(\"param3\", \"prop_c\"));\n"
              "    return par;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int* (*ptr)(const int*); };\n" // #13057
              "int* f(S* s) {\n"
              "    int x = 0;\n"
              "    return s->ptr(&x);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void danglingLifetimeUserConstructor()
    {
        check("struct A {\n"
              "    int* i;\n"
              "    A(int& x)\n"
              "    : i(&x)\n"
              "    {}\n"
              "};\n"
              "A f() {\n"
              "    int i = 0;\n"
              "    A a{i};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:9] -> [test.cpp:8:9] -> [test.cpp:10:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    int* i;\n"
              "    A(int& x);\n"
              "};\n"
              "A f() {\n"
              "    int i = 0;\n"
              "    A a{i};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:7:9] -> [test.cpp:6:9] -> [test.cpp:8:12]: (error, inconclusive) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    int* i;\n"
              "    A(const int& x);\n"
              "};\n"
              "A f() {\n"
              "    int i = 0;\n"
              "    A a{i};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int& i;\n"
              "    A(int& x)\n"
              "    : i(x)\n"
              "    {}\n"
              "};\n"
              "A f() {\n"
              "    int i = 0;\n"
              "    A a{i};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:9] -> [test.cpp:8:9] -> [test.cpp:10:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    int& i;\n"
              "    A(const std::vector<int>& x)\n"
              "    : i(x[0])\n"
              "    {}\n"
              "};\n"
              "A f() {\n"
              "    std::vector<int> v = {0};\n"
              "    A a{v};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:9] -> [test.cpp:8:22] -> [test.cpp:10:12]: (error) Returning object that points to local variable 'v' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    int* i;\n"
              "    A(const std::vector<int>& x)\n"
              "    : i(x.data())\n"
              "    {}\n"
              "};\n"
              "A f() {\n"
              "    std::vector<int> v = {0};\n"
              "    A a{v};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:9] -> [test.cpp:8:22] -> [test.cpp:10:12]: (error) Returning object that points to local variable 'v' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    const int* i;\n"
              "    A(const int& x)\n"
              "    : i(&x)\n"
              "    {}\n"
              "};\n"
              "A f() {\n"
              "    A a{0};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:9] -> [test.cpp:9:12]: (error) Returning object that will be invalid when returning. [returnDanglingLifetime]\n",
                      errout_str());

        check("struct A {\n"
              "    const int* i;\n"
              "    A(const int& x)\n"
              "    : i(&x)\n"
              "    {}\n"
              "};\n"
              "A f() {\n"
              "    int i = 0;\n"
              "    return A{i};\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:14] -> [test.cpp:8:9] -> [test.cpp:9:13]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    std::string v;\n"
              "    A(const std::string& s)\n"
              "    : v(s)\n"
              "    {}\n"
              "};\n"
              "A f() {\n"
              "    std::string s;\n"
              "    return A{s};\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    std::string_view v;\n"
              "    A(const std::string& s)\n"
              "    : v(s)\n"
              "    {}\n"
              "};\n"
              "A f() {\n"
              "    std::string s;\n"
              "    return A{s};\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:14] -> [test.cpp:8:17] -> [test.cpp:9:13]: (error) Returning object that points to local variable 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    const int* i;\n"
              "    A(const int& x)\n"
              "    : i(&x)\n"
              "    {}\n"
              "};\n"
              "A f() {\n"
              "    return A{0};\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:14] -> [test.cpp:8:13]: (error) Returning object that will be invalid when returning. [returnDanglingLifetime]\n",
                      errout_str());

        check("struct A {\n"
              "    int n;\n"
              "    A(const int &x) : n(x) {}\n"
              "};\n"
              "A f() {\n"
              "    A m(4);\n"
              "    return m;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct B {};\n"
              "struct A {\n"
              "    B n;\n"
              "    A(const B &x) : n(x) {}\n"
              "};\n"
              "A f() {\n"
              "    A m(B{});\n"
              "    return m;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    A(std::vector<std::string> &filenames)\n"
              "    : files(filenames) {}\n"
              "    std::vector<std::string> &files;\n"
              "};\n"
              "A f() {\n"
              "    std::vector<std::string> files;\n"
              "    return A(files);\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:8:14] -> [test.cpp:7:30] -> [test.cpp:8:13]: (error) Returning object that points to local variable 'files' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct S {\n"
              "    explicit S(std::string& s);\n"
              "}\n"
              "S f() {\n"
              "    std::string m(\"abc\");\n"
              "    return S(m);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    std::string msg;\n"
              "    explicit S(const char* m) : msg(m) {}\n"
              "};\n"
              "S f() {\n"
              "    std::string s(\"abc\");\n"
              "    return S(s.c_str());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    explicit S(const char* p) { m = p; }\n"
              "    void g();\n"
              "    std::string m;\n"
              "    int* t{};\n"
              "};\n"
              "void f(const std::stringstream& buffer) {\n"
              "    S s(buffer.str().c_str());\n"
              "    s.g();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void danglingLifetimeAggregateConstructor() {
        check("struct A {\n"
              "    const int& x;\n"
              "    int y;\n"
              "};\n"
              "A f() {\n"
              "    int i = 0;\n"
              "    return A{i, i};\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:7:14] -> [test.cpp:6:9] -> [test.cpp:7:13]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    const int& x;\n"
              "    int y;\n"
              "};\n"
              "A f() {\n"
              "    int i = 0;\n"
              "    return {i, i};\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:7:13] -> [test.cpp:6:9] -> [test.cpp:7:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    const int& x;\n"
              "    int y;\n"
              "};\n"
              "A f() {\n"
              "    int i = 0;\n"
              "    A r{i, i};\n"
              "    return r;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:7:9] -> [test.cpp:6:9] -> [test.cpp:8:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    const int& x;\n"
              "    int y;\n"
              "};\n"
              "A f() {\n"
              "    int i = 0;\n"
              "    A r = {i, i};\n"
              "    return r;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:7:12] -> [test.cpp:6:9] -> [test.cpp:8:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    const int& x;\n"
              "    int y;\n"
              "};\n"
              "A f(int& x) {\n"
              "    int i = 0;\n"
              "    return A{i, x};\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:7:14] -> [test.cpp:6:9] -> [test.cpp:7:13]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    const int& x;\n"
              "    int y;\n"
              "};\n"
              "A f(int& x) {\n"
              "    int i = 0;\n"
              "    return A{x, i};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    const int& x;\n"
              "    int y;\n"
              "};\n"
              "A f(int& x) {\n"
              "    return A{x, x};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int i; const int& j; };\n"
              "A f(int& x) {\n"
              "    int y = 0;\n"
              "    return A{y, x};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct a {\n"
              "    std::string m;\n"
              "};\n"
              "a f() {\n"
              "    std::array<char, 1024> m {};\n"
              "    return { m.data() };\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { std::string s; };\n" // #13167
              "std::vector<S> f() {\n"
              "    std::vector<S> v;\n"
              "    {\n"
              "        std::string a{ \"abc\" };\n"
              "        v.push_back({ a.c_str() });\n"
              "    }\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { std::string_view sv; };\n"
              "std::vector<S> f() {\n"
              "    std::vector<S> v;\n"
              "    {\n"
              "        std::string a{ \"abc\" };\n"
              "        v.push_back({ a.c_str() });\n"
              "    }\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:6:30] -> [test.cpp:6:30] -> [test.cpp:6:21] -> [test.cpp:5:21] -> [test.cpp:8:12]: (error) Returning object that points to local variable 'a' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct S { const std::string& r; };\n" // #14209
              "S f() {\n"
              "    std::string s;\n"
              "    return S{ s };\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:15] -> [test.cpp:3:17] -> [test.cpp:4:13]: (error) Returning object that points to local variable 's' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A { int& x; };\n" // #14247
              "A f() {\n"
              "    int x = 0;\n"
              "    A a{.x = x};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:14] -> [test.cpp:3:9] -> [test.cpp:5:12]: (error) Returning object that points to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A { int x; int& r};\n"
              "A f(int& r) {\n"
              "    int x = 0;\n"
              "    A a{.x = x, .r = r};\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int& x; };\n"
              "A f() {\n"
              "    int x = 0;\n"
              "    A a{ .x{x} };\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:13] -> [test.cpp:3:9] -> [test.cpp:5:12]: (error) Returning object that points to local variable 'x' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A { int x; int& r};\n"
              "A f(int& r) {\n"
              "    int x = 0;\n"
              "    A a{ .x{x}, .r{r} };\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; bool b; };\n" // #14260
              "void f() {\n"
              "    struct S s = { .i = 0, true };\n"
              "}\n"); // don't crash
        ASSERT_EQUALS("", errout_str());
    }

    void danglingLifetimeInitList() {
        check("std::vector<int*> f() {\n"
              "    int i = 0;\n"
              "    std::vector<int*> v = {&i, &i};\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:28] -> [test.cpp:3:28] -> [test.cpp:2:9] -> [test.cpp:4:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n"
            "[test.cpp:3:32] -> [test.cpp:3:32] -> [test.cpp:2:9] -> [test.cpp:4:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::vector<int*> f() {\n"
              "    int i = 0;\n"
              "    std::vector<int*> v{&i, &i};\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:25] -> [test.cpp:3:25] -> [test.cpp:2:9] -> [test.cpp:4:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n"
            "[test.cpp:3:29] -> [test.cpp:3:29] -> [test.cpp:2:9] -> [test.cpp:4:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::vector<int*> f() {\n"
              "    int i = 0;\n"
              "    return {&i, &i};\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:13] -> [test.cpp:3:13] -> [test.cpp:2:9] -> [test.cpp:3:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n"
            "[test.cpp:3:17] -> [test.cpp:3:17] -> [test.cpp:2:9] -> [test.cpp:3:12]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("std::vector<int*> f(int& x) {\n"
              "    return {&x, &x};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<std::string> f() {\n"
              "    std::set<std::string> x;\n"
              "    x.insert(\"1\");\n"
              "    x.insert(\"2\");\n"
              "    return { x.begin(), x.end() };\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void danglingLifetimeImplicitConversion() {
        check("struct A { A(const char *a); };\n"
              "A f() {\n"
              "   std::string ba(\"hello\");\n"
              "   return ba.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { A(const char *a); };\n"
              "A f() {\n"
              "   std::string ba(\"hello\");\n"
              "   A bp = ba.c_str();\n"
              "   return bp;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { A(const char *a); };\n"
              "std::vector<A> f() {\n"
              "   std::string ba(\"hello\");\n"
              "   std::vector<A> v;\n"
              "   v.push_back(ba.c_str());\n"
              "   return v;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(const std::string& x) {\n"
              "  const char c[] = \"\";\n"
              "  if (!x.empty())\n"
              "    return x + c;\n"
              "  return \"\";\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(const std::string& x) {\n"
              "  const char c[] = \"123\";\n"
              "  if (!x.empty())\n"
              "    return c + 1;\n"
              "  return \"\";\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void danglingTemporaryLifetime() {
        check("struct C {\n" // #11091
              "    C(C& rhs);\n"
              "    explicit C(const S& n, const S& p = {});\n"
              "    bool f() const;\n"
              "    S m;\n"
              "};\n"
              "void f() {\n"
              "    C c(\"\");\n"
              "    while (c.f()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("const int& g(const int& x) {\n"
              "    return x;\n"
              "}\n"
              "void f(int& i) {\n"
              "    int* x = &g(0);\n"
              "    i += *x;\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:1:26] -> [test.cpp:2:12] -> [test.cpp:5:15] -> [test.cpp:5:14] -> [test.cpp:5:17] -> [test.cpp:6:11]: (error) Using pointer that is a temporary. [danglingTemporaryLifetime]\n",
            errout_str());

        check("QString f() {\n"
              "    QString a(\"dummyValue\");\n"
              "    const char* b = a.toStdString().c_str();\n"
              "    QString c = b;\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:42] -> [test.cpp:3:34] -> [test.cpp:4:15]: (error) Using pointer that is a temporary. [danglingTemporaryLifetime]\n",
                      errout_str());

        check("auto f(std::string s) {\n"
              "    const char *x = s.substr(1,2).c_str();\n"
              "    auto i = s.substr(4,5).begin();\n"
              "    return *i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:33] -> [test.cpp:3:22] -> [test.cpp:4:13]: (error) Using iterator that is a temporary. [danglingTemporaryLifetime]\n",
                      errout_str());

        check("std::string f() {\n"
              "    std::stringstream tmp;\n"
              "    const std::string &str = tmp.str();\n"
              "    return std::string(str.c_str(), 1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int get_value();\n"
              "const int &get_reference1() {\n"
              "  const int &x = get_value();\n"
              "  return x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:16] -> [test.cpp:4:10]: (error) Reference to temporary returned. [returnTempReference]\n", errout_str());

        check("int get_value();\n"
              "const int &get_reference2() {\n"
              "  const int &x1 = get_value();\n"
              "  const int &x2 = x1;\n"
              "  return x2;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:17] -> [test.cpp:3:17] -> [test.cpp:5:10]: (error) Reference to temporary returned. [returnTempReference]\n",
                      errout_str());

        check("const std::string& getState() {\n"
              "    static const std::string& state = \"\";\n"
              "    return state;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct var {\n"
              "    void fun();\n"
              "}x;\n"
              "var* T(const char*) {\n"
              "    return &x;\n"
              "}\n"
              "std::string GetTemp();\n"
              "void f() {\n"
              "    auto a = T(GetTemp().c_str());\n"
              "    a->fun();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    std::map<int, int> m_;\n"
              "};\n"
              "struct B {\n"
              "    A a_;\n"
              "};\n"
              "B func();\n"
              "void f() {\n"
              "    const std::map<int, int>::iterator& m = func().a_.m_.begin();\n"
              "    (void)m->first;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9:63] -> [test.cpp:9:49] -> [test.cpp:10:11]: (error) Using iterator that is a temporary. [danglingTemporaryLifetime]\n",
                      errout_str());

        check("struct A {\n" // #14054
              "    std::map<int, int> m_;\n"
              "};\n"
              "struct B {\n"
              "    A a_;\n"
              "};\n"
              "B func();\n"
              "void f() {\n"
              "    const std::map<int, int>::iterator m = func().a_.m_.begin();\n"
              "    (void)m->first;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9:62] -> [test.cpp:9:48] -> [test.cpp:10:11]: (error) Using iterator that is a temporary. [danglingTemporaryLifetime]\n",
                      errout_str());

        check("void f(bool b) {\n"
              "  std::vector<int> ints = g();\n"
              "  auto *ptr = &ints;\n"
              "  if (b)\n"
              "    ptr = &ints;\n"
              "  for (auto it = ptr->begin(); it != ptr->end(); ++it)\n"
              "  {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct String {\n" // #10469
              "    void Append(uint8_t Val);\n"
              "    String& operator+=(const char s[]);\n"
              "    String& operator+=(const std::string& Str) {\n"
              "        return operator+=(Str.c_str());\n"
              "    }\n"
              "    void operator+=(uint8_t Val) {\n"
              "        Append(Val);\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #11057
        check("struct S {\n"
              "    int& r;\n"
              "};\n"
              "void f(int i) {\n"
              "    const S a[] = { { i } };\n"
              "    for (const auto& s : a) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<char*> f(const std::vector<std::string>& args) {\n" // #9773
              "    std::vector<char*> cargs;\n"
              "    for (const auto& a : args) {\n"
              "      cargs.push_back(const_cast<char*>(a.data()));\n"
              "    }\n"
              "    return cargs;\n"
              "}\n"
              "void g() {\n"
              "    std::vector<char*> cargs = f({ \"0\", \"0\" });\n"
              "    (void)cargs;\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:6:12] -> [test.cpp:4:47] -> [test.cpp:3:22] -> [test.cpp:1:58] -> [test.cpp:4:40] -> [test.cpp:9:34] -> [test.cpp:9:34] -> [test.cpp:10:11]: (error) Using object that is a temporary. [danglingTemporaryLifetime]\n", errout_str());

        check("struct C {\n" // #9194
              "    const int& m;\n"
              "    C(const int& i) : m(i) {}\n"
              "    int get() { return m; }\n"
              "};\n"
              "int f() {\n"
              "    C c(42);\n"
              "    return c.get();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:9] -> [test.cpp:7:9] -> [test.cpp:8:12]: (error) Using object that is a temporary. [danglingTemporaryLifetime]\n", errout_str());

        // #11298
        check("struct S {\n"
              "    std::string g();  \n"
              "};\n"
              "struct T {\n"
              "  void f(); \n"
              "  S* p = nullptr;\n"
              "};\n"
              "struct U {\n"
              "  explicit U(const char* s);\n"
              "  bool h();\n"
              "  int i;\n"
              "};\n"
              "void T::f() {\n"
              "    U u(p->g().c_str());\n"
              "    if (u.h()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11442
        check("const std::string& f(const P< std::string >& value) {\n"
              "   static const std::string empty;\n"
              "   return value.get() == nullptr ? empty : *value;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11472
        check("namespace N {\n"
              "    struct T { int m; };\n"
              "    int i;\n"
              "    const T& f(const T* p) {\n"
              "        return p != nullptr ? *p : *reinterpret_cast<const ::N::T*>(&i);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11609
        check("struct S {\n"
              "    void f(const std::string& s) {\n"
              "        auto it = m.find(s.substr(1,4));\n"
              "        if (it == m.end()) {}\n"
              "    }\n"
              "    std::map<std::string, int> m;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #11628
        check("std::vector<int>* g();\n"
              "void f() {\n"
              "	std::unique_ptr<std::vector<int>> p(g());\n"
              "	if (!p) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #13760
        check("template<class T>\n"
              "auto f(T&& x) {\n"
              "    return [&] {\n"
              "        return x();\n"
              "    };\n"
              "}\n"
              "auto g() {\n"
              "    auto y = f([](auto x) { return 1; });\n"
              "    return y();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12] -> [test.cpp:2:13] -> [test.cpp:4:16] -> [test.cpp:8:16] -> [test.cpp:8:16] -> [test.cpp:9:12]: (error) Using object that is a temporary. [danglingTemporaryLifetime]\n",
            errout_str());
    }

    void danglingLifetimeBorrowedMembers()
    {
        // #10585
        check("struct Info { int k; };\n"
              "struct MoreInfo {\n"
              "    int* k;\n"
              "    char dat;\n"
              "};\n"
              "struct Fields {\n"
              "    Info info;\n"
              "};\n"
              "template <typename T> void func1(T val){}\n"
              "template <typename T> void func2(T val){}\n"
              "Fields* get();\n"
              "void doit() {\n"
              "    MoreInfo rech;\n"
              "    rech.k = &get()->info.k;\n"
              "    func1(&rech.dat);\n"
              "    func2(rech.k);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int x; };\n"
              "A* g();\n"
              "void f() {\n"
              "    A** ap = &g();\n"
              "    (*ap)->x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:14] -> [test.cpp:4:16] -> [test.cpp:5:7]: (error) Using pointer that is a temporary. [danglingTemporaryLifetime]\n",
                      errout_str());

        check("struct A { int* x; };\n"
              "A g();\n"
              "void f() {\n"
              "    int* x = g().x;\n"
              "    (void)*x + 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int x; };\n"
              "struct B { A* a; }\n"
              "B g();\n"
              "void f() {\n"
              "    int* x = &g()->a.x;\n"
              "    (void)*x + 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int x; };\n"
              "struct B { A* g(); };\n"
              "A* g();\n"
              "void f(B b) {\n"
              "    A** ap = &b.g();\n"
              "    (*ap)->x;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:14] -> [test.cpp:5:18] -> [test.cpp:6:7]: (error) Using pointer that is a temporary. [danglingTemporaryLifetime]\n",
                      errout_str());
    }

    void danglingLifetimeClassMemberFunctions()
    {
        check("struct S {\n"
              "    S(int i) : i(i) {}\n"
              "    int i;\n"
              "    int* ptr() { return &i; }\n"
              "};\n"
              "int* fun(int i) { \n"
              "    return S(i).ptr();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:25] -> [test.cpp:4:25] -> [test.cpp:7:13] -> [test.cpp:7:20]: (error) Returning pointer that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct Fred\n"
              "{\n"
              "    int x[2];\n"
              "    Fred() {\n"
              "        x[0] = 0x41;\n"
              "        x[1] = 0x42;\n"
              "    }\n"
              "    const int *get_x() {\n"
              "        return x;\n"
              "    }\n"
              "};\n"
              "static const int *foo() {\n"
              "    Fred fred;\n"
              "    return fred.get_x();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:16] -> [test.cpp:9:16] -> [test.cpp:14:12] -> [test.cpp:13:10] -> [test.cpp:14:22]: (error) Returning pointer to local variable 'fred' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    int i;\n"
              "    auto f() const {\n"
              "        return [=]{ return i; };\n"
              "    }\n"
              "};\n"
              "auto g() {\n"
              "    return A().f();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:16] -> [test.cpp:4:28] -> [test.cpp:8:13] -> [test.cpp:8:17]: (error) Returning object that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    int i;\n"
              "    auto f() const {\n"
              "        return [*this]{ return i; };\n"
              "    }\n"
              "};\n"
              "auto g() {\n"
              "    return A().f();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int* i;\n"
              "    auto f() const {\n"
              "        return [*this]{ return i; };\n"
              "    }\n"
              "};\n"
              "auto g() {\n"
              "    int i = 0;\n"
              "    return A{&i}.f();\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:9:14] -> [test.cpp:9:14] -> [test.cpp:9:13] -> [test.cpp:4:16] -> [test.cpp:4:18] -> [test.cpp:8:9] -> [test.cpp:9:19]: (error) Returning object that points to local variable 'i' that will be invalid when returning. [returnDanglingLifetime]\n",
            errout_str());

        check("struct S {\n"
              "    int i{};\n"
              "};\n"
              "struct T {\n"
              "    S getS() const { return S{ j }; }\n"
              "    int j{};\n"
              "};\n"
              "void f(S* p) {\n"
              "    S ret;\n"
              "    {\n"
              "        T t;\n"
              "        ret = t.getS();\n"
              "    }\n"
              "    *p = ret;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int val=42;\n"
              "    const A& Get() const & {\n"
              "        return *this;\n"
              "    }\n"
              "};\n"
              "const A *ptr;\n"
              "int main() {\n"
              "    ptr = &A().Get();\n"
              "    return ptr->val;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:16] -> [test.cpp:9:16] -> [test.cpp:9:11] -> [test.cpp:9:13] -> [test.cpp:10:12]: (error) Using pointer that is a temporary. [danglingTemporaryLifetime]\n",
            errout_str());

        check("struct A {\n"
              "    int val = 42;\n"
              "    const A& Get() const {\n"
              "        return *this;\n"
              "    }\n"
              "};\n"
              "int main() {\n"
              "    const A& r = A().Get();\n"
              "    return r.val;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:8:16] -> [test.cpp:4:16] -> [test.cpp:8:22] -> [test.cpp:9:12]: (error) Using reference to dangling temporary. [danglingTempReference]\n",
            errout_str());
    }

    void invalidLifetime() {
        check("void foo(int a) {\n"
              "    std::function<void()> f;\n"
              "    if (a > 0) {\n"
              "        int b = a + 1;\n"
              "        f = [&]{ return b; };\n"
              "    }\n"
              "    f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:25] -> [test.cpp:4:13] -> [test.cpp:7:5]: (error) Using lambda that captures local variable 'b' that is out of scope. [invalidLifetime]\n", errout_str());

        check("void f(bool b)  {\n"
              "  int* x;\n"
              "  if(b) {\n"
              "    int y[6] = {0,1,2,3,4,5};\n"
              "    x = y;\n"
              "  }\n"
              "  x[3];\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:5:9] -> [test.cpp:4:9] -> [test.cpp:7:3]: (error) Using pointer to local variable 'y' that is out of scope. [invalidLifetime]\n",
            errout_str());

        check("void foo(int a) {\n"
              "    std::function<void()> f;\n"
              "    if (a > 0) {\n"
              "        int b = a + 1;\n"
              "        f = [&]{ return b; };\n"
              "        f();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct a {\n"
              "  b();\n"
              "  std::list<int> c;\n"
              "};\n"
              "void a::b() {\n"
              "  c.end()\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void b(char f[], char c[]) {\n"
              "  std::string d(c); {\n"
              "    std::string e;\n"
              "    b(f, e.c_str())\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n"
              "    std::string s;\n"
              "    if(b) {\n"
              "        char buf[3];\n"
              "        s = buf;\n"
              "    }\n"
              "    std::cout << s;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int &a[];\n"
              "void b(){int *c = a};");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    int x;\n"
              "};\n"
              "struct B {\n"
              "    std::function<void()> x;\n"
              "    void f() {\n"
              "        this->x = [&] {\n"
              "            B y;\n"
              "            return y.x;\n"
              "        };\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("namespace test {\n"
              "class Foo {};\n"
              "struct Bar {\n"
              "  Foo *_foo;\n"
              "};\n"
              "\n"
              "int f(Bar *bar);\n"
              "\n"
              "void g(Bar *bar) {\n"
              "  {\n"
              "    Foo foo;\n"
              "    bar->_foo = &foo;\n"
              "    bar->_foo = nullptr;\n"
              "  }\n"
              "  f(bar);\n"
              "}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("class Foo {};\n"
              "struct Bar {\n"
              "  Foo *_foo;\n"
              "};\n"
              "\n"
              "int f(Bar *bar);\n"
              "\n"
              "void g(Bar *bar) {\n"
              "  {\n"
              "    Foo foo;\n"
              "    bar->_foo = &foo;\n"
              "    bar->_foo = nullptr;\n"
              "  }\n"
              "  f(bar);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("namespace test {\n"
              "class Foo {};\n"
              "struct Bar {\n"
              "  Foo *_foo;\n"
              "};\n"
              "\n"
              "int f(Bar *bar);\n"
              "\n"
              "void g(Bar *bar) {\n"
              "  {\n"
              "    Foo foo;\n"
              "    bar->_foo = &foo;\n"
              "  }\n"
              "  f(bar);\n"
              "}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:12:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("class Foo {};\n"
              "struct Bar {\n"
              "  Foo *_foo;\n"
              "};\n"
              "\n"
              "int f(Bar *bar);\n"
              "\n"
              "void g(Bar *bar) {\n"
              "  {\n"
              "    Foo foo;\n"
              "    bar->_foo = &foo;\n"
              "  }\n"
              "  f(bar);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:11:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("class Foo {};\n" // #10750
              "struct Bar {\n"
              "  Foo *_foo;\n"
              "};\n"
              "int f(Bar *bar);\n"
              "void g(Bar *bar) {\n"
              "  {\n"
              "    Foo foo;\n"
              "    {\n"
              "      bar->_foo = &foo;\n"
              "    }\n"
              "  }\n"
              "  f(bar);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:10:7]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n", errout_str());

        check("void f(std::string_view text);\n" // #11508
              "void g() {\n"
              "  std::string teststr;\n"
              "  f(teststr);"
              "}\n"
              "void f(std::string_view text) {"
              "  g(text.data());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::span<int> data);\n" // #11508
              "void g() {\n"
              "  std::vector<int> v;\n"
              "  f(v);"
              "}\n"
              "void f(std::span<int> data) {"
              "  g(data.begin());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    int* a[1];\n"
              "    int* b[1][1];\n"
              "};\n"
              "void f(S* s) {\n"
              "    int i = 0;\n"
              "    s->a[0] = &i;\n"
              "}\n"
              "void g(S* s) {\n"
              "    int i = 0;\n"
              "    s->b[0][0] = &i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n"
                      "[test.cpp:11:5]: (error) Address of local auto-variable assigned to a function parameter. [autoVariables]\n",
                      errout_str());
    }

    void deadPointer() {
        check("void f() {\n"
              "  int *p = p1;\n"
              "  if (cond) {\n"
              "    int x;\n"
              "    p = &x;\n"
              "  }\n"
              "  *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:4:9] -> [test.cpp:7:4]: (error) Using pointer to local variable 'x' that is out of scope. [invalidLifetime]\n", errout_str());

        // FP: don't warn in subfunction
        check("void f(struct KEY *key) {\n"
              "  key->x = 0;\n"
              "}\n"
              "\n"
              "int main() {\n"
              "  struct KEY *tmp = 0;\n"
              "  struct KEY k;\n"
              "\n"
              "  if (condition) {\n"
              "    tmp = &k;\n"
              "  } else {\n"
              "  }\n"
              "  f(tmp);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Don't warn about references (#6399)
        check("void f() {\n"
              "    wxAuiToolBarItem* former_hover = NULL;\n"
              "    for (i = 0, count = m_items.GetCount(); i < count; ++i) {\n"
              "        wxAuiToolBarItem& item = m_items.Item(i);\n"
              "        former_hover = &item;\n"
              "    }\n"
              "    if (former_hover != pitem)\n"
              "        dosth();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    wxAuiToolBarItem* former_hover = NULL;\n"
              "    for (i = 0, count = m_items.GetCount(); i < count; ++i) {\n"
              "        wxAuiToolBarItem item = m_items.Item(i);\n"
              "        former_hover = &item;\n"
              "    }\n"
              "    if (former_hover != pitem)\n"
              "        dosth();\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:5:24] -> [test.cpp:3:47] -> [test.cpp:4:26] -> [test.cpp:7:9]: (error) Using pointer to local variable 'item' that is out of scope. [invalidLifetime]\n",
            errout_str());

        // #6575
        check("void trp_deliver_signal()  {\n"
              "    union {\n"
              "        Uint32 theData[25];\n"
              "        EventReport repData;\n"
              "    };\n"
              "    EventReport * rep = &repData;\n"
              "    rep->setEventType(NDB_LE_Connected);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8785
        check("int f(bool a, bool b) {\n"
              "    int *iPtr = 0;\n"
              "    if(b) {\n"
              "        int x = 42;\n"
              "        iPtr = &x;\n"
              "    }\n"
              "    if(b && a)\n"
              "        return *iPtr;\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:16] -> [test.cpp:4:13] -> [test.cpp:8:17]: (error) Using pointer to local variable 'x' that is out of scope. [invalidLifetime]\n", errout_str());

        // #11753
        check("int main(int argc, const char *argv[]) {\n"
              "    const char* s = \"\";\n"
              "    if (argc > 0) {\n"
              "        char buff[32]{};\n"
              "        s = std::strncpy(buff, argv[0], 31);\n"
              "    }\n"
              "    std::cout << s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:26] -> [test.cpp:4:14] -> [test.cpp:7:18]: (error) Using pointer to local variable 'buff' that is out of scope. [invalidLifetime]\n", errout_str());

        check("char* f(char* dst) {\n"
              "    const char* src = \"abc\";\n"
              "    return strcpy(dst, src);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void splitNamespaceAuto() { // #10473
        check("namespace ns\n"
              "{\n"
              "    auto var{ 0 };\n"
              "}\n"
              "namespace ns\n"
              "{\n"
              "    int i;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void incompleteTypeArray() { // #13787
        check("struct S (*a[10]);\n"
              "void f(int i, struct S* p) {\n"
              "    a[i] = &p[i];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str()); // don't crash
    }

};

REGISTER_TEST(TestAutoVariables)
