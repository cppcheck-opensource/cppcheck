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

#include "checkleakautovar.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include <cstddef>
#include <string>

class TestLeakAutoVar : public TestFixture {
public:
    TestLeakAutoVar() : TestFixture("TestLeakAutoVar") {}

private:
    const Settings settings = settingsBuilder().library("std.cfg").checkLibrary().build();

    void run() override {
        mNewTemplate = true;
        // Assign
        TEST_CASE(assign1);
        TEST_CASE(assign2);
        TEST_CASE(assign3);
        TEST_CASE(assign4);
        TEST_CASE(assign5);
        TEST_CASE(assign6);
        TEST_CASE(assign7);
        TEST_CASE(assign8);
        TEST_CASE(assign9);
        TEST_CASE(assign10);
        TEST_CASE(assign11); // #3942: x = a(b(p));
        TEST_CASE(assign12); // #4236: FP. bar(&x);
        TEST_CASE(assign13); // #4237: FP. char*&ref=p; p=malloc(10); free(ref);
        TEST_CASE(assign14);
        TEST_CASE(assign15);
        TEST_CASE(assign16);
        TEST_CASE(assign17); // #9047
        TEST_CASE(assign18);
        TEST_CASE(assign19);
        TEST_CASE(assign20); // #9187
        TEST_CASE(assign21); // #10186
        TEST_CASE(assign22); // #9139
        TEST_CASE(assign23);
        TEST_CASE(assign24); // #7440
        TEST_CASE(assign25);
        TEST_CASE(assign26);

        TEST_CASE(memcpy1); // #11542
        TEST_CASE(memcpy2);

        TEST_CASE(isAutoDealloc);

        TEST_CASE(realloc1);
        TEST_CASE(realloc2);
        TEST_CASE(realloc3);
        TEST_CASE(realloc4);
        TEST_CASE(realloc5); // #9292, #9990
        TEST_CASE(freopen1);
        TEST_CASE(freopen2);

        TEST_CASE(deallocuse1);
        TEST_CASE(deallocuse3);
        TEST_CASE(deallocuse4);
        TEST_CASE(deallocuse5); // #4018: FP. free(p), p = 0;
        TEST_CASE(deallocuse6); // #4034: FP. x = p = f();
        TEST_CASE(deallocuse7); // #6467, #6469, #6473
        TEST_CASE(deallocuse8); // #1765
        TEST_CASE(deallocuse9); // #9781
        TEST_CASE(deallocuse10);
        TEST_CASE(deallocuse11); // #8302
        TEST_CASE(deallocuse12);
        TEST_CASE(deallocuse13);
        TEST_CASE(deallocuse14);
        TEST_CASE(deallocuse15);
        TEST_CASE(deallocuse16); // #8109: delete with comma operator

        TEST_CASE(doublefree1);
        TEST_CASE(doublefree2);
        TEST_CASE(doublefree3); // #4914
        TEST_CASE(doublefree4); // #5451 - FP when exit is called
        TEST_CASE(doublefree5); // #5522
        TEST_CASE(doublefree6); // #7685
        TEST_CASE(doublefree7);
        TEST_CASE(doublefree8);
        TEST_CASE(doublefree9);
        TEST_CASE(doublefree10); // #8706
        TEST_CASE(doublefree11);
        TEST_CASE(doublefree12); // #10502
        TEST_CASE(doublefree13); // #11008
        TEST_CASE(doublefree14); // #9708
        TEST_CASE(doublefree15);
        TEST_CASE(doublefree16);
        TEST_CASE(doublefree17); // #8109: delete with comma operator
        TEST_CASE(doublefree18);
        TEST_CASE(doublefree19); // #13960

        // exit
        TEST_CASE(exit1);
        TEST_CASE(exit2);
        TEST_CASE(exit3);
        TEST_CASE(exit4);

        // handling function calls
        TEST_CASE(functioncall1);

        // goto
        TEST_CASE(goto1);
        TEST_CASE(goto2);
        TEST_CASE(goto3); // #11431

        // if/else
        TEST_CASE(ifelse1);
        TEST_CASE(ifelse2);
        TEST_CASE(ifelse3);
        TEST_CASE(ifelse4);
        TEST_CASE(ifelse5);
        TEST_CASE(ifelse6); // #3370
        TEST_CASE(ifelse7); // #5576 - if (fd < 0)
        TEST_CASE(ifelse8); // #5747 - if (fd == -1)
        TEST_CASE(ifelse9); // #5273 - if (X(p==NULL, 0))
        TEST_CASE(ifelse10); // #8794 - if (!(x!=NULL))
        TEST_CASE(ifelse11); // #8365 - if (NULL == (p = malloc(4)))
        TEST_CASE(ifelse12); // #8340 - if ((*p = malloc(4)) == NULL)
        TEST_CASE(ifelse13); // #8392
        TEST_CASE(ifelse14); // #9130 - if (x == (char*)NULL)
        TEST_CASE(ifelse15); // #9206 - if (global_ptr = malloc(1))
        TEST_CASE(ifelse16); // #9635 - if (p = malloc(4), p == NULL)
        TEST_CASE(ifelse17); //  if (!!(!p))
        TEST_CASE(ifelse18);
        TEST_CASE(ifelse19);
        TEST_CASE(ifelse20); // #10182
        TEST_CASE(ifelse21);
        TEST_CASE(ifelse22); // #10187
        TEST_CASE(ifelse23); // #5473
        TEST_CASE(ifelse24); // #1733
        TEST_CASE(ifelse25); // #9966
        TEST_CASE(ifelse26);
        TEST_CASE(ifelse27);
        TEST_CASE(ifelse28); // #11038
        TEST_CASE(ifelse29);

        // switch
        TEST_CASE(switch1);

        // loops
        TEST_CASE(loop1);
        TEST_CASE(loop2);

        // mismatching allocation/deallocation
        TEST_CASE(mismatchAllocDealloc);

        TEST_CASE(smartPointerDeleter);
        TEST_CASE(smartPointerRelease);

        // Execution reaches a 'return'
        TEST_CASE(return1);
        TEST_CASE(return2);
        TEST_CASE(return3);
        TEST_CASE(return4);
        TEST_CASE(return5);
        TEST_CASE(return6); // #8282 return {p, p}
        TEST_CASE(return7); // #9343 return (uint8_t*)x
        TEST_CASE(return8);
        TEST_CASE(return9);
        TEST_CASE(return10);
        TEST_CASE(return11); // #13098
        TEST_CASE(return12); // #12238
        TEST_CASE(return13);

        // General tests: variable type, allocation type, etc
        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);  // #3954 - reference pointer
        TEST_CASE(test4);  // #5923 - static pointer
        TEST_CASE(test5);  // unknown type

        // Execution reaches a 'throw'
        TEST_CASE(throw1);
        TEST_CASE(throw2);

        // Possible leak => Further configuration is needed for complete analysis
        TEST_CASE(configuration1);
        TEST_CASE(configuration2);
        TEST_CASE(configuration3);
        TEST_CASE(configuration4);
        TEST_CASE(configuration5);
        TEST_CASE(configuration6);

        TEST_CASE(ptrptr);

        TEST_CASE(nestedAllocation);
        TEST_CASE(testKeywords); // #6767

        TEST_CASE(inlineFunction); // #3989

        TEST_CASE(smartPtrInContainer); // #8262

        TEST_CASE(functionCallCastConfig); // #9652
        TEST_CASE(functionCallLeakIgnoreConfig); // #7923
    }

    struct CheckOptions
    {
        bool cpp = false;
        const Settings *s = nullptr;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings& settings1 = options.s ? *options.s : settings;

        // Tokenize..
        SimpleTokenizer tokenizer(settings1, *this, options.cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for leaks..
        runChecks<CheckLeakAutoVar>(tokenizer, this);
    }

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const Settings & s) {
        // Tokenize..
        SimpleTokenizer tokenizer(s, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for leaks..
        runChecks<CheckLeakAutoVar>(tokenizer, this);
    }

    void assign1() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p = NULL;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.c:3:5]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void assign2() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    char *q = p;\n"
              "    free(q);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign3() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    char *q = p + 1;\n"
              "    free(q - 1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign4() {
        check("void f() {\n"
              "    char *a = malloc(10);\n"
              "    a += 10;\n"
              "    free(a - 10);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign5() {
        check("void foo()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    list += p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign6() { // #2806 - FP when there is redundant assignment
        check("void foo() {\n"
              "    char *p = malloc(10);\n"
              "    p = strcpy(p,q);\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign7() {
        check("void foo(struct str *d) {\n"
              "    struct str *p = malloc(10);\n"
              "    d->p = p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign8() {  // linux list
        check("void foo(struct str *d) {\n"
              "    struct str *p = malloc(10);\n"
              "    d->p = &p->x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign9() {
        check("void foo() {\n"
              "    char *p = x();\n"
              "    free(p);\n"
              "    p = NULL;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign10() {
        check("void foo() {\n"
              "    char *p;\n"
              "    if (x) { p = malloc(10); }\n"
              "    if (!x) { p = NULL; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign11() { // #3942 - FP for x = a(b(p));
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    x = a(b(p));\n"
              "}");
        ASSERT_EQUALS("[test.c:4:1]: (information) --check-library: Function b() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n", errout_str());
    }

    void assign12() { // #4236: FP. bar(&x)
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    free(p);\n"
              "    bar(&p);\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign13() { // #4237: FP. char *&ref=p; p=malloc(10); free(ref);
        check("void f() {\n"
              "    char *p;\n"
              "    char * &ref = p;\n"
              "    p = malloc(10);\n"
              "    free(ref);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        TODO_ASSERT_EQUALS("", "[test.cpp:6:1]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void assign14() {
        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x && (p = malloc(10))) { }"
              "}");
        ASSERT_EQUALS("[test.c:3:35]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x && (p = new char[10])) { }"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:37]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void assign15() {
        // #8120
        check("void f() {\n"
              "   baz *p;\n"
              "   p = malloc(sizeof *p);\n"
              "   free(p);\n"
              "   p = malloc(sizeof *p);\n"
              "   free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign16() {
        check("void f() {\n"
              "   char *p = malloc(10);\n"
              "   free(p);\n"
              "   if (p=dostuff()) *p = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign17() { // #9047
        check("void f() {\n"
              "    char *p = (char*)malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.c:3:1]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void f() {\n"
              "    char *p = (char*)(int*)malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.c:3:1]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void assign18() {
        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x && (p = (char*)malloc(10))) { }"
              "}");
        ASSERT_EQUALS("[test.c:3:42]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x && (p = (char*)(int*)malloc(10))) { }"
              "}");
        ASSERT_EQUALS("[test.c:3:48]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void assign19() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    free((void*)p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign20() { // #9187
        check("void f() {\n"
              "    char *p = static_cast<int>(malloc(10));\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:1]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void assign21() {
        check("void f(int **x) {\n" // #10186
              "    void *p = malloc(10);\n"
              "    *x = (int*)p;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n" // #10996
              "void f() {\n"
              "    S* s = new S();\n"
              "    (void)s;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:5:1]: (error) Memory leak: s [memleak]\n", errout_str());
    }

    void assign22() { // #9139
        const Settings s = settingsBuilder().library("posix.cfg").checkLibrary().build();
        check("void f(char tempFileName[256]) {\n"
              "    const int fd = socket(AF_INET, SOCK_PACKET, 0 );\n"
              "}", dinit(CheckOptions, $.cpp = true, $.s = &s));
        ASSERT_EQUALS("[test.cpp:3:1]: (error) Resource leak: fd [resourceLeak]\n", errout_str());

        check("void f() {\n"
              "    const void * const p = malloc(10);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:1]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void assign23() {
        const Settings s = settingsBuilder().library("posix.cfg").checkLibrary().build();
        check("void f() {\n"
              "    int n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14;\n"
              "    *&n1 = open(\"xx.log\", O_RDONLY);\n"
              "    *&(n2) = open(\"xx.log\", O_RDONLY);\n"
              "    *(&n3) = open(\"xx.log\", O_RDONLY);\n"
              "    *&*&n4 = open(\"xx.log\", O_RDONLY);\n"
              "    *&*&*&(n5) = open(\"xx.log\", O_RDONLY);\n"
              "    *&*&(*&n6) = open(\"xx.log\", O_RDONLY);\n"
              "    *&*(&*&n7) = open(\"xx.log\", O_RDONLY);\n"
              "    *(&*&n8) = open(\"xx.log\", O_RDONLY);\n"
              "    *&(*&*&(*&n9)) = open(\"xx.log\", O_RDONLY);\n"
              "    (n10) = open(\"xx.log\", O_RDONLY);\n"
              "    ((n11)) = open(\"xx.log\", O_RDONLY);\n"
              "    ((*&n12)) = open(\"xx.log\", O_RDONLY);\n"
              "    *(&(*&n13)) = open(\"xx.log\", O_RDONLY);\n"
              "    ((*&(*&n14))) = open(\"xx.log\", O_RDONLY);\n"
              "}\n", dinit(CheckOptions, $.cpp = true, $.s = &s));
        ASSERT_EQUALS("[test.cpp:17:1]: (error) Resource leak: n1 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n2 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n3 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n4 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n5 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n6 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n7 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n8 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n9 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n10 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n11 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n12 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n13 [resourceLeak]\n"
                      "[test.cpp:17:1]: (error) Resource leak: n14 [resourceLeak]\n",
                      errout_str());
    }

    void assign24() {
        check("void f() {\n" // #7440
              "    char* data = new char[100];\n"
              "    char** dataPtr = &data;\n"
              "    delete[] *dataPtr;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char* data = new char[100];\n"
              "    char** dataPtr = &data;\n"
              "    printf(\"test\");\n"
              "    delete[] *dataPtr;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #9279
              "    int* p = new int;\n"
              "    *p = 42;\n"
              "    g();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:8]: (information) --check-library: Function g() should have <noreturn> configuration [checkLibraryNoReturn]\n",
                      errout_str());

        check("void g();\n"
              "void f() {\n"
              "    int* p = new int;\n"
              "    *p = 42;\n"
              "    g();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:6:1]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void g() {}\n"
              "void f() {\n"
              "    int* p = new int;\n"
              "    *p = 42;\n"
              "    g();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:6:1]: (error) Memory leak: p [memleak]\n", errout_str());

        check("[[noreturn]] void g();\n"
              "void f() {\n"
              "    int* p = new int;\n"
              "    *p = 42;\n"
              "    g();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void g() { exit(1); }\n"
              "void f() {\n"
              "    int* p = new int;\n"
              "    *p = 42;\n"
              "    g();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void g() {}\n" // #10517
              "void f() {\n"
              "    char* p = malloc(10);\n"
              "    g();\n"
              "}\n");
        ASSERT_EQUALS("[test.c:5:1]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void assign25() {
        check("void f() {\n" // #11796
              "    int* p{ new int };\n"
              "    int* q(new int);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:1]: (error) Memory leak: p [memleak]\n"
                      "[test.cpp:4:1]: (error) Memory leak: q [memleak]\n",
                      errout_str());

        check("struct S : B {\n" // #12239
              "    void f();\n"
              "    void g();\n"
              "};\n"
              "void S::f() {\n"
              "    FD* fd(new FD(this));\n"
              "    fd->exec();\n"
              "}\n"
              "void S::g() {\n"
              "    FD* fd{ new FD(this) };\n"
              "    fd->exec();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("struct C {\n" // #12327
              "    char* m_p;\n"
              "    C(char* p) : m_p(p) {}\n"
              "};\n"
              "std::list<C> gli;\n"
              "void f() {\n"
              "    std::list<C> li;\n"
              "    char* p = new char[1];\n"
              "    C c(p);\n"
              "    li.push_back(c);\n"
              "    C c2(li.front());\n"
              "    delete[] c2.m_p;\n"
              "}\n"
              "void g() {\n"
              "    char* p = new char[1];\n"
              "    C c(p);\n"
              "    gli.push_back(c);\n"
              "}\n"
              "void h() {\n"
              "    std::list<C> li;\n"
              "    char* p = new char[1];\n"
              "    C c(p);\n"
              "    li.push_back(c);\n"
              "    delete[] li.front().m_p;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12890
              "    int** p;\n"
              "    S() {\n"
              "        p = std::malloc(sizeof(int*));\n"
              "        p[0] = new int;\n"
              "    }\n"
              "    ~S() {\n"
              "        delete p[0];\n"
              "        std::free(p);\n"
              "    }\n"
              "};\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void assign26() {
        check("void f(int*& x) {\n" // #8235
              "    int* p = (int*)malloc(10);\n"
              "    x = p ? p : nullptr;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(int*& x) {\n"
              "    int* p = (int*)malloc(10);\n"
              "    x = p != nullptr ? p : nullptr;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(const char* n) {\n" // #12724
              "    FILE* fp = fopen(n, \"r\");\n"
              "    bool b = (fp == NULL);\n"
              "    if (b)\n"
              "        return;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:6:1]: (error) Resource leak: fp [resourceLeak]\n", errout_str());
    }

    void memcpy1() { // #11542
        check("void f(char** old, char* value) {\n"
              "    char *str = strdup(value);\n"
              "    memcpy(old, &str, sizeof(char*));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void memcpy2() {
        check("void f(char* old, char* value, size_t len) {\n"
              "    char *str = strdup(value);\n"
              "    memcpy(old, str, len);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:1]: (error) Memory leak: str [memleak]\n", errout_str());
    }

    void isAutoDealloc() {
        check("void f() {\n"
              "    char *p = new char[100];"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:29]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void f() {\n"
              "    Fred *fred = new Fred;"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::string *str = new std::string;"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:40]: (error) Memory leak: str [memleak]\n", errout_str());

        check("class TestType {\n" // #9028
              "public:\n"
              "    char ca[12];\n"
              "};\n"
              "void f() {\n"
              "    TestType *tt = new TestType();\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:7:1]: (error) Memory leak: tt [memleak]\n", errout_str());

        check("void f(Bar& b) {\n" // #7622
              "    char* data = new char[10];\n"
              "    b = Bar(*new Foo(data));\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:1]: (information) --check-library: Function Foo() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n", errout_str());

        check("class B {};\n"
              "    class D : public B {};\n"
              "    void g() {\n"
              "        auto d = new D();\n"
              "        if (d) {}\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:6:1]: (error) Memory leak: d [memleak]\n", errout_str());

        check("struct S {\n" // #12354
              "    int i{};\n"
              "    void f();\n"
              "};\n"
              "void f(S* p, bool b) {\n"
              "    if (b)\n"
              "        p = new S();\n"
              "    p->f();\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:9:1]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void realloc1() {
        check("void f() {\n"
              "    void *p = malloc(10);\n"
              "    void *q = realloc(p, 20);\n"
              "    free(q)\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc2() {
        check("void f() {\n"
              "    void *p = malloc(10);\n"
              "    void *q = realloc(p, 20);\n"
              "}");
        ASSERT_EQUALS("[test.c:4:1]: (error) Memory leak: q [memleak]\n", errout_str());
    }

    void realloc3() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    char *q = (char*) realloc(p, 20);\n"
              "}");
        ASSERT_EQUALS("[test.c:4:1]: (error) Memory leak: q [memleak]\n", errout_str());
    }

    void realloc4() {
        check("void f(void *p) {\n"
              "    void * q = realloc(p, 10);\n"
              "    if (q == NULL)\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("[test.c:5:1]: (error) Memory leak: q [memleak]\n", errout_str());
    }

    void realloc5() {
        // #9292
        check("void * f(void * ptr, size_t size) {\n"
              "    void *datap = realloc(ptr, size);\n"
              "    if (size && !datap)\n"
              "        free(ptr);\n"
              "    return datap;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #9990
        check("void f() {\n"
              "    void * p1 = malloc(10);\n"
              "    if (!p1)\n"
              "        return;\n"
              "    void * p2 = realloc(p1, 42);\n"
              "    if (!p2) {\n"
              "        free(p1);\n"
              "        return;\n"
              "    }\n"
              "    free(p2);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void freopen1() {
        check("void f() {\n"
              "    void *p = fopen(name,a);\n"
              "    void *q = freopen(name, b, p);\n"
              "    fclose(q)\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void freopen2() {
        check("void f() {\n"
              "    void *p = fopen(name,a);\n"
              "    void *q = freopen(name, b, p);\n"
              "}");
        ASSERT_EQUALS("[test.c:4:1]: (error) Resource leak: q [resourceLeak]\n", errout_str());
    }

    void deallocuse1() {
        check("void f(char *p) {\n"
              "    free(p);\n"
              "    *p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.c:3:6]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n", errout_str());

        check("void f(char *p) {\n"
              "    free(p);\n"
              "    char c = *p;\n"
              "}");
        ASSERT_EQUALS("[test.c:3:15]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n", errout_str());
    }

    void deallocuse3() {
        check("void f(struct str *p) {\n"
              "    free(p);\n"
              "    p = p->next;\n"
              "}");
        ASSERT_EQUALS("[test.c:3:9]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n", errout_str());
    }

    void deallocuse4() {
        check("void f(char *p) {\n"
              "    free(p);\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("[test.c:2:5] -> [test.c:3:5]: (error) Returning/dereferencing 'p' after it is deallocated / released [deallocret]\n", errout_str());

        check("void f(char *p) {\n"
              "  if (!p) free(p);\n"
              "  return p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char *p) {\n"
              "  if (!p) delete p;\n"
              "  return p;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(char *p) {\n"
              "  if (!p) delete [] p;\n"
              "  return p;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(void* p) {\n"
              "   if (a) {\n"
              "      free(p);\n"
              "       return;\n"
              "   }\n"
              "   g(p);\n"
              "   return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void deallocuse5() {  // #4018
        check("void f(char *p) {\n"
              "    free(p), p = 0;\n"
              "    *p = 0;\n"  // <- Make sure pointer info is reset. It is NOT a freed pointer dereference
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void deallocuse6() {  // #4034
        check("void f(char *p) {\n"
              "    free(p);\n"
              "    x = p = foo();\n"  // <- p is not dereferenced
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void deallocuse7() {  // #6467, #6469, #6473, #6648
        check("struct Foo { int* ptr; };\n"
              "void f(Foo* foo) {\n"
              "    delete foo->ptr;\n"
              "    foo->ptr = new Foo;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("struct Foo { int* ptr; };\n"
              "void f(Foo* foo) {\n"
              "    delete foo->ptr;\n"
              "    x = *foo->ptr;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        TODO_ASSERT_EQUALS("[test.cpp:4:6]: (error) Dereferencing 'ptr' after it is deallocated / released [deallocuse]\n", "", errout_str());

        check("void parse() {\n"
              "    struct Buf {\n"
              "        Buf(uint32_t len) : m_buf(new uint8_t[len]) {}\n"
              "        ~Buf() { delete[]m_buf; }\n"
              "        uint8_t *m_buf;\n"
              "    };\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("struct Foo {\n"
              "    Foo();\n"
              "    Foo* ptr;\n"
              "    void func();\n"
              "};\n"
              "void bar(Foo* foo) {\n"
              "    delete foo->ptr;\n"
              "    foo->ptr = new Foo;\n"
              "    foo->ptr->func();\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo(void (*conv)(char**)) {\n"
              "  char * ptr=(char*)malloc(42);\n"
              "  free(ptr);\n"
              "  (*conv)(&ptr);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void deallocuse8() {  // #1765
        check("void f() {\n"
              "    int *ptr = new int;\n"
              "    delete(ptr);\n"
              "    *ptr = 0;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Dereferencing 'ptr' after it is deallocated / released [deallocuse]\n", errout_str());
    }

    void deallocuse9() {
        check("void f(Type* p) {\n" // #9781
              "  std::shared_ptr<Type> sp(p);\n"
              "  bool b = p->foo();\n"
              "  return b;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n" // #8635
              "    std::vector<std::unique_ptr<A>> array_;\n"
              "    A* foo() {\n"
              "        A* a = new A();\n"
              "        array_.push_back(std::unique_ptr<A>(a));\n"
              "        return a;\n"
              "    }\n"
              "};\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("int g(int *p) {\n" // #9838
              "    std::unique_ptr<int> temp(p);\n"
              "    return DoSomething(p);\n"
              "}\n"
              "int f() {\n"
              "    return g(new int(3));\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void deallocuse10() {
        check("void f(char* p) {\n"
              "    free(p);\n"
              "    p[0] = 10;\n"
              "}\n");
        ASSERT_EQUALS("[test.c:3:5]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n", errout_str());

        check("int f(int* p) {\n"
              "    free(p);\n"
              "    return p[1];\n"
              "}\n");
        ASSERT_EQUALS("[test.c:2:5] -> [test.c:3:5]: (error) Returning/dereferencing 'p' after it is deallocated / released [deallocret]\n", errout_str());
    }

    void deallocuse11() { // #8302
        check("int f() {\n"
              "  int *array = new int[42];\n"
              "  delete [] array;\n"
              "  return array[1];" // <<
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:3] -> [test.cpp:4:3]: (error) Returning/dereferencing 'array' after it is deallocated / released [deallocret]\n", errout_str());

        check("int f() {\n"
              "  int *array = (int*)malloc(40);\n"
              "  free(array);\n"
              "  return array[1];" // <<
              "}");
        ASSERT_EQUALS("[test.c:3:3] -> [test.c:4:3]: (error) Returning/dereferencing 'array' after it is deallocated / released [deallocret]\n", errout_str());
    }

    void deallocuse12() {
        check("struct foo { int x; }\n"
              "void f1(struct foo *f) {\n"
              "  free(f);\n"
              "}\n"
              "void f2(struct foo *f, int *out) {\n"
              "  *out = f->x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void deallocuse13() {
        check("void f() {\n" // #9695
              "    auto* a = new int[2];\n"
              "    delete[] a;\n"
              "    a[1] = 0;\n"
              "    auto* b = static_cast<int*>(malloc(8));\n"
              "    free(b);\n"
              "    b[1] = 0;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Dereferencing 'a' after it is deallocated / released [deallocuse]\n"
                      "[test.cpp:7:5]: (error) Dereferencing 'b' after it is deallocated / released [deallocuse]\n",
                      errout_str());
    }

    void deallocuse14() {
        check("struct S { void f(); };\n" // #10905
              "void g() {\n"
              "    S* s = new S;\n"
              "    delete s;\n"
              "    s->f();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:5:5]: (error) Dereferencing 's' after it is deallocated / released [deallocuse]\n",
                      errout_str());

        check("void f() {\n"
              "    int *p = (int*)malloc(4);\n"
              "    free(p);\n"
              "    if (*p == 5) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.c:4:10]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n",
                      errout_str());

        check("int g(int);\n"
              "void f(int* p) {\n"
              "    free(p);\n"
              "    g(*p);\n"
              "}\n"
              "int h(int* p) {\n"
              "    free(p);\n"
              "    return g(*p);\n"
              "}\n");
        ASSERT_EQUALS("[test.c:4:8]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n"
                      "[test.c:7:5] -> [test.c:8:5]: (error) Returning/dereferencing 'p' after it is deallocated / released [deallocret]\n",
                      errout_str());

        check("int g(int);\n"
              "void f(int* p) {\n"
              "    free(p);\n"
              "    g(1 + *p);\n"
              "}\n"
              "int h(int* p) {\n"
              "    free(p);\n"
              "    return g(1 + *p);\n"
              "}\n");
        ASSERT_EQUALS("[test.c:4:12]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n"
                      "[test.c:7:5] -> [test.c:8:5]: (error) Returning/dereferencing 'p' after it is deallocated / released [deallocret]\n",
                      errout_str());

        check("int g(int, int);\n"
              "void f(int* p) {\n"
              "    free(p);\n"
              "    g(0, 1 + *p);\n"
              "}\n"
              "int h(int* p) {\n"
              "    free(p);\n"
              "    return g(0, 1 + *p);\n"
              "}\n");
        ASSERT_EQUALS("[test.c:4:15]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n"
                      "[test.c:7:5] -> [test.c:8:5]: (error) Returning/dereferencing 'p' after it is deallocated / released [deallocret]\n",
                      errout_str());

        check("void f() {\n"
              "    FOREACH(callables, ());\n"
              "}\n");
        ASSERT_EQUALS("[test.c:2:27]: (information) --check-library: Function FOREACH() should have <noreturn> configuration [checkLibraryNoReturn]\n", errout_str()); // don't crash

        check("int f() {\n" // #12321
              "    std::invoke([](int i) {\n"
              "        int* p = (int*)malloc(4);\n"
              "        *p = 0;\n"
              "        if (i) {\n"
              "            free(p);\n"
              "            return;\n"
              "        }\n"
              "        free(p);\n"
              "    }, 1);\n"
              "    return 0;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void deallocuse15() {
        check("bool FileExists(const char* filename) {\n" // #12490
              "    FILE* f = fopen(filename, \"rb\");\n"
              "    if (f) fclose(f);\n"
              "    return f;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool f() {\n" // #12590
              "    FILE* fd = fopen(\"/foo/bar\", \"w\");\n"
              "    if (fd == nullptr)\n"
              "        return false;\n"
              "    return fclose(fd) == 0;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("int f(const char* fileName) {\n" // #13136
              "    FILE* h = fopen(fileName, \"rb\");\n"
              "    if (fseek(h, 0L, SEEK_END) == -1)\n"
              "        fclose(h);\n"
              "    int i = ftell(h);\n"
              "    return i;\n"
              "}\n");
        ASSERT_EQUALS("[test.c:5:19]: (error) Dereferencing 'h' after it is deallocated / released [deallocuse]\n", errout_str());
    }

    void deallocuse16() {
        check("void f() {\n"
              "    int *a = nullptr;\n"
              "    int *c = new int;\n"
              "    delete (a, c);\n"
              "    *c = 10;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:5:6]: (error) Dereferencing 'c' after it is deallocated / released [deallocuse]\n", errout_str());
    }

    void doublefree1() {  // #3895
        check("void f(char *p) {\n"
              "    if (x)\n"
              "        free(p);\n"
              "    else\n"
              "        p = 0;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.c:3:9] -> [test.c:6:5]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());

        check(
            "void foo(char *p) {\n"
            "  free(p);\n"
            "  free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:2:3] -> [test.c:3:3]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());

        check(
            "void foo(char *p, char *r) {\n"
            "  free(p);\n"
            "  free(r);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo() {\n"
            "  free(p);\n"
            "  free(r);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(char *p) {\n"
            "  if (x < 3) free(p);\n"
            "  else { if (x > 9) free(p); }\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(char *p) {\n"
            "  free(p);\n"
            "  getNext(&p);\n"
            "  free(p);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(char *p) {\n"
            "  free(p);\n"
            "  bar();\n"
            "  free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:2:3] -> [test.c:4:3]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());

        check(
            "void foo(char *p) {\n"
            "  free(p);\n"
            "  printf(\"Freed memory at location %x\", p);\n"
            "  free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:3:41]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n"
                      "[test.c:2:3] -> [test.c:4:3]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n",
                      errout_str());

        check(
            "void foo(FILE *p) {\n"
            "  fclose(p);\n"
            "  fclose(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:2:3] -> [test.c:3:3]: (error) Resource handle 'p' freed twice. [doubleFree]\n", errout_str());

        check(
            "void foo(FILE *p, FILE *r) {\n"
            "  fclose(p);\n"
            "  fclose(r);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(FILE *p) {\n"
            "  if (x < 3) fclose(p);\n"
            "  else { if (x > 9) fclose(p); }\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(FILE *p) {\n"
            "  fclose(p);\n"
            "  gethandle(&p);\n"
            "  fclose(p);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(FILE *p) {\n"
            "  fclose(p);\n"
            "  gethandle();\n"
            "  fclose(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:2:3] -> [test.c:4:3]: (error) Resource handle 'p' freed twice. [doubleFree]\n", errout_str());

        check(
            "void foo(Data* p) {\n"
            "  free(p->a);\n"
            "  free(p->b);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void f() {\n"
            "    char *p; p = malloc(100);\n"
            "    if (x) {\n"
            "        free(p);\n"
            "        exit();\n"
            "    }\n"
            "    free(p);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void f() {\n"
            "    char *p; p = malloc(100);\n"
            "    if (x) {\n"
            "        free(p);\n"
            "        x = 0;\n"
            "    }\n"
            "    free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:4:9] -> [test.c:7:5]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());

        check(
            "void f() {\n"
            "    char *p; p = do_something();\n"
            "    free(p);\n"
            "    p = do_something();\n"
            "    free(p);\n"
            "}");
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(char *p) {\n"
            "  delete p;\n"
            "  delete p;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:10] -> [test.cpp:3:10]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());

        check(
            "void foo(char *p, char *r) {\n"
            "  delete p;\n"
            "  delete r;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(P p) {\n"
            "  delete p.x;\n"
            "  delete p;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(char **p) {\n"
            "  delete p[0];\n"
            "  delete p[1];\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(char *p) {\n"
            "  delete p;\n"
            "  getNext(&p);\n"
            "  delete p;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(char *p) {\n"
            "  delete p;\n"
            "  bar();\n"
            "  delete p;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:10] -> [test.cpp:4:10]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());

        check(
            "void foo(char *p) {\n"
            "  delete[] p;\n"
            "  delete[] p;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:3:12]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());

        check(
            "void foo(char *p, char *r) {\n"
            "  delete[] p;\n"
            "  delete[] r;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(char *p) {\n"
            "  delete[] p;\n"
            "  getNext(&p);\n"
            "  delete[] p;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(char *p) {\n"
            "  delete[] p;\n"
            "  bar();\n"
            "  delete[] p;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:4:12]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());

        check(
            "LineMarker::~LineMarker() {\n"
            "  delete pxpm;\n"
            "}\n"
            "LineMarker &LineMarker::operator=(const LineMarker &) {\n"
            "  delete pxpm;\n"
            "  pxpm = NULL;\n"
            "  return *this;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo()\n"
            "{\n"
            "  int* ptr; ptr = NULL;\n"
            "  try\n"
            "    {\n"
            "      ptr = new int(4);\n"
            "    }\n"
            "  catch(...)\n"
            "    {\n"
            "      delete ptr;\n"
            "      throw;\n"
            "    }\n"
            "  delete ptr;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "int foo()\n"
            "{\n"
            "   int* a; a = new int;\n"
            "   bool doDelete; doDelete = true;\n"
            "   if (a != 0)\n"
            "   {\n"
            "       doDelete = false;\n"
            "       delete a;\n"
            "   }\n"
            "   if(doDelete)\n"
            "       delete a;\n"
            "   return 0;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        TODO_ASSERT_EQUALS("", "[test.cpp:8:8] -> [test.cpp:11:15]: (error) Memory pointed to by 'a' is freed twice. [doubleFree]\n", errout_str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    while(true) {\n"
            "        x = new char[100];\n"
            "        if (y++ > 100)\n"
            "            break;\n"
            "        delete[] x;\n"
            "    }\n"
            "    delete[] x;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    for (int i = 0; i < 10000; i++) {\n"
            "        x = new char[100];\n"
            "        delete[] x;\n"
            "    }\n"
            "    delete[] x;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        TODO_ASSERT_EQUALS("[test.cpp:8]: (error) Memory pointed to by 'x' is freed twice.\n", "", errout_str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    while (isRunning()) {\n"
            "        x = new char[100];\n"
            "        delete[] x;\n"
            "    }\n"
            "    delete[] x;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        TODO_ASSERT_EQUALS("[test.cpp:8]: (error) Memory pointed to by 'x' is freed twice.\n", "", errout_str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    while (isRunning()) {\n"
            "        x = malloc(100);\n"
            "        free(x);\n"
            "    }\n"
            "    free(x);\n"
            "}");
        TODO_ASSERT_EQUALS("[test.c:8]: (error) Memory pointed to by 'x' is freed twice.\n", "", errout_str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    for (;;) {\n"
            "        x = new char[100];\n"
            "        if (y++ > 100)\n"
            "            break;\n"
            "        delete[] x;\n"
            "    }\n"
            "    delete[] x;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void foo(int y)\n"
            "{\n"
            "    char * x; x = NULL;\n"
            "    do {\n"
            "        x = new char[100];\n"
            "        if (y++ > 100)\n"
            "            break;\n"
            "        delete[] x;\n"
            "    } while (true);\n"
            "    delete[] x;\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void f()\n"
            "{\n"
            "    char *p; p = 0;\n"
            "    if (x < 100) {\n"
            "        p = malloc(10);\n"
            "        free(p);\n"
            "    }\n"
            "    free(p);\n"
            "}");
        ASSERT_EQUALS("[test.c:6:9] -> [test.c:8:5]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());

        check(
            "void MyFunction()\n"
            "{\n"
            "    char* data; data = new char[100];\n"
            "    try\n"
            "    {\n"
            "    }\n"
            "    catch(err)\n"
            "    {\n"
            "        delete[] data;\n"
            "        MyThrow(err);\n"
            "    }\n"
            "    delete[] data;\n"
            "}\n"

            "void MyThrow(err)\n"
            "{\n"
            "    throw(err);\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check(
            "void MyFunction()\n"
            "{\n"
            "    char* data; data = new char[100];\n"
            "    try\n"
            "    {\n"
            "    }\n"
            "    catch(err)\n"
            "    {\n"
            "        delete[] data;\n"
            "        MyExit(err);\n"
            "    }\n"
            "    delete[] data;\n"
            "}\n"

            "void MyExit(err)\n"
            "{\n"
            "    exit(err);\n"
            "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check( // #6252
            "struct Wrapper {\n"
            "    Thing* m_thing;\n"
            "    Wrapper() : m_thing(0) {\n"
            "    }\n"
            "    ~Wrapper() {\n"
            "        delete m_thing;\n"
            "    }\n"
            "    void changeThing() {\n"
            "        delete m_thing;\n"
            "        m_thing = new Thing;\n"
            "    }\n"
            "};", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        // #7401
        check("void pCodeLabelDestruct(pCode *pc) {\n"
              "    free(PCL(pc)->label);\n"
              "    free(pc);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree2() {  // #3891
        check("void *f(int a) {\n"
              "    char *p = malloc(10);\n"
              "    if (a == 2) { free(p); return ((void*)1); }\n"
              "    free(p);\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree3() {  // #4914
        check("void foo() {\n"
              "   bool done = false;\n"
              "   do {\n"
              "       char *bar = malloc(10)\n"
              "       if(condition()) {\n"
              "           free(bar);\n"
              "           continue;\n"
              "       }\n"
              "       done = true;\n"
              "       free(bar)\n"
              "   } while(!done);\n"
              "   return;"
              "}"
              );
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree4() {
        check("void f(char *p) {\n" // #5451 - exit
              "  if (x) {\n"
              "    free(p);\n"
              "    exit(1);\n"
              "  }\n"
              "  free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(void* p, int i) {\n" // #11391
              "    if (i)\n"
              "        goto cleanup;\n"
              "    free(p);\n"
              "    exit(0);\n"
              "cleanup:\n"
              "    free(p);\n"
              "    exit(1);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree5() {  // #5522
        check("void f(char *p) {\n"
              "  free(p);\n"
              "  x = (q == p);\n"
              "  free(p);\n"
              "}");
        ASSERT_EQUALS("[test.c:2:3] -> [test.c:4:3]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());
    }

    void doublefree6() { // #7685
        check("void do_wordexp(FILE *f) {\n"
              "  free(getword(f));\n"
              "  fclose(f);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree7() {
        check("void f(char *p, int x) {\n"
              "    free(p);\n"
              "    if (x && (p = malloc(10)))\n"
              "        free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(char *p, int x) {\n"
              "    delete[] p;\n"
              "    if (x && (p = new char[10]))\n"
              "        delete[] p;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree8() {
        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int> x(i);\n"
              "    delete i;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:4:12]: (error) Memory pointed to by 'i' is freed twice. [doubleFree]\n", errout_str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    delete i;\n"
              "    std::unique_ptr<int> x(i);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:5] -> [test.cpp:4:28]: (error) Memory pointed to by 'i' is freed twice. [doubleFree]\n", errout_str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int> x{i};\n"
              "    delete i;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:4:12]: (error) Memory pointed to by 'i' is freed twice. [doubleFree]\n", errout_str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::shared_ptr<int> x(i);\n"
              "    delete i;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:4:12]: (error) Memory pointed to by 'i' is freed twice. [doubleFree]\n", errout_str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::shared_ptr<int> x{i};\n"
              "    delete i;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:4:12]: (error) Memory pointed to by 'i' is freed twice. [doubleFree]\n", errout_str());

        // Check for use-after-free FP
        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::shared_ptr<int> x{i};\n"
              "    *i = 123;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int * i = new int[1];\n"
              "    std::unique_ptr<int[]> x(i);\n"
              "    delete i;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:4:12]: (error) Memory pointed to by 'i' is freed twice. [doubleFree]\n", errout_str());

        check("using namespace std;\n" // #9708
              "void f() {\n"
              "    int* i = new int;\n"
              "    unique_ptr<int> x(i);\n"
              "    delete i;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:5] -> [test.cpp:5:12]: (error) Memory pointed to by 'i' is freed twice. [doubleFree]\n", errout_str());
    }

    void doublefree9() {
        check("struct foo {\n"
              "    int* get(int) { return new int(); }\n"
              "};\n"
              "void f(foo* b) {\n"
              "    std::unique_ptr<int> x(b->get(0));\n"
              "    std::unique_ptr<int> y(b->get(1));\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree10() {
        check("void f(char* s) {\n"
              "    char *p = malloc(strlen(s));\n"
              "    if (p != NULL) {\n"
              "        strcat(p, s);\n"
              "        if (strlen(s) != 10)\n"
              "            free(p); p = NULL;\n"
              "    }\n"
              "    if (p != NULL)\n"
              "        free(p);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(char* s) {\n"
              "    char *p = malloc(strlen(s));\n"
              "    if (p != NULL) {\n"
              "        strcat(p, s);\n"
              "        if (strlen(s) != 10)\n"
              "            free(p), p = NULL;\n"
              "    }\n"
              "    if (p != NULL)\n"
              "        free(p);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree11() {
        check("void f() {\n"
              "    void * p = malloc(5);\n"
              "    void * q = realloc(p, 10);\n"
              "    if (q == NULL) {\n"
              "        free(p);\n"
              "        return;\n"
              "    }\n"
              "    free(p);\n"
              "    if (q == NULL)\n"
              "        return;\n"
              "    free(q);\n"
              "}");
        ASSERT_EQUALS("[test.c:3:16] -> [test.c:8:5]: (error) Memory pointed to by 'p' is freed twice. [doubleFree]\n", errout_str());
    }

    void doublefree12() { // #10502
        check("int f(FILE *fp, const bool b) {\n"
              "    if (b)\n"
              "        return fclose(fp);\n"
              "    fclose(fp);\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree13() { // #11008
        check("struct buf_t { void* ptr; };\n"
              "void f() {\n"
              "    struct buf_t buf;\n"
              "    if ((buf.ptr = malloc(10)) == NULL)\n"
              "        return;\n"
              "    free(buf.ptr);\n"
              "    if ((buf.ptr = malloc(10)) == NULL)\n"
              "        return;\n"
              "    free(buf.ptr);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree14() { // #9708
        check("using namespace std;\n"
              " \n"
              "int main()\n"
              "{\n"
              "    int *i = new int;\n"
              "    unique_ptr<int> x(i);\n"
              "    delete i;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:6:5] -> [test.cpp:7:12]: (error) Memory pointed to by 'i' is freed twice. [doubleFree]\n", errout_str());

        check("using namespace std;\n"
              " \n"
              "int main()\n"
              "{\n"
              "    int *i = new int;\n"
              "    unique_ptr<int> x(i);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree15() { // #11966
        check("void f(FILE* fp) {\n"
              "    static_cast<void>(fclose(fp));\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree16() { // #12236
        check("void f() {\n"
              "    FILE* f = fopen(\"abc\", \"r\");\n"
              "    decltype(fclose(f)) y;\n"
              "    y = fclose(f);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    FILE *fp = fopen(\"abc\", \"r\");\n"
              "    if (({\n"
              "        __typeof__(fclose(fp)) r;\n"
              "        r = (fclose(fp));\n"
              "        r;\n"
              "    })) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree17() {
        check("void f() {\n"
              "    int *a = nullptr;\n"
              "    int *b = nullptr;\n"
              "    int *c = new int;\n"
              "    delete (a, c);\n"
              "    delete (b, c);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:5:5] -> [test.cpp:6:16]: (error) Memory pointed to by 'c' is freed twice. [doubleFree]\n", errout_str());
    }

    void doublefree18() {
        check("typedef struct {\n" // #13918
              "    FILE * fp;\n"
              "} S;\n"
              "void f(S* s, FILE* x) {\n"
              "    if (fclose(s->fp)) {}\n"
              "    s->fp = x;\n"
              "    if (fclose(s->fp)) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree19() {
        check("struct S {\n"
              "  void *x;\n"
              "};\n"
              "void f(struct S *p)\n"
              "{\n"
              "  free(p->x);\n"
              "  free(p->x);\n"
              "}\n");
        ASSERT_EQUALS("[test.c:6:3] -> [test.c:7:3]: (error) Memory pointed to by 'x' is freed twice. [doubleFree]\n", errout_str());
    }

    void exit1() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    exit(0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void exit2() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    fatal_error();\n"
              "}");
        ASSERT_EQUALS("[test.c:3:18]: (information) --check-library: Function fatal_error() should have <noreturn> configuration [checkLibraryNoReturn]\n",
                      errout_str());
    }

    void exit3() {
        check("void f() {\n"
              "  char *p = malloc(100);\n"
              "  if (x) {\n"
              "    free(p);\n"
              "    ::exit(0);\n"
              "  }"
              "  free(p);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  char *p = malloc(100);\n"
              "  if (x) {\n"
              "    free(p);\n"
              "    std::exit(0);\n"
              "  }"
              "  free(p);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void exit4() {
        check("void __attribute__((__noreturn__)) (*func_notret)(void);\n"
              "int main(int argc) {\n"
              "    void* ptr = malloc(1000);\n"
              "    if (argc == 1) {\n"
              "        free(ptr);\n"
              "        func_notret();\n"
              "    }\n"
              "    free(ptr);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void functioncall1() {
        check("void f(struct S *p) {\n"
              "  p->x = malloc(10);\n"
              "  free(p->x);\n"
              "  p->x = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(s_t s) {\n" // #11061
              "    s->p = (char*)malloc(10);\n"
              "    free((void*)s->p);\n"
              "    s->p = NULL;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {};\n"
              "void f(int i, std::vector<std::unique_ptr<S>> &v) {\n"
              "    if (i < 1) {\n"
              "        auto s = new S;\n"
              "        v.push_back(std::unique_ptr<S>(s));\n"
              "    }\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str()); // don't crash

        check("void g(size_t len) {\n" // #12365
              "    char* b = new char[len + 1]{};\n"
              "    std::string str = std::string(b);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:1]: (error) Memory leak: b [memleak]\n", errout_str());
    }

    void goto1() {
        check("static void f() {\n"
              "    int err = -ENOMEM;\n"
              "    char *reg = malloc(100);\n"
              "    if (err) {\n"
              "        free(reg);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void goto2() { // #4231
        check("static char * f() {\n"
              "x:\n"
              "    char *p = malloc(100);\n"
              "    if (err) {\n"
              "        free(p);\n"
              "        goto x;\n"
              "    }\n"
              "    return p;\n"  // no error since there is a goto
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void goto3() { // #11431
        check("void f() {\n"
              "    int* p = (int*)malloc(2);\n"
              "    if (!p) {\n"
              "        p = (int*)malloc(1);\n"
              "        if (!p)\n"
              "            goto err;\n"
              "    }\n"
              "    free(p);\n"
              "err:\n"
              "    (void)0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse1() {
        check("int f() {\n"
              "    char *p = NULL;\n"
              "    if (x) { p = malloc(10); }\n"
              "    else { return 0; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse2() {
        check("int f() {\n"
              "    char *p = NULL;\n"
              "    if (x) { p = malloc(10); }\n"
              "    else { return 0; }\n"
              "}");
        ASSERT_EQUALS("[test.c:5:1]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void ifelse3() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (!p) { return; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("char * f(size_t size) {"
              "    void *p = malloc(1);"
              "    if (!p && size != 0)"
              "        return NULL;"
              "    return p;"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (p) { } else { return; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3866 - UNLIKELY
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (UNLIKELY(!p)) { return; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse4() {
        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x) { p = malloc(10); }\n"
              "    if (x) { free(p); }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x) {\n"
              "    char *p;\n"
              "    if (x) { p = malloc(10); }\n"
              "    if (!x) { return; }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse5() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    if (!p && x) { p = malloc(10); }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse6() { // #3370
        check("void f(int x) {\n"
              "    int *a = malloc(20);\n"
              "    if (x)\n"
              "        free(a);\n"
              "    else\n"
              "        a = 0;\n"
              "}");
        ASSERT_EQUALS("[test.c:6:9]: (error) Memory leak: a [memleak]\n", errout_str());
    }

    void ifelse7() { // #5576
        check("void f() {\n"
              "    int x = malloc(20);\n"
              "    if (x < 0)\n"  // assume negative value indicates its unallocated
              "        return;\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse8() { // #5747
        check("int f() {\n"
              "    int fd = socket(AF_INET, SOCK_PACKET, 0 );\n"
              "    if (fd == -1)\n"
              "        return -1;\n"
              "    return fd;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    int fd = socket(AF_INET, SOCK_PACKET, 0 );\n"
              "    if (fd != -1)\n"
              "        return fd;\n"
              "    return -1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse9() { // #5273
        check("void f() {\n"
              "    char *p = malloc(100);\n"
              "    if (dostuff(p==NULL,0))\n"
              "        return;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse10() { // #8794
        check("void f() {\n"
              "    void *x = malloc(1U);\n"
              "    if (!(x != NULL))\n"
              "        return;\n"
              "    free(x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse11() { // #8365
        check("void f() {\n"
              "    void *p;\n"
              "    if (NULL == (p = malloc(4)))\n"
              "        return;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse12() { // #8340
        check("void f(char **p) {\n"
              "    if ((*p = malloc(4)) == NULL)\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse13() { // #8392
        check("int f(int fd, const char *mode) {\n"
              "    char *path;\n"
              "    if (fd == -1 || (path = (char *)malloc(10)) == NULL)\n"
              "        return 1;\n"
              "    free(path);\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(int fd, const char *mode) {\n"
              "    char *path;\n"
              "    if ((path = (char *)malloc(10)) == NULL || fd == -1)\n"
              "        return 1;\n" // <- memory leak
              "    free(path);\n"
              "    return 0;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4] memory leak", "", errout_str());
    }

    void ifelse14() { // #9130
        check("char* f() {\n"
              "    char* buf = malloc(10);\n"
              "    if (buf == (char*)NULL)\n"
              "        return NULL;\n"
              "    return buf;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse15() { // #9206
        check("struct SSS { int a; };\n"
              "SSS* global_ptr;\n"
              "void test_alloc() {\n"
              "   if ( global_ptr = new SSS()) {}\n"
              "   return;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("FILE* hFile;\n"
              "int openFile( void ) {\n"
              "   if ((hFile = fopen(\"1.txt\", \"wb\" )) == NULL) return 0;\n"
              "   return 1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse16() { // #9635
        check("void f(void) {\n"
              "    char *p;\n"
              "    if(p = malloc(4), p == NULL)\n"
              "        return;\n"
              "    free(p);\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(void) {\n"
              "    char *p, q;\n"
              "    if(p = malloc(4), q = 1, p == NULL)\n"
              "        return;\n"
              "    free(p);\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse17() {
        check("int *f() {\n"
              "    int *p = realloc(nullptr, 10);\n"
              "    if (!p)\n"
              "        return NULL;\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int *f() {\n"
              "    int *p = realloc(nullptr, 10);\n"
              "    if (!!(!p))\n"
              "        return NULL;\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse18() {
        check("void f() {\n"
              "    void * p = malloc(10);\n"
              "    void * q = realloc(p, 20);\n"
              "    if (q == 0)\n"
              "        return;\n"
              "    free(q);\n"
              "}");
        ASSERT_EQUALS("[test.c:5:9]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void f() {\n"
              "    void * p = malloc(10);\n"
              "    void * q = realloc(p, 20);\n"
              "    if (q != 0) {\n"
              "        free(q);\n"
              "        return;\n"
              "    } else\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("[test.c:8:9]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void ifelse19() {
        check("void f() {\n"
              "    static char * a;\n"
              "    char * b = realloc(a, 10);\n"
              "    if (!b)\n"
              "        return;\n"
              "    a = b;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse20() {
        check("void f() {\n"
              "    if (x > 0)\n"
              "        void * p1 = malloc(5);\n"
              "    else\n"
              "        void * p2 = malloc(2);\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.c:3:30]: (error) Memory leak: p1 [memleak]\n"
                      "[test.c:5:30]: (error) Memory leak: p2 [memleak]\n", errout_str());

        check("void f() {\n"
              "    if (x > 0)\n"
              "        void * p1 = malloc(5);\n"
              "    else\n"
              "        void * p2 = malloc(2);\n"
              "}");
        ASSERT_EQUALS("[test.c:3:30]: (error) Memory leak: p1 [memleak]\n"
                      "[test.c:5:30]: (error) Memory leak: p2 [memleak]\n", errout_str());
    }

    void ifelse21() {
        check("void f() {\n"
              "    if (y) {\n"
              "        void * p;\n"
              "        if (x > 0)\n"
              "            p = malloc(5);\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.c:6:5]: (error) Memory leak: p [memleak]\n",  errout_str());
    }

    void ifelse22() { // #10187
        check("int f(const char * pathname, int flags) {\n"
              "    int fd = socket(pathname, flags);\n"
              "    if (fd >= 0)\n"
              "        return fd;\n"
              "    return -1;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(const char * pathname, int flags) {\n"
              "    int fd = socket(pathname, flags);\n"
              "    if (fd <= -1)\n"
              "        return -1;\n"
              "    return fd;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse23() { // #5473
        check("void f() {\n"
              "    if (FILE* fp = fopen(\"x\", \"r\")) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.c:2:38]: (error) Resource leak: fp [resourceLeak]\n", errout_str());
    }

    void ifelse24() { // #1733
        const Settings s = settingsBuilder().library("std.cfg").library("posix.cfg").checkLibrary().build();

        check("void f() {\n"
              "    char* temp = strdup(\"temp.txt\");\n"
              "    FILE* fp;\n"
              "    if (NULL == x || NULL == (fp = fopen(temp, \"rt\")))\n"
              "        return;\n"
              "}\n", s);
        ASSERT_EQUALS("[test.cpp:5:9]: (error) Memory leak: temp [memleak]\n"
                      "[test.cpp:6:1]: (error) Memory leak: temp [memleak]\n"
                      "[test.cpp:6:1]: (error) Resource leak: fp [resourceLeak]\n",
                      errout_str());

        check("FILE* f() {\n"
              "    char* temp = strdup(\"temp.txt\");\n"
              "    FILE* fp = fopen(temp, \"rt\");\n"
              "    return fp;\n"
              "}\n", s);
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Memory leak: temp [memleak]\n", errout_str());

        check("FILE* f() {\n"
              "    char* temp = strdup(\"temp.txt\");\n"
              "    FILE* fp = NULL;\n"
              "    fopen_s(&fp, temp, \"rt\");\n"
              "    return fp;\n"
              "}\n", s);
        ASSERT_EQUALS("[test.cpp:5:5]: (error) Memory leak: temp [memleak]\n", errout_str());

        check("void f() {\n"
              "    char* temp = strdup(\"temp.txt\");\n"
              "    FILE* fp = fopen(\"a.txt\", \"rb\");\n"
              "    if (fp)\n"
              "        freopen(temp, \"rt\", fp);\n"
              "}\n", s);
        ASSERT_EQUALS("[test.cpp:6:1]: (error) Memory leak: temp [memleak]\n"
                      "[test.cpp:6:1]: (error) Resource leak: fp [resourceLeak]\n",
                      errout_str());

        check("FILE* f() {\n"
              "    char* temp = strdup(\"temp.txt\");\n"
              "    return fopen(temp, \"rt\");\n"
              "}\n", s);
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: temp\n", "", errout_str());
    }

    void ifelse25() { // #9966
        check("void f() {\n"
              "    void *p, *p2;\n"
              "    if((p2 = p = malloc(10)) == NULL)\n"
              "        return;\n"
              "    (void)p;\n"
              "    free(p2);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse26() { // don't crash
        check("union tidi {\n"
              "    long long ti;\n"
              "    unsigned int di[2];\n"
              "};\n"
              "void f(long long val) {\n"
              "    if (val == ({ union tidi d = {.di = {0x0, 0x80000000}}; d.ti; })) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse27() {
        check("struct key { void* p; };\n"
              "int f(struct key** handle) {\n"
              "    struct key* key;\n"
              "    if (!(key = calloc(1, sizeof(*key))))\n"
              "        return 0;\n"
              "    if (!(key->p = malloc(4))) {\n"
              "        free(key);\n"
              "        return 0;\n"
              "    }\n"
              "    *handle = key;\n"
              "    return 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse28() { // #11038
        check("char * f(void) {\n"
              "   char *buf = (char*)malloc(42*sizeof(char));\n"
              "   char *temp;\n"
              "   if ((temp = (char*)realloc(buf, 16)) != NULL)\n"
              "   {  buf = temp;  }\n"
              "   return buf;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse29() { // #13296
        check("struct S {\n"
              "    typedef int (S::* func_t)(int) const;\n"
              "    void f(func_t pfn);\n"
              "    int g(int) const;\n"
              "};\n"
              "void S::f(func_t pfn) {\n"
              "    if (pfn == (func_t)&S::g) {}\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str()); // don't crash
    }

    void switch1() {
        check("void f() {\n"
              "    char *p = 0;\n"
              "    switch (x) {\n"
              "    case 123: p = malloc(100); break;\n"
              "    default: return;\n"
              "    }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void loop1() {
        // test the handling of { }
        check("void f() {\n"
              "    char *p;\n"
              "    for (i=0;i<5;i++) { }\n"
              "    if (x) { free(p) }\n"
              "    else { a = p; }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void loop2() {
        check("void f() {\n" // #11786
              "    int* p = (int*)malloc(sizeof(int));\n"
              "    if (1) {\n"
              "        while (0) {}\n"
              "        free(p);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(node **p) {\n"
              "    node* n = *p;\n"
              "    if (n->left == NULL) {\n"
              "        *p = n->right;\n"
              "        free(n);\n"
              "    }\n"
              "    else if (n->right == NULL) {\n"
              "        *p = n->left;\n"
              "        free(n);\n"
              "    }\n"
              "    else {\n"
              "        for (int i = 0; i < 4; ++i) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void mismatchAllocDealloc() {
        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    free(f);\n"
              "}");
        ASSERT_EQUALS("[test.c:2:12] -> [test.c:3:5]: (error) Mismatching allocation and deallocation: f [mismatchAllocDealloc]\n", errout_str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    free((void*)f);\n"
              "}");
        ASSERT_EQUALS("[test.c:2:12] -> [test.c:3:5]: (error) Mismatching allocation and deallocation: f [mismatchAllocDealloc]\n", errout_str());

        check("void f() {\n"
              "    char *cPtr = new char[100];\n"
              "    delete[] cPtr;\n"
              "    cPtr = new char[100]('x');\n"
              "    delete[] cPtr;\n"
              "    cPtr = new char[100];\n"
              "    delete cPtr;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:6:12] -> [test.cpp:7:12]: (error) Mismatching allocation and deallocation: cPtr [mismatchAllocDealloc]\n", errout_str());

        check("void f() {\n"
              "    char *cPtr = new char[100];\n"
              "    free(cPtr);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:18] -> [test.cpp:3:5]: (error) Mismatching allocation and deallocation: cPtr [mismatchAllocDealloc]\n", errout_str());

        check("void f() {\n"
              "    char *cPtr = new (buf) char[100];\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int * i = new int[1];\n"
              "    std::unique_ptr<int> x(i);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:15] -> [test.cpp:3:28]: (error) Mismatching allocation and deallocation: i [mismatchAllocDealloc]\n", errout_str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int[]> x(i);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:15] -> [test.cpp:3:30]: (error) Mismatching allocation and deallocation: i [mismatchAllocDealloc]\n", errout_str());

        check("void f() {\n"
              "   void* a = malloc(1);\n"
              "   void* b = freopen(f, p, a);\n"
              "   free(b);\n"
              "}");
        ASSERT_EQUALS("[test.c:2:14] -> [test.c:3:14]: (error) Mismatching allocation and deallocation: a [mismatchAllocDealloc]\n"
                      "[test.c:3:14] -> [test.c:4:4]: (error) Mismatching allocation and deallocation: b [mismatchAllocDealloc]\n", errout_str());

        check("void f() {\n"
              "   void* a;\n"
              "   void* b = realloc(a, 10);\n"
              "   free(b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "   int * i = new int;\n"
              "   int * j = realloc(i, 2 * sizeof(int));\n"
              "   delete[] j;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:14] -> [test.cpp:3:14]: (error) Mismatching allocation and deallocation: i [mismatchAllocDealloc]\n"
                      "[test.cpp:3:14] -> [test.cpp:4:13]: (error) Mismatching allocation and deallocation: j [mismatchAllocDealloc]\n", errout_str());

        check("static void deleter(int* p) { free(p); }\n" // #11392
              "void f() {\n"
              "    if (int* p = static_cast<int*>(malloc(4))) {\n"
              "        std::unique_ptr<int, decltype(&deleter)> guard(p, &deleter);\n"
              "    }\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    if (int* p = static_cast<int*>(malloc(4))) {\n"
              "        std::unique_ptr<int, decltype(&deleter)> guard(p, &deleter);\n"
              "    }\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(int i) {\n"
              "    int* a = new int[i] {};\n"
              "    delete[] a;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void smartPointerDeleter() {
        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::unique_ptr<FILE> fp{f};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:3:30]: (error) Mismatching allocation and deallocation: f [mismatchAllocDealloc]\n", errout_str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::unique_ptr<FILE, decltype(&fclose)> fp{f, &fclose};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, &fclose};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("struct deleter { void operator()(FILE* f) { fclose(f); }};\n"
              "void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::unique_ptr<FILE, deleter> fp{f};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("int * create();\n"
              "void destroy(int * x);\n"
              "void f() {\n"
              "    int x * = create()\n"
              "    std::unique_ptr<int, decltype(&destroy)> xp{x, &destroy()};\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("int * create();\n"
              "void destroy(int * x);\n"
              "void f() {\n"
              "    int x * = create()\n"
              "    std::unique_ptr<int, decltype(&destroy)> xp(x, &destroy());\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, [](FILE* x) { fclose(x); }};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, +[](FILE* x) { fclose(x); }};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, [](FILE* x) { free(f); }};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:3:30]: (error) Mismatching allocation and deallocation: f [mismatchAllocDealloc]\n", errout_str());

        check("void f() {\n"
              "    FILE*f=fopen(fname,a);\n"
              "    std::shared_ptr<FILE> fp{f, [](FILE* x) {}};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:12] -> [test.cpp:3:30]: (error) Mismatching allocation and deallocation: f [mismatchAllocDealloc]\n", errout_str());

        check("class C;\n"
              "void f() {\n"
              "  C* c = new C{};\n"
              "  std::shared_ptr<C> a{c, [](C*) {}};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("class C;\n"
              "void f() {\n"
              "  C* c = new C{};\n"
              "  std::shared_ptr<C> a{c, [](C* x) { delete x; }};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("struct DirDeleter {\n" // #12544
              "    void operator()(DIR* d) { ::closedir(d); }\n"
              "};\n"
              "void openDir(DIR* dir) {\n"
              "    std::shared_ptr<DIR> dp(dir, DirDeleter());\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:2:44]: (information) --check-library: Function closedir() should have <noreturn> configuration [checkLibraryNoReturn]\n", errout_str()); // don't crash
    }
    void smartPointerRelease() {
        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int> x(i);\n"
              "    x.release();\n"
              "    delete i;\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    int * i = new int;\n"
              "    std::unique_ptr<int> x(i);\n"
              "    x.release();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:5:1]: (error) Memory leak: i [memleak]\n", errout_str());
    }

    void return1() {
        check("int f() {\n"
              "    char *p = malloc(100);\n"
              "    return 123;\n"
              "}");
        ASSERT_EQUALS("[test.c:3:5]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void return2() {
        check("char *f() {\n"
              "    char *p = malloc(100);\n"
              "    return p;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void return3() {
        check("struct dev * f() {\n"
              "    struct ABC *abc = malloc(100);\n"
              "    return &abc->dev;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void return4() { // ticket #3862
        // avoid false positives
        check("void f(char *p, int x) {\n"
              "    if (x==12) {\n"
              "        free(p);\n"
              "        throw 1;\n"
              "    }\n"
              "    free(p);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(char *p, int x) {\n"
              "    if (x==12) {\n"
              "        delete p;\n"
              "        throw 1;\n"
              "    }\n"
              "    delete p;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(char *p, int x) {\n"
              "    if (x==12) {\n"
              "        delete [] p;\n"
              "        throw 1;\n"
              "    }\n"
              "    delete [] p;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void return5() { // ticket #6397 - conditional allocation/deallocation and conditional return
        // avoid false positives
        check("void f(int *p, int x) {\n"
              "    if (x != 0) {\n"
              "        free(p);\n"
              "    }\n"
              "    if (x != 0) {\n"
              "        return;\n"
              "    }\n"
              "    *p = 0;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void return6() { // #8282
        check("std::pair<char*, char*> f(size_t n) {\n"
              "   char* p = (char* )malloc(n);\n"
              "   return {p, p};\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void return7() { // #9343
        check("uint8_t *f() {\n"
              "    void *x = malloc(1);\n"
              "    return (uint8_t *)x;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("uint8_t f() {\n"
              "    void *x = malloc(1);\n"
              "    return (uint8_t)x;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Memory leak: x [memleak]\n", errout_str());

        check("void** f() {\n"
              "    void *x = malloc(1);\n"
              "    return (void**)x;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (long long)x;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (void*)(short)x;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Memory leak: x [memleak]\n", errout_str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (mytype)x;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void* f() {\n" // Do not crash
              "    void *x = malloc(1);\n"
              "    return (mytype)y;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Memory leak: x [memleak]\n", errout_str());
    }

    void return8() {
        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (x);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return ((x));\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void* f() {\n"
              "    void *x = malloc(1);\n"
              "    return ((((x))));\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("char* f() {\n"
              "    void *x = malloc(1);\n"
              "    return (char*)(x);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void return9() {
        check("void* f() {\n"
              "    void *x = malloc (sizeof (struct alloc));\n"
              "    return x + sizeof (struct alloc);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void return10() {
        check("char f() {\n" // #11758
              "    char* p = (char*)malloc(1);\n"
              "    p[0] = 'x';\n"
              "    return p[0];\n"
              "}");
        ASSERT_EQUALS("[test.c:4:5]: (error) Memory leak: p [memleak]\n", errout_str());

        check("struct S { int f(); };\n" // #11746
              "int g() {\n"
              "    S* s = new S;\n"
              "    delete s;\n"
              "    return s->f();\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:5] -> [test.cpp:5:5]: (error) Returning/dereferencing 's' after it is deallocated / released [deallocret]\n", errout_str());

        check("int f() {\n"
              "    int* p = new int(3);\n"
              "    delete p;\n"
              "    return *p;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:5] -> [test.cpp:4:5]: (error) Returning/dereferencing 'p' after it is deallocated / released [deallocret]\n", errout_str());
    }

    void return11() { // #13098
        check("char malloc_memleak(void) {\n"
              "    bool flag = false;\n"
              "    char *ptr = malloc(10);\n"
              "    if (flag) {\n"
              "        free(ptr);\n"
              "    }\n"
              "    return 'a';\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Memory leak: ptr [memleak]\n", errout_str());

        check("char malloc_memleak(void) {\n"
              "    bool flag = false;\n"
              "    char *ptr = malloc(10);\n"
              "    if (flag) {\n"
              "        free(ptr);\n"
              "    }\n"
              "    return 'a';\n"
              "}\n");
        ASSERT_EQUALS("[test.c:7:5]: (error) Memory leak: ptr [memleak]\n", errout_str());
    }

    void return12() { // #12238
        CheckOptions options;
        options.cpp = true;
        check("void f(size_t size) {\n"
              "    void* buffer = malloc(size);\n"
              "    std::vector<void*> v{ buffer };\n"
              "    x->g(v);\n"
              "}\n", options);
        ASSERT_EQUALS("", errout_str());
    }

    void return13() { // #4638
        CheckOptions options;
        options.cpp = true;
        check("bool f() {\n"
              "    int* p = new int;\n"
              "    return p;\n"
              "}\n"
              "bool g() {\n"
              "    void* p = malloc(4);\n"
              "    return p;\n"
              "}\n", options);
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Memory leak: p [memleak]\n"
                      "[test.cpp:7:5]: (error) Memory leak: p [memleak]\n",
                      errout_str());
    }

    void test1() {
        check("void f(double*&p) {\n" // 3809
              "    p = malloc(0x100);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(int*& p) {\n" // #4400
              "    p = (int*)malloc(4);\n"
              "    p = (int*)malloc(4);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void f() {\n"
              "    int* p = (int*)malloc(4);\n"
              "    int*& r = p;\n"
              "    r = (int*)malloc(4);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: p\n", "", errout_str());

        check("void f() {\n"
              "    int* p = (int*)malloc(4);\n"
              "    int*& r = p;\n"
              "    free(r);\n"
              "    p = (int*)malloc(4);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        TODO_ASSERT_EQUALS("", "[test.cpp:6:1]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void test2() { // 3899
        check("struct Fred {\n"
              "    char *p;\n"
              "    void f1() { free(p); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void test3() { // 3954 - reference pointer
        check("void f() {\n"
              "    char *&p = x();\n"
              "    p = malloc(10);\n"
              "};", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void test4() { // 5923 - static pointer
        check("void f() {\n"
              "    static char *p;\n"
              "    if (!p) p = malloc(10);\n"
              "    if (x) { free(p); p = 0; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void test5() { // unknown type
        check("void f() { Fred *p = malloc(10); }", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:1:34]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void f() { Fred *p = malloc(10); }");
        ASSERT_EQUALS("[test.c:1:34]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void f() { Fred *p = new Fred; }", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() { Fred fred = malloc(10); }", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void throw1() { // 3987 - Execution reach a 'throw'
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    throw 123;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Memory leak: p [memleak]\n", errout_str());

        check("void f() {\n"
              "    char *p;\n"
              "    try {\n"
              "        p = malloc(10);\n"
              "        throw 123;\n"
              "    } catch (...) { }\n"
              "    free(p);\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void throw2() { // do not miss ::NS::Except()
        check("namespace NS {\n"
              "    class Except {\n"
              "    };\n"
              "}\n"
              "void foo(int i)\n"
              "{\n"
              "    int *pi = new int;\n"
              "    if (i == 42) {\n"
              "        delete pi;\n"
              "        throw ::NS::Except();\n"
              "    }\n"
              "    delete pi;\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void configuration1() {
        // Possible leak => configuration is required for complete analysis
        // The user should be able to "white list" and "black list" functions.

        // possible leak. If the function 'x' deallocates the pointer or
        // takes the address, there is no leak.
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    x(p);\n"
              "}");
        ASSERT_EQUALS("[test.c:3:9]: (information) --check-library: Function x() should have <noreturn> configuration [checkLibraryNoReturn]\n"
                      "[test.c:4:1]: (information) --check-library: Function x() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n",
                      errout_str());

        check("void cb();\n" // #11190, #11523
              "void f() {\n"
              "    cb();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void configuration2() {
        // possible leak. If the function 'x' deallocates the pointer or
        // takes the address, there is no leak.
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    x(&p);\n"
              "}");
        ASSERT_EQUALS("[test.c:3:10]: (information) --check-library: Function x() should have <noreturn> configuration [checkLibraryNoReturn]\n"
                      "[test.c:4:1]: (information) --check-library: Function x() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n",
                      errout_str());
    }

    void configuration3() {
        {
            const char code[] = "void f() {\n"
                                "    char *p = malloc(10);\n"
                                "    if (set_data(p)) { }\n"
                                "}";
            check(code);
            ASSERT_EQUALS("[test.c:4:1]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n", errout_str());
            check(code, dinit(CheckOptions, $.cpp = true));
            ASSERT_EQUALS("[test.cpp:4:1]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n", errout_str());
        }

        {
            const char code[] = "void f() {\n"
                                "    char *p = malloc(10);\n"
                                "    if (set_data(p)) { return; }\n"
                                "}";
            check(code);
            ASSERT_EQUALS("[test.c:3:24]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n"
                          "[test.c:4:1]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n"
                          , errout_str());
            check(code, dinit(CheckOptions, $.cpp = true));
            ASSERT_EQUALS("[test.cpp:3:24]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n"
                          "[test.cpp:4:1]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n"
                          , errout_str());
        }
    }

    void configuration4() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    int ret = set_data(p);\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("[test.c:4:5]: (information) --check-library: Function set_data() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n", errout_str());
    }

    void configuration5() {
        check("void f() {\n"
              "    int(i);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    static_assert(1 == sizeof(char), \"test\");\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("namespace pal {\n" // #11237
              "    struct AutoTimer {};\n"
              "}\n"
              "int main() {\n"
              "    pal::AutoTimer();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("struct AutoTimer {};\n"
              "int main() {\n"
              "    AutoTimer();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #8666
              "    asm(\"assembler code\");\n"
              "    asm volatile(\"assembler code\");\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #11239
              "    asm goto(\"assembler code\");\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    FILE* p = fopen(\"abc.txt\", \"r\");\n"
              "    if (fclose(p) != 0) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S;\n"
              "void f(int a, int b, S*& p) {\n"
              "    if (a == -1) {\n"
              "        FILE* file = fopen(\"abc.txt\", \"r\");\n"
              "    }\n"
              "    if (b) {\n"
              "        void* buf = malloc(10);\n"
              "        p = reinterpret_cast<S*>(buf);\n"
              "    }\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:5:5]: (error) Resource leak: file [resourceLeak]\n", errout_str());
    }

    void configuration6() {
        check("void f() {}\n" // #11198
              "void g() {\n"
              "    f();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::function<void()> cb) {\n" // #11189
              "    cb();\n"
              "}\n"
              "void g(void (*cb)()) {\n"
              "    cb();\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("", errout_str());
    }

    void ptrptr() {
        check("void f() {\n"
              "    char **p = malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.c:3:1]: (error) Memory leak: p [memleak]\n", errout_str());
    }

    void nestedAllocation() {
        check("void QueueDSMCCPacket(unsigned char *data, int length) {\n"
              "    unsigned char *dataCopy = malloc(length * sizeof(unsigned char));\n"
              "    m_dsmccQueue.enqueue(new DSMCCPacket(dataCopy));\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:1]: (information) --check-library: Function DSMCCPacket() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n", errout_str());

        check("void QueueDSMCCPacket(unsigned char *data, int length) {\n"
              "    unsigned char *dataCopy = malloc(length * sizeof(unsigned char));\n"
              "    m_dsmccQueue.enqueue(new DSMCCPacket(somethingunrelated));\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:1]: (error) Memory leak: dataCopy [memleak]\n", errout_str());

        check("void f() {\n"
              "  char *buf = new char[1000];\n"
              "  clist.push_back(new (std::nothrow) C(buf));\n"
              "}", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:1]: (information) --check-library: Function C() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n", errout_str());
    }

    void testKeywords() {
        check("int main(int argc, char **argv) {\n"
              "  double *new = malloc(1*sizeof(double));\n"
              "  free(new);\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void inlineFunction() {
        check("int test() {\n"
              "  char *c;\n"
              "  int ret() {\n"
              "        free(c);\n"
              "        return 0;\n"
              "    }\n"
              "    c = malloc(128);\n"
              "    return ret();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // #8262
    void smartPtrInContainer() {
        check("std::list< std::shared_ptr<int> > mList;\n"
              "void test(){\n"
              "  int *pt = new int(1);\n"
              "  mList.push_back(std::shared_ptr<int>(pt));\n"
              "}\n",
              dinit(CheckOptions, $.cpp = true)
              );
        ASSERT_EQUALS("", errout_str());
    }

    void functionCallCastConfig() { // #9652
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def format=\"2\">\n"
                                   "  <function name=\"free_func\">\n"
                                   "    <noreturn>false</noreturn>\n"
                                   "    <arg nr=\"1\">\n"
                                   "      <not-uninit/>\n"
                                   "    </arg>\n"
                                   "    <arg nr=\"2\">\n"
                                   "      <not-uninit/>\n"
                                   "    </arg>\n"
                                   "  </function>\n"
                                   "</def>";
        const Settings settingsFunctionCall = settingsBuilder(settings).libraryxml(xmldata).build();

        check("void test_func()\n"
              "{\n"
              "    char * buf = malloc(4);\n"
              "    free_func((void *)(1), buf);\n"
              "}", settingsFunctionCall);
        ASSERT_EQUALS("[test.cpp:5:1]: (information) --check-library: Function free_func() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n", errout_str());

        check("void g(void*);\n"
              "void h(int, void*);\n"
              "void f1() {\n"
              "    int* p = new int;\n"
              "    g(static_cast<void*>(p));\n"
              "}\n"
              "void f2() {\n"
              "    int* p = new int;\n"
              "    h(1, static_cast<void*>(p));\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:6:1]: (information) --check-library: Function g() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n"
                      "[test.cpp:10:1]: (information) --check-library: Function h() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n",
                      errout_str());
    }

    void functionCallLeakIgnoreConfig() { // #7923
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def format=\"2\">\n"
                                   "  <function name=\"SomeClass::someMethod\">\n"
                                   "    <leak-ignore/>\n"
                                   "    <noreturn>false</noreturn>\n"
                                   "    <arg nr=\"1\" direction=\"in\"/>\n"
                                   "  </function>\n"
                                   "</def>\n";
        const Settings settingsLeakIgnore = settingsBuilder().libraryxml(xmldata).checkLibrary().build();
        check("void f() {\n"
              "    double* a = new double[1024];\n"
              "    SomeClass::someMethod(a);\n"
              "}\n", settingsLeakIgnore);
        ASSERT_EQUALS("[test.cpp:4:1]: (error) Memory leak: a [memleak]\n", errout_str());

        check("void bar(int* p) {\n"
              "    if (p)\n"
              "        free(p);\n"
              "}\n"
              "void f() {\n"
              "    int* p = malloc(4);\n"
              "    bar(p);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(int n) {\n"
              "    char* p = new char[n];\n"
              "    v.push_back(p);\n"
              "}\n", dinit(CheckOptions, $.cpp = true));
        ASSERT_EQUALS("[test.cpp:4:1]: (information) --check-library: Function unknown::push_back() should have <use>/<leak-ignore> configuration [checkLibraryUseIgnore]\n", errout_str());
    }
};

REGISTER_TEST(TestLeakAutoVar)

class TestLeakAutoVarRecursiveCountLimit : public TestFixture {
public:
    TestLeakAutoVarRecursiveCountLimit() : TestFixture("TestLeakAutoVarRecursiveCountLimit") {}

private:
    const Settings settings = settingsBuilder().library("std.cfg").checkLibrary().build();

#define checkP(...) checkP_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkP_(const char* file, int line, const char (&code)[size]) {
        SimpleTokenizer2 tokenizer(settings, *this, code, "test.c");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        // Check for leaks..
        runChecks<CheckLeakAutoVar>(tokenizer, this);
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(recursiveCountLimit); // #5872 #6157 #9097
    }

    void recursiveCountLimit() { // #5872 #6157 #9097
        ASSERT_THROW_INTERNAL_EQUALS(checkP("#define ONE     else if (0) { }\n"
                                            "#define TEN     ONE ONE ONE ONE ONE ONE ONE ONE ONE ONE\n"
                                            "#define HUN     TEN TEN TEN TEN TEN TEN TEN TEN TEN TEN\n"
                                            "#define THOU    HUN HUN HUN HUN HUN HUN HUN HUN HUN HUN\n"
                                            "void foo() {\n"
                                            "  if (0) { }\n"
                                            "  THOU THOU\n"
                                            "}"), LIMIT, "Internal limit: CheckLeakAutoVar::checkScope() Maximum recursive count of 1000 reached.");
        ASSERT_NO_THROW(checkP("#define ONE     if (0) { }\n"
                               "#define TEN     ONE ONE ONE ONE ONE ONE ONE ONE ONE ONE\n"
                               "#define HUN     TEN TEN TEN TEN TEN TEN TEN TEN TEN TEN\n"
                               "#define THOU    HUN HUN HUN HUN HUN HUN HUN HUN HUN HUN\n"
                               "void foo() {\n"
                               "  if (0) { }\n"
                               "  THOU THOU\n"
                               "}"));
    }
};

#if !defined(__MINGW32__) && !defined(__CYGWIN__)
// TODO: this crashes with a stack overflow for MinGW and error 2816 for Cygwinin the CI
REGISTER_TEST(TestLeakAutoVarRecursiveCountLimit)
#endif

class TestLeakAutoVarStrcpy : public TestFixture {
public:
    TestLeakAutoVarStrcpy() : TestFixture("TestLeakAutoVarStrcpy") {}

private:
    const Settings settings = settingsBuilder().library("std.cfg").checkLibrary().build();

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for leaks..
        runChecks<CheckLeakAutoVar>(tokenizer, this);
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(returnedValue); // #9298
        TEST_CASE(deallocuse2);
        TEST_CASE(fclose_false_positive); // #9575
        TEST_CASE(strcpy_false_negative);
        TEST_CASE(doubleFree);
        TEST_CASE(memleak_std_string);
    }

    void returnedValue() { // #9298
        check("char *m;\n"
              "void strcpy_returnedvalue(const char* str)\n"
              "{\n"
              "    char* ptr = new char[strlen(str)+1];\n"
              "    m = strcpy(ptr, str);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void deallocuse2() {
        check("void f(char *p) {\n"
              "    free(p);\n"
              "    strcpy(a, p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n", errout_str());

        check("void f(char *p, const char *q) {\n" // #11665
              "    free(p);\n"
              "    strcpy(p, q);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12]: (error) Dereferencing 'p' after it is deallocated / released [deallocuse]\n",
                      errout_str());

        check("void f(char *p) {\n"   // #3041 - assigning pointer when it's used
              "    free(p);\n"
              "    strcpy(a, p=b());\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void fclose_false_positive() { // #9575
        check("int  f(FILE *fp) { return fclose(fp); }");
        ASSERT_EQUALS("", errout_str());
    }

    void strcpy_false_negative() { // #12289
        check("void f() {\n"
              "    char* buf = new char[12];\n"
              "    strcpy(buf, \"123\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:1]: (error) Memory leak: buf [memleak]\n", errout_str());
    }

    void doubleFree() {
        check("void f(char* p) {\n"
              "    free(p);\n"
              "    printf(\"%s\", p = strdup(\"abc\"));\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void memleak_std_string() {
        check("struct S {\n" // #12354
              "    std::string s;\n"
              "    void f();\n"
              "};\n"
              "void f(S* p, bool b) {\n"
              "    if (b)\n"
              "        p = new S();\n"
              "    p->f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:1]: (error) Memory leak: p [memleak]\n", errout_str());

        check("class B { std::string s; };\n" // #12062
              "class D : public B {};\n"
              "void g() {\n"
              "    auto d = new D();\n"
              "    if (d) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:1]: (error) Memory leak: d [memleak]\n", errout_str());
    }
};

REGISTER_TEST(TestLeakAutoVarStrcpy)


class TestLeakAutoVarWindows : public TestFixture {
public:
    TestLeakAutoVarWindows() : TestFixture("TestLeakAutoVarWindows") {}

private:
    const Settings settings = settingsBuilder().library("windows.cfg").build();

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this, false);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for leaks..
        runChecks<CheckLeakAutoVar>(tokenizer, this);
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(heapDoubleFree);
    }

    void heapDoubleFree() {
        check("void f() {\n"
              "  HANDLE MyHeap = HeapCreate(0, 0, 0);\n"
              "  int *a = HeapAlloc(MyHeap, 0, sizeof(int));\n"
              "  int *b = HeapAlloc(MyHeap, 0, sizeof(int));\n"
              "  HeapFree(MyHeap, 0, a);\n"
              "  HeapFree(MyHeap, 0, b);\n"
              "  HeapDestroy(MyHeap);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  int *a = HeapAlloc(GetProcessHeap(), 0, sizeof(int))\n"
              "  int *b = HeapAlloc(GetProcessHeap(), 0, sizeof(int));\n"
              "  HeapFree(GetProcessHeap(), 0, a);\n"
              "  HeapFree(GetProcessHeap(), 0, b);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  HANDLE MyHeap = HeapCreate(0, 0, 0);\n"
              "  int *a = HeapAlloc(MyHeap, 0, sizeof(int));\n"
              "  int *b = HeapAlloc(MyHeap, 0, sizeof(int));\n"
              "  HeapFree(MyHeap, 0, a);\n"
              "  HeapDestroy(MyHeap);\n"
              "}\n");
        ASSERT_EQUALS("[test.c:7:1]: (error) Memory leak: b [memleak]\n", errout_str());

        check("void f() {\n"
              "  HANDLE MyHeap = HeapCreate(0, 0, 0);\n"
              "  int *a = HeapAlloc(MyHeap, 0, sizeof(int));\n"
              "  int *b = HeapAlloc(MyHeap, 0, sizeof(int));\n"
              "  HeapFree(MyHeap, 0, a);\n"
              "  HeapFree(MyHeap, 0, b);\n"
              "}\n");
        ASSERT_EQUALS("[test.c:7:1]: (error) Resource leak: MyHeap [resourceLeak]\n", errout_str());

        check("void f() {\n"
              "  HANDLE MyHeap = HeapCreate(0, 0, 0);\n"
              "  int *a = HeapAlloc(MyHeap, 0, sizeof(int));\n"
              "  int *b = HeapAlloc(MyHeap, 0, sizeof(int));\n"
              "  HeapFree(MyHeap, 0, a);\n"
              "}\n");
        ASSERT_EQUALS("[test.c:6:1]: (error) Resource leak: MyHeap [resourceLeak]\n"
                      "[test.c:6:1]: (error) Memory leak: b [memleak]\n",
                      errout_str());
    }
};

REGISTER_TEST(TestLeakAutoVarWindows)

class TestLeakAutoVarPosix : public TestFixture {
public:
    TestLeakAutoVarPosix() : TestFixture("TestLeakAutoVarPosix") {}

private:
    const Settings settings = settingsBuilder().library("std.cfg").library("posix.cfg").build();

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for leaks..
        runChecks<CheckLeakAutoVar>(tokenizer, this);
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(memleak_getline);
        TEST_CASE(deallocuse_fdopen);
        TEST_CASE(doublefree_fdopen); // #12781
        TEST_CASE(memleak_open); // #13356
    }

    void memleak_getline() {
        check("void f(std::ifstream &is) {\n" // #12297
              "    std::string str;\n"
              "    if (getline(is, str, 'x').good()) {};\n"
              "    if (!getline(is, str, 'x').good()) {};\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void deallocuse_fdopen() {
        check("struct FileCloser {\n" // #13126
              "    void operator()(std::FILE * ptr) const {\n"
              "        std::fclose(ptr);\n"
              "    }\n"
              "};\n"
              "void f(char* fileName) {\n"
              "    std::unique_ptr<std::FILE, FileCloser> out;\n"
              "    int fd = mkstemps(fileName, 4);\n"
              "    if (fd != -1)\n"
              "        out.reset(fdopen(fd, \"w\"));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void doublefree_fdopen() { // #12781
        check("void foo(void) {\n"
              "    int fd;\n"
              "    FILE *stream;\n"
              "    fd = open(\"/foo\", O_RDONLY);\n"
              "    if (fd == -1) return;\n"
              "    stream = fdopen(fd, \"r\");\n"
              "    if (!stream) {\n"
              "        close(fd);\n"
              "        return;\n"
              "    }\n"
              "    fclose(stream);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void memleak_open() { // #13356
        check("int f() {\n"
              "    int fd = open(\"abc \", O_RDONLY);\n"
              "    if (fd > -1) {\n"
              "        return fd;\n"
              "    }\n"
              "    return -1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestLeakAutoVarPosix)
