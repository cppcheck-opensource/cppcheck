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
#include "checkbufferoverrun.h"
#include "ctu.h"
#include "errortypes.h"
#include "helpers.h"
#include "platform.h"
#include "settings.h"
#include "fixture.h"

#include <cstddef>
#include <list>
#include <string>

class TestBufferOverrun : public TestFixture {
public:
    TestBufferOverrun() : TestFixture("TestBufferOverrun") {}

private:
    /*const*/ Settings settings0 = settingsBuilder().library("std.cfg").severity(Severity::warning).severity(Severity::style).build();
    const Settings settings0_i = settingsBuilder(settings0).certainty(Certainty::inconclusive).build();
    const Settings settings0_p = settingsBuilder(settings0).severity(Severity::portability).build();
    const Settings settings1 = settingsBuilder(settings0).severity(Severity::performance).certainty(Certainty::inconclusive).build();

    struct CheckOptions
    {
        bool cpp = true;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        check_(file, line, code, settings0_i, options.cpp);
    }

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const Settings& settings, bool cpp = true) {
        SimpleTokenizer tokenizer(settings, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for buffer overruns..
        runChecks<CheckBufferOverrun>(tokenizer, this);
    }

    // TODO: get rid of this
    void check_(const char* file, int line, const std::string& code) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings0_i, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for buffer overruns..
        runChecks<CheckBufferOverrun>(tokenizer, this);
    }

#define checkP(...) checkP_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkP_(const char* file, int line, const char (&code)[size])
    {
        SimpleTokenizer2 tokenizer(settings1, *this, code, "test.cpp");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check for buffer overruns..
        runChecks<CheckBufferOverrun>(tokenizer, this);
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(noerr1);
        TEST_CASE(noerr2);
        TEST_CASE(noerr3);
        TEST_CASE(noerr4);

        TEST_CASE(sizeof3);

        TEST_CASE(array_index_1);
        TEST_CASE(array_index_2);
        TEST_CASE(array_index_3);
        TEST_CASE(array_index_4);
        TEST_CASE(array_index_6);
        TEST_CASE(array_index_7);
        TEST_CASE(array_index_11);
        TEST_CASE(array_index_12);
        TEST_CASE(array_index_13);
        TEST_CASE(array_index_14);
        TEST_CASE(array_index_15);
        TEST_CASE(array_index_16);
        TEST_CASE(array_index_17);
        TEST_CASE(array_index_18);
        TEST_CASE(array_index_19);
        TEST_CASE(array_index_20);
        TEST_CASE(array_index_21);
        TEST_CASE(array_index_22);
        TEST_CASE(array_index_23);
        TEST_CASE(array_index_24); // ticket #1492 and #1539
        TEST_CASE(array_index_25); // ticket #1536
        TEST_CASE(array_index_26);
        TEST_CASE(array_index_27);
        TEST_CASE(array_index_28); // ticket #1418
        TEST_CASE(array_index_29); // ticket #1734
        TEST_CASE(array_index_30); // ticket #2086 - out of bounds when type is unknown
        TEST_CASE(array_index_31); // ticket #2120 - out of bounds in subfunction when type is unknown
        TEST_CASE(array_index_32);
        TEST_CASE(array_index_33); // ticket #3044
        TEST_CASE(array_index_34); // ticket #3063
        TEST_CASE(array_index_35); // ticket #2889
        TEST_CASE(array_index_36); // ticket #2960
        TEST_CASE(array_index_37);
        TEST_CASE(array_index_38); // ticket #3273
        TEST_CASE(array_index_39);
        TEST_CASE(array_index_40); // loop variable calculation, taking address
        TEST_CASE(array_index_41); // structs with the same name
        TEST_CASE(array_index_42);
        TEST_CASE(array_index_43); // struct with array
        TEST_CASE(array_index_44); // #3979
        TEST_CASE(array_index_45); // #4207 - calling function with variable number of parameters (...)
        TEST_CASE(array_index_46); // #4840 - two-statement for loop
        TEST_CASE(array_index_47); // #5849
        TEST_CASE(array_index_48); // #9478
        TEST_CASE(array_index_49); // #8653
        TEST_CASE(array_index_50);
        TEST_CASE(array_index_51); // #3763
        TEST_CASE(array_index_52); // #7682
        TEST_CASE(array_index_53); // #4750
        TEST_CASE(array_index_54); // #10268
        TEST_CASE(array_index_55); // #10254
        TEST_CASE(array_index_56); // #10284
        TEST_CASE(array_index_57); // #10023
        TEST_CASE(array_index_58); // #7524
        TEST_CASE(array_index_59); // #10413
        TEST_CASE(array_index_60); // #10617, #9824
        TEST_CASE(array_index_61); // #10621
        TEST_CASE(array_index_62); // #7684
        TEST_CASE(array_index_63); // #10979
        TEST_CASE(array_index_64); // #10878
        TEST_CASE(array_index_65); // #11066
        TEST_CASE(array_index_66); // #10740
        TEST_CASE(array_index_67); // #1596
        TEST_CASE(array_index_68); // #6655
        TEST_CASE(array_index_69); // #6370
        TEST_CASE(array_index_70); // #11355
        TEST_CASE(array_index_71); // #11461
        TEST_CASE(array_index_72); // #11784
        TEST_CASE(array_index_73); // #11530
        TEST_CASE(array_index_74); // #11088
        TEST_CASE(array_index_75);
        TEST_CASE(array_index_76);
        TEST_CASE(array_index_multidim);
        TEST_CASE(array_index_switch_in_for);
        TEST_CASE(array_index_for_in_for);   // FP: #2634
        TEST_CASE(array_index_bounds);
        TEST_CASE(array_index_calculation);
        TEST_CASE(array_index_negative1);
        TEST_CASE(array_index_negative2);    // ticket #3063
        TEST_CASE(array_index_negative3);
        TEST_CASE(array_index_negative4);
        TEST_CASE(array_index_negative5);    // #10526
        TEST_CASE(array_index_negative6);    // #11349
        TEST_CASE(array_index_negative7);    // #5685
        TEST_CASE(array_index_negative8);    // #11651
        TEST_CASE(array_index_negative9);
        TEST_CASE(array_index_negative10);
        TEST_CASE(array_index_for_decr);
        TEST_CASE(array_index_varnames);     // FP: struct member #1576, FN: #1586
        TEST_CASE(array_index_for_continue); // for,continue
        TEST_CASE(array_index_for);          // FN: for,if
        TEST_CASE(array_index_for_neq);      // #2211: Using != in condition
        TEST_CASE(array_index_for_question); // #2561: for, ?:
        TEST_CASE(array_index_for_andand_oror);  // FN: using && or || in the for loop condition
        TEST_CASE(array_index_for_varid0);   // #4228: No varid for counter variable
        TEST_CASE(array_index_vla_for);      // #3221: access VLA inside for
        TEST_CASE(array_index_extern);       // FP when using 'extern'. #1684
        TEST_CASE(array_index_cast);         // FP after cast. #2841
        TEST_CASE(array_index_string_literal);
        TEST_CASE(array_index_same_struct_and_var_name); // #4751 - not handled well when struct name and var name is same
        TEST_CASE(array_index_valueflow);
        TEST_CASE(array_index_valueflow_pointer);
        TEST_CASE(array_index_function_parameter);
        TEST_CASE(array_index_enum_array); // #8439
        TEST_CASE(array_index_container); // #9386
        TEST_CASE(array_index_two_for_loops);
        TEST_CASE(array_index_new); // #7690

        TEST_CASE(buffer_overrun_2_struct);
        TEST_CASE(buffer_overrun_3);
        TEST_CASE(buffer_overrun_4);
        TEST_CASE(buffer_overrun_5);
        TEST_CASE(buffer_overrun_6);
        TEST_CASE(buffer_overrun_7);
        TEST_CASE(buffer_overrun_8);
        TEST_CASE(buffer_overrun_9);
        TEST_CASE(buffer_overrun_10);
        TEST_CASE(buffer_overrun_11);
        TEST_CASE(buffer_overrun_15); // ticket #1787
        TEST_CASE(buffer_overrun_16);
        TEST_CASE(buffer_overrun_18); // ticket #2576 - for, calculation with loop variable
        TEST_CASE(buffer_overrun_19); // #2597 - class member with unknown type
        TEST_CASE(buffer_overrun_21);
        TEST_CASE(buffer_overrun_24); // index variable is changed in for-loop
        TEST_CASE(buffer_overrun_26); // #4432 (segmentation fault)
        TEST_CASE(buffer_overrun_27); // #4444 (segmentation fault)
        TEST_CASE(buffer_overrun_29); // #7083: false positive: typedef and initialization with strings
        TEST_CASE(buffer_overrun_30); // #6367
        TEST_CASE(buffer_overrun_31);
        TEST_CASE(buffer_overrun_32); //#10244
        TEST_CASE(buffer_overrun_33); //#2019
        TEST_CASE(buffer_overrun_34); //#11035
        TEST_CASE(buffer_overrun_35); //#2304
        TEST_CASE(buffer_overrun_36);
        TEST_CASE(buffer_overrun_errorpath);
        TEST_CASE(buffer_overrun_bailoutIfSwitch);  // ticket #2378 : bailoutIfSwitch
        TEST_CASE(buffer_overrun_function_array_argument);
        TEST_CASE(possible_buffer_overrun_1); // #3035
        TEST_CASE(buffer_overrun_readSizeFromCfg);
        TEST_CASE(buffer_overrun_handle_addr_of_var); // ticket #7570 - correctly handle &var

        TEST_CASE(valueflow_string); // using ValueFlow string values in checking

        // It is undefined behaviour to point out of bounds of an array
        // the address beyond the last element is in bounds
        // char a[10];
        // char *p1 = a + 10;  // OK
        // char *p2 = a + 11   // UB
        TEST_CASE(pointer_out_of_bounds_1);
        TEST_CASE(pointer_out_of_bounds_2);
        TEST_CASE(pointer_out_of_bounds_3);
        TEST_CASE(pointer_out_of_bounds_4);
        TEST_CASE(pointer_out_of_bounds_5); // #10227
        TEST_CASE(pointer_out_of_bounds_sub);

        TEST_CASE(strcat1);

        TEST_CASE(varid1);
        TEST_CASE(varid2);  // ticket #4764

        TEST_CASE(assign1);

        TEST_CASE(alloc_new);      // Buffer allocated with new
        TEST_CASE(alloc_malloc);   // Buffer allocated with malloc
        TEST_CASE(alloc_string);   // statically allocated buffer
        TEST_CASE(alloc_alloca);   // Buffer allocated with alloca

        // TODO TEST_CASE(countSprintfLength);
        TEST_CASE(minsize_argvalue);
        TEST_CASE(minsize_sizeof);
        TEST_CASE(minsize_strlen);
        TEST_CASE(minsize_mul);
        TEST_CASE(unknownType);

        TEST_CASE(terminateStrncpy1);
        TEST_CASE(terminateStrncpy2);
        TEST_CASE(terminateStrncpy3);
        TEST_CASE(terminateStrncpy4);
        TEST_CASE(terminateStrncpy5); // #9944
        TEST_CASE(terminateStrncpy_handle_addr_of_var); // #7570
        TEST_CASE(recursive_long_time);

        TEST_CASE(crash1);  // Ticket #1587 - crash
        TEST_CASE(crash2);  // Ticket #3034 - crash
        TEST_CASE(crash3);  // Ticket #5426 - crash
        TEST_CASE(crash4);  // Ticket #8679 - crash
        TEST_CASE(crash5);  // Ticket #8644 - crash
        TEST_CASE(crash6);  // Ticket #9024 - crash
        TEST_CASE(crash7);  // Ticket #9073 - crash

        TEST_CASE(insecureCmdLineArgs);
        TEST_CASE(checkBufferAllocatedWithStrlen);

        TEST_CASE(scope);   // handling different scopes

        TEST_CASE(getErrorMessages);

        // Access array and then check if the used index is within bounds
        TEST_CASE(arrayIndexThenCheck);
        TEST_CASE(arrayIndexEarlyReturn); // #6884

        TEST_CASE(bufferNotZeroTerminated);

        TEST_CASE(negativeMemoryAllocationSizeError); // #389
        TEST_CASE(negativeArraySize);

        TEST_CASE(pointerAddition1);

        TEST_CASE(ctu_malloc);
        TEST_CASE(ctu_array);
        TEST_CASE(ctu_variable);
        TEST_CASE(ctu_arithmetic);

        TEST_CASE(objectIndex);

        TEST_CASE(checkPipeParameterSize); // ticket #3521
    }



    void noerr1() {
        check("extern int ab;\n"
              "void f()\n"
              "{\n"
              "    if (ab)\n"
              "    {\n"
              "        char str[50];\n"
              "    }\n"
              "    if (ab)\n"
              "    {\n"
              "        char str[50];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    void noerr2() {
        check("static char buf[2];\n"
              "void f1(char *str)\n"
              "{\n"
              "    strcpy(buf,str);\n"
              "}\n"
              "void f2(char *str)\n"
              "{\n"
              "    strcat(buf,str);\n"
              "}\n"
              "void f3(char *str)\n"
              "{\n"
              "    sprintf(buf,\"%s\",str);\n"
              "}\n"
              "void f4(const char str[])\n"
              "{\n"
              "    strcpy(buf, str);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    void noerr3() {
        check("struct { char data[10]; } abc;\n"
              "static char f()\n"
              "{\n"
              "    char data[1];\n"
              "    return abc.data[1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    void noerr4() {
        // The memory isn't read or written and therefore there is no error.
        check("static void f() {\n"
              "    char data[100];\n"
              "    const char *p = data + 100;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void sizeof3() {
        check("struct group { int gr_gid; };\n"
              "void f()\n"
              "{\n"
              "    char group[32];\n"
              "    snprintf(group, 32, \"%u\", 0);\n"
              "    struct group *gr;\n"
              "    snprintf(group, 32, \"%u\", gr->gr_gid);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_1() {
        check("void f()\n"
              "{\n"
              "    char str[0x10] = {0};\n"
              "    str[15] = 0;\n"
              "    str[16] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:8]: (error) Array 'str[16]' accessed at index 16, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("char f()\n"
              "{\n"
              "    char str[16] = {0};\n"
              "    return str[16];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:15]: (error) Array 'str[16]' accessed at index 16, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // test stack array
        check("int f()\n"
              "{\n"
              "   int x[ 3 ] = { 0, 1, 2 };\n"
              "   int y;\n"
              "   y = x[ 4 ];\n"
              "   return y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9]: (error) Array 'x[3]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("int f()\n"
              "{\n"
              "   int x[ 3 ] = { 0, 1, 2 };\n"
              "   int y;\n"
              "   y = x[ 2 ];\n"
              "   return y;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int x[5] = {0};\n"
              "int a = x[10];");
        ASSERT_EQUALS("[test.cpp:2:10]: (error) Array 'x[5]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("int x[5] = {0};\n"
              "int a = (x)[10];");
        ASSERT_EQUALS("[test.cpp:2:12]: (error) Array 'x[5]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }


    void array_index_2() {
        check("void a(int i)\n" // valueflow
              "{\n"
              "    char *str = new char[0x10];\n"
              "    str[i] = 0;\n"
              "}\n"
              "void b() { a(16); }");
        ASSERT_EQUALS("[test.cpp:4:8]: (error) Array 'str[16]' accessed at index 16, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_4() {
        check("char c = \"abc\"[4];");
        ASSERT_EQUALS("[test.cpp:1:15]: (error) Array '\"abc\"[4]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("p = &\"abc\"[4];");
        ASSERT_EQUALS("", errout_str());

        check("char c = \"\\0abc\"[2];");
        ASSERT_EQUALS("", errout_str());

        check("char c = L\"abc\"[4];");
        ASSERT_EQUALS("[test.cpp:1:16]: (error) Array 'L\"abc\"[4]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_3() {
        check("void f()\n"
              "{\n"
              "    int val[50];\n"
              "    int i, sum=0;\n"
              "    for (i = 0; i < 100; i++)\n"
              "        sum += val[i];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:19]: (error) Array 'val[50]' accessed at index 99, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    int val[50];\n"
              "    int i, sum=0;\n"
              "    for (i = 1; i < 100; i++)\n"
              "        sum += val[i];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:19]: (error) Array 'val[50]' accessed at index 99, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f(int a)\n"
              "{\n"
              "    int val[50];\n"
              "    int i, sum=0;\n"
              "    for (i = a; i < 100; i++)\n"
              "        sum += val[i];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:19]: (error) Array 'val[50]' accessed at index 99, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("typedef struct g g2[3];\n"
              "void foo(char *a)\n"
              "{\n"
              "  for (int i = 0; i < 4; i++)\n"
              "  {\n"
              "    a[i]=0;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int argc)\n"
              "{\n"
              "  char a[2];\n"
              "  for (int i = 4; i < argc; i++){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(int a[10]) {\n"
              "    for (int i=0;i<50;++i) {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Array 'a[10]' accessed at index 49, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_6() {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    abc.str[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:12]: (error) Array 'abc.str[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static char f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    return abc.str[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:19]: (error) Array 'abc.str[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // This is not out of bounds because it is a variable length array
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC) + 10);\n"
              "    x->str[1] = 0;"
              "}");
        ASSERT_EQUALS("", errout_str());

        // This is not out of bounds because it is not a variable length array
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "    int x;\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC) + 10);\n"
              "    x->str[1] = 0;"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        // This is not out of bounds because it is a variable length array
        // and the index is within the memory allocated.
        /** @todo this works by accident because we ignore any access to this array */
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC) + 10);\n"
              "    x->str[10] = 0;"
              "}");
        ASSERT_EQUALS("", errout_str());

        // This is out of bounds because it is outside the memory allocated.
        /** @todo this doesn't work because of a bug in sizeof(struct)  */
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC) + 10);\n"
              "    x->str[11] = 0;"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (error) Array 'str[1]' accessed at index 11, which is out of bounds.\n", "", errout_str());

        // This is out of bounds if 'sizeof(ABC)' is 1 (No padding)
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(ABC) + 10);\n"
              "    x->str[11] = 0;"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        // This is out of bounds because it is outside the memory allocated
        /** @todo this doesn't work because of a bug in sizeof(struct) */
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC));\n"
              "    x->str[1] = 0;"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (error) Array 'str[1]' accessed at index 1, which is out of bounds.\n", "", errout_str());

        // This is out of bounds because it is outside the memory allocated
        // But only if 'sizeof(ABC)' is 1 (No padding)
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(ABC));\n"
              "    x->str[1] = 0;"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        // This is out of bounds because it is not a variable array
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC x;\n"
              "    x.str[1] = 0;"
              "}");
        ASSERT_EQUALS("[test.cpp:9:10]: (error) Array 'x.str[1]' accessed at index 1, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("struct foo\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "void x()\n"
              "{\n"
              "    foo f;\n"
              "    for ( unsigned int i = 0; i < 64; ++i )\n"
              "        f.str[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:14]: (error) Array 'f.str[10]' accessed at index 63, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("struct AB { char a[NUM]; char b[NUM]; }\n"
              "void f(struct AB *ab) {\n"
              "    ab->a[0] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("union { char a[1]; int b; } ab;\n"
              "void f() {\n"
              "    ab.a[2] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Array 'ab.a[1]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }


    void array_index_7() {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void f(struct ABC *abc)\n"
              "{\n"
              "    abc->str[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:13]: (error) Array 'abc->str[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_11() {
        check("class ABC\n"
              "{\n"
              "public:\n"
              "    ABC();\n"
              "    char *str[10];\n"
              "    struct ABC *next();\n"
              "};\n"
              "\n"
              "static void f(ABC *abc1)\n"
              "{\n"
              "    for ( ABC *abc = abc1; abc; abc = abc->next() )\n"
              "    {\n"
              "        abc->str[10] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:13:17]: (error) Array 'abc->str[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_12() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char str[10];\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{\n"
              "    str[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:8]: (error) Array 'str[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char str[10];\n"
              "public:\n"
              "    char c();\n"
              "};\n"
              "char Fred::c()\n"
              "{\n"
              "    return str[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:15]: (error) Array 'str[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_13() {
        check("void f()\n"
              "{\n"
              "    char buf[10];\n"
              "    for (int i = 0; i < 100; i++)\n"
              "    {\n"
              "        if (i < 10)\n"
              "            int x = buf[i];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_14() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i+10] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'a[10]' accessed at index 19, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_15() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[10+i] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'a[10]' accessed at index 19, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_16() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i+1] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'a[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_17() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i*2] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'a[10]' accessed at index 18, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    int a[12];\n"
              "    for (int i = 0; i < 12; i+=6)\n"
              "        a[i+5] = i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    int a[12];\n"
              "    for (int i = 0; i < 12; i+=6)\n"
              "        a[i+6] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'a[12]' accessed at index 12, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"  // #4398
              "    int a[2];\n"
              "    for (int i = 0; i < 4; i+=2)\n"
              "        a[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'a[2]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"  // #4398
              "    int a[2];\n"
              "    for (int i = 0; i < 4; i+=2)\n"
              "        do_stuff(a+i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_18() {
        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i+=1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        ++i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i=4;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:10]: (error) Array 'a[5]' accessed at index 5, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    int a[6];\n"
              "    for (int i = 0; i < 7; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i+=1;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Buffer overrun\n", "", errout_str());
    }

    void array_index_19() {
        // "One Past the End" is legal, as long as pointer is not dereferenced.
        check("void f()\n"
              "{\n"
              "  char a[2];\n"
              "  char *end = &(a[2]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Getting more than one past the end is not legal
        check("void f()\n"
              "{\n"
              "  char a[2];\n"
              "  char *end = &(a[3]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (error) Array 'a[2]' accessed at index 3, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_20() {
        check("void f()\n"
              "{\n"
              " char a[8];\n"
              " int b[10];\n"
              " for ( int i = 0; i < 9; i++ )\n"
              "  b[i] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_21() {
        check("class A {\n"
              " int indices[2];\n"
              " void foo(int indices[3]);\n"
              "};\n"
              "\n"
              "void A::foo(int indices[3]) {\n"
              " for(int j=0; j<3; ++j) {\n"
              "  int b = indices[j];\n"
              " }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_22() {
        check("int main() {\n"
              "  size_t indices[2];\n"
              "  int b = indices[2];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (error) Array 'indices[2]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_23() {
        check("void foo()\n"
              "{\n"
              "    char c[10];\n"
              "    c[1<<23]='a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 'c[10]' accessed at index 8388608, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_24() {
        // ticket #1492 and #1539
        const std::string charMaxPlusOne(settings0.platform.defaultSign == 'u' ? "256" : "128");
        check("void f(char n) {\n"
              "    int a[n];\n"     // n <= CHAR_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[" + charMaxPlusOne + "] = 0;\n"   // 128/256 > CHAR_MAX
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'a[" + charMaxPlusOne + "]' accessed at index -1, which is out of bounds. [negativeIndex]\n"
                      "[test.cpp:4:6]: (error) Array 'a[" + charMaxPlusOne + "]' accessed at index " + charMaxPlusOne + ", which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f(signed char n) {\n"
              "    int a[n];\n"     // n <= SCHAR_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[128] = 0;\n"   // 128 > SCHAR_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'a[128]' accessed at index -1, which is out of bounds. [negativeIndex]\n"
                      "[test.cpp:4:6]: (error) Array 'a[128]' accessed at index 128, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f(unsigned char n) {\n"
              "    int a[n];\n"     // n <= UCHAR_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[256] = 0;\n"   // 256 > UCHAR_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'a[256]' accessed at index -1, which is out of bounds. [negativeIndex]\n"
                      "[test.cpp:4:6]: (error) Array 'a[256]' accessed at index 256, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f(short n) {\n"
              "    int a[n];\n"     // n <= SHRT_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[32768] = 0;\n" // 32768 > SHRT_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'a[32768]' accessed at index -1, which is out of bounds. [negativeIndex]\n"
                      "[test.cpp:4:6]: (error) Array 'a[32768]' accessed at index 32768, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f(unsigned short n) {\n"
              "    int a[n];\n"     // n <= USHRT_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[65536] = 0;\n" // 65536 > USHRT_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'a[65536]' accessed at index -1, which is out of bounds. [negativeIndex]\n"
                      "[test.cpp:4:6]: (error) Array 'a[65536]' accessed at index 65536, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f(signed short n) {\n"
              "    int a[n];\n"     // n <= SHRT_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[32768] = 0;\n" // 32768 > SHRT_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'a[32768]' accessed at index -1, which is out of bounds. [negativeIndex]\n"
                      "[test.cpp:4:6]: (error) Array 'a[32768]' accessed at index 32768, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f(int n) {\n"
              "    int a[n];\n"     // n <= INT_MAX
              "    a[-1] = 0;\n"    // negative index
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'a[2147483648]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());

        check("void f(unsigned int n) {\n"
              "    int a[n];\n"     // n <= UINT_MAX
              "    a[-1] = 0;\n"    // negative index
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'a[4294967296]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());

        check("void f(signed int n) {\n"
              "    int a[n];\n"     // n <= INT_MAX
              "    a[-1] = 0;\n"    // negative index
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'a[2147483648]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());
    }

    void array_index_25() { // #1536
        check("void foo()\n"
              "{\n"
              "   long l[SOME_SIZE];\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_26() {
        check("void f()\n"
              "{\n"
              "    int a[3];\n"
              "    for (int i = 3; 0 <= i; i--)\n"
              "        a[i] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'a[3]' accessed at index 3, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    int a[4];\n"
              "    for (int i = 3; 0 <= i; i--)\n"
              "        a[i] = i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_27() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i-1] = a[i];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'a[10]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());
    }

    void array_index_28() {
        // ticket #1418
        check("void f()\n"
              "{\n"
              "    int i[2];\n"
              "    int *ip = i + 1;\n"
              "    ip[-10] = 1;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Array ip[-10] out of bounds.\n", "", errout_str());
    }

    void array_index_29() {
        // ticket #1724
        check("void f()\n"
              "{\n"
              "    int iBuf[10];"
              "    int *i = iBuf + 9;"
              "    int *ii = i + -5;"
              "    ii[10] = 0;"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Array ii[10] out of bounds.\n", "", errout_str());
    }

    void array_index_30() {
        // ticket #2086 - unknown type
        // extracttests.start: typedef unsigned char UINT8;
        check("void f() {\n"
              "    UINT8 x[2];\n"
              "    x[5] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'x[2]' accessed at index 5, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_31() {
        // ticket #2120 - sub function, unknown type
        check("struct s1 {\n"
              "    unknown_type_t delay[3];\n"
              "};\n"
              "\n"
              "void x(unknown_type_t *delay, const int *net) {\n"
              "    delay[0] = 0;\n"
              "}\n"
              "\n"
              "void y() {\n"
              "    struct s1 obj;\n"
              "    x(obj.delay, 123);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct s1 {\n"
              "    unknown_type_t delay[3];\n"
              "};\n"
              "\n"
              "void x(unknown_type_t *delay, const int *net) {\n"
              "    delay[4] = 0;\n"
              "}\n"
              "\n"
              "void y() {\n"
              "    struct s1 obj;\n"
              "    x(obj.delay, 123);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:6]: (error) Array 'obj.delay[3]' accessed at index 4, which is out of bounds.\n",
                           "",
                           errout_str());

        check("struct s1 {\n"
              "    float a[0];\n"
              "};\n"
              "\n"
              "void f() {\n"
              "    struct s1 *obj;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_32() {
        check("class X\n"
              "{\n"
              "    public:\n"
              "    X()\n"
              "    {\n"
              "       m_x[0] = 0;\n"
              "       m_x[1] = 0;\n"
              "    }\n"
              "    int m_x[1];\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7:11]: (error) Array 'm_x[1]' accessed at index 1, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_33() {
        check("void foo(char bar[][4]) {\n"
              "    baz(bar[5]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_34() { // ticket #3063
        check("void foo() {\n"
              "    int y[2][2][2];\n"
              "    y[0][2][0] = 0;\n"
              "    y[0][0][2] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Array 'y[2][2][2]' accessed at index y[0][2][0], which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:4:6]: (error) Array 'y[2][2][2]' accessed at index y[0][0][2], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("struct TEST\n"
              "{\n"
              "    char a[10];\n"
              "    char b[10][5];\n"
              "};\n"
              "void foo()\n"
              "{\n"
              "    TEST test;\n"
              "    test.a[10] = 3;\n"
              "    test.b[10][2] = 4;\n"
              "    test.b[0][19] = 4;\n"
              "    TEST *ptest;\n"
              "    ptest = &test;\n"
              "    ptest->a[10] = 3;\n"
              "    ptest->b[10][2] = 4;\n"
              "    ptest->b[0][19] = 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:11]: (error) Array 'test.a[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:10:11]: (error) Array 'test.b[10][5]' accessed at index test.b[10][2], which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:11:11]: (error) Array 'test.b[10][5]' accessed at index test.b[0][19], which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:14:13]: (error) Array 'ptest->a[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:15:13]: (error) Array 'ptest->b[10][5]' accessed at index ptest->b[10][2], which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:16:13]: (error) Array 'ptest->b[10][5]' accessed at index ptest->b[0][19], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("struct TEST\n"
              "{\n"
              "    char a[10][5];\n"
              "};\n"
              "void foo()\n"
              "{\n"
              "    TEST test;\n"
              "    test.a[9][5] = 4;\n"
              "    test.a[0][50] = 4;\n"
              "    TEST *ptest;\n"
              "    ptest = &test;\n"
              "    ptest->a[9][5] = 4;\n"
              "    ptest->a[0][50] = 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:11]: (error) Array 'test.a[10][5]' accessed at index test.a[9][5], which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:9:11]: (error) Array 'test.a[10][5]' accessed at index test.a[0][50], which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:12:13]: (error) Array 'ptest->a[10][5]' accessed at index ptest->a[9][5], which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:13:13]: (error) Array 'ptest->a[10][5]' accessed at index ptest->a[0][50], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_35() { // ticket #2889
        check("void f() {\n"
              "    struct Struct { unsigned m_Var[1]; } s;\n"
              "    s.m_Var[1] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Array 's.m_Var[1]' accessed at index 1, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("struct Struct { unsigned m_Var[1]; };\n"
              "void f() {\n"
              "    struct Struct s;\n"
              "    s.m_Var[1] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Array 's.m_Var[1]' accessed at index 1, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("struct Struct { unsigned m_Var[1]; };\n"
              "void f() {\n"
              "    struct Struct * s = calloc(40);\n"
              "    s->m_Var[1] = 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_36() { // ticket #2960
        check("class Fred {\n"
              "    Fred(const Fred &);\n"
              "private:\n"
              "    bool m_b[2];\n"
              "};\n"
              "Fred::Fred(const Fred & rhs) {\n"
              "    m_b[2] = rhs.m_b[2];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:8]: (error) Array 'm_b[2]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:7:21]: (error) Array 'rhs.m_b[2]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_37() {
        check("class Fred {\n"
              "    char x[X];\n"
              "    Fred() {\n"
              "        for (unsigned int i = 0; i < 15; i++)\n"
              "            i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_38() { //ticket #3273
        check("void aFunction() {\n"
              "    double aDoubleArray[ 10 ];\n"
              "    unsigned int i; i = 0;\n"
              "    for( i = 0; i < 6; i++ )\n"
              "    {\n"
              "        unsigned int j; j = 0;\n"
              "        for( j = 0; j < 5; j++ )\n"
              "        {\n"
              "            unsigned int x; x = 0;\n"
              "            for( x = 0; x < 4; x++ )\n"
              "            {\n"
              "            }\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_39() { // ticket 3387
        check("void aFunction()\n"
              "{\n"
              "    char a[10];\n"
              "    a[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 'a[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_40() {
        check("void f() {\n"
              "    char a[10];\n"
              "    for (int i = 0; i < 10; ++i)\n"
              "        f2(&a[i + 1]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_41() {
        // Don't generate false positives when structs have the same name
        check("void a() {\n"
              "    struct Fred { char data[6]; } fred;\n"
              "    fred.data[4] = 0;\n"  // <- no error
              "}\n"
              "\n"
              "void b() {\n"
              "    struct Fred { char data[3]; } fred;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void a() {\n"
              "    struct Fred { char data[6]; } fred;\n"
              "    fred.data[4] = 0;\n"  // <- no error
              "}\n"
              "\n"
              "void b() {\n"
              "    struct Fred { char data[3]; } fred;\n"
              "    fred.data[4] = 0;\n"  // <- error
              "}");
        ASSERT_EQUALS("[test.cpp:8:14]: (error) Array 'fred.data[3]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_42() { // ticket #3569

        check("void f()\n"
              "{\n"
              "  char *p; p = (char *)malloc(10);\n"
              "  p[10] = 7;\n"
              "  free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Array 'p[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "  float *p; p = (float *)malloc(10 * sizeof(float));\n"
              "  p[10] = 7;\n"
              "  free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Array 'p[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "  char *p; p = (char *)malloc(10);\n"
              "  p[0] = 0;\n"
              "  p[9] = 9;\n"
              "  free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "  char *p; p = new char[10];\n"
              "  p[0] = 0;\n"
              "  p[9] = 9;\n"
              "  delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "  char *p(new char[10]);\n"
              "  p[0] = 0;\n"
              "  p[9] = 9;\n"
              "  delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "  char *p = NULL;"
              "  try{\n"
              "  p = new char[10];\n"
              "  }\n"
              "  catch(...){\n"
              "  return;\n"
              "  }"
              "  p[0] = 0;\n"
              "  p[9] = 9;\n"
              "  delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_43() { // #3838

        check("int f( )\n"
              "{\n"
              "  struct {\n"
              "    int arr[ 3 ];\n"
              "  } var[ 1 ];\n"
              "   int y;\n"
              "   var[ 0 ].arr[ 0 ] = 0;\n"
              "   var[ 0 ].arr[ 1 ] = 1;\n"
              "   var[ 0 ].arr[ 2 ] = 2;\n"
              "   y = var[ 0 ].arr[ 3 ];\n" // <-- array access out of bounds
              "   return y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:20]: (error) Array 'var[0].arr[3]' accessed at index 3, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("int f( )\n"
              "{\n"
              "  struct {\n"
              "    int arr[ 3 ];\n"
              "  } var[ 1 ];\n"
              "   int y=1;\n"
              "   var[ 0 ].arr[ 0 ] = 0;\n"
              "   var[ 0 ].arr[ 1 ] = 1;\n"
              "   var[ 0 ].arr[ 2 ] = 2;\n"
              "   y = var[ 0 ].arr[ 2 ];\n"
              "   return y;\n"
              "}");
        ASSERT_EQUALS("", errout_str());


        check("int f( ){\n"
              "struct Struct{\n"
              "    int arr[ 3 ];\n"
              "};\n"
              "int y;\n"
              "Struct var;\n"
              "var.arr[ 0 ] = 0;\n"
              "var.arr[ 1 ] = 1;\n"
              "var.arr[ 2 ] = 2;\n"
              "var.arr[ 3 ] = 3;\n" // <-- array access out of bounds
              "y=var.arr[ 3 ];\n"   // <-- array access out of bounds
              "return y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:8]: (error) Array 'var.arr[3]' accessed at index 3, which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:11:10]: (error) Array 'var.arr[3]' accessed at index 3, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());


        check("void f( ) {\n"
              "struct S{\n"
              "    int var[ 3 ];\n"
              "} ;\n"
              "S var[2];\n"
              "var[0].var[ 0 ] = 0;\n"
              "var[0].var[ 1 ] = 1;\n"
              "var[0].var[ 2 ] = 2;\n"
              "var[0].var[ 4 ] = 4;\n" // <-- array access out of bounds
              "}");
        ASSERT_EQUALS("[test.cpp:9:11]: (error) Array 'var[0].var[3]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f( ) {\n"
              "struct S{\n"
              "    int var[ 3 ];\n"
              "} ;\n"
              "S var[2];\n"
              "var[0].var[ 0 ] = 0;\n"
              "var[0].var[ 1 ] = 1;\n"
              "var[0].var[ 2 ] = 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // avoid FPs (modified examples taken from #3838)
        check("struct AB { int a[10]; int b[10]; };\n"
              "int main() {\n"
              "    struct AB ab;\n"
              "    int * p = &ab.a[10];\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct AB { int a[10]; int b[10]; };\n"
              "int main() {\n"
              "    struct AB ab[1];\n"
              "    int * p = &ab[0].a[10];\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct AB { int a[10]; int b[10]; };\n"
              "int main() {\n"
              "    struct AB ab[1];\n"
              "    int * p = &ab[10].a[0];\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (error) Array 'ab[1]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_44() { // #3979 (false positive)

        check("void f()\n"
              "{\n"
              "    char buf[2];\n"
              "    int i;\n"
              "    for (i = 2; --i >= 0; )\n"
              "    {\n"
              "        buf[i] = 1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    double buf[2];\n"
              "    for (int i = 2; i--; )\n"
              "    {\n"
              "        buf[i] = 2.;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_45() { // #4207 - handling of function with variable number of parameters / unnamed arguments
        // Variable number of arguments
        check("void f(const char *format, ...) {\n"
              "    va_args args;\n"
              "    va_start(args, format);\n"
              "}\n"
              "void test() {\n"
              "    CHAR buffer[1024];\n"
              "    f(\"%s\", buffer);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Unnamed argument
        check("void f(char *) {\n"
              "    dostuff();\n"
              "}\n"
              "void test() {\n"
              "    char buffer[1024];\n"
              "    f(buffer);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // Two statement for-loop
    void array_index_46() {
        // #4840
        check("void bufferAccessOutOfBounds2() {\n"
              "    char *buffer[]={\"a\",\"b\",\"c\"};\n"
              "    for(int i=3; i--;) {\n"
              "        printf(\"files(%i): %s\", 3-i, buffer[3-i]);\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Array 'buffer[3]' accessed at index 3, which is out of bounds.\n", "", errout_str());

        check("void f() {\n"
              "    int buffer[9];\n"
              "    long int i;\n"
              "    for(i=10; i--;) {\n"
              "        buffer[i] = i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:15]: (error) Array 'buffer[9]' accessed at index 9, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // Correct access limits -> i from 9 to 0
        check("void f() {\n"
              "    int buffer[10];\n"
              "    for(unsigned long int i=10; i--;) {\n"
              "        buffer[i] = i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_47() {
        // #5849
        check("int s[4];\n"
              "void f() {\n"
              "    for (int i = 2; i < 0; i++)\n"
              "        s[i] = 5;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_48() {
        // #9478
        check("void test(void)\n"
              "{\n"
              "    int array[4] = { 1,2,3,4 };\n"
              "    for (int i = 1; i <= 4; i++) {\n"
              "        printf(\" %i\", i);\n"
              "        array[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:14]: (error) Array 'array[4]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void test(void)\n"
              "{\n"
              "    int array[4] = { 1,2,3,4 };\n"
              "    for (int i = 1; i <= 4; i++) {\n"
              "        scanf(\"%i\", &i);\n"
              "        array[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_49() {
        // #8653
        check("void f() {\n"
              "    int i, k;\n"
              "    int arr[34] = {};\n"
              "    i = 1;\n"
              "    for (k = 0; k < 34 && i < 34; k++) {\n"
              "        i++;\n"
              "    }\n"
              "    arr[k];\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_50() {
        check("void f(const char * str) {\n"
              "    int len = strlen(str);\n"
              "    (void)str[len - 1];\n"
              "}\n"
              "void g() {\n"
              "    f(\"12345678\");\n"
              "    f(\"12345\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_51() {
        check("void f(void){\n"
              "    int k=0, dd, d[1U] = {1};\n"
              "    for (dd=d[k]; k<10; dd=d[++k]){;}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:29]: (error) Array 'd[1]' accessed at index 1, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_52() {
        check("char f(void)\n"
              "{\n"
              "    char buf[10];\n"
              "    for(int i = 0, j= 11; i < j; ++i)\n"
              "       buf[i] = 0;\n"
              "    return buf[0];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:11]: (error) Array 'buf[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_53() {
        check("double M[3][1];\n"
              " \n"
              "void matrix()\n"
              "{\n"
              "    for (int i=0; i < 3; i++)\n"
              "        for (int j = 0; j < 3; j++)\n"
              "             M[i][j]=0.0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:15]: (error) Array 'M[3][1]' accessed at index M[*][2], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_54() {
        check("void f() {\n"
              "    g(0);\n"
              "}\n"
              "void g(unsigned int x) {\n"
              "    int b[4];\n"
              "    for (unsigned int i = 0; i < 4; i += 2) {\n"
              "        b[i]   = 0;\n"
              "        b[i+1] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_55() {
        check("void make(const char* s, size_t len) {\n"
              "    for (size_t i = 0; i < len; ++i)\n"
              "        s[i];\n"
              "}\n"
              "void make(const char* s) {\n"
              "    make(s, strlen(s));\n"
              "}\n"
              "void f() {\n"
              "    make(\"my-utf8-payload\");\n"
              "}\n"
              "void f2() {\n"
              "    make(\"false\");\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_56() {
        check("struct s {\n"
              "    int array[1];\n"
              "    int index;\n"
              "};\n"
              "void f(struct s foo) {\n"
              "    foo.array[foo.index++] = 1;\n"
              "    if (foo.index == 1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_57() {
        check("void f(std::vector<int>& v) {\n"
              "    int a[3] = { 1, 2, 3 };\n"
              "    int i = 0;\n"
              "    for (auto& x : v) {\n"
              "        int c = a[i++];\n"
              "        if (i == 3)\n"
              "            i = 0;\n"
              "        x = c;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int>& v) {\n"
              "    int a[3] = { 1, 2, 3 };\n"
              "    int i = 0;\n"
              "    for (auto& x : v) {\n"
              "        int c = a[i++];\n"
              "        if (i == 4)\n"
              "            i = 0;\n"
              "        x = c;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:6:15] -> [test.cpp:5:18]: (warning) Either the condition 'i==4' is redundant or the array 'a[3]' is accessed at index 3, which is out of bounds. [arrayIndexOutOfBoundsCond]\n",
            errout_str());
    }

    void array_index_58()
    {
        check("int f(int x, int y) {\n"
              "    int a[3]= {0,1,2};\n"
              "    if(x<2)\n"
              "        y = a[x] + 1;\n"
              "    else\n"
              "        y = a[x];\n"
              "    return y;\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:9] -> [test.cpp:6:14]: (warning) Either the condition 'x<2' is redundant or the array 'a[3]' is accessed at index 3, which is out of bounds. [arrayIndexOutOfBoundsCond]\n",
            errout_str());

        check("void f() {\n" // #2199
              "    char a[5];\n"
              "    for (int i = 0; i < 5; i++) {\n"
              "        i += 8;\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}\n"
              "void g() {\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 5; i++) {\n"
              "        a[i + 7] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:5:10]: (error) Array 'a[5]' accessed at index 8, which is out of bounds. [arrayIndexOutOfBounds]\n"
            "[test.cpp:11:10]: (error) Array 'a[5]' accessed at index 11, which is out of bounds. [arrayIndexOutOfBounds]\n",
            errout_str());
    }

    void array_index_59() // #10413
    {
        check("long f(long b) {\n"
              "  const long a[] = { 0, 1, };\n"
              "  const long c = std::size(a);\n"
              "  if (b < 0 || b >= c)\n"
              "    return 0;\n"
              "  return a[b];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a, int b) {\n"
              "    const int S[2] = {};\n"
              "    if (a < 0) {}\n"
              "    else {\n"
              "        if (b < 0) {}\n"
              "        else if (S[b] > S[a]) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int a[2] = {};\n"
              "void f(int i) {\n"
              "    g(i < 0 ? 0 : a[i]);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_60()
    {
        checkP("#define CKR(B) if (!(B)) { return -1; }\n"
               "int f(int i) {\n"
               "  const int A[3] = {};\n"
               "  CKR(i < 3);\n"
               "  if (i > 0)\n"
               "      i = A[i];\n"
               "  return i;\n"
               "}\n");
        ASSERT_EQUALS("", errout_str());

        checkP("#define ASSERT(expression, action) if (expression) {action;}\n"
               "int array[5];\n"
               "void func (int index) {\n"
               "    ASSERT(index > 5, return);\n"
               "    array[index]++;\n"
               "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:5] -> [test.cpp:5:10]: (warning) Either the condition 'index>5' is redundant or the array 'array[5]' is accessed at index 5, which is out of bounds. [arrayIndexOutOfBoundsCond]\n",
            errout_str());
    }

    void array_index_61()
    {
        check("int f(int i) {\n"
              "  const int M[] = { 0, 1, 2, 3 };\n"
              "  if (i > 4)\n"
              "      return -1;\n"
              "  if (i < 0 || i == std::size(M))\n"
              "    return 0; \n"
              "  return M[i];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { enum E { e0 }; };\n"
              "const S::E M[4] = { S::E:e0, S::E:e0, S::E:e0, S::E:e0 };\n"
              "int f(int i) {\n"
              "  if (i > std::size(M) + 1)\n"
              "	  return -1;\n"
              "  if (i < 0 || i >= std::size(M))\n"
              "	  return 0;\n"
              "  return M[i]; \n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_62()
    {
        check("struct X {\n"
              "    static int GetSize() {return 11;}\n"
              "};\n"
              "char f() {\n"
              "    char buf[10]= {0};\n"
              "    for(int i = 0; i < X::GetSize(); ++i) \n"
              "       buf[i] = 0;\n"
              "    return buf[0];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:11]: (error) Array 'buf[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n",
                      errout_str());
    }

    void array_index_63()
    {
        check("int b[4];\n" // #10979
              "void f(int i) {\n"
              "    if (i >= 0 && i < sizeof(b) / sizeof(*(b)))\n"
              "        b[i] = 0;\n"
              "    if (i >= 0 && i < sizeof(b) / sizeof((b)[0]))\n"
              "        b[i] = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_64() // #10878
    {
        check("struct Array {\n"
              "    int x[10];\n"
              "    int& accessArrayRef(int a) { return x[a]; }\n"
              "};\n"
              "void f() {\n"
              "    Array array = {};\n"
              "    array.accessArrayRef(10);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:42]: (error) Array 'x[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("int i = 10;\n"
              "struct Array {\n"
              "    int x[10];\n"
              "    int& accessArrayRef(int a) { return x[a]; }\n"
              "};\n"
              "void f() {\n"
              "    Array array = {};\n"
              "    array.accessArrayRef(i);\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3:42]: (error) Array 'x[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", "", errout_str());
    }

    void array_index_65() // #11066
    {
        check("char P[] = { 2, 1 };\n"
              "char f[2];\n"
              "void e(char* c) {\n"
              "    register j;\n"
              "    for (j = 0; j < 2; j++)\n"
              "        c[j] = f[P[j] - 1];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_66()
    {
        check("void foo(int j) {\n"
              "    int offsets[256];\n"
              "    while (x) {\n"
              "        if (j >= 256) break;\n"
              "        offsets[++j] = -1;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:15] -> [test.cpp:5:16]: (warning) Either the condition 'j>=256' is redundant or the array 'offsets[256]' is accessed at index 256, which is out of bounds. [arrayIndexOutOfBoundsCond]\n",
            errout_str());
    }

    void array_index_67() {
        check("void func(int i) {\n" // #1596
              "    int types[3];\n"
              "    int type_cnt = 0;\n"
              "    if (i == 0) {\n"
              "        types[type_cnt] = 0;\n"
              "        type_cnt++;\n"
              "        types[type_cnt] = 0;\n"
              "        type_cnt++;\n"
              "        types[type_cnt] = 0;\n"
              "        type_cnt++;\n"
              "    } else {\n"
              "        types[type_cnt] = 1;\n"
              "        type_cnt++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_68() { // #6655
        check("int ia[10];\n"
              "void f(int len) {\n"
              "    for (int i = 0; i < len; i++)\n"
              "        ia[i] = 0;\n"
              "}\n"
              "int g() {\n"
              "    f(20);\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Array 'ia[10]' accessed at index 19, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    // #6370
    void array_index_69()
    {
        check("void f() {\n"
              "    const int e[] = {0,10,20,30};\n"
              "    int a[4];\n"
              "    for(int i = 0; i < 4; ++i)\n"
              "      a[e[i]] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:8]: (error) Array 'a[4]' accessed at index 30, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    // #11355
    void array_index_70() {
        check("void f() {\n"
              "    static const char a[] = ((\"test\"));\n"
              "    printf(\"%c\", a[5]);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:19]: (error) Array 'a[5]' accessed at index 5, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    // #11461
    void array_index_71()
    {
        check("unsigned int f(unsigned int Idx) {\n"
              "  if (Idx < 64)\n"
              "    return 0;\n"
              "  Idx -= 64;\n"
              "  int arr[64] = { 0 };\n"
              "  return arr[Idx];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // #11784
    void array_index_72()
    {
        check("char f(int i) {\n"
              "  char d[4] = {};\n"
              "  for (; i < 3; i++) {}\n"
              "  for (i++; i > 0;) {\n"
              "    d[--i] = 1;\n"
              "    break;\n"
              "  }\n"
              "  return d[3];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // #11530
    void array_index_73()
    {
        check("void f() {\n"
              "  int k = 0;\n"
              "  std::function<void(int)> a[1] = {};\n"
              "  a[k++](0);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // #11088
    void array_index_74()
    {
        check("void foo(const char *keys) {\n"
              "  const char *prefix = \"<Shift+\";\n"
              "  const size_t prefix_len = strlen(prefix);\n"
              "  if (strncmp(keys, prefix, prefix_len)) { return; }\n"
              "  if (keys[prefix_len] == '>') {}\n"
              "}\n"
              "void bar() {\n"
              "  foo(\"q\");\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // #1644
    void array_index_75()
    {
        check("void f() {\n"
              "    char buf[10];\n"
              "    int i = 10;\n"
              "    while (i >= 3)\n"
              "        buf[i--] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Array 'buf[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    // #12592
    void array_index_76()
    {
        check("void cb0(void*, int i) {\n"
              "    const char s[] = \"\";\n"
              "    (void)s[i];\n"
              "}\n"
              "void cb1(int i, void*) {\n"
              "    const char s[] = \"\";\n"
              "    (void)s[i];\n"
              "}\n"
              "void f() {\n"
              "    cb0(nullptr, 1);\n"
              "    cb1(1, nullptr);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Array 's[1]' accessed at index 1, which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:7:12]: (error) Array 's[1]' accessed at index 1, which is out of bounds. [arrayIndexOutOfBounds]\n",
                      errout_str());
    }

    void array_index_multidim() {
        check("void f()\n"
              "{\n"
              "  char a[2][2];\n"
              "  a[1][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2];\n"
              "  a[1][1][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "  char a[2][2];\n"
              "  a[2][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Array 'a[2][2]' accessed at index a[2][1], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "  char a[2][2];\n"
              "  a[1][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Array 'a[2][2]' accessed at index a[1][2], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2];\n"
              "  a[2][1][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Array 'a[2][2][2]' accessed at index a[2][1][1], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2];\n"
              "  a[1][2][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Array 'a[2][2][2]' accessed at index a[1][2][1], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2][2];\n"
              "  a[1][2][1][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Array 'a[2][2][2][2]' accessed at index a[1][2][1][1], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2];\n"
              "  a[1][1][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Array 'a[2][2][2]' accessed at index a[1][1][2], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "  char a[10][10][10];\n"
              "  a[2*3][4*3][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:4]: (error) Array 'a[10][10][10]' accessed at index a[6][12][2], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "  char a[10][10][10];\n"
              "  a[6][40][10] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:4]: (error) Array 'a[10][10][10]' accessed at index a[6][40][10], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "  char a[1][1][1];\n"
              "  a[2][2][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:4]: (error) Array 'a[1][1][1]' accessed at index a[2][2][2], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "  char a[6][6][6];\n"
              "  a[6][6][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:4]: (error) Array 'a[6][6][6]' accessed at index a[6][6][2], which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "  int a[2][2];\n"
              "  p = &a[2][0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // unknown dim..
        check("void f()\n"
              "{\n"
              "  int a[2][countof(x)] = {{1,2},{3,4}};\n"
              "  a[0][0] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void draw_quad(float z)  {\n"
              "    int i;\n"
              "    float (*vertices)[2][4];\n"
              "    vertices[0][0][0] = z;\n"
              "    vertices[0][0][1] = z;\n"
              "    vertices[1][0][0] = z;\n"
              "    vertices[1][0][1] = z;\n"
              "    vertices[2][0][0] = z;\n"
              "    vertices[2][0][1] = z;\n"
              "    vertices[3][0][0] = z;\n"
              "    vertices[3][0][1] = z;\n"
              "    for (i = 0; i < 4; i++) {\n"
              "        vertices[i][0][2] = z;\n"
              "        vertices[i][0][3] = 1.0;\n"
              "        vertices[i][1][0] = 2.0;\n"
              "        vertices[i][1][1] = 3.0;\n"
              "        vertices[i][1][2] = 4.0;\n"
              "        vertices[i][1][3] = 5.0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        {
            check("int foo() {\n"
                  "  const size_t A = 4;\n"
                  "  const size_t B = 2;\n"
                  "  extern int stuff[A][B];\n"
                  "  return stuff[0][1];\n"
                  "}");
            ASSERT_EQUALS("", errout_str());

            // TODO: better handling of VLAs in symboldatabase. should be
            //       possible to use ValueFlow values.
            check("int foo() {\n"
                  "  const size_t A = 4;\n"
                  "  const size_t B = 2;\n"
                  "  extern int stuff[A][B];\n"
                  "  return stuff[0][1];\n"
                  "}");
            TODO_ASSERT_EQUALS("error", "", errout_str());
        }
    }

    void array_index_switch_in_for() {
        check("void f()\n"
              "{\n"
              " int ar[10];\n"
              " for (int i = 0; i < 10; ++i)\n"
              " {\n"
              "  switch(i)\n"
              "  {\n"
              "   case 9:\n"
              "    ar[i] = 0;\n"
              "    break;\n"
              "   default:\n"
              "    ar[i] = ar[i+1];\n"
              "    break;\n"
              "  };\n"
              " }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              " int ar[10];\n"
              " for (int i = 0; i < 10; ++i)\n"
              " {\n"
              "  switch(i)\n"
              "  {\n"
              "   case 8:\n"
              "    ar[i] = 0;\n"
              "    break;\n"
              "   default:\n"
              "    ar[i] = ar[i+1];\n"
              "    break;\n"
              "  };\n"
              " }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:12]: (error) Array index out of bounds.\n", "", errout_str());
    }

    void array_index_for_in_for() {
        check("void f() {\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 10; ++i) {\n"
              "        for (int j = i; j < 5; ++j) {\n"
              "            a[i] = 0;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_bounds() {
        // #10275
        check("int a[10];\n"
              "void f(int i) {\n"
              "  if (i >= 0 && i < 10) {}\n"
              "  a[i] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:19] -> [test.cpp:4:4]: (warning) Either the condition 'i<10' is redundant or the array 'a[10]' is accessed at index 10, which is out of bounds. [arrayIndexOutOfBoundsCond]\n"
                      "[test.cpp:3:9] -> [test.cpp:4:4]: (warning) Either the condition 'i>=0' is redundant or the array 'a[10]' is accessed at index -1, which is out of bounds. [negativeIndex]\n",
                      errout_str());
    }

    void array_index_calculation() {
        // #1193 - false negative: array out of bounds in loop when there is calculation
        check("void f()\n"
              "{\n"
              "    char data[8];\n"
              "    for (int i = 19; i < 36; ++i) {\n"
              "        data[i/2] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Array 'data[8]' accessed at index 17, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // #2199 - false negative: array out of bounds in loop when there is calculation
        check("void f()\n"
              "{\n"
              "    char arr[5];\n"
              "    for (int i = 0; i < 5; ++i) {\n"
              "        arr[i + 7] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Array 'arr[5]' accessed at index 11, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_negative1() {
        // #948 - array index out of bound not detected 'a[-1] = 0'
        check("void f()\n"
              "{\n"
              "    char data[8];\n"
              "    data[-1] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Array 'data[8]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    char data[8][4];\n"
              "    data[5][-1] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Array 'data[8][4]' accessed at index data[*][-1], which is out of bounds. [negativeIndex]\n", errout_str());

        // #1614 - negative index is ok for pointers
        check("void foo(char *p)\n"
              "{\n"
              "    p[-1] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    char s[] = \"abc\";\n"
              "    char *p = s + strlen(s);\n"
              "    if (p[-1]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ticket #1850
        check("int f(const std::map<int, std::map<int,int> > &m)\n"
              "{\n"
              "    return m[0][-1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_negative2() { // ticket #3063
        check("struct TEST { char a[10]; };\n"
              "void foo() {\n"
              "    TEST test;\n"
              "    test.a[-1] = 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Array 'test.a[10]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());
    }

    void array_index_negative3() {
        check("int f(int i) {\n"
              "    int p[2] = {0, 0};\n"
              "    if(i >= 2)\n"
              "        return 0;\n"
              "    else if(i == 0)\n"
              "        return 0;\n"
              "    return p[i - 1];\n"
              "}\n"
              "void g(int i) {\n"
              "    if( i == 0 )\n"
              "        return f(i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_negative4()
    {
        check("void f(void) {\n"
              "    int buf[64]={};\n"
              "    int i;\n"
              "    for(i=0; i <16; ++i){}\n"
              "    for(; i < 24; ++i){ buf[i] = buf[i-16];}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_negative5() // #10526
    {
        check("int i;\n"
              "std::vector<int> v;\n"
              "bool f() {\n"
              "    if (i != 0) {\n"
              "        if (v.begin() != v.end()) {\n"
              "            if (i < 0)\n"
              "                return false;\n"
              "            const int a[4] = { 0, 1, 2, 3 };\n"
              "            return a[i - 1] > 0;\n"
              "        }\n"
              "    }\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // #11349
    void array_index_negative6()
    {
        check("void f(int i) {\n"
              "  int j = i;\n"
              "  const int a[5] = {};\n"
              "  const int k = j < 0 ? 0 : j;\n"
              "  (void)a[k];\n"
              "  if (i == -3) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // #5685
    void array_index_negative7()
    {
        check("void f() {\n"
              "    int i = -9;\n"
              "    int a[5];\n"
              "    for (; i < 5; i++)\n"
              "        a[i] = 1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'a[5]' accessed at index -9, which is out of bounds. [negativeIndex]\n", errout_str());
    }

    // #11651
    void array_index_negative8()
    {
        check("unsigned g(char*);\n"
              "void f() {\n"
              "    char buf[10];\n"
              "    unsigned u = g(buf);\n"
              "    for (int i = u, j = sizeof(i); --i >= 0;)\n"
              "        char c = buf[i];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    // #8075
    void array_index_negative9()
    {
        check("int g(int i) {\n"
              "    if (i < 10)\n"
              "        return -1;\n"
              "    return 0;\n"
              "}\n"
              "void f() {\n"
              "    int a[] = { 1, 2, 3 };\n"
              "    printf(\"%d\\n\", a[g(4)]);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:21]: (error) Array 'a[3]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());
    }

    // #11844
    void array_index_negative10()
    {
        check("struct S { int a[4]; };\n"
              "void f(S* p, int k) {\n"
              "  int m = 3;\n"
              "  if (k)\n"
              "    m = 2;\n"
              "  for (int j = m + 1; j <= 4; j++)\n"
              "    p->a[j-1];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_for_decr() {
        check("void f()\n"
              "{\n"
              "    char data[8];\n"
              "    for (int i = 10; i > 0; --i) {\n"
              "        data[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Array 'data[8]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    char val[5];\n"
              "    for (unsigned int i = 3; i < 5; --i) {\n"
              "        val[i+1] = val[i];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    char val[5];\n"
              "    for (int i = 3; i < 5; --i) {\n"
              "        val[i+1] = val[i];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Array 'val[5]' accessed at index -9994, which is out of bounds. [negativeIndex]\n"
                      "[test.cpp:5:23]: (error) Array 'val[5]' accessed at index -9995, which is out of bounds. [negativeIndex]\n", errout_str());
    }


    void array_index_varnames() {
        check("struct A {\n"
              "    char data[4];\n"
              "    struct B { char data[3]; };\n"
              "    B b;\n"
              "};\n"
              "\n"
              "void f()\n"
              "{\n"
              "    A a;\n"
              "    a.data[3] = 0;\n"
              "    a.b.data[2] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #1586
        check("struct A {\n"
              "    char data[4];\n"
              "    struct B { char data[3]; };\n"
              "    B b;\n"
              "};\n"
              "\n"
              "void f()\n"
              "{\n"
              "    A a;\n"
              "    a.data[4] = 0;\n"
              "    a.b.data[3] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:11]: (error) Array 'a.data[4]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n"
                      "[test.cpp:11:13]: (error) Array 'a.b.data[3]' accessed at index 3, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_for_andand_oror() {  // #3907 - using && or ||
        // extracttests.start: volatile int y;

        check("void f() {\n"
              "    char data[2];\n"
              "    int x;\n"
              "    for (x = 0; x < 10 && y; x++) {\n"
              "        data[x] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Array 'data[2]' accessed at index 9, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    char data[2];\n"
              "    int x;\n"
              "    for (x = 0; x < 10 || y; x++) {\n"
              "        data[x] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Array 'data[2]' accessed at index 9, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    char data[2];\n"
              "    int x;\n"
              "    for (x = 0; x <= 10 && y; x++) {\n"
              "        data[x] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Array 'data[2]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    char data[2];\n"
              "    int x;\n"
              "    for (x = 0; y && x <= 10; x++) {\n"
              "        data[x] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Array 'data[2]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("int f() {\n" // #9126
              "    int i, c;\n"
              "    char* words[100] = {0};\n"
              "    g(words);\n"
              "    for (i = c = 0; (i < N) && (c < 1); i++) {\n"
              "        if (words[i][0] == '|')\n"
              "            c++;\n"
              "     }\n"
              "    return c;\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_for_continue() {
        // #3913
        check("void f() {\n"
              "    int a[2];\n"
              "    for (int i = 0; i < 2; ++i) {\n"
              "        if (i == 0) {\n"
              "            continue;\n"
              "        }\n"
              "        a[i - 1] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: int maybe();
        check("void f() {\n"
              "    int a[2];\n"
              "    for (int i = 0; i < 2; ++i) {\n"
              "        if (maybe()) {\n"
              "            continue;\n"
              "        }\n"
              "        a[i - 1] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:10]: (error) Array 'a[2]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());
    }

    void array_index_for() {
        // Ticket #2370 - No false negative when there is no "break"
        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 20; ++i) {\n"
              "        if (i==1) {\n"
              "        }\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:10]: (error) Array 'a[10]' accessed at index 19, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // Ticket #2385 - No false positive
        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 20; ++i) {\n"
              "        if (i<10) {\n"
              "        } else {\n"
              "            a[i-10] = 0;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #3893 - start value out of bounds
        // extracttests.start: int maybe(); int dostuff();
        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 10; maybe(); dostuff()) {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'a[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // #7686
        check("char f() {\n"
              "    char buf[10];\n"
              "    const bool a = true, b = true;\n"
              "    for (int i = 0; i < (a && b ? 11 : 10); ++i)\n"
              "        buf[i] = 0;\n"
              "    return buf[0];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Array 'buf[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    const int a[10] = {};\n"
              "    for (int n = 0; 1; ++n) {\n"
              "        if (a[n] < 1) {\n"
              "            switch (a[n]) {\n"
              "            case 0:\n"
              "                break;\n"
              "            }\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:14]: (error) Array 'a[10]' accessed at index 9998, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_for_neq() {
        // Ticket #2211 - for loop using != in the condition
        check("void f() {\n"
              "    int a[5];\n"
              "    for (int i = 0; i != 10; ++i) {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'a[5]' accessed at index 9, which is out of bounds. [arrayIndexOutOfBounds]\n",
                      errout_str());
    }

    void array_index_for_question() {
        // Ticket #2561 - using ?: inside for loop
        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i != 10; ++i) {\n"
              "        i == 0 ? 0 : a[i-1];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i != 10; ++i) {\n"
              "        some_condition ? 0 : a[i-1];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:31]: (error) Array 'a[10]' accessed at index -1, which is out of bounds. [negativeIndex]\n",
                      errout_str());

        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i != 10; ++i) {\n"
              "        i==0 ? 0 : a[i-1];\n"
              "        a[i-1] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'a[10]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());
    }

    void array_index_for_varid0() { // #4228: No varid for counter variable
        check("void f() {\n"
              "   char a[10];\n"
              "   for (i=0; i<10; i++);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_vla_for() {
        // #3221 - access VLA inside for
        check("void f(int len) {\n"
              "    char a[len];\n"
              "    for (int i=0; i<7; ++i) {\n"
              "        a[0] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_extern() {
        // Ticket #1684. FP when using 'extern'.
        check("extern char arr[15];\n"
              "char arr[15] = \"abc\";");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_cast() {
        // Ticket #2841. FP when using cast.

        // Different types => no error
        check("void f1(char *buf) {\n"
              "    buf[4] = 0;\n"
              "}\n"
              "void f2() {\n"
              "    int x[2];\n"
              "    f1(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Same type => error
        check("void f1(const char buf[]) {\n"
              "    char c = buf[4];\n"
              "}\n"
              "void f2() {\n"
              "    char x[2];\n"
              "    f1(x);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:2]: (error) Array 'x[2]' accessed at index 4, which is out of bounds.\n",
                           "",
                           errout_str());
    }

    void array_index_string_literal() {
        check("void f() {\n"
              "    const char *str = \"abc\";\n"
              "    bar(str[10]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Array 'str[4]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    const char *str = \"abc\";\n"
              "    bar(str[4]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Array 'str[4]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    const char *str = \"abc\";\n"
              "    bar(str[3]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    const char *str = \"a\tc\";\n"
              "    bar(str[4]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Array 'str[4]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n" // #6973
              "    const char *name = \"\";\n"
              "    if ( name[0] == 'U' ? name[1] : 0) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int main(int argc, char **argv) {\n"
              "    char str[6] = \"\\0\";\n"
              "    unsigned short port = 65535;\n"
              "    snprintf(str, sizeof(str), \"%hu\", port);\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("int f(int x) {\n" // #11020
              "    const char* p = (x == 0 ? \"12345\" : \"ABC\");\n"
              "    int s = 0;\n"
              "    for (int i = 0; p[i]; i++)\n"
              "        s += p[i];\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_same_struct_and_var_name() {
        // don't throw internal error
        check("struct tt {\n"
              "    char name[21];\n"
              "} ;\n"
              "void doswitch(struct tt *x)\n"
              "{\n"
              "    struct tt *tt=x;\n"
              "    tt->name;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // detect error
        check("struct tt {\n"
              "    char name[21];\n"
              "} ;\n"
              "void doswitch(struct tt *x)\n"
              "{\n"
              "    struct tt *tt=x;\n"
              "    tt->name[22] = 123;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:13]: (error) Array 'tt->name[21]' accessed at index 22, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_valueflow() {
        check("void f(int i) {\n"
              "    char str[3];\n"
              "    str[i] = 0;\n"
              "    if (i==10) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:10] -> [test.cpp:3:8]: (warning) Either the condition 'i==10' is redundant or the array 'str[3]' is accessed at index 10, which is out of bounds. [arrayIndexOutOfBoundsCond]\n", errout_str());

        check("void f(int i) {\n"
              "    char str[3];\n"
              "    str[i] = 0;\n"
              "    switch (i) {\n"
              "    case 10: break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5] -> [test.cpp:3:8]: (warning) Either the switch case 'case 10' is redundant or the array 'str[3]' is accessed at index 10, which is out of bounds. [arrayIndexOutOfBoundsCond]\n", errout_str());

        check("void f() {\n"
              "    char str[3];\n"
              "    str[((unsigned char)3) - 1] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"  // #5416 FP
              "    char *str[3];\n"
              "    do_something(&str[0][5]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class X { static const int x[100]; };\n" // #6070
              "const int X::x[100] = {0};");
        ASSERT_EQUALS("", errout_str());

        check("namespace { class X { static const int x[100]; };\n" // #6232
              "const int X::x[100] = {0}; }");
        ASSERT_EQUALS("", errout_str());

        check("class ActorSprite { static ImageSet * targetCursorImages[2][10]; };\n"
              "ImageSet *ActorSprite::targetCursorImages[2][10];");
        ASSERT_EQUALS("", errout_str());

        check("int f(const std::size_t s) {\n" // #10130
              "    const char a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };\n"
              "    return (s > sizeof(a)) ? 11 : (int)a[s];\n"
              "}\n"
              "int g() {\n"
              "    return f(16);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:15] -> [test.cpp:3:41]: (warning) Either the condition 's>sizeof(a)' is redundant or the array 'a[16]' is accessed at index 16, which is out of bounds. [arrayIndexOutOfBoundsCond]\n",
                      errout_str());

        check("void f(int fd) {\n" // #12318
              "    char buf[10];\n"
              "    int size = 0;\n"
              "    int pos = -1;\n"
              "    do {\n"
              "        pos++;\n"
              "        size = read(fd, &buf[pos], 1);\n"
              "    } while (size > 0);\n"
              "    buf[pos] = '\\0';\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

    }

    void array_index_valueflow_pointer() {
        check("void f() {\n"
              "  int a[10];\n"
              "  int *p = a;\n"
              "  p[20] = 0;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (error) Array 'a[10]' accessed at index 20, which is out of bounds.\n", "", errout_str());

        {
            // address of
            check("void f() {\n"
                  "  int a[10];\n"
                  "  int *p = a;\n"
                  "  p[10] = 0;\n"
                  "}");
            TODO_ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4:6]: (error) Array 'a[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", "", errout_str());

            check("void f() {\n"
                  "  int a[10];\n"
                  "  int *p = a;\n"
                  "  dostuff(&p[10]);\n"
                  "}");
            ASSERT_EQUALS("", errout_str());
        }

        check("void f() {\n"
              "  int a[X];\n" // unknown size
              "  int *p = a;\n"
              "  p[20] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int a[2];\n"
              "  char *p = (char *)a;\n" // cast
              "  p[4] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_function_parameter() {
        check("void f(char a[10]) {\n"
              "  a[20] = 0;\n" // <- cppcheck warn here even though it's not a definite access out of bounds
              "}");
        ASSERT_EQUALS("[test.cpp:2:4]: (error) Array 'a[10]' accessed at index 20, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f(char a[10]) {\n" // #6353 - reassign 'a'
              "  a += 4;\n"
              "  a[-1] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char a[10]) {\n"
              "  a[0] = 0;\n"
              "  a[-1] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:4]: (error) Array 'a[10]' accessed at index -1, which is out of bounds. [negativeIndex]\n", errout_str());
    }

    void array_index_enum_array() { // #8439
        check("enum E : unsigned int { e1, e2 };\n"
              "void f() {\n"
              "    E arrE[] = { e1, e2 };\n"
              "    arrE[sizeof(arrE)] = e1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Array 'arrE[2]' accessed at index 8, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void array_index_container() { // #9386
        check("constexpr int blockLen = 10;\n"
              "void foo(std::array<uint8_t, blockLen * 2>& a) {\n"
              "    a[2] = 2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_two_for_loops() {
        check("bool b();\n"
              "void f()\n"
              "{\n"
              "    int val[50];\n"
              "    int i, sum=0;\n"
              "    for (i = 1; b() && i < 50; i++)\n"
              "        sum += val[i];\n"
              "    if (i < 50)\n"
              "        sum -= val[i];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool b();\n"
              "void f()\n"
              "{\n"
              "    int val[50];\n"
              "    int i, sum=0;\n"
              "    for (i = 1; b() && i < 50; i++)\n"
              "        sum += val[i];\n"
              "    for (; i < 50;) {\n"
              "        sum -= val[i];\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool b();\n"
              "void f()\n"
              "{\n"
              "    int val[50];\n"
              "    int i, sum=0;\n"
              "    for (i = 1; b() && i < 50; i++)\n"
              "        sum += val[i];\n"
              "    for (; i < 50; i++)\n"
              "        sum -= val[i];\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void array_index_new() { // #7690
        check("void f() {\n"
              "    int* z = new int;\n"
              "    for (int n = 0; n < 8; ++n)\n"
              "        z[n] = 0;\n"
              "    delete[] z;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'z[1]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    int* z = new int(1);\n"
              "    for (int n = 0; n < 8; ++n)\n"
              "        z[n] = 0;\n"
              "    delete[] z;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'z[1]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    int* z = new int{};\n"
              "    for (int n = 0; n < 8; ++n)\n"
              "        z[n] = 0;\n"
              "    delete[] z;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'z[1]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    int* z = new int[5];\n"
              "    for (int n = 0; n < 8; ++n)\n"
              "        z[n] = 0;\n"
              "    delete[] z;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'z[5]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void g() {\n"
              "    int* z = new int[5]();\n"
              "    for (int n = 0; n < 8; ++n)\n"
              "        z[n] = 1;\n"
              "    delete[] z;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'z[5]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void h() {\n"
              "    int** z = new int* [5];\n"
              "    for (int n = 0; n < 8; ++n)\n"
              "        z[n] = nullptr;\n"
              "    delete[] z;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'z[5]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void h() {\n"
              "    int** z = new int* [5]();\n"
              "    for (int n = 0; n < 8; ++n)\n"
              "        z[n] = nullptr;\n"
              "    delete[] z;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'z[5]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void h() {\n"
              "    int** z = new int* [5]{};\n"
              "    for (int n = 0; n < 8; ++n)\n"
              "        z[n] = nullptr;\n"
              "    delete[] z;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'z[5]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void h() {\n"
              "    int** z = new int* [5]{ 0 };\n"
              "    for (int n = 0; n < 8; ++n)\n"
              "        z[n] = nullptr;\n"
              "    delete[] z;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: (error) Array 'z[5]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void buffer_overrun_2_struct() {
        check("struct ABC\n"
              "{\n"
              "    char str[5];\n"
              "};\n"
              "\n"
              "static void f(struct ABC *abc)\n"
              "{\n"
              "    strcpy( abc->str, \"abcdef\" );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:16]: (error) Buffer is accessed out of bounds: abc->str [bufferAccessOutOfBounds]\n", errout_str());

        check("struct ABC\n"
              "{\n"
              "    char str[5];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    strcpy( abc.str, \"abcdef\" );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:16]: (error) Buffer is accessed out of bounds: abc.str [bufferAccessOutOfBounds]\n", errout_str());

        check("struct ABC\n"
              "{\n"
              "    char str[5];\n"
              "};\n"
              "\n"
              "static void f(struct ABC &abc)\n"
              "{\n"
              "    strcpy( abc.str, \"abcdef\" );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:16]: (error) Buffer is accessed out of bounds: abc.str [bufferAccessOutOfBounds]\n", errout_str());

        check("static void f()\n"
              "{\n"
              "    struct ABC\n"
              "    {\n"
              "        char str[5];\n"
              "    } abc;\n"
              "    strcpy( abc.str, \"abcdef\" );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:16]: (error) Buffer is accessed out of bounds: abc.str [bufferAccessOutOfBounds]\n", errout_str());

        check("static void f()\n"
              "{\n"
              "    struct ABC\n"
              "    {\n"
              "        char str[5];\n"
              "    };\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    strcpy( abc->str, \"abcdef\" );\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:16]: (error) Buffer is accessed out of bounds: abc->str [bufferAccessOutOfBounds]\n", errout_str());
    }


    void buffer_overrun_3() {
        check("int a[10];\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    int i;\n"
              "    for (i = 0; i <= 10; ++i)\n"
              "        a[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:10]: (error) Array 'a[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("struct S { int b; } static e[1];\n" // #11052
              "int f() { return e[1].b; }\n");
        ASSERT_EQUALS("[test.cpp:2:19]: (error) Array 'e[1]' accessed at index 1, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }


    void buffer_overrun_4() {
        check("void foo()\n"
              "{\n"
              "    const char *p[2];\n"
              "    for (int i = 0; i < 8; ++i)\n"
              "        p[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'p[2]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // No false positive
        check("void foo(int x, int y)\n"
              "{\n"
              "    const char *p[2];\n"
              "    const char *s = y + p[1];\n"
              "    p[1] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // There is no error here
        check("void f1(char *s,int size)\n"
              "{\n"
              "  if( size > 10 ) strcpy(s,\"abc\");\n"
              "}\n"
              "void f2()\n"
              "{\n"
              "  char s[3];\n"
              "  f1(s,20);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:3]: (error) Buffer is accessed out of bounds.\n", "", errout_str());

        check("void f1(char *s,int size)\n"
              "{\n"
              "  if( size > 10 ) strcpy(s,\"abc\");\n"
              "}\n"
              "void f2()\n"
              "{\n"
              "  char s[3];\n"
              "  f1(s,3);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_5() {
        check("void f()\n"
              "{\n"
              "    char n[5];\n"
              "    sprintf(n, \"d\");\n"
              "    printf(\"hello!\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_6() {
        check("void f()\n"
              "{\n"
              "   char n[5];\n"
              "   strcat(n, \"abc\");\n"
              "   strcat(n, \"def\");\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Buffer is accessed out of bounds: n\n", "", errout_str());

        check("void f()\n" // #12489
              "{\n"
              "   char d[3] = {};\n"
              "   strcat(d, \"12345678\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Buffer is accessed out of bounds: d [bufferAccessOutOfBounds]\n", errout_str());

        check("void f()\n"
              "{\n"
              "   char d[3] = \"ab\"; \n"
              "   strcat(d, \"c\");\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4:11]: (error) Buffer is accessed out of bounds: d [bufferAccessOutOfBounds]\n", "", errout_str());
    }

    void buffer_overrun_7() {
        // ticket #731
        check("void f()\n"
              "{\n"
              "    char a[2];\n"
              "    strcpy(a, \"a\\0\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_8() {
        // ticket #714
        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; i = i + 100)\n"
              "    {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; i = 100 + i)\n"
              "    {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_9() {
        // ticket #738
        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; )\n"
              "    {\n"
              "        a[i] = 0;\n"
              "        i += 100;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_10() {
        // ticket #740
        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (int i = 0; i < 4; i++)\n"
              "    {\n"
              "        char b = a[i];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_11() {
        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (float i=0; i<10.0;i=i+0.1)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (float i=0; i<10.0;i=0.1+i)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_15() { // ticket #1787
        check("class A : public B {\n"
              "    char val[2];\n"
              "    void f(int i, int ii);\n"
              "};\n"
              "void A::f(int i, int ii)\n"
              "{\n"
              "    strcpy(val, \"ab\") ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:12]: (error) Buffer is accessed out of bounds: val [bufferAccessOutOfBounds]\n", errout_str());
    }

    void buffer_overrun_16() {
        // unknown types
        check("void f() {\n"
              "    struct Foo foo[5];\n"
              "    memset(foo, 0, sizeof(foo));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"  // ticket #2093
              "    gchar x[3];\n"
              "    strcpy(x, \"12\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("extern char a[10];\n"
              "void f() {\n"
              "    char b[25] = {0};\n"
              "    std::memcpy(b, a, 10);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_18() { // ticket #2576
        check("class A {\n"
              "    void foo();\n"
              "    bool b[7];\n"
              "};\n"
              "\n"
              "void A::foo() {\n"
              "    for (int i=0; i<6; i++) {\n"
              "        b[i] = b[i+1];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class A {\n"
              "    void foo();\n"
              "    bool b[7];\n"
              "};\n"
              "\n"
              "void A::foo() {\n"
              "    for (int i=0; i<7; i++) {\n"
              "        b[i] = b[i+1];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:17]: (error) Array 'b[7]' accessed at index 7, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void buffer_overrun_19() { // #2597 - class member with unknown type
        check("class A {\n"
              "public:\n"
              "    u8 buf[10];\n"
              "    A();"
              "};\n"
              "\n"
              "A::A() {\n"
              "    memset(buf, 0, 10);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_21() {
        check("void foo()\n"
              "{ { {\n"
              "    char dst[4];\n"
              "    const char *src = \"AAAAAAAAAAAAAAAAAAAAA\";\n"
              "    for (size_t i = 0; i <= 4; i++)\n"
              "        dst[i] = src[i];\n"
              "} } }");
        ASSERT_EQUALS("[test.cpp:6:12]: (error) Array 'dst[4]' accessed at index 4, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void buffer_overrun_24() { // index variable is changed in for-loop
        // ticket #4106
        check("void main() {\n"
              "   int array[] = {1,2};\n"
              "   int x = 0;\n"
              "   for( int i = 0; i<6; ) {\n"
              "      x += array[i];\n"
              "       i++;  }\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());

        // ticket #4096
        check("void main() {\n"
              "   int array[] = {1,2};\n"
              "   int x = 0;\n"
              "   for( int i = 0; i<6; ) {\n"
              "      x += array[i++];\n"
              "   }\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

    void buffer_overrun_26() { // ticket #4432 (segmentation fault)
        check("extern int split();\n"
              "void regress() {\n"
              "    char inbuf[1000];\n"
              "    char *f[10];\n"
              "    split(inbuf, f, 10, \"\t\t\");\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_27() { // ticket #4444 (segmentation fault)
        check("void abc(struct foobar[5]);\n"
              "void main() {\n"
              "struct foobar x[5];\n"
              "abc(x);\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    // #7083: false positive: typedef and initialization with strings
    void buffer_overrun_29() {
        check("typedef char testChar[10];\n"
              "int main(){\n"
              "  testChar tc1 = \"\";\n"
              "  tc1[5]='a';\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    // #6367
    void buffer_overrun_30() {
        check("struct S { int m[9]; };\n"
              "int f(S * s) {\n"
              "    return s->m[sizeof(s->m)];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Array 's->m[9]' accessed at index 36, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void buffer_overrun_31() {
        check("void f(WhereInfo *pWInfo, int *aiCur) {\n"
              "  memcpy(aiCur, pWInfo->aiCurOnePass, sizeof(int)*2);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_32() {
        // destination size is too small
        check("void f(void) {\n"
              "    const char src[3] = \"abc\";\n"
              "    char dest[1] = \"a\";\n"
              "    (void)strxfrm(dest,src,1);\n"
              "    (void)strxfrm(dest,src,2);\n"// <<
              "}");
        ASSERT_EQUALS("[test.cpp:5:19]: (error) Buffer is accessed out of bounds: dest [bufferAccessOutOfBounds]\n", errout_str());
        // destination size is too small
        check("void f(void) {\n"
              "    const char src[3] = \"abc\";\n"
              "    char dest[2] = \"ab\";\n"
              "    (void)strxfrm(dest,src,1);\n"
              "    (void)strxfrm(dest,src,2);\n"
              "    (void)strxfrm(dest,src,3);\n" // <<
              "}");
        ASSERT_EQUALS("[test.cpp:6:19]: (error) Buffer is accessed out of bounds: dest [bufferAccessOutOfBounds]\n", errout_str());
        // source size is too small
        check("void f(void) {\n"
              "    const char src[2] = \"ab\";\n"
              "    char dest[3] = \"abc\";\n"
              "    (void)strxfrm(dest,src,1);\n"
              "    (void)strxfrm(dest,src,2);\n"
              "    (void)strxfrm(dest,src,3);\n" // <<
              "}");
        ASSERT_EQUALS("[test.cpp:6:24]: (error) Buffer is accessed out of bounds: src [bufferAccessOutOfBounds]\n", errout_str());
        // source size is too small
        check("void f(void) {\n"
              "    const char src[1] = \"a\";\n"
              "    char dest[3] = \"abc\";\n"
              "    (void)strxfrm(dest,src,1);\n"
              "    (void)strxfrm(dest,src,2);\n" // <<
              "}");
        ASSERT_EQUALS("[test.cpp:5:24]: (error) Buffer is accessed out of bounds: src [bufferAccessOutOfBounds]\n", errout_str());
    }

    void buffer_overrun_33() { // #2019
        check("int f() {\n"
              "   int z[16];\n"
              "   for (int i=0; i<20; i++)\n"
              "      for (int j=0; j<20; j++)\n"
              "          z[i] = 0;\n"
              "   return z[0];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:12]: (error) Array 'z[16]' accessed at index 19, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void buffer_overrun_34() { // #11035
        check("struct S {\n"
              "    std::vector<int> v;\n"
              "    int a[15] = {};\n"
              "    int g() const { return v.size(); }\n"
              "    int f(int i) const {\n"
              "        if (i < 0 || i >= g())\n"
              "            return 0;\n"
              "        return a[i];\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_35() { // #2304
        check("void f() {\n"
              "    char* q = \"0123456789\";\n"
              "    char* p = (char*)malloc(sizeof(q) + 1);\n"
              "    strcpy(p, q);\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Buffer is accessed out of bounds: p [bufferAccessOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    char* q = \"0123456789\";\n"
              "    char* p = (char*)malloc(1);\n"
              "    strcpy(p, q);\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Buffer is accessed out of bounds: p [bufferAccessOutOfBounds]\n", errout_str());

        check("typedef struct { char buf[1]; } S;\n"
              "S* f() {\n"
              "    S* s = NULL;\n"
              "    s = (S*)malloc(sizeof(S) + 10);\n"
              "    sprintf((char*)s->buf, \"abc\");\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_36() { // #11708
        check("void f(double d) {\n"
              "    char a[80];\n"
              "    sprintf(a, \"%2.1f\", d);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_errorpath() {
        setMultiline();
        Settings s = settings0;
        s.templateLocation = "{file}:{line}:note:{info}";

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    memset(p, 0, 20);\n"
              "}", s);
        ASSERT_EQUALS("[test.cpp:3:12]: error: Buffer is accessed out of bounds: p [bufferAccessOutOfBounds]\n"
                      "[test.cpp:2:13]: note: Assign p, buffer with size 10\n"
                      "[test.cpp:3:12]: note: Buffer overrun\n", errout_str());
    }

    void buffer_overrun_bailoutIfSwitch() {
        // No false positive
        check("void f1(char *s) {\n"
              "    if (x) s[100] = 0;\n"
              "}\n"
              "\n"
              "void f2() {\n"
              "    char a[10];\n"
              "    f1(a);"
              "}");
        ASSERT_EQUALS("", errout_str());

        // No false positive
        check("void f1(char *s) {\n"
              "    if (x) return;\n"
              "    s[100] = 0;\n"
              "}\n"
              "\n"
              "void f2() {\n"
              "    char a[10];\n"
              "    f1(a);"
              "}");
        ASSERT_EQUALS("", errout_str());

        // No false negative
        check("void f1(char *s) {\n"
              "    if (x) { }\n"
              "    s[100] = 0;\n"
              "}\n"
              "\n"
              "void f2() {\n"
              "    char a[10];\n"
              "    f1(a);"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:3]: (error) Array 'a[10]' accessed at index 100, which is out of bounds.\n", "", errout_str());
    }

    void buffer_overrun_function_array_argument() {
        setMultiline();

        check("void f(char a[10]);\n"
              "void g() {\n"
              "    char a[2];\n"
              "    f(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: warning: Buffer 'a' is too small, the function 'f' expects a bigger buffer in 1st argument [argumentSize]\n"
                      "[test.cpp:4:5]: note: Function 'f' is called\n"
                      "[test.cpp:1:13]: note: Declaration of 1st function argument.\n"
                      "[test.cpp:3:10]: note: Passing buffer 'a' to function that is declared here\n"
                      "[test.cpp:4:5]: note: Buffer 'a' is too small, the function 'f' expects a bigger buffer in 1st argument\n", errout_str());

        check("void f(float a[10][3]);\n"
              "void g() {\n"
              "    float a[2][3];\n"
              "    f(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: warning: Buffer 'a' is too small, the function 'f' expects a bigger buffer in 1st argument [argumentSize]\n"
                      "[test.cpp:4:5]: note: Function 'f' is called\n"
                      "[test.cpp:1:14]: note: Declaration of 1st function argument.\n"
                      "[test.cpp:3:11]: note: Passing buffer 'a' to function that is declared here\n"
                      "[test.cpp:4:5]: note: Buffer 'a' is too small, the function 'f' expects a bigger buffer in 1st argument\n", errout_str());

        check("void f(int a[20]);\n"
              "void g() {\n"
              "    int a[2];\n"
              "    f(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: warning: Buffer 'a' is too small, the function 'f' expects a bigger buffer in 1st argument [argumentSize]\n"
                      "[test.cpp:4:5]: note: Function 'f' is called\n"
                      "[test.cpp:1:12]: note: Declaration of 1st function argument.\n"
                      "[test.cpp:3:9]: note: Passing buffer 'a' to function that is declared here\n"
                      "[test.cpp:4:5]: note: Buffer 'a' is too small, the function 'f' expects a bigger buffer in 1st argument\n", errout_str());

        check("void f(int a[]) {\n"
              "  switch (2) {\n"
              "    case 1:\n"
              "      a[1] = 1;\n"
              "    }\n"
              "}\n"
              "int a[1];\n"
              "f(a);\n"
              "");
        ASSERT_EQUALS("", errout_str());

        check("void CreateLeafTex(unsigned char buf[256][2048][4]);\n"
              "void foo() {\n"
              "  unsigned char(* tree)[2048][4] = new unsigned char[256][2048][4];\n"
              "  CreateLeafTex(tree);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a[10]) {\n" // #10069
              "    int i = 0;\n"
              "    for (i = 0; i < 10; i++)\n"
              "        a[i] = i * 2;\n"
              "}\n"
              "void g() {\n"
              "    int b[5];\n"
              "    f(b);\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:5]: warning: Buffer 'b' is too small, the function 'f' expects a bigger buffer in 1st argument [argumentSize]\n"
                      "[test.cpp:8:5]: note: Function 'f' is called\n"
                      "[test.cpp:1:12]: note: Declaration of 1st function argument.\n"
                      "[test.cpp:7:9]: note: Passing buffer 'b' to function that is declared here\n"
                      "[test.cpp:8:5]: note: Buffer 'b' is too small, the function 'f' expects a bigger buffer in 1st argument\n",
                      errout_str());
    }

    void possible_buffer_overrun_1() { // #3035
        check("void foo() {\n"
              "    char * data = (char *)alloca(50);\n"
              "    char src[100];\n"
              "    memset(src, 'C', 99);\n"
              "    src[99] = '\\0';\n"
              "    strcat(data, src);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (warning) Possible buffer overflow if strlen(src) is larger than sizeof(data)-strlen(data).\n", "", errout_str());

        check("void foo() {\n"
              "    char * data = (char *)alloca(100);\n"
              "    char src[100];\n"
              "    memset(src, 'C', 99);\n"
              "    src[99] = '\\0';\n"
              "    strcat(data, src);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char src[100]) {\n"
              "    char * data = (char *)alloca(50);\n"
              "    strcat(data, src);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning) Possible buffer overflow if strlen(src) is larger than sizeof(data)-strlen(data).\n", "", errout_str());

        check("void foo(char src[100]) {\n"
              "    char * data = (char *)alloca(100);\n"
              "    strcat(data, src);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    char * data = (char *)alloca(50);\n"
              "    char src[100];\n"
              "    memset(src, 'C', 99);\n"
              "    src[99] = '\\0';\n"
              "    strcpy(data, src);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (warning) Possible buffer overflow if strlen(src) is larger than or equal to sizeof(data).\n", "", errout_str());

        check("void foo() {\n"
              "    char * data = (char *)alloca(100);\n"
              "    char src[100];\n"
              "    memset(src, 'C', 99);\n"
              "    src[99] = '\\0';\n"
              "    strcpy(data, src);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char src[100]) {\n"
              "    char * data = (char *)alloca(50);\n"
              "    strcpy(data, src);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning) Possible buffer overflow if strlen(src) is larger than or equal to sizeof(data).\n", "", errout_str());

        check("void foo(char src[100]) {\n"
              "    char * data = (char *)alloca(100);\n"
              "    strcpy(data, src);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void buffer_overrun_readSizeFromCfg() {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <podtype name=\"u8\" sign=\"u\" size=\"1\"/>\n"
                                   "  <function name=\"mystrcpy\">\n"
                                   "    <noreturn>false</noreturn>\n"
                                   "    <arg nr=\"1\">\n"
                                   "      <minsize type=\"strlen\" arg=\"2\"/>\n"
                                   "    </arg>\n"
                                   "    <arg nr=\"2\"/>\n"
                                   "  </function>\n"
                                   "</def>";
        const Settings settings = settingsBuilder().libraryxml(xmldata).build();

        // Attempt to get size from Cfg files, no false positives if size is not specified
        check("void f() {\n"
              "  u8 str[256];\n"
              "  mystrcpy(str, \"abcd\");\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  u8 str[2];\n"
              "  mystrcpy(str, \"abcd\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Buffer is accessed out of bounds: str [bufferAccessOutOfBounds]\n", errout_str());

        // The same for structs, where the message comes from a different check
        check("void f() {\n"
              "    struct { u8 str[256]; } ms;\n"
              "    mystrcpy(ms.str, \"abcd\");\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    struct { u8 str[2]; } ms;\n"
              "    mystrcpy(ms.str, \"abcd\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Buffer is accessed out of bounds: ms.str [bufferAccessOutOfBounds]\n", errout_str());
    }

    void buffer_overrun_handle_addr_of_var() { // #7570
        check("void f() {\n"
              "  int i;\n"
              "  memset(i, 0, 1000);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int i;\n"
              "  memset(&i, 0, 1000);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Buffer is accessed out of bounds: &i [bufferAccessOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "  int i[2];\n"
              "  memset(&i, 0, 1000);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Buffer is accessed out of bounds: i [bufferAccessOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "  int i;\n"
              "  memset(&i, 0, sizeof(i));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int i[10];\n"
              "  memset(&i[1], 0, 1000);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3:10]: (error) Buffer is accessed out of bounds: &i[1] [bufferAccessOutOfBounds]\n", "", errout_str());

        check("struct S { int x; };\n" // #8616
              "void f() {\n"
              "    S s;\n"
              "    memset(&s, 0, sizeof(s) * 2);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:12]: (error) Buffer is accessed out of bounds: &s [bufferAccessOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    char x;\n"
              "    memset(&x, 0, 16);\n"
              "}\n"
              "void g(char y) {\n"
              "    char x;\n"
              "    memcpy(&x, &y, 16);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Buffer is accessed out of bounds: &x [bufferAccessOutOfBounds]\n"
                      "[test.cpp:7:12]: (error) Buffer is accessed out of bounds: &x [bufferAccessOutOfBounds]\n"
                      "[test.cpp:7:16]: (error) Buffer is accessed out of bounds: &y [bufferAccessOutOfBounds]\n",
                      errout_str());
    }

    void valueflow_string() { // using ValueFlow string values in checking
        check("char f() {\n"
              "  const char *x = s;\n"
              "  if (cond) x = \"abcde\";\n"
              "  return x[20];\n" // <- array index out of bounds when x is "abcde"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) Array 'x[6]' accessed at index 20, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void pointer_out_of_bounds_1() {
        // extracttests.start: void dostuff(char *);

        check("void f() {\n"
              "    char a[10];\n"
              "    char *p = a + 100;\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:3:17]: (portability) Undefined behaviour, pointer arithmetic 'a+100' is out of bounds. [pointerOutOfBounds]\n", errout_str());

        check("char *f() {\n"
              "    char a[10];\n"
              "    return a + 100;\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:3:14]: (portability) Undefined behaviour, pointer arithmetic 'a+100' is out of bounds. [pointerOutOfBounds]\n", errout_str());

        check("void f(int i) {\n"
              "    char x[10];\n"
              "    if (i == 123) {}\n"
              "    dostuff(x+i);\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:4:14]: (portability) Undefined behaviour, when 'i' is 123 the pointer arithmetic 'x+i' is out of bounds. [pointerOutOfBoundsCond]\n", errout_str());

        check("void f(int i) {\n"
              "    char x[10];\n"
              "    if (i == -1) {}\n"
              "    dostuff(x+i);\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:4:14]: (portability) Undefined behaviour, when 'i' is -1 the pointer arithmetic 'x+i' is out of bounds. [pointerOutOfBoundsCond]\n", errout_str());

        check("void f() {\n" // #6350 - fp when there is cast of buffer
              "  wchar_t buf[64];\n"
              "  p = (unsigned char *) buf + sizeof (buf);\n"
              "}", settings0_p, false);
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    const char   d[] = \"0123456789\";\n"
              "    char *cp = d + 3;\n"
              "    return cp - d;\n"
              "}", settings0_p);
        ASSERT_EQUALS("", errout_str());
    }

    void pointer_out_of_bounds_2() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p += 100;\n"
              "    free(p);"
              "}", settings0_p);
        TODO_ASSERT_EQUALS("[test.cpp:3]: (portability) Undefined behaviour, pointer arithmetic 'p+100' is out of bounds.\n", "", errout_str());

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p += 10;\n"
              "    *p = 0;\n"
              "    free(p);"
              "}", settings0_p);
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) p is out of bounds.\n", "", errout_str());

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p += 10;\n"
              "    p -= 10;\n"
              "    *p = 0;\n"
              "    free(p);"
              "}", settings0_p);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p += 10;\n"
              "    p = p - 1;\n"
              "    *p = 0;\n"
              "    free(p);"
              "}", settings0_p);
        ASSERT_EQUALS("", errout_str());
    }

    void pointer_out_of_bounds_3() {
        check("struct S { int a[10]; };\n"
              "void f(struct S *s) {\n"
              "    int *p = s->a + 100;\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:3:19]: (portability) Undefined behaviour, pointer arithmetic 's->a+100' is out of bounds. [pointerOutOfBounds]\n", errout_str());

        check("template <class T> class Vector\n"
              "{\n"
              "public:\n"
              "    void test() const;\n"
              "    T* data();\n"
              "};\n"
              "template <class T>\n"
              "void Vector<T>::test() const\n"
              "{\n"
              "    const T* PDat = data();\n"
              "    const T* P2 = PDat + 1;\n"
              "    const T* P1 = P2 - 1;\n"
              "}\n"
              "Vector<std::array<long, 2>> Foo;\n", settings0_p);
        ASSERT_EQUALS("", errout_str());
    }

    void pointer_out_of_bounds_4() {
        check("const char* f() {\n"
              "    g(\"Hello\" + 6);\n"
              "}", settings0_p);
        ASSERT_EQUALS("", errout_str());

        check("const char* f() {\n"
              "    g(\"Hello\" + 7);\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:2:15]: (portability) Undefined behaviour, pointer arithmetic '\"Hello\"+7' is out of bounds. [pointerOutOfBounds]\n", errout_str());

        check("const char16_t* f() {\n"
              "    g(u\"Hello\" + 6);\n"
              "}", settings0_p);
        ASSERT_EQUALS("", errout_str());

        check("const char16_t* f() {\n"
              "    g(u\"Hello\" + 7);\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:2:16]: (portability) Undefined behaviour, pointer arithmetic 'u\"Hello\"+7' is out of bounds. [pointerOutOfBounds]\n", errout_str());

        check("void f() {\n" // #4647
              "    int val = 5;\n"
              "    std::string hi = \"hi\" + val;\n"
              "    std::cout << hi << std::endl;\n"
              "}\n", settings0_p);
        ASSERT_EQUALS("[test.cpp:3:27]: (portability) Undefined behaviour, pointer arithmetic '\"hi\"+val' is out of bounds. [pointerOutOfBounds]\n", errout_str());

        check("void f(const char* s, int len) {\n" // #11026
              "    const char* end = s + len;\n"
              "    printf(\"%s, %d\\n\", s, *end);\n"
              "}\n"
              "void g() {\n"
              "    f(\"a\", 1);\n"
              "    f(\"bbb\", 3);\n"
              "}\n", settings0_p);
        ASSERT_EQUALS("", errout_str());

        check("void f(int i, const char* a) {\n" // #11140
              "    (void)a[i];\n"
              "}\n"
              "void g() {\n"
              "    for (int i = 0; \"01234\"[i]; ++i)\n"
              "        f(i, \"56789\");\n"
              "}\n"
              "void h() {\n"
              "    for (int i = 0; \"012\"[i]; ++i)\n"
              "        f(i, \"345\");\n"
              "}\n", settings0_p);
        ASSERT_EQUALS("", errout_str());
    }

    void pointer_out_of_bounds_5() { // #10227
        check("int foo(char str[6]) {\n"
              "    return !((0 && *(\"STRING\" + 14) == 0) || memcmp(str, \"STRING\", 6) == 0);\n"
              "}\n", settings0_p);
        ASSERT_EQUALS("", errout_str());
    }

    void pointer_out_of_bounds_sub() {
        // extracttests.start: void dostuff(char *);

        check("char *f() {\n"
              "    char x[10];\n"
              "    return x-1;\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:3:13]: (portability) Undefined behaviour, pointer arithmetic 'x-1' is out of bounds. [pointerOutOfBounds]\n", errout_str());

        check("void f(int i) {\n"
              "    char x[10];\n"
              "    if (i == 123) {}\n"
              "    dostuff(x-i);\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:3:11] -> [test.cpp:4:14]: (portability) Undefined behaviour, when 'i' is 123 the pointer arithmetic 'x-i' is out of bounds. [pointerOutOfBoundsCond]\n", errout_str());

        check("void f(int i) {\n"
              "    char x[10];\n"
              "    if (i == -20) {}\n"
              "    dostuff(x-i);\n"
              "}", settings0_p);
        TODO_ASSERT_EQUALS("[test.cpp:4]: (portability) Undefined behaviour, when 'i' is -20 the pointer arithmetic 'x-i' is out of bounds.\n", "", errout_str());

        check("void f(const char *x[10]) {\n"
              "    return x-4;\n"
              "}", settings0_p);
        ASSERT_EQUALS("", errout_str());
    }

    void strcat1() {
        check("struct Foo { char a[4]; };\n"
              "void f() {\n"
              "  struct Foo x = {0};\n"
              "  strcat(x.a, \"aa\");\n"
              "  strcat(x.a, \"aa\");\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Buffer is accessed out of bounds.\n", "", errout_str());
    }

    void varid1() {
        check("void foo()\n"
              "{\n"
              "    char str[10];\n"
              "    if (str[0])\n"
              "    {\n"
              "        char str[50];\n"
              "        str[30] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varid2() { // #4764
        check("struct foo {\n"
              "  void bar() { return; }\n"
              "  type<> member[1];\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void assign1() {
        check("char str[3] = {'a', 'b', 'c'};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    str[3] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:8]: (error) Array 'str[3]' accessed at index 3, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void alloc_new() {
        check("void foo()\n"
              "{\n"
              "    char *s; s = new char[10];\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 's[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // ticket #1670 - false negative when using return
        check("char f()\n"
              "{\n"
              "    int *s; s = new int[10];\n"
              "    return s[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Array 's[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("struct Fred { char c[10]; };\n"
              "char f()\n"
              "{\n"
              "    Fred *f; f = new Fred;\n"
              "    return f->c[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:16]: (error) Array 'f->c[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("static const size_t MAX_SIZE = UNAVAILABLE_TO_CPPCHECK;\n"
              "struct Thing { char data[MAX_SIZE]; };\n"
              "char f4(const Thing& t) { return !t.data[0]; }");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "  char * buf; buf = new char[8];\n"
              "  buf[7] = 0;\n"
              "  delete [] buf;\n"
              "  buf = new char[9];\n"
              "  buf[8] = 0;\n"
              "  delete [] buf;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "  char * buf; buf = new char[8];\n"
              "  buf[7] = 0;\n"
              "  delete [] buf;\n"
              "  buf = new char[9];\n"
              "  buf[9] = 0;\n"
              "  delete [] buf;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:6]: (error) Array 'buf[9]' accessed at index 9, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void foo()\n"
              "{\n"
              "    enum E { Size = 10 };\n"
              "    char *s; s = new char[Size];\n"
              "    s[Size] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:6]: (error) Array 's[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void foo()\n"
              "{\n"
              "    enum E { ZERO };\n"
              "    E *e; e = new E[10];\n"
              "    e[10] = ZERO;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:6]: (error) Array 'e[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    // data is allocated with malloc
    void alloc_malloc() {
        check("void foo()\n"
              "{\n"
              "    char *s; s = (char *)malloc(10);\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 's[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // ticket #842
        check("void f() {\n"
              "    int *tab4 = (int *)malloc(20 * sizeof(int));\n"
              "    tab4[20] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Array 'tab4[20]' accessed at index 20, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // ticket #1478
        check("void foo() {\n"
              "    char *p = (char *)malloc(10);\n"
              "    free(p);\n"
              "    p = (char *)malloc(10);\n"
              "    p[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:6]: (error) Array 'p[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        // ticket #1134
        check("void f() {\n"
              "    int *x, i;\n"
              "    x = (int *)malloc(10 * sizeof(int));\n"
              "    x[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 'x[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "  int *tab4; tab4 = malloc(20 * sizeof(int));\n"
              "  tab4[19] = 0;\n"
              "  free(tab4);\n"
              "  tab4 = malloc(21 * sizeof(int));\n"
              "  tab4[20] = 0;\n"
              "  free(tab4);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int *tab4 = malloc(20 * sizeof(int));\n"
              "  tab4[19] = 0;\n"
              "  tab4 = realloc(tab4,21 * sizeof(int));\n"
              "  tab4[20] = 0;\n"
              "  free(tab4);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    enum E { Size = 20 };\n"
              "    E *tab4 = (E *)malloc(Size * 4);\n"
              "    tab4[Size] = Size;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Array 'tab4[20]' accessed at index 20, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    enum E { Size = 20 };\n"
              "    E *tab4 = (E *)malloc(4 * Size);\n"
              "    tab4[Size] = Size;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Array 'tab4[20]' accessed at index 20, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    enum E { ZERO };\n"
              "    E *tab4 = (E *)malloc(20 * sizeof(E));\n"
              "    tab4[20] = ZERO;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Array 'tab4[20]' accessed at index 20, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n" // #8721
              "  unsigned char **cache = malloc(32);\n"
              "  cache[i] = malloc(65536);\n"
              "  cache[i][0xFFFF] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int **a = malloc(2 * sizeof(int*));\n"
              "  for (int i = 0; i < 3; i++)\n"
              "    a[i] = NULL;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 'a[2]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "  int **a = new int*[2];\n"
              "  for (int i = 0; i < 3; i++)\n"
              "    a[i] = NULL;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 'a[2]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void f(const uint8_t* a) {\n" // 10421
              "    uint8_t* p = (uint8_t*)malloc(20U * sizeof(uint8_t));\n"
              "    if (!p) return false;\n"
              "    for (uint8_t i = 1; i < 30; ++i)\n"
              "        p[i] = a[i];\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:10]: (error) Array 'p[20]' accessed at index 29, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    // statically allocated buffer
    void alloc_string() {
        check("void foo()\n"
              "{\n"
              "    const char *s = \"123\";\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 's[4]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void foo()\n"
              "{\n"
              "    char *s; s = \"\";\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 's[1]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());

        check("void foo() {\n"
              "    const char *s = \"\";\n"
              "    s = y();\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n" // #7718
              "{\n"
              "    std::string s = \"123\";\n"
              "    s.resize(100);\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // data is allocated with alloca
    void alloc_alloca() {
        check("void foo()\n"
              "{\n"
              "    char *s = (char *)alloca(10);\n"
              "    s[10] = 0;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4:6]: (error) Array 's[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", "", errout_str());
    }
    /*
        void countSprintfLength() const {
            std::list<const Token*> unknownParameter(1, nullptr);

            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("Hello", unknownParameter));
            ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("s", unknownParameter));
            ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("i", unknownParameter));
            ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("%d", unknownParameter));
            ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("%1d", unknownParameter));
            ASSERT_EQUALS(3, CheckBufferOverrun::countSprintfLength("%2.2d", unknownParameter));
            ASSERT_EQUALS(1, CheckBufferOverrun::countSprintfLength("%s", unknownParameter));
            ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("f%s", unknownParameter));
            ASSERT_EQUALS(1, CheckBufferOverrun::countSprintfLength("%-s", unknownParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%-5s", unknownParameter));
            ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("\\\"", unknownParameter));
            ASSERT_EQUALS(7, CheckBufferOverrun::countSprintfLength("Hello \\0Text", unknownParameter));
            ASSERT_EQUALS(1, CheckBufferOverrun::countSprintfLength("\\0", unknownParameter));
            ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("%%", unknownParameter));
            ASSERT_EQUALS(3, CheckBufferOverrun::countSprintfLength("%d%d", unknownParameter));
            ASSERT_EQUALS(3, CheckBufferOverrun::countSprintfLength("\\\\a%s\\0a", unknownParameter));
            ASSERT_EQUALS(10, CheckBufferOverrun::countSprintfLength("\\\\\\\\Hello%d \\0Text\\\\\\\\", unknownParameter));
            ASSERT_EQUALS(4, CheckBufferOverrun::countSprintfLength("%%%%%d", unknownParameter));

            Token strTok;
            std::list<const Token*> stringAsParameter(1, &strTok);
            strTok.str("\"\"");
            ASSERT_EQUALS(4, CheckBufferOverrun::countSprintfLength("str%s", stringAsParameter));
            strTok.str("\"12345\"");
            ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("str%s", stringAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%-4s", stringAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%-5s", stringAsParameter));
            ASSERT_EQUALS(7, CheckBufferOverrun::countSprintfLength("%-6s", stringAsParameter));
            ASSERT_EQUALS(5, CheckBufferOverrun::countSprintfLength("%.4s", stringAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%.5s", stringAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%.6s", stringAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%5.6s", stringAsParameter));
            ASSERT_EQUALS(7, CheckBufferOverrun::countSprintfLength("%6.6s", stringAsParameter));

            Token numTok;
            numTok.str("12345");
            std::list<const Token*> intAsParameter(1, &numTok);
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%02ld", intAsParameter));
            ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("%08ld", intAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%.2d", intAsParameter));
            ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("%08.2d", intAsParameter));
            TODO_ASSERT_EQUALS(5, 2, CheckBufferOverrun::countSprintfLength("%x", intAsParameter));
            ASSERT_EQUALS(5, CheckBufferOverrun::countSprintfLength("%4x", intAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%5x", intAsParameter));
            ASSERT_EQUALS(5, CheckBufferOverrun::countSprintfLength("%.4x", intAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%.5x", intAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%1.5x", intAsParameter));
            ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%5.1x", intAsParameter));

            Token floatTok;
            floatTok.str("1.12345f");
            std::list<const Token*> floatAsParameter(1, &floatTok);
            TODO_ASSERT_EQUALS(5, 3, CheckBufferOverrun::countSprintfLength("%.2f", floatAsParameter));
            ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("%8.2f", floatAsParameter));
            TODO_ASSERT_EQUALS(5, 3, CheckBufferOverrun::countSprintfLength("%2.2f", floatAsParameter));

            Token floatTok2;
            floatTok2.str("100.12345f");
            std::list<const Token*> floatAsParameter2(1, &floatTok2);
            TODO_ASSERT_EQUALS(7, 3, CheckBufferOverrun::countSprintfLength("%2.2f", floatAsParameter2));
            TODO_ASSERT_EQUALS(7, 3, CheckBufferOverrun::countSprintfLength("%.2f", floatAsParameter));
            TODO_ASSERT_EQUALS(7, 5, CheckBufferOverrun::countSprintfLength("%4.2f", floatAsParameter));

            std::list<const Token*> multipleParams = { &strTok, nullptr, &numTok };
            ASSERT_EQUALS(15, CheckBufferOverrun::countSprintfLength("str%s%d%d", multipleParams));
            ASSERT_EQUALS(26, CheckBufferOverrun::countSprintfLength("str%-6s%08ld%08ld", multipleParams));
        }
     */

    // extracttests.disable

    void minsize_argvalue() {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"mymemset\">\n"
                                   "    <noreturn>false</noreturn>\n"
                                   "    <arg nr=\"1\">\n"
                                   "      <minsize type=\"argvalue\" arg=\"3\"/>\n"
                                   "    </arg>\n"
                                   "    <arg nr=\"2\"/>\n"
                                   "    <arg nr=\"3\"/>\n"
                                   "  </function>\n"
                                   "</def>";
        // use a platform which has wchar_t with a sizeof 4
        const Settings settings = settingsBuilder().libraryxml(xmldata).severity(Severity::warning).platform(Platform::Unix32).build();
        ASSERT_EQUALS(4, settings.platform.sizeof_wchar_t);

        check("void f() {\n"
              "    char c[10];\n"
              "    mymemset(c, 0, 10);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char c[10];\n"
              "    mymemset(c, 0, 11);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Buffer is accessed out of bounds: c [bufferAccessOutOfBounds]\n", errout_str());

        check("struct S {\n"
              "    char a[5];\n"
              "};\n"
              "void f() {\n"
              "    S s;\n"
              "    mymemset(s.a, 0, 10);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:6:15]: (error) Buffer is accessed out of bounds: s.a [bufferAccessOutOfBounds]\n", errout_str());

        check("void foo() {\n"
              "    char s[10];\n"
              "    mymemset(s, 0, '*');\n"
              "}", settings);
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning) The size argument is given as a char constant.\n"
                           "[test.cpp:3:14]: (error) Buffer is accessed out of bounds: s [bufferAccessOutOfBounds]\n", "[test.cpp:3:14]: (error) Buffer is accessed out of bounds: s [bufferAccessOutOfBounds]\n", errout_str());

        // ticket #836
        check("void f(void) {\n"
              "  char a[10];\n"
              "  mymemset(a+5, 0, 10);\n"
              "}", settings);
        TODO_ASSERT_EQUALS("[test.cpp:3:13]: (error) Buffer is accessed out of bounds: a [bufferAccessOutOfBounds]\n", "", errout_str());

        // Ticket #909
        check("void f(void) {\n"
              "    char str[] = \"abcd\";\n"
              "    mymemset(str, 0, 6);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Buffer is accessed out of bounds: str [bufferAccessOutOfBounds]\n", errout_str());

        check("void f(void) {\n"
              "    char str[] = \"abcd\";\n"
              "    mymemset(str, 0, 5);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f(void) {\n"
              "    wchar_t str[] = L\"abcd\";\n"
              "    mymemset(str, 0, 21);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Buffer is accessed out of bounds: str [bufferAccessOutOfBounds]\n", errout_str());

        check("void f(void) {\n"
              "    wchar_t str[] = L\"abcd\";\n"
              "    mymemset(str, 0, 20);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        // ticket #1659 - overflowing variable when using memcpy
        check("void f(void) {\n"
              "  char c;\n"
              "  mymemset(&c, 0, 4);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Buffer is accessed out of bounds: &c [bufferAccessOutOfBounds]\n", errout_str());

        // ticket #2121 - buffer access out of bounds when using uint32_t
        check("void f(void) {\n"
              "    unknown_type_t buf[4];\n"
              "    mymemset(buf, 0, 100);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        // #3124 - multidimensional array
        check("int main() {\n"
              "    char b[5][6];\n"
              "    mymemset(b, 0, 5 * 6);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("int main() {\n"
              "    char b[5][6];\n"
              "    mymemset(b, 0, 6 * 6);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Buffer is accessed out of bounds: b [bufferAccessOutOfBounds]\n", errout_str());

        check("int main() {\n"
              "    char b[5][6];\n"
              "    mymemset(b, 0, 31);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:14]: (error) Buffer is accessed out of bounds: b [bufferAccessOutOfBounds]\n", errout_str());

        // #4968 - not standard function
        check("void f() {\n"
              "    char str[3];\n"
              "    foo.mymemset(str, 0, 100);\n"
              "    foo::mymemset(str, 0, 100);\n"
              "    std::mymemset(str, 0, 100);\n"
              "}", settings);
        TODO_ASSERT_EQUALS("[test.cpp:5:15]: (error) Buffer is accessed out of bounds: str [bufferAccessOutOfBounds]\n", "", errout_str());

        // #5257 - check strings
        check("void f() {\n"
              "  mymemset(\"abc\", 0, 20);\n"
              "}", settings);
        TODO_ASSERT_EQUALS("[test.cpp:2]: (error) Buffer is accessed out of bounds.\n",
                           "",
                           errout_str());

        check("void f() {\n"
              "  mymemset(temp, \"abc\", 4);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #6816 - fp when array has known string value
              "    char c[10] = \"c\";\n"
              "    mymemset(c, 0, 10);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());
    }

    void minsize_sizeof() {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"mystrncpy\">\n"
                                   "    <noreturn>false</noreturn>\n"
                                   "    <arg nr=\"1\">\n"
                                   "      <minsize type=\"strlen\" arg=\"2\"/>\n"
                                   "      <minsize type=\"argvalue\" arg=\"3\"/>\n"
                                   "    </arg>\n"
                                   "    <arg nr=\"2\"/>\n"
                                   "    <arg nr=\"3\"/>\n"
                                   "  </function>\n"
                                   "</def>";
        const Settings settings = settingsBuilder().libraryxml(xmldata).build();

        check("void f() {\n"
              "    char c[7];\n"
              "    mystrncpy(c, \"hello\", 7);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              " char c[6];\n"
              " mystrncpy(c,\"hello\",6);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              " char c[5];\n"
              " mystrncpy(c,\"hello\",6);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Buffer is accessed out of bounds: c [bufferAccessOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    char c[6];\n"
              "    mystrncpy(c,\"hello!\",7);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Buffer is accessed out of bounds: c [bufferAccessOutOfBounds]\n", errout_str());

        check("void f(unsigned int addr) {\n"
              "    memset((void *)addr, 0, 1000);\n"
              "}", settings0); // TODO: use settings?
        ASSERT_EQUALS("", errout_str());

        check("struct AB { char a[10]; };\n"
              "void foo(AB *ab) {\n"
              "    mystrncpy(x, ab->a, 100);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void a(char *p) { mystrncpy(p,\"hello world!\",10); }\n" // #3168
              "void b() {\n"
              "    char buf[5];\n"
              "    a(buf);"
              "}", settings);
        TODO_ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:1]: (error) Buffer is accessed out of bounds: buf\n",
                           "",
                           errout_str());
    }

    void minsize_strlen() {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"mysprintf\">\n"
                                   "    <noreturn>false</noreturn>\n"
                                   "    <formatstr/>\n"
                                   "    <arg nr=\"1\">\n"
                                   "      <minsize type=\"strlen\" arg=\"2\"/>\n"
                                   "    </arg>\n"
                                   "    <arg nr=\"2\">\n"
                                   "      <formatstr/>\n"
                                   "    </arg>\n"
                                   "  </function>\n"
                                   "</def>";
        const Settings settings = settingsBuilder().libraryxml(xmldata).build();

        // formatstr..
        check("void f() {\n"
              "    char str[3];\n"
              "    mysprintf(str, \"test\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Buffer is accessed out of bounds: str [bufferAccessOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    char str[5];\n"
              "    mysprintf(str, \"%s\", \"abcde\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Buffer is accessed out of bounds: str [bufferAccessOutOfBounds]\n", errout_str());

        check("int getnumber();\n"
              "void f()\n"
              "{\n"
              "    char str[5];\n"
              "    mysprintf(str, \"%d: %s\", getnumber(), \"abcde\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:5:15]: (error) Buffer is accessed out of bounds: str [bufferAccessOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    char str[5];\n"
              "    mysprintf(str, \"test%s\", \"\");\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char *str = new char[5];\n"
              "    mysprintf(str, \"abcde\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Buffer is accessed out of bounds: str [bufferAccessOutOfBounds]\n", errout_str());

        check("void f(int condition) {\n"
              "    char str[5];\n"
              "    mysprintf(str, \"test%s\", condition ? \"12\" : \"34\");\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f(int condition) {\n"
              "    char str[5];\n"
              "    mysprintf(str, \"test%s\", condition ? \"12\" : \"345\");\n"
              "}", settings);
        TODO_ASSERT_EQUALS("error", "", errout_str());

        check("struct Foo { char a[1]; };\n"
              "void f() {\n"
              "  struct Foo x;\n"
              "  mysprintf(x.a, \"aa\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:4:14]: (error) Buffer is accessed out of bounds: x.a [bufferAccessOutOfBounds]\n", errout_str());

        // ticket #900
        check("void f() {\n"
              "  char *a = new char(30);\n"
              "  mysprintf(a, \"a\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Buffer is accessed out of bounds: a [bufferAccessOutOfBounds]\n", errout_str());

        check("void f(char value) {\n"
              "  char *a = new char(value);\n"
              "  mysprintf(a, \"a\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Buffer is accessed out of bounds: a [bufferAccessOutOfBounds]\n", errout_str());

        // This is out of bounds if 'sizeof(ABC)' is 1 (No padding)
        check("struct Foo { char a[1]; };\n"
              "void f() {\n"
              "  struct Foo *x = malloc(sizeof(Foo));\n"
              "  mysprintf(x->a, \"aa\");\n"
              "}", settings);
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error, inconclusive) Buffer is accessed out of bounds: x.a\n", "", errout_str());

        check("struct Foo { char a[1]; };\n"
              "void f() {\n"
              "  struct Foo *x = malloc(sizeof(Foo) + 10);\n"
              "  mysprintf(x->a, \"aa\");\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("struct Foo { char a[1]; };\n"
              "void f() {\n"
              "  struct Foo x;\n"
              "  mysprintf(x.a, \"aa\");\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:4:14]: (error) Buffer is accessed out of bounds: x.a [bufferAccessOutOfBounds]\n", errout_str());

        check("struct Foo {\n" // #6668 - unknown size
              "  char a[LEN];\n"
              "  void f();\n"
              "};"
              "void Foo::f() {\n"
              "  mysprintf(a, \"abcd\");\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());
    }

    void minsize_mul() {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"myfread\">\n"
                                   "    <arg nr=\"1\">\n"
                                   "      <minsize type=\"mul\" arg=\"2\" arg2=\"3\"/>\n"
                                   "    </arg>\n"
                                   "    <arg nr=\"2\"/>\n"
                                   "    <arg nr=\"3\"/>\n"
                                   "    <arg nr=\"4\"/>\n"
                                   "  </function>\n"
                                   "</def>";
        const Settings settings = settingsBuilder().libraryxml(xmldata).build();

        check("void f() {\n"
              "    char c[5];\n"
              "    myfread(c, 1, 5, stdin);\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char c[5];\n"
              "    myfread(c, 1, 6, stdin);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Buffer is accessed out of bounds: c [bufferAccessOutOfBounds]\n", errout_str());
    }

    // extracttests.enable

    void unknownType() {
        check("void f()\n"
              "{\n"
              " UnknownType *a = malloc(4);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }
    // extracttests.disable
    void terminateStrncpy1() {
        check("void foo ( char *bar ) {\n"
              "    char baz[100];\n"
              "    strncpy(baz, bar, 100);\n"
              "    strncpy(baz, bar, 100);\n"
              "    baz[99] = 0;\n"
              "    strncpy(baz, bar, 100);\n"
              "    baz[99] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo ( char *bar ) {\n"
              "    char baz[100];\n"
              "    strncpy(baz, bar, 100);\n"
              "    baz[99] = '\\0';\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo ( char *bar ) {\n"
              "    char baz[100];\n"
              "    strncpy(baz, bar, 100);\n"
              "    baz[x+1] = '\\0';\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Test with invalid code that there is no segfault
        check("char baz[100];\n"
              "strncpy(baz, \"var\", 100)");
        ASSERT_EQUALS("", errout_str());

        // Test that there are no duplicate error messages
        check("void foo ( char *bar ) {\n"
              "    char baz[100];\n"
              "    strncpy(baz, bar, 100);\n"
              "    foo(baz);\n"
              "    foo(baz);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning, inconclusive) The buffer 'baz' may not be null-terminated after the call to strncpy(). [terminateStrncpy]\n", errout_str());
    }

    void terminateStrncpy2() {
        check("char *foo ( char *bar ) {\n"
              "    char baz[100];\n"
              "    strncpy(baz, bar, 100);\n"
              "    bar[99] = 0;\n"
              "    return strdup(baz);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning, inconclusive) The buffer 'baz' may not be null-terminated after the call to strncpy(). [terminateStrncpy]\n", errout_str());
    }

    void terminateStrncpy3() {
        // Ticket #2170 - false positive
        // The function bar is risky. But it might work that way intentionally.
        check("char str[100];\n"
              "\n"
              "void foo(char *a) {\n"
              "    strncpy(str, a, 100);\n"
              "}\n"
              "\n"
              "void bar(char *p) {\n"
              "    strncpy(p, str, 100);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:5]: (warning, inconclusive) The buffer 'str' may not be null-terminated after the call to strncpy(). [terminateStrncpy]\n", errout_str());
    }

    void terminateStrncpy4() {
        check("void bar() {\n"
              "    char buf[4];\n"
              "    strncpy(buf, \"ab\", 4);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void bar() {\n"
              "    char buf[4];\n"
              "    strncpy(buf, \"abcde\", 4);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning, inconclusive) The buffer 'buf' may not be null-terminated after the call to strncpy(). [terminateStrncpy]\n", errout_str());
    }

    void terminateStrncpy5() { // #9944
        check("void f(const std::string& buf) {\n"
              "    char v[255];\n"
              "    if (buf.size() >= sizeof(v))\n"
              "        return;\n"
              "    strncpy(v, buf.c_str(), sizeof(v));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::string& buf) {\n"
              "    char v[255];\n"
              "    if (buf.size() >= sizeof(v))\n"
              "        strncpy(v, buf.c_str(), sizeof(v));\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:9]: (warning, inconclusive) The buffer 'v' may not be null-terminated after the call to strncpy(). [terminateStrncpy]\n", errout_str());
    }
    // extracttests.enable

    void terminateStrncpy_handle_addr_of_var() { // #7570
        check("void foo() {\n"
              "  char c[6];\n"
              "  strncpy(&c, \"hello!\", 6);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3]: (warning, inconclusive) The buffer 'c' may not be null-terminated after the call to strncpy(). [terminateStrncpy]\n", errout_str());

        check("void foo() {\n"
              "  char c[6];\n"
              "  strncpy(&c, \"hello\\0\", 6);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void recursive_long_time() {
        // Just test that recursive check doesn't take long time
        check("char *f2 ( char *b )\n"
              "{\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "}\n"
              "void f()\n"
              "{\n"
              "    char a[10];\n"
              "    f2(a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    // Ticket #1587 - crash
    void crash1() {
        check("struct struct A\n"
              "{\n"
              "    int alloclen;\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    struct A *str;\n"
              "    str = malloc(4);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void crash2() {
        check("void a(char *p) {\n"
              "    f( { if(finally_arg); } );\n"
              "}\n"
              "\n"
              "void b() {\n"
              "    char arr[64];\n"
              "    a(arr);\n"
              "}");
    }

    void crash3() {
        check("struct b { unknown v[0]; };\n"
              "void d() { struct b *f; f = malloc(108); }");
    }

    void crash4() { // #8679
        check("__thread void *thread_local_var; "
              "int main() { "
              "  thread_local_var = malloc(1337); "
              "  return 0; "
              "}");

        check("thread_local void *thread_local_var; "
              "int main() { "
              "  thread_local_var = malloc(1337); "
              "  return 0; "
              "}");
    }

    void crash5() { // 8644 - token has varId() but variable() is null
        check("int a() {\n"
              "    void b(char **dst) {\n"
              "        *dst = malloc(50);\n"
              "    }\n"
              "}");
    }

    void crash6() {
        check("void start(char* name) {\n"
              "char snapname[64] = { 0 };\n"
              "strncpy(snapname, \"snapshot\", arrayLength(snapname));\n"
              "}");
    }

    void crash7() { // 9073 - [ has no astParent
        check("char x[10];\n"
              "void f() { x[10]; }");
    }

    void insecureCmdLineArgs() {
        check("int main(int argc, char *argv[])\n"
              "{\n"
              "    if(argc>1)\n"
              "    {\n"
              "        char buf[2];\n"
              "        char *p = strdup(argv[1]);\n"
              "        strcpy(buf,p);\n"
              "        free(p);\n"
              "    }\n"
              "    return 0;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, char *argv[])\n"
              "{\n"
              "    if(argc>1)\n"
              "    {\n"
              "        char buf[2] = {'\\0','\\0'};\n"
              "        char *p = strdup(argv[1]);\n"
              "        strcat(buf,p);\n"
              "        free(p);\n"
              "    }\n"
              "    return 0;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(const int argc, char* argv[])\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, const char* argv[])\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(const int argc, const char* argv[])\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, char* argv[])\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    strcat(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, const char *const *const argv, char **envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(const int argc, const char *const *const argv, const char *const *const envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    strcat(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(const int argc, const char **argv, char **envp)\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    strcat(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, const char **argv, char **envp)\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    strcat(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(const int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    strcat(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, char **options)\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, options[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, char **options)\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    strcat(prog, options[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, char **options)\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, *options);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, char **options)\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog+3, *options);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    if (strlen(argv[0]) < 10)\n"
              "        strcpy(prog, argv[0]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    if (10 > strlen(argv[0]))\n"
              "        strcat(prog, argv[0]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    argv[0][0] = '\\0';\n"
              "    strcpy(prog, argv[0]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5835
        check("int main(int argc, char* argv[]) {\n"
              "    char prog[10];\n"
              "    strcpy(prog, argv[0]);\n"
              "    strcpy(prog, argv[0]);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Buffer overrun possible for long command line arguments.\n"
                           "[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", "", errout_str());

        // #7964
        check("int main(int argc, char *argv[]) {\n"
              "  char *strcpy();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("int main(int argc, char *argv[]) {\n"
              "  char *strcat();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void checkBufferAllocatedWithStrlen() {
        check("void f(char *a) {\n"
              "  char *b = new char[strlen(a)];\n"
              "  strcpy(b, a);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", "", errout_str());

        check("void f(char *a) {\n"
              "  char *b = new char[strlen(a) + 1];\n"
              "  strcpy(b, a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char *a) {\n"
              "  char *b = new char[strlen(a)];\n"
              "  a[0] = '\\0';\n"
              "  strcpy(b, a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char *a) {\n"
              "  char *b = (char *)malloc(strlen(a));\n"
              "  b = realloc(b, 10000);\n"
              "  strcpy(b, a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char *a) {\n"
              "  char *b = (char *)malloc(strlen(a));\n"
              "  strcpy(b, a);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", "", errout_str());

        check("void f(char *a) {\n"
              "  char *b = (char *)malloc(strlen(a));\n"
              "  {\n"
              "    strcpy(b, a);\n"
              "  }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds.\n", "", errout_str());

        check("void f(char *a) {\n"
              "  char *b = (char *)malloc(strlen(a) + 1);\n"
              "  strcpy(b, a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char *a, char *c) {\n"
              "  char *b = (char *)realloc(c, strlen(a));\n"
              "  strcpy(b, a);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", "", errout_str());

        check("void f(char *a, char *c) {\n"
              "  char *b = (char *)realloc(c, strlen(a) + 1);\n"
              "  strcpy(b, a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char *a) {\n"
              "  char *b = (char *)malloc(strlen(a));\n"
              "  strcpy(b, a);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", "", errout_str());
    }

    void scope() {
        check("class A {\n"
              "private:\n"
              "    struct X { char buf[10]; };\n"
              "};\n"
              "\n"
              "void f()\n"
              "{\n"
              "    X x;\n"
              "    x.buf[10] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class A {\n"
              "public:\n"
              "    struct X { char buf[10]; };\n"
              "};\n"
              "\n"
              "void f()\n"
              "{\n"
              "    A::X x;\n"
              "    x.buf[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:10]: (error) Array 'x.buf[10]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]\n", errout_str());
    }

    void getErrorMessages() {
        // Ticket #2292: segmentation fault when using --errorlist
        const Check& c = getCheck<CheckBufferOverrun>();
        c.getErrorMessages(this, nullptr);
        // we are not interested in the output - just consume it
        ignore_errout();
    }

    void arrayIndexThenCheck() {
        // extracttests.start: volatile int y;

        check("void f(const char s[]) {\n"
              "    if (s[i] == 'x' && i < y) {\n"
              "    }"
              "}");
        ASSERT_EQUALS("", errout_str()); // No message because i is unknown and thus gets no varid. Avoid an internalError here.

        check("void f(const char s[], int i) {\n"
              "    if (s[i] == 'x' && i < y) {\n"
              "    }"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Array index 'i' is used before limits check. [arrayIndexThenCheck]\n", errout_str());

        check("void f(const char s[]) {\n"
              "    for (int i = 0; s[i] == 'x' && i < y; ++i) {\n"
              "    }"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (style) Array index 'i' is used before limits check. [arrayIndexThenCheck]\n", errout_str());

        check("void f(const int a[], unsigned i) {\n"
              "    if((a[i] < 2) && (i <= 42)) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Array index 'i' is used before limits check. [arrayIndexThenCheck]\n", errout_str());

        check("void f(const int a[], unsigned i) {\n"
              "    if((a[i] < 2) && (42 >= i)) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (style) Array index 'i' is used before limits check. [arrayIndexThenCheck]\n", errout_str());

        // extracttests.start: int elen;
        check("void f(char* e, int y) {\n"
              "    if (e[y] == '/' && elen > y + 1 && e[y + 1] == '?') {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: int foo(int); int func(int);
        check("void f(const int a[], unsigned i) {\n"
              "    if(a[i] < func(i) && i <= 42) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (style) Array index 'i' is used before limits check. [arrayIndexThenCheck]\n", errout_str());

        check("void f(const int a[], unsigned i) {\n"
              "    if (i <= 42 && a[i] < func(i)) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const int a[], unsigned i) {\n"
              "    if (foo(a[i] + 3) < func(i) && i <= 42) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (style) Array index 'i' is used before limits check. [arrayIndexThenCheck]\n", errout_str());

        check("void f(int i) {\n" // sizeof
              "  sizeof(a)/sizeof(a[i]) && i < 10;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // extracttests.start: extern int buf[];
        check("void f(int i) {\n" // ?:
              "  if ((i < 10 ? buf[i] : 1) && (i < 5 ? buf[i] : 5)){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void arrayIndexEarlyReturn() { // #6884
        check("extern const char *Names[2];\n"
              "const char* getName(int value) {\n"
              "  if ((value < 0) || (value > 1))\n"
              "    return \"???\";\n"
              "  const char* name = Names[value]; \n"
              "  switch (value) {\n"
              "  case 2:\n"
              "    break; \n"
              "  }\n"
              "  return name;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void bufferNotZeroTerminated() {
        check("void f() {\n"
              "    char c[6];\n"
              "    strncpy(c,\"hello!\",6);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (warning, inconclusive) The buffer 'c' may not be null-terminated after the call to strncpy(). [terminateStrncpy]\n", errout_str());

        check("void f() {\n"
              "    char c[6];\n"
              "    memcpy(c,\"hello!\",6);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'c' may not be null-terminated after the call to memcpy().\n", "", errout_str());

        check("void f() {\n"
              "    char c[6];\n"
              "    memmove(c,\"hello!\",6);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'c' may not be null-terminated after the call to memmove().\n", "", errout_str());
    }

    void negativeMemoryAllocationSizeError() { // #389
        check("void f()\n"
              "{\n"
              "   int *a;\n"
              "   a = new int[-1];\n"
              "   delete [] a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:8]: (error) Memory allocation size is negative. [negativeMemoryAllocationSize]\n", errout_str());

        check("void f()\n"
              "{\n"
              "   int *a;\n"
              "   a = (int *)malloc( -10 );\n"
              "   free(a);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4:8]: (error) Memory allocation size is negative. [negativeMemoryAllocationSize]\n", "", errout_str());

        check("void f()\n"
              "{\n"
              "   int *a;\n"
              "   a = (int *)malloc( -10);\n"
              "   free(a);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4:8]: (error) Memory allocation size is negative. [negativeMemoryAllocationSize]\n", "", errout_str());

        check("void f()\n"
              "{\n"
              "   int *a;\n"
              "   a = (int *)alloca( -10 );\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4:8]: (error) Memory allocation size is negative. [negativeMemoryAllocationSize]\n", "", errout_str());

        check("int* f(int n) {\n" // #11145
              "    int d = -1;\n"
              "    for (int i = 0; i < n; ++i)\n"
              "        d = std::max(i, d);\n"
              "    int* p = new int[d];\n"
              "    return p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:14]: (warning, inconclusive) Memory allocation size is negative. [negativeMemoryAllocationSize]\n", errout_str());
    }

    void negativeArraySize() {
        check("void f(int sz) {\n" // #1760 - VLA
              "   int a[sz];\n"
              "}\n"
              "void x() { f(-100); }");
        ASSERT_EQUALS("[test.cpp:2:8]: (error) Declaration of array 'a' with negative size is undefined behaviour [negativeArraySize]\n", errout_str());

        // don't warn for constant sizes -> this is a compiler error so this is used for static assertions for instance
        check("int x, y;\n"
              "int a[-1];\n"
              "int b[x?1:-1];\n"
              "int c[x?y:-1];");
        ASSERT_EQUALS("", errout_str());
    }

    void pointerAddition1() {
        check("void f() {\n"
              "    char arr[10];\n"
              "    char *p = arr + 20;\n"
              "}", settings0_p);
        ASSERT_EQUALS("[test.cpp:3:19]: (portability) Undefined behaviour, pointer arithmetic 'arr+20' is out of bounds. [pointerOutOfBounds]\n", errout_str());

        check("char(*g())[1];\n" // #7950
              "void f() {\n"
              "    int a[2];\n"
              "    int* b = a + sizeof(*g());\n"
              "}\n", settings0_p);
        ASSERT_EQUALS("", errout_str());
    }

#define ctu(...) ctu_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void ctu_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings0, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        CTU::FileInfo *ctu = CTU::getFileInfo(tokenizer);

        // Check code..
        std::list<Check::FileInfo*> fileInfo;
        Check& c = getCheck<CheckBufferOverrun>();
        fileInfo.push_back(c.getFileInfo(tokenizer, settings0, ""));
        c.analyseWholeProgram(*ctu, fileInfo, settings0, *this); // TODO: check result
        while (!fileInfo.empty()) {
            delete fileInfo.back();
            fileInfo.pop_back();
        }
        delete ctu;
    }

    void ctu_malloc() {
        ctu("void dostuff(char *p) {\n"
            "  p[-3] = 0;\n"
            "}\n"
            "\n"
            "int main() {\n"
            "  char *s = malloc(4);\n"
            "  dostuff(s);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:6:11] -> [test.cpp:7:10] -> [test.cpp:2:3]: (error) Array index out of bounds; buffer 'p' is accessed at offset -3. [ctuArrayIndex]\n", errout_str());

        ctu("void dostuff(char *p) {\n"
            "  p[4] = 0;\n"
            "}\n"
            "\n"
            "int main() {\n"
            "  char *s = malloc(4);\n"
            "  dostuff(s);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:6:11] -> [test.cpp:7:10] -> [test.cpp:2:3]: (error) Array index out of bounds; 'p' buffer size is 4 and it is accessed at offset 4. [ctuArrayIndex]\n", errout_str());

        ctu("void f(int* p) {\n" // #10415
            "    int b[1];\n"
            "    b[0] = p[5];\n"
            "    std::cout << b[0];\n"
            "}\n"
            "void g() {\n"
            "    int* a = new int[1];\n"
            "    a[0] = 5;\n"
            "    f(a);\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:7:12] -> [test.cpp:9:6] -> [test.cpp:3:12]: (error) Array index out of bounds; 'p' buffer size is 4 and it is accessed at offset 20. [ctuArrayIndex]\n", errout_str());

        ctu("void bar(int *p) { p[4] = 42; }\n" // #13403
            "void f() {\n"
            "    int *p = (int*)malloc(4 * sizeof(int));\n"
            "    if (!p) return;\n"
            "    bar(p);\n"
            "    free(p);\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:3:12] -> [test.cpp:4:9] -> [test.cpp:5:8] -> [test.cpp:1:20]: (error) Array index out of bounds; 'p' buffer size is 16 and it is accessed at offset 16. [ctuArrayIndex]\n", errout_str());
    }

    void ctu_array() {
        ctu("void dostuff(char *p) {\n"
            "    p[10] = 0;\n"
            "}\n"
            "int main() {\n"
            "  char str[4];\n"
            "  dostuff(str);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:6:10] -> [test.cpp:2:5]: (error) Array index out of bounds; 'p' buffer size is 4 and it is accessed at offset 10. [ctuArrayIndex]\n", errout_str());

        ctu("static void memclr( char *data )\n"
            "{\n"
            "    data[10] = 0;\n"
            "}\n"
            "\n"
            "static void f()\n"
            "{\n"
            "    char str[5];\n"
            "    memclr( str );\n"
            "}");
        ASSERT_EQUALS("[test.cpp:9:11] -> [test.cpp:3:5]: (error) Array index out of bounds; 'data' buffer size is 5 and it is accessed at offset 10. [ctuArrayIndex]\n", errout_str());

        ctu("static void memclr( int i, char *data )\n"
            "{\n"
            "    data[10] = 0;\n"
            "}\n"
            "\n"
            "static void f()\n"
            "{\n"
            "    char str[5];\n"
            "    memclr( 0, str );\n"
            "}");
        ASSERT_EQUALS("[test.cpp:9:11] -> [test.cpp:3:5]: (error) Array index out of bounds; 'data' buffer size is 5 and it is accessed at offset 10. [ctuArrayIndex]\n", errout_str());

        ctu("static void memclr( int i, char *data )\n"
            "{\n"
            "    data[i] = 0;\n"
            "}\n"
            "\n"
            "static void f()\n"
            "{\n"
            "    char str[5];\n"
            "    memclr( 10, str );\n"
            "}");
        TODO_ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (possible error) Array index out of bounds.\n",
                           "", errout_str());

        // This is not an error
        ctu("static void memclr( char *data, int size )\n"
            "{\n"
            "    if( size > 10 )"
            "      data[10] = 0;\n"
            "}\n"
            "\n"
            "static void f()\n"
            "{\n"
            "    char str[5];\n"
            "    memclr( str, 5 );\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // #2097
        ctu("void foo(int *p)\n"
            "{\n"
            "    --p;\n"
            "    p[2] = 0;\n"
            "}\n"
            "\n"
            "void bar()\n"
            "{\n"
            "    int p[3];\n"
            "    foo(p+1);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // #9112
        ctu("static void get_mac_address(const u8 *strbuf)\n"
            "{\n"
            "    (strbuf[2]);\n"
            "}\n"
            "\n"
            "static void program_mac_address(u32 mem_base)\n"
            "{\n"
            "    u8 macstrbuf[17] = { 0 };\n"
            "    get_mac_address(macstrbuf);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        // #9788
        ctu("void f1(char *s) { s[2] = 'B'; }\n"
            "void f2(char s[]) { s[2] = 'B'; }\n"
            "void g() {\n"
            "    char str[2];\n"
            "    f1(str);\n"
            "    f2(str);\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:5:7] -> [test.cpp:1:20]: (error) Array index out of bounds; 's' buffer size is 2 and it is accessed at offset 2. [ctuArrayIndex]\n"
                      "[test.cpp:6:7] -> [test.cpp:2:21]: (error) Array index out of bounds; 's' buffer size is 2 and it is accessed at offset 2. [ctuArrayIndex]\n",
                      errout_str());

        // #5140
        ctu("void g(const char* argv[]) { std::cout << \"argv: \" << argv[4] << std::endl; }\n"
            "void f() {\n"
            "    const char* argv[] = { \"test\" };\n"
            "    g(argv);\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4:6] -> [test.cpp:1:55]: (error) Array index out of bounds; 'argv' buffer size is 1 and it is accessed at offset 4. [ctuArrayIndex]\n",
                      errout_str());

        ctu("void g(const char* argv[]) { std::cout << \"argv: \" << argv[5] << std::endl; }\n"
            "void f() {\n"
            "    const char* argv[1] = { \"test\" };\n"
            "    g(argv);\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:4:6] -> [test.cpp:1:55]: (error) Array index out of bounds; 'argv' buffer size is 1 and it is accessed at offset 5. [ctuArrayIndex]\n",
                      errout_str());

        ctu("void g(int *b) { b[0] = 0; }\n"
            "void f() {\n"
            "    GLint a[1];\n"
            "    g(a);\n"
            "}\n");
        ASSERT_EQUALS("", errout_str());

        ctu("const int a[1] = { 1 };\n" // #11042
            "void g(const int* d) {\n"
            "    (void)d[2];\n"
            "}\n"
            "void f() {\n"
            "    g(a);\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:6:6] -> [test.cpp:3:11]: (error) Array index out of bounds; 'd' buffer size is 4 and it is accessed at offset 8. [ctuArrayIndex]\n",
                      errout_str());
    }

    void ctu_variable() {
        ctu("void dostuff(int *p) {\n"
            "    p[10] = 0;\n"
            "}\n"
            "int main() {\n"
            "  int x = 4;\n"
            "  dostuff(&x);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:6:10] -> [test.cpp:2:5]: (error) Array index out of bounds; 'p' buffer size is 4 and it is accessed at offset 40. [ctuArrayIndex]\n", errout_str());
    }

    void ctu_arithmetic() {
        ctu("void dostuff(int *p) { x = p + 10; }\n"
            "int main() {\n"
            "  int x[3];\n"
            "  dostuff(x);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:4:10] -> [test.cpp:1:28]: (error) Pointer arithmetic overflow; 'p' buffer size is 12 [ctuPointerArith]\n", errout_str());

        ctu("void f(const char *p) {\n" // #11361
            "    const char* c = p + 1;\n"
            "}\n"
            "void g() {\n"
            "    const char s[N] = \"ab\";\n"
            "    f(s);\n"
            "}\n");
        ASSERT_EQUALS("", errout_str());

        setMultiline();
        ctu("void g(char* p) {\n"
            "    memset(p + 10, 0, 10);\n"
            "}\n"
            "void f() {\n"
            "    char a[10] = {};\n"
            "    g(a);\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2:12]: error: Pointer arithmetic overflow; 'p' buffer size is 10 [ctuPointerArith]\n"
                      "[test.cpp:6:6]: note: Calling function g, 1st argument is accessed out of bounds\n"
                      "[test.cpp:2:12]: note: Using argument p\n",
                      errout_str());
    }

    void objectIndex() {
        check("int f() {\n"
              "    int i;\n"
              "    return (&i)[1];\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:13] -> [test.cpp:3:16]: (error) The address of variable 'i' is accessed at non-zero index. [objectIndex]\n",
            errout_str());

        check("int f(int j) {\n"
              "    int i;\n"
              "    return (&i)[j];\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:3:13] -> [test.cpp:3:16]: (warning) The address of variable 'i' might be accessed at non-zero index. [objectIndex]\n",
            errout_str());

        check("int f() {\n"
              "    int i;\n"
              "    return (&i)[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int * i) {\n"
              "    return i[1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(std::vector<int> i) {\n"
              "    return i[1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(std::vector<int> i) {\n"
              "    return i.data()[1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int* f(std::vector<int>& i) {\n"
              "    return &(i[1]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int i; int j; };\n"
              "int f() {\n"
              "    A x;\n"
              "    return (&x.i)[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct A { int i; int j; };\n"
              "int f() {\n"
              "    A x;\n"
              "    int * i = &x.i;\n"
              "    return i[0];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int x = 0;\n"
              "  std::map<int, int*> m;\n"
              "  m[0] = &x;\n"
              "  m[1] = &x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "  int x = 0;\n"
              "  std::map<int, int*> m;\n"
              "  m[0] = &x;\n"
              "  return m[0][1];\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:10] -> [test.cpp:5:14]: (error) The address of variable 'x' is accessed at non-zero index. [objectIndex]\n",
            errout_str());

        check("int x = 0;\n"
              "int f() {\n"
              "  std::map<int, int*> m;\n"
              "  m[0] = &x;\n"
              "  return m[0][1];\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:10] -> [test.cpp:5:14]: (error) The address of variable 'x' is accessed at non-zero index. [objectIndex]\n",
            errout_str());

        check("int f(int * y) {\n"
              "  int x = 0;\n"
              "  std::map<int, int*> m;\n"
              "  m[0] = &x;\n"
              "  m[1] = y;\n"
              "  return m[1][1];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void print(char** test);\n"
              "int main(){\n"
              "    char* test = \"abcdef\";\n"
              "    print(&test);\n"
              "    return 0;\n"
              "}\n"
              "void print(char** test){\n"
              "    for(int i=0;i<strlen(*test);i++)\n"
              "        printf(\"%c\",*test[i]);\n"
              "}\n");
        TODO_ASSERT_EQUALS(
            "[test.cpp:4] -> [test.cpp:4] -> [test.cpp:9]: (warning) The address of local variable 'test' might be accessed at non-zero index.\n",
            "",
            errout_str());

        check("void Bar(uint8_t data);\n"
              "void Foo(const uint8_t * const data, const uint8_t length) {\n"
              "        for(uint8_t index = 0U; index < length ; ++index)\n"
              "            Bar(data[index]);\n"
              "}\n"
              "void test() {\n"
              "    const uint8_t data = 0U;\n"
              "    Foo(&data,1U);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int foo(int n, int* p) {\n"
              "    int res = 0;\n"
              "    for(int i = 0; i < n; i++ )\n"
              "        res += p[i];\n"
              "    return res;\n"
              "}\n"
              "int bar() {\n"
              "    int single_value = 0;\n"
              "    return foo(1, &single_value);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const char* app, size_t applen) {\n" // #10137
              "    char* tmp_de = NULL;\n"
              "    char** str = &tmp_de;\n"
              "    char* tmp = (char*)realloc(*str, applen + 1);\n"
              "    if (tmp) {\n"
              "        *str = tmp;\n"
              "        memcpy(*str, app, applen);\n"
              "        (*str)[applen] = '\\0';\n"
              "    }\n"
              "    free(*str);\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("template <typename T, unsigned N>\n"
              "using vector = Eigen::Matrix<T, N, 1>;\n"
              "template <typename V>\n"
              "void scharr(image2d<vector<V, 2>>& out) {\n"
              "    vector<V, 2>* out_row = &out(r, 0);\n"
              "    out_row[c] = vector<V, 2>(1,2);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const uint8_t* d, const uint8_t L) {\n" // #10092
              "    for (uint8_t i = 0U; i < L; ++i)\n"
              "        g(d[i]);\n"
              "}\n"
              "void h() {\n"
              "    const uint8_t u = 4;\n"
              "    f(&u, N);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("uint32_t f(uint32_t u) {\n" // #10154
              "    return ((uint8_t*)&u)[3];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("uint32_t f(uint32_t u) {\n"
              "    return ((uint8_t*)&u)[4];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:23] -> [test.cpp:2:26]: (error) The address of variable 'u' is accessed at non-zero index. [objectIndex]\n", errout_str());

        check("uint32_t f(uint32_t u) {\n"
              "    return reinterpret_cast<unsigned char*>(&u)[3];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("uint32_t f(uint32_t u) {\n"
              "    return reinterpret_cast<unsigned char*>(&u)[4];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:45] -> [test.cpp:2:48]: (error) The address of variable 'u' is accessed at non-zero index. [objectIndex]\n", errout_str());

        check("uint32_t f(uint32_t u) {\n"
              "    uint8_t* p = (uint8_t*)&u;\n"
              "    return p[3];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("uint32_t f(uint32_t u) {\n"
              "    uint8_t* p = (uint8_t*)&u;\n"
              "    return p[4];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:28] -> [test.cpp:3:13]: (error) The address of variable 'u' is accessed at non-zero index. [objectIndex]\n", errout_str());

        check("uint32_t f(uint32_t* pu) {\n"
              "    uint8_t* p = (uint8_t*)pu;\n"
              "    return p[4];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { uint8_t padding[500]; };\n" // #10133
              "S s = { 0 };\n"
              "uint8_t f() {\n"
              "    uint8_t* p = (uint8_t*)&s;\n"
              "    return p[10];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct X {\n" // #2654
              "    int  a;\n"
              "    char b;\n"
              "};\n"
              "void f() {\n"
              "    const X s;\n"
              "    const int* y = &s.a;\n"
              "    (void)y[0];\n"
              "    (void)y[1];\n"
              "    (void)y[2];\n"
              "}\n");
        TODO_ASSERT_EQUALS("error", "", errout_str());
    }

    void checkPipeParameterSize() { // #3521

        const Settings settings = settingsBuilder().library("posix.cfg").build();

        check("void f(){\n"
              "int pipefd[1];\n" // <--  array of two integers is needed
              "if (pipe(pipefd) == -1) {\n"
              "    return;\n"
              "  }\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Buffer is accessed out of bounds: pipefd [bufferAccessOutOfBounds]\n", errout_str());

        check("void f(){\n"
              "int pipefd[2];\n"
              "if (pipe(pipefd) == -1) {\n"
              "    return;\n"
              "  }\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());

        check("void f(){\n"
              "char pipefd[2];\n"
              "if (pipe((int*)pipefd) == -1) {\n"
              "    return;\n"
              "  }\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Buffer is accessed out of bounds: (int*)pipefd [bufferAccessOutOfBounds]\n", errout_str());

        check("void f(){\n"
              "char pipefd[20];\n" // Strange, but large enough
              "if (pipe((int*)pipefd) == -1) {\n"
              "    return;\n"
              "  }\n"
              "}", settings);
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestBufferOverrun)
