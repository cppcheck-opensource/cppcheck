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


#include "checkstring.h"
#include "errortypes.h"
#include "helpers.h"
#include "settings.h"
#include "fixture.h"

#include <cstddef>
#include <string>

class TestString : public TestFixture {
public:
    TestString() : TestFixture("TestString") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::warning).severity(Severity::style).build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(stringLiteralWrite);

        TEST_CASE(alwaysTrueFalseStringCompare);
        TEST_CASE(suspiciousStringCompare);
        TEST_CASE(suspiciousStringCompare_char);

        TEST_CASE(strPlusChar1);     // "/usr" + '/'
        TEST_CASE(strPlusChar2);     // "/usr" + ch
        TEST_CASE(strPlusChar3);     // ok: path + "/sub" + '/'
        TEST_CASE(strPlusChar4);     // L"/usr" + L'/'

        TEST_CASE(snprintf1);       // Dangerous usage of snprintf
        TEST_CASE(sprintf1);        // Dangerous usage of sprintf
        TEST_CASE(sprintf2);
        TEST_CASE(sprintf3);
        TEST_CASE(sprintf4);        // struct member
        TEST_CASE(sprintf5);        // another struct member
        TEST_CASE(sprintf6);        // (char*)
        TEST_CASE(sprintf7);        // (char*)(void*)
        TEST_CASE(wsprintf1);       // Dangerous usage of wsprintf

        TEST_CASE(incorrectStringCompare);

        TEST_CASE(deadStrcmp);
    }

    struct CheckOptions
    {
        bool cpp = true;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        SimpleTokenizer2 tokenizer(settings, *this, code, options.cpp ? "test.cpp" : "test.c");

        // Tokenize..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        CheckString check;
        runChecks(check, tokenizer, *this);
    }

    void stringLiteralWrite() {
        check("void f() {\n"
              "  char *abc = \"abc\";\n"
              "  abc[0] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3] -> [test.cpp:2:15]: (error) Modifying string literal \"abc\" directly or indirectly is undefined behaviour. [stringLiteralWrite]\n", errout_str());

        check("void f() {\n"
              "  char *abc = \"abc\";\n"
              "  *abc = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:4] -> [test.cpp:2:15]: (error) Modifying string literal \"abc\" directly or indirectly is undefined behaviour. [stringLiteralWrite]\n", errout_str());

        check("void f() {\n"
              "  char *abc = \"A very long string literal\";\n"
              "  abc[0] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3] -> [test.cpp:2:15]: (error) Modifying string literal \"A very long stri..\" directly or indirectly is undefined behaviour. [stringLiteralWrite]\n", errout_str());

        check("void f() {\n"
              "  QString abc = \"abc\";\n"
              "  abc[0] = 'a';\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo_FP1(char *p) {\n"
              "  p[1] = 'B';\n"
              "}\n"
              "void foo_FP2(void) {\n"
              "  char* s = \"Y\";\n"
              "  foo_FP1(s);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:2:3] -> [test.cpp:5:13]: (error) Modifying string literal \"Y\" directly or indirectly is undefined behaviour. [stringLiteralWrite]\n",
            errout_str());

        check("void foo_FP1(char *p) {\n"
              "  p[1] = 'B';\n"
              "}\n"
              "void foo_FP2(void) {\n"
              "  char s[10] = \"Y\";\n"
              "  foo_FP1(s);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  wchar_t *abc = L\"abc\";\n"
              "  abc[0] = u'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3] -> [test.cpp:2:18]: (error) Modifying string literal L\"abc\" directly or indirectly is undefined behaviour. [stringLiteralWrite]\n", errout_str());

        check("void f() {\n"
              "  char16_t *abc = u\"abc\";\n"
              "  abc[0] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:3] -> [test.cpp:2:19]: (error) Modifying string literal u\"abc\" directly or indirectly is undefined behaviour. [stringLiteralWrite]\n", errout_str());

        check("void foo() {\n" // #8332
              "    int i;\n"
              "    char *p  = \"string literal\";\n"
              "    for( i = 0; i < strlen(p); i++) {\n"
              "        p[i] = \'X\';\n" // <<
              "    }\n"
              "    printf(\"%s\\n\", p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9] -> [test.cpp:3:16]: (error) Modifying string literal \"string literal\" directly or indirectly is undefined behaviour. [stringLiteralWrite]\n", errout_str());
    }

    void alwaysTrueFalseStringCompare() {
        check("void f() {\n"
              "  if (strcmp(\"A\",\"A\")){}\n"
              "  if (strncmp(\"A\",\"A\",1)){}\n"
              "  if (strcasecmp(\"A\",\"A\")){}\n"
              "  if (strncasecmp(\"A\",\"A\",1)){}\n"
              "  if (memcmp(\"A\",\"A\",1)){}\n"
              "  if (strverscmp(\"A\",\"A\")){}\n"
              "  if (bcmp(\"A\",\"A\",1)){}\n"
              "  if (wcsncasecmp(L\"A\",L\"A\",1)){}\n"
              "  if (wcsncmp(L\"A\",L\"A\",1)){}\n"
              "  if (wmemcmp(L\"A\",L\"A\",1)){}\n"
              "  if (wcscmp(L\"A\",L\"A\")){}\n"
              "  if (wcscasecmp(L\"A\",L\"A\")){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:3:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:4:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:5:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:6:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:7:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:8:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:9:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:10:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:11:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:12:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n"
                      "[test.cpp:13:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n", errout_str());

        // avoid false positives when the address is modified #6415
        check("void f(void *p, int offset)  {\n"
              "     if (!memcmp(p, p + offset, 42)){}\n"
              "     if (!memcmp(p + offset, p, 42)){}\n"
              "     if (!memcmp(offset + p, p, 42)){}\n"
              "     if (!memcmp(p, offset + p, 42)){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // avoid false positives when the address is modified #6415
        check("void f(char *c, int offset)  {\n"
              "     if (!memcmp(c, c + offset, 42)){}\n"
              "     if (!memcmp(c + offset, c, 42)){}\n"
              "     if (!memcmp(offset + c, c, 42)){}\n"
              "     if (!memcmp(c, offset + c, 42)){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // avoid false positives when the address is modified #6415
        check("void f(std::string s, int offset)  {\n"
              "     if (!memcmp(s.c_str(), s.c_str() + offset, 42)){}\n"
              "     if (!memcmp(s.c_str() + offset, s.c_str(), 42)){}\n"
              "     if (!memcmp(offset + s.c_str(), s.c_str(), 42)){}\n"
              "     if (!memcmp(s.c_str(), offset + s.c_str(), 42)){}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f(void) {\n" // #13527
              "    const wchar_t* str1 = L\"Hello\";\n"
              "    return wcsicmp(str1, str1);\n" // <<
              "}");
        ASSERT_EQUALS("[test.cpp:3:12]: (warning) Comparison of identical string variables. [stringCompare]\n", errout_str());

        check("int main()\n"
              "{\n"
              "  if (strcmp(\"00FF00\", \"00FF00\") == 0)"
              "  {"
              "    std::cout << \"Equal\";"
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n", errout_str());

        check("void f() {\n"
              "  if (strcmp($\"00FF00\", \"00FF00\") == 0) {}"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  if ($strcmp(\"00FF00\", \"00FF00\") == 0) {}"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int main()\n"
              "{\n"
              "  if (stricmp(\"hotdog\",\"HOTdog\") == 0)"
              "  {"
              "    std::cout << \"Equal\";"
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n", errout_str());

        check("int main()\n"
              "{\n"
              "  if (QString::compare(\"Hamburger\", \"Hotdog\") == 0)"
              "  {"
              "    std::cout << \"Equal\";"
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n", errout_str());

        check("int main()\n"
              "{\n"
              "  if (QString::compare(argv[2], \"hotdog\") == 0)"
              "  {"
              "    std::cout << \"Equal\";"
              "  }"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int main()\n"
              "{\n"
              "  if (strncmp(\"hotdog\",\"hotdog\", 6) == 0)"
              "  {"
              "    std::cout << \"Equal\";"
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n", errout_str());

        check("int foo(const char *buf)\n"
              "{\n"
              "  if (strcmp(buf, buf) == 0)"
              "  {"
              "    std::cout << \"Equal\";"
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (warning) Comparison of identical string variables. [stringCompare]\n", errout_str());

        check("int foo(const std::string& buf)\n"
              "{\n"
              "  if (stricmp(buf.c_str(), buf.c_str()) == 0)"
              "  {"
              "    std::cout << \"Equal\";"
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3:7]: (warning) Comparison of identical string variables. [stringCompare]\n", errout_str());

        check("int main() {\n"
              "  if (\"str\" == \"str\") {\n"
              "    std::cout << \"Equal\";\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:6]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n", errout_str());

        check("int main() {\n"
              "  if (\"str\" != \"str\") {\n"
              "    std::cout << \"Equal\";\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:6]: (warning) Unnecessary comparison of static strings. [staticStringCompare]\n", errout_str());

        check("int main() {\n"
              "  if (a+\"str\" != \"str\"+b) {\n"
              "    std::cout << \"Equal\";\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void suspiciousStringCompare() {
        check("bool foo(char* c) {\n"
              "    return c == \"x\";\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (warning) String literal compared with variable 'c'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        check("bool foo(char** c) {\n"
              "    return c[3] == \"x\";\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) String literal compared with variable 'c[3]'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        check("bool foo(wchar_t* c) {\n"
              "    return c == L\"x\";\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (warning) String literal compared with variable 'c'. Did you intend to use wcscmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        check("bool foo(const char* c) {\n"
              "    return \"x\" == c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:16]: (warning) String literal compared with variable 'c'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        check("bool foo(char* c) {\n"
              "    return foo+\"x\" == c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool foo(char* c) {\n"
              "    return \"x\" == c+foo;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool foo(char* c) {\n"
              "    return \"x\" == c+foo;\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:2:16]: (warning) String literal compared with variable 'c+foo'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        check("bool foo(Foo c) {\n"
              "    return \"x\" == c.foo;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool foo(Foo c) {\n"
              "    return \"x\" == c.foo;\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:2:16]: (warning) String literal compared with variable 'c.foo'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        check("bool foo(const std::string& c) {\n"
              "    return \"x\" == c;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool foo(const Foo* c) {\n"
              "    return \"x\" == c->bar();\n" // #4314
              "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #4257
        check("bool foo() {\n"
              "MyString *str=Getter();\n"
              "return *str==\"bug\"; }\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:12]: (warning) String literal compared with variable '*str'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        // Ticket #4257
        check("bool foo() {\n"
              "MyString *str=Getter();\n"
              "return *str==\"bug\"; }");
        ASSERT_EQUALS("", errout_str());

        // Ticket #4257
        check("bool foo() {\n"
              "MyString **str=OtherGetter();\n"
              "return *str==\"bug\"; }", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:12]: (warning) String literal compared with variable '*str'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        // Ticket #4257
        check("bool foo() {\n"
              "MyString str=OtherGetter2();\n"
              "return &str==\"bug\"; }", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:3:12]: (warning) String literal compared with variable '&str'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        // Ticket #5734
        check("int foo(char c) {\n"
              "return c == '4';}");
        ASSERT_EQUALS("", errout_str());
        check("int foo(char c) {\n"
              "return c == '4';}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());
        check("int foo(char c) {\n"
              "return c == \"42\"[0];}");
        ASSERT_EQUALS("", errout_str());
        check("int foo(char c) {\n"
              "return c == \"42\"[0];}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        // 5639 String literal compared with char buffer in a struct
        check("struct Example {\n"
              "   char buffer[200];\n"
              "};\n"
              "void foo() {\n"
              "   struct Example example;\n"
              "   if (example.buffer == \"test\") ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:23]: (warning) String literal compared with variable 'example.buffer'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());
        check("struct Example {\n"
              "   char buffer[200];\n"
              "};\n"
              "void foo() {\n"
              "   struct Example example;\n"
              "   if (example.buffer == \"test\") ;\n"
              "}\n", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("[test.c:6:23]: (warning) String literal compared with variable 'example.buffer'. Did you intend to use strcmp() instead? [literalWithCharPtrCompare]\n", errout_str());

        // #9726
        check("void f(std::vector<std::string> theArgs) {\n"
              "   std::string arg1(*theArgs.begin());\n"
              "   if(arg1 == \"aaa\") {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void suspiciousStringCompare_char() {
        check("bool foo(char* c) {\n"
              "    return c == 'x';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (warning) Char literal compared with pointer 'c'. Did you intend to dereference it? [charLiteralWithCharPtrCompare]\n", errout_str());

        check("bool foo(wchar_t* c) {\n"
              "    return c == L'x';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:14]: (warning) Char literal compared with pointer 'c'. Did you intend to dereference it? [charLiteralWithCharPtrCompare]\n", errout_str());

        check("bool foo(char* c) {\n"
              "    return '\\0' != c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Char literal compared with pointer 'c'. Did you intend to dereference it? [charLiteralWithCharPtrCompare]\n", errout_str());

        check("bool foo(char c) {\n"
              "    return c == '\\0';\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool foo(char* c) {\n"
              "    return c[0] == '\\0';\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool foo(char** c) {\n"
              "    return c[0] == '\\0';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Char literal compared with pointer 'c[0]'. Did you intend to dereference it? [charLiteralWithCharPtrCompare]\n", errout_str());

        check("bool foo(char** c) {\n"
              "    return *c == '\\0';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:15]: (warning) Char literal compared with pointer '*c'. Did you intend to dereference it? [charLiteralWithCharPtrCompare]\n", errout_str());

        check("bool foo(char c) {\n"
              "    return c == 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool foo(char* c) {\n"
              "    return *c == 0;\n"
              "}", dinit(CheckOptions, $.cpp = false));
        ASSERT_EQUALS("", errout_str());

        check("bool foo(char* c) {\n"
              "    return *c == 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool foo(Foo* c) {\n"
              "    return 0 == c->x;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(char* c) {\n"
              "    if(c == '\\0') bar();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:10]: (warning) Char literal compared with pointer 'c'. Did you intend to dereference it? [charLiteralWithCharPtrCompare]\n", errout_str());

        check("void f() {\n"
              "  struct { struct { char *str; } x; } a;\n"
              "  return a.x.str == '\\0';"
              "}");
        ASSERT_EQUALS("[test.cpp:3:18]: (warning) Char literal compared with pointer 'a.x.str'. Did you intend to dereference it? [charLiteralWithCharPtrCompare]\n", errout_str());

        check("void f() {\n"
              "  struct { struct { char *str; } x; } a;\n"
              "  return *a.x.str == '\\0';"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    void snprintf1() {
        check("void foo()\n"
              "{\n"
              "    char buf[100];\n"
              "    snprintf(buf,100,\"%s\",buf);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:27]: (error) Undefined behavior: Variable 'buf' is used as parameter and destination in snprintf(). [sprintfOverlappingData]\n", errout_str());
    }

    void sprintf1() {
        check("void foo()\n"
              "{\n"
              "    char buf[100];\n"
              "    sprintf(buf,\"%s\",buf);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:22]: (error) Undefined behavior: Variable 'buf' is used as parameter and destination in sprintf(). [sprintfOverlappingData]\n", errout_str());
    }

    void sprintf2() {
        check("void foo()\n"
              "{\n"
              "    char buf[100];\n"
              "    sprintf(buf,\"%i\",sizeof(buf));\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void sprintf3() {
        check("void foo()\n"
              "{\n"
              "    char buf[100];\n"
              "    sprintf(buf,\"%i\",sizeof(buf));\n"
              "    if (buf[0]);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void sprintf4() {
        check("struct A\n"
              "{\n"
              "    char filename[128];\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    const char* filename = \"hello\";\n"
              "    struct A a;\n"
              "    snprintf(a.filename, 128, \"%s\", filename);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void sprintf5() {
        check("struct A\n"
              "{\n"
              "    char filename[128];\n"
              "};\n"
              "\n"
              "void foo(struct A *a)\n"
              "{\n"
              "    snprintf(a->filename, 128, \"%s\", a->filename);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:39]: (error) Undefined behavior: Variable 'a->filename' is used as parameter and destination in snprintf(). [sprintfOverlappingData]\n", errout_str());
    }

    void sprintf6() {
        check("void foo()\n"
              "{\n"
              "    char buf[100];\n"
              "    sprintf((char*)buf,\"%s\",(char*)buf);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:29]: (error) Undefined behavior: Variable 'buf' is used as parameter and destination in sprintf(). [sprintfOverlappingData]\n", errout_str());
    }

    void sprintf7() {
        check("void foo()\n"
              "{\n"
              "    char buf[100];\n"
              "    sprintf((char*)(void*)buf,\"%s\",(void*)(char*)buf);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:36]: (error) Undefined behavior: Variable 'buf' is used as parameter and destination in sprintf(). [sprintfOverlappingData]\n", errout_str());
    }

    void wsprintf1() {
        check("void foo()\n"
              "{\n"
              "    wchar_t buf[100];\n"
              "    swprintf(buf,10, \"%s\",buf);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:27]: (error) Undefined behavior: Variable 'buf' is used as parameter and destination in swprintf(). [sprintfOverlappingData]\n", errout_str());
    }

    void strPlusChar1() {
        // Strange looking pointer arithmetic..
        check("void foo()\n"
              "{\n"
              "    const char *p = \"/usr\" + '/';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:28]: (error) Unusual pointer arithmetic. A value of type 'char' is added to a string literal. [strPlusChar]\n", errout_str());
    }

    void strPlusChar2() {
        // Strange looking pointer arithmetic..
        check("void foo()\n"
              "{\n"
              "    char ch = 1;\n"
              "    const char *p = ch + \"/usr\";\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Strange looking pointer arithmetic..
        check("void foo()\n"
              "{\n"
              "    int i = 1;\n"
              "    const char* psz = \"Bla\";\n"
              "    const std::string str = i + psz;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void strPlusChar3() {
        // Strange looking pointer arithmetic..
        check("void foo()\n"
              "{\n"
              "    std::string temp = \"/tmp\";\n"
              "    std::string path = temp + '/' + \"sub\" + '/';\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void strPlusChar4() {
        // Strange looking pointer arithmetic, wide char..
        check("void foo()\n"
              "{\n"
              "    const wchar_t *p = L\"/usr\" + L'/';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:32]: (error) Unusual pointer arithmetic. A value of type 'wchar_t' is added to a string literal. [strPlusChar]\n", errout_str());

        check("void foo(wchar_t c)\n"
              "{\n"
              "    const wchar_t *p = L\"/usr\" + c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:32]: (error) Unusual pointer arithmetic. A value of type 'wchar_t' is added to a string literal. [strPlusChar]\n", errout_str());
    }

    void incorrectStringCompare() {
        check("int f() {\n"
              "    return test.substr( 0 , 4 ) == \"Hello\" ? 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) String literal \"Hello\" doesn't match length argument for substr(). [incorrectStringCompare]\n", errout_str());

        check("int f() {\n"
              "    return test.substr( 0 , 4 ) == L\"Hello\" ? 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) String literal L\"Hello\" doesn't match length argument for substr(). [incorrectStringCompare]\n", errout_str());

        check("int f() {\n"
              "    return test.substr( 0 , 5 ) == \"Hello\" ? 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    return \"Hello\" == test.substr( 0 , 4 ) ? 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:28]: (warning) String literal \"Hello\" doesn't match length argument for substr(). [incorrectStringCompare]\n", errout_str());

        check("int f() {\n"
              "    return \"Hello\" == foo.bar<int>().z[1].substr(i+j*4, 4) ? 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:43]: (warning) String literal \"Hello\" doesn't match length argument for substr(). [incorrectStringCompare]\n", errout_str());

        check("int f() {\n"
              "    return \"Hello\" == test.substr( 0 , 5 ) ? 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    if (\"Hello\") { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("int f() {\n"
              "    if (\"Hello\" && test) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:9]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("int f() {\n"
              "    if (test && \"Hello\") { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:17]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("int f() {\n"
              "    while (\"Hello\") { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("int f() {\n"
              "    return \"Hello\" ? 1 : 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:12]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("int f() {\n"
              "    assert (test || \"Hello\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:21]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("int f() {\n"
              "    assert (test && \"Hello\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    assert (\"Hello\" || test);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:13]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("int f() {\n"
              "    assert (\"Hello\" && test);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    BOOST_ASSERT (\"Hello\" && test);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    return f2(\"Hello\");\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7750 warn about char literals in boolean expressions
        check("void f() {\n"
              "  if('a'){}\n"
              "  if(L'b'){}\n"
              "  if(1 && 'c'){}\n"
              "  int x = 'd' ? 1 : 2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:6]: (warning) Conversion of char literal 'a' to bool always evaluates to true. [incorrectCharBooleanError]\n"
                      "[test.cpp:3:6]: (warning) Conversion of char literal L'b' to bool always evaluates to true. [incorrectCharBooleanError]\n"
                      "[test.cpp:4:11]: (warning) Conversion of char literal 'c' to bool always evaluates to true. [incorrectCharBooleanError]\n"
                      "[test.cpp:5:11]: (warning) Conversion of char literal 'd' to bool always evaluates to true. [incorrectCharBooleanError]\n"
                      , errout_str());

        check("void f() {\n"
              "  if('\\0'){}\n"
              "  if(L'\\0'){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:6]: (warning) Conversion of char literal '\\0' to bool always evaluates to false. [incorrectCharBooleanError]\n"
                      "[test.cpp:3:6]: (warning) Conversion of char literal L'\\0' to bool always evaluates to false. [incorrectCharBooleanError]\n",
                      errout_str());

        check("void f() {\n"
              "  if('\\0' || cond){}\n"
              "  if(L'\\0' || cond){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:6]: (warning) Conversion of char literal '\\0' to bool always evaluates to false. [incorrectCharBooleanError]\n"
                      "[test.cpp:3:6]: (warning) Conversion of char literal L'\\0' to bool always evaluates to false. [incorrectCharBooleanError]\n", errout_str());

        check("void f(bool b);\n" // #9450
              "void f(std::string s);\n"
              "void g() {\n"
              "    f(\"abc\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:7]: (warning) Conversion of string literal \"abc\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("void g(bool);\n"
              "    void f(std::map<std::string, std::vector<int>>&m) {\n"
              "    if (m.count(\"abc\"))\n"
              "        g(m[\"abc\"][0] ? true : false);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(bool b);\n"
              "void f() {\n"
              "    g('\\0');\n"
              "    g('a');\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:7]: (warning) Conversion of char literal '\\0' to bool always evaluates to false. [incorrectCharBooleanError]\n"
                      "[test.cpp:4:7]: (warning) Conversion of char literal 'a' to bool always evaluates to true. [incorrectCharBooleanError]\n",
                      errout_str());

        check("#define ERROR(msg) if (msg) printf(\"%s\\n\", msg);\n"
              "void f() {\n"
              "	  ERROR(\"abc\")\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void g(int, bool);\n"
              "void f() {\n"
              "    MyAssert(!\"abc\");\n"
              "    g(2, !\"def\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:11]: (warning) Conversion of string literal \"def\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("bool f(const char *p) {\n"
              "    if (*p == '\\0')\n"
              "        return *p == '\\0';\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const int* p, const int* q) {\n"
              "    assert((p != NULL && q != NULL) || !\"abc\");\n"
              "    ASSERT((void*)(\"def\") == 0);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("class C {\n" // #6109
              "    void check(const char code[], bool validate = true, const char* filename = \"test.cpp\");\n"
              "    void f() {\n"
              "        check(\"class A<B&, C>;\", \"test.C\");\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("[test.cpp:4:34]: (warning) Conversion of string literal \"test.C\" to bool always evaluates to true. [incorrectStringBooleanError]\n", errout_str());

        check("#define MACRO(C) if(!(C)) { error(__FILE__, __LINE__, __FUNCTION__, #C); return; }\n" // #13067
              "void f() {\n"
              "    MACRO(false && \"abc\");\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("#define strequ(s1,s2) ((void *)s1 && (void *)s2 && strcmp(s1, s2) == 0)\n" // #13093
              "void f(const char* p) {\n"
              "    if (strequ(p, \"ALL\")) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string_view);\n"
              "void f(bool);\n"
              "void g() {\n"
              "    f(\"\"sv);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void deadStrcmp() {
        check("void f(const char *str) {\n"
              "  if (strcmp(str, \"abc\") == 0 || strcmp(str, \"def\")) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:40]: (warning) The expression 'strcmp(str,\"def\") != 0' is suspicious. It overlaps 'strcmp(str,\"abc\") == 0'. [overlappingStrcmp]\n", errout_str());

        check("void f(const wchar_t *str) {\n"
              "  if (wcscmp(str, L\"abc\") == 0 || wcscmp(str, L\"def\")) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:41]: (warning) The expression 'wcscmp(str,L\"def\") != 0' is suspicious. It overlaps 'wcscmp(str,L\"abc\") == 0'. [overlappingStrcmp]\n", errout_str());

        check("struct X {\n"
              "  char *str;\n"
              "};\n"
              "\n"
              "void f(const struct X *x) {\n"
              "  if (strcmp(x->str, \"abc\") == 0 || strcmp(x->str, \"def\")) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:43]: (warning) The expression 'strcmp(x->str,\"def\") != 0' is suspicious. It overlaps 'strcmp(x->str,\"abc\") == 0'. [overlappingStrcmp]\n", errout_str());
    }
};

REGISTER_TEST(TestString)
