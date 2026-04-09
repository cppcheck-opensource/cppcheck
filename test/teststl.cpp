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

#include "checkstl.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"
#include "standards.h"
#include "utils.h"

#include <cstddef>
#include <string>

class TestStl : public TestFixture {
public:
    TestStl() : TestFixture("TestStl") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::warning).severity(Severity::style).severity(Severity::performance).library("std.cfg").build();
    const Settings settings_i = settingsBuilder(settings).certainty(Certainty::inconclusive).build();
    const Settings settingsCpp03 = settingsBuilder(settings).cpp(Standards::CPP03).build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(outOfBounds);
        TEST_CASE(outOfBoundsSymbolic);
        TEST_CASE(outOfBoundsIndexExpression);
        TEST_CASE(outOfBoundsIterator);

        TEST_CASE(iterator1);
        TEST_CASE(iterator2);
        TEST_CASE(iterator3);
        TEST_CASE(iterator4);
        TEST_CASE(iterator5);
        TEST_CASE(iterator6);
        TEST_CASE(iterator7);
        TEST_CASE(iterator8);
        TEST_CASE(iterator9);
        TEST_CASE(iterator10);
        TEST_CASE(iterator11);
        TEST_CASE(iterator12);
        TEST_CASE(iterator13);
        TEST_CASE(iterator14); // #8191
        TEST_CASE(iterator15); // #8341
        TEST_CASE(iterator16);
        TEST_CASE(iterator17);
        TEST_CASE(iterator18);
        TEST_CASE(iterator19);
        TEST_CASE(iterator20);
        TEST_CASE(iterator21);
        TEST_CASE(iterator22);
        TEST_CASE(iterator23);
        TEST_CASE(iterator24);
        TEST_CASE(iterator25); // #9742
        TEST_CASE(iterator26); // #9176
        TEST_CASE(iterator27); // #10378
        TEST_CASE(iterator28); // #10450
        TEST_CASE(iterator29);
        TEST_CASE(iterator30);
        TEST_CASE(iterator31);
        TEST_CASE(iteratorExpression);
        TEST_CASE(iteratorSameExpression);
        TEST_CASE(mismatchingContainerIterator);
        TEST_CASE(eraseIteratorOutOfBounds);

        TEST_CASE(dereference);
        TEST_CASE(dereference_break);  // #3644 - handle "break"
        TEST_CASE(dereference_member);

        TEST_CASE(STLSize);
        TEST_CASE(STLSizeNoErr);
        TEST_CASE(negativeIndex);
        TEST_CASE(negativeIndexMultiline);
        TEST_CASE(erase1);
        TEST_CASE(erase2);
        TEST_CASE(erase3);
        TEST_CASE(erase4);
        TEST_CASE(erase5);
        TEST_CASE(erase6);
        TEST_CASE(eraseBreak);
        TEST_CASE(eraseContinue);
        TEST_CASE(eraseReturn1);
        TEST_CASE(eraseReturn2);
        TEST_CASE(eraseReturn3);
        TEST_CASE(eraseGoto);
        TEST_CASE(eraseAssign1);
        TEST_CASE(eraseAssign2);
        TEST_CASE(eraseAssign3);
        TEST_CASE(eraseAssign4);
        TEST_CASE(eraseAssignByFunctionCall);
        TEST_CASE(eraseErase);
        TEST_CASE(eraseByValue);
        TEST_CASE(eraseIf);
        TEST_CASE(eraseOnVector);

        TEST_CASE(pushback1);
        TEST_CASE(pushback2);
        TEST_CASE(pushback3);
        TEST_CASE(pushback4);
        TEST_CASE(pushback5);
        TEST_CASE(pushback6);
        TEST_CASE(pushback7);
        TEST_CASE(pushback8);
        TEST_CASE(pushback9);
        TEST_CASE(pushback10);
        TEST_CASE(pushback11);
        TEST_CASE(pushback12);
        TEST_CASE(pushback13);
        TEST_CASE(insert1);
        TEST_CASE(insert2);
        TEST_CASE(popback1);

        TEST_CASE(stlBoundaries1);
        TEST_CASE(stlBoundaries2);
        TEST_CASE(stlBoundaries3);
        TEST_CASE(stlBoundaries4); // #4364
        TEST_CASE(stlBoundaries5); // #4352
        TEST_CASE(stlBoundaries6); // #7106

        // if (str.find("ab"))
        TEST_CASE(if_find);
        TEST_CASE(if_str_find);

        TEST_CASE(size1);
        TEST_CASE(size2);
        TEST_CASE(size3);
        TEST_CASE(size4); // #2652 - don't warn about vector/deque

        // Redundant conditions..
        // if (ints.find(123) != ints.end()) ints.remove(123);
        TEST_CASE(redundantCondition1);

        // missing inner comparison when incrementing iterator inside loop
        TEST_CASE(missingInnerComparison1);
        TEST_CASE(missingInnerComparison2);     // no FP when there is comparison
        TEST_CASE(missingInnerComparison3);     // no FP when there is iterator shadowing
        TEST_CASE(missingInnerComparison4);     // no FP when "break;" is used
        TEST_CASE(missingInnerComparison5);     // Ticket #2154 - FP
        TEST_CASE(missingInnerComparison6);     // #2643 - 'it=foo.insert(++it,0);'

        // catch common problems when using the string::c_str() function
        TEST_CASE(cstr);

        TEST_CASE(uselessCalls);
        TEST_CASE(stabilityOfChecks); // #4684 cppcheck crash in template function call

        TEST_CASE(dereferenceInvalidIterator);
        TEST_CASE(dereferenceInvalidIterator2); // #6572
        TEST_CASE(dereference_auto);

        TEST_CASE(loopAlgoElementAssign);
        TEST_CASE(loopAlgoAccumulateAssign);
        TEST_CASE(loopAlgoContainerInsert);
        TEST_CASE(loopAlgoIncrement);
        TEST_CASE(loopAlgoConditional);
        TEST_CASE(loopAlgoMinMax);
        TEST_CASE(loopAlgoMultipleReturn);

        TEST_CASE(invalidContainer);
        TEST_CASE(invalidContainerLoop);
        TEST_CASE(findInsert);

        TEST_CASE(checkKnownEmptyContainer);
        TEST_CASE(checkMutexes);
    }

    struct CheckOptions
    {
        bool inconclusive = false;
    };

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const CheckOptions& options = make_default_obj()) {
        const Settings& s = options.inconclusive ? settings_i : settings;

        check_(file, line, code, s);
    }

    template<size_t size>
    void check_(const char* file, int line, const char (&code)[size], const Settings& s) {
        SimpleTokenizer tokenizer(s, *this);

        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        runChecks<CheckStl>(tokenizer, this);
    }

    // TODO: get rid of this
    void check_(const char* file, int line, const std::string& code) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);

        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        runChecks<CheckStl>(tokenizer, this);
    }

#define checkNormal(...) checkNormal_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkNormal_(const char* file, int line, const char (&code)[size]) {
        // Tokenize..
        SimpleTokenizer tokenizer(settings, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // Check..
        runChecks<CheckStl>(tokenizer, this);
    }

    void outOfBounds() {
        setMultiline();

        checkNormal("bool f(const int a, const int b)\n" // #8648
                    "{\n"
                    "    std::cout << a << b;\n"
                    "    return true;\n"
                    "}\n"
                    "void f(const std::vector<int> &v)\n"
                    "{\n"
                    "    if(v.size() >=2 &&\n"
                    "            bar(v[2], v[3]) )\n"                          // v[3] is accessed
                    "    {;}\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:9:18]: warning: Either the condition 'v.size()>=2' is redundant or size of 'v' can be 2. Expression 'v[2]' causes access out of bounds. [containerOutOfBounds]\n"
                      "[test.cpp:8:17]: note: condition 'v.size()>=2'\n"
                      "[test.cpp:9:18]: note: Access out of bounds\n"
                      "[test.cpp:9:24]: warning: Either the condition 'v.size()>=2' is redundant or size of 'v' can be 2. Expression 'v[3]' causes access out of bounds. [containerOutOfBounds]\n"
                      "[test.cpp:8:17]: note: condition 'v.size()>=2'\n"
                      "[test.cpp:9:24]: note: Access out of bounds\n", errout_str());

        checkNormal("void f() {\n"
                    "  std::string s;\n"
                    "  s[10] = 1;\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:3:4]: error: Out of bounds access in expression 's[10]' because 's' is empty. [containerOutOfBounds]\n", errout_str());

        checkNormal("void f() {\n"
                    "  std::string s = \"abcd\";\n"
                    "  s[10] = 1;\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:3:4]: error: Out of bounds access in 's[10]', if 's' size is 4 and '10' is 10 [containerOutOfBounds]\n", errout_str());

        checkNormal("void f(std::vector<int> v) {\n"
                    "    v.front();\n"
                    "    if (v.empty()) {}\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:2:12]: warning: Either the condition 'v.empty()' is redundant or expression 'v.front()' causes access out of bounds. [containerOutOfBounds]\n"
                      "[test.cpp:3:16]: note: condition 'v.empty()'\n"
                      "[test.cpp:2:12]: note: Access out of bounds\n", errout_str());

        checkNormal("void f(std::vector<int> v) {\n"
                    "    if (v.size() == 3) {}\n"
                    "    v[16] = 0;\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:3:6]: warning: Either the condition 'v.size()==3' is redundant or size of 'v' can be 3. Expression 'v[16]' causes access out of bounds. [containerOutOfBounds]\n"
                      "[test.cpp:2:18]: note: condition 'v.size()==3'\n"
                      "[test.cpp:3:6]: note: Access out of bounds\n", errout_str());

        checkNormal("void f(std::vector<int> v) {\n"
                    "    int i = 16;\n"
                    "    if (v.size() == 3) {\n"
                    "        v[i] = 0;\n"
                    "    }\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:4:10]: warning: Either the condition 'v.size()==3' is redundant or size of 'v' can be 3. Expression 'v[i]' causes access out of bounds. [containerOutOfBounds]\n"
                      "[test.cpp:3:18]: note: condition 'v.size()==3'\n"
                      "[test.cpp:4:10]: note: Access out of bounds\n", errout_str());

        checkNormal("void f(std::vector<int> v, int i) {\n"
                    "    if (v.size() == 3 || i == 16) {}\n"
                    "    v[i] = 0;\n"
                    "}");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(std::map<int,int> x) {\n"
                    "    if (x.empty()) { x[1] = 2; }\n"
                    "}");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(std::string s) {\n"
                    "    if (s.size() == 1) {\n"
                    "        s[2] = 0;\n"
                    "    }\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:3:10]: warning: Either the condition 's.size()==1' is redundant or size of 's' can be 1. Expression 's[2]' causes access out of bounds. [containerOutOfBounds]\n"
                      "[test.cpp:2:18]: note: condition 's.size()==1'\n"
                      "[test.cpp:3:10]: note: Access out of bounds\n", errout_str());

        // Do not crash
        checkNormal("void a() {\n"
                    "  std::string b[];\n"
                    "  for (auto c : b)\n"
                    "    c.data();\n"
                    "}");
        ASSERT_EQUALS("", errout_str());

        checkNormal("std::string f(std::string x) {\n"
                    "  if (x.empty()) return {};\n"
                    "  x[0];\n"
                    "}");
        ASSERT_EQUALS("", errout_str());

        checkNormal("std::string f(std::string x) {\n"
                    "  if (x.empty()) return std::string{};\n"
                    "  x[0];\n"
                    "}");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f() {\n"
                    "  std::string s;\n"
                    "  x = s.begin() + 1;\n"
                    "}");
        ASSERT_EQUALS(
            "[test.cpp:3:17]: error: Out of bounds access in expression 's.begin()+1' because 's' is empty. [containerOutOfBounds]\n"
            "[test.cpp:3:17]: error: Out of bounds access in expression 's.begin()+1' because 's' is empty. [containerOutOfBounds]\n",   // duplicate
            errout_str());

        checkNormal("void f(int x) {\n"
                    "  std::string s;\n"
                    "  auto it = s.begin() + x;\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:3:23]: error: Out of bounds access in expression 's.begin()+x' because 's' is empty and 'x' may be non-zero. [containerOutOfBounds]\n", errout_str());

        checkNormal("char fstr1(){const std::string s = \"<a><b>\"; return s[42]; }\n"
                    "wchar_t fwstr1(){const std::wstring s = L\"<a><b>\"; return s[42]; }");
        ASSERT_EQUALS("[test.cpp:1:54]: error: Out of bounds access in 's[42]', if 's' size is 6 and '42' is 42 [containerOutOfBounds]\n"
                      "[test.cpp:2:60]: error: Out of bounds access in 's[42]', if 's' size is 6 and '42' is 42 [containerOutOfBounds]\n", errout_str());

        checkNormal("char fstr1(){const std::string s = \"<a><b>\"; return s[1]; }\n"
                    "wchar_t fwstr1(){const std::wstring s = L\"<a><b>\"; return s[1]; }");
        ASSERT_EQUALS("", errout_str());

        checkNormal("int f() {\n"
                    "    std::vector<int> v;\n"
                    "    std::vector<int> * pv = &v;\n"
                    "    return (*pv)[42];\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:4:17]: error: Out of bounds access in expression '(*pv)[42]' because '*pv' is empty. [containerOutOfBounds]\n", errout_str());

        checkNormal("void f() {\n"
                    "  std::string s;\n"
                    "  ++abc[s];\n"
                    "}");
        ASSERT_EQUALS("", errout_str());

        // # 9274
        checkNormal("char f(bool b) {\n"
                    "    const std::string s = \"<a><b>\";\n"
                    "    int x = 6;\n"
                    "    if(b) ++x;\n"
                    "    return s[x];\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:5:13]: error: Out of bounds access in 's[x]', if 's' size is 6 and 'x' is 6 [containerOutOfBounds]\n", errout_str());

        checkNormal("void f() {\n"
                    "    static const int N = 4;\n"
                    "    std::array<int, N> x;\n"
                    "    x[0] = 0;\n"
                    "}");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(bool b) {\n"
                    "    std::vector<int> x;\n"
                    "    if (b)\n"
                    "        x.push_back(1);\n"
                    "    if (x.size() < 2)\n"
                    "        return;\n"
                    "    x[0] = 2;\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(bool b) {\n"
                    "    std::vector<int> v;\n"
                    "    if(v.at(b?42:0)) {}\n"
                    "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:12]: error: Out of bounds access in expression 'v.at(b?42:0)' because 'v' is empty. [containerOutOfBounds]\n",
            errout_str());

        checkNormal("void f(std::vector<int> v, bool b){\n"
                    "    if (v.size() == 1)\n"
                    "        if(v.at(b?42:0)) {}\n"
                    "}\n");
        ASSERT_EQUALS(
            "[test.cpp:3:16]: warning: Either the condition 'v.size()==1' is redundant or size of 'v' can be 1. Expression 'v.at(b?42:0)' causes access out of bounds. [containerOutOfBounds]\n"
            "[test.cpp:2:18]: note: condition 'v.size()==1'\n"
            "[test.cpp:3:16]: note: Access out of bounds\n",
            errout_str());

        check("struct T {\n"
              "  std::vector<int>* v;\n"
              "};\n"
              "struct S {\n"
              "  T t;\n"
              "};\n"
              "long g(S& s);\n"
              "int f() {\n"
              "  std::vector<int> ArrS;\n"
              "  S s = { { &ArrS } };\n"
              "  g(s);\n"
              "  return ArrS[0];\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n"
              "  std::vector<int>* v;\n"
              "};\n"
              "struct S {\n"
              "  std::vector<T> t;\n"
              "};\n"
              "long g(S& s);\n"
              "int f() {\n"
              "  std::vector<int> ArrS;\n"
              "  S s = { { { &ArrS } } };\n"
              "  g(s);\n"
              "  return ArrS[0];\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n"
              "  std::vector<int>* v;\n"
              "};\n"
              "struct S {\n"
              "  std::vector<std::vector<T>> t;\n"
              "};\n"
              "long g(S& s);\n"
              "int f() {\n"
              "  std::vector<int> ArrS;\n"
              "  S s = { { { { &ArrS } } } };\n"
              "  g(s);\n"
              "  return ArrS[0];\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n"
              "  std::vector<int>* v;\n"
              "};\n"
              "struct S {\n"
              "  T t;\n"
              "};\n"
              "long g(S& s);\n"
              "int f() {\n"
              "  std::vector<int> ArrS;\n"
              "  S s { { &ArrS } };\n"
              "  g(s);\n"
              "  return ArrS[0];\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n"
              "  std::vector<int>* v;\n"
              "};\n"
              "struct S {\n"
              "  std::vector<T> t;\n"
              "};\n"
              "long g(S& s);\n"
              "int f() {\n"
              "  std::vector<int> ArrS;\n"
              "  S s { { { &ArrS } } };\n"
              "  g(s);\n"
              "  return ArrS[0];\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n"
              "  std::vector<int>* v;\n"
              "};\n"
              "struct S {\n"
              "  std::vector<std::vector<T>> t;\n"
              "};\n"
              "long g(S& s);\n"
              "int f() {\n"
              "  std::vector<int> ArrS;\n"
              "  S s { { { { &ArrS } } } };\n"
              "  g(s);\n"
              "  return ArrS[0];\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        checkNormal("extern void Bar(const double, const double);\n"
                    "void f(std::vector<double> &r, const double ) {\n"
                    "    std::vector<double> result;\n"
                    "    double d = 0.0;\n"
                    "    const double inc = 0.1;\n"
                    "    for(unsigned int i = 0; i < 10; ++i) {\n"
                    "        result.push_back(d);\n"
                    "        d = (i + 1) * inc;\n"
                    "    }\n"
                    "    Bar(1.0, d);\n"
                    "    Bar(10U, result.size());\n"
                    "    Bar(0.0, result[0]);\n"
                    "    Bar(0.34, result[1]);\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(size_t entries) {\n"
                    "    std::vector<uint8_t> v;\n"
                    "    if (v.size() < entries + 2)\n"
                    "        v.resize(entries + 2);\n"
                    "    v[0] = 1;\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(size_t entries) {\n"
                    "    std::vector<uint8_t> v;\n"
                    "    if (v.size() < entries)\n"
                    "        v.resize(entries);\n"
                    "    v[0] = 1;\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:5:6]: error: Out of bounds access in expression 'v[0]' because 'v' is empty. [containerOutOfBounds]\n", errout_str());

        checkNormal("void f(size_t entries) {\n"
                    "    if (entries < 2) return;\n"
                    "    std::vector<uint8_t> v;\n"
                    "    if (v.size() < entries)\n"
                    "        v.resize(entries);\n"
                    "    v[0] = 1;\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(size_t entries) {\n"
                    "    if (entries == 0) return;\n"
                    "    std::vector<uint8_t> v;\n"
                    "    if (v.size() < entries)\n"
                    "        v.resize(entries);\n"
                    "    v[0] = 1;\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void foo(std::vector<int>* PArr, int n) {\n"
                    " std::vector<int> Arr;\n"
                    " if (!PArr)\n"
                    "   PArr = &Arr;\n"
                    " PArr->resize(n);\n"
                    " for (int i = 0; i < n; ++i)\n"
                    "   (*PArr)[i] = 1;\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("int f() {\n"
                    "    std::vector<int> v;\n"
                    "    std::vector<int> * pv = &v;\n"
                    "    return (*pv).at(42);\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:4:20]: error: Out of bounds access in expression '(*pv).at(42)' because '*pv' is empty. [containerOutOfBounds]\n",
                      errout_str());

        checkNormal("std::string f(const char* DirName) {\n"
                    "  if (DirName == nullptr)\n"
                    "      return {};\n"
                    "  std::string Name{ DirName };\n"
                    "  if (!Name.empty() && Name.back() != '\\\\')\n"
                    "    Name += '\\\\';\n"
                    "  return Name;\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("bool f(bool b) {\n"
                    "  std::vector<int> v;\n"
                    "  if (b)\n"
                    "    v.push_back(0);\n"
                    "  for(auto i:v)\n"
                    "    if (v[i] > 0)\n"
                    "      return true;\n"
                    "  return false;\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:5:3]: style: Consider using std::any_of algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        checkNormal("std::vector<int> range(int n);\n"
                    "bool f(bool b) {\n"
                    "  std::vector<int> v;\n"
                    "  if (b)\n"
                    "    v.push_back(1);\n"
                    "  assert(range(v.size()).size() == v.size());\n"
                    "  for(auto i:range(v.size()))\n"
                    "    if (v[i] > 0)\n"
                    "      return true;\n"
                    "  return false;\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:7:3]: style: Consider using std::any_of algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        checkNormal("bool g();\n"
                    "int f(int x) {\n"
                    "    std::vector<int> v;\n"
                    "    if (g())\n"
                    "        v.emplace_back(x);\n"
                    "    const auto n = (int)v.size();\n"
                    "    for (int i = 0; i < n; ++i)\n"
                    "        if (v[i] > 0)\n"
                    "            return i;\n"
                    "    return 0;\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:8:0]: style: Consider using std::find_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        checkNormal("bool g();\n"
                    "int f(int x) {\n"
                    "    std::vector<int> v;\n"
                    "    if (g())\n"
                    "        v.emplace_back(x);\n"
                    "    const auto n = static_cast<int>(v.size());\n"
                    "    for (int i = 0; i < n; ++i)\n"
                    "        if (v[i] > 0)\n"
                    "            return i;\n"
                    "    return 0;\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:8:0]: style: Consider using std::find_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        checkNormal("bool g();\n"
                    "void f(int x) {\n"
                    "    std::vector<int> v;\n"
                    "    if (g())\n"
                    "        v.emplace_back(x);\n"
                    "    const int n = v.size();\n"
                    "    h(n);\n"
                    "    for (int i = 0; i < n; ++i)\n"
                    "        h(v[i]);\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void foo(const std::vector<int> &v) {\n"
                    "    if(v.size() >=1 && v[0] == 4 && v[1] == 2){}\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:2:38]: warning: Either the condition 'v.size()>=1' is redundant or size of 'v' can be 1. Expression 'v[1]' causes access out of bounds. [containerOutOfBounds]\n"
                      "[test.cpp:2:17]: note: condition 'v.size()>=1'\n"
                      "[test.cpp:2:38]: note: Access out of bounds\n", errout_str());

        checkNormal("int f(int x, int y) {\n"
                    "    std::vector<int> a = {0,1,2};\n"
                    "    if(x<2)\n"
                    "        y = a[x] + 1;\n"
                    "    else\n"
                    "        y = a[x];\n"
                    "    return y;\n"
                    "}\n");
        ASSERT_EQUALS(
            "[test.cpp:6:14]: warning: Either the condition 'x<2' is redundant or 'x' can have the value greater or equal to 3. Expression 'a[x]' causes access out of bounds. [containerOutOfBounds]\n"
            "[test.cpp:3:9]: note: condition 'x<2'\n"
            "[test.cpp:6:14]: note: Access out of bounds\n",
            errout_str());

        checkNormal("int f(std::vector<int> v) {\n"
                    "    if (v.size() > 3)\n"
                    "        return v[v.size() - 3];\n"
                    "    return 0;\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(std::vector<int> v) {\n"
                    "    v[v.size() - 1];\n"
                    "    if (v.size() == 1) {}\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(int n) {\n"
                    "    std::vector<int> v = {1, 2, 3, 4};\n"
                    "    const int i = qMin(n, v.size());\n"
                    "    if (i > 1)\n"
                    "        v[i] = 1;\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(std::vector<int>& v, int i) {\n"
                    "    if (i > -1) {\n"
                    "        v.erase(v.begin() + i);\n"
                    "        if (v.empty()) {}\n"
                    "    }\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void g(const char *, ...) { exit(1); }\n" // #10025
                    "void f(const char c[]) {\n"
                    "    std::vector<int> v = get();\n"
                    "    if (v.empty())\n"
                    "        g(\"\", c[0]);\n"
                    "    return h(&v[0], v.size()); \n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(int i, std::vector<int> v) {\n" // #9157
                    "    if (i <= (int)v.size()) {\n"
                    "        if (v[i]) {}\n"
                    "    }\n"
                    "    if (i <= static_cast<int>(v.size())) {\n"
                    "        if (v[i]) {}\n"
                    "    }\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:3:14]: warning: Either the condition 'i<=(int)v.size()' is redundant or 'i' can have the value v.size(). Expression 'v[i]' causes access out of bounds. [containerOutOfBounds]\n"
                      "[test.cpp:2:11]: note: condition 'i<=(int)v.size()'\n"
                      "[test.cpp:3:14]: note: Access out of bounds\n"
                      "[test.cpp:6:14]: warning: Either the condition 'i<=static_cast<int>(v.size())' is redundant or 'i' can have the value v.size(). Expression 'v[i]' causes access out of bounds. [containerOutOfBounds]\n"
                      "[test.cpp:5:11]: note: condition 'i<=static_cast<int>(v.size())'\n"
                      "[test.cpp:6:14]: note: Access out of bounds\n",
                      errout_str());

        check("template<class Iterator>\n"
              "void b(Iterator d) {\n"
              "  std::string c = \"a\";\n"
              "  d + c.length();\n"
              "}\n"
              "void f() {\n"
              "  std::string buf;\n"
              "  b(buf.begin());\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:5]: error: Out of bounds access in expression 'd+c.length()' because 'buf' is empty. [containerOutOfBounds]\n",
                      errout_str());

        check("template<class Iterator>\n"
              "void b(Iterator d) {\n"
              "  std::string c = \"a\";\n"
              "  sort(d, d + c.length());\n"
              "}\n"
              "void f() {\n"
              "  std::string buf;\n"
              "  b(buf.begin());\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("int f(const std::vector<int> &v) {\n"
              "    return !v.empty() ? 42 : v.back();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:2:36]: warning: Either the condition 'v.empty()' is redundant or expression 'v.back()' causes access out of bounds. [containerOutOfBounds]\n"
            "[test.cpp:2:20]: note: condition 'v.empty()'\n"
            "[test.cpp:2:36]: note: Access out of bounds\n",
            errout_str());

        check("std::vector<int> g() {\n" // #10779
              "    std::vector<int> v(10);\n"
              "    for(int i = 0; i <= 10; ++i)\n"
              "        v[i] = 42;\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:10]: error: Out of bounds access in 'v[i]', if 'v' size is 10 and 'i' is 10 [containerOutOfBounds]\n",
                      errout_str());

        check("void f() {\n"
              "    int s = 2;\n"
              "    std::vector <int> v(s);\n"
              "    v[100] = 1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:6]: error: Out of bounds access in 'v[100]', if 'v' size is 2 and '100' is 100 [containerOutOfBounds]\n",
                      errout_str());

        check("void f() {\n"
              "    std::vector <int> v({ 1, 2, 3 });\n"
              "    v[100] = 1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:6]: error: Out of bounds access in 'v[100]', if 'v' size is 3 and '100' is 100 [containerOutOfBounds]\n",
                      errout_str());

        check("void f() {\n"
              "    char c[] = { 1, 2, 3 };\n"
              "    std::vector<char> v(c, sizeof(c) + c);\n"
              "    v[100] = 1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:6]: error: Out of bounds access in 'v[100]', if 'v' size is 3 and '100' is 100 [containerOutOfBounds]\n",
                      errout_str());

        check("void f() {\n"
              "    char c[] = { 1, 2, 3 };\n"
              "    std::vector<char> v{ c, c + sizeof(c) };\n"
              "    v[100] = 1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:6]: error: Out of bounds access in 'v[100]', if 'v' size is 3 and '100' is 100 [containerOutOfBounds]\n",
                      errout_str());

        check("void f() {\n"
              "    int i[] = { 1, 2, 3 };\n"
              "    std::vector<int> v(i, i + sizeof(i) / 4);\n"
              "    v[100] = 1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:6]: error: Out of bounds access in 'v[100]', if 'v' size is 3 and '100' is 100 [containerOutOfBounds]\n",
                      errout_str());

        check("void f() {\n" // #6615
              "    int i[] = { 1, 2, 3 };\n"
              "    std::vector<int> v(i, i + sizeof(i) / sizeof(int));\n"
              "    v[100] = 1;\n"
              "}\n");
        TODO_ASSERT_EQUALS("test.cpp:4:error:Out of bounds access in 'v[100]', if 'v' size is 3 and '100' is 100\n",
                           "",
                           errout_str());

        check("void f() {\n"
              "    std::array<int, 10> a = {};\n"
              "    a[10];\n"
              "    constexpr std::array<int, 10> b = {};\n"
              "    b[10];\n"
              "    const std::array<int, 10> c = {};\n"
              "    c[10];\n"
              "    static constexpr std::array<int, 10> d = {};\n"
              "    d[10];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:6]: error: Out of bounds access in 'a[10]', if 'a' size is 10 and '10' is 10 [containerOutOfBounds]\n"
                      "[test.cpp:5:6]: error: Out of bounds access in 'b[10]', if 'b' size is 10 and '10' is 10 [containerOutOfBounds]\n"
                      "[test.cpp:7:6]: error: Out of bounds access in 'c[10]', if 'c' size is 10 and '10' is 10 [containerOutOfBounds]\n"
                      "[test.cpp:9:6]: error: Out of bounds access in 'd[10]', if 'd' size is 10 and '10' is 10 [containerOutOfBounds]\n",
                      errout_str());

        check("struct test_fixed {\n"
              "    std::array<int, 10> array = {};\n"
              "    void index(int i) { array[i]; }\n"
              "};\n"
              "void f() {\n"
              "    test_fixed x = test_fixed();\n"
              "    x.index(10);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:30]: error: Out of bounds access in 'array[i]', if 'array' size is 10 and 'i' is 10 [containerOutOfBounds]\n",
                      errout_str());

        check("struct test_constexpr {\n"
              "    static constexpr std::array<int, 10> array = {};\n"
              "    void index(int i) { array[i]; }\n"
              "};\n"
              "void f() {\n"
              "    test_constexpr x = test_constexpr();\n"
              "    x.index(10);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:30]: error: Out of bounds access in 'array[i]', if 'array' size is 10 and 'i' is 10 [containerOutOfBounds]\n",
                      errout_str());

        checkNormal("struct A {\n"
                    "    const std::vector<int>& v;\n"
                    "    A(const std::vector<int>& x) : v(x)\n"
                    "    {}\n"
                    "    int f() const {\n"
                    "        return v[0];\n"
                    "    }\n"
                    "};\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("struct A {\n"
                    "    static const std::vector<int> v;\n"
                    "    int f() const {\n"
                    "        return v[0];\n"
                    "    }\n"
                    "};\n"
                    "const std::vector<int> A::v = {1, 2};\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("struct a {\n"
                    "    std::vector<int> g() const;\n"
                    "};\n"
                    "int f(const a& b) {\n"
                    "    auto c = b.g();\n"
                    "    assert(not c.empty());\n"
                    "    int d = c.front();\n"
                    "    return d;\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("std::string f() {\n"
                    "    std::map<int, std::string> m = { { 1, \"1\" } };\n"
                    "    return m.at(1);\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("struct A {\n"
                    "  virtual void init_v(std::vector<int> *v) = 0;\n"
                    "};\n"
                    "A* create_a();\n"
                    "struct B {\n"
                    "  B() : a(create_a()) {}\n"
                    "  void init_v(std::vector<int> *v) {\n"
                    "    a->init_v(v);\n"
                    "  }\n"
                    "  A* a;\n"
                    "};\n"
                    "void f() {\n"
                    "  B b;\n"
                    "  std::vector<int> v;\n"
                    "  b.init_v(&v);\n"
                    "  v[0];\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("void f(std::vector<int>* v) {\n"
                    "  if (v->empty())\n"
                    "    v->push_back(1);\n"
                    "  auto x = v->back();\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("template <typename T, uint8_t count>\n"
                    "struct Foo {\n"
                    "    std::array<T, count> items = {0};\n"
                    "    T maxCount = count;\n"
                    "    explicit Foo(const T& maxValue = (std::numeric_limits<T>::max)()) : maxCount(maxValue) {}\n"
                    "    bool Set(const uint8_t idx) {\n"
                    "        if (CheckBounds(idx) && items[idx] < maxCount) {\n"
                    "            items[idx] += 1;\n"
                    "            return true;\n"
                    "        }\n"
                    "        return false;\n"
                    "    }\n"
                    "    static bool CheckBounds(const uint8_t idx) { return idx < count; }\n"
                    "};\n"
                    "void f() {\n"
                    "    Foo<uint8_t, 42U> x;\n"
                    "    if (x.Set(42U)) {}\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("struct S { void g(std::span<int>& r) const; };\n" // #11828
                    "int f(const S& s) {\n"
                    "    std::span<int> t;\n"
                    "    s.g(t);\n"
                    "    return t[0];\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("char h() {\n"
                    "    std::string s;\n"
                    "    std::string_view sv(s);\n"
                    "    return s[2];\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:4:13]: error: Out of bounds access in expression 's[2]' because 's' is empty. [containerOutOfBounds]\n", errout_str());

        checkNormal("void f() {\n" // #12738
                    "    std::vector<double> v{ 0, 0.1 };\n"
                    "    (void)v[0];\n"
                    "}\n");
        ASSERT_EQUALS("", errout_str());

        checkNormal("int f(const std::vector<int>& v) {\n" // #8983
                    "    return v[2];\n"
                    "}\n"
                    "int g() {\n"
                    "    return f({});\n"
                    "}\n"
                    "int h() {\n"
                    "    return f({ 1, 2 });\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:2:13]: error: Out of bounds access in 'v[2]', if 'v' size is 2 and '2' is 2 [containerOutOfBounds]\n"
                      "[test.cpp:2:13]: error: Out of bounds access in expression 'v[2]' because 'v' is empty. [containerOutOfBounds]\n",
                      errout_str());

        checkNormal("int f(int x, const std::vector<int>& v) {\n"
                    "    return x + v[0];\n"
                    "}\n"
                    "int g() {\n"
                    "    return f(1, {});\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:2:17]: error: Out of bounds access in expression 'v[0]' because 'v' is empty. [containerOutOfBounds]\n",
                      errout_str());

        checkNormal("bool f(const std::string_view s) { return s[500] == 'x'; }\n" // #12046
                    "bool g() { return f(\" \"); }\n");
        ASSERT_EQUALS("[test.cpp:1:44]: error: Out of bounds access in 's[500]', if 's' size is 1 and '500' is 500 [containerOutOfBounds]\n",
                      errout_str());

        checkNormal("int main() {\n" // #14342
                    "    const int a[] = { 1, 2, 3 };\n"
                    "    std::span<const int> x{ a };\n"
                    "    return x[3];\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:4:13]: error: Out of bounds access in 'x[3]', if 'x' size is 1 and '3' is 3 [containerOutOfBounds]\n",
                      errout_str());

        checkNormal("int main() {\n"
                    "    const char a[] = \"abc\";\n"
                    "    std::string_view x{ a };\n"
                    "    return x[5];\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:4:13]: error: Out of bounds access in 'x[5]', if 'x' size is 4 and '5' is 5 [containerOutOfBounds]\n",
                      errout_str());

        checkNormal("int f(const std::string& v) {\n"
                    "    return v[2];\n"
                    "}\n"
                    "int main() {\n"
                    "    std::string_view x{ \"a\" };\n"
                    "    return f(std::string(x));\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:2:13]: error: Out of bounds access in 'v[2]', if 'v' size is 1 and '2' is 2 [containerOutOfBounds]\n",
                      errout_str());
    }

    void outOfBoundsSymbolic()
    {
        check("void foo(std::string textline, int col) {\n"
              "    if(col > textline.size())\n"
              "        return false;\n"
              "    int x = textline[col];\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:2:12] -> [test.cpp:4:21]: (warning) Either the condition 'col>textline.size()' is redundant or 'col' can have the value textline.size(). Expression 'textline[col]' causes access out of bounds. [containerOutOfBounds]\n",
            errout_str());

        check("void f(const std::vector<int>& v) {\n" // #12742
              "    for (unsigned i = 0; i < v.size();)\n"
              "        (void)v[i++];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void outOfBoundsIndexExpression() {
        setMultiline();

        checkNormal("void f(std::string s) {\n"
                    "  s[s.size()] = 1;\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:2:3]: error: Out of bounds access of s, index 's.size()' is out of bounds. [containerOutOfBoundsIndexExpression]\n", errout_str());

        checkNormal("void f(std::string s) {\n"
                    "  s[s.size()+1] = 1;\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:2:3]: error: Out of bounds access of s, index 's.size()+1' is out of bounds. [containerOutOfBoundsIndexExpression]\n", errout_str());

        checkNormal("void f(std::string s) {\n"
                    "  s[1+s.size()] = 1;\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:2:3]: error: Out of bounds access of s, index '1+s.size()' is out of bounds. [containerOutOfBoundsIndexExpression]\n", errout_str());

        checkNormal("void f(std::string s) {\n"
                    "  s[x*s.size()] = 1;\n"
                    "}");
        ASSERT_EQUALS("[test.cpp:2:3]: error: Out of bounds access of s, index 'x*s.size()' is out of bounds. [containerOutOfBoundsIndexExpression]\n", errout_str());

        checkNormal("bool f(std::string_view& sv) {\n" // #10031
                    "    return sv[sv.size()] == '\\0';\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:2:12]: error: Out of bounds access of sv, index 'sv.size()' is out of bounds. [containerOutOfBoundsIndexExpression]\n", errout_str());
    }
    void outOfBoundsIterator() {
        check("int f() {\n"
              "    std::vector<int> v;\n"
              "    auto it = v.begin();\n"
              "    return *it;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:13]: (error) Out of bounds access in expression 'it' because 'v' is empty. [containerOutOfBounds]\n",
                      errout_str());

        check("int f() {\n"
              "    std::vector<int> v;\n"
              "    v.push_back(0);\n"
              "    auto it = v.begin() + 1;\n"
              "    return *it;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Out of bounds access in 'it', if 'v' size is 1 and 'it' is at position 1 from the beginning [containerOutOfBounds]\n",
                      errout_str());

        check("int f() {\n"
              "    std::vector<int> v;\n"
              "    v.push_back(0);\n"
              "    auto it = v.end() - 1;\n"
              "    return *it;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f() {\n"
              "    std::vector<int> v;\n"
              "    v.push_back(0);\n"
              "    auto it = v.end() - 2;\n"
              "    return *it;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:13]: (error) Out of bounds access in 'it', if 'v' size is 1 and 'it' is at position 2 from the end [containerOutOfBounds]\n",
                      errout_str());

        check("void g(int);\n"
              "void f(std::vector<int> x) {\n"
              "    std::map<int, int> m;\n"
              "    if (!m.empty()) {\n"
              "        g(m.begin()->second);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::vector<int> vec;\n"
              "    std::vector<int>::iterator it = vec.begin();\n"
              "    *it = 1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Out of bounds access in expression 'it' because 'vec' is empty. [containerOutOfBounds]\n",
                      errout_str());

        check("void f() {\n"
              "    std::vector<int> vec;\n"
              "    auto it = vec.begin();\n"
              "    *it = 1;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Out of bounds access in expression 'it' because 'vec' is empty. [containerOutOfBounds]\n",
                      errout_str());
    }

    void iterator1() {
        check("void f()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    for (std::list<int>::iterator it = l1.begin(); it != l2.end(); ++it)\n"
              "    { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:40]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void f()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    for (std::list<int>::iterator it = l1.begin(); l2.end() != it; ++it)\n"
              "    { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:52]: (error) Iterators of different containers 'l2' and 'l1' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("struct C { std::list<int> l1; void func(); };\n"
              "void C::func() {\n"
              "    std::list<int>::iterator it;\n"
              "    for (it = l1.begin(); it != l1.end(); ++it) { }\n"
              "    C c;\n"
              "    for (it = c.l1.begin(); it != c.l1.end(); ++it) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Same check with reverse iterator
        check("void f()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    for (std::list<int>::const_reverse_iterator it = l1.rbegin(); it != l2.rend(); ++it)\n"
              "    { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:54]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n",
                      errout_str());
    }

    void iterator2() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it = l1.begin();\n"
              "    while (it != l2.end())\n"
              "    {\n"
              "        ++it;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:35]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it = l1.begin();\n"
              "    while (l2.end() != it)\n"
              "    {\n"
              "        ++it;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:12]: (error) Iterators of different containers 'l2' and 'l1' are used together. [mismatchingContainers]\n",
                      errout_str());
    }

    void iterator3() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it = l1.begin();\n"
              "    l2.insert(it, 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Same iterator is used with different containers 'l1' and 'l2'. [iterators1]\n"
                      "[test.cpp:6:5]: (error) Iterator 'it' referring to container 'l1' is used with container 'l2'. [mismatchingContainerIterator]\n",
                      errout_str());

        check("void foo() {\n" // #5803
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it = l1.begin();\n"
              "    l2.insert(it, l1.end());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n" // #7658
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it = l1.begin();\n"
              "    std::list<int>::iterator end = l1.end();\n"
              "    l2.insert(it, end);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // only warn for insert when there are preciself 2 arguments.
        check("void foo() {\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it = l1.begin();\n"
              "    l2.insert(it);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void foo() {\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it = l1.begin();\n"
              "    l2.insert(it,0,1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

    }

    void iterator4() {
        check("void foo(std::vector<std::string> &test)\n"
              "{\n"
              "    std::set<int> result;\n"
              "    for (std::vector<std::string>::const_iterator cit = test.begin();\n"
              "        cit != test.end();\n"
              "        ++cit)\n"
              "    {\n"
              "        result.insert(cit->size());\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator5() {
        check("void foo(std::vector<int> ints1, std::vector<int> ints2)\n"
              "{\n"
              "    std::vector<int>::iterator it = std::find(ints1.begin(), ints2.end(), 22);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:47]: (error) Iterators of different containers 'ints1' and 'ints2' are used together. [mismatchingContainers]\n",
                      errout_str());
    }

    void iterator6() {
        // Ticket #1357
        check("void foo(const std::set<int> &ints1)\n"
              "{\n"
              "    std::set<int> ints2;\n"
              "    std::set<int>::iterator it1 = ints1.begin();\n"
              "    std::set<int>::iterator it2 = ints1.end();\n"
              "    ints2.insert(it1, it2);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const std::set<int> &ints1)\n"
              "{\n"
              "    std::set<int> ints2;\n"
              "    std::set<int>::iterator it1 = ints1.begin();\n"
              "    std::set<int>::iterator it2 = ints2.end();\n"
              "    ints2.insert(it1, it2);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Iterators of different containers are used together.\n", "", errout_str());
    }

    void iterator7() {
        check("void foo(std::vector<int> ints1, std::vector<int> ints2)\n"
              "{\n"
              "    std::vector<int>::iterator it = std::inplace_merge(ints1.begin(), std::advance(ints1.rbegin(), 5), ints2.end());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:56]: (error) Iterators of different containers 'ints1' and 'ints2' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo(std::vector<int> ints1, std::vector<int> ints2)\n"
              "{\n"
              "    std::vector<int>::iterator it = std::inplace_merge(ints1.begin(), std::advance(ints2.rbegin(), 5), ints1.end());\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator8() {
        check("void foo(std::vector<int> ints1, std::vector<int> ints2)\n"
              "{\n"
              "    std::vector<int>::iterator it = std::find_first_of(ints1.begin(), ints2.end(), ints1.begin(), ints1.end());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:56]: (error) Iterators of different containers 'ints1' and 'ints2' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo(std::vector<int> ints1, std::vector<int> ints2)\n"
              "{\n"
              "    std::vector<int>::iterator it = std::find_first_of(ints1.begin(), ints1.end(), ints2.begin(), ints1.end());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:84]: (error) Iterators of different containers 'ints2' and 'ints1' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo(std::vector<int> ints1, std::vector<int> ints2)\n"
              "{\n"
              "    std::vector<int>::iterator it = std::find_first_of(foo.bar.begin(), foo.bar.end()-6, ints2.begin(), ints1.end());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:90]: (error) Iterators of different containers 'ints2' and 'ints1' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo(std::vector<int> ints1, std::vector<int> ints2)\n"
              "{\n"
              "    std::vector<int>::iterator it = std::find_first_of(ints1.begin(), ints1.end(), ints2.begin(), ints2.end());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #6839
        check("void f(const std::wstring& a, const std::wstring& b) {\n"
              "    const std::string tp1 = std::string(a.begin(), b.end());\n"
              "    const std::wstring tp2 = std::string(b.begin(), a.end());\n"
              "    const std::u16string tp3(a.begin(), b.end());\n"
              "    const std::u32string tp4(b.begin(), a.end());\n"
              "    const std::string fp1 = std::string(a.begin(), a.end());\n"
              "    const std::string tp2(a.begin(), a.end());\n"
              "}");
        ASSERT_EQUALS( // TODO "[test.cpp:2]: (error) Iterators of different containers are used together.\n"
            // TODO "[test.cpp:3]: (error) Iterators of different containers are used together.\n"
            "[test.cpp:4:26]: (error) Iterators of different containers 'tp3' and 'a' are used together. [mismatchingContainers]\n"
            "[test.cpp:5:26]: (error) Iterators of different containers 'tp4' and 'b' are used together. [mismatchingContainers]\n",
            errout_str());
    }

    void iterator9() {
        // Ticket #1600
        check("void foo(std::vector<int> &r)\n"
              "{\n"
              "    std::vector<int>::iterator aI = r.begin();\n"
              "    while(aI != r.end())\n"
              "    {\n"
              "        if (*aI == 0)\n"
              "        {\n"
              "            r.insert(aI, 42);\n"
              "            return;\n"
              "        }\n"
              "        ++aI;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Ticket #2481
        check("void foo(std::vector<int> &r)\n"
              "{\n"
              "    std::vector<int>::iterator aI = r.begin();\n"
              "    while(aI != r.end())\n"
              "    {\n"
              "        if (*aI == 0)\n"
              "        {\n"
              "            r.insert(aI, 42);\n"
              "            break;\n"
              "        }\n"
              "        ++aI;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Execution path checking..
        check("void foo(std::vector<int> &r, int c)\n"
              "{\n"
              "    std::vector<int>::iterator aI = r.begin();\n"
              "    while(aI != r.end())\n"
              "    {\n"
              "        if (*aI == 0)\n"
              "        {\n"
              "            r.insert(aI, 42);\n"
              "            if (c)\n"
              "            {\n"
              "                return;\n"
              "            }\n"
              "        }\n"
              "        ++aI;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:14] (error) After insert(), the iterator 'aI' may be invalid.", "", errout_str());
    }

    void iterator10() {
        // Ticket #1679
        check("void foo()\n"
              "{\n"
              "    std::set<int> s1;\n"
              "    std::set<int> s2;\n"
              "    for (std::set<int>::iterator it = s1.begin(); it != s1.end(); ++it)\n"
              "    {\n"
              "        if (true) { }\n"
              "        if (it != s2.end()) continue;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:39]: (error) Iterators of different containers 's1' and 's2' are used together. [mismatchingContainers]\n",
                      errout_str());
    }

    void iterator11() {
        // Ticket #3433
        check("int main() {\n"
              "    map<int, int> myMap;\n"
              "    vector<string> myVector;\n"
              "    for(vector<int>::iterator x = myVector.begin(); x != myVector.end(); x++)\n"
              "        myMap.erase(*x);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator12() {
        // Ticket #3201
        check("void f() {\n"
              "    std::map<int, int> map1;\n"
              "    std::map<int, int> map2;\n"
              "    std::map<int, int>::const_iterator it = map1.find(123);\n"
              "    if (it == map2.end()) { }"
              "}");
        ASSERT_EQUALS("[test.cpp:4:45]: (error) Iterators of different containers 'map1' and 'map2' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void f() {\n"
              "    std::map<int, int> map1;\n"
              "    std::map<int, int> map2;\n"
              "    std::map<int, int>::const_iterator it = map1.find(123);\n"
              "    if (map2.end() == it) { }"
              "}");
        ASSERT_EQUALS("[test.cpp:5:9]: (error) Iterators of different containers 'map2' and 'map1' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void f(std::string &s) {\n"
              "    int pos = s.find(x);\n"
              "    s.erase(pos);\n"
              "    s.erase(pos);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator13() {
        check("void f() {\n"
              "    std::vector<int> a;\n"
              "    std::vector<int> t;\n"
              "    std::vector<int>::const_iterator it;\n"
              "    it = a.begin();\n"
              "    while (it!=a.end())\n"
              "        ++it;\n"
              "    it = t.begin();\n"
              "    while (it!=a.end())\n"
              "        ++it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:10]: (error) Iterators of different containers 't' and 'a' are used together. [mismatchingContainers]\n",
                      errout_str());

        // #4062
        check("void f() {\n"
              "    std::vector<int> a;\n"
              "    std::vector<int> t;\n"
              "    std::vector<int>::const_iterator it;\n"
              "    it = a.begin();\n"
              "    while (it!=a.end())\n"
              "        ++it;\n"
              "    it = t.begin();\n"
              "    while (it!=t.end())\n"
              "        ++it;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::vector<int> a;\n"
              "    std::vector<int> t;\n"
              "    std::vector<int>::const_iterator it;\n"
              "    if(z)\n"
              "        it = a.begin();\n"
              "    else\n"
              "        it = t.begin();\n"
              "    while (z && it!=a.end())\n"
              "        ++it;\n"
              "    while (!z && it!=t.end())\n"
              "        ++it;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator14() {
        check("void f() {\n"
              "    std::map<int,Foo> x;\n"
              "    std::map<int,Foo>::const_iterator it;\n"
              "    for (it = x.find(0)->second.begin(); it != x.find(0)->second.end(); ++it) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator15() {
        check("void f(C1* x, std::list<int> a) {\n"
              "  std::list<int>::iterator pos = a.begin();\n"
              "  for(pos = x[0]->plist.begin(); pos != x[0]->plist.end(); ++pos) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator16() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l2.end();\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:36]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.end();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    while (it2 != it1)\n"
              "    {\n"
              "        ++it2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:36]: (error) Iterators of different containers 'l2' and 'l1' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it2 = l2.end();\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:36]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo()\n"
              "{\n"
              "    std::set<int> l1;\n"
              "    std::set<int> l2(10, 4);\n"
              "    std::set<int>::iterator it1 = l1.begin();\n"
              "    std::set<int>::iterator it2 = l2.find(4);\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:35]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n",
                      errout_str());
    }

    void iterator17() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    { it2 = l2.end(); }\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:36]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "    it2 = l2.end();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    it1 = l2.end();\n"
              "    it1 = l1.end();\n"
              "    if (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    { it2 = l2.end(); }\n"
              "    it2 = l1.end();\n"
              "    { it2 = l2.end(); }\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:36]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n",
                      errout_str());
    }

    void iterator18() {
        check("void foo(std::list<int> l1, std::list<int> l2)\n"
              "{\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    while (++it1 != --it2)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(std::list<int> l1, std::list<int> l2)\n"
              "{\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    while (it1++ != --it2)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(std::list<int> l1, std::list<int> l2)\n"
              "{\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    if (--it2 > it1++)\n"
              "    {\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("", "[test.cpp:5:15]: (error) Dangerous comparison using operator< on iterator. [stlBoundaries]\n", errout_str());
    }

    void iterator19() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    {\n"
              "        std::list<int> l1;\n"
              "        if (it1 != l1.end())\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:7:13] -> [test.cpp:4:36]: (error) Same iterator is used with containers 'l1' that are temporaries or defined in different scopes. [iterators3]\n",
            errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    {\n"
              "        std::list<int> l1;\n"
              "        if (l1.end() > it1)\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS(
            "[test.cpp:7] -> [test.cpp:4]: (error) Same iterator is used with containers 'l1' that are defined in different scopes.\n",
            "[test.cpp:7:19] -> [test.cpp:7:13]: (error) Same iterator is used with containers 'l1' that are temporaries or defined in different scopes. [iterators3]\n"
            "[test.cpp:7:22]: (error) Dangerous comparison using operator< on iterator. [stlBoundaries]\n",
            errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    {\n"
              "        std::list<int> l1;\n"
              "        std::list<int>::iterator it2 = l1.begin();\n"
              "        if (it1 != it2)\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:8:13] -> [test.cpp:4:36]: (error) Same iterator is used with containers 'l1' that are temporaries or defined in different scopes. [iterators3]\n",
            errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    {\n"
              "        std::list<int> l1;\n"
              "        std::list<int>::iterator it2 = l1.begin();\n"
              "        if (it2 != it1)\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:8:13] -> [test.cpp:7:40]: (error) Same iterator is used with containers 'l1' that are temporaries or defined in different scopes. [iterators3]\n",
            errout_str());

        check("std::set<int> g() {\n"
              "    static const std::set<int> s = {1};\n"
              "    return s;\n"
              "}\n"
              "void f() {\n"
              "    if (g().find(2) == g().end()) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:6:17] -> [test.cpp:6:10]: (error) Same iterator is used with containers 'g()' that are temporaries or defined in different scopes. [iterators3]\n",
            errout_str());

        check("std::set<long> f() {\n" // #5804
              "    std::set<long> s;\n"
              "    return s;\n"
              "}\n"
              "void g() {\n"
              "    for (std::set<long>::iterator it = f().begin(); it != f().end(); ++it) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:6:53] -> [test.cpp:6:41]: (error) Same iterator is used with containers 'f()' that are temporaries or defined in different scopes. [iterators3]\n",
            errout_str());
    }

    void iterator20() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    it1 = it2;\n"
              "    while (it1 != l1.end())\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:36]: (error) Iterators of different containers 'l2' and 'l1' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("std::list<int> l3;\n"
              "std::list<int>::iterator bar()\n"
              "{\n"
              "    return l3.end();\n"
              "}\n"
              "void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    it1 = bar();\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:13] -> [test.cpp:10] -> [test.cpp:11]: (error) Comparison of iterators from containers 'l1' and 'l2'.\n", "", errout_str());

    }

    void iterator21() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.end();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    if (it1 != it2)\n"
              "    {\n"
              "    }\n"
              "    if (it2 != it1)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:36]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n"
                      "[test.cpp:6:36]: (error) Iterators of different containers 'l2' and 'l1' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.end();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    if (it1 != it2 && it1 != it2)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:36]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n"
                      "[test.cpp:5:36]: (error) Iterators of different containers 'l1' and 'l2' are used together. [mismatchingContainers]\n", // duplicate
                      errout_str());
    }

    void iterator22() { // #7107
        check("void foo() {\n"
              "    std::list<int> &l = x.l;\n"
              "    std::list<int>::iterator it = l.find(123);\n"
              "    x.l.erase(it);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator23() { // #9550
        check("struct A {\n"
              "    struct B {\n"
              "        bool operator==(const A::B& b) const;\n"
              "        int x;\n"
              "        int y;\n"
              "        int z;\n"
              "    };\n"
              "};\n"
              "bool A::B::operator==(const A::B& b) const {\n"
              "    return std::tie(x, y, z) == std::tie(b.x, b.y, b.z);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator24() {
        // #9556
        check("void f(int a, int b) {\n"
              "  if (&a == &b) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int a, int b) {\n"
              "  if (std::for_each(&a, &b + 1, [](auto) {})) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (error) Iterators of different containers 'a' and 'b' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void f(int a, int b) {\n"
              "  if (std::for_each(&a, &b, [](auto) {})) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (error) Iterators of different containers 'a' and 'b' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("void f(int a) {\n"
              "  if (std::for_each(&a, &a, [](auto) {})) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:21]: (style) Same iterators expression are used for algorithm. [sameIteratorExpression]\n", errout_str());

        check("void f(int a) {\n"
              "  if (std::for_each(&a, &a + 1, [](auto) {})) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator25() {
        // #9742
        check("struct S {\n"
              "  std::vector<int>& v;\n"
              "};\n"
              "struct T {\n"
              "    bool operator()(const S& lhs, const S& rhs) const {\n"
              "        return &lhs.v != &rhs.v;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator26() { // #9176
        check(
            "#include <map>\n"
            "int main()\n"
            "{"
            "  std::map<char const*, int> m{ {\"a\", 1} };\n"
            "  if (auto iter = m.find(\"x\"); iter != m.end()) {\n"
            "    return iter->second;\n"
            "  }\n"
            "  return 0;\n"
            "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator27() {
        // #10378
        check("struct A {\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "int f(std::map<int, A> m) {\n"
              "    auto it =  m.find( 1 );\n"
              "    const int a( it == m.cend() ? 0 : it->second.a );\n"
              "    const int b( it == m.cend() ? 0 : it->second.b );\n"
              "    return a + b;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator28()
    {
        // #10450
        check("struct S {\n"
              "    struct Private {\n"
              "        std::list<int> l;\n"
              "    };\n"
              "    std::unique_ptr<Private> p;\n"
              "    int foo();\n"
              "};\n"
              "int S::foo() {\n"
              "    for(auto iter = p->l.begin(); iter != p->l.end(); ++iter) {\n"
              "        if(*iter == 1) {\n"
              "            p->l.erase(iter);\n"
              "            return 1;\n"
              "        }\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:10:24]: (style) Consider using std::find_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());
    }

    void iterator29()
    {
        // #11511
        check("std::vector<int>& g();\n"
              "void f() {\n"
              "    auto v = g();\n"
              "    auto it = g().begin();\n"
              "    while (it != g().end())\n"
              "        it = v.erase(it);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:14]: (error) Iterator 'it' referring to container 'g()' is used with container 'v'. [mismatchingContainerIterator]\n", errout_str());

        check("std::vector<int>& g(int);\n"
              "void f(int i, int j) {\n"
              "    auto& r = g(i);\n"
              "    auto it = g(j).begin();\n"
              "    while (it != g(j).end())\n"
              "        it = r.erase(it);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:14]: (error) Iterator 'it' referring to container 'g(j)' is used with container 'r'. [mismatchingContainerIterator]\n", errout_str());

        check("std::vector<int>& g();\n"
              "void f() {\n"
              "    auto& r = g();\n"
              "    auto it = g().begin();\n"
              "    while (it != g().end())\n"
              "        it = r.erase(it);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator30()
    {
        check("struct S {\n" // #12641
              "    bool b;\n"
              "    std::list<int> A, B;\n"
              "    void f();\n"
              "};\n"
              "void S::f() {\n"
              "    std::list<int>::iterator i = (b ? B : A).begin();\n"
              "    while (i != (b ? B : A).end()) {\n"
              "        ++i;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void iterator31()
    {
        check("struct S {\n" // #13327
              "    std::string a;\n"
              "};\n"
              "struct T {\n"
              "    S s;\n"
              "};\n"
              "bool f(const S& s) {\n"
              "    std::string b;\n"
              "    return s.a.c_str() == b.c_str();\n"
              "}\n"
              "bool g(const T& t) {\n"
              "    std::string b;\n"
              "    return t.s.a.c_str() == b.c_str();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9:13]: (error) Iterators of different containers 's.a' and 'b' are used together. [mismatchingContainers]\n"
                      "[test.cpp:13:15]: (error) Iterators of different containers 't.s.a' and 'b' are used together. [mismatchingContainers]\n",
                      errout_str());
    }

    void iteratorExpression() {
        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    (void)std::find(f().begin(), g().end(), 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:22]: (error) Iterators of different containers 'f()' and 'g()' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    if(f().begin() == g().end()) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:9]: (error) Iterators of different containers 'f()' and 'g()' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    auto size = f().end() - g().begin();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:18]: (error) Iterators of different containers 'f()' and 'g()' are used together. [mismatchingContainers]\n",
                      errout_str());

        check("struct A {\n"
              "    std::vector<int>& f();\n"
              "    std::vector<int>& g();\n"
              "};\n"
              "void foo() {\n"
              "    (void)std::find(A().f().begin(), A().g().end(), 0);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:6:26]: (error) Iterators of different containers 'A().f()' and 'A().g()' are used together. [mismatchingContainers]\n",
            errout_str());

        check("struct A {\n"
              "    std::vector<int>& f();\n"
              "    std::vector<int>& g();\n"
              "};\n"
              "void foo() {\n"
              "    (void)std::find(A{} .f().begin(), A{} .g().end(), 0);\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:6:27]: (error) Iterators of different containers 'A{}.f()' and 'A{}.g()' are used together. [mismatchingContainers]\n",
            errout_str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    (void)std::find(begin(f()), end(g()), 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:28]: (warning) Iterators to containers from different expressions 'f()' and 'g()' are used together. [mismatchingContainerExpression]\n", errout_str());

        check("struct A {\n"
              "    std::vector<int>& f();\n"
              "    std::vector<int>& g();\n"
              "};\n"
              "void foo() {\n"
              "    (void)std::find(A().f().begin(), A().f().end(), 0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    if(bar(f().begin()) == g().end()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::vector<int>& f();\n"
              "void foo() {\n"
              "    auto it = f().end() - 1;\n"
              "    f().begin() - it;\n"
              "    f().begin()+1 - it;\n"
              "    f().begin() - (it + 1);\n"
              "    f().begin() - f().end();\n"
              "    f().begin()+1 - f().end();\n"
              "    f().begin() - (f().end() + 1);\n"
              "    (void)std::find(f().begin(), it, 0);\n"
              "    (void)std::find(f().begin(), it + 1, 0);\n"
              "    (void)std::find(f().begin() + 1, it + 1, 0);\n"
              "    (void)std::find(f().begin() + 1, it, 0);\n"
              "    (void)std::find(f().begin(), f().end(), 0);\n"
              "    (void)std::find(f().begin() + 1, f().end(), 0);\n"
              "    (void)std::find(f().begin(), f().end() - 1, 0);\n"
              "    (void)std::find(f().begin() + 1, f().end() - 1, 0);\n"
              "    (void)std::find(begin(f()), end(f()));\n"
              "    (void)std::find(begin(f()) + 1, end(f()), 0);\n"
              "    (void)std::find(begin(f()), end(f()) - 1, 0);\n"
              "    (void)std::find(begin(f()) + 1, end(f()) - 1, 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:30]: (error) Dereference of an invalid iterator: f().end()+1 [derefInvalidIterator]\n", errout_str());

        check("std::vector<int>& f();\n"
              "void foo() {\n"
              "    if(f().begin() == f().end()) {}\n"
              "    if(f().begin() == f().end()+1) {}\n"
              "    if(f().begin()+1 == f().end()) {}\n"
              "    if(f().begin()+1 == f().end()+1) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:32]: (error) Dereference of an invalid iterator: f().end()+1 [derefInvalidIterator]\n"
                      "[test.cpp:6:34]: (error) Dereference of an invalid iterator: f().end()+1 [derefInvalidIterator]\n",
                      errout_str());

        check("std::vector<int>& f();\n"
              "void foo() {\n"
              "    if(std::begin(f()) == std::end(f())) {}\n"
              "    if(std::begin(f()) == std::end(f())+1) {}\n"
              "    if(std::begin(f())+1 == std::end(f())) {}\n"
              "    if(std::begin(f())+1 == std::end(f())+1) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:40]: (error) Dereference of an invalid iterator: std::end(f())+1 [derefInvalidIterator]\n"
                      "[test.cpp:6:42]: (error) Dereference of an invalid iterator: std::end(f())+1 [derefInvalidIterator]\n",
                      errout_str());

        check("template<int N>\n"
              "std::vector<int>& f();\n"
              "void foo() {\n"
              "    if(f<1>().begin() == f<1>().end()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  if (a.begin().x == b.begin().x) {}\n"
              "  if (begin(a).x == begin(b).x) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::list<int*> a, std::list<int*> b) {\n"
              "  if (*a.begin() == *b.begin()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    if(f().begin(1) == f().end()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(const uint8_t* data, const uint32_t dataLength) {\n"
              "    const uint32_t minimumLength = sizeof(uint16_t) + sizeof(uint16_t);\n"
              "    if (dataLength >= minimumLength) {\n"
              "        char* payload = new char[dataLength - minimumLength];\n"
              "        std::copy(&data[minimumLength], &data[dataLength], payload);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool f(const std::vector<int>& a, const std::vector<int>& b) {\n" // #11469
              "    return (a.begin() - a.end()) == (b.begin() - b.end());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #11469
              "    const std::vector<int>* vec() const { return &v; }\n"
              "    const std::vector<int> v;\n"
              "};\n"
              "void f(const S& a, const S& b) {\n"
              "    if (a.vec()->begin() - a.vec()->end() != b.vec()->begin() - b.vec()->end()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void iteratorSameExpression() {
        check("void f(std::vector<int> v) {\n"
              "    std::for_each(v.begin(), v.begin(), [](int){});\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:26]: (style) Same iterators expression are used for algorithm. [sameIteratorExpression]\n", errout_str());

        check("std::vector<int>& g();\n"
              "void f() {\n"
              "    std::for_each(g().begin(), g().begin(), [](int){});\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:28]: (style) Same iterators expression are used for algorithm. [sameIteratorExpression]\n", errout_str());

        check("void f(std::vector<int> v) {\n"
              "    std::for_each(v.end(), v.end(), [](int){});\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:24]: (style) Same iterators expression are used for algorithm. [sameIteratorExpression]\n", errout_str());

        check("std::vector<int>& g();\n"
              "void f() {\n"
              "    std::for_each(g().end(), g().end(), [](int){});\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:26]: (style) Same iterators expression are used for algorithm. [sameIteratorExpression]\n", errout_str());

        check("std::vector<int>::iterator g();\n"
              "void f(std::vector<int> v) {\n"
              "    std::for_each(g(), g(), [](int){});\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:20]: (style) Same iterators expression are used for algorithm. [sameIteratorExpression]\n", errout_str());

        check("void f(std::vector<int>::iterator it) {\n"
              "    std::for_each(it, it, [](int){});\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:19]: (style) Same iterators expression are used for algorithm. [sameIteratorExpression]\n", errout_str());
    }

    void mismatchingContainerIterator() {
        check("std::vector<int> to_vector(int value) {\n"
              "    std::vector<int> a, b;\n"
              "    a.insert(b.end(), value);\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Iterator 'b.end()' referring to container 'b' is used with container 'a'. [mismatchingContainerIterator]\n", errout_str());

        check("std::vector<int> f(std::vector<int> a, std::vector<int> b) {\n"
              "    a.erase(b.begin());\n"
              "    return a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (error) Iterator 'b.begin()' referring to container 'b' is used with container 'a'. [mismatchingContainerIterator]\n", errout_str());

        // #9973
        check("void f() {\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>& l = l2;\n"
              "    for (auto it = l.begin(); it != l.end(); ++it) {\n"
              "        if (*it == 1) {\n"
              "            l.erase(it);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:23]: (style) Consider using std::find_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        // #10012
        check("struct a {\n"
              "    int b;\n"
              "    int end() { return b; }\n"
              "};\n"
              "void f(a c, a d) {\n"
              "    if (c.end() == d.end()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10467
        check("void f(std::array<std::vector<int>, N>& A) {\n"
              "  for (auto& a : A) {\n"
              "    auto it = std::find_if(a.begin(), a.end(), \n"
              "                           [](auto i) { return i == 0; });\n"
              "    if (it != a.end()) {}\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10604
        check("struct S {\n"
              "    std::vector<int> v;\n"
              "};\n"
              "void f(S& s, int m) {\n"
              "    s.v.erase(s.v.begin() + m);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11093
        check("struct S {\n"
              "    std::vector<int> v1, v2;\n"
              "    void f(bool b) {\n"
              "        std::vector<int>& v = b ? v1 : v2;\n"
              "        v.erase(v.begin());\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #12377
        check("void f(bool b) {\n"
              "    std::vector<int> *pv;\n"
              "    if (b) {\n"
              "        std::vector<int>& r = get1();\n"
              "        pv = &r;\n"
              "    }\n"
              "    else {\n"
              "        std::vector<int>& r = get2();\n"
              "        pv = &r;\n"
              "    }\n"
              "    std::vector<int>::iterator it = pv->begin();\n"
              "    it = pv->erase(it, it + 2);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    std::vector<int> v;\n"
              "    void f() {\n"
              "        std::vector<int>* p = &v;\n"
              "        p->insert(std::find(p->begin(), p->end(), 0), 1);\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n"
              "    std::vector<int> v;\n"
              "    void f(int i) {\n"
              "        std::vector<int>* p = &v;\n"
              "        if (p->size() > i)\n"
              "            p->erase(p->begin() + i, p->end());\n"
              "    }\n"
              "};\n");
        ASSERT_EQUALS("", errout_str());

        // #11067
        check("struct S {\n"
              "    std::vector<int> v;\n"
              "    std::list<std::vector<int>::const_iterator> li;\n"
              "    void f();\n"
              "};\n"
              "void S::f() {\n"
              "    v.erase(*li.begin());\n"
              "    li.pop_front();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::set<std::string>& a, std::stack<std::set<std::string>::iterator>& b) {\n"
              "    while (!b.empty()) {\n"
              "        a.erase(b.top());\n"
              "        b.pop();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int>& a, std::vector<std::vector<int>::iterator>& b) {\n"
              "    auto it = b.begin();\n"
              "    a.erase(*it);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("namespace N {\n" // #12443
              "    std::vector<int> v;\n"
              "}\n"
              "using namespace N;\n"
              "void f() {\n"
              "    auto it = std::find(v.begin(), v.end(), 0);\n"
              "    if (it != N::v.end()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(void* p) {\n" // #12445
              "    std::vector<int>&v = *(std::vector<int>*)(p);\n"
              "    v.erase(v.begin(), v.end());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #13408
        check("void f(const std::vector<int>& v) {\n"
              "    for (const auto& i : v) {\n"
              "        if (std::distance(&*v.cbegin(), &i)) {}\n"
              "    }   \n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseIteratorOutOfBounds() {
        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    v.erase(v.begin());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:7]: (error) Calling function 'erase()' on the iterator 'v.begin()' which is out of bounds. [eraseIteratorOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    v.erase(v.end());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:7]: (error) Calling function 'erase()' on the iterator 'v.end()' which is out of bounds. [eraseIteratorOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    auto it = v.begin();\n"
              "    v.erase(it);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Calling function 'erase()' on the iterator 'it' which is out of bounds. [eraseIteratorOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    std::vector<int> v{ 1, 2, 3 };\n"
              "    auto it = v.end();\n"
              "    v.erase(it);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:7]: (error) Calling function 'erase()' on the iterator 'it' which is out of bounds. [eraseIteratorOutOfBounds]\n", errout_str());

        check("void f() {\n"
              "    std::vector<int> v{ 1, 2, 3 };\n"
              "    auto it = v.begin();\n"
              "    v.erase(it);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::vector<int> v{ 1, 2, 3 };\n"
              "    v.erase(v.end() - 1);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::vector<int> v{ 1, 2, 3 };\n"
              "    v.erase(v.begin() - 1);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:7]: (error) Calling function 'erase()' on the iterator 'v.begin()-1' which is out of bounds. [eraseIteratorOutOfBounds]\n"
                      "[test.cpp:3:23]: (error) Dereference of an invalid iterator: v.begin()-1 [derefInvalidIterator]\n",
                      errout_str());

        check("void f(std::vector<int>& v, std::vector<int>::iterator it) {\n"
              "    if (it == v.end()) {}\n"
              "    v.erase(it);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:7]: (warning) Either the condition 'it==v.end()' is redundant or function 'erase()' is called on the iterator 'it' which is out of bounds. [eraseIteratorOutOfBoundsCond]\n",
                      errout_str());

        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    ((v).erase)(v.begin());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10]: (error) Calling function 'erase()' on the iterator 'v.begin()' which is out of bounds. [eraseIteratorOutOfBounds]\n",
                      errout_str());
    }

    // Dereferencing invalid pointer
    void dereference() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> ints{1,2,3,4,5};\n"
              "    std::vector<int>::iterator iter;\n"
              "    iter = ints.begin() + 2;\n"
              "    ints.erase(iter);\n"
              "    std::cout << (*iter) << std::endl;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        TODO_ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:6] -> [test.cpp:3] -> [test.cpp:7]: (error) Using iterator to local container 'ints' that may be invalid.\n", "[test.cpp:5:22] -> [test.cpp:6:10] -> [test.cpp:3:22] -> [test.cpp:7:20]: (error, inconclusive) Using iterator to local container 'ints' that may be invalid. [invalidContainer]\n", errout_str());

        // #6554 "False positive eraseDereference - erase in while() loop"
        check("typedef std::map<Packet> packetMap;\n"
              "packetMap waitingPackets;\n"
              "void ProcessRawPacket() {\n"
              "    packetMap::iterator wpi;\n"
              "    while ((wpi = waitingPackets.find(lastInOrder + 1)) != waitingPackets.end()) {\n"
              "        waitingPackets.erase(wpi);\n"
              "        for (unsigned pos = 0; pos < buf.size(); ) {     }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8509 Uniform initialization ignored for iterator
        check("void f() {\n"
              "  std::vector<int> ints;\n"
              "  std::vector<int>::const_iterator iter {ints.cbegin()};\n"
              "  std::cout << (*iter) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void dereference_break() {  // #3644
        check("void f(std::vector<int> &ints) {\n"
              "    std::vector<int>::iterator iter;\n"
              "    for (iter=ints.begin();iter!=ints.end();++iter) {\n"
              "        if (*iter == 2) {\n"
              "            ints.erase(iter);\n"
              "            break;\n"
              "        }\n"
              "        if (*iter == 3) {\n"
              "            ints.erase(iter);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void dereference_member() {
        check("void f()\n"
              "{\n"
              "    std::map<int, int> ints;\n"
              "    std::map<int, int>::iterator iter;\n"
              "    iter = ints.begin();\n"
              "    ints.erase(iter);\n"
              "    std::cout << iter->first << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:18] -> [test.cpp:6:5]: (error) Iterator 'iter' used after element has been erased. [eraseDereference]\n"
                      "[test.cpp:6:10]: (error) Calling function 'erase()' on the iterator 'iter' which is out of bounds. [eraseIteratorOutOfBounds]\n",
                      errout_str());

        // Reverse iterator
        check("void f()\n"
              "{\n"
              "    std::map<int, int> ints;\n"
              "    std::map<int, int>::reverse_iterator iter;\n"
              "    iter = ints.rbegin();\n"
              "    ints.erase(iter);\n"
              "    std::cout << iter->first << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:18] -> [test.cpp:6:5]: (error) Iterator 'iter' used after element has been erased. [eraseDereference]\n"
                      "[test.cpp:6:10]: (error) Calling function 'erase()' on the iterator 'iter' which is out of bounds. [eraseIteratorOutOfBounds]\n",
                      errout_str());
    }

    void dereference_auto() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> ints{1,2,3,4,5};\n"
              "    auto iter = ints.begin() + 2;\n"
              "    ints.erase(iter);\n"
              "    std::cout << (*iter) << std::endl;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        TODO_ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:5] -> [test.cpp:3] -> [test.cpp:6]: (error) Using iterator to local container 'ints' that may be invalid.\n", "[test.cpp:4:27] -> [test.cpp:5:10] -> [test.cpp:3:22] -> [test.cpp:6:20]: (error, inconclusive) Using iterator to local container 'ints' that may be invalid. [invalidContainer]\n", errout_str());

        check("void f() {\n"
              "    auto x = *myList.begin();\n"
              "    myList.erase(x);\n"
              "    auto b = x.first;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("const CXXRecordDecl *CXXRecordDecl::getTemplateInstantiationPattern() const {\n"
              "    if (auto *TD = dyn_cast<ClassTemplateSpecializationDecl>(this)) {\n"
              "        auto From = TD->getInstantiatedFrom();\n"
              "    }\n"
              "    return nullptr;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void STLSize() {
        check("void foo()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
              "    {\n"
              "       foo[ii] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:6:11]: (error) Out of bounds access in expression 'foo[ii]' because 'foo' is empty. [containerOutOfBounds]\n",
            errout_str());

        check("void foo(std::vector<int> foo) {\n"
              "    for (unsigned int ii = 0; ii <= foo.size(); ++ii) {\n"
              "       foo.at(ii) = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (error) When ii==foo.size(), foo.at(ii) is out of bounds. [stlOutOfBounds]\n", errout_str());

        check("void foo(std::string& foo) {\n"
              "    for (unsigned int ii = 0; ii <= foo.length(); ++ii) {\n"
              "       foo[ii] = 'x';\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (error) When ii==foo.size(), foo[ii] is out of bounds. [stlOutOfBounds]\n", errout_str());

        check("void foo(std::string& foo, unsigned int ii) {\n"
              "    if (ii <= foo.length()) {\n"
              "       foo[ii] = 'x';\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (error) When ii==foo.size(), foo[ii] is out of bounds. [stlOutOfBounds]\n", errout_str());

        check("void foo(std::string& foo, unsigned int ii) {\n"
              "    do {\n"
              "       foo[ii] = 'x';\n"
              "       ++i;\n"
              "    } while(ii <= foo.length());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (error) When ii==foo.size(), foo[ii] is out of bounds. [stlOutOfBounds]\n", errout_str());

        check("void foo(std::string& foo, unsigned int ii) {\n"
              "    if (anything()) {\n"
              "    } else if (ii <= foo.length()) {\n"
              "       foo[ii] = 'x';\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:11]: (error) When ii==foo.size(), foo[ii] is out of bounds. [stlOutOfBounds]\n", errout_str());

        check("void foo()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    foo.push_back(1);\n"
              "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
              "    {\n"
              "    }\n"
              "    int ii = 0;\n"
              "    foo[ii] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    for (B b : D()) {}\n" // Don't crash on range-based for-loop
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void foo(std::vector<int> foo) {\n"
              "    for (unsigned int ii = 0; ii <= foo.size() + 1; ++ii) {\n"
              "       foo.at(ii) = 0;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3:11]: (error) When ii==foo.size(), foo.at(ii) is out of bounds. [stlOutOfBounds]\n", "", errout_str());
    }

    void STLSizeNoErr() {
        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii < foo.size(); ++ii)\n"
                  "    {\n"
                  "       foo[ii] = 0;\n"
                  "    }\n"
                  "}");
            ASSERT_EQUALS("", errout_str());
        }

        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
                  "    {\n"
                  "    }\n"
                  "}");
            ASSERT_EQUALS("", errout_str());
        }

        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
                  "    {\n"
                  "        if (ii == foo.size())\n"
                  "        {\n"
                  "        }\n"
                  "        else\n"
                  "        {\n"
                  "            foo[ii] = 0;\n"
                  "        }\n"
                  "    }\n"
                  "}");
            ASSERT_EQUALS(
                "[test.cpp:11:16]: (error) Out of bounds access in expression 'foo[ii]' because 'foo' is empty. [containerOutOfBounds]\n",
                errout_str());
        }

        {
            check("void f(const std::map<int,int> &data) {\n"
                  "    int i = x;"
                  "    for (int i = 5; i <= data.size(); i++)\n"
                  "        data[i] = 0;\n"
                  "}");
            ASSERT_EQUALS("", errout_str());
        }

        {
            check("void foo(std::vector<int> foo) {\n"
                  "    for (unsigned int ii = 0; ii <= foo.size() - 1; ++ii) {\n"
                  "       foo.at(ii) = 0;\n"
                  "    }\n"
                  "}");
            ASSERT_EQUALS("", errout_str());
        }
    }

    void negativeIndex() {
        check("void f(const std::vector<int> &v) {\n"
              "  v[-11] = 123;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:3]: (error) Array index -11 is out of bounds. [negativeContainerIndex]\n", errout_str());

        check("int f(int x, const std::vector<int>& a) {\n"
              "    if (!(x < 5))\n"
              "        return a[x - 5];\n"
              "    else\n"
              "        return a[4 - x];\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::array<int,6> values;\n"
              "int get_value();\n"
              "int compute() {\n"
              "    int i = get_value();\n"
              "    if( i < 0 || i > 5)\n"
              "        return -1;\n"
              "    int sum = 0;\n"
              "    for( int j = i+1; j < 7; ++j)\n"
              "        sum += values[j-1];\n"
              "    return sum;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct B { virtual int g() { return 0; } };\n" // #11831
              "struct C {\n"
              "    int h() const { return b->g(); }\n"
              "    B* b;\n"
              "};\n"
              "struct O {\n"
              "    int f() const;\n"
              "    std::vector<int> v;\n"
              "    C c;\n"
              "};\n"
              "int O::f() const { return v[c.h() - 1]; }\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::vector<int>& v) {\n" // #13196
              "    if (v.empty())\n"
              "        return;\n"
              "    int idx = -1;\n"
              "    for (int i = 0; i < v.size(); ++i) {\n"
              "        idx = i;\n"
              "    }\n"
              "    (void)v[idx];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        Settings s = settings;
        s.daca = true;

        check("void f() {\n"
              "    const char a[][5] = { \"1\", \"true\", \"on\", \"yes\" };\n"
              "}\n", s);
        ASSERT_EQUALS("", errout_str());
    }

    void negativeIndexMultiline() {
        setMultiline();
        Settings s = settings;
        s.verbose = true;

        check("bool valid(int);\n" // #11697
              "void f(int i, const std::vector<int>& v) {\n"
              "    if (!valid(i))\n"
              "        return;\n"
              "    if (v[i]) {}\n"
              "}\n"
              "void g(const std::vector<int>& w) {\n"
              "    f(-1, w);\n"
              "}\n", s);
        ASSERT_EQUALS("[test.cpp:5:9]: warning: Array index -1 is out of bounds. [negativeContainerIndex]\n"
                      "[test.cpp:8:8]: note: Calling function 'f', 1st argument '-1' value is -1\n"
                      "[test.cpp:3:9]: note: Assuming condition is false\n"
                      "[test.cpp:5:9]: note: Negative array index\n",
                      errout_str());
    }

    void erase1() {
        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it) {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "    for (it = foo.begin(); it != foo.end(); ++it) {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5] -> [test.cpp:5:18]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n"
                      "[test.cpp:7:5] -> [test.cpp:8:18]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n", errout_str());

        check("void f(std::list<int> &ints)\n"
              "{\n"
              "    std::list<int>::iterator i = ints.begin();\n"
              "    i = ints.erase(i);\n"
              "    *i = 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator i;\n"
              "    while (i != x.y.end())\n"
              "        i = x.y.erase(i);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #2101
        check("void f(vector< std::list<int> > &ints, unsigned int i)\n"
              "{\n"
              "    std::list<int>::iterator it;\n"
              "    for(it = ints[i].begin(); it != ints[i].end(); it++) {\n"
              "        if (*it % 2)\n"
              "            it = ints[i].erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void erase2() {
        check("static void f()\n"
              "{\n"
              "    for (iterator it = foo.begin(); it != foo.end(); it = next)\n"
              "    {\n"
              "        next = it;\n"
              "        next++;\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void erase3() {
        check("static void f(std::list<abc> &foo)\n"
              "{\n"
              "    std::list<abc>::iterator it = foo.begin();\n"
              "    foo.erase(it->a);\n"
              "    if (it->b);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void erase4() {
        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it, it2;\n"
              "    for (it = foo.begin(); it != i2; ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5] -> [test.cpp:6:18]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it = foo.begin();\n"
              "    for (; it != i2; ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5] -> [test.cpp:6:18]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it = foo.begin();\n"
              "    while (it != i2)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5] -> [test.cpp:6:18]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it = foo.begin();\n"
              "    while (it != i2)\n"
              "    {\n"
              "        foo.erase(++it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5] -> [test.cpp:6:18]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n", errout_str());
    }

    void erase5() {
        check("void f()\n"
              "{\n"
              "    std::list<int> foo;\n"
              "    std::list<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (*it == 123)\n"
              "            foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:5] -> [test.cpp:8:22]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n", errout_str());
    }

    void erase6() {
        check("void f() {\n"
              "    std::vector<int> vec(3);\n"
              "    std::vector<int>::iterator it;\n"
              "    std::vector<int>::iterator itEnd = vec.end();\n"
              "    for (it = vec.begin(); it != itEnd; it = vec.begin(), itEnd = vec.end())\n"
              "    {\n"
              "        vec.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseBreak() {
        check("void f()\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        if (x)"
              "            break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5] -> [test.cpp:5:18]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (x) {\n"
              "            foo.erase(it);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(int x)\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        if (x)"
              "            return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5] -> [test.cpp:5:18]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n", errout_str());

    }

    void eraseContinue() {
        check("void f(std::vector<int> &ints)\n"
              "{\n"
              "    std::vector<int>::iterator it;\n"
              "    std::vector<int>::iterator jt = ints.begin();\n"
              "    for (it = ints.begin(); it != ints.end(); it = jt) {\n"
              "        ++jt;\n"
              "        if (*it == 1) {\n"
              "            jt = ints.erase(it);\n"
              "            continue;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::map<uint32, uint32> my_map) {\n" // #7365
              "  std::map<uint32, uint32>::iterator itr = my_map.begin();\n"
              "  switch (itr->first) {\n"
              "  case 0:\n"
              "    my_map.erase(itr);\n"
              "    continue;\n"
              "  case 1:\n"
              "    itr->second = 1;\n"
              "    break;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseReturn1() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseReturn2() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (*it == 1) {\n"
              "            foo.erase(it);\n"
              "            return;\n"
              "        }\n"
              "        else {\n"
              "            foo.erase(it);\n"
              "            return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseReturn3() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (somecondition) {\n"
              "            if (*it == 1)\n"
              "                foo.erase(it);\n"
              "            else\n"
              "                *it = 0;\n"
              "            return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (a) {\n"
              "            if (b)\n"
              "                foo.erase(it);\n" // <- TODO: erase shound't cause inconclusive valueflow
              "            else\n"
              "                *it = 0;\n"
              "        }\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS(
            "[test.cpp:5] -> [test.cpp:7] -> [test.cpp:8] -> [test.cpp:8] -> [test.cpp:7] -> [test.cpp:5] -> [test.cpp:9] -> [test.cpp:3] -> [test.cpp:5]: (error) Using iterator to local container 'foo' that may be invalid.\n",
            "[test.cpp:5:24] -> [test.cpp:7:13] -> [test.cpp:8:17] -> [test.cpp:8:17] -> [test.cpp:7:13] -> [test.cpp:5:31] -> [test.cpp:9:21] -> [test.cpp:3:22] -> [test.cpp:5:28]: (error, inconclusive) Using iterator to local container 'foo' that may be invalid. [invalidContainer]\n",
            errout_str());
    }

    void eraseGoto() {
        check("void f()\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        goto abc;\n"
              "    }\n"
              "bar:\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseAssign1() {
        check("void f()\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        it = foo.begin();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseAssign2() {
        check("void f(std::list<int> &ints)\n"
              "{\n"
              "    for (std::list<int>::iterator it = ints.begin(); it != ints.end();) {\n"
              "        if (*it == 123) {\n"
              "            std::list<int>::iterator copy = it;\n"
              "            ++copy;\n"
              "            ints.erase(it);\n"
              "            it = copy;\n"
              "        } else {\n"
              "            it->second = 123;\n"
              "            ++it;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseAssign3() {
        check("void f(std::list<list<int> >& l) {\n"
              "    std::list<std::list<int> >::const_iterator i = l.begin();\n"
              "    std::list<int>::const_iterator j = (*i).begin();\n"
              "    cout << *j << endl;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseAssign4() {
        check("void f(std::list<int> data) {\n"
              "  std::list<int>::const_iterator it = data.begin();\n"
              "  it = data.erase(it);\n"
              "  it = data.erase(it);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(Data data) {\n"
              "  std::list<int>::const_iterator it = data.ints.begin();\n"
              "  it = data.ints.erase(it);\n"
              "  it = data.ints.erase(it);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseAssignByFunctionCall() {
        check("void f(std::list<list<int> >& l) {\n"
              "    std::list<foo>::const_iterator i;\n"
              "    bar(i);\n"
              "    cout << *i;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseErase() {
        check("void f(std::vector<ints> &ints)\n"
              "{\n"
              "    std::vector<int>::iterator iter;\n"
              "    iter = ints.begin() + 2;\n"
              "    ints.erase(iter);\n"
              "    ints.erase(iter);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        TODO_ASSERT_EQUALS("[test.cpp:1] -> [test.cpp:4] -> [test.cpp:5] -> [test.cpp:1] -> [test.cpp:6]: (error) Using iterator to local container 'ints' that may be invalid.\n", "[test.cpp:1:31] -> [test.cpp:4:22] -> [test.cpp:5:10] -> [test.cpp:1:27] -> [test.cpp:6:16]: (error, inconclusive) Using iterator to local container 'ints' that may be invalid. [invalidContainer]\n", errout_str());
    }

    void eraseByValue() {
        check("void f()\n"
              "{\n"
              "    std::set<int> foo;\n"
              "    for (std::set<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(*it);\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Iterator 'it' becomes invalid when deleted by value from 'foo'\n", "", errout_str());

        check("int f(std::set<int> foo) {\n"
              "    std::set<int>::iterator it = foo.begin();\n"
              "    foo.erase(*it);\n"
              "    return *it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:12] -> [test.cpp:3:5]: (error) Iterator 'it' used after element has been erased. [eraseDereference]\n", errout_str());

        check("void f(std::set<int> foo)\n"
              "{\n"
              "    std::set<int>::iterator it = foo.begin();\n"
              "    foo.erase(*it);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #5669
        check("void f() {\n"
              "    HashSet_Ref::iterator aIt = m_ImplementationMap.find( xEle );\n"
              "    m_SetLoadedFactories.erase(*aIt);\n"
              "    m_SetLoadedFactories.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::list<int>& m_ImplementationMap) {\n"
              "    std::list<int>::iterator aIt = m_ImplementationMap.begin();\n"
              "    m_ImplementationMap.erase(*aIt);\n"
              "    m_ImplementationMap.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Invalid iterator: aIt [invalidIterator1]\n", errout_str());

        check("void f(const std::list<int>& m_ImplementationMap) {\n"
              "    std::list<int>::iterator aIt = m_ImplementationMap.begin();\n"
              "    std::list<int>::iterator bIt = m_ImplementationMap.begin();\n"
              "    m_ImplementationMap.erase(*bIt);\n"
              "    m_ImplementationMap.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void eraseIf() {
        // #4816
        check("void func(std::list<std::string> strlist) {\n"
              "    for (std::list<std::string>::iterator str = strlist.begin(); str != strlist.end(); str++) {\n"
              "        if (func2(*str)) {\n"
              "            strlist.erase(str);\n"
              "            if (strlist.empty())\n"
              "                 return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5] -> [test.cpp:4:26]: (error) Iterator 'str' used after element has been erased. [eraseDereference]\n", errout_str());
    }

    void eraseOnVector() {
        check("void f(std::vector<int>& v) {\n"
              "    std::vector<int>::iterator aIt = v.begin();\n"
              "    v.erase(something(unknown));\n" // All iterators become invalidated when erasing from std::vector
              "    v.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:27] -> [test.cpp:2:45] -> [test.cpp:3:7] -> [test.cpp:1:26] -> [test.cpp:4:13]: (error) Using iterator to local container 'v' that may be invalid. [invalidContainer]\n", errout_str());

        check("void f(std::vector<int>& v) {\n"
              "    std::vector<int>::iterator aIt = v.begin();\n"
              "    std::vector<int>::iterator bIt = v.begin();\n"
              "    v.erase(bIt);\n" // All iterators become invalidated when erasing from std::vector
              "    aIt = v.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:27] -> [test.cpp:2:45] -> [test.cpp:4:7] -> [test.cpp:1:26] -> [test.cpp:5:19]: (error) Using iterator to local container 'v' that may be invalid. [invalidContainer]\n", errout_str());
    }

    void pushback1() {
        check("void f(const std::vector<int> &foo)\n"
              "{\n"
              "    std::vector<int>::const_iterator it = foo.begin();\n"
              "    foo.push_back(123);\n"
              "    *it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:35] -> [test.cpp:3:52] -> [test.cpp:4:9] -> [test.cpp:1:32] -> [test.cpp:5:6]: (error) Using iterator to local container 'foo' that may be invalid. [invalidContainer]\n", errout_str());
    }

    void pushback2() {
        check("void f()\n"
              "{\n"
              "    std::vector<int>::const_iterator it = foo.begin();\n"
              "    foo.push_back(123);\n"
              "    {\n"
              "        int *it = &foo[0];\n"
              "        *it = 456;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void pushback3() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    foo.push_back(10);\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.push_back(123);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:24] -> [test.cpp:6:31] -> [test.cpp:8:13] -> [test.cpp:3:22] -> [test.cpp:6:28]: (error) Using iterator to local container 'foo' that may be invalid. [invalidContainer]\n", errout_str());
    }

    void pushback4() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    ints.push_back(1);\n"
              "    int *first = &ints[0];\n"
              "    ints.push_back(2);\n"
              "    *first;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:18] -> [test.cpp:6:10] -> [test.cpp:3:22] -> [test.cpp:7:6]: (error) Using pointer to local variable 'ints' that may be invalid. [invalidContainer]\n", errout_str());
    }

    void pushback5() {
        check("void f()\n"
              "{\n"
              "    std::vector<int>::const_iterator i;\n"
              "\n"
              "    for (i=v.begin(); i!=v.end(); ++i)\n"
              "    {\n"
              "    }\n"
              "\n"
              "    for (i=rhs.v.begin(); i!=rhs.v.end(); ++i)\n"
              "    {\n"
              "        v.push_back(*i);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void pushback6() {
        // ticket #735
        check("void f()\n"
              "{\n"
              "    std::vector<int> v;\n"
              "    v.push_back(1);\n"
              "    v.push_back(2);\n"
              "    for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it)\n"
              "    {\n"
              "        if (*it == 1)\n"
              "            v.push_back(10);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:49] -> [test.cpp:8:17] -> [test.cpp:8:17] -> [test.cpp:6:56] -> [test.cpp:9:15] -> [test.cpp:3:22] -> [test.cpp:6:53]: (error) Using iterator to local container 'v' that may be invalid. [invalidContainer]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    std::vector<int> v;\n"
              "    vector.push_back(1);\n"
              "    vector.push_back(2);\n"
              "    for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it)\n"
              "    {\n"
              "        if (*it == 1)\n"
              "            v.push_back(10);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:49] -> [test.cpp:8:17] -> [test.cpp:8:17] -> [test.cpp:6:56] -> [test.cpp:9:15] -> [test.cpp:3:22] -> [test.cpp:6:53]: (error) Using iterator to local container 'v' that may be invalid. [invalidContainer]\n", errout_str());
    }

    void pushback7() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    foo.push_back(10);\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); it++)\n"
              "    {\n"
              "        foo.push_back(123);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:24] -> [test.cpp:6:31] -> [test.cpp:8:13] -> [test.cpp:3:22] -> [test.cpp:6:28]: (error) Using iterator to local container 'foo' that may be invalid. [invalidContainer]\n", errout_str());
    }

    void pushback8() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    std::vector<int>::const_iterator end = ints.end();\n"
              "    ints.push_back(10);\n"
              "    std::vector<int>::iterator it;\n"
              "    unsigned int sum = 0;\n"
              "    for (it = ints.begin(); it != end; ++it)\n"
              "    {\n"
              "        sum += *it;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10:13]: (style) Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]\n"
                      "[test.cpp:4:52] -> [test.cpp:5:10] -> [test.cpp:3:22] -> [test.cpp:8:35]: (error) Using iterator to local container 'ints' that may be invalid. [invalidContainer]\n",
                      errout_str());
    }

    void pushback9() {
        check("struct A {\n"
              "    std::vector<int> ints;\n"
              "};\n"
              "\n"
              "void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    A a;\n"
              "    std::vector<int>::const_iterator i = ints.begin();\n"
              "    std::vector<int>::const_iterator e = ints.end();\n"
              "    while (i != e)\n"
              "    {\n"
              "        a.ints.push_back(*i);\n"
              "        ++i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void pushback10() {
        check("void f(std::vector<int> &foo)\n"
              "{\n"
              "    std::vector<int>::const_iterator it = foo.begin();\n"
              "    foo.reserve(100);\n"
              "    *it = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:29] -> [test.cpp:3:52] -> [test.cpp:4:9] -> [test.cpp:1:26] -> [test.cpp:5:6]: (error) Using iterator to local container 'foo' that may be invalid. [invalidContainer]\n", errout_str());

        // in loop
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    foo.push_back(10);\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.reserve(123);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:24] -> [test.cpp:6:31] -> [test.cpp:8:13] -> [test.cpp:3:22] -> [test.cpp:6:28]: (error) Using iterator to local container 'foo' that may be invalid. [invalidContainer]\n", errout_str());
    }

    void pushback11() {
        // #2798
        check("void f() {\n"
              "    std::vector<int> ints;\n"
              "    std::vector<int>::iterator it = ints.begin();\n"
              "    if (it == ints.begin()) {\n"
              "        ints.push_back(0);\n"
              "    } else {\n"
              "        ints.insert(it,0);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void pushback12() {
        // #4197
        check("void foo(double s)\n"
              "{\n"
              "    std::vector<double> vec;\n"
              "    for( std::vector<double>::iterator it = vec.begin(); it != vec.end(); ++it )\n"
              "    {\n"
              "        vec.insert( it, s );\n"
              "        for(unsigned int i = 0; i < 42; i++)\n"
              "        {}\n"
              "        *it;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS(
            "[test.cpp:4:54] -> [test.cpp:6:13] -> [test.cpp:3:25] -> [test.cpp:9:10]: (error, inconclusive) Using iterator to local container 'vec' that may be invalid. [invalidContainer]\n",
            errout_str());
    }

    void pushback13() {
        check("bool Preprocessor::ConcatenateIncludeName(SmallString<128> &FilenameBuffer, SourceLocation &End) {\n"
              "    unsigned PreAppendSize = FilenameBuffer.size();\n"
              "    FilenameBuffer.resize(PreAppendSize + CurTok.getLength());\n"
              "    const char *BufPtr = &FilenameBuffer[PreAppendSize];\n"
              "    return true;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void insert1() {
        check("void f(std::vector<int> &ints)\n"
              "{\n"
              "    std::vector<int>::iterator iter = ints.begin() + 5;\n"
              "    ints.insert(ints.begin(), 1);\n"
              "    ++iter;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:30] -> [test.cpp:3:49] -> [test.cpp:4:10] -> [test.cpp:1:26] -> [test.cpp:5:7]: (error) Using iterator to local container 'ints' that may be invalid. [invalidContainer]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    std::vector<int>::iterator iter = ints.begin();\n"
              "    ints.insert(iter, 1);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    std::vector<int>::iterator iter = ints.begin();\n"
              "    ints.insert(iter, 1);\n"
              "    ints.insert(iter, 2);\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        TODO_ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:5] -> [test.cpp:3] -> [test.cpp:6]: (error) Using iterator to local container 'ints' that may be invalid.\n", "[test.cpp:4:49] -> [test.cpp:5:10] -> [test.cpp:3:22] -> [test.cpp:6:17]: (error, inconclusive) Using iterator to local container 'ints' that may be invalid. [invalidContainer]\n", errout_str());

        check("void* f(const std::vector<Bar>& bars) {\n"
              "    std::vector<Bar>::iterator i = bars.begin();\n"
              "    bars.insert(Bar());\n"
              "    void* v = &i->foo;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:37] -> [test.cpp:2:46] -> [test.cpp:3:10] -> [test.cpp:1:33] -> [test.cpp:4:16]: (error) Using iterator to local container 'bars' that may be invalid. [invalidContainer]\n", errout_str());

        check("Foo f(const std::vector<Bar>& bars) {\n"
              "    std::vector<Bar>::iterator i = bars.begin();\n"
              "    bars.insert(Bar());\n"
              "    return i->foo;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:35] -> [test.cpp:2:46] -> [test.cpp:3:10] -> [test.cpp:1:31] -> [test.cpp:4:12]: (error) Using iterator to local container 'bars' that may be invalid. [invalidContainer]\n", errout_str());

        check("void f(const std::vector<Bar>& bars) {\n"
              "    for(std::vector<Bar>::iterator i = bars.begin(); i != bars.end(); ++i) {\n"
              "        i = bars.insert(i, bar);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // TODO: This shouldn't be inconclusive
        check("void f(const std::vector<Bar>& bars) {\n"
              "    for(std::vector<Bar>::iterator i = bars.begin(); i != bars.end(); ++i) {\n"
              "        bars.insert(i, bar);\n"
              "        i = bars.insert(i, bar);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:36] -> [test.cpp:2:50] -> [test.cpp:3:14] -> [test.cpp:1:32] -> [test.cpp:4:25]: (error, inconclusive) Using iterator to local container 'bars' that may be invalid. [invalidContainer]\n", errout_str());

        // TODO: This shouldn't be inconclusive
        check("void* f(const std::vector<Bar>& bars) {\n"
              "    std::vector<Bar>::iterator i = bars.begin();\n"
              "    bars.insert(i, Bar());\n"
              "    i = bars.insert(i, Bar());\n"
              "    void* v = &i->foo;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1:37] -> [test.cpp:2:46] -> [test.cpp:3:10] -> [test.cpp:1:33] -> [test.cpp:4:21]: (error, inconclusive) Using iterator to local container 'bars' that may be invalid. [invalidContainer]\n", errout_str());
    }

    void insert2() {
        // Ticket: #2169
        check("void f(std::vector<int> &vec) {\n"
              "    for(std::vector<int>::iterator iter = vec.begin(); iter != vec.end(); ++iter)\n"
              "    {\n"
              "        vec.insert(iter, 0);\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int> &vec) {\n"
              "    for(std::vector<int>::iterator iter = vec.begin(); iter != vec.end(); ++iter)\n"
              "    {\n"
              "        if (*it == 0) {\n"
              "            vec.insert(iter, 0);\n"
              "            return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void popback1() { // #11553
        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    v.pop_back();\n"
              "    std::list<int> l;\n"
              "    l.pop_front();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:15]: (error) Out of bounds access in expression 'v.pop_back()' because 'v' is empty. [containerOutOfBounds]\n"
                      "[test.cpp:5:16]: (error) Out of bounds access in expression 'l.pop_front()' because 'l' is empty. [containerOutOfBounds]\n",
                      errout_str());

        check("void f(std::vector<int>& v) {\n"
              "    if (v.empty()) {}\n"
              "    v.pop_back();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:16] -> [test.cpp:3:15]: (warning) Either the condition 'v.empty()' is redundant or expression 'v.pop_back()' causes access out of bounds. [containerOutOfBounds]\n",
                      errout_str());

        check("void f(std::vector<int>& v) {\n"
              "    v.pop_back();\n"
              "    if (v.empty()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void stlBoundaries1() {
        const std::string stlCont[] = {
            "list", "set", "multiset", "map", "multimap"
        };

        for (size_t i = 0; i < getArrayLength(stlCont); ++i) {
            check("void f()\n"
                  "{\n"
                  "    std::" + stlCont[i] + "<int>::iterator it;\n"
                  "    for (it = ab.begin(); it < ab.end(); ++it)\n"
                  "        ;\n"
                  "}");

            ASSERT_EQUALS_MSG("[test.cpp:4:25]: (error) Dangerous comparison using operator< on iterator. [stlBoundaries]\n", errout_str(), stlCont[i]);
        }

        check("void f() {\n"
              "    std::forward_list<int>::iterator it;\n"
              "    for (it = ab.begin(); ab.end() > it; ++it) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:36]: (error) Dangerous comparison using operator< on iterator. [stlBoundaries]\n", errout_str());

        // #5926 no FP Dangerous comparison using operator< on iterator on std::deque
        check("void f() {\n"
              "    std::deque<int>::iterator it;\n"
              "    for (it = ab.begin(); ab.end() > it; ++it) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void stlBoundaries2() {
        check("void f()\n"
              "{\n"
              "    std::vector<std::string> files;\n"
              "    std::vector<std::string>::const_iterator it;\n"
              "    for (it = files.begin(); it < files.end(); it++) { }\n"
              "    for (it = files.begin(); it < files.end(); it++) { };\n"
              "}");

        ASSERT_EQUALS("", errout_str());
    }

    void stlBoundaries3() {
        check("void f()\n"
              "{\n"
              "    set<int> files;\n"
              "    set<int>::const_iterator current;\n"
              "    for (current = files.begin(); current != files.end(); ++current)\n"
              "    {\n"
              "       assert(*current < 100)\n"
              "    }\n"
              "}");

        ASSERT_EQUALS("", errout_str());

        check("bool f() {\n"
              "    static set<Foo>::const_iterator current;\n"
              "    return 25 > current->bar;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (error) Invalid iterator 'current' used. [eraseDereference]\n", errout_str());
    }

    void stlBoundaries4() {

        check("void f() {\n"
              "    std::forward_list<std::vector<std::vector<int>>>::iterator it;\n"
              "    for (it = ab.begin(); ab.end() > it; ++it) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:36]: (error) Dangerous comparison using operator< on iterator. [stlBoundaries]\n", errout_str());

        // don't crash
        check("void f() {\n"
              "    if (list < 0) ;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    if (list < 0) {\n"
              "        std::forward_list<std::vector<std::vector<int>>>::iterator it;\n"
              "        for (it = ab.begin(); ab.end() > it; ++it) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:40]: (error) Dangerous comparison using operator< on iterator. [stlBoundaries]\n", errout_str());
    }

    void stlBoundaries5() {
        check("class iterator { int foo(); };\n"
              "int foo() {\n"
              "    iterator i;\n"
              "    return i.foo();;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("class iterator {\n"
              "    Class operator*();\n"
              "    iterator& operator++();\n"
              "    int foo();\n"
              "};\n"
              "int foo() {\n"
              "    iterator i;\n"
              "    return i.foo();;\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:8:12]: (error, inconclusive) Invalid iterator 'i' used. [eraseDereference]\n", errout_str());
    }

    void stlBoundaries6() { // #7106
        check("void foo(std::vector<int>& vec) {\n"
              "    for (Function::iterator BB : vec) {\n"
              "        for (int Inst : *BB)\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }


    void if_find() {
        // ---------------------------
        // set::find
        // ---------------------------

        // error (simple)
        check("void f(std::set<int> s)\n"
              "{\n"
              "    if (s.find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked. [stlIfFind]\n", errout_str());

        // error (pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if ((*s).find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked. [stlIfFind]\n", errout_str());

        // error (pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if (s->find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked. [stlIfFind]\n", errout_str());

        // error (array-like pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if (s[0].find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked. [stlIfFind]\n", errout_str());

        // error (array)
        check("void f(std::set<int> s [10])\n"
              "{\n"
              "    if (s[0].find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked. [stlIfFind]\n", errout_str());

        // error (undefined length array)
        check("void f(std::set<int> s [])\n"
              "{\n"
              "    if (s[0].find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked. [stlIfFind]\n", errout_str());

        // error (vector)
        check("void f(std::vector<std::set<int> > s)\n"
              "{\n"
              "    if (s[0].find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked. [stlIfFind]\n", errout_str());

        // error (assignment)
        check("void f(std::set<int> s)\n"
              "{\n"
              "    if (a || (x = s.find(12))) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:19]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked. [stlIfFind]\n", errout_str());

        // ok (simple)
        check("void f(std::set<int> s)\n"
              "{\n"
              "    if (s.find(123) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ok (pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if ((*s).find(12) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ok (array-like pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if (s[0].find(12) != s->end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ok (array)
        check("void f(std::set<int> s [10])\n"
              "{\n"
              "    if (s[0].find(123) != s->end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ok (undefined length array)
        check("void f(std::set<int> s [])\n"
              "{\n"
              "    if (s[0].find(123) != s->end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ok (assignment)
        check("void f(std::set<int> s)\n"
              "{\n"
              "    if (a || (x = s.find(12)) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ok (dereference, #6402)
        check("void f(std::set<Foo> s) {\n"
              "    if (s.find(12).member) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::set<int> s) {\n"
              "    if (auto result = s.find(123); result != s.end()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ---------------------------
        // std::find
        // ---------------------------

        // error
        check("void f()\n"
              "{\n"
              "    if (std::find(a,b,c)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked. [stlIfFind]\n", errout_str());

        // ok
        check("void f()\n"
              "{\n"
              "    if (std::find(a,b,c) != c) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // ok (less than comparison, #6217)
        check("void f(std::vector<int> s)\n"
              "{\n"
              "    if (std::find(a, b, c) < d) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #3714 - segmentation fault for syntax error
        ASSERT_THROW_INTERNAL(check("void f() {\n"
                                    "    if (()) { }\n"
                                    "}"),
                              AST);

        // #3865
        check("void f() {\n"
              "    if ((std::find(a,b,c)) != b) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void if_str_find() {
        // error (simple)
        check("void f(const std::string &s)\n"
              "{\n"
              "    if (s.find(\"abc\")) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (performance) Inefficient usage of string::find() in condition; string::starts_with() could be faster. [stlIfStrFind]\n", errout_str());

        // error (pointer)
        check("void f(const std::string *s)\n"
              "{\n"
              "    if ((*s).find(\"abc\")) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:11]: (performance) Inefficient usage of string::find() in condition; string::starts_with() could be faster. [stlIfStrFind]\n", errout_str());

        // error (pointer)
        check("void f(const std::string *s)\n"
              "{\n"
              "    if (s->find(\"abc\")) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (performance) Inefficient usage of string::find() in condition; string::starts_with() could be faster. [stlIfStrFind]\n", errout_str());

        // error (vector)
        check("void f(const std::vector<std::string> &s)\n"
              "{\n"
              "    if (s[0].find(\"abc\")) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (performance) Inefficient usage of string::find() in condition; string::starts_with() could be faster. [stlIfStrFind]\n", errout_str());

        // #3162
        check("void f(const std::string& s1, const std::string& s2)\n"
              "{\n"
              "    if ((!s1.empty()) && (0 == s1.find(s2))) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:32]: (performance) Inefficient usage of string::find() in condition; string::starts_with() could be faster. [stlIfStrFind]\n", errout_str());

        // #4102
        check("void f(const std::string &define) {\n"
              "    if (define.find(\"=\") + 1U == define.size());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string a, std::string b) {\n"  // #4480
              "    if (a.find(\"<\") < b.find(\">\")) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::string &s) {\n"
              "    if (foo(s.find(\"abc\"))) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7349 - std::string::find_first_of
        check("void f(const std::string &s) {\n"
              "    if (s.find_first_of(\"abc\")==0) { }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // # 10153
        check("int main() {\n"
              "  for (;;) {\n"
              "    std::string line = getLine();\n"
              "    if (line.find(\" GL_EXTENSIONS =\") < 12)\n"
              "      return 1;\n"
              "  }\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }


    void size1() {
        {
            const char code[] = "struct Fred {\n"
                                "    void foo();\n"
                                "    std::list<int> x;\n"
                                "};\n"
                                "void Fred::foo()\n"
                                "{\n"
                                "    if (x.size() == 0) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:7:9]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "std::list<int> x;\n"
                                "void f()\n"
                                "{\n"
                                "    if (x.size() == 0) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:9]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (x.size() == 0) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:9]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (0 == x.size()) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:14]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (x.size() != 0) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:9]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (0 != x.size()) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:14]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (x.size() > 0) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:9]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (0 < x.size()) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:13]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] =  "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (x.size() >= 1) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:9]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (x.size() < 1) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:9]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (1 <= x.size()) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:14]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (1 > x.size()) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:13]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (x.size()) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:9]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    if (!x.size()) {}\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:10]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        check("void f()\n"
              "{\n"
              "    std::list<int> x;\n"
              "    fun(x.size());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        {
            const char code[] ="void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    fun(!x.size());\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:10]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f()\n"
                                "{\n"
                                "    std::list<int> x;\n"
                                "    fun(a && x.size());\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS("[test.cpp:4:14]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n", errout_str());
            check(code);
            ASSERT_EQUALS("", errout_str());
        }

        check("void f() {\n" // #4039
              "    std::list<int> x;\n"
              "    fun(width % x.size() != 0);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #4584
        check("void f() {\n"
              "    std::list<int> x;\n"
              "    if (foo + 1 > x.size()) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        check("void f() {\n"
              "    std::list<int> x;\n"
              "    if (x.size() < 1 + foo) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void size2() {
        const char code[] = "struct Fred {\n"
                            "    std::list<int> x;\n"
                            "};\n"
                            "struct Wilma {\n"
                            "    Fred f;\n"
                            "    void foo();\n"
                            "};\n"
                            "void Wilma::foo()\n"
                            "{\n"
                            "    if (f.x.size() == 0) {}\n"
                            "}";
        check(code, settingsCpp03);
        ASSERT_EQUALS(
            "[test.cpp:10:11]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n"
            "[test.cpp:10:11]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n",   // duplicate
            errout_str());

        check(code);
        ASSERT_EQUALS("", errout_str());
    }

    void size3() {
        {
            const char code[] = "namespace N {\n"
                                "    class Zzz {\n"
                                "    public:\n"
                                "        std::list<int> x;\n"
                                "    };\n"
                                "}\n"
                                "using namespace N;\n"
                                "Zzz * zzz;\n"
                                "int main() {\n"
                                "    if (zzz->x.size() > 0) { }\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS(
                "[test.cpp:10:14]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n"
                "[test.cpp:10:14]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n",   // duplicate
                errout_str());
        }

        {
            const char code[] = "namespace N {\n"
                                "    class Zzz {\n"
                                "    public:\n"
                                "        std::list<int> x;\n"
                                "    };\n"
                                "}\n"
                                "using namespace N;\n"
                                "int main() {\n"
                                "    Zzz * zzz;\n"
                                "    if (zzz->x.size() > 0) { }\n"
                                "}";
            check(code, settingsCpp03);
            ASSERT_EQUALS(
                "[test.cpp:10:14]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n"
                "[test.cpp:10:14]: (performance) Possible inefficient checking for 'x' emptiness. [stlSize]\n",   // duplicate
                errout_str());

            check(code);
            ASSERT_EQUALS("", errout_str());
        }
    }

    void size4() { // #2652 - don't warn about vector/deque
        check("void f(std::vector<int> &v) {\n"
              "    if (v.size() > 0U) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::deque<int> &v) {\n"
              "    if (v.size() > 0U) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::array<int,3> &a) {\n"
              "    if (a.size() > 0U) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void redundantCondition1() {
        check("void f(string haystack)\n"
              "{\n"
              "    if (haystack.find(needle) != haystack.end())\n"
              "        haystack.remove(needle);"
              "}");
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Redundant checking of STL container element existence before removing it. [redundantIfRemove]\n", errout_str());
    }

    void missingInnerComparison1() {
        check("void f(std::set<int> &ints) {\n"
              "    for (std::set<int>::iterator it = ints.begin(); it != ints.end(); ++it) {\n"
              "        if (a) {\n"
              "            it++;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:13] -> [test.cpp:2:71]: (warning) Missing bounds check for extra iterator increment in loop. [StlMissingComparison]\n", errout_str());

        check("void f(std::map<int,int> &ints) {\n"
              "    for (std::map<int,int>::iterator it = ints.begin(); it != ints.end(); ++it) {\n"
              "        ++it->second;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::vector<std::string> &v) {\n"
              "    for(std::vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it) {\n"
              "        if(it+2 != v.end())\n"
              "        {\n"
              "            ++it;\n"
              "            ++it;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void missingInnerComparison2() {
        check("void f(std::set<int> &ints) {\n"
              "    for (std::set<int>::iterator it = ints.begin(); it != ints.end(); ++it) {\n"
              "        if (a) {\n"
              "            it++;\n"
              "            if (it == ints.end())\n"
              "                return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void missingInnerComparison3() {
        check("void f(std::set<int> &ints) {\n"
              "    for (std::set<int>::iterator it = ints.begin(); it != ints.end(); ++it) {\n"
              "        for (std::set<int>::iterator it = ints2.begin(); it != ints2.end(); ++it)\n"
              "        { }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void missingInnerComparison4() {
        check("function f1(std::list<int> &l1) {\n"
              "    for(std::list<int>::iterator i = l1.begin(); i != l1.end(); i++) {\n"
              "        if (*i == 44) {\n"
              "            l1.insert(++i, 55);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:23]: (style) Consider using std::find_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("function f1(std::list<int> &l1) {\n"
              "    for(std::list<int>::iterator i = l1.begin(); i != l1.end(); i++) {\n"
              "        if (*i == 44) {\n"
              "            l1.insert(++i, 55);\n"
              "            return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:23]: (style) Consider using std::find_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());
    }

    void missingInnerComparison5() {
        check("void f() {\n"
              "    for(it = map1.begin(); it != map1.end(); it++) {\n"
              "        str[i++] = (*it).first;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void missingInnerComparison6() {
        check("void f(std::string &s) {\n"
              "    for(string::iterator it = s.begin(); it != s.end(); it++) {\n"
              "        it = s.insert(++it, 0);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());
    }

    void cstr() {
        check("void f() {\n"
              "    std::string errmsg;\n"
              "    throw errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after throwing exception. [stlcstrthrow]\n", errout_str());

        check("const char *get_msg() {\n"
              "    std::string errmsg;\n"
              "    return errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("const char *get_msg() {\n"
              "    std::ostringstream errmsg;\n"
              "    return errmsg.str().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("const char *get_msg() {\n"
              "    std::string errmsg;\n"
              "    return std::string(\"ERROR: \" + errmsg).c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("const char *get_msg() {\n"
              "    std::string errmsg;\n"
              "    return (\"ERROR: \" + errmsg).c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("const char *get_msg() {\n"
              "    std::string errmsg;\n"
              "    return (\"ERROR: \" + std::string(\"crash me\")).c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("void f() {\n"
              "    std::ostringstream errmsg;\n"
              "    const char *c = errmsg.str().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:17]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("std::string f();\n"
              "\n"
              "void foo() {\n"
              "    const char *c = f().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4:17]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("class Foo {\n"
              "    const char *f();\n"
              "};\n"
              "const char *Foo::f() {\n"
              "    std::string s;\n"
              "    return s.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("class Foo {\n"
              "    std::string GetVal() const;\n"
              "};\n"
              "const char *f() {\n"
              "    Foo f;\n"
              "    return f.GetVal().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("const char* foo() {\n"
              "    static std::string text;\n"
              "    text = \"hello world\\n\";\n"
              "    return text.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout_str()); // #3427

        // Implicit conversion back to std::string
        check("std::string get_msg() {\n"
              "    std::string errmsg;\n"
              "    return errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (performance) Returning the result of c_str() in a function that returns std::string is slow and redundant. [stlcstrReturn]\n", errout_str());

        check("const std::string& get_msg() {\n"
              "    std::string errmsg;\n"
              "    return errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:5]: (performance) Returning the result of c_str() in a function that returns std::string is slow and redundant. [stlcstrReturn]\n", errout_str());

        check("class Foo {\n"
              "    std::string GetVal() const;\n"
              "};\n"
              "std::string f() {\n"
              "    Foo f;\n"
              "    return f.GetVal().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (performance) Returning the result of c_str() in a function that returns std::string is slow and redundant. [stlcstrReturn]\n", errout_str());

        check("std::string get_msg() {\n"
              "    std::string errmsg;\n"
              "    return errmsg;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string get_msg() {\n" // #3678
              "    MyStringClass errmsg;\n"
              "    return errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void Foo1(const std::string& str) {}\n"
              "void Foo2(const char* c, const std::string str) {}\n"
              "void Foo3(std::string& rstr) {}\n"
              "void Foo4(std::string str, const std::string& str) {}\n"
              "void Bar() {\n"
              "    std::string str = \"bar\";\n"
              "    std::stringstream ss(\"foo\");\n"
              "    Foo1(str);\n"
              "    Foo1(str.c_str());\n"
              "    Foo2(str.c_str(), str);\n"
              "    Foo2(str.c_str(), str.c_str());\n"
              "    Foo3(str.c_str());\n"
              "    Foo4(str, str);\n"
              "    Foo4(str.c_str(), str);\n"
              "    Foo4(str, str.c_str());\n"
              "    Foo4(ss.str(), ss.str().c_str());\n"
              "    Foo4(str.c_str(), str.c_str());\n"
              "}");

        ASSERT_EQUALS("[test.cpp:9:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:11:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 2 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:14:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:15:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 2 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:16:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 2 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:17:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:17:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 2 is slow and redundant. [stlcstrParam]\n", errout_str());

        check("void Foo1(const std::string& str) {}\n"
              "void Foo2(char* c, const std::string str) {}\n"
              "void Bar() {\n"
              "    std::string str = \"bar\";\n"
              "    Foo1(str, foo);\n" // Don't crash
              "    Foo2(str.c_str());\n" // Don't crash
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct Foo {\n"
              "    void func(std::string str) const {}\n"
              "    static void sfunc(std::string str) {}\n"
              "};\n"
              "void func(std::string str) {}\n"
              "void Bar() {\n"
              "    std::string str = \"bar\";\n"
              "    Foo foo;\n"
              "    func(str.c_str());\n"
              "    Foo::sfunc(str.c_str());\n"
              "    foo.func(str.c_str());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:10:10]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:11:9]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n",
                      errout_str());

        check("void f(const std::string& s);\n" // #8336
              "struct T {\n"
              "    std::string g();\n"
              "    std::string a[1];\n"
              "    struct U { std::string s; } u;"
              "};\n"
              "namespace N { namespace O { std::string s; } }\n"
              "void g(const std::vector<std::string>& v, T& t) {\n"
              "    for (std::vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it)\n"
              "        f(it->c_str());\n"
              "    f(v.begin()->c_str());\n"
              "    f(t.g().c_str());\n"
              "    f(t.a[0].c_str());\n"
              "    f(t.u.s.c_str());\n"
              "    f(N::O::s.c_str());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9:9]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:10:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:11:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:12:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:13:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n"
                      "[test.cpp:14:5]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant. [stlcstrParam]\n",
                      errout_str());

        check("void svgFile(const std::string &content, const std::string &fileName, const double end = 1000., const double start = 0.);\n"
              "void Bar(std::string filename) {\n"
              "    std::string str = \"bar\";\n"
              "    std::ofstream svgFile(filename.c_str(), std::ios::trunc);\n"
              "    svgFile << \"test\";\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void Foo(const char* p) {}\n"
              "void Foo(const std::string& str) {Foo(str.c_str());}\n" // Overloaded
              "void Bar() {\n"
              "    std::string str = \"bar\";\n"
              "    Foo(str);\n"
              "    Foo(str.c_str());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("int atoi(const std::string& str) {\n" // #3729: Don't suggest recursive call
              "    return atoi(str.c_str());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "\n"
              "const char *f()\n"
              "{\n"
              "    return hello().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("class Fred {\n"
              "    std::string hello();\n"
              "    const char *f();\n"
              "};\n"
              "std::string Fred::hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "const char *Fred::f()\n"
              "{\n"
              "    return hello().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        // #4183 - using MyStringClass.c_str()
        check("void a(const std::string &str);\n"
              "\n"
              "void b() {\n"
              "    MyStringClass s;\n"
              "    a(s.c_str());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("std::string Format(const char * name) {\n" // #4938
              "    return String::Format(\"%s:\", name).c_str();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #7480
        check("struct InternalMapInfo {\n"
              "    std::string author;\n"
              "};\n"
              "const char* GetMapAuthor(int index) {\n"
              "    const InternalMapInfo* mapInfo = &internal_getMapInfo;\n"
              "    return mapInfo->author.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct InternalMapInfo {\n"
              "    std::string author;\n"
              "};\n"
              "std::string GetMapAuthor(int index) {\n"
              "    const InternalMapInfo* mapInfo = &internal_getMapInfo;\n"
              "    return mapInfo->author.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (performance) Returning the result of c_str() in a function that returns std::string is slow and redundant. [stlcstrReturn]\n", errout_str());

        check("struct InternalMapInfo {\n"
              "    std::string author;\n"
              "};\n"
              "const char* GetMapAuthor(int index) {\n"
              "    const InternalMapInfo mapInfo = internal_getMapInfo;\n"
              "    return mapInfo.author.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("struct S {\n" // #7656
              "    std::string data;\n"
              "};\n"
              "const S& getS();\n"
              "const char* test() {\n"
              "    const struct S &s = getS();\n"
              "    return s.data.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #7930
              "    std::string data;\n"
              "};\n"
              "const char* test() {\n"
              "    S s;\n"
              "    std::string &ref = s.data;\n"
              "    return ref.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("void f(const wchar_t* w, int i = 0, ...);\n" // #10357
              "void f(const std::string& s, int i = 0);\n"
              "void g(const std::wstring& p) {\n"
              "    f(p.c_str());\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" //#9161
              "    const char* f() const noexcept {\n"
              "        return (\"\" + m).c_str();\n"
              "    }\n"
              "    std::string m;\n"
              "};\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:9]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call. [stlcstr]\n", errout_str());

        check("struct S {\n" // #10493
              "    void f(const char** pp);\n"
              "    std::string s;\n"
              "};\n"
              "void S::f(const char** pp) {\n"
              "    try {\n"
              "        *pp = member.c_str();\n"
              "    }\n"
              "    catch (...) {\n"
              "        s = \"xyz\";\n"
              "        *pp = member.c_str();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(const std::string& a) {\n"
              "    std::string b(a.c_str());\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:17]: (performance) Constructing a std::string from the result of c_str() is slow and redundant. [stlcstrConstructor]\n", errout_str());

        check("std::string f(const std::string& a) {\n"
              "    std::string b{ a.c_str() };\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:17]: (performance) Constructing a std::string from the result of c_str() is slow and redundant. [stlcstrConstructor]\n", errout_str());

        check("std::string f(const std::string& a) {\n"
              "    std::string b = a.c_str();\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:17]: (performance) Assigning the result of c_str() to a std::string is slow and redundant. [stlcstrAssignment]\n", errout_str());

        check("std::string g(const std::string& a, const std::string& b) {\n"
              "    return a + b.c_str();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:14]: (performance) Concatenating the result of c_str() and a std::string is slow and redundant. [stlcstrConcat]\n", errout_str());

        check("std::string g(const std::string& a, const std::string& b) {\n"
              "    return a.c_str() + b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:22]: (performance) Concatenating the result of c_str() and a std::string is slow and redundant. [stlcstrConcat]\n", errout_str());

        check("std::vector<double> v;\n" // don't crash
              "int i;\n"
              "void f() {\n"
              "    const double* const QM_R__ buf(v.data() + i);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct T {\n" // #7515
              "    std::string g();\n"
              "    std::string a[1];\n"
              "    std::vector<std::string> v;\n"
              "};\n"
              "void f(std::stringstream& strm, const std::string& s, T& t) {\n"
              "    strm << s.c_str();\n"
              "    strm << \"abc\" << s.c_str();\n"
              "    strm << \"abc\" << s.c_str() << \"def\";\n"
              "    strm << \"abc\" << t.g().c_str() << \"def\";\n"
              "    strm << t.a[0].c_str();\n"
              "    strm << t.v.begin()->c_str();\n"
              "    auto it = t.v.begin()\n"
              "    strm << it->c_str();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7:10]: (performance) Passing the result of c_str() to a stream is slow and redundant. [stlcstrStream]\n"
                      "[test.cpp:8:19]: (performance) Passing the result of c_str() to a stream is slow and redundant. [stlcstrStream]\n"
                      "[test.cpp:9:19]: (performance) Passing the result of c_str() to a stream is slow and redundant. [stlcstrStream]\n"
                      "[test.cpp:10:19]: (performance) Passing the result of c_str() to a stream is slow and redundant. [stlcstrStream]\n"
                      "[test.cpp:11:10]: (performance) Passing the result of c_str() to a stream is slow and redundant. [stlcstrStream]\n"
                      "[test.cpp:12:10]: (performance) Passing the result of c_str() to a stream is slow and redundant. [stlcstrStream]\n"
                      "[test.cpp:14:10]: (performance) Passing the result of c_str() to a stream is slow and redundant. [stlcstrStream]\n",
                      errout_str());

        check("void f(const std::string& s) {\n"
              "    std::cout << s.c_str();\n"
              "    std::cerr << s.c_str();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:15]: (performance) Passing the result of c_str() to a stream is slow and redundant. [stlcstrStream]\n"
                      "[test.cpp:3:15]: (performance) Passing the result of c_str() to a stream is slow and redundant. [stlcstrStream]\n",
                      errout_str());

        check("struct S { std::string str; };\n"
              "struct T { S s; };\n"
              "struct U { T t[1]; };\n"
              "void f(const T& t, const U& u, std::string& str) {\n"
              "    if (str.empty())\n"
              "        str = t.s.str.c_str();\n"
              "    else\n"
              "        str = u.t[0].s.str.c_str();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6:9]: (performance) Assigning the result of c_str() to a std::string is slow and redundant. [stlcstrAssignment]\n"
                      "[test.cpp:8:9]: (performance) Assigning the result of c_str() to a std::string is slow and redundant. [stlcstrAssignment]\n",
                      errout_str());

        check("void f(std::string_view);\n" // #11547
              "void g(const std::string & s) {\n"
              "    f(s.c_str());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (performance) Passing the result of c_str() to a function that takes std::string_view as argument no. 1 is slow and redundant. [stlcstrParam]\n",
                      errout_str());

        check("std::string_view f(const std::string& s) {\n"
              "    std::string_view sv = s.c_str();\n"
              "    return sv;\n"
              "}\n"
              "std::string_view g(const std::string& s) {\n"
              "    std::string_view sv{ s.c_str() };\n"
              "    return sv;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:22]: (performance) Assigning the result of c_str() to a std::string_view is slow and redundant. [stlcstrAssignment]\n"
                      "[test.cpp:6:22]: (performance) Constructing a std::string_view from the result of c_str() is slow and redundant. [stlcstrConstructor]\n",
                      errout_str());

        check("void f(const std::string& s) {\n" // #11819
              "    std::string_view sv(s.data(), 13);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("struct S { std::string x; };\n" // #11802
              "std::vector<std::shared_ptr<S>> global;\n"
              "const char* f() {\n"
              "    auto s = std::make_shared<S>();\n"
              "    global.push_back(s);\n"
              "    return s->x.c_str();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(const std::string& s) {\n" // #14533
              "    auto x = std::string(\"abc\") + s.c_str();\n"
              "    auto y = s.c_str() + std::string(\"def\");\n"
              "    return x + y;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:33]: (performance) Concatenating the result of c_str() and a std::string is slow and redundant. [stlcstrConcat]\n"
                      "[test.cpp:3:24]: (performance) Concatenating the result of c_str() and a std::string is slow and redundant. [stlcstrConcat]\n",
                      errout_str());

        check("std::string get();\n"
              "std::string f(const std::string& s) {\n"
              "    return get() + s.c_str();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:18]: (performance) Concatenating the result of c_str() and a std::string is slow and redundant. [stlcstrConcat]\n",
                      errout_str());

        check("std::string get();\n" // #14536
              "    std::string f(const std::string& s) {\n"
              "    return s + get().c_str();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:14]: (performance) Concatenating the result of c_str() and a std::string is slow and redundant. [stlcstrConcat]\n",
                      errout_str());

        check("std::string get();\n"
              "std::string f(std::string & s) {\n"
              "    s = get().c_str();\n"
              "    std::string s2{ get().c_str() };\n"
              "    return s2;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:5]: (performance) Assigning the result of c_str() to a std::string is slow and redundant. [stlcstrAssignment]\n"
                      "[test.cpp:4:17]: (performance) Constructing a std::string from the result of c_str() is slow and redundant. [stlcstrConstructor]\n",
                      errout_str());

        check("void f() {\n"
              "    std::string s;\n"
              "    auto a = + s.c_str();\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(const std::string& a) {\n" // #14600
              "    std::string b(a.c_str() + 1 + 2);\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:17]: (performance) Constructing a std::string from the result of c_str() is slow and redundant. [stlcstrConstructor]\n",
                      errout_str());
    }

    void uselessCalls() {
        check("void f()\n"
              "{\n"
              "    string s1, s2;\n"
              "    s1.swap(s2);\n"
              "    s2.swap(s2);\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    std::string s1, s2;\n"
              "    s1.swap(s2);\n"
              "    s2.swap(s2);\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:5]: (performance) It is inefficient to swap a object with itself by calling 's2.swap(s2)' [uselessCallsSwap]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    std::string s1, s2;\n"
              "    s1.compare(s2);\n"
              "    s2.compare(s2);\n"
              "    s1.compare(s2.c_str());\n"
              "    s1.compare(0, s1.size(), s1);\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:16]: (warning) It is inefficient to call 's2.compare(s2)' as it always returns 0. [uselessCallsCompare]\n", errout_str());

        // #7370 False positive uselessCallsCompare on unknown type
        check("class ReplayIteratorImpl{\n"
              "  int Compare(ReplayIteratorImpl* other) {\n"
              "    int cmp;\n"
              "    int ret = cursor_->compare(cursor_, other->cursor_, &cmp);\n"
              "    return (cmp);\n"
              "  }\n"
              "  WT_CURSOR *cursor_;\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("void f()\n"
              "{\n"
              "    int x=1;\n"
              "    std::string s1, s2;\n"
              "    s1 = s1.substr();\n"
              "    s2 = s1.substr(x);\n"
              "    s1 = s2.substr(0, x);\n"
              "    s1 = s2.substr(0,std::string::npos);\n"
              "    s1 = s2.substr(x+5-n, 0);\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5:10]: (performance) Ineffective call of function \'substr\' because it returns a copy of "
                      "the object. Use operator= instead. [uselessCallsSubstr]\n"
                      "[test.cpp:8:10]: (performance) Ineffective call of function \'substr\' because it returns a copy of "
                      "the object. Use operator= instead. [uselessCallsSubstr]\n"
                      "[test.cpp:9:10]: (performance) Ineffective call of function \'substr\' because it returns an empty string. [uselessCallsSubstr]\n", errout_str());

        check("void f()\n"
              "{\n"
              "    int x=1;\n"
              "    string s1, s2;\n"
              "    s1 = s1.substr();\n"
              "    s2 = s1.substr(x);\n"
              "    s1 = s2.substr(0, x);\n"
              "    s1 = s2.substr(0,std::string::npos);\n"
              "    s1 = s2.substr(x+5-n, 0);\n"
              "};");
        ASSERT_EQUALS("", errout_str());

        check("int main()\n"
              "{\n"
              "    std::string str = \"a1b1\";\n"
              "    return str.find(str[1], 2);\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("bool foo(std::vector<int>& v) {\n"
              "    v.empty();\n"
              "    return v.empty();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Ineffective call of function 'empty()'. Did you intend to call 'clear()' instead? [uselessCallsEmpty]\n", errout_str());

        check("void f() {\n" // #4938
              "    OdString str;\n"
              "    str.empty();\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n" // #4032
              "    const std::string greeting(\"Hello World !!!\");\n"
              "    const std::string::size_type npos = greeting.rfind(\" \");\n"
              "    if (npos != std::string::npos)\n"
              "        std::cout << greeting.substr(0, npos) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int> a) {\n"
              "    std::remove(a.begin(), a.end(), val);\n"
              "    std::remove_if(a.begin(), a.end(), val);\n"
              "    std::unique(a.begin(), a.end(), val);\n"
              "    x = std::remove(a.begin(), a.end(), val);\n"
              "    a.erase(std::remove(a.begin(), a.end(), val));\n"
              "    std::remove(\"foo.txt\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:5]: (warning) Return value of std::remove() ignored. Elements remain in container. [uselessCallsRemove]\n"
                      "[test.cpp:3:5]: (warning) Return value of std::remove_if() ignored. Elements remain in container. [uselessCallsRemove]\n"
                      "[test.cpp:4:5]: (warning) Return value of std::unique() ignored. Elements remain in container. [uselessCallsRemove]\n", errout_str());

        // #4431 - fp
        check("bool f() {\n"
              "    return x ? true : (y.empty());\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #8360
        check("void f(std::string s) {\n"
              "    for (;s.empty();) {}\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // #11166
        check("std::string f(std::string s) {\n"
              "    s = s.substr(0, s.size() - 1);\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9]: (performance) Ineffective call of function 'substr' because a prefix of the string is assigned to itself. Use resize() or pop_back() instead. [uselessCallsSubstr]\n",
                      errout_str());

        check("std::string f(std::string s, std::size_t start, std::size_t end, const std::string& i) {\n"
              "    s = s.substr(0, start) + i + s.substr(end + 1);\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:9]: (performance) Ineffective call of function 'substr' because a prefix of the string is assigned to itself. Use replace() instead. [uselessCallsSubstr]\n",
                      errout_str());

        check("std::string f(std::string s, std::size_t end) {\n"
              "    s = { s.begin(), s.begin() + end };\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (performance) Inefficient constructor call: container 's' is assigned a partial copy of itself. Use erase() or resize() instead. [uselessCallsConstructor]\n",
                      errout_str());

        check("std::list<int> f(std::list<int> l, std::size_t end) {\n"
              "    l = { l.begin(), l.begin() + end };\n"
              "    return l;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (performance) Inefficient constructor call: container 'l' is assigned a partial copy of itself. Use erase() or resize() instead. [uselessCallsConstructor]\n",
                      errout_str());

        check("std::string f(std::string s, std::size_t end) {\n"
              "    s = std::string{ s.begin(), s.begin() + end };\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (performance) Inefficient constructor call: container 's' is assigned a partial copy of itself. Use erase() or resize() instead. [uselessCallsConstructor]\n",
                      errout_str());

        check("std::string f(std::string s, std::size_t end) {\n"
              "    s = std::string(s.begin(), s.begin() + end);\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (performance) Inefficient constructor call: container 's' is assigned a partial copy of itself. Use erase() or resize() instead. [uselessCallsConstructor]\n",
                      errout_str());

        check("std::vector<int> f(std::vector<int> v, std::size_t end) {\n"
              "    v = std::vector<int>(v.begin(), v.begin() + end);\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (performance) Inefficient constructor call: container 'v' is assigned a partial copy of itself. Use erase() or resize() instead. [uselessCallsConstructor]\n",
                      errout_str());
    }

    void stabilityOfChecks() {
        // Stability test: 4684 cppcheck crash in template function call.
        check("template<class T>\n"
              "class EffectivityRangeData {};\n"
              "template<class T>\n"
              "class EffectivityRange {\n"
              "    void unite() {\n"
              "        x < vector < EffectivityRangeData<int >> >();\n"
              "        EffectivityRange<int> er;\n"
              "    }\n"
              "    void shift() { EffectivityRangeData<int>::iterator it;  }\n"
              "};");
        ASSERT_EQUALS("", errout_str());
    }

    void dereferenceInvalidIterator() {
        // Test simplest "if" with && case
        check("void foo(std::string::iterator& i) {\n"
              "    if (std::isalpha(*i) && i != str.end()) {\n"
              "        std::cout << *i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:22]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        check("void foo(std::string::iterator& i) {\n"
              "    if(foo) { bar(); }\n"
              "    else if (std::isalpha(*i) && i != str.end()) {\n"
              "        std::cout << *i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3:27]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        // Test suggested correction doesn't report an error
        check("void foo(std::string::iterator& i) {\n"
              "    if (i != str.end() && std::isalpha(*i)) {\n"
              "        std::cout << *i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Test "while" with "&&" case
        check("void foo(std::string::iterator& i) {\n"
              "    while (std::isalpha(*i) && i != str.end()) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        check("void foo(std::string::iterator& i) {\n"
              "    do {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    } while (std::isalpha(*i) && i != str.end());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5:27]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        // Test "while" with "||" case
        check("void foo(std::string::iterator& i) {\n"
              "    while (!(!std::isalpha(*i) || i == str.end())) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:28]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        // Test fix for "while" with "||" case
        check("void foo(std::string::iterator& i) {\n"
              "    while (!(i == str.end() || !std::isalpha(*i))) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Test "for" with "&&" case
        check("void foo(std::string::iterator& i) {\n"
              "    for (; std::isalpha(*i) && i != str.end() ;) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        // Test "for" with "||" case
        check("void foo(std::string::iterator& i) {\n"
              "    for (; std::isalpha(*i) || i == str.end() ;) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        // Test that a dereference outside the condition part of a "for"
        // loop does not result in a false positive
        check("void foo(std::string::iterator& i) {\n"
              "    for (char c = *i; isRunning && i != str.end() ;) {\n"
              "        std::cout << c;\n"
              "        i ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Test that other "&&" terms in the condition don't invalidate the check
        check("void foo(char* c, std::string::iterator& i) {\n"
              "    if (*c && std::isalpha(*i) && i != str.end()) {\n"
              "        std::cout << *i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:28]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        // Test that dereference of different variable doesn't trigger a false positive
        check("void foo(const char* c, std::string::iterator& i) {\n"
              "    if (std::isalpha(*c) && i != str.end()) {\n"
              "        std::cout << *c;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        // Test case involving "rend()" instead of "end()"
        check("void foo(std::string::iterator& i) {\n"
              "    while (std::isalpha(*i) && i != str.rend()) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2:25]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        // Test that mixed "&&" and "||" don't result in a false positive
        check("void foo(std::string::iterator& i) {\n"
              "    if ((i == str.end() || *i) || (isFoo() && i != str.end())) {\n"
              "        std::cout << \"foo\";\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::vector <int> v;\n"
              "    std::vector <int>::iterator i = v.end();\n"
              "    *i=0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        check("void f() {\n"
              "    std::vector <int> v;\n"
              "    std::vector <int>::iterator i = std::end(v);\n"
              "    *i=0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:6]: (error) Dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        check("void f(std::vector <int> v) {\n"
              "    std::vector <int>::iterator i = v.end();\n"
              "    *i=0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:6]: (error) Dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        check("void f(std::vector <int> v) {\n"
              "    std::vector <int>::iterator i = v.end();\n"
              "    *(i+1)=0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:8]: (error) Dereference of an invalid iterator: i+1 [derefInvalidIterator]\n", errout_str());

        check("void f(std::vector <int> v) {\n"
              "    std::vector <int>::iterator i = v.end();\n"
              "    *(i-1)=0;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector <int> v) {\n"
              "    std::vector <int>::iterator i = v.begin();\n"
              "    *(i-1)=0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:8]: (error) Dereference of an invalid iterator: i-1 [derefInvalidIterator]\n", errout_str());

        check("void f(std::vector <int> v) {\n"
              "    std::vector <int>::iterator i = std::begin(v);\n"
              "    *(i-1)=0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:8]: (error) Dereference of an invalid iterator: i-1 [derefInvalidIterator]\n", errout_str());

        check("void f(std::vector <int> v, bool b) {\n"
              "    std::vector <int>::iterator i = v.begin();\n"
              "    if (b)\n"
              "        i = v.end();\n"
              "    *i=0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:6]: (warning) Possible dereference of an invalid iterator: i [derefInvalidIterator]\n", errout_str());

        check("void f(std::vector <int> v, bool b) {\n"
              "    std::vector <int>::iterator i = v.begin();\n"
              "    if (b)\n"
              "        i = v.end();\n"
              "    *(i+1)=0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:8]: (warning) Possible dereference of an invalid iterator: i+1 [derefInvalidIterator]\n", errout_str());

        check("void f(std::vector <int> v, bool b) {\n"
              "    std::vector <int>::iterator i = v.begin();\n"
              "    if (b)\n"
              "        i = v.end();\n"
              "    *(i-1)=0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5:8]: (warning) Possible dereference of an invalid iterator: i-1 [derefInvalidIterator]\n", errout_str());

        check("int f(std::vector<int> v, int pos) {\n"
              "    if (pos >= 0)\n"
              "        return *(v.begin() + pos);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(std::vector<int> v, int i) {\n"
              "    auto it = std::find(v.begin(), v.end(), i);\n"
              "    if (it != v.end()) {}\n"
              "    return *it;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:12] -> [test.cpp:4:13]: (warning) Either the condition 'it!=v.end()' is redundant or there is possible dereference of an invalid iterator: it. [derefInvalidIteratorRedundantCheck]\n", errout_str());

        check("void f(std::vector<int> & v) {\n"
              "    std::vector<int>::iterator i= v.begin();\n"
              "    if(i == v.end() && *(i+1) == *i) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:3:27]: (warning) Either the condition 'i==v.end()' is redundant or there is possible dereference of an invalid iterator: i+1. [derefInvalidIteratorRedundantCheck]\n"
                      "[test.cpp:3:10] -> [test.cpp:3:35]: (warning) Either the condition 'i==v.end()' is redundant or there is possible dereference of an invalid iterator: i. [derefInvalidIteratorRedundantCheck]\n", errout_str());


        check("void f(std::vector<int> & v) {\n"
              "    std::vector<int>::iterator i= v.begin();\n"
              "    if(i == v.end() && *i == *(i+1)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:3:25]: (warning) Either the condition 'i==v.end()' is redundant or there is possible dereference of an invalid iterator: i. [derefInvalidIteratorRedundantCheck]\n"
                      "[test.cpp:3:10] -> [test.cpp:3:33]: (warning) Either the condition 'i==v.end()' is redundant or there is possible dereference of an invalid iterator: i+1. [derefInvalidIteratorRedundantCheck]\n", errout_str());

        check("void f(std::vector<int> & v) {\n"
              "    std::vector<int>::iterator i= v.begin();\n"
              "    if(i != v.end() && *i == *(i+1)) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:3:33]: (warning) Either the condition 'i!=v.end()' is redundant or there is possible dereference of an invalid iterator: i+1. [derefInvalidIteratorRedundantCheck]\n", errout_str());

        check("void f(std::vector<int> & v) {\n"
              "    std::vector<int>::iterator i= v.begin();\n"
              "    if(i != v.end()) {\n"
              "        if (*(i+1) == *i) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:4:16]: (warning) Either the condition 'i!=v.end()' is redundant or there is possible dereference of an invalid iterator: i+1. [derefInvalidIteratorRedundantCheck]\n", errout_str());

        check("void f(std::vector<int> & v) {\n"
              "    std::vector<int>::iterator i= v.begin();\n"
              "    if(i == v.end()) { return; }\n"
              "    if (*(i+1) == *i) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:10] -> [test.cpp:4:12]: (warning) Either the condition 'i==v.end()' is redundant or there is possible dereference of an invalid iterator: i+1. [derefInvalidIteratorRedundantCheck]\n", errout_str());

        check("void f(std::vector<int> & v) {\n"
              "    std::vector<int>::iterator i= v.begin();\n"
              "    if(i != v.end() && (i+1) != v.end() && *(i+1) == *i) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string s) {\n"
              "    for (std::string::const_iterator i = s.begin(); i != s.end(); ++i) {\n"
              "        if (i != s.end() && (i + 1) != s.end() && *(i + 1) == *i) {\n"
              "            if (!isalpha(*(i + 2))) {\n"
              "                std::string modifier;\n"
              "                modifier += *i;\n"
              "                modifier += *(i + 1);\n"
              "            }\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:37] -> [test.cpp:4:30]: (warning) Either the condition '(i+1)!=s.end()' is redundant or there is possible dereference of an invalid iterator: i+2. [derefInvalidIteratorRedundantCheck]\n", errout_str());

        check("void f(int v, std::map<int, int> &items) {\n"
              "    for (auto it = items.begin(); it != items.end();)\n"
              "        (it->first == v) ? it = items.erase(it) : ++it;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("void f(std::string s) {\n"
              "    for (std::string::const_iterator i = s.begin(); i != s.end(); ++i) {\n"
              "        if (i != s.end() && (i + 1) != s.end() && *(i + 1) == *i) {\n"
              "            if ((i + 2) != s.end() && !isalpha(*(i + 2))) {\n"
              "                std::string modifier;\n"
              "                modifier += *i;\n"
              "                modifier += *(i + 1);\n"
              "            }\n"
              "        }\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(std::vector<int>::iterator it, const std::vector<int>& vector) {\n"
              "    if (!(it != vector.end() && it != vector.begin()))\n"
              "        throw std::out_of_range();\n"
              "    if (it != vector.end() && *it == 0)\n"
              "        return -1;\n"
              "    return *it;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(std::vector<int> &vect) {\n"
              "    const int &v = *vect.emplace(vect.end());\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("extern bool bar(int);\n"
              "void f(std::vector<int> & v) {\n"
              "    std::vector<int>::iterator i= v.begin();\n"
              "    if(i == v.end() && bar(*(i+1)) ) {}\n"
              "}\n");
        ASSERT_EQUALS(
            "[test.cpp:4:10] -> [test.cpp:4:31]: (warning) Either the condition 'i==v.end()' is redundant or there is possible dereference of an invalid iterator: i+1. [derefInvalidIteratorRedundantCheck]\n",
            errout_str());

        // #10657
        check("std::list<int> mValues;\n"
              "typedef std::list<int>::iterator ValueIterator;\n"
              "void foo(ValueIterator beginValue, ValueIterator endValue) {\n"
              "    ValueIterator prevValue = beginValue;\n"
              "    ValueIterator curValue = beginValue;\n"
              "    for (++curValue; prevValue != endValue && curValue != mValues.end(); ++curValue) {\n"
              "        a = bar(*curValue);\n"
              "        prevValue = curValue;\n"
              "    }\n"
              "    if (endValue == mValues.end()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10642
        check("int f(std::vector<int> v) {\n"
              "    return *(v.begin() + v.size() - 1);\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #10716
        check("struct a;\n"
              "class b {\n"
              "  void c(std::map<std::string, a *> &);\n"
              "  std::string d;\n"
              "  std::map<std::string, std::set<std::string>> e;\n"
              "};\n"
              "void b::c(std::map<std::string, a *> &) {\n"
              "  e.clear();\n"
              "  auto f = *e[d].begin();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9:23]: (error) Out of bounds access in expression 'e[d].begin()' because 'e[d]' is empty. [containerOutOfBounds]\n",
                      errout_str());

        // #10151
        check("std::set<int>::iterator f(std::set<int>& s) {\n"
              "for (auto it = s.begin(); it != s.end(); ++it)\n"
              "    if (*it == 42)\n"
              "        return s.erase(it);\n"
              "    return s.end();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:0]: (style) Consider using std::find_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        // #11381
        check("int f(std::map<int, int>& map) {\n"
              "    auto it = map.find(1);\n"
              "    if (it == map.end()) {\n"
              "        bool bInserted;\n"
              "        std::tie(it, bInserted) = map.emplace(1, 42);\n"
              "    }\n"
              "    return debug_valueflow(it)->second;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #11557
        check("bool f(const std::vector<int*>& v, std::vector<int*>::iterator it, bool b) {\n"
              "    if (it == v.end())\n"
              "        return false;\n"
              "    if (b && ((it + 1) == v.end() || (*(it + 1)) != nullptr))\n"
              "        return false;\n"
              "    return true;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        // #6925
        check("void f(const std::string& s, std::string::iterator i) {\n"
              "    if (i != s.end() && *(i + 1) == *i) {\n"
              "        if (i + 1 != s.end()) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3:19] -> [test.cpp:2:29]: (warning) Either the condition 'i+1!=s.end()' is redundant or there is possible dereference of an invalid iterator: i+1. [derefInvalidIteratorRedundantCheck]\n",
                      errout_str());

        check("void f(bool b, std::vector<std::string> v) {\n" // #12680
              "    if (!v.empty()) {\n"
              "        auto it = v.begin();\n"
              "        if (b) {\n"
              "            v.clear();\n"
              "            it = v.begin();\n"
              "        }\n"
              "        for (++it; it != v.end(); ++it) {}\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str()); // don't crash

        check("void f(int);\n" // #13064
              "void g() {\n"
              "    std::vector<int> v{ 0 };\n"
              "    auto it = std::find(v.begin(), v.end(), 0);\n"
              "    if (it == v.end()) {\n"
              "        f({});\n"
              "        it = v.begin();\n"
              "    }\n"
              "    *it;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int g() {\n" // #13332
              "    const std::vector<int> v = { 1, 2, 3, 4 };\n"
              "    const std::vector<int>::const_iterator a[2] = { v.begin(), v.end() };\n"
              "    return *a[0];\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void dereferenceInvalidIterator2() {
        // Self-implemented iterator class
        check("class iterator {\n"
              "public:\n"
              "    CCommitPointer m_ptr;\n"
              "    iterator() {}\n"
              "    CCommitPointer& operator*() {\n"
              "        return m_ptr;\n"
              "    }\n"
              "    CCommitPointer* operator->() {\n"
              "        return &m_ptr;\n"
              "    }\n"
              "    iterator& operator++() {\n"
              "        ++m_ptr.m_place;\n"
              "        return *this;\n"
              "    }\n"
              "    };\n"
              "    iterator begin() {\n"
              "    iterator it;\n"
              "    it->m_place = 0;\n"
              "    return it;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:18:5]: (error, inconclusive) Invalid iterator 'it' used. [eraseDereference]\n", errout_str());

        check("int f(const std::vector<int>& v) {\n" // #11895
              "    auto it = v.end();\n"
              "    std::advance(it, -2);\n"
              "    return *it;\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void loopAlgoElementAssign() {
        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x = 1;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Consider using std::fill algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x = x + 1;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Consider using std::transform algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo(int a, int b) {\n"
              "    for(int& x:v)\n"
              "        x = a + b;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Consider using std::fill or std::generate algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x += 1;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Consider using std::transform algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x = f();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Consider using std::generate algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    for(int& x:v) {\n"
              "        f();\n"
              "        x = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    for(int& x:v) {\n"
              "        x = 1;\n"
              "        f();\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // There should probably be a message for unconditional break
        check("void foo() {\n"
              "    for(int& x:v) {\n"
              "        x = 1;\n"
              "        break;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x = ++x;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void loopAlgoAccumulateAssign() {
        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n += x;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = n + x;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n += 1;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::distance algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = n + 1;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::distance algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool f(int);\n"
              "void foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v)\n"
              "        b &= f(x);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool f(int);\n"
              "void foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v)\n"
              "        b |= f(x);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool f(int);\n"
              "void foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v)\n"
              "        b = b && f(x);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool f(int);\n"
              "void foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v)\n"
              "        b = b || f(x);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int& x:v)\n"
              "        n = ++x;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("std::size_t f(const std::map<std::string, std::size_t>& m) {\n" // #10412
              "    std::size_t t = 0;\n"
              "    for (std::map<std::string, std::size_t>::const_iterator i = m.begin(); i != m.end(); ++i) {\n"
              "        t += i->second;\n"
              "    }\n"
              "    for (std::map<std::string, std::size_t>::const_iterator i = m.begin(); i != m.end(); i++) {\n"
              "        t += i->second;\n"
              "    }\n"
              "    return t; \n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]\n"
                      "[test.cpp:7:11]: (style) Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]\n",
                      errout_str());

        check("int g(const std::vector<int>& v) {\n"
              "    int t = 0;\n"
              "    for (auto i = v.begin(); i != v.end(); ++i) {\n"
              "        t += *i;\n"
              "    }\n"
              "    return t;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("auto g(const std::vector<int>& v) {\n"
              "    std::vector<std::vector<int>::iterator> r;\n"
              "    for (auto i = v.begin(); i != v.end(); ++i) {\n"
              "        r.push_back(i);\n"
              "    }\n"
              "    return r;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(std::vector<std::string> v) {\n"
              "    std::string ret;\n"
              "    for (const std::string& s : v)\n"
              "        ret += s + '\\n';\n"
              "    return ret;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("std::string f(const std::string& s) {\n"
              "    std::string ret;\n"
              "    for (char c : s)\n"
              "        if (c != ' ')\n"
              "            ret += i;\n"
              "    return ret;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(const std::vector<int>& v) {\n"
              "    int sum = 0;\n"
              "    for (auto it = v.begin(); it != v.end(); it += 2)\n"
              "        sum += *it;\n"
              "    return sum;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(const std::vector<int>& v) {\n" // #12900
              "    int x{};\n"
              "    for (const auto i : v)\n"
              "        x = dostuff(i);\n"
              "    return x;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(const std::vector<int>& v) {\n" // #11493
              "    int s = 0;\n"
              "    for (std::size_t i = 0; i < v.size(); ++i)\n"
              "        s += v[i];\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("int f(int n) {\n"
              "    int s = 0;\n"
              "    for (int i = 0; i < n; ++i)\n"
              "        s += g(i);\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int g(int);\n"
              "int f(const std::vector<int>&v, int n) {\n"
              "    int s = 0;\n"
              "    for (int i = 0; i < n; ++i) {\n"
              "        s += g(i) + v[i];\n"
              "    }\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void loopAlgoContainerInsert() {
        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(x);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::copy algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(f(x));\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::transform algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(x + 1);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::transform algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_front(x);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::copy algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_front(f(x));\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::transform algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_front(x + 1);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::transform algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(v);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(0);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void loopAlgoIncrement() {
        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n++;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:9]: (style) Consider using std::distance algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        ++n;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::distance algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x++;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:9]: (style) Consider using std::transform algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        ++x;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:11]: (style) Consider using std::transform algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());
    }

    void loopAlgoConditional() {
        check("bool pred(int x);\n"
              "void foo() {\n"
              "    for(int& x:v) {\n"
              "        if (pred(x)) {\n"
              "            x = 1;\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:15]: (style) Consider using std::replace_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            n += x;\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:6:15]: (style) Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            n += 1;\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:6:15]: (style) Consider using std::count_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            n++;\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:6:13]: (style) Consider using std::count_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    for(int& x:v) {\n"
              "        if (pred(x)) {\n"
              "            x = x + 1;\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:15]: (style) Consider using std::transform algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            c.push_back(x);\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:6:14]: (style) Consider using std::copy_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            return false;\n"
              "        }\n"
              "    }\n"
              "    return true;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:5]: (style) Consider using std::all_of or std::none_of algorithm instead of a raw loop. [useStlAlgorithm]\n",
                      errout_str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "    return true;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:22]: (style) Consider using std::any_of algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool pred(int x);\n"
              "void f();\n"
              "void foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            f();\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:22]: (style) Consider using std::any_of algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool pred(int x);\n"
              "void f(int x);\n"
              "void foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            f(x);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:22]: (style) Consider using std::find_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            b = true;\n"
              "        }\n"
              "    }\n"
              "    if(b) {}\n"
              "    return true;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            b |= true;\n"
              "        }\n"
              "    }\n"
              "    return true;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            b &= true;\n"
              "        }\n"
              "    }\n"
              "    return true;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            return false;\n"
              "        }\n"
              "        return true;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // There is no transform_if
        check("bool pred(int x);\n"
              "void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            c.push_back(x + 1);\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    for(int& x:v) {\n"
              "        x++;\n"
              "        if (pred(x)) {\n"
              "            x = 1;\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool pred(int x);\n"
              "void f();\n"
              "void foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            if(x) { return; }\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool g(int);\n"
              "int f(const std::vector<int>& v) {\n"
              "    int ret = 0;\n"
              "    for (const auto i : v)\n"
              "        if (!g(i))\n"
              "            ret = 1;\n"
              "    return ret;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("int f(const std::vector<int>& v) {\n"
              "    int ret = 0;\n"
              "    for (const auto i : v)\n"
              "        if (i < 5)\n"
              "            ret = 1;\n"
              "    return ret;\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (style) Consider using std::any_of, std::all_of, std::none_of algorithm instead of a raw loop.\n",
                           "",
                           errout_str());

        check("int f(const std::vector<int>& v) {\n"
              "    int ret = 0;\n"
              "    for (const auto i : v)\n"
              "        if (i < 5) {\n"
              "            ret = 1;\n"
              "            break;\n"
              "        }\n"
              "    return ret;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4:20]: (style) Consider using std::any_of algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("struct T {\n"
              "    std::vector<int> v0, v1;\n"
              "    void g();\n"
              "};\n"
              "void T::g() {\n"
              "    for (std::vector<int>::const_iterator it0 = v0.cbegin(); it0 != v0.cend(); ++it0) {\n"
              "        std::vector<int>::iterator it1;\n"
              "        for (it1 = v1.begin(); it1 != v1.end(); ++it1)\n"
              "            if (*it0 == *it1)\n"
              "                break;\n"
              "        if (it1 != v1.end())\n"
              "            v1.erase(it1);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9:0]: (style) Consider using std::find_if algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("bool f(const std::set<std::string>& set, const std::string& f) {\n" // #11595
              "    for (const std::string& s : set) {\n"
              "        if (f.length() >= s.length() && f.compare(0, s.length(), s) == 0) {\n"
              "            return true;\n"
              "        }\n"
              "    }\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Consider using std::any_of algorithm instead of a raw loop. [useStlAlgorithm]\n",
                      errout_str());

        check("void f() {\n" // #12064
              "    for (const auto& animal : { \"cat\", \"bat\", \"tiger\", \"rat\" })\n"
              "        if (std::strlen(animal) > 4)\n"
              "            throw 1;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool f(const std::vector<int>& v, const std::vector<int>& w, int n) {\n"
              "    for (int i = 0; i < n; ++i)\n"
              "        if (v[i] == w[i])\n"
              "            return true;\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool f(int n) {\n"
              "    for (int i = 0; i < n; ++i)\n"
              "        if (g(i))\n"
              "            return true;\n"
              "    return false;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool g(int);\n"
              "bool f(const std::vector<int>&v, int n) {\n"
              "    bool b{};\n"
              "    for (int i = 0; i < n; ++i) {\n"
              "        if (v[i] > 0 && g(i)) {\n"
              "            b = true;\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "    return b;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());

        check("bool get_index(const std::vector<int>& items, int value, size_t& index) {\n" // #14047
              "    size_t current_index = 0;\n"
              "    for (size_t i = 0; i < items.size(); ++i) {\n"
              "        if (items[i] == value) {\n"
              "            if (current_index != index) {\n"
              "                current_index++;\n"
              "                continue;\n"
              "            }\n"
              "            index = i;\n"
              "            return true;\n"
              "        }\n"
              "    }\n"
              "    return false;\n"
              "}\n"
              "int main() {\n"
              "    std::vector<int> items{ 0, 1, 1, 1 };\n"
              "    size_t index = 2;\n"
              "    get_index(items, 1, index);\n"
              "    return index;\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }

    void loopAlgoMinMax() {
        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x > n ? x : n;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::max_element algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x < n ? x : n;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::min_element algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x > n ? n : x;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::min_element algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x < n ? n : x;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::max_element algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void foo(int m) {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x > m ? x : n;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:11]: (style) Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("void f(const std::vector<int>& v) {\n" // #9091
              "    int maxY = 0;\n"
              "    for (int y : v) {\n"
              "        if (y > maxY)\n"
              "            maxY = y;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:18]: (style) Consider using std::max_element algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("int f(const std::vector<int>& v) {\n"
              "    int minY = 0;\n"
              "    for (int y : v) {\n"
              "        if (y < minY)\n"
              "            minY = y;\n"
              "    }\n"
              "    return minY;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:18]: (style) Consider using std::min_element algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("int f(const std::vector<int>& v) {\n"
              "    int max = 0;\n"
              "    for (int i = 0; i < n; ++i)\n"
              "        max = v[i] > max ? v[i] : max;\n"
              "    return max;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Consider using std::max_element algorithm instead of a raw loop. [useStlAlgorithm]\n",
                      errout_str());

        check("int f(const std::vector<int>& v) {\n"
              "    int min = 0;\n"
              "    for (int i = 0; i < n; ++i)\n"
              "        min = v[i] < min ? v[i] : min;\n"
              "    return min;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:13]: (style) Consider using std::min_element algorithm instead of a raw loop. [useStlAlgorithm]\n",
                      errout_str());
    }

    void loopAlgoMultipleReturn()
    {
        check("bool f(const std::vector<int>& v) {\n"
              "    for (auto i : v) {\n"
              "        if (i < 0)\n"
              "            continue;\n"
              "        if (i)\n"
              "            return true;\n"
              "    }\n"
              "    return false;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Consider using std::any_of algorithm instead of a raw loop. [useStlAlgorithm]\n",
                      errout_str());

        check("bool g(const std::vector<int>& v) {\n"
              "    for (auto i : v) {\n"
              "        if (i % 5 == 0)\n"
              "            return true;\n"
              "        if (i % 7 == 0)\n"
              "            return true;\n"
              "    }\n"
              "    return false;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Consider using std::any_of algorithm instead of a raw loop. [useStlAlgorithm]\n",
                      errout_str());

        check("bool g(const std::vector<int>& v) {\n"
              "    for (auto i : v) {\n"
              "        if (i % 5 == 0)\n"
              "            return false;\n"
              "        if (i % 7 == 0)\n"
              "            return true;\n"
              "    }\n"
              "    return false;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool g(std::vector<int>& v) {\n"
              "    for (auto& i : v) {\n"
              "        if (i % 5 == 0)\n"
              "            return false;\n"
              "        if (i % 7 == 0)\n"
              "            i++;\n"
              "    }\n"
              "    return false;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool g(const std::vector<int>& v, int& j) {\n"
              "    for (auto i : v) {\n"
              "        if (i % 5 == 0)\n"
              "            return false;\n"
              "        if (i % 7 == 0)\n"
              "            j++;\n"
              "    }\n"
              "    return false;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool g(const std::vector<int>& v, int& j) {\n"
              "    for (auto i : v) {\n"
              "        int& k = j;\n"
              "        if (i % 5 == 0)\n"
              "            return false;\n"
              "        if (i % 7 == 0)\n"
              "            k++;\n"
              "    }\n"
              "    return false;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool g(const std::vector<int>& v, int& j) {\n"
              "    for (auto i : v) {\n"
              "        int* k = &j;\n"
              "        if (i % 5 == 0)\n"
              "            return false;\n"
              "        if (i % 7 == 0)\n"
              "            (*k)++;\n"
              "    }\n"
              "    return false;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("bool g(const std::vector<int>& v, int j) {\n"
              "    for (auto i : v) {\n"
              "        int k = j;\n"
              "        if (i % 5 == 0)\n"
              "            return false;\n"
              "        if (i % 7 == 0)\n"
              "            k++;\n"
              "    }\n"
              "    return false;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:5]: (style) Consider using std::all_of or std::none_of algorithm instead of a raw loop. [useStlAlgorithm]\n",
                      errout_str());

        check("class C {\n"
              "private:\n"
              "    QString s;\n"
              "public:\n"
              "    C(QString);\n"
              "private:\n"
              "    void f() {\n"
              "        QVERIFY(QDir(s).exists());\n"
              "    }\n"
              "    void f(const QStringList& d) {\n"
              "        for (QString f : d)\n"
              "          QDir(s);\n"
              "    }\n"
              "};\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void invalidContainer() {
        check("void f(std::vector<int> &v) {\n"
              "    auto v0 = v.begin();\n"
              "    v.push_back(123);\n"
              "    std::cout << *v0 << std::endl;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:1:27] -> [test.cpp:2:22] -> [test.cpp:3:7] -> [test.cpp:1:26] -> [test.cpp:4:19]: (error) Using iterator to local container 'v' that may be invalid. [invalidContainer]\n", errout_str());

        check("std::string e();\n"
              "void a() {\n"
              "  std::vector<std::string> b;\n"
              "  for (std::vector<std::string>::const_iterator c; c != b.end(); ++c) {\n"
              "    std::string f = e();\n"
              "    std::string::const_iterator d = f.begin();\n"
              "    if (d != f.end()) {}\n"
              "  }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int> &v) {\n"
              "    int *v0 = &v[0];\n"
              "    v.push_back(123);\n"
              "    std::cout << (*v0)[0] << std::endl;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:1:27] -> [test.cpp:2:15] -> [test.cpp:3:7] -> [test.cpp:1:26] -> [test.cpp:4:20]: (error) Using pointer to local variable 'v' that may be invalid. [invalidContainer]\n", errout_str());

        check("void f() {\n"
              "    std::vector<int> v = {1};\n"
              "    int &v0 = v.front();\n"
              "    v.push_back(123);\n"
              "    std::cout << v0 << std::endl;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:3:13] -> [test.cpp:3:17] -> [test.cpp:4:7] -> [test.cpp:5:18]: (error) Reference to v that may be invalid. [invalidContainerReference]\n",
            errout_str());

        check("void f() {\n"
              "    std::vector<int> v = {1};\n"
              "    int &v0 = v[0];\n"
              "    v.push_back(123);\n"
              "    std::cout << v0 << std::endl;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:13] -> [test.cpp:4:7] -> [test.cpp:5:18]: (error) Reference to v that may be invalid. [invalidContainerReference]\n",
                      errout_str());

        check("void f(std::vector<int> &v) {\n"
              "    int &v0 = v.front();\n"
              "    v.push_back(123);\n"
              "    std::cout << v0 << std::endl;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:2:13] -> [test.cpp:2:17] -> [test.cpp:1:27] -> [test.cpp:3:7] -> [test.cpp:4:18]: (error) Reference to v that may be invalid. [invalidContainerReference]\n",
            errout_str());

        check("void f(std::vector<int> &v) {\n"
              "    int &v0 = v[0];\n"
              "    v.push_back(123);\n"
              "    std::cout << v0 << std::endl;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:2:13] -> [test.cpp:1:27] -> [test.cpp:3:7] -> [test.cpp:4:18]: (error) Reference to v that may be invalid. [invalidContainerReference]\n",
            errout_str());

        check("void f(std::vector<int> &v) {\n"
              "    std::vector<int> *v0 = &v;\n"
              "    v.push_back(123);\n"
              "    std::cout << (*v0)[0] << std::endl;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("const std::vector<int> * g(int);\n"
              "void f() {\n"
              "    const std::vector<int> *v = g(1);\n"
              "    if (v && v->size() == 1U) {\n"
              "        const int &m = v->front();\n"
              "    }\n"
              "\n"
              "    v = g(2);\n"
              "    if (v && v->size() == 1U) {\n"
              "        const int &m = v->front();\n"
              "        if (m == 0) {}\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("std::vector<std::string> g();\n"
              "void f() {\n"
              "    std::vector<std::string> x = g();\n"
              "    const std::string& y = x[1];\n"
              "    std::string z;\n"
              "    z += \"\";\n"
              "    z += y;\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<char> v)\n"
              "{\n"
              "    auto *cur = v.data();\n"
              "    auto *end = cur + v.size();\n"
              "    while (cur < end) {\n"
              "        v.erase(v.begin(), FindNext(v));\n"
              "        cur = v.data();\n"
              "        end = cur + v.size();\n"
              "    }\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #9598
        check("void f(std::vector<std::string> v) {\n"
              "    for (auto it = v.begin(); it != v.end(); it = v.erase(it))\n"
              "        *it;\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #9714
        check("void f() {\n"
              "  auto v = std::vector<std::string>();\n"
              "  std::string x;\n"
              "  v.push_back(x.insert(0, \"x\"));\n"
              "  v.push_back(\"y\");\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #9783
        check("std::string GetTaskIDPerUUID(int);\n"
              "void InitializeJumpList(CString s);\n"
              "void foo() {\n"
              "    CString sAppID = GetTaskIDPerUUID(123).c_str();\n"
              "    InitializeJumpList(sAppID);\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
        // #9796
        check("struct A {};\n"
              "void f() {\n"
              "    std::vector<A *> v;\n"
              "    A *a = new A();\n"
              "    v.push_back(a);\n"
              "    A *b = v.back();\n"
              "    v.pop_back();\n"
              "    delete b;\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct A {};\n"
              "void f() {\n"
              "    std::vector<A *, std::allocator<A*>> v;\n"
              "    A *a = new A();\n"
              "    v.push_back(a);\n"
              "    A *b = v.back();\n"
              "    v.pop_back();\n"
              "    delete b;\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct A {};\n"
              "void f() {\n"
              "    std::vector<std::shared_ptr<A>> v;\n"
              "    std::shared_ptr<A> a = std::make_shared<A>();\n"
              "    v.push_back(a);\n"
              "    std::shared_ptr<A> b = v.back();\n"
              "    v.pop_back();\n"
              "    delete b;\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #9780
        check("int f() {\n"
              "    std::vector<int> vect;\n"
              "    MyStruct info{};\n"
              "    info.vect = &vect;\n"
              "    vect.push_back(1);\n"
              "    return info.ret;\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #9133
        check("struct Fred {\n"
              "    std::vector<int> v;\n"
              "    void foo();\n"
              "    void bar();\n"
              "};\n"
              "void Fred::foo() {\n"
              "    std::vector<int>::iterator it = v.begin();\n"
              "    bar();\n"
              "    it++;\n"
              "}\n"
              "void Fred::bar() {\n"
              "    v.push_back(0);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:7:44] -> [test.cpp:8:5] -> [test.cpp:12:7] -> [test.cpp:9:5]: (error) Using iterator to member container 'v' that may be invalid. [invalidContainer]\n",
            errout_str());

        check("void foo(std::vector<int>& v) {\n"
              "    std::vector<int>::iterator it = v.begin();\n"
              "    bar(v);\n"
              "    it++;\n"
              "}\n"
              "void bar(std::vector<int>& v) {\n"
              "    v.push_back(0);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:1:29] -> [test.cpp:2:44] -> [test.cpp:3:5] -> [test.cpp:7:7] -> [test.cpp:1:28] -> [test.cpp:4:5]: (error) Using iterator to local container 'v' that may be invalid. [invalidContainer]\n",
            errout_str());

        // #10264
        check("void f(std::vector<std::string>& x) {\n"
              "  struct I {\n"
              "    std::vector<std::string> *px{};\n"
              "  };\n"
              "  I i = { &x };\n"
              "  x.clear();\n"
              "  Parse(i);\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "  std::string x;\n"
              "  struct V {\n"
              "    std::string* pStr{};\n"
              "  };\n"
              "  struct I {\n"
              "    std::vector<V> v;\n"
              "  };\n"
              "  I b[] = {{{{ &x }}}};\n"
              "  x = \"Arial\";\n"
              "  I cb[1];\n"
              "  for (long i = 0; i < 1; ++i)\n"
              "    cb[i] = b[i];\n"
              "}\n",dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #9836
        check("void f() {\n"
              "    auto v = std::vector<std::vector<std::string> >{ std::vector<std::string>{ \"hello\" } };\n"
              "    auto p = &(v.at(0).at(0));\n"
              "    v.clear();\n"
              "    std::cout << *p << std::endl;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:3:24] -> [test.cpp:3:18] -> [test.cpp:3:14] -> [test.cpp:4:7] -> [test.cpp:2:10] -> [test.cpp:5:19]: (error) Using pointer to local variable 'v' that may be invalid. [invalidContainer]\n",
            errout_str());

        check("struct A {\n"
              "    const std::vector<int>* i;\n"
              "    A(const std::vector<int>& v)\n"
              "    : i(&v)\n"
              "    {}\n"
              "};\n"
              "int f() {\n"
              "    std::vector<int> v;\n"
              "    A a{v};\n"
              "    v.push_back(1);\n"
              "    return a.i->front();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    const std::vector<int>* i;\n"
              "    A(const std::vector<int>& v)\n"
              "    : i(&v)\n"
              "    {}\n"
              "};\n"
              "void g(const std::vector<int>& v);\n"
              "void f() {\n"
              "    std::vector<int> v;\n"
              "    A a{v};\n"
              "    v.push_back(1);\n"
              "    g(a);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #10984
        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    auto g = [&v]{};\n"
              "    v.push_back(1);\n"
              "    g();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int> v) {\n"
              "    auto it = v.begin();\n"
              "    auto g = [&]{ std::cout << *it << std::endl;};\n"
              "    v.push_back(1);\n"
              "    g();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:2:22] -> [test.cpp:3:33] -> [test.cpp:4:7] -> [test.cpp:1:25] -> [test.cpp:5:5]: (error) Using iterator to local container 'v' that may be invalid. [invalidContainer]\n",
            errout_str());

        check("void f(std::vector<int> v) {\n"
              "    auto it = v.begin();\n"
              "    auto g = [=]{ std::cout << *it << std::endl;};\n"
              "    v.push_back(1);\n"
              "    g();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:2:22] -> [test.cpp:4:7] -> [test.cpp:1:25] -> [test.cpp:5:5]: (error) Using iterator to local container 'v' that may be invalid. [invalidContainer]\n",
            errout_str());

        check("struct A {\n"
              "    int* p;\n"
              "    void g();\n"
              "};\n"
              "void f(std::vector<int> v) {\n"
              "    auto it = v.begin();\n"
              "    A a{v.data()};\n"
              "    v.push_back(1);\n"
              "    a.g();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:7:15] -> [test.cpp:8:7] -> [test.cpp:5:25] -> [test.cpp:9:5]: (error) Using object that points to local variable 'v' that may be invalid. [invalidContainer]\n",
            errout_str());

        check("struct A {\n"
              "    int*& p;\n"
              "    void g();\n"
              "};\n"
              "void f(std::vector<int> v) {\n"
              "    auto* p = v.data();\n"
              "    A a{p};\n"
              "    v.push_back(1);\n"
              "    a.g();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:6:21] -> [test.cpp:7:9] -> [test.cpp:8:7] -> [test.cpp:5:25] -> [test.cpp:9:5]: (error) Using object that points to local variable 'v' that may be invalid. [invalidContainer]\n",
            errout_str());

        // #11028
        check("void f(std::vector<int> c) {\n"
              "    std::vector<int> d(c.begin(), c.end());\n"
              "    c.erase(c.begin());\n"
              "    d.push_back(0);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #11147
        check("void f(std::string& s) {\n"
              "    if (!s.empty()) {\n"
              "        std::string::iterator it = s.begin();\n"
              "        s = s.substr(it - s.begin());\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:4:13]: (performance) Ineffective call of function 'substr' because a prefix of the string is assigned to itself. Use resize() or pop_back() instead. [uselessCallsSubstr]\n",
            errout_str());

        // #11630
        check("int main(int argc, const char* argv[]) {\n"
              "    std::vector<std::string> args(argv + 1, argv + argc);\n"
              "    args.push_back(\"-h\");\n"
              "    args.front();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int>& v) {\n" // #13108
              "    auto it = unknown(v);\n"
              "    auto w = std::vector<int>{ it, v.end() };\n"
              "    v.erase(it, v.end());\n"
              "    for (const auto& i : w) {}\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #13410
        check("int f(std::vector<int>& v) {\n"
              "    const int* i = &*v.cbegin();\n"
              "    v.push_back(1);\n"
              "    return *i;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:1:26] -> [test.cpp:2:30] -> [test.cpp:1:26] -> [test.cpp:2:20] -> [test.cpp:2:20] -> [test.cpp:3:7] -> [test.cpp:1:25] -> [test.cpp:4:13]: (error) Using pointer to local variable 'v' that may be invalid. [invalidContainer]\n",
            errout_str());

        // #9834
        check("struct CJ {\n"
              "    std::string m_string1 = \"hello\";\n"
              "};\n"
              "void f() {\n"
              "    std::vector<CJ> vec1;\n"
              "    vec1.push_back(CJ());\n"
              "    auto& a_ref = vec1.at(0).m_string1;\n"
              "    vec1.clear();\n"
              "    std::cout << a_ref << std::endl;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:7:17] -> [test.cpp:7:24] -> [test.cpp:8:10] -> [test.cpp:9:18]: (error) Reference to vec1 that may be invalid. [invalidContainerReference]\n",
            errout_str());
    }

    void invalidContainerLoop() {
        // #9435
        check("void f(std::vector<int> v) {\n"
              "    for (auto i : v) {\n"
              "        if (i < 5)\n"
              "            v.push_back(i * 2);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:2:5] -> [test.cpp:4:15]: (error) Calling 'push_back' while iterating the container is invalid. [invalidContainerLoop]\n", errout_str());

        // #9713
        check("void f() {\n"
              "  std::vector<int> v{1, 2, 3};\n"
              "  for (int i : v) {\n"
              "    if (i == 2) {\n"
              "      v.clear();\n"
              "      return;\n"
              "    }\n"
              "  }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:17]: (style) Consider using std::any_of algorithm instead of a raw loop. [useStlAlgorithm]\n", errout_str());

        check("struct A {\n"
              "  std::vector<int> v;\n"
              "  void add(int i) {\n"
              "    v.push_back(i);\n"
              "  } \n"
              "  void f() {\n"
              "    for(auto i:v)\n"
              "      add(i);\n"
              "  }\n"
              "};\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS(
            "[test.cpp:4:7] -> [test.cpp:7:5] -> [test.cpp:8:7]: (error) Calling 'add' while iterating the container is invalid. [invalidContainerLoop]\n",
            errout_str());

        check("struct S { int i; };\n" // #14013
              "void f() {\n"
              "    std::vector<std::unique_ptr<S>> v;\n"
              "    for (int i = 0; i < 5; ++i) {\n"
              "        std::unique_ptr<S>& r = v.emplace_back(std::make_unique<S>());\n"
              "        r->i = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct S { int i; };\n"
              "void f() {\n"
              "    std::vector<std::unique_ptr<S>> v;\n"
              "    for (int i = 0; i < 5; ++i) {\n"
              "        std::unique_ptr<S>& r{ v.emplace_back(std::make_unique<S>()) };\n"
              "        r->i = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void findInsert() {
        check("void f1(std::set<unsigned>& s, unsigned x) {\n"
              "    if (s.find(x) == s.end()) {\n"
              "        s.insert(x);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:18]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f2(std::map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.find(x) == m.end()) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:22]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f3(std::map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:22]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f4(std::set<unsigned>& s, unsigned x) {\n"
              "    if (s.find(x) == s.end()) {\n"
              "        s.insert(x);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:18]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f5(std::map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:22]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f6(std::map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:22]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f1(std::unordered_set<unsigned>& s, unsigned x) {\n"
              "    if (s.find(x) == s.end()) {\n"
              "        s.insert(x);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:18]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f2(std::unordered_map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.find(x) == m.end()) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:22]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f3(std::unordered_map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:22]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f4(std::unordered_set<unsigned>& s, unsigned x) {\n"
              "    if (s.find(x) == s.end()) {\n"
              "        s.insert(x);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:18]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f5(std::unordered_map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:22]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void f6(std::unordered_map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:22]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());

        check("void g1(std::map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.find(x) == m.end()) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 2;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void g1(std::map<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 2;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f1(QSet<unsigned>& s, unsigned x) {\n"
              "    if (s.find(x) == s.end()) {\n"
              "        s.insert(x);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f1(std::multiset<unsigned>& s, unsigned x) {\n"
              "    if (s.find(x) == s.end()) {\n"
              "        s.insert(x);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f2(std::multimap<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.find(x) == m.end()) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f3(std::multimap<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f4(std::multiset<unsigned>& s, unsigned x) {\n"
              "    if (s.find(x) == s.end()) {\n"
              "        s.insert(x);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f5(std::multimap<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f1(std::unordered_multiset<unsigned>& s, unsigned x) {\n"
              "    if (s.find(x) == s.end()) {\n"
              "        s.insert(x);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f2(std::unordered_multimap<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.find(x) == m.end()) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f3(std::unordered_multimap<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f4(std::unordered_multiset<unsigned>& s, unsigned x) {\n"
              "    if (s.find(x) == s.end()) {\n"
              "        s.insert(x);\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f5(std::unordered_multimap<unsigned, unsigned>& m, unsigned x) {\n"
              "    if (m.count(x) == 0) {\n"
              "        m.emplace(x, 1);\n"
              "    } else {\n"
              "        m[x] = 1;\n"
              "    }\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        // #9218 - not small type => do not warn if cpp standard is < c++17
        {
            Settings s = settings;
            const char code[] = "void f1(std::set<LargeType>& s, const LargeType& x) {\n"
                                "    if (s.find(x) == s.end()) {\n"
                                "        s.insert(x);\n"
                                "    }\n"
                                "}\n";
            s.standards.cpp = Standards::CPP11;
            check(code, s);
            ASSERT_EQUALS("", errout_str());
            s.standards.cpp = Standards::CPP14;
            check(code, s);
            ASSERT_EQUALS("", errout_str());
            s.standards.cpp = Standards::CPP17;
            check(code, s);
            ASSERT_EQUALS("[test.cpp:3:18]: (performance) Searching before insertion is not necessary. [stlFindInsert]\n", errout_str());
        }

        { // #10558
            Settings s = settings;
            s.standards.cpp = Standards::CPP03;
            check("void foo() {\n"
                  "   std::map<int, int> x;\n"
                  "   int data = 0;\n"
                  "   for(int i=0; i<10; ++i) {\n"
                  "      data += 123;\n"
                  "      if(x.find(5) == x.end())\n"
                  "         x[5] = data;\n"
                  "   }\n"
                  "}", s);
            ASSERT_EQUALS("", errout_str());

            s.standards.cpp = Standards::CPP11;
            check("void foo() {\n"
                  "   std::map<int, int> x;\n"
                  "   int data = 0;\n"
                  "   for(int i=0; i<10; ++i) {\n"
                  "      data += 123;\n"
                  "      if(x.find(5) == x.end())\n"
                  "         x[5] = data;\n"
                  "   }\n"
                  "}", s);
            ASSERT_EQUALS("[test.cpp:7:17]: (performance) Searching before insertion is not necessary. Instead of 'x[5]=data' consider using 'x.emplace(5, data);'. [stlFindInsert]\n", errout_str());

            check("void foo() {\n"
                  "   std::map<int, int> x;\n"
                  "   int data = 0;\n"
                  "   for(int i=0; i<10; ++i) {\n"
                  "      data += 123;\n"
                  "      if(x.find(5) == x.end())\n"
                  "         x[5] = data;\n"
                  "   }\n"
                  "}"); // TODO: use s?
            ASSERT_EQUALS("[test.cpp:7:17]: (performance) Searching before insertion is not necessary. Instead of 'x[5]=data' consider using 'x.try_emplace(5, data);'. [stlFindInsert]\n", errout_str());
        }
    }

    void checkKnownEmptyContainer() {
        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    for(auto x:v) {}\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Iterating over container 'v' that is always empty. [knownEmptyContainer]\n", errout_str());

        check("void f(std::vector<int> v) {\n"
              "    v.clear();\n"
              "    for(auto x:v) {}\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Iterating over container 'v' that is always empty. [knownEmptyContainer]\n", errout_str());

        check("void f(std::vector<int> v) {\n"
              "    if (!v.empty()) { return; }\n"
              "    for(auto x:v) {}\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:16]: (style) Iterating over container 'v' that is always empty. [knownEmptyContainer]\n", errout_str());

        check("void f(std::vector<int> v) {\n"
              "    if (v.empty()) { return; }\n"
              "    for(auto x:v) {}\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    std::sort(v.begin(), v.end());\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:22]: (style) Using sort with iterator 'v.begin()' that is always empty. [knownEmptyContainer]\n", errout_str());

        check("void f() {\n" // #1201
              "    std::vector<int> v1{ 0, 1 };\n"
              "    std::vector<int> v2;\n"
              "    std::copy(v1.begin(), v1.end(), v2.begin());\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:45]: (style) Using copy with iterator 'v2.begin()' that is always empty. [knownEmptyContainer]\n", errout_str());

        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    v.insert(v.end(), 1);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct A {\n"
              "    explicit A(std::vector<int>*);\n"
              "};\n"
              "A f() {\n"
              "    std::vector<int> v;\n"
              "    A a(&v);\n"
              "    for(auto&& x:v) {}\n"
              "    return a;\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("static void f1(std::list<std::string>& parameters) {\n"
              "    parameters.push_back(a);\n"
              "}\n"
              "int f2(std::list<std::string>& parameters) {\n"
              "    f1(parameters);\n"
              "}\n"
              "void f3() {\n"
              "    std::list<std::string> parameters;\n"
              "    int res = ::f2(parameters);\n"
              "    for (auto param : parameters) {}\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("namespace ns {\n"
              "    using ArrayType = std::vector<int>;\n"
              "}\n"
              "using namespace ns;\n"
              "static void f() {\n"
              "    const ArrayType arr;\n"
              "    for (const auto &a : arr) {}\n"
              "}",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:7:26]: (style) Iterating over container 'arr' that is always empty. [knownEmptyContainer]\n", errout_str());

        check("struct S {\n"
              "    std::vector<int> v;\n"
              "};\n"
              "void foo(S& s) {\n"
              "    s.v.clear();\n"
              "    bar(s);\n"
              "    std::sort(s.v.begin(), s.v.end());\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(const std::vector<int>& v, int e) {\n"
              " if (!v.empty()) {\n"
              "     if (e < 0 || true) {\n"
              "         if (e < 0)\n"
              "             return;\n"
              "     }\n"
              " }\n"
              " for (auto i : v) {}\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::vector<int> v;\n"
              "    auto& rv = v;\n"
              "    rv.push_back(42);\n"
              "    for (auto i : v) {}\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("extern void f(std::string&&);\n"
              "static void func() {\n"
              "    std::string s;\n"
              "    const std::string& s_ref = s;\n"
              "    f(std::move(s));\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("struct S {\n" // #12757
              "    template<class T = int>\n"
              "    void clear() {}\n"
              "    template<class T = int>\n"
              "    std::vector<T> get() const { return {}; }\n"
              "    std::vector<int> m;\n"
              "};\n"
              "template<> void S::clear() { m.clear(); }\n"
              "template<> std::vector<int> S::get() const {\n"
              "    for (const auto& i : m) {}\n"
              "    return {};\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(bool b) {\n" // #13121
              "    static std::string s = {};\n"
              "    for (auto c : s) {}\n"
              "    if (b)\n"
              "        s += \'a\';\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f(std::vector<int>::iterator it) {\n" // #13727
              "    std::vector<int> v;\n"
              "    v.insert<std::vector<int>::iterator>(v.end(), it, it + 1);\n"
              "    for (auto i : v) {}\n"
              "}\n", dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());
    }

    void checkMutexes() {
        check("void f() {\n"
              "    static std::mutex m;\n"
              "    static std::lock_guard<std::mutex> g(m);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:40]: (warning) Lock guard is defined globally. Lock guards are intended to be local. A global lock guard could lead to a deadlock since it won't unlock until the end of the program. [globalLockGuard]\n", errout_str());

        check("void f() {\n"
              "    static std::mutex m;\n"
              "    std::lock_guard<std::mutex> g(m);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    static std::mutex m;\n"
              "    static std::unique_lock<std::mutex> g(m, std::defer_lock);\n"
              "    static std::lock(g);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:41]: (warning) Lock guard is defined globally. Lock guards are intended to be local. A global lock guard could lead to a deadlock since it won't unlock until the end of the program. [globalLockGuard]\n", errout_str());

        check("void f() {\n"
              "    static std::mutex m;\n"
              "    std::unique_lock<std::mutex> g(m, std::defer_lock);\n"
              "    std::lock(g);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void f() {\n"
              "    std::mutex m;\n"
              "    std::lock_guard<std::mutex> g(m);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:33]: (warning) The lock is ineffective because the mutex is locked at the same scope as the mutex itself. [localMutex]\n", errout_str());

        check("void f() {\n"
              "    std::mutex m;\n"
              "    std::unique_lock<std::mutex> g(m);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:34]: (warning) The lock is ineffective because the mutex is locked at the same scope as the mutex itself. [localMutex]\n", errout_str());

        check("void f() {\n"
              "    std::mutex m;\n"
              "    std::unique_lock<std::mutex> g(m, std::defer_lock);\n"
              "    std::lock(g);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:3:34]: (warning) The lock is ineffective because the mutex is locked at the same scope as the mutex itself. [localMutex]\n", errout_str());

        check("void g();\n"
              "void f() {\n"
              "    static std::mutex m;\n"
              "    m.lock();\n"
              "    g();\n"
              "    m.unlock();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void g();\n"
              "void f() {\n"
              "    std::mutex m;\n"
              "    m.lock();\n"
              "    g();\n"
              "    m.unlock();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:5]: (warning) The lock is ineffective because the mutex is locked at the same scope as the mutex itself. [localMutex]\n", errout_str());

        check("class A {\n"
              "    std::mutex m;\n"
              "    void f() {\n"
              "        std::lock_guard<std::mutex> g(m);\n"
              "    }\n"
              "};\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("class A {\n"
              "    std::mutex m;\n"
              "    void g();\n"
              "    void f() {\n"
              "        m.lock();\n"
              "        g();\n"
              "        m.unlock();\n"
              "    }\n"
              "};\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("class A {\n"
              "    std::mutex m;\n"
              "    void f() {\n"
              "        static std::lock_guard<std::mutex> g(m);\n"
              "    }\n"
              "};\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:44]: (warning) Lock guard is defined globally. Lock guards are intended to be local. A global lock guard could lead to a deadlock since it won't unlock until the end of the program. [globalLockGuard]\n", errout_str());

        check("std::mutex& h();\n"
              "void f() {\n"
              "    auto& m = h();\n"
              "    std::lock_guard<std::mutex> g(m);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void g();\n"
              "std::mutex& h();\n"
              "void f() {\n"
              "    auto& m = h();\n"
              "    m.lock();\n"
              "    g();\n"
              "    m.unlock();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("std::mutex& h();\n"
              "void f() {\n"
              "    auto m = h();\n"
              "    std::lock_guard<std::mutex> g(m);\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:4:33]: (warning) The lock is ineffective because the mutex is locked at the same scope as the mutex itself. [localMutex]\n", errout_str());

        check("void g();\n"
              "std::mutex& h();\n"
              "void f() {\n"
              "    auto m = h();\n"
              "    m.lock();\n"
              "    g();\n"
              "    m.unlock();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("[test.cpp:5:5]: (warning) The lock is ineffective because the mutex is locked at the same scope as the mutex itself. [localMutex]\n", errout_str());

        check("void foo();\n"
              "void bar();\n"
              "void f() {\n"
              "    std::mutex m;\n"
              "    std::thread t([&m](){\n"
              "        m.lock();\n"
              "        foo();\n"
              "        m.unlock();\n"
              "    });\n"
              "    m.lock();\n"
              "    bar();\n"
              "    m.unlock();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo();\n"
              "void bar();\n"
              "void f() {\n"
              "    std::mutex m;\n"
              "    std::thread t([&m](){\n"
              "        std::unique_lock<std::mutex> g{m};\n"
              "        foo();\n"
              "    });\n"
              "    std::unique_lock<std::mutex> g{m};\n"
              "    bar();\n"
              "}\n",
              dinit(CheckOptions, $.inconclusive = true));
        ASSERT_EQUALS("", errout_str());

        check("void foo() { int f = 0; auto g(f); g = g; }");
        ASSERT_EQUALS("", errout_str());

        check("struct foobar {\n"
              "    int foo;\n"
              "    std::shared_mutex foo_mtx;\n"
              "    int bar;\n"
              "    std::shared_mutex bar_mtx;\n"
              "};\n"
              "void f() {\n"
              "    foobar xyz;\n"
              "    {\n"
              "        std::shared_lock shared_foo_lock(xyz.foo_mtx, std::defer_lock);\n"
              "        std::shared_lock shared_bar_lock(xyz.bar_mtx, std::defer_lock);\n"
              "        std::scoped_lock shared_multi_lock(shared_foo_lock, shared_bar_lock);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout_str());
    }
};

REGISTER_TEST(TestStl)
