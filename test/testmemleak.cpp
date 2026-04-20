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

#include "checkmemoryleak.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"

#include <cstddef>
#include <list>

class TestMemleak : public TestFixture {
public:
    TestMemleak() : TestFixture("TestMemleak") {}

private:
    void run() override {
        TEST_CASE(testFunctionReturnType);
        TEST_CASE(open);
    }

#define functionReturnType(...) functionReturnType_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    CheckMemoryLeak::AllocType functionReturnType_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        const CheckMemoryLeak c(&tokenizer, this, &settingsDefault);

        return (c.functionReturnType)(&tokenizer.getSymbolDatabase()->scopeList.front().functionList.front());
    }

    void testFunctionReturnType() {
        {
            const char code[] = "const char *foo()\n"
                                "{ return 0; }";
            ASSERT_EQUALS(CheckMemoryLeak::No, functionReturnType(code));
        }

        {
            const char code[] = "Fred *newFred()\n"
                                "{ return new Fred; }";
            ASSERT_EQUALS(CheckMemoryLeak::New, functionReturnType(code));
        }

        {
            const char code[] = "char *foo()\n"
                                "{ return new char[100]; }";
            ASSERT_EQUALS(CheckMemoryLeak::NewArray, functionReturnType(code));
        }

        {
            const char code[] = "char *foo()\n"
                                "{\n"
                                "    char *p = new char[100];\n"
                                "    return p;\n"
                                "}";
            ASSERT_EQUALS(CheckMemoryLeak::NewArray, functionReturnType(code));
        }
    }

    void open() {
        const char code[] = "class A {\n"
                            "  static int open() {\n"
                            "    return 1;\n"
                            "  }\n"
                            "\n"
                            "  A() {\n"
                            "    int ret = open();\n"
                            "  }\n"
                            "};\n";

        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        // there is no allocation
        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "ret =");
        const CheckMemoryLeak check(&tokenizer, nullptr, &settingsDefault);
        ASSERT_EQUALS(CheckMemoryLeak::No, check.getAllocationType(tok->tokAt(2), 1));
    }
};

REGISTER_TEST(TestMemleak)





class TestMemleakInFunction : public TestFixture {
public:
    TestMemleakInFunction() : TestFixture("TestMemleakInFunction") {}

private:
    const Settings settings = settingsBuilder().library("std.cfg").library("posix.cfg").build();

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for memory leaks..
        CheckMemoryLeakInFunction checkMemoryLeak(&tokenizer, &settings, this);
        checkMemoryLeak.checkReallocUsage();
    }


    void run() override {
        mNewTemplate = true;
        TEST_CASE(realloc1);
        TEST_CASE(realloc2);
        TEST_CASE(realloc3);
        TEST_CASE(realloc4);
        TEST_CASE(realloc5);
        TEST_CASE(realloc7);
        TEST_CASE(realloc8);
        TEST_CASE(realloc9);
        TEST_CASE(realloc10);
        TEST_CASE(realloc11);
        TEST_CASE(realloc12);
        TEST_CASE(realloc13);
        TEST_CASE(realloc14);
        TEST_CASE(realloc15);
        TEST_CASE(realloc16);
        TEST_CASE(realloc17);
        TEST_CASE(realloc18);
        TEST_CASE(realloc19);
        TEST_CASE(realloc20);
        TEST_CASE(realloc21);
        TEST_CASE(realloc22);
        TEST_CASE(realloc23);
        TEST_CASE(realloc24); // #9228
    }

    void realloc1() {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = realloc(a, 100);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure [memleakOnRealloc]\n", errout_str());
    }

    void realloc2() {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = (char *)realloc(a, 100);\n"
              "    free(a);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4:5]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure [memleakOnRealloc]\n", errout_str());
    }

    void realloc3() {
        check("void foo()\n"
              "{\n"
              "    char *a = 0;\n"
              "    if ((a = realloc(a, 100)) == NULL)\n"
              "        return;\n"
              "    free(a);\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void realloc4() {
        check("void foo()\n"
              "{\n"
              "    static char *a = 0;\n"
              "    if ((a = realloc(a, 100)) == NULL)\n"
              "        return;\n"
              "    free(a);\n"
              "}");

        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: a\n",
                           "[test.cpp:4:10]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure [memleakOnRealloc]\n",
                           errout_str());
    }

    void realloc5() {
        check("void foo()\n"
              "{\n"
              "    char *buf;\n"
              "    char *new_buf;\n"
              "    buf = calloc( 10 );\n"
              "    new_buf = realloc ( buf, 20);\n"
              "    if ( !new_buf )\n"
              "        free(buf);\n"
              "    else\n"
              "        free(new_buf);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc7() {
        check("bool foo(size_t nLen, char* pData)\n"
              "{\n"
              "    pData = (char*) realloc(pData, sizeof(char) + (nLen + 1)*sizeof(char));\n"
              "    if ( pData == NULL )\n"
              "    {\n"
              "        return false;\n"
              "    }\n"
              "    free(pData);\n"
              "    return true;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc8() {
        check("void foo()\n"
              "{\n"
              "    char *origBuf = m_buf;\n"
              "    m_buf = (char *) realloc (m_buf, m_capacity + growBy);\n"
              "    if (!m_buf) {\n"
              "        m_buf = origBuf;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc9() {
        check("void foo()\n"
              "{\n"
              "    x = realloc(x,100);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc10() {
        check("void foo() {\n"
              "    char *pa, *pb;\n"
              "    pa = pb = malloc(10);\n"
              "    pa = realloc(pa, 20);"
              "    exit();\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc11() {
        check("void foo() {\n"
              "    char *p;\n"
              "    p = realloc(p, size);\n"
              "    if (!p)\n"
              "        error();\n"
              "    usep(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc12() {
        check("void foo(int x)\n"
              "{\n"
              "    char *a = 0;\n"
              "    if ((a = realloc(a, x + 100)) == NULL)\n"
              "        return;\n"
              "    free(a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc13() {
        check("void foo()\n"
              "{\n"
              "    char **str;\n"
              "    *str = realloc(*str,100);\n"
              "    free (*str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Common realloc mistake: \'str\' nulled but not freed upon failure [memleakOnRealloc]\n", errout_str());
    }

    void realloc14() {
        check("void foo() {\n"
              "    char *p;\n"
              "    p = realloc(p, size + 1);\n"
              "    if (!p)\n"
              "        error();\n"
              "    usep(p);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc15() {
        check("bool foo() {\n"
              "    char ** m_options;\n"
              "    m_options = (char**)realloc( m_options, 2 * sizeof(char*));\n"
              "    if( m_options == NULL )\n"
              "        return false;\n"
              "    return true;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Common realloc mistake: \'m_options\' nulled but not freed upon failure [memleakOnRealloc]\n", errout_str());
    }

    void realloc16() {
        check("void f(char *zLine) {\n"
              "  zLine = realloc(zLine, 42);\n"
              "  if (zLine) {\n"
              "    free(zLine);\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc17() {
        check("void foo()\n"
              "{\n"
              "    void ***a = malloc(sizeof(a));\n"
              "    ***a = realloc(***(a), sizeof(a) * 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:8]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure [memleakOnRealloc]\n", errout_str());
    }

    void realloc18() {
        check("void foo()\n"
              "{\n"
              "    void *a = malloc(sizeof(a));\n"
              "    a = realloc((void*)a, sizeof(a) * 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure [memleakOnRealloc]\n", errout_str());
    }

    void realloc19() {
        check("void foo()\n"
              "{\n"
              "    void *a = malloc(sizeof(a));\n"
              "    a = (realloc((void*)((a)), sizeof(a) * 2));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure [memleakOnRealloc]\n", errout_str());
    }

    void realloc20() {
        check("void foo()\n"
              "{\n"
              "    void *a = malloc(sizeof(a));\n"
              "    a = realloc((a) + 1, sizeof(a) * 2);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc21() {
        check("char *foo(char *bs0)\n"
              "{\n"
              "    char *bs = bs0;\n"
              "    bs = realloc(bs, 100);\n"
              "    if (bs == NULL) return bs0;\n"
              "    return bs;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc22() {
        check("void foo(char **bsp)\n"
              "{\n"
              "    char *bs = *bsp;\n"
              "    bs = realloc(bs, 100);\n"
              "    if (bs == NULL) return;\n"
              "    *bsp = bs;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc23() {
        check("void foo(struct ABC *s)\n"
              "{\n"
              "    uint32_t *cigar = s->cigar;\n"
              "    if (!(cigar = realloc(cigar, 100 * sizeof(*cigar))))\n"
              "        return;\n"
              "    s->cigar = cigar;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void realloc24() { // #9228
        check("void f() {\n"
              "void *a = NULL;\n"
              "a = realloc(a, 20);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "void *a = NULL;\n"
              "a = malloc(10);\n"
              "a = realloc(a, 20);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:1]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure [memleakOnRealloc]\n", errout_str());

        check("void f() {\n"
              "void *a = nullptr;\n"
              "a = malloc(10);\n"
              "a = realloc(a, 20);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:1]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure [memleakOnRealloc]\n", errout_str());

        check("void f(char *b) {\n"
              "void *a = NULL;\n"
              "a = b;\n"
              "a = realloc(a, 20);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestMemleakInFunction)








class TestMemleakInClass : public TestFixture {
public:
    TestMemleakInClass() : TestFixture("TestMemleakInClass") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::warning).severity(Severity::style).library("std.cfg").build();

    /**
     * Tokenize and execute leak check for given code
     * @param code Source code
     */
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for memory leaks..
        CheckMemoryLeakInClass checkMemoryLeak(&tokenizer, &settings, this);
        (checkMemoryLeak.check)();
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(class1);
        TEST_CASE(class2);
        TEST_CASE(class3);
        TEST_CASE(class4);
        TEST_CASE(class6);
        TEST_CASE(class7);
        TEST_CASE(class8);
        TEST_CASE(class9);
        TEST_CASE(class10);
        TEST_CASE(class11);
        TEST_CASE(class12);
        TEST_CASE(class13);
        TEST_CASE(class14);
        TEST_CASE(class15);
        TEST_CASE(class16);
        TEST_CASE(class17);
        TEST_CASE(class18);
        TEST_CASE(class19); // ticket #2219
        TEST_CASE(class20);
        TEST_CASE(class21); // ticket #2517
        TEST_CASE(class22); // ticket #3012
        TEST_CASE(class23); // ticket #3303
        TEST_CASE(class24); // ticket #3806 - false positive in copy constructor
        TEST_CASE(class25); // ticket #4367 - false positive implementation for destructor is not seen
        TEST_CASE(class26); // ticket #10789
        TEST_CASE(class27); // ticket #8126

        TEST_CASE(staticvar);

        TEST_CASE(free_member_in_sub_func);

        TEST_CASE(mismatch1);
        TEST_CASE(mismatch2); // #5659

        // allocating member variable in public function
        TEST_CASE(func1);
        TEST_CASE(func2);
    }


    void class1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "    char *str2;\n"
              "public:\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "    char *str2;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "        str2 = new char[10];\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        delete [] str2;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());
    }

    void class2() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "public:\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "}\n"
              "\n"
              "Fred::~Fred()\n"
              "{\n"
              "    free(str1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:17:5]: (error) Mismatching allocation and deallocation: Fred::str1 [mismatchAllocDealloc]\n", errout_str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        free(str1);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:12:9]: (error) Mismatching allocation and deallocation: Fred::str1 [mismatchAllocDealloc]\n", errout_str());
    }

    void class3() {
        check("class Token;\n"
              "\n"
              "class Tokenizer\n"
              "{\n"
              "private:\n"
              "    Token *_tokens;\n"
              "\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "    void deleteTokens(Token *tok);\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "    _tokens = new Token;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "}\n"
              "\n"
              "void Tokenizer::deleteTokens(Token *tok)\n"
              "{\n"
              "    while (tok)\n"
              "    {\n"
              "        Token *next = tok->next();\n"
              "        delete tok;\n"
              "        tok = next;\n"
              "    }\n"
              "}");

        ASSERT_EQUALS("", errout_str());

        check("class Token;\n"
              "\n"
              "class Tokenizer\n"
              "{\n"
              "private:\n"
              "    Token *_tokens;\n"
              "\n"
              "public:\n"
              "    Tokenizer()\n"
              "    {\n"
              "        _tokens = new Token;\n"
              "    }\n"
              "    ~Tokenizer()\n"
              "    {\n"
              "        deleteTokens(_tokens);\n"
              "    }\n"
              "    void deleteTokens(Token *tok)\n"
              "    {\n"
              "        while (tok)\n"
              "        {\n"
              "            Token *next = tok->next();\n"
              "            delete tok;\n"
              "            tok = next;\n"
              "        }\n"
              "    }\n"
              "};");

        ASSERT_EQUALS("", errout_str());
    }

    void class4() {
        check("struct ABC;\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    void addAbc(ABC *abc);\n"
              "public:\n"
              "    void click();\n"
              "};\n"
              "\n"
              "void Fred::addAbc(ABC* abc)\n"
              "{\n"
              "    AbcPosts->Add(abc);\n"
              "}\n"
              "\n"
              "void Fred::click()\n"
              "{\n"
              "    ABC *p = new ABC;\n"
              "    addAbc( p );\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct ABC;\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    void addAbc(ABC* abc)\n"
              "    {\n"
              "        AbcPosts->Add(abc);\n"
              "    }\n"
              "public:\n"
              "    void click()\n"
              "    {\n"
              "        ABC *p = new ABC;\n"
              "        addAbc( p );\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class6() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void Fred::foo()\n"
              "{\n"
              "    char *str = new char[100];\n"
              "    delete [] str;\n"
              "    hello();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void foo()\n"
              "    {\n"
              "        char *str = new char[100];\n"
              "        delete [] str;\n"
              "        hello();\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class7() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int *i;\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    this->i = new int;\n"
              "}\n"
              "Fred::~Fred()\n"
              "{\n"
              "    delete this->i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int *i;\n"
              "    Fred()\n"
              "    {\n"
              "        this->i = new int;\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        delete this->i;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class8() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    void a();\n"
              "    void doNothing() { }\n"
              "};\n"
              "\n"
              "void A::a()\n"
              "{\n"
              "    int* c = new int(1);\n"
              "    delete c;\n"
              "    doNothing(c);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    void a()\n"
              "    {\n"
              "        int* c = new int(1);\n"
              "        delete c;\n"
              "        doNothing(c);\n"
              "    }\n"
              "    void doNothing() { }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class9() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "    ~A();\n"
              "};\n"
              "\n"
              "A::A()\n"
              "{ p = new int; }\n"
              "\n"
              "A::~A()\n"
              "{ delete (p); }");
        ASSERT_EQUALS("", errout_str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A()\n"
              "    { p = new int; }\n"
              "    ~A()\n"
              "    { delete (p); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class10() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "};\n"
              "A::A()\n"
              "{ p = new int; }");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A() { p = new int; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());
    }

    void class11() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A() : p(new int[10])\n"
              "    { }"
              "};");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "};\n"
              "A::A() : p(new int[10])\n"
              "{ }");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());
    }

    void class12() {
        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A();\n"
              "    void cleanup();"
              "};\n"
              "\n"
              "A::A()\n"
              "{ p = new int[10]; }\n"
              "\n"
              "A::~A()\n"
              "{ }\n"
              "\n"
              "void A::cleanup()\n"
              "{ delete [] p; }");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = new int[10]; }\n"
              "    ~A()\n"
              "    { }\n"
              "    void cleanup()\n"
              "    { delete [] p; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4:10]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());
    }

    void class13() {
        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A();\n"
              "    void foo();"
              "};\n"
              "\n"
              "A::A()\n"
              "{ }\n"
              "\n"
              "A::~A()\n"
              "{ }\n"
              "\n"
              "void A::foo()\n"
              "{ p = new int[10]; delete [] p; }");
        ASSERT_EQUALS("[test.cpp:17:3]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated. [publicAllocationError]\n", errout_str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { }\n"
              "    ~A()\n"
              "    { }\n"
              "    void foo()\n"
              "    { p = new int[10]; delete [] p; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:11:7]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated. [publicAllocationError]\n", errout_str());
    }

    void class14() {
        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init();\n"
              "};\n"
              "\n"
              "void A::init()\n"
              "{ p = new int[10]; }");
        ASSERT_EQUALS("[test.cpp:9:3]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated. [publicAllocationError]\n"
                      "[test.cpp:3:10]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init()\n"
              "    { p = new int[10]; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:7]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated. [publicAllocationError]\n"
                      "[test.cpp:3:10]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init();\n"
              "};\n"
              "\n"
              "void A::init()\n"
              "{ p = new int; }");
        ASSERT_EQUALS("[test.cpp:9:3]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated. [publicAllocationError]\n"
                      "[test.cpp:3:10]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init()\n"
              "    { p = new int; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:7]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated. [publicAllocationError]\n"
                      "[test.cpp:3:10]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init();\n"
              "};\n"
              "\n"
              "void A::init()\n"
              "{ p = malloc(sizeof(int)*10); }");
        ASSERT_EQUALS("[test.cpp:9:3]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated. [publicAllocationError]\n"
                      "[test.cpp:3:10]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init()\n"
              "    { p = malloc(sizeof(int)*10); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6:7]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated. [publicAllocationError]\n"
                      "[test.cpp:3:10]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());
    }

    void class15() {
        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { delete [] p; }\n"
              "};\n"
              "A::A()\n"
              "{ p = new int[10]; }");
        ASSERT_EQUALS("", errout_str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = new int[10]; }\n"
              "    ~A() { delete [] p; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { delete p; }\n"
              "};\n"
              "A::A()\n"
              "{ p = new int; }");
        ASSERT_EQUALS("", errout_str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = new int; }\n"
              "    ~A() { delete p; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { free(p); }\n"
              "};\n"
              "A::A()\n"
              "{ p = malloc(sizeof(int)*10); }");
        ASSERT_EQUALS("", errout_str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = malloc(sizeof(int)*10); }\n"
              "    ~A() { free(p); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class16() {
        // Ticket #1510
        check("class A\n"
              "{\n"
              "    int *a;\n"
              "    int *b;\n"
              "public:\n"
              "    A() { a = b = new int[10]; }\n"
              "    ~A() { delete [] a; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class17() {
        // Ticket #1557
        check("class A {\n"
              "private:\n"
              "    char *pd;\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void A::foo()\n"
              "{\n"
              "    A::pd = new char[12];\n"
              "    delete [] A::pd;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:5]: (warning) Possible leak in public function. The pointer 'pd' is not deallocated before it is allocated. [publicAllocationError]\n", errout_str());

        check("class A {\n"
              "private:\n"
              "    char *pd;\n"
              "public:\n"
              "    void foo()\n"
              "    {\n"
              "        pd = new char[12];\n"
              "        delete [] pd;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7:9]: (warning) Possible leak in public function. The pointer 'pd' is not deallocated before it is allocated. [publicAllocationError]\n", errout_str());

        check("class A {\n"
              "private:\n"
              "    char *pd;\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void A::foo()\n"
              "{\n"
              "    pd = new char[12];\n"
              "    delete [] pd;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:5]: (warning) Possible leak in public function. The pointer 'pd' is not deallocated before it is allocated. [publicAllocationError]\n", errout_str());
    }

    void class18() {
        // Ticket #853
        check("class  A : public x\n"
              "{\n"
              "public:\n"
              "  A()\n"
              "  {\n"
              "    a = new char[10];\n"
              "    foo(a);\n"
              "  }\n"
              "private:\n"
              "  char *a;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("class  A : public x\n"
              "{\n"
              "public:\n"
              "  A();\n"
              "private:\n"
              "  char *a;\n"
              "};\n"
              "A::A()\n"
              "{\n"
              "  a = new char[10];\n"
              "  foo(a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void class19() {
        // Ticket #2219
        check("class Foo\n"
              "{\n"
              "private:\n"
              "    TRadioButton* rp1;\n"
              "    TRadioButton* rp2;\n"
              "public:\n"
              "    Foo();\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "    rp1 = new TRadioButton(this);\n"
              "    rp2 = new TRadioButton(this);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("class TRadioButton { };\n"
              "class Foo\n"
              "{\n"
              "private:\n"
              "    TRadioButton* rp1;\n"
              "    TRadioButton* rp2;\n"
              "public:\n"
              "    Foo();\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "    rp1 = new TRadioButton;\n"
              "    rp2 = new TRadioButton;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:19]: (style) Class 'Foo' is unsafe, 'Foo::rp1' can leak by wrong usage. [unsafeClassCanLeak]\n"
                      "[test.cpp:6:19]: (style) Class 'Foo' is unsafe, 'Foo::rp2' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("class TRadioButton { };\n"
              "class Foo\n"
              "{\n"
              "private:\n"
              "    TRadioButton* rp1;\n"
              "    TRadioButton* rp2;\n"
              "public:\n"
              "    Foo();\n"
              "    ~Foo();\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "    rp1 = new TRadioButton;\n"
              "    rp2 = new TRadioButton;\n"
              "}\n"
              "Foo::~Foo()\n"
              "{\n"
              "    delete rp1;\n"
              "    delete rp2;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void class20() {
        check("namespace ns1 {\n"
              "    class Fred\n"
              "    {\n"
              "    private:\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    public:\n"
              "        Fred()\n"
              "        {\n"
              "            str1 = new char[10];\n"
              "            str2 = new char[10];\n"
              "        }\n"
              "        ~Fred()\n"
              "        {\n"
              "            delete [] str2;\n"
              "        }\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:15]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("namespace ns1 {\n"
              "    class Fred\n"
              "    {\n"
              "    private:\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    public:\n"
              "        Fred();\n"
              "        ~Fred();\n"
              "    };\n"
              "\n"
              "    Fred::Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "        str2 = new char[10];\n"
              "    }\n"
              "\n"
              "    Fred::~Fred()\n"
              "    {\n"
              "        delete [] str2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:15]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("namespace ns1 {\n"
              "    class Fred\n"
              "    {\n"
              "    private:\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    public:\n"
              "        Fred();\n"
              "        ~Fred();\n"
              "    };\n"
              "}\n"
              "ns1::Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "ns1::Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:15]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("namespace ns1 {\n"
              "    namespace ns2 {\n"
              "        class Fred\n"
              "        {\n"
              "        private:\n"
              "            char *str1;\n"
              "            char *str2;\n"
              "        public:\n"
              "            Fred();\n"
              "            ~Fred();\n"
              "        };\n"
              "    }\n"
              "}\n"
              "ns1::ns2::Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "ns1::ns2::Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:19]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());

        check("namespace ns1 {\n"
              "    namespace ns2 {\n"
              "        namespace ns3 {\n"
              "            class Fred\n"
              "            {\n"
              "            private:\n"
              "                char *str1;\n"
              "                char *str2;\n"
              "            public:\n"
              "                Fred();\n"
              "                ~Fred();\n"
              "            };\n"
              "        }\n"
              "    }\n"
              "}\n"
              "ns1::ns2::ns3::Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "ns1::ns2::ns3::Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:23]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());
    }

    void class21() { // ticket #2517
        check("struct B { };\n"
              "struct C\n"
              "{\n"
              "    B * b;\n"
              "    C(B * x) : b(x) { }\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B *b;\n"
              "    C *c;\n"
              "public:\n"
              "    A() : b(new B()), c(new C(b)) { }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9:8]: (style) Class 'A' is unsafe, 'A::b' can leak by wrong usage. [unsafeClassCanLeak]\n"
                           "[test.cpp:10]: (style) Class 'A' is unsafe, 'A::c' can leak by wrong usage.\n",
                           "[test.cpp:9:8]: (style) Class 'A' is unsafe, 'A::b' can leak by wrong usage. [unsafeClassCanLeak]\n",
                           errout_str());

        check("struct B { };\n"
              "struct C\n"
              "{\n"
              "    B * b;\n"
              "    C(B * x) : b(x) { }\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B *b;\n"
              "    C *c;\n"
              "public:\n"
              "    A()\n"
              "    {\n"
              "       b = new B();\n"
              "       c = new C(b);\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9:8]: (style) Class 'A' is unsafe, 'A::b' can leak by wrong usage. [unsafeClassCanLeak]\n"
                           "[test.cpp:10]: (style) Class 'A' is unsafe, 'A::c' can leak by wrong usage.\n",
                           "[test.cpp:9:8]: (style) Class 'A' is unsafe, 'A::b' can leak by wrong usage. [unsafeClassCanLeak]\n",
                           errout_str());
    }

    void class22() { // ticket #3012 - false positive
        check("class Fred {\n"
              "private:\n"
              "    int * a;\n"
              "private:\n"
              "    Fred() { a = new int; }\n"
              "    ~Fred() { (delete(a), (a)=NULL); }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class23() { // ticket #3303 - false positive
        check("class CDataImpl {\n"
              "public:\n"
              "    CDataImpl() { m_refcount = 1; }\n"
              "    void Release() { if (--m_refcount == 0) delete this; }\n"
              "private:\n"
              "    int m_refcount;\n"
              "};\n"
              "\n"
              "class CData {\n"
              "public:\n"
              "    CData() : m_impl(new CDataImpl()) { }\n"
              "    ~CData() { if (m_impl) m_impl->Release(); }\n"
              "private:\n"
              "    CDataImpl *m_impl;\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class24() { // ticket #3806 - false positive in copy constructor
        check("class Fred {\n"
              "private:\n"
              "    int * a;\n"
              "public:\n"
              "    Fred(const Fred &fred) { a = new int; }\n"
              "    ~Fred() { delete a; }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class25() { // ticket #4367 - false positive when implementation for destructor is not seen
        check("class Fred {\n"
              "private:\n"
              "    int * a;\n"
              "public:\n"
              "    Fred() { a = new int; }\n"
              "    ~Fred();\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void class26() { // ticket #10789 - crash
        check("class C;\n"
              "struct S {\n"
              "    S() { p = new C; }\n"
              "    ~S();\n"
              "    C* p;\n"
              "};\n"
              "S::~S() = default;\n");
        ASSERT_EQUALS("[test.cpp:5:8]: (style) Class 'S' is unsafe, 'S::p' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());
    }

    void class27() { // ticket #8126 - array of pointers
        check("struct S {\n"
              "    S() {\n"
              "        for (int i = 0; i < 5; i++)\n"
              "            a[i] = new char[3];\n"
              "    }\n"
              "    char* a[5];\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:6:11]: (style) Class 'S' is unsafe, 'S::a' can leak by wrong usage. [unsafeClassCanLeak]\n", errout_str());
    }

    void staticvar() {
        check("class A\n"
              "{\n"
              "private:\n"
              "    static int * p;\n"
              "public:"
              "    A()\n"
              "    {\n"
              "        if (!p)\n"
              "            p = new int[100];\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }


    void free_member_in_sub_func() {
        // Member function
        check("class Tokenizer\n"
              "{\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "\n"
              "private:\n"
              "    int *_tokens;\n"
              "    static void deleteTokens(int *tok);\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "     _tokens = new int;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "    _tokens = 0;\n"
              "}\n"
              "\n"
              "void Tokenizer::deleteTokens(int *tok)\n"
              "{\n"
              "    delete tok;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Global function
        check("void deleteTokens(int *tok)\n"
              "{\n"
              "    delete tok;\n"
              "}\n"
              "class Tokenizer\n"
              "{\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "\n"
              "private:\n"
              "    int *_tokens;\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "     _tokens = new int;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "    _tokens = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void mismatch1() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    A(int i);\n"
              "    ~A();\n"
              "private:\n"
              "    char* pkt_buffer;\n"
              "};\n"
              "\n"
              "A::A(int i)\n"
              "{\n"
              "    pkt_buffer = new char[8192];\n"
              "    if (i != 1) {\n"
              "        delete pkt_buffer;\n"
              "        pkt_buffer = 0;\n"
              "    }\n"
              "}\n"
              "\n"
              "A::~A() {\n"
              "    delete [] pkt_buffer;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:14:9]: (error) Mismatching allocation and deallocation: A::pkt_buffer [mismatchAllocDealloc]\n", errout_str());

        check("struct S {\n" // 5678
              "    ~S();\n"
              "    void f();\n"
              "    int* p;\n"
              "};\n"
              "void S::f() {\n"
              "    p = new char[1];\n"
              "    delete p;\n"
              "    p = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8:5]: (error) Mismatching allocation and deallocation: S::p [mismatchAllocDealloc]\n", errout_str());
    }

    void mismatch2() { // #5659
        check("namespace NS\n"
              "{\n"
              "class Foo\n"
              "{\n"
              "public:\n"
              "  void fct();\n"
              "\n"
              "private:\n"
              "  char* data_;\n"
              "};\n"
              "}\n"
              "\n"
              "using namespace NS;\n"
              "\n"
              "void Foo::fct()\n"
              "{\n"
              "  data_ = new char[42];\n"
              "  delete data_;\n"
              "  data_ = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:17:3]: (warning) Possible leak in public function. The pointer 'data_' is not deallocated before it is allocated. [publicAllocationError]\n"
                      "[test.cpp:18:3]: (error) Mismatching allocation and deallocation: Foo::data_ [mismatchAllocDealloc]\n", errout_str());

        check("namespace NS\n"
              "{\n"
              "class Foo\n"
              "{\n"
              "public:\n"
              "  void fct(int i);\n"
              "\n"
              "private:\n"
              "  char* data_;\n"
              "};\n"
              "}\n"
              "\n"
              "using namespace NS;\n"
              "\n"
              "void Foo::fct(int i)\n"
              "{\n"
              "  data_ = new char[42];\n"
              "  delete data_;\n"
              "  data_ = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:17:3]: (warning) Possible leak in public function. The pointer 'data_' is not deallocated before it is allocated. [publicAllocationError]\n"
                      "[test.cpp:18:3]: (error) Mismatching allocation and deallocation: Foo::data_ [mismatchAllocDealloc]\n", errout_str());
    }

    void func1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *s;\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { free(s); }\n"
              "    void xy()\n"
              "    { s = malloc(100); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9:7]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated. [publicAllocationError]\n", errout_str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { free(s); }\n"
              "    void xy()\n"
              "    { s = malloc(100); }\n"
              "private:\n"
              "    char *s;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7:7]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated. [publicAllocationError]\n", errout_str());
    }

    void func2() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *s;\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { free(s); }\n"
              "    const Fred & operator = (const Fred &f)\n"
              "    { s = malloc(100); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9:7]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated. [publicAllocationError]\n", errout_str());
    }
};

REGISTER_TEST(TestMemleakInClass)







class TestMemleakStructMember : public TestFixture {
public:
    TestMemleakStructMember() : TestFixture("TestMemleakStructMember") {}

private:
    const Settings settings = settingsBuilder().library("std.cfg").library("posix.cfg").build();

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], bool cpp = true) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for memory leaks..
        CheckMemoryLeakStructMember checkMemoryLeakStructMember(&tokenizer, &settings, this);
        (checkMemoryLeakStructMember.check)();
    }

    void run() override {
        mNewTemplate = true;
        // testing that errors are detected
        TEST_CASE(err);

        // handle / bail out when "goto" is found
        TEST_CASE(goto_);

        // Don't report errors if the struct is returned
        TEST_CASE(ret1);
        TEST_CASE(ret2);

        // assignments
        TEST_CASE(assign1);
        TEST_CASE(assign2);
        TEST_CASE(assign3);
        TEST_CASE(assign4); // #11019
        TEST_CASE(assign5);
        TEST_CASE(assign6);

        // Failed allocation
        TEST_CASE(failedAllocation);

        TEST_CASE(function1);   // Deallocating in function
        TEST_CASE(function2);   // #2848: Taking address in function
        TEST_CASE(function3);   // #3024: kernel list
        TEST_CASE(function4);   // #3038: Deallocating in function
        TEST_CASE(function5);   // #10381, #10382, #10158

        // Handle if-else
        TEST_CASE(ifelse);

        // Linked list
        TEST_CASE(linkedlist);

        // struct variable is a global variable
        TEST_CASE(globalvar);

        // local struct variable
        TEST_CASE(localvars);

        // struct variable is a reference variable
        TEST_CASE(refvar);

        // Segmentation fault in CheckMemoryLeakStructMember
        TEST_CASE(trac5030);

        TEST_CASE(varid); // #5201: Analysis confused by (variable).attribute notation
        TEST_CASE(varid_2); // #5315: Analysis confused by ((variable).attribute) notation

        TEST_CASE(customAllocation);

        TEST_CASE(lambdaInScope); // #9793
    }

    void err() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5]: (error) Memory leak: abc.a [memleak]\n", errout_str());

        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:1]: (error) Memory leak: abc.a [memleak]\n", errout_str());

        check("static ABC * foo()\n"
              "{\n"
              "    ABC *abc = malloc(sizeof(ABC));\n"
              "    abc->a = malloc(10);\n"
              "    abc->b = malloc(10);\n"
              "    if (abc->b == 0)\n"
              "    {\n"
              "        return 0;\n"
              "    }\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:9]: (error) Memory leak: abc.a [memleak]\n", errout_str());

        check("static void foo(int a)\n"
              "{\n"
              "    ABC *abc = malloc(sizeof(ABC));\n"
              "    abc->a = malloc(10);\n"
              "    if (a == 1)\n"
              "    {\n"
              "        free(abc->a);\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:1]: (error) Memory leak: abc.a [memleak]\n", errout_str());
    }

    void goto_() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    if (abc->a)\n"
              "    { goto out; }\n"
              "    free(abc);\n"
              "    return;\n"
              "out:\n"
              "    free(abc->a);\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void ret1() {
        check("static ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("static void foo(struct ABC *abc)\n"
              "{\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7302
        check("void* foo() {\n"
              "    struct ABC abc;\n"
              "    abc.a = malloc(10);\n"
              "    return abc.a;\n"
              "}", false);
        ASSERT_EQUALS("", errout_str());

        check("void* foo() {\n"
              "    struct ABC abc;\n"
              "    abc.a = malloc(10);\n"
              "    return abc.b;\n"
              "}", false);
        ASSERT_EQUALS("[test.c:4:5]: (error) Memory leak: abc.a [memleak]\n", errout_str());
    }

    void ret2() {
        check("static ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    return &abc->self;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign1() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = abc1;\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc;\n"
              "    abc1 = abc = malloc(sizeof(ABC));\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("static void foo()\n"
              "{\n"
              " struct msn_entry *ptr;\n"
              " ptr = malloc(sizeof(struct msn_entry));\n"
              " ptr->msn = malloc(100);\n"
              " back = ptr;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

    }

    void assign2() {
        check("static void foo() {\n"
              "    struct ABC *abc = malloc(123);\n"
              "    abc->a = abc->b = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void assign3() {
        check("void f(struct s *f1) {\n"
              "    struct s f2;\n"
              "    f2.a = malloc(100);\n"
              "    *f1 = f2;\n"
              "}", false);
        ASSERT_EQUALS("", errout_str());
    }

    void assign4() {
        check("struct S { int a, b, c; };\n" // #11019
              "void f() {\n"
              "    struct S s;\n"
              "    *&s.a = open(\"xx.log\", O_RDONLY);\n"
              "    ((s).b) = open(\"xx.log\", O_RDONLY);\n"
              "    (&s)->c = open(\"xx.log\", O_RDONLY);\n"
              "}\n", false);
        ASSERT_EQUALS("[test.c:7:1]: (error) Resource leak: s.a [resourceLeak]\n"
                      "[test.c:7:1]: (error) Resource leak: s.b [resourceLeak]\n"
                      "[test.c:7:1]: (error) Resource leak: s.c [resourceLeak]\n",
                      errout_str());

        check("struct S { int *p, *q; };\n" // #7705
              "void f(S s) {\n"
              "    s.p = new int[10];\n"
              "    s.q = malloc(40);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:1]: (error) Memory leak: s.p [memleak]\n"
                      "[test.cpp:5:1]: (error) Memory leak: s.q [memleak]\n",
                      errout_str());

        check("struct S** f(struct S** s) {\n" // don't throw
              "    struct S** ret = malloc(sizeof(*ret));\n"
              "    ret[0] = malloc(sizeof(**s));\n"
              "    ret[0]->g = strdup(s[0]->g);\n"
              "    return ret;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void run_rcmd(enum rcommand rcmd, rsh_session *sess, char *cmd) {\n"
              "    sess->fp = popen(cmd, rcmd == RSH_PIPE_READ ? \"r\" : \"w\");\n"
              "}\n", false);
        ASSERT_EQUALS("", errout_str());

        check("struct S { char* a[2]; };\n"
              "enum E { E0, E1 };\n"
              "void f(struct S* s, enum E e, const char* n) {\n"
              "    free(s->a[e]);\n"
              "    s->a[e] = strdup(n);\n"
              "}\n", false);
        ASSERT_EQUALS("", errout_str());

        check("void f(struct S** s, const char* c) {\n"
              "    *s = malloc(sizeof(struct S));\n"
              "    (*s)->value = strdup(c);\n"
              "}\n", false);
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    size_t mpsz;\n"
              "    void* hdr;\n"
              "};\n"
              "void f(struct S s[static 1U], int fd, size_t size) {\n"
              "    s->mpsz = size;\n"
              "    s->hdr = mmap(NULL, s->mpsz, PROT_READ, MAP_SHARED, fd, 0);\n"
              "}\n", false);
        ASSERT_EQUALS("", errout_str());

        check("void f(type_t t) {\n"
              "    t->p = malloc(10);\n"
              "    t->x.p = malloc(10);\n"
              "    t->y[2].p = malloc(10);\n"
              "}\n", false);
        ASSERT_EQUALS("", errout_str());
    }

    void assign5() {
        check("struct S { int fd; };\n"
              "void f() {\n"
              "    struct S* s = (struct S*)malloc(sizeof(struct S));\n"
              "    s->fd = open(\"abc\", O_RDWR | O_NOCTTY);\n"
              "    free(s);\n"
              "}\n"
              "void g() {\n"
              "    struct S* s = static_cast<struct S*>(malloc(sizeof(struct S)));\n"
              "    s->fd = open(\"abc\", O_RDWR | O_NOCTTY);\n"
              "    free(s);\n"
              "}\n"
              "void h() {\n"
              "    S* s = new S;\n"
              "    s->fd = open(\"abc\", O_RDWR | O_NOCTTY);\n"
              "    delete s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:5]: (error) Resource leak: s.fd [resourceLeak]\n"
                      "[test.cpp:10:5]: (error) Resource leak: s.fd [resourceLeak]\n"
                      "[test.cpp:16:1]: (error) Resource leak: s.fd [resourceLeak]\n",
                      errout_str());

        check("struct S { int fd; };\n" // #13031
              "void f() {\n"
              "    struct S* s = malloc(sizeof(struct S));\n"
              "    s->fd = open(\"abc\", O_RDWR | O_NOCTTY);\n"
              "    if (s->fd < 0) {\n"
              "        free(s);\n"
              "        return NULL;\n"
              "    }\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void assign6() {
        check("struct S { S* p; };\n" // #14524
              "void f() {\n"
              "    S s;\n"
              "    s.p = static_cast<S*>(malloc(sizeof(S)));\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:1]: (error) Memory leak: s.p [memleak]\n", errout_str());
    }

    void failedAllocation() {
        check("static struct ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    if (!abc->a)\n"
              "    {\n"
              "        free(abc);\n"
              "        return 0;\n"
              "    }\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void function1() {
        // Not found function => assume that the function may deallocate
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    func(abc);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abclist.push_back(abc);\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // #2848: Taking address in function 'assign'
    void function2() {
        check("void f() {\n"
              "  A a = { 0 };\n"
              "  a.foo = (char *) malloc(10);\n"
              "  assign(&a);\n"
              "}", false);
        ASSERT_EQUALS("", errout_str());
    }

    // #3024: kernel list
    void function3() {
        check("void f() {\n"
              "  struct ABC *abc = malloc(100);\n"
              "  abc.a = (char *) malloc(10);\n"
              "  list_add_tail(&abc->list, head);\n"
              "}", false);
        ASSERT_EQUALS("", errout_str());
    }

    // #3038: deallocating in function
    void function4() {
        check("void a(char *p) { char *x = p; free(x); }\n"
              "void b() {\n"
              "  struct ABC abc;\n"
              "  abc.a = (char *) malloc(10);\n"
              "  a(abc.a);\n"
              "}", false);
        ASSERT_EQUALS("", errout_str());
    }

    void function5() {
        check("struct s f() {\n" // #10381
              "    struct s s1;\n"
              "    s1->x = malloc(1);\n"
              "    return (s1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct nc_rpc nc_rpc_getconfig() {\n" // #10382
              "    struct nc_rpc rpc;\n"
              "    rpc->filter = malloc(1);\n"
              "    return (nc_rpc)rpc;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("T* f(const char *str) {\n" // #10158
              "    S* s = malloc(sizeof(S));\n"
              "    s->str = strdup(str);\n"
              "    return NewT(s);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("typedef struct s { char* str; } attr_t;\n" // #10152
              "attr_t* f(int type) {\n"
              "    attr_t a;\n"
              "    switch (type) {\n"
              "    case 1:\n"
              "        a.str = strdup(\"?\");\n"
              "        break;\n"
              "    default:\n"
              "        return NULL;\n"
              "    }\n"
              "    return g(&a);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { int *p; };\n"
              "S f(int i) {\n"
              "    S s;\n"
              "    switch(i) {\n"
              "    case 1:\n"
              "        s.p = new int;\n"
              "        break;\n"
              "    default: {\n"
              "        return {};\n"
              "    }\n"
              "    }\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void ifelse() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    if (x)"
              "    {\n"
              "        abc->a = malloc(10);\n"
              "    }\n"
              "    else\n"
              "    {\n"
              "        free(abc);\n"
              "        return;\n"
              "    }\n"
              "    free(abc->a);\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void linkedlist() {
        // #3904 - false positive when linked list is used
        check("static void foo() {\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->next = malloc(sizeof(struct ABC));\n"
              "    abc->next->next = NULL;\n"
              "\n"
              "    while (abc) {\n"
              "        struct ABC *next = abc->next;\n"
              "        free(abc);\n"
              "        abc = next;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void globalvar() {
        check("struct ABC *abc;\n"
              "\n"
              "static void foo()\n"
              "{\n"
              "    abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // Ticket #933 Leaks with struct members not detected
    void localvars() {
        // Test error case
        const char code1[] = "struct A {\n"
                             "    FILE* f;\n"
                             "    char* c;\n"
                             "    void* m;\n"
                             "};\n"
                             "\n"
                             "void func() {\n"
                             "    struct A a;\n"
                             "    a.f = fopen(\"test\", \"r\");\n"
                             "    a.c = new char[12];\n"
                             "    a.m = malloc(12);\n"
                             "}";

        check(code1, true);
        ASSERT_EQUALS("[test.cpp:12:1]: (error) Resource leak: a.f [resourceLeak]\n"
                      "[test.cpp:12:1]: (error) Memory leak: a.c [memleak]\n"
                      "[test.cpp:12:1]: (error) Memory leak: a.m [memleak]\n", errout_str());
        check(code1, false);
        ASSERT_EQUALS("[test.c:12:1]: (error) Resource leak: a.f [resourceLeak]\n"
                      "[test.c:12:1]: (error) Memory leak: a.m [memleak]\n", errout_str());

        // Test OK case
        const char code2[] = "struct A {\n"
                             "    FILE* f;\n"
                             "    char* c;\n"
                             "    void* m;\n"
                             "};\n"
                             "\n"
                             "void func() {\n"
                             "    struct A a;\n"
                             "    a.f = fopen(\"test\", \"r\");\n"
                             "    a.c = new char[12];\n"
                             "    a.m = malloc(12);\n"
                             "    fclose(a.f);\n"
                             "    delete [] a.c;\n"
                             "    free(a.m);\n"
                             "}";

        check(code2, true);
        ASSERT_EQUALS("", errout_str());

        // Test unknown struct. In C++, it might have a destructor
        const char code3[] = "void func() {\n"
                             "    struct A a;\n"
                             "    a.f = fopen(\"test\", \"r\");\n"
                             "}";

        check(code3, true);
        ASSERT_EQUALS("", errout_str());
        check(code3, false);
        ASSERT_EQUALS("[test.c:4:1]: (error) Resource leak: a.f [resourceLeak]\n", errout_str());

        // Test struct with destructor
        const char code4[] = "struct A {\n"
                             "    FILE* f;\n"
                             "    ~A();\n"
                             "};\n"
                             "void func() {\n"
                             "    struct A a;\n"
                             "    a.f = fopen(\"test\", \"r\");\n"
                             "}";

        check(code4, true);
        ASSERT_EQUALS("", errout_str());
    }

    void refvar() { // #8116
        check("struct Test\n"
              "{\n"
              "  int* data;\n"
              "};\n"
              "\n"
              "void foo(Test* x)\n"
              "{\n"
              "  Test& y = *x;\n"
              "  y.data = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    // don't crash
    void trac5030() {
        check("bool bob( char const **column_ptrs ) {\n"
              "unique_ptr<char[]>otherbuffer{new char[otherbufsize+1]};\n"
              "char *const oldbuffer = otherbuffer.get();\n"
              "int const oldbufsize = otherbufsize;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void varid() { // #5201
        check("struct S {\n"
              "  void *state_check_buff;\n"
              "};\n"
              "void f() {\n"
              "  S s;\n"
              "  (s).state_check_buff = (void* )malloc(1);\n"
              "  if (s.state_check_buff == 0)\n"
              "    return;\n"
              "}", false);
        ASSERT_EQUALS("[test.c:9:1]: (error) Memory leak: s.state_check_buff [memleak]\n", errout_str());
    }

    void varid_2() { // #5315
        check("typedef struct foo { char *realm; } foo;\n"
              "void build_principal() {\n"
              "  foo f;\n"
              "  ((f)->realm) = strdup(realm);\n"
              "  if(f->realm == NULL) {}\n"
              "}", false);
        ASSERT_EQUALS("[test.c:6:1]: (error) Memory leak: f.realm [memleak]\n", errout_str());
    }

    void customAllocation() { // #4770
        check("char *myalloc(void) {\n"
              "    return malloc(100);\n"
              "}\n"
              "void func() {\n"
              "    struct ABC abc;\n"
              "    abc.a = myalloc();\n"
              "}", false);
        ASSERT_EQUALS("[test.c:7:1]: (error) Memory leak: abc.a [memleak]\n", errout_str());
    }

    void lambdaInScope() {
        check( // #9793
            "struct S { int * p{nullptr}; };\n"
            "int main()\n"
            "{\n"
            "    S s;\n"
            "    s.p = new int[10];\n"
            "    for (int i = 0; i < 10; ++i) {\n"
            "        s.p[i] = []() { return 1; }();\n"
            "    }\n"
            "    delete[] s.p;\n"
            "    return 0;\n"
            "}", true);
        ASSERT_EQUALS("", errout_str());

        check(
            "struct S { int* p; };\n"
            "void f() {\n"
            "    auto g = []() {\n"
            "      S s;\n"
            "      s.p = new int;\n"
            "    };\n"
            "}\n", true);
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Memory leak: s.p [memleak]\n", errout_str());

        check(
            "struct S { int* p; };\n"
            "void f() {\n"
            "    S s;\n"
            "    s.p = new int;\n"
            "    auto g = [&]() {\n"
            "        delete s.p;\n"
            "    };\n"
            "    g();\n"
            "}\n"
            "void h() {\n"
            "    S s;\n"
            "    s.p = new int;\n"
            "    [&]() {\n"
            "        delete s.p;\n"
            "    }();\n"
            "}\n", true);
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestMemleakStructMember)





class TestMemleakNoVar : public TestFixture {
public:
    TestMemleakNoVar() : TestFixture("TestMemleakNoVar") {}

private:
    const Settings settings = settingsBuilder().certainty(Certainty::inconclusive).severity(Severity::warning).library("std.cfg").library("posix.cfg").build();

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check for memory leaks..
        CheckMemoryLeakNoVar checkMemoryLeakNoVar(&tokenizer, &settings, this);
        (checkMemoryLeakNoVar.check)();
    }

    void run() override {
        mNewTemplate = true;
        // pass allocated memory to function..
        TEST_CASE(functionParameter);

        // never use leakable resource
        TEST_CASE(missingAssignment);

        // pass allocated memory to function using a smart pointer
        TEST_CASE(smartPointerFunctionParam);
        TEST_CASE(resourceLeak);

        // Test getAllocationType for subfunction
        TEST_CASE(getAllocationType);

        TEST_CASE(crash1); // #10729
        TEST_CASE(openDevNull); // #9653
    }

    void functionParameter() {
        // standard function..
        check("void x() {\n"
              "    strcpy(a, strdup(p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (error) Allocation with strdup, strcpy doesn't release it. [leakNoVarFunctionCall]\n", errout_str());

        check("char *x() {\n"
              "    char *ret = strcpy(malloc(10), \"abc\");\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("char *x() {\n"
              "    return strcpy(malloc(10), \"abc\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void x() {\n"
              "    free(malloc(10));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // user function..
        check("void set_error(const char *msg) {\n"
              "}\n"
              "\n"
              "void x() {\n"
              "    set_error(strdup(p));\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5:15]: (error) Allocation with strdup, set_error doesn't release it. [leakNoVarFunctionCall]\n", "", errout_str());

        check("void f()\n"
              "{\n"
              "    int fd;\n"
              "    fd = mkstemp(strdup(\"/tmp/file.XXXXXXXX\"));\n"
              "    close(fd);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Allocation with strdup, mkstemp doesn't release it.\n", "", errout_str());

        check("void f()\n"
              "{\n"
              "    if(TRUE || strcmp(strdup(a), b));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:23]: (error) Allocation with strdup, strcmp doesn't release it. [leakNoVarFunctionCall]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    if(!strcmp(strdup(a), b) == 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Allocation with strdup, strcmp doesn't release it. [leakNoVarFunctionCall]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    42, strcmp(strdup(a), b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Allocation with strdup, strcmp doesn't release it. [leakNoVarFunctionCall]\n", errout_str());

        check("void f() {\n"
              "   assert(freopen(\"/dev/null\", \"r\", stdin));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void x() {\n"
              "    strcpy(a, (void*)strdup(p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (error) Allocation with strdup, strcpy doesn't release it. [leakNoVarFunctionCall]\n", errout_str());

        check("void* malloc1() {\n"
              "    return (malloc(1));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("char *x() {\n"
              "    char *ret = (char*)strcpy(malloc(10), \"abc\");\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    free(malloc(1));\n"
              "    strcpy(a, strdup(p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Allocation with strdup, strcpy doesn't release it. [leakNoVarFunctionCall]\n", errout_str());

        check("void f() {\n"
              "    memcmp(calloc(10, 10), strdup(q), 100);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (error) Allocation with calloc, memcmp doesn't release it. [leakNoVarFunctionCall]\n"
                      "[test.cpp:2:28]: (error) Allocation with strdup, memcmp doesn't release it. [leakNoVarFunctionCall]\n", errout_str());

        check("void* f(int size) {\n"
              "    return (void*) malloc(size);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int* f(int size) {\n"
              "    return static_cast<int*>(malloc(size));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() { if (new int[42]) {} }\n" // #10857
              "void g() { if (malloc(42)) {} }\n");
        ASSERT_EQUALS("[test.cpp:1:16]: (error) Allocation with new, if doesn't release it. [leakNoVarFunctionCall]\n"
                      "[test.cpp:2:16]: (error) Allocation with malloc, if doesn't release it. [leakNoVarFunctionCall]\n",
                      errout_str());

        check("const char* string(const char* s) {\n"
              "    StringSet::iterator it = strings_.find(s);\n"
              "    if (it != strings_.end())\n"
              "        return *it;\n"
              "    return *strings_.insert(it, std::strcpy(new char[std::strlen(s) + 1], s));\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    static void load(const QString& projPath) {\n"
              "        if (proj_)\n"
              "            return;\n"
              "        proj_ = new ProjectT(projPath);\n"
              "        proj_->open(new OpenCallback());\n"
              "    }\n"
              "private:\n"
              "    static Core::ProjectBase* proj_;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::string& s, int n) {\n"
              "    std::unique_ptr<char[]> u;\n"
              "    u.reset(strcpy(new char[n], s.c_str()));\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { char* p; };\n"
              "void f(S* s, int N) {\n"
              "    s->p = s->p ? strcpy(new char[N], s->p) : nullptr;\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {};\n" // #11866
              "void f(bool b);\n"
              "void g() {\n"
              "    f(new int());\n"
              "    f(new std::vector<int>());\n"
              "    f(new S());\n"
              "    f(new tm());\n"
              "    f(malloc(sizeof(S)));\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Allocation with new, f doesn't release it. [leakNoVarFunctionCall]\n"
                      "[test.cpp:5:7]: (error) Allocation with new, f doesn't release it. [leakNoVarFunctionCall]\n"
                      "[test.cpp:6:7]: (error) Allocation with new, f doesn't release it. [leakNoVarFunctionCall]\n"
                      "[test.cpp:7:7]: (error) Allocation with new, f doesn't release it. [leakNoVarFunctionCall]\n"
                      "[test.cpp:8:7]: (error) Allocation with malloc, f doesn't release it. [leakNoVarFunctionCall]\n",
                      errout_str());

        check("void f(uintptr_t u);\n"
              "void g() {\n"
              "    f((uintptr_t)new int());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(uint8_t u);\n"
              "void g() {\n"
              "    f((uint8_t)new int());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:16]: (error) Allocation with new, f doesn't release it. [leakNoVarFunctionCall]\n",
                      errout_str());

        check("void f(int i, T t);\n"
              "void g(int i, U* u);\n"
              "void h() {\n"
              "    f(1, new int());\n"
              "    g(1, new int());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(T t);\n"
              "struct U {};\n"
              "void g() {\n"
              "    f(new U());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n" // #14339
              "    void g(int* p) { p_ = p; }\n"
              "    int *p_;\n"
              "};\n"
              "void f(A& a) {\n"
              "    a.g(new int);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void missingAssignment() {
        check("void x()\n"
              "{\n"
              "    malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n", errout_str());

        check("void x()\n"
              "{\n"
              "    calloc(10, 1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Return value of allocation function 'calloc' is not stored. [leakReturnValNotUsed]\n", errout_str());

        check("void x()\n"
              "{\n"
              "    strdup(\"Test\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Return value of allocation function 'strdup' is not stored. [leakReturnValNotUsed]\n", errout_str());

        check("void x()\n"
              "{\n"
              "    (char*) malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:13]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n", errout_str());

        check("void x()\n"
              "{\n"
              "    char* ptr = malloc(10);\n"
              "    foo(ptr);\n"
              "    free(ptr);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f( void ) {\n" // FP: #11246
              "  void* address = malloc( 1 );\n"
              "  int ok = address != 0;\n"
              "  if ( ok )\n"
              "    free( address ), address = 0;\n"
              "  return 0;\n"
              "} ");
        ASSERT_EQUALS("", errout_str());

        check("char** x(const char* str) {\n"
              "    char* ptr[] = { malloc(10), malloc(5), strdup(str) };\n"
              "    return ptr;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void x()\n"
              "{\n"
              "    42,malloc(42);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:8]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n", errout_str());

        check("void *f()\n"
              "{\n"
              "    return malloc(10);\n"
              "}\n"
              "void x()\n"
              "{\n"
              "    f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Return value of allocation function 'f' is not stored. [leakReturnValNotUsed]\n", errout_str());

        check("void f()\n" // #8100
              "{\n"
              "    auto lambda = [](){return malloc(10);};\n"
              "}\n"
              "void x()\n"
              "{\n"
              "    f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void *f() {\n" // #8848
              "    struct S { void *alloc() { return malloc(10); } };\n"
              "}\n"
              "void x()\n"
              "{\n"
              "    f();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void x()\n"
              "{\n"
              "    if(!malloc(5)) fail();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n", errout_str());

        check("FOO* factory() {\n"
              "    FOO* foo = new (std::nothrow) FOO;\n"
              "    return foo;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #6536
        check("struct S { S(int) {} };\n"
              "void foo(int i) {\n"
              "  S socket(i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #6693
        check("struct CTest {\n"
              "    void Initialise();\n"
              "    void malloc();\n"
              "};\n"
              "void CTest::Initialise() {\n"
              "    malloc();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n" // #7348 - cast
              "    p = (::X*)malloc(42);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7182 "crash: CheckMemoryLeak::functionReturnType()"
        check("template<typename... Ts> auto unary_right_comma (Ts... ts) { return (ts , ...); }\n"
              "template<typename T, typename... Ts> auto binary_left_comma (T x, Ts... ts) { return (x , ... , ts); }\n"
              "int main() {\n"
              "  unary_right_comma (a);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    new int[10];\n"
              "    new int[10][5];\n"
              "    new int[10]();\n"
              "    new int[10]{};\n"
              "    new int[] { 1, 2, 3 };\n"
              "    new std::string;\n"
              "    new int;\n"
              "    new int();\n"
              "    new int(1);\n"
              "    new int{};\n"
              "    new int{ 1 };\n"
              "    new uint8_t[4];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:3:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:4:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:5:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:6:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:7:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:8:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:9:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:10:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:11:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:12:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:13:5]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n",
                      errout_str());

        check("void f(int* p) {\n"
              "    new auto('c');\n"
              "    new(p) int;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:1:29]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                           "[test.cpp:3]: (error) Return value of allocation function 'new' is not stored.\n",
                           "",
                           errout_str());

        check("void g(int* p) {\n"
              "    new QWidget;\n"
              "    new QWidget();\n"
              "    new QWidget{ this };\n"
              "    h(new int[10], 1);\n"
              "    h(new int[10][5], 1);\n"
              "    h(new int[10](), 1);\n"
              "    h(new int[10]{}, 1);\n"
              "    h(new int[] { 1, 2, 3 }, 1);\n"
              "    h(new auto('c'), 1);\n"
              "    h(new std::string, 1);\n"
              "    h(new int, 1);\n"
              "    h(new int{}, 1);\n"
              "    h(new int(), 1);\n"
              "    h(new int{ 1 }, 1);\n"
              "    h(new int(1), 1);\n"
              "    h(new(p) int, 1);\n"
              "    h(new QWidget, 1);\n"
              "    C{ new int[10], 1 };\n"
              "    C{ new int[10](), 1 };\n"
              "    C{ new int[10]{}, 1 };\n"
              "    C{ new int[] { 1, 2, 3 }, 1 };\n"
              "    C{ new auto('c'), 1 };\n"
              "    C{ new std::string, 1 };\n"
              "    C{ new int, 1 };\n"
              "    C{ new int{}, 1 };\n"
              "    C{ new int(), 1 };\n"
              "    C{ new int{ 1 }, 1 };\n"
              "    C{ new int(1), 1 };\n"
              "    C{ new(p) int, 1 };\n"
              "    C{ new QWidget, 1 };\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) { if (b && malloc(42)) {} }\n" //  // #10858
              "void g(bool b) { if (b || malloc(42)) {} }\n");
        ASSERT_EQUALS("[test.cpp:1:27]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:2:27]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n",
                      errout_str());

        check("void f0(const bool b) { b ? new int : nullptr; }\n" // #11155
              "void f1(const bool b) { b ? nullptr : new int; }\n"
              "int* g0(const bool b) { return b ? new int : nullptr; }\n"
              "void g1(const bool b) { h(b, b ? nullptr : new int); }\n");
        ASSERT_EQUALS("[test.cpp:1:29]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:2:39]: (error) Return value of allocation function 'new' is not stored. [leakReturnValNotUsed]\n",
                      errout_str());

        check("void f() {\n" // #11157
              "    switch (*new int) { case 42: break; }\n"
              "    switch (*malloc(42)) { case 42: break; }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:14]: (error) Allocation with new, switch doesn't release it. [leakNoVarFunctionCall]\n"
                      "[test.cpp:3:14]: (error) Allocation with malloc, switch doesn't release it. [leakNoVarFunctionCall]\n",
                      errout_str());

        check("void f() {\n"
              "    Ref<StringBuffer> remove(new StringBuffer());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #11039
              "    delete new int;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #11327
              "    int* p = (new int[3]) + 1;\n"
              "    delete[] &p[-1];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(FILE* fp1, FILE* fp2) {\n" // #14171
              "    if (freopen(NULL, \"w+b\", fp1) == NULL) {}\n"
              "    if (std::freopen(NULL, \"w+b\", fp2) == NULL) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #14172
              "    if (malloc(4) == nullptr) {}\n"
              "    if (::malloc(4) == nullptr) {}\n"
              "    if (std::malloc(4) == nullptr) {}\n"
              "    if (::std::malloc(4) == nullptr) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:3:11]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:4:14]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n"
                      "[test.cpp:5:16]: (error) Return value of allocation function 'malloc' is not stored. [leakReturnValNotUsed]\n",
                      errout_str());
    }

    void smartPointerFunctionParam() {
        check("void x() {\n"
              "    f(shared_ptr<int>(new int(42)), g());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_shared<int>() instead. [leakUnsafeArgAlloc]\n", errout_str());

        check("void x() {\n"
              "    h(12, f(shared_ptr<int>(new int(42)), g()));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:11]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_shared<int>() instead. [leakUnsafeArgAlloc]\n", errout_str());

        check("void x() {\n"
              "    f(unique_ptr<int>(new int(42)), g());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_unique<int>() instead. [leakUnsafeArgAlloc]\n", errout_str());

        check("void x() {\n"
              "    f(g(), shared_ptr<int>(new int(42)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_shared<int>() instead. [leakUnsafeArgAlloc]\n", errout_str());

        check("void x() {\n"
              "    f(g(), unique_ptr<int>(new int(42)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_unique<int>() instead. [leakUnsafeArgAlloc]\n", errout_str());

        check("void x() {\n"
              "    f(shared_ptr<char>(new char), make_unique<int>(32));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Unsafe allocation. If make_unique<int>() throws, memory could be leaked. Use make_shared<char>() instead. [leakUnsafeArgAlloc]\n", errout_str());

        check("void x() {\n"
              "    f(g(124), h(\"test\", 234), shared_ptr<char>(new char));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Unsafe allocation. If h() throws, memory could be leaked. Use make_shared<char>() instead. [leakUnsafeArgAlloc]\n", errout_str());

        check("void x() {\n"
              "    f(shared_ptr<std::string>(new std::string(\"\")), g<std::string>());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning, inconclusive) Unsafe allocation. If g<std::string>() throws, memory could be leaked. Use make_shared<std::string>() instead. [leakUnsafeArgAlloc]\n", errout_str());

        check("void g(int x) throw() { }\n"
              "void x() {\n"
              "    f(g(124), shared_ptr<char>(new char));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void __declspec(nothrow) g(int x) { }\n"
              "void x() {\n"
              "    f(g(124), shared_ptr<char>(new char));\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }
    void resourceLeak() {
        check("bool f(const char* name)\n" // FP: #12253
              "{\n"
              "    FILE* fp = fopen(name, \"r\");\n"
              "    bool result = (fp == nullptr);\n"
              "    if (result) return !result;\n"
              "    fclose(fp);\n"
              "    return result;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "  fopen(\"file.txt\", \"r\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:3]: (error) Return value of allocation function 'fopen' is not stored. [leakReturnValNotUsed]\n", errout_str());

        check("void foo() {\n"
              "  FILE *f = fopen(\"file.txt\", \"r\");\n"
              "  freopen(\"file.txt\", \"r\", f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3]: (error) Return value of allocation function 'freopen' is not stored. [leakReturnValNotUsed]\n", errout_str());

        check("void foo() {\n"
              "  freopen(\"file.txt\", \"r\", stdin);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder h ( fopen(\"file.txt\", \"r\"));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder ( fopen(\"file.txt\", \"r\"));\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder h { fopen(\"file.txt\", \"r\")};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder h = fopen(\"file.txt\", \"r\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Holder {\n"
              "  Holder(FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder { fopen(\"file.txt\", \"r\")};\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Holder {\n"
              "  Holder(int i, FILE* f) : file(f) {}\n"
              "  ~Holder() { fclose(file); }\n"
              "  FILE* file;\n"
              "};\n"
              "void foo() {\n"
              "  Holder { 0, fopen(\"file.txt\", \"r\")};\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void getAllocationType() {
        // #7845
        check("class Thing { Thing(); };\n"
              "Thing * makeThing() { Thing *thing = new Thing; return thing; }\n"
              "\n"
              "void f() {\n"
              "  makeThing();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #10631
        check("struct Thing {\n"
              "    Thing();\n"
              "};\n"
              "std::vector<Thing*> g_things;\n"
              "Thing* makeThing() {\n"
              "    Thing* n = new Thing();\n"
              "    return n;\n"
              "}\n"
              "Thing::Thing() {\n"
              "    g_things.push_back(this);\n"
              "}\n"
              "void f() {\n"
              "    makeThing();\n"
              "    for(Thing* t : g_things) {\n"
              "        delete t;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void crash1() { // #10729
        check("void foo() {\n"
              "    extern void *realloc (void *ptr, size_t size);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    extern void *malloc (size_t size);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void openDevNull() {
        check("void f() {\n" // #9653
              "    (void)open(\"/dev/null\", O_RDONLY);\n"
              "    open(\"/dev/null\", O_WRONLY);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }
};
REGISTER_TEST(TestMemleakNoVar)


