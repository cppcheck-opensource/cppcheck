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

#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "library.h"
#include "settings.h"
#include "standards.h"
#include "token.h"
#include "tokenlist.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#define ASSERT_EQ(expected, actual)   ASSERT(expected == actual)

class TestLibrary : public TestFixture {
public:
    TestLibrary() : TestFixture("TestLibrary") {}

private:
    void run() override {
        TEST_CASE(isCompliantValidationExpression);
        TEST_CASE(empty);
        TEST_CASE(function);
        TEST_CASE(function_match_scope);
        TEST_CASE(function_match_args);
        TEST_CASE(function_match_args_default);
        TEST_CASE(function_match_var);
        TEST_CASE(function_arg);
        TEST_CASE(function_arg_any);
        TEST_CASE(function_arg_variadic);
        TEST_CASE(function_arg_direction);
        TEST_CASE(function_arg_valid);
        TEST_CASE(function_arg_minsize);
        TEST_CASE(function_namespace);
        TEST_CASE(function_method);
        TEST_CASE(function_baseClassMethod); // calling method in base class
        TEST_CASE(function_warn);
        TEST_CASE(memory);
        TEST_CASE(memory2); // define extra "free" allocation functions
        TEST_CASE(memory3);
        TEST_CASE(resource);
        TEST_CASE(podtype);
        TEST_CASE(container);
        TEST_CASE(containerActionToFromString);
        TEST_CASE(containerYieldToFromString);
        TEST_CASE(version);
        TEST_CASE(loadLibErrors);
        TEST_CASE(loadLibCombinations);
        TEST_CASE(smartpointer);
    }

    void isCompliantValidationExpression() const {
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("-1"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("1"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("1:"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression(":1"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("-1,42"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("-1,-42"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("-1.0:42.0"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("1.175494e-38:3.402823e+38"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("1.175494e-38,3.402823e+38"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("1.175494e-38:"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression(":1.175494e-38"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression(":42.0"));
        ASSERT_EQUALS(true, Library::isCompliantValidationExpression("!42.0"));

        // Robustness tests
        ASSERT_EQUALS(false, Library::isCompliantValidationExpression(nullptr));
        ASSERT_EQUALS(false, Library::isCompliantValidationExpression("x"));
        ASSERT_EQUALS(false, Library::isCompliantValidationExpression("!"));
        ASSERT_EQUALS(false, Library::isCompliantValidationExpression(""));
    }

    void empty() const {
        // Reading an empty library file is considered to be OK
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n<def/>";
        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT(library.functions().empty());
    }

    void function() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"foo\">\n"
                                   "    <noreturn>false</noreturn>\n"
                                   "  </function>\n"
                                   "</def>";

        const char code[] = "foo();";
        SimpleTokenList tokenList(code);
        tokenList.front()->next()->astOperand1(tokenList.front());

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT_EQUALS(library.functions().size(), 1U);
        ASSERT(library.functions().at("foo").argumentChecks.empty());
        ASSERT(library.isnotnoreturn(tokenList.front()));
    }

    void function_match_scope() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"foo\">\n"
                                   "    <arg nr=\"1\"/>"
                                   "  </function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        {
            const char code[] = "fred.foo(123);"; // <- wrong scope, not library function
            const SimpleTokenList tokenList(code);

            ASSERT(library.isNotLibraryFunction(tokenList.front()->tokAt(2)));
        }
        {
            const char code[] = "Fred::foo(123);"; // <- wrong scope, not library function
            const SimpleTokenList tokenList(code);

            ASSERT(library.isNotLibraryFunction(tokenList.front()->tokAt(2)));
        }
    }

    void function_match_args() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"foo\">\n"
                                   "    <arg nr=\"1\"/>"
                                   "  </function>\n"
                                   "</def>";

        TokenList tokenList(settingsDefault, Standards::Language::CPP);
        const char code[] = "foo();"; // <- too few arguments, not library function
        ASSERT(tokenList.createTokensFromString(code));
        Token::createMutualLinks(tokenList.front()->next(), tokenList.back()->previous());
        tokenList.createAst();

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT(library.isNotLibraryFunction(tokenList.front()));
    }

    void function_match_args_default() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"foo\">\n"
                                   "    <arg nr=\"1\"/>"
                                   "    <arg nr=\"2\" default=\"0\"/>"
                                   "  </function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));

        {
            TokenList tokenList(settingsDefault, Standards::Language::CPP);
            const char code[] = "foo();"; // <- too few arguments, not library function
            ASSERT(tokenList.createTokensFromString(code));
            Token::createMutualLinks(tokenList.front()->next(), tokenList.back()->previous());
            tokenList.createAst();

            ASSERT(library.isNotLibraryFunction(tokenList.front()));
        }
        {
            TokenList tokenList(settingsDefault, Standards::Language::CPP);
            const char code[] = "foo(a);"; // <- library function
            ASSERT(tokenList.createTokensFromString(code));
            Token::createMutualLinks(tokenList.front()->next(), tokenList.back()->previous());
            tokenList.createAst();

            const Library::Function* func = nullptr;
            ASSERT(!library.isNotLibraryFunction(tokenList.front(), &func));
            ASSERT(func);
        }
        {
            TokenList tokenList(settingsDefault, Standards::Language::CPP);
            const char code[] = "foo(a, b);"; // <- library function
            ASSERT(tokenList.createTokensFromString(code));
            Token::createMutualLinks(tokenList.front()->next(), tokenList.back()->previous());
            tokenList.createAst();

            const Library::Function* func = nullptr;
            ASSERT(!library.isNotLibraryFunction(tokenList.front(), &func));
            ASSERT(func);
        }
        {
            TokenList tokenList(settingsDefault, Standards::Language::CPP);
            const char code[] = "foo(a, b, c);"; // <- too much arguments, not library function
            ASSERT(tokenList.createTokensFromString(code));
            Token::createMutualLinks(tokenList.front()->next(), tokenList.back()->previous());
            tokenList.createAst();

            ASSERT(library.isNotLibraryFunction(tokenList.front()));
        }
    }

    void function_match_var() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"foo\">\n"
                                   "    <arg nr=\"1\"/>"
                                   "  </function>\n"
                                   "</def>";

        const char code[] = "Fred foo(123);"; // <- Variable declaration, not library function
        SimpleTokenList tokenList(code);
        tokenList.front()->next()->astOperand1(tokenList.front());
        tokenList.front()->next()->varId(1);

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT(library.isNotLibraryFunction(tokenList.front()->next()));
    }

    void function_arg() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"foo\">\n"
                                   "    <arg nr=\"1\"><not-uninit/></arg>\n"
                                   "    <arg nr=\"2\"><not-null/></arg>\n"
                                   "    <arg nr=\"3\"><formatstr/></arg>\n"
                                   "    <arg nr=\"4\"><strz/></arg>\n"
                                   "    <arg nr=\"5\" default=\"0\"><not-bool/></arg>\n"
                                   "  </function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        const auto& foo_fn_args = library.functions().at("foo").argumentChecks;
        ASSERT_EQUALS(0, foo_fn_args.at(1).notuninit);
        ASSERT_EQUALS(true, foo_fn_args.at(2).notnull);
        ASSERT_EQUALS(true, foo_fn_args.at(3).formatstr);
        ASSERT_EQUALS(true, foo_fn_args.at(4).strz);
        ASSERT_EQUALS(false, foo_fn_args.at(4).optional);
        ASSERT_EQUALS(true, foo_fn_args.at(5).notbool);
        ASSERT_EQUALS(true, foo_fn_args.at(5).optional);
    }

    void function_arg_any() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "<function name=\"foo\">\n"
                                   "   <arg nr=\"any\"><not-uninit/></arg>\n"
                                   "</function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT_EQUALS(0, library.functions().at("foo").argumentChecks.at(-1).notuninit);
    }

    void function_arg_variadic() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "<function name=\"foo\">\n"
                                   "   <arg nr=\"1\"></arg>\n"
                                   "   <arg nr=\"variadic\"><not-uninit/></arg>\n"
                                   "</function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT_EQUALS(0, library.functions().at("foo").argumentChecks.at(-1).notuninit);

        const char code[] = "foo(a,b,c,d,e);";
        SimpleTokenList tokenList(code);
        tokenList.front()->next()->astOperand1(tokenList.front());

        ASSERT_EQUALS(false, library.isuninitargbad(tokenList.front(), 1));
        ASSERT_EQUALS(true, library.isuninitargbad(tokenList.front(), 2));
        ASSERT_EQUALS(true, library.isuninitargbad(tokenList.front(), 3));
        ASSERT_EQUALS(true, library.isuninitargbad(tokenList.front(), 4));
    }

    void function_arg_direction() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "<function name=\"foo\">\n"
                                   "   <arg nr=\"1\" direction=\"in\"></arg>\n"
                                   "   <arg nr=\"2\" direction=\"out\"></arg>\n"
                                   "   <arg nr=\"3\" direction=\"inout\"></arg>\n"
                                   "   <arg nr=\"4\"></arg>\n"
                                   "</function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));

        const char code[] = "foo(a,b,c,d);";
        SimpleTokenList tokenList(code);
        tokenList.front()->next()->astOperand1(tokenList.front());

        ASSERT(Library::ArgumentChecks::Direction::DIR_IN == library.getArgDirection(tokenList.front(), 1));
        ASSERT(Library::ArgumentChecks::Direction::DIR_OUT == library.getArgDirection(tokenList.front(), 2));
        ASSERT(Library::ArgumentChecks::Direction::DIR_INOUT == library.getArgDirection(tokenList.front(), 3));
        ASSERT(Library::ArgumentChecks::Direction::DIR_UNKNOWN == library.getArgDirection(tokenList.front(), 4));
    }

    void function_arg_valid() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"foo\">\n"
                                   "    <arg nr=\"1\"><valid>1:</valid></arg>\n"
                                   "    <arg nr=\"2\"><valid>-7:0</valid></arg>\n"
                                   "    <arg nr=\"3\"><valid>1:5,8</valid></arg>\n"
                                   "    <arg nr=\"4\"><valid>-1,5</valid></arg>\n"
                                   "    <arg nr=\"5\"><valid>:1,5</valid></arg>\n"
                                   "    <arg nr=\"6\"><valid>1.5:</valid></arg>\n"
                                   "    <arg nr=\"7\"><valid>-6.7:-5.5,-3.3:-2.7</valid></arg>\n"
                                   "    <arg nr=\"8\"><valid>0.0:</valid></arg>\n"
                                   "    <arg nr=\"9\"><valid>:2.0</valid></arg>\n"
                                   "    <arg nr=\"10\"><valid>0.0</valid></arg>\n"
                                   "    <arg nr=\"11\"><valid>!0.0</valid></arg>\n"
                                   "  </function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));

        const char code[] = "foo(a,b,c,d,e,f,g,h,i,j,k);";
        SimpleTokenList tokenList(code);
        tokenList.front()->next()->astOperand1(tokenList.front());

        // 1-
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 1, -10, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 1, -10.0, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 1, 0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 1, 0.0, settingsDefault));
        ASSERT_EQUALS(true, library.isIntArgValid(tokenList.front(), 1, 1, settingsDefault));
        ASSERT_EQUALS(true, library.isFloatArgValid(tokenList.front(), 1, 1.0, settingsDefault));
        ASSERT_EQUALS(true, library.isIntArgValid(tokenList.front(), 1, 10, settingsDefault));
        ASSERT_EQUALS(true, library.isFloatArgValid(tokenList.front(), 1, 10.0, settingsDefault));

        // -7-0
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 2, -10, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 2, -10.0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 2, -7.5, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 2, -7.1, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 2, -7, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 2, -7.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 2, -3, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 2, -3.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 2, -3.5, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 2, 0, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 2, 0.0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 2, 0.5, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 2, 1, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 2, 1.0, settingsDefault));

        // 1-5,8
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 3, 0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 3, 0.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 3, 1, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 3, 1.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 3, 3, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 3, 3.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 3, 5, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 3, 5.0, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 3, 6, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 3, 6.0, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 3, 7, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 3, 7.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 3, 8, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 3, 8.0, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 3, 9, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 3, 9.0, settingsDefault));

        // -1,5
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 4, -10, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 4, -10.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 4, -1, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 4, -1.0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 4, 5.000001, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 4, 5.5, settingsDefault));

        // :1,5
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 5, -10, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 5, -10.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 5, 1, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 5, 1.0, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 5, 2, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 5, 2.0, settingsDefault));

        // 1.5:
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 6, 0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 6, 0.0, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 6, 1, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 6, 1.499999, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 6, 1.5, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 6, 2, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 6, 10, settingsDefault));

        // -6.7:-5.5,-3.3:-2.7
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 7, -7, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 7, -7.0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 7, -6.7000001, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 7, -6.7, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 7, -6, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 7, -6.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 7, -5.5, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 7, -5.4999999, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 7, -3.3000001, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 7, -3.3, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 7, -3, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 7, -3.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 7, -2.7, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 7, -2.6999999, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 7, -2, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 7, -2.0, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 7, 0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 7, 0.0, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 7, 3, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 7, 3.0, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 7, 6, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 7, 6.0, settingsDefault));

        // 0.0:
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 8, -1, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 8, -1.0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 8, -0.00000001, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 8, 0, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 8, 0.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 8, 0.000000001, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 8, 1, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 8, 1.0, settingsDefault));

        // :2.0
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 9, -1, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 9, -1.0, settingsDefault));
        ASSERT_EQUALS(true,  library.isIntArgValid(tokenList.front(), 9, 2, settingsDefault));
        ASSERT_EQUALS(true,  library.isFloatArgValid(tokenList.front(), 9, 2.0, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 9, 2.00000001, settingsDefault));
        ASSERT_EQUALS(false, library.isIntArgValid(tokenList.front(), 9, 200, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 9, 200.0, settingsDefault));

        // 0.0
        ASSERT_EQUALS(true, library.isIntArgValid(tokenList.front(), 10, 0, settingsDefault));
        ASSERT_EQUALS(true, library.isFloatArgValid(tokenList.front(), 10, 0.0, settingsDefault));

        // ! 0.0
        ASSERT_EQUALS(true, library.isFloatArgValid(tokenList.front(), 11, -0.42, settingsDefault));
        ASSERT_EQUALS(false, library.isFloatArgValid(tokenList.front(), 11, 0.0, settingsDefault));
        ASSERT_EQUALS(true, library.isFloatArgValid(tokenList.front(), 11, 0.42, settingsDefault));
    }

    void function_arg_minsize() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"foo\">\n"
                                   "    <arg nr=\"1\"><minsize type=\"strlen\" arg=\"2\"/></arg>\n"
                                   "    <arg nr=\"2\"><minsize type=\"argvalue\" arg=\"3\"/></arg>\n"
                                   "    <arg nr=\"3\"/>\n"
                                   "    <arg nr=\"4\"><minsize type=\"value\" value=\"500\"/></arg>\n"
                                   "    <arg nr=\"5\"><minsize type=\"value\" value=\"4\" baseType=\"int\"/></arg>\n"
                                   "  </function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));

        const char code[] = "foo(a,b,c,d,e);";
        SimpleTokenList tokenList(code);
        tokenList.front()->next()->astOperand1(tokenList.front());

        // arg1: type=strlen arg2
        const std::vector<Library::ArgumentChecks::MinSize> *minsizes = library.argminsizes(tokenList.front(),1);
        ASSERT_EQUALS(true, minsizes != nullptr);
        ASSERT_EQUALS(1U, minsizes ? minsizes->size() : 1U);
        if (minsizes && minsizes->size() == 1U) {
            const Library::ArgumentChecks::MinSize &m = minsizes->front();
            ASSERT_EQUALS(true, Library::ArgumentChecks::MinSize::Type::STRLEN == m.type);
            ASSERT_EQUALS(2, m.arg);
        }

        // arg2: type=argvalue arg3
        minsizes = library.argminsizes(tokenList.front(), 2);
        ASSERT_EQUALS(true, minsizes != nullptr);
        ASSERT_EQUALS(1U, minsizes ? minsizes->size() : 1U);
        if (minsizes && minsizes->size() == 1U) {
            const Library::ArgumentChecks::MinSize &m = minsizes->front();
            ASSERT_EQUALS(true, Library::ArgumentChecks::MinSize::Type::ARGVALUE == m.type);
            ASSERT_EQUALS(3, m.arg);
        }

        // arg4: type=value
        minsizes = library.argminsizes(tokenList.front(), 4);
        ASSERT_EQUALS(true, minsizes != nullptr);
        ASSERT_EQUALS(1U, minsizes ? minsizes->size() : 1U);
        if (minsizes && minsizes->size() == 1U) {
            const Library::ArgumentChecks::MinSize &m = minsizes->front();
            ASSERT(Library::ArgumentChecks::MinSize::Type::VALUE == m.type);
            ASSERT_EQUALS(500, m.value);
            ASSERT_EQUALS("", m.baseType);
        }

        // arg5: type=value
        minsizes = library.argminsizes(tokenList.front(), 5);
        ASSERT_EQUALS(true, minsizes != nullptr);
        ASSERT_EQUALS(1U, minsizes ? minsizes->size() : 1U);
        if (minsizes && minsizes->size() == 1U) {
            const Library::ArgumentChecks::MinSize& m = minsizes->front();
            ASSERT(Library::ArgumentChecks::MinSize::Type::VALUE == m.type);
            ASSERT_EQUALS(4, m.value);
            ASSERT_EQUALS("int", m.baseType);
        }
    }

    void function_namespace() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"Foo::foo,bar\">\n"
                                   "    <noreturn>false</noreturn>\n"
                                   "  </function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT_EQUALS(library.functions().size(), 2U);
        ASSERT(library.functions().at("Foo::foo").argumentChecks.empty());
        ASSERT(library.functions().at("bar").argumentChecks.empty());

        {
            const char code[] = "Foo::foo();";
            const SimpleTokenList tokenList(code);
            ASSERT(library.isnotnoreturn(tokenList.front()->tokAt(2)));
        }

        {
            const char code[] = "bar();";
            const SimpleTokenList tokenList(code);
            ASSERT(library.isnotnoreturn(tokenList.front()));
        }
    }

    void function_method() {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"CString::Format\">\n"
                                   "    <noreturn>false</noreturn>\n"
                                   "  </function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT_EQUALS(library.functions().size(), 1U);

        {
            SimpleTokenizer tokenizer(settingsDefault, *this);
            const char code[] = "CString str; str.Format();";
            ASSERT(tokenizer.tokenize(code));
            ASSERT(library.isnotnoreturn(Token::findsimplematch(tokenizer.tokens(), "Format")));
        }

        {
            SimpleTokenizer tokenizer(settingsDefault, *this);
            const char code[] = "HardDrive hd; hd.Format();";
            ASSERT(tokenizer.tokenize(code));
            ASSERT(!library.isnotnoreturn(Token::findsimplematch(tokenizer.tokens(), "Format")));
        }
    }

    void function_baseClassMethod() {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"Base::f\">\n"
                                   "    <arg nr=\"1\"><not-null/></arg>\n"
                                   "  </function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));

        {
            SimpleTokenizer tokenizer(settingsDefault, *this);
            const char code[] = "struct X : public Base { void dostuff() { f(0); } };";
            ASSERT(tokenizer.tokenize(code));
            ASSERT(library.isnullargbad(Token::findsimplematch(tokenizer.tokens(), "f"),1));
        }

        {
            SimpleTokenizer tokenizer(settingsDefault, *this);
            const char code[] = "struct X : public Base { void dostuff() { f(1,2); } };";
            ASSERT(tokenizer.tokenize(code));
            ASSERT(!library.isnullargbad(Token::findsimplematch(tokenizer.tokens(), "f"),1));
        }
    }

    void function_warn() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <function name=\"a\">\n"
                                   "    <warn severity=\"style\" cstd=\"c99\">Message</warn>\n"
                                   "  </function>\n"
                                   "  <function name=\"b\">\n"
                                   "    <warn severity=\"performance\" cppstd=\"c++11\" reason=\"Obsolescent\" alternatives=\"c,d,e\"/>\n"
                                   "  </function>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));

        const char code[] = "a(); b();";
        const SimpleTokenList tokenList(code);

        const Library::WarnInfo* a = library.getWarnInfo(tokenList.front());
        const Library::WarnInfo* b = library.getWarnInfo(tokenList.front()->tokAt(4));

        ASSERT_EQUALS(2, library.functionwarn().size());
        ASSERT(a && b);
        if (a && b) {
            ASSERT_EQUALS("Message", a->message);
            ASSERT_EQUALS_ENUM(Severity::style, a->severity);
            ASSERT_EQUALS(Standards::C99, a->standards.c);
            ASSERT_EQUALS(Standards::CPP03, a->standards.cpp);

            ASSERT_EQUALS("Obsolescent function 'b' called. It is recommended to use 'c', 'd' or 'e' instead.", b->message);
            ASSERT_EQUALS_ENUM(Severity::performance, b->severity);
            ASSERT_EQUALS(Standards::C89, b->standards.c);
            ASSERT_EQUALS(Standards::CPP11, b->standards.cpp);
        }
    }

    void memory() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <memory>\n"
                                   "    <alloc>CreateX</alloc>\n"
                                   "    <dealloc>DeleteX</dealloc>\n"
                                   "  </memory>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT(library.functions().empty());

        const Library::AllocFunc* af = library.getAllocFuncInfo("CreateX");
        ASSERT(af && af->arg == -1);
        ASSERT(Library::ismemory(af));
        ASSERT_EQUALS(library.allocId("CreateX"), library.deallocId("DeleteX"));
        const Library::AllocFunc* df = library.getDeallocFuncInfo("DeleteX");
        ASSERT(df && df->arg == 1);
    }
    void memory2() const {
        constexpr char xmldata1[] = "<?xml version=\"1.0\"?>\n"
                                    "<def>\n"
                                    "  <memory>\n"
                                    "    <alloc>malloc</alloc>\n"
                                    "    <dealloc>free</dealloc>\n"
                                    "  </memory>\n"
                                    "</def>";
        constexpr char xmldata2[] = "<?xml version=\"1.0\"?>\n"
                                    "<def>\n"
                                    "  <memory>\n"
                                    "    <alloc>foo</alloc>\n"
                                    "    <dealloc>free</dealloc>\n"
                                    "  </memory>\n"
                                    "</def>";

        Library library;
        ASSERT_EQUALS(true, LibraryHelper::loadxmldata(library, xmldata1, sizeof(xmldata1)));
        ASSERT_EQUALS(true, LibraryHelper::loadxmldata(library, xmldata2, sizeof(xmldata2)));

        ASSERT_EQUALS(library.deallocId("free"), library.allocId("malloc"));
        ASSERT_EQUALS(library.deallocId("free"), library.allocId("foo"));
    }
    void memory3() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <memory>\n"
                                   "    <alloc arg=\"5\" init=\"false\">CreateX</alloc>\n"
                                   "    <dealloc arg=\"2\">DeleteX</dealloc>\n"
                                   "  </memory>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT(library.functions().empty());

        const Library::AllocFunc* af = library.getAllocFuncInfo("CreateX");
        ASSERT(af && af->arg == 5 && !af->initData);
        const Library::AllocFunc* df = library.getDeallocFuncInfo("DeleteX");
        ASSERT(df && df->arg == 2);
    }

    void resource() const {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <resource>\n"
                                   "    <alloc>CreateX</alloc>\n"
                                   "    <dealloc>DeleteX</dealloc>\n"
                                   "  </resource>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        ASSERT(library.functions().empty());

        ASSERT(Library::isresource(library.allocId("CreateX")));
        ASSERT_EQUALS(library.allocId("CreateX"), library.deallocId("DeleteX"));
    }

    void podtype() const {
        {
            constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                       "<def>\n"
                                       "  <podtype name=\"s8\" sign=\"s\" size=\"1\"/>\n"
                                       "  <podtype name=\"u8\" sign=\"u\" size=\"1\"/>\n"
                                       "  <podtype name=\"u16\" sign=\"u\" size=\"2\"/>\n"
                                       "  <podtype name=\"s16\" sign=\"s\" size=\"2\"/>\n"
                                       "</def>";
            Library library;
            ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
            // s8
            {
                const Library::PodType * const type = library.podtype("s8");
                ASSERT_EQUALS(true, type != nullptr);
                if (type) {
                    ASSERT_EQUALS(1U, type->size);
                    ASSERT_EQUALS('s', type->sign);
                }
            }
            // u8
            {
                const Library::PodType * const type = library.podtype("u8");
                ASSERT_EQUALS(true, type != nullptr);
                if (type) {
                    ASSERT_EQUALS(1U, type->size);
                    ASSERT_EQUALS('u', type->sign);
                }
            }
            // u16
            {
                const Library::PodType * const type = library.podtype("u16");
                ASSERT_EQUALS(true, type != nullptr);
                if (type) {
                    ASSERT_EQUALS(2U, type->size);
                    ASSERT_EQUALS('u', type->sign);
                }
            }
            // s16
            {
                const Library::PodType * const type = library.podtype("s16");
                ASSERT_EQUALS(true, type != nullptr);
                if (type) {
                    ASSERT_EQUALS(2U, type->size);
                    ASSERT_EQUALS('s', type->sign);
                }
            }
            // robustness test: provide cfg without PodType
            {
                const Library::PodType * const type = library.podtype("nonExistingPodType");
                ASSERT_EQUALS(true, type == nullptr);
            }
        }
    }

    void container() {
        constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                   "<def>\n"
                                   "  <container id=\"A\" startPattern=\"std :: A &lt;\" endPattern=\"&gt; !!::\" itEndPattern=\"&gt; :: iterator\">\n"
                                   "    <type templateParameter=\"1\"/>\n"
                                   "    <size templateParameter=\"4\">\n"
                                   "      <function name=\"resize\" action=\"resize\"/>\n"
                                   "      <function name=\"clear\" action=\"clear\"/>\n"
                                   "      <function name=\"size\" yields=\"size\"/>\n"
                                   "      <function name=\"empty\" yields=\"empty\"/>\n"
                                   "      <function name=\"push_back\" action=\"push\"/>\n"
                                   "      <function name=\"pop_back\" action=\"pop\"/>\n"
                                   "    </size>\n"
                                   "    <access>\n"
                                   "      <function name=\"at\" yields=\"at_index\"/>\n"
                                   "      <function name=\"begin\" yields=\"start-iterator\"/>\n"
                                   "      <function name=\"end\" yields=\"end-iterator\"/>\n"
                                   "      <function name=\"data\" yields=\"buffer\"/>\n"
                                   "      <function name=\"c_str\" yields=\"buffer-nt\"/>\n"
                                   "      <function name=\"front\" yields=\"item\"/>\n"
                                   "      <function name=\"find\" action=\"find\"/>\n"
                                   "      <function name=\"cfind\" action=\"find-const\"/>\n"
                                   "    </access>\n"
                                   "  </container>\n"
                                   "  <container id=\"B\" startPattern=\"std :: B &lt;\" inherits=\"A\" opLessAllowed=\"false\">\n"
                                   "    <size templateParameter=\"3\"/>\n" // Inherits all but templateParameter
                                   "  </container>\n"
                                   "  <container id=\"C\">\n"
                                   "    <type string=\"std-like\"/>\n"
                                   "    <access indexOperator=\"array-like\"/>\n"
                                   "  </container>\n"
                                   "  <container id=\"E\" startPattern=\"std :: E\"/>\n"
                                   "  <container id=\"F\" startPattern=\"std :: F\" itEndPattern=\":: iterator\"/>\n"
                                   "</def>";

        Library library;
        ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));

        const Library::Container& A = library.containers().at("A");
        const Library::Container& B = library.containers().at("B");
        const Library::Container& C = library.containers().at("C");
        const Library::Container& E = library.containers().at("E");
        const Library::Container& F = library.containers().at("F");

        ASSERT_EQUALS(A.type_templateArgNo, 1);
        ASSERT_EQUALS(A.size_templateArgNo, 4);
        ASSERT_EQUALS(A.startPattern, "std :: A <");
        ASSERT_EQUALS(A.endPattern, "> !!::");
        ASSERT_EQUALS(A.itEndPattern, "> :: iterator");
        ASSERT_EQUALS(A.stdStringLike, false);
        ASSERT_EQUALS(A.arrayLike_indexOp, false);
        ASSERT_EQUALS(A.opLessAllowed, true);
        ASSERT_EQ(Library::Container::Yield::SIZE, A.getYield("size"));
        ASSERT_EQ(Library::Container::Yield::EMPTY, A.getYield("empty"));
        ASSERT_EQ(Library::Container::Yield::AT_INDEX, A.getYield("at"));
        ASSERT_EQ(Library::Container::Yield::START_ITERATOR, A.getYield("begin"));
        ASSERT_EQ(Library::Container::Yield::END_ITERATOR, A.getYield("end"));
        ASSERT_EQ(Library::Container::Yield::BUFFER, A.getYield("data"));
        ASSERT_EQ(Library::Container::Yield::BUFFER_NT, A.getYield("c_str"));
        ASSERT_EQ(Library::Container::Yield::ITEM, A.getYield("front"));
        ASSERT_EQ(Library::Container::Yield::NO_YIELD, A.getYield("foo"));
        ASSERT_EQ(Library::Container::Action::RESIZE, A.getAction("resize"));
        ASSERT_EQ(Library::Container::Action::CLEAR, A.getAction("clear"));
        ASSERT_EQ(Library::Container::Action::PUSH, A.getAction("push_back"));
        ASSERT_EQ(Library::Container::Action::POP, A.getAction("pop_back"));
        ASSERT_EQ(Library::Container::Action::FIND, A.getAction("find"));
        ASSERT_EQ(Library::Container::Action::FIND_CONST, A.getAction("cfind"));
        ASSERT_EQ(Library::Container::Action::NO_ACTION, A.getAction("foo"));

        ASSERT_EQUALS(B.type_templateArgNo, 1);
        ASSERT_EQUALS(B.size_templateArgNo, 3);
        ASSERT_EQUALS(B.startPattern, "std :: B <");
        ASSERT_EQUALS(B.endPattern, "> !!::");
        ASSERT_EQUALS(B.itEndPattern, "> :: iterator");
        ASSERT_EQUALS(B.functions.size(), A.functions.size());
        ASSERT_EQUALS(B.opLessAllowed, false);

        ASSERT(C.functions.empty());
        ASSERT_EQUALS(C.type_templateArgNo, -1);
        ASSERT_EQUALS(C.size_templateArgNo, -1);
        ASSERT_EQUALS(C.stdStringLike, true);
        ASSERT_EQUALS(C.arrayLike_indexOp, true);

        ASSERT_EQUALS(E.startPattern, "std :: E");
        ASSERT_EQUALS(E.endPattern, "");
        ASSERT_EQUALS(E.itEndPattern, "");

        ASSERT_EQUALS(F.startPattern, "std :: F");
        ASSERT_EQUALS(F.endPattern, "");
        ASSERT_EQUALS(F.itEndPattern, ":: iterator");

        ASSERT(!library.detectContainerOrIterator(nullptr));

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::A<int> a;"));
            ASSERT_EQUALS(&A, library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            bool isIterator;
            ASSERT_EQUALS(&A, library.detectContainerOrIterator(var.tokens(), &isIterator));
            ASSERT(!isIterator);
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::A<int>::size_type a_s;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens()));
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::A<int>::iterator a_it;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT_EQUALS(&A, library.detectIterator(var.tokens()));
            bool isIterator;
            ASSERT_EQUALS(&A, library.detectContainerOrIterator(var.tokens(), &isIterator));
            ASSERT(isIterator);
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::B<int> b;"));
            ASSERT_EQUALS(&B, library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            bool isIterator;
            ASSERT_EQUALS(&B, library.detectContainerOrIterator(var.tokens(), &isIterator));
            ASSERT(!isIterator);
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::B<int>::size_type b_s;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens()));
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::B<int>::iterator b_it;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT_EQUALS(&B, library.detectIterator(var.tokens()));
            bool isIterator;
            ASSERT_EQUALS(&B, library.detectContainerOrIterator(var.tokens(), &isIterator));
            ASSERT(isIterator);
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("C c;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens()));
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("D d;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens()));
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::E e;"));
            ASSERT(library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            bool isIterator;
            ASSERT_EQUALS(&E, library.detectContainerOrIterator(var.tokens(), &isIterator));
            ASSERT(!isIterator);
            ASSERT(!library.detectContainerOrIterator(var.tokens(), nullptr, true));
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("E e;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens()));
            ASSERT_EQUALS(&E, library.detectContainerOrIterator(var.tokens(), nullptr, true));
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::E::iterator I;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens(), nullptr, true));
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::E::size_type p;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens(), nullptr, true));
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::F f;"));
            ASSERT(library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            bool isIterator;
            ASSERT_EQUALS(&F, library.detectContainerOrIterator(var.tokens(), &isIterator));
            ASSERT(!isIterator);
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("std::F::iterator I;"));
            ASSERT(!library.detectContainer(var.tokens()));
            TODO_ASSERT(library.detectIterator(var.tokens()));
            bool isIterator = false;
            TODO_ASSERT_EQUALS(reinterpret_cast<intptr_t>(&F), 0, reinterpret_cast<intptr_t>(library.detectContainerOrIterator(var.tokens(), &isIterator)));
            TODO_ASSERT(isIterator);
        }

        {
            SimpleTokenizer var(*this);
            ASSERT(var.tokenize("F::iterator I;"));
            ASSERT(!library.detectContainer(var.tokens()));
            ASSERT(!library.detectIterator(var.tokens()));
            ASSERT(!library.detectContainerOrIterator(var.tokens()));
            bool isIterator = false;
            TODO_ASSERT_EQUALS(reinterpret_cast<intptr_t>(&F), 0, reinterpret_cast<intptr_t>(library.detectContainerOrIterator(var.tokens(), &isIterator, true)));
            TODO_ASSERT(isIterator);
        }
    }

#define LOADLIBERROR(...) loadLibError(__FILE__, __LINE__, __VA_ARGS__)
    template<std::size_t size>
    void loadLibError(const char* file, unsigned line, const char (&xmldata)[size], Library::ErrorCode errorcode) const {
        Library library;
        Library::Error liberr;
        assertEquals(file, line, true, LibraryHelper::loadxmldata(library, liberr, xmldata, size-1));
        assertEquals(file, line, true, errorcode == liberr.errorcode);
    }

    void version() const {
        {
            constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                       "<def>\n"
                                       "</def>";
            Library library;
            ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        }
        {
            constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                       "<def format=\"1\">\n"
                                       "</def>";
            Library library;
            ASSERT(LibraryHelper::loadxmldata(library, xmldata, sizeof(xmldata)));
        }
        {
            constexpr char xmldata[] = "<?xml version=\"1.0\"?>\n"
                                       "<def format=\"42\">\n"
                                       "</def>";
            LOADLIBERROR(xmldata, Library::ErrorCode::UNSUPPORTED_FORMAT);
        }
    }

    void containerActionToFromString() const {
        for (uint16_t i = 0; i < static_cast<uint16_t>(Library::Container::Action::NO_ACTION); ++i) {
            const auto a = static_cast<Library::Container::Action>(i);
            const std::string& s = Library::Container::toString(a);
            ASSERT_EQUALS(i, static_cast<uint16_t>(Library::Container::actionFrom(s)));
        }
    }

    void containerYieldToFromString() const {
        for (uint16_t i = 0; i < static_cast<uint16_t>(Library::Container::Yield::NO_YIELD); ++i) {
            const auto y = static_cast<Library::Container::Yield>(i);
            const std::string& s = Library::Container::toString(y);
            ASSERT_EQUALS(i, static_cast<uint16_t>(Library::Container::yieldFrom(s)));
        }
    }

#define LOADLIB_ERROR_INVALID_RANGE(valid) LOADLIBERROR("<?xml version=\"1.0\"?>\n" \
                                                        "<def>\n" \
                                                        "<function name=\"f\">\n" \
                                                        "<arg nr=\"1\">\n" \
                                                        "<valid>" valid  "</valid>\n" \
                                                        "</arg>\n" \
                                                        "</function>\n" \
                                                        "</def>", \
                                                        Library::ErrorCode::BAD_ATTRIBUTE_VALUE)

    void loadLibErrors() const {

        LOADLIBERROR("<?xml version=\"1.0\"?>",
                     Library::ErrorCode::BAD_XML);

        LOADLIBERROR("<?xml version=\"1.0\"?>\n"
                     "<def>\n"
                     "   <X name=\"uint8_t,std::uint8_t\" size=\"1\"/>\n"
                     "</def>",
                     Library::ErrorCode::UNKNOWN_ELEMENT);

        // #define without attributes
        LOADLIBERROR("<?xml version=\"1.0\"?>\n"
                     "<def>\n"
                     "  <define />\n" // no attributes provided at all
                     "</def>",
                     Library::ErrorCode::MISSING_ATTRIBUTE);

        // #define with name but without value
        LOADLIBERROR("<?xml version=\"1.0\"?>\n"
                     "<def>\n"
                     "  <define name=\"foo\" />\n" // no value provided
                     "</def>",
                     Library::ErrorCode::MISSING_ATTRIBUTE);

        LOADLIBERROR("<?xml version=\"1.0\"?>\n"
                     "<def>\n"
                     "  <define value=\"1\" />\n" // no name provided
                     "</def>",
                     Library::ErrorCode::MISSING_ATTRIBUTE);

        LOADLIBERROR("<?xml version=\"1.0\"?>\n"
                     "<X>\n"
                     "</X>",
                     Library::ErrorCode::UNSUPPORTED_FORMAT);

        // empty range
        LOADLIB_ERROR_INVALID_RANGE("");

        // letter as range
        LOADLIB_ERROR_INVALID_RANGE("a");

        // letter and number as range
        LOADLIB_ERROR_INVALID_RANGE("1a");

        // digit followed by dash
        LOADLIB_ERROR_INVALID_RANGE("0:2-1");

        // single dash
        LOADLIB_ERROR_INVALID_RANGE("-");

        // range with multiple colons
        LOADLIB_ERROR_INVALID_RANGE("1:2:3");

        // extra dot
        LOADLIB_ERROR_INVALID_RANGE("1.0.0:10");

        // consecutive dots
        LOADLIB_ERROR_INVALID_RANGE("1..0:10");

        // dot followed by dash
        LOADLIB_ERROR_INVALID_RANGE("1.-0:10");

        // dot without preceding number
        LOADLIB_ERROR_INVALID_RANGE(".5:10");

        // dash followed by dot
        LOADLIB_ERROR_INVALID_RANGE("-.5:10");

        // colon followed by dot without preceding number
        LOADLIB_ERROR_INVALID_RANGE("0:.5");

        // colon followed by dash followed by dot
        LOADLIB_ERROR_INVALID_RANGE("-10:-.5");

        // dot not followed by number
        LOADLIB_ERROR_INVALID_RANGE("1:5.");

        // dot not followed by number
        LOADLIB_ERROR_INVALID_RANGE("1.:5");

        // dot followed by comma
        LOADLIB_ERROR_INVALID_RANGE("1:5.,6:10");

        // comma followed by dot
        LOADLIB_ERROR_INVALID_RANGE("-10:0,.5:");
    }

    void loadLibCombinations() const {
        {
            const Settings s = settingsBuilder().library("std.cfg").library("gnu.cfg").library("bsd.cfg").build();
            ASSERT_EQUALS(s.library.defines().empty(), false);
        }
        {
            const Settings s = settingsBuilder().library("std.cfg").library("microsoft_sal.cfg").build();
            ASSERT_EQUALS(s.library.defines().empty(), false);
        }
        {
            const Settings s = settingsBuilder().library("std.cfg").library("windows.cfg").library("mfc.cfg").build();
            ASSERT_EQUALS(s.library.defines().empty(), false);
        }
    }

    void smartpointer() const {
        const Settings s = settingsBuilder().library("std.cfg").build();
        const Library& library = s.library;

        ASSERT(!library.detectSmartPointer(nullptr));

        // TODO: add more tests
    }
};

REGISTER_TEST(TestLibrary)
