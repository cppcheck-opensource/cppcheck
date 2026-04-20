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

#include "fixture.h"
#include "helpers.h"
#include "settings.h"
#include "standards.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>


class TestToken : public TestFixture {
public:
    TestToken() : TestFixture("TestToken") {}

private:
    class TokenTest final : public Token
    {
        friend class TestToken;
    };

    const TokenList list{settingsDefault, Standards::Language::C};

    std::vector<std::string> arithmeticalOps;
    std::vector<std::string> logicalOps;
    std::vector<std::string> bitOps;
    std::vector<std::string> comparisonOps;
    std::vector<std::string> extendedOps;
    std::vector<std::string> assignmentOps;

    void run() override {
        arithmeticalOps = { "+", "-", "*", "/", "%", "<<", ">>" };
        logicalOps = { "&&", "||", "!" };
        comparisonOps = { "==", "!=", "<", "<=", ">", ">=" };
        bitOps = { "&", "|", "^", "~" };
        extendedOps = { ",", "[", "]", "(", ")", "?", ":" };
        assignmentOps = { "=", "+=", "-=", "*=", "/=", "%=", "&=", "^=", "|=", "<<=", ">>=" };

        TEST_CASE(nextprevious);
        TEST_CASE(multiCompare);
        TEST_CASE(multiCompare2);                   // #3294 - false negative multi compare between "=" and "=="
        TEST_CASE(multiCompare3);                   // false positive for %or% on code using "|="
        TEST_CASE(multiCompare4);
        TEST_CASE(multiCompare5);
        TEST_CASE(multiCompare6);
        TEST_CASE(charTypes);
        TEST_CASE(stringTypes);
        TEST_CASE(getStrLength);
        TEST_CASE(getStrSize);
        TEST_CASE(strValue);
        TEST_CASE(concatStr);

        TEST_CASE(deleteLast);
        TEST_CASE(deleteFirst);
        TEST_CASE(nextArgument);
        TEST_CASE(eraseTokens);

        TEST_CASE(matchAny);
        TEST_CASE(matchSingleChar);
        TEST_CASE(matchNothingOrAnyNotElse);
        TEST_CASE(matchType);
        TEST_CASE(matchChar);
        TEST_CASE(matchCompOp);
        TEST_CASE(matchStr);
        TEST_CASE(matchVarid);
        TEST_CASE(matchNumeric);
        TEST_CASE(matchBoolean);
        TEST_CASE(matchOr);
        TEST_CASE(matchOp);
        TEST_CASE(matchConstOp);

        TEST_CASE(isArithmeticalOp);
        TEST_CASE(isOp);
        TEST_CASE(isConstOp);
        TEST_CASE(isExtendedOp);
        TEST_CASE(isAssignmentOp);
        TEST_CASE(isStandardType);
        TEST_CASE(literals);
        TEST_CASE(operators);

        TEST_CASE(updateProperties);
        TEST_CASE(isNameGuarantees1);
        TEST_CASE(isNameGuarantees2);
        TEST_CASE(isNameGuarantees3);
        TEST_CASE(isNameGuarantees4);
        TEST_CASE(isNameGuarantees5);
        TEST_CASE(isNameGuarantees6);

        TEST_CASE(canFindMatchingBracketsNeedsOpen);
        TEST_CASE(canFindMatchingBracketsInnerPair);
        TEST_CASE(canFindMatchingBracketsOuterPair);
        TEST_CASE(canFindMatchingBracketsWithTooManyClosing);
        TEST_CASE(canFindMatchingBracketsWithTooManyOpening);
        TEST_CASE(findClosingBracket);
        TEST_CASE(findClosingBracket2);
        TEST_CASE(findClosingBracket3);
        TEST_CASE(findClosingBracket4);

        TEST_CASE(expressionString);

        TEST_CASE(hasKnownIntValue);

        TEST_CASE(update_property_info);
        TEST_CASE(update_property_info_evariable);
        TEST_CASE(update_property_info_ekeyword_c);
        TEST_CASE(update_property_info_ekeyword_cpp);
        TEST_CASE(update_property_info_ebracket_link);
        TEST_CASE(update_property_info_ecomparisonop_link);
        TEST_CASE(update_property_info_etype_c);
        TEST_CASE(update_property_info_etype_cpp);
        TEST_CASE(update_property_info_replace); // #13743

        TEST_CASE(varid_reset);
    }

    void nextprevious() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        auto *token = new Token(list, std::move(tokensFrontBack));
        token->str("1");
        (void)token->insertToken("2");
        (void)token->next()->insertToken("3");
        Token *last = token->tokAt(2);
        ASSERT_EQUALS(token->str(), "1");
        ASSERT_EQUALS(token->strAt(1), "2");
        ASSERT_EQUALS(token->tokAt(2)->str(), "3");
        ASSERT_EQUALS_MSG(true, last->next() == nullptr, "Null was expected");

        ASSERT_EQUALS(last->str(), "3");
        ASSERT_EQUALS(last->strAt(-1), "2");
        ASSERT_EQUALS(last->tokAt(-2)->str(), "1");
        ASSERT_EQUALS_MSG(true, token->previous() == nullptr, "Null was expected");

        TokenList::deleteTokens(token);
    }

#define MatchCheck(...) MatchCheck_(__FILE__, __LINE__, __VA_ARGS__)
    bool MatchCheck_(const char* file, int line, const std::string& code, const std::string& pattern) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        const std::string code2 = ";" + code + ";";
        try {
            ASSERT_LOC(tokenizer.tokenize(code2), file, line);
        } catch (...) {}
        return Token::Match(tokenizer.tokens()->next(), pattern.c_str());
    }

    void multiCompare() const {
        // Test for found
        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token one(list, std::move(tokensFrontBack));
            one.str("one");
            ASSERT_EQUALS(1, TokenTest::multiCompare(&one, "one|two", 0));
        }

        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token two(list, std::move(tokensFrontBack));
            two.str("two");
            ASSERT_EQUALS(1, TokenTest::multiCompare(&two, "one|two", 0));
            ASSERT_EQUALS(1, TokenTest::multiCompare(&two, "verybig|two|", 0));
        }

        // Test for empty string found
        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token notfound(list, std::move(tokensFrontBack));
            notfound.str("notfound");
            ASSERT_EQUALS(0, TokenTest::multiCompare(&notfound, "one|two|", 0));

            // Test for not found
            ASSERT_EQUALS(-1, TokenTest::multiCompare(&notfound, "one|two", 0));
        }

        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token s(list, std::move(tokensFrontBack));
            s.str("s");
            ASSERT_EQUALS(-1, TokenTest::multiCompare(&s, "verybig|two", 0));
        }

        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token ne(list, std::move(tokensFrontBack));
            ne.str("ne");
            ASSERT_EQUALS(-1, TokenTest::multiCompare(&ne, "one|two", 0));
        }

        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token a(list, std::move(tokensFrontBack));
            a.str("a");
            ASSERT_EQUALS(-1, TokenTest::multiCompare(&a, "abc|def", 0));
        }

        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token abcd(list, std::move(tokensFrontBack));
            abcd.str("abcd");
            ASSERT_EQUALS(-1, TokenTest::multiCompare(&abcd, "abc|def", 0));
        }

        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token def(list, std::move(tokensFrontBack));
            def.str("default");
            ASSERT_EQUALS(-1, TokenTest::multiCompare(&def, "abc|def", 0));
        }

        // %op%
        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token plus(list, std::move(tokensFrontBack));
            plus.str("+");
            ASSERT_EQUALS(1, TokenTest::multiCompare(&plus, "one|%op%", 0));
            ASSERT_EQUALS(1, TokenTest::multiCompare(&plus, "%op%|two", 0));
        }
        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token x(list, std::move(tokensFrontBack));
            x.str("x");
            ASSERT_EQUALS(-1, TokenTest::multiCompare(&x, "one|%op%", 0));
            ASSERT_EQUALS(-1, TokenTest::multiCompare(&x, "%op%|two", 0));
        }

    }

    void multiCompare2() const { // #3294
        // Original pattern that failed: [[,(=<>+-*|&^] %num% [+-*/] %num% ]|,|)|;|=|%op%
        const SimpleTokenList toks("a == 1");
        ASSERT_EQUALS(true, Token::Match(toks.front(), "a =|%op%"));
    }

    void multiCompare3() const {
        // Original pattern that failed: "return|(|&&|%oror% %name% &&|%oror%|==|!=|<=|>=|<|>|-|%or% %name% )|&&|%oror%|;"
        // Code snippet that failed: "return lv@86 |= rv@87 ;"

        // Note: Also test "reverse" alternative pattern, two different code paths to handle it
        const SimpleTokenList toks("return a |= b ;");
        ASSERT_EQUALS(false, Token::Match(toks.front(), "return %name% xyz|%or% %name% ;"));
        ASSERT_EQUALS(false, Token::Match(toks.front(), "return %name% %or%|xyz %name% ;"));

        const SimpleTokenList toks2("return a | b ;");
        ASSERT_EQUALS(true, Token::Match(toks2.front(), "return %name% xyz|%or% %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks2.front(), "return %name% %or%|xyz %name% ;"));

        const SimpleTokenList toks3("return a || b ;");
        ASSERT_EQUALS(false, Token::Match(toks3.front(), "return %name% xyz|%or% %name% ;"));
        ASSERT_EQUALS(false, Token::Match(toks3.front(), "return %name% %or%|xyz %name% ;"));

        ASSERT_EQUALS(true, Token::Match(toks3.front(), "return %name% xyz|%oror% %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks3.front(), "return %name% %oror%|xyz %name% ;"));

        const SimpleTokenList toks4("a % b ;");
        ASSERT_EQUALS(true, Token::Match(toks4.front(), "%name% >>|<<|&|%or%|^|% %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks4.front(), "%name% %|>>|<<|&|%or%|^ %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks4.front(), "%name% >>|<<|&|%or%|%|^ %name% ;"));

        //%name%|%num% support
        const SimpleTokenList num("100");
        ASSERT_EQUALS(true, Token::Match(num.front(), "%num%|%name%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%name%|%num%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%name%|%num%|%bool%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%name%|%bool%|%num%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%name%|%bool%|%str%|%num%"));
        ASSERT_EQUALS(false, Token::Match(num.front(), "%bool%|%name%"));
        ASSERT_EQUALS(false, Token::Match(num.front(), "%type%|%bool%|%char%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%type%|%bool%|100"));

        const SimpleTokenList numparen("( 100 )");
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %num%|%name% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %name%|%num% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %name%|%num%|%bool% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %name%|%bool%|%num% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %name%|%bool%|%str%|%num% )|"));
        ASSERT_EQUALS(false, Token::Match(numparen.front(), "(| %bool%|%name% )|"));

        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %num%|%name%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %name%|%num%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %name%|%num%|%bool%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %name%|%bool%|%num%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %name%|%bool%|%str%|%num%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %bool%|%name%| )|"));
    }

    void multiCompare4() {
        SimpleTokenizer var(*this);
        ASSERT(var.tokenize("std :: queue < int > foo ;"));

        ASSERT_EQUALS(Token::eBracket, var.tokens()->tokAt(3)->tokType());
        ASSERT_EQUALS(Token::eBracket, var.tokens()->tokAt(5)->tokType());

        ASSERT_EQUALS(false, Token::Match(var.tokens(), "std :: queue %op%"));
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "std :: queue x|%op%"));
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "std :: queue %op%|x"));
    }

    void multiCompare5() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("||");
        ASSERT_EQUALS(true, TokenTest::multiCompare(&tok, "+|%or%|%oror%", 0) >= 0);
    }

    void multiCompare6() const {
        {
            const SimpleTokenList stl("x %= y;");
            ASSERT_EQUALS(true, Token::Match(stl.front(), "%name% %= %name%"));
        }
        {
            const SimpleTokenList stl("x += y;");
            ASSERT_EQUALS(false, Token::Match(stl.front(), "%name% %= %name%"));
        }
    }

    void charTypes() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));

        tok.str("'a'");
        ASSERT_EQUALS(true, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("u8'a'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(true, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("u'a'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(true, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("U'a'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(true, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("L'a'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(true, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("'aaa'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(true, tok.isCMultiChar());

        tok.str("'\\''");
        ASSERT_EQUALS(true, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("'\\r\\n'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(true, tok.isCMultiChar());

        tok.str("'\\x10'");
        ASSERT_EQUALS(true, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());
    }

    void stringTypes() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));

        tok.str("\"a\"");
        ASSERT_EQUALS(true, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("u8\"a\"");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(true, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("u\"a\"");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(true, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("U\"a\"");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(true, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("L\"a\"");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(true, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());
    }

    void getStrLength() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));

        tok.str("\"\"");
        ASSERT_EQUALS(0, Token::getStrLength(&tok));

        tok.str("\"test\"");
        ASSERT_EQUALS(4, Token::getStrLength(&tok));

        tok.str("\"test \\\\test\"");
        ASSERT_EQUALS(10, Token::getStrLength(&tok));

        tok.str("\"a\\0\"");
        ASSERT_EQUALS(1, Token::getStrLength(&tok));

        tok.str("L\"\"");
        ASSERT_EQUALS(0, Token::getStrLength(&tok));

        tok.str("u8\"test\"");
        ASSERT_EQUALS(4, Token::getStrLength(&tok));

        tok.str("U\"test \\\\test\"");
        ASSERT_EQUALS(10, Token::getStrLength(&tok));

        tok.str("u\"a\\0\"");
        ASSERT_EQUALS(1, Token::getStrLength(&tok));
    }

    void getStrSize() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));

        tok.str("\"\"");
        ASSERT_EQUALS(sizeof(""), Token::getStrSize(&tok, settingsDefault));

        tok.str("\"abc\"");
        ASSERT_EQUALS(sizeof("abc"), Token::getStrSize(&tok, settingsDefault));

        tok.str("\"\\0abc\"");
        ASSERT_EQUALS(sizeof("\0abc"), Token::getStrSize(&tok, settingsDefault));

        tok.str("\"\\\\\"");
        ASSERT_EQUALS(sizeof("\\"), Token::getStrSize(&tok, settingsDefault));
    }

    void strValue() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));

        tok.str("\"\"");
        ASSERT_EQUALS("", tok.strValue());

        tok.str("\"0\"");
        ASSERT_EQUALS("0", tok.strValue());

        tok.str("\"a\\n\"");
        ASSERT_EQUALS("a\n", tok.strValue());

        tok.str("\"a\\r\"");
        ASSERT_EQUALS("a\r", tok.strValue());

        tok.str("\"a\\t\"");
        ASSERT_EQUALS("a\t", tok.strValue());

        tok.str("\"\\\\\"");
        ASSERT_EQUALS("\\", tok.strValue());

        tok.str("\"a\\0\"");
        ASSERT_EQUALS("a", tok.strValue());

        tok.str("L\"a\\t\"");
        ASSERT_EQUALS("a\t", tok.strValue());

        tok.str("U\"a\\0\"");
        ASSERT_EQUALS("a", tok.strValue());
    }

    void concatStr() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));

        tok.str("\"\"");
        tok.concatStr("\"\"");
        ASSERT_EQUALS("", tok.strValue());
        ASSERT(tok.isCChar());

        tok.str("\"ab\"");
        tok.concatStr("\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isCChar());

        tok.str("L\"ab\"");
        tok.concatStr("L\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isLong());

        tok.str("L\"ab\"");
        tok.concatStr("\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isLong());

        tok.str("\"ab\"");
        tok.concatStr("L\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isLong());

        tok.str("\"ab\"");
        tok.concatStr("L\"\"");
        ASSERT_EQUALS("ab", tok.strValue());
        ASSERT(tok.isLong());

        tok.str("\"ab\"");
        tok.concatStr("u8\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isUtf8());
    }

    void deleteLast() const {
        auto listEnds = std::make_shared<TokensFrontBack>();
        Token ** const tokensBack = &(listEnds->back);
        Token tok(list, listEnds);
        (void)tok.insertToken("aba");
        ASSERT_EQUALS(true, *tokensBack == tok.next());
        tok.deleteNext();
        ASSERT_EQUALS(true, *tokensBack == &tok);
    }

    void deleteFirst() const {
        auto listEnds = std::make_shared<TokensFrontBack>();
        Token ** const tokensFront = &(listEnds->front);
        Token tok(list, listEnds);

        (void)tok.insertToken("aba");

        ASSERT_EQUALS(true, *tokensFront == tok.previous());
        tok.deletePrevious();
        ASSERT_EQUALS(true, *tokensFront == &tok);
    }

    void nextArgument() {
        SimpleTokenizer example1(*this);
        ASSERT(example1.tokenize("foo(1, 2, 3, 4);"));
        ASSERT_EQUALS(true, Token::simpleMatch(example1.tokens()->tokAt(2)->nextArgument(), "2 , 3"));
        ASSERT_EQUALS(true, Token::simpleMatch(example1.tokens()->tokAt(4)->nextArgument(), "3 , 4"));

        SimpleTokenizer example2(*this);
        ASSERT(example2.tokenize("foo();"));
        ASSERT_EQUALS(true, example2.tokens()->tokAt(2)->nextArgument() == nullptr);

        SimpleTokenizer example3(*this);
        ASSERT(example3.tokenize("foo(bar(a, b), 2, 3);"));
        ASSERT_EQUALS(true, Token::simpleMatch(example3.tokens()->tokAt(2)->nextArgument(), "2 , 3"));

        SimpleTokenizer example4(*this);
        ASSERT(example4.tokenize("foo(x.i[1], \"\", 3);"));
        ASSERT_EQUALS(true, Token::simpleMatch(example4.tokens()->tokAt(2)->nextArgument(), "\"\" , 3"));
    }

    void eraseTokens() const {
        SimpleTokenList code("begin ; { this code will be removed } end", Standards::Language::C);
        Token::eraseTokens(code.front()->next(), code.front()->tokAt(9));
        ASSERT_EQUALS("begin ; end", code.front()->stringifyList(nullptr, false));
    }


    void matchAny() const {
        const SimpleTokenList varBitOrVar("abc|def");
        ASSERT_EQUALS(true, Token::Match(varBitOrVar.front(), "%name% %or% %name%"));

        const SimpleTokenList varLogOrVar("abc||def");
        ASSERT_EQUALS(true, Token::Match(varLogOrVar.front(), "%name% %oror% %name%"));
    }

    void matchSingleChar() const {
        const SimpleTokenList singleChar("a");
        ASSERT_EQUALS(true, Token::Match(singleChar.front(), "[a|bc]"));
        ASSERT_EQUALS(false, Token::Match(singleChar.front(), "[d|ef]"));

        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token multiChar(list, std::move(tokensFrontBack));
        multiChar.str("[ab");
        ASSERT_EQUALS(false, Token::Match(&multiChar, "[ab|def]"));
    }

    void matchNothingOrAnyNotElse() const {
        const SimpleTokenList empty_String("");
        ASSERT_EQUALS(true, Token::Match(empty_String.front(), "!!else"));
        ASSERT_EQUALS(false, Token::Match(empty_String.front(), "!!else something"));

        const SimpleTokenList ifSemicolon("if ;");
        ASSERT_EQUALS(true, Token::Match(ifSemicolon.front(), "if ; !!else"));

        const SimpleTokenList ifSemicolonSomething("if ; something");
        ASSERT_EQUALS(true, Token::Match(ifSemicolonSomething.front(), "if ; !!else"));

        const SimpleTokenList justElse("else");
        ASSERT_EQUALS(false, Token::Match(justElse.front(), "!!else"));

        const SimpleTokenList ifSemicolonElse("if ; else");
        ASSERT_EQUALS(false, Token::Match(ifSemicolonElse.front(), "if ; !!else"));
    }

    void matchType() {
        const SimpleTokenList type("abc");
        ASSERT_EQUALS(true, Token::Match(type.front(), "%type%"));

        SimpleTokenizer isVar(*this);
        ASSERT(isVar.tokenize("int a = 3 ;"));
        ASSERT_EQUALS(true, Token::Match(isVar.tokens(), "%type%"));
        ASSERT_EQUALS(true, Token::Match(isVar.tokens(), "%type% %name%"));
        ASSERT_EQUALS(false, Token::Match(isVar.tokens(), "%type% %type%"));

        // TODO: %type% should not match keywords other than fundamental types
        const SimpleTokenList noType1_cpp("delete");
        ASSERT_EQUALS(true, Token::Match(noType1_cpp.front(), "%type%"));

        const SimpleTokenList noType1_c("delete", Standards::Language::C);
        ASSERT_EQUALS(true, Token::Match(noType1_c.front(), "%type%"));

        const SimpleTokenList noType2("void delete");
        ASSERT_EQUALS(true, Token::Match(noType2.front(), "!!foo %type%"));
    }

    void matchChar() const {
        const SimpleTokenList chr1("'a'");
        ASSERT_EQUALS(true, Token::Match(chr1.front(), "%char%"));

        const SimpleTokenList chr2("'1'");
        ASSERT_EQUALS(true, Token::Match(chr2.front(), "%char%"));

        const SimpleTokenList noChr("\"10\"");
        ASSERT_EQUALS(false, Token::Match(noChr.front(), "%char%"));
    }

    void matchCompOp() const {
        const SimpleTokenList comp1("<=");
        ASSERT_EQUALS(true, Token::Match(comp1.front(), "%comp%"));

        const SimpleTokenList comp2(">");
        ASSERT_EQUALS(true, Token::Match(comp2.front(), "%comp%"));

        const SimpleTokenList noComp("=");
        ASSERT_EQUALS(false, Token::Match(noComp.front(), "%comp%"));
    }

    void matchStr() const {
        const SimpleTokenList noStr1("abc");
        ASSERT_EQUALS(false, Token::Match(noStr1.front(), "%str%"));

        const SimpleTokenList noStr2("'a'");
        ASSERT_EQUALS(false, Token::Match(noStr2.front(), "%str%"));

        const SimpleTokenList str("\"abc\"");
        ASSERT_EQUALS(true, Token::Match(str.front(), "%str%"));

        // Empty string
        const SimpleTokenList emptyStr("\"\"");
        ASSERT_EQUALS(true, Token::Match(emptyStr.front(), "%str%"));
    }

    void matchVarid() {
        SimpleTokenizer var(*this);
        ASSERT(var.tokenize("int a ; int b ;"));

        // Varid == 0 should throw exception
        ASSERT_THROW_INTERNAL_EQUALS((void)Token::Match(var.tokens(), "%type% %varid% ; %type% %name%", 0),INTERNAL,"Internal error. Token::Match called with varid 0.");

        ASSERT_EQUALS(true, Token::Match(var.tokens(), "%type% %varid% ; %type% %name%", 1));
        ASSERT_EQUALS(true, Token::Match(var.tokens(), "%type% %name% ; %type% %varid%", 2));

        // Try to match two different varids in one match call
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "%type% %varid% ; %type% %varid%", 2));

        // %var% matches with every varid other than 0
        ASSERT_EQUALS(true, Token::Match(var.tokens(), "%type% %var% ;"));
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "%var% %var% ;"));
    }

    void matchNumeric() const {
        const SimpleTokenList nonNumeric("abc");
        ASSERT_EQUALS(false, Token::Match(nonNumeric.front(), "%num%"));

        const SimpleTokenList msLiteral("5ms"); // #11438
        ASSERT_EQUALS(false, Token::Match(msLiteral.front(), "%num%"));

        const SimpleTokenList sLiteral("3s");
        ASSERT_EQUALS(false, Token::Match(sLiteral.front(), "%num%"));

        const SimpleTokenList octal("0123");
        ASSERT_EQUALS(true, Token::Match(octal.front(), "%num%"));

        const SimpleTokenList decimal("4567");
        ASSERT_EQUALS(true, Token::Match(decimal.front(), "%num%"));

        const SimpleTokenList hexadecimal("0xDEADBEEF");
        ASSERT_EQUALS(true, Token::Match(hexadecimal.front(), "%num%"));

        const SimpleTokenList floatingPoint("0.0f");
        ASSERT_EQUALS(true, Token::Match(floatingPoint.front(), "%num%"));

        const SimpleTokenList signedLong("0L");
        ASSERT_EQUALS(true, Token::Match(signedLong.front(), "%num%"));

        const SimpleTokenList negativeSignedLong("-0L");
        ASSERT_EQUALS(true, Token::Match(negativeSignedLong.front(), "- %num%"));

        const SimpleTokenList positiveSignedLong("+0L");
        ASSERT_EQUALS(true, Token::Match(positiveSignedLong.front(), "+ %num%"));

        const SimpleTokenList unsignedInt("0U");
        ASSERT_EQUALS(true, Token::Match(unsignedInt.front(), "%num%"));

        const SimpleTokenList unsignedLong("0UL");
        ASSERT_EQUALS(true, Token::Match(unsignedLong.front(), "%num%"));

        const SimpleTokenList unsignedLongLong("0ULL");
        ASSERT_EQUALS(true, Token::Match(unsignedLongLong.front(), "%num%"));

        const SimpleTokenList positive("+666");
        ASSERT_EQUALS(true, Token::Match(positive.front(), "+ %num%"));

        const SimpleTokenList negative("-42");
        ASSERT_EQUALS(true, Token::Match(negative.front(), "- %num%"));

        const SimpleTokenList negativeNull("-.0");
        ASSERT_EQUALS(true, Token::Match(negativeNull.front(), "- %num%"));

        const SimpleTokenList positiveNull("+.0");
        ASSERT_EQUALS(true, Token::Match(positiveNull.front(), "+ %num%"));
    }


    void matchBoolean() const {
        const SimpleTokenList yes("YES");
        ASSERT_EQUALS(false, Token::Match(yes.front(), "%bool%"));

        const SimpleTokenList positive("true");
        ASSERT_EQUALS(true, Token::Match(positive.front(), "%bool%"));

        const SimpleTokenList negative("false");
        ASSERT_EQUALS(true, Token::Match(negative.front(), "%bool%"));
    }

    void matchOr() const {
        const SimpleTokenList bitwiseOr(";|;");
        ASSERT_EQUALS(true,  Token::Match(bitwiseOr.front(), "; %or%"));
        ASSERT_EQUALS(true,  Token::Match(bitwiseOr.front(), "; %op%"));
        ASSERT_EQUALS(false, Token::Match(bitwiseOr.front(), "; %oror%"));

        const SimpleTokenList bitwiseOrAssignment(";|=;");
        ASSERT_EQUALS(false,  Token::Match(bitwiseOrAssignment.front(), "; %or%"));
        ASSERT_EQUALS(true,  Token::Match(bitwiseOrAssignment.front(), "; %op%"));
        ASSERT_EQUALS(false, Token::Match(bitwiseOrAssignment.front(), "; %oror%"));

        const SimpleTokenList logicalOr(";||;");
        ASSERT_EQUALS(false, Token::Match(logicalOr.front(), "; %or%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.front(), "; %op%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.front(), "; %oror%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.front(), "; &&|%oror%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.front(), "; %oror%|&&"));

        const SimpleTokenList logicalAnd(";&&;");
        ASSERT_EQUALS(true, Token::simpleMatch(logicalAnd.front(), "; &&"));
        ASSERT_EQUALS(true, Token::Match(logicalAnd.front(), "; &&|%oror%"));
        ASSERT_EQUALS(true, Token::Match(logicalAnd.front(), "; %oror%|&&"));
    }

    static void append_vector(std::vector<std::string> &dest, const std::vector<std::string> &src) {
        dest.insert(dest.end(), src.cbegin(), src.cend());
    }

    void matchOp() {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);
        append_vector(test_ops, assignmentOps);

        ASSERT_EQUALS(true, std::all_of(test_ops.cbegin(), test_ops.cend(), [&](const std::string& s) {
            return MatchCheck(s, "%op%");
        }));

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            ASSERT_EQUALS_MSG(false, MatchCheck(*other_op, "%op%"), "Failing other operator: " + *other_op);
        }
    }

    void matchConstOp() {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);

        ASSERT_EQUALS(true, std::all_of(test_ops.cbegin(), test_ops.cend(), [&](const std::string& s) {
            return MatchCheck(s, "%cop%");
        }));

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);
        append_vector(other_ops, assignmentOps);

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            ASSERT_EQUALS_MSG(false, MatchCheck(*other_op, "%cop%"), "Failing other operator: " + *other_op);
        }
    }


    void isArithmeticalOp() const {
        for (auto test_op = arithmeticalOps.cbegin(); test_op != arithmeticalOps.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isArithmeticalOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, bitOps);
        append_vector(other_ops, comparisonOps);
        append_vector(other_ops, logicalOps);
        append_vector(other_ops, extendedOps);
        append_vector(other_ops, assignmentOps);

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isArithmeticalOp(), "Failing arithmetical operator: " + *other_op);
        }
    }

    void isOp() const {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);
        append_vector(test_ops, assignmentOps);

        for (auto test_op = test_ops.cbegin(); test_op != test_ops.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isOp(), "Failing normal operator: " + *other_op);
        }
    }

    void isConstOp() const {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);

        for (auto test_op = test_ops.cbegin(); test_op != test_ops.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isConstOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);
        append_vector(other_ops, assignmentOps);

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isConstOp(), "Failing normal operator: " + *other_op);
        }
    }

    void isExtendedOp() const {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);
        append_vector(test_ops, extendedOps);

        for (auto test_op = test_ops.cbegin(); test_op != test_ops.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isExtendedOp());
        }

        // Negative test against assignment operators
        for (auto other_op = assignmentOps.cbegin(); other_op != assignmentOps.cend(); ++other_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isExtendedOp(), "Failing assignment operator: " + *other_op);
        }
    }

    void isAssignmentOp() const {
        for (auto test_op = assignmentOps.cbegin(); test_op != assignmentOps.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isAssignmentOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, arithmeticalOps);
        append_vector(other_ops, bitOps);
        append_vector(other_ops, comparisonOps);
        append_vector(other_ops, logicalOps);
        append_vector(other_ops, extendedOps);

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isAssignmentOp(), "Failing assignment operator: " + *other_op);
        }
    }

    void operators() const {
        for (auto test_op = extendedOps.cbegin(); test_op != extendedOps.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eExtendedOp, tok.tokType());
        }
        for (auto test_op = logicalOps.cbegin(); test_op != logicalOps.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eLogicalOp, tok.tokType());
        }
        for (auto test_op = bitOps.cbegin(); test_op != bitOps.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eBitOp, tok.tokType());
        }
        for (auto test_op = comparisonOps.cbegin(); test_op != comparisonOps.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eComparisonOp, tok.tokType());
        }
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("++");
        ASSERT_EQUALS(Token::eIncDecOp, tok.tokType());
        tok.str("--");
        ASSERT_EQUALS(Token::eIncDecOp, tok.tokType());
    }

    void literals() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));

        tok.str("\"foo\"");
        ASSERT(tok.tokType() == Token::eString);
        tok.str("\"\"");
        ASSERT(tok.tokType() == Token::eString);
        tok.str("'f'");
        ASSERT(tok.tokType() == Token::eChar);
        tok.str("12345");
        ASSERT(tok.tokType() == Token::eNumber);
        tok.str("-55");
        ASSERT(tok.tokType() == Token::eNumber);
        tok.str("true");
        ASSERT(tok.tokType() == Token::eBoolean);
        tok.str("false");
        ASSERT(tok.tokType() == Token::eBoolean);
    }

    void isStandardType() const {
        std::vector<std::string> standard_types;
        standard_types.emplace_back("bool");
        standard_types.emplace_back("char");
        standard_types.emplace_back("short");
        standard_types.emplace_back("int");
        standard_types.emplace_back("long");
        standard_types.emplace_back("float");
        standard_types.emplace_back("double");
        standard_types.emplace_back("size_t");

        for (auto test_op = standard_types.cbegin(); test_op != standard_types.cend(); ++test_op) {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str(*test_op);
            ASSERT_EQUALS_MSG(true, tok.isStandardType(), "Failing standard type: " + *test_op);
        }

        // Negative test
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("string");
        ASSERT_EQUALS(false, tok.isStandardType());

        // Change back to standard type
        tok.str("int");
        ASSERT_EQUALS(true, tok.isStandardType());

        // token can't be both type and variable
        tok.str("abc");
        tok.isStandardType(true);
        tok.varId(123);
        ASSERT_EQUALS(false, tok.isStandardType());
    }

    void updateProperties() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("foobar");

        ASSERT_EQUALS(true, tok.isName());
        ASSERT_EQUALS(false, tok.isNumber());

        tok.str("123456");

        ASSERT_EQUALS(false, tok.isName());
        ASSERT_EQUALS(true, tok.isNumber());
    }

    void isNameGuarantees1() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("Name");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees2() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("_name");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees3() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("_123");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees4() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("123456");
        ASSERT_EQUALS(false, tok.isName());
        ASSERT_EQUALS(true, tok.isNumber());
    }

    void isNameGuarantees5() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("a123456");
        ASSERT_EQUALS(true, tok.isName());
        ASSERT_EQUALS(false, tok.isNumber());
    }

    void isNameGuarantees6() const {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("$f");
        ASSERT_EQUALS(true, tok.isName());
    }

    void canFindMatchingBracketsNeedsOpen() {
        SimpleTokenizer var(*this);
        ASSERT(var.tokenize("std::deque<std::set<int> > intsets;"));

        const Token* const t = var.tokens()->findClosingBracket();
        ASSERT(t == nullptr);
    }

    void canFindMatchingBracketsInnerPair() {
        SimpleTokenizer var(*this);
        ASSERT(var.tokenize("std::deque<std::set<int> > intsets;"));

        const Token * const t = var.tokens()->tokAt(7)->findClosingBracket();
        ASSERT_EQUALS(">", t->str());
        ASSERT(var.tokens()->tokAt(9) == t);
    }

    void canFindMatchingBracketsOuterPair() {
        SimpleTokenizer var(*this);
        ASSERT(var.tokenize("std::deque<std::set<int> > intsets;"));

        const Token* const t = var.tokens()->tokAt(3)->findClosingBracket();
        ASSERT_EQUALS(">", t->str());
        ASSERT(var.tokens()->tokAt(10) == t);
    }

    void canFindMatchingBracketsWithTooManyClosing() {
        SimpleTokenizer var(*this);
        ASSERT(var.tokenize("X< 1>2 > x1;"));

        const Token* const t = var.tokens()->next()->findClosingBracket();
        ASSERT_EQUALS(">", t->str());
        ASSERT(var.tokens()->tokAt(3) == t);
    }

    void canFindMatchingBracketsWithTooManyOpening() {
        SimpleTokenizer var(*this);
        ASSERT(var.tokenize("X < (2 < 1) > x1;"));

        const Token* t = var.tokens()->next()->findClosingBracket();
        ASSERT(t != nullptr && t->str() == ">");

        t = var.tokens()->tokAt(4)->findClosingBracket();
        ASSERT(t == nullptr);
    }

    void findClosingBracket() {
        SimpleTokenizer var(*this);
        ASSERT(var.tokenize("template<typename X, typename...Y> struct S : public Fred<Wilma<Y...>> {}"));

        const Token* const t = var.tokens()->next()->findClosingBracket();
        ASSERT(Token::simpleMatch(t, "> struct"));
    }

    void findClosingBracket2() {
        SimpleTokenizer var(*this); // #11275
        ASSERT(var.tokenize("const auto g = []<typename T>() {};\n"));

        const Token* const t = Token::findsimplematch(var.tokens(), "<");
        ASSERT(t && Token::simpleMatch(t->findClosingBracket(), ">"));
    }

    void findClosingBracket3() {
        SimpleTokenizer var(*this); // #12789
        ASSERT(var.tokenize("template <size_t I = 0, typename... ArgsT, std::enable_if_t<I < sizeof...(ArgsT)>* = nullptr>\n"
                            "void f();\n"));
        const Token* const t = Token::findsimplematch(var.tokens(), "<");
        ASSERT(t && Token::simpleMatch(t->findClosingBracket(), ">"));
    }

    void findClosingBracket4() {
        SimpleTokenizer var(*this); // #12923
        ASSERT(var.tokenize("template<template<class E> class T = std::vector, class U = std::vector<int>, class V = void>\n"
                            "class C;\n"));
        const Token *const t = Token::findsimplematch(var.tokens(), "<");
        ASSERT(t);
        const Token *const closing = t->findClosingBracket();
        ASSERT(closing && closing == var.tokens()->tokAt(28));
    }

    void expressionString() {
        SimpleTokenizer var1(*this);
        ASSERT(var1.tokenize("void f() { *((unsigned long long *)x) = 0; }"));
        const Token *const tok1 = Token::findsimplematch(var1.tokens(), "*");
        ASSERT_EQUALS("*((unsigned long long*)x)", tok1->expressionString());

        SimpleTokenizer var2(*this);
        ASSERT(var2.tokenize("typedef unsigned long long u64; void f() { *((u64 *)x) = 0; }"));
        const Token *const tok2 = Token::findsimplematch(var2.tokens(), "*");
        ASSERT_EQUALS("*((unsigned long long*)x)", tok2->expressionString());

        SimpleTokenizer data3(*this);
        ASSERT(data3.tokenize("void f() { return (t){1,2}; }"));
        ASSERT_EQUALS("return(t){1,2}", data3.tokens()->tokAt(5)->expressionString());

        SimpleTokenizer data4(*this);
        ASSERT(data4.tokenize("void f() { return L\"a\"; }"));
        ASSERT_EQUALS("returnL\"a\"", data4.tokens()->tokAt(5)->expressionString());

        SimpleTokenizer data5(*this);
        ASSERT(data5.tokenize("void f() { return U\"a\"; }"));
        ASSERT_EQUALS("returnU\"a\"", data5.tokens()->tokAt(5)->expressionString());

        SimpleTokenizer data6(*this);
        ASSERT(data6.tokenize("x = \"\\0\\x1\\x2\\x3\\x4\\x5\\x6\\x7\";"));
        ASSERT_EQUALS("x=\"\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\"", data6.tokens()->next()->expressionString());
    }

    void hasKnownIntValue() const {
        // pointer might be NULL
        ValueFlow::Value v1(0);

        // pointer points at buffer that is 2 bytes
        ValueFlow::Value v2(2);
        v2.valueType = ValueFlow::Value::ValueType::BUFFER_SIZE;
        v2.setKnown();

        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token token(list, std::move(tokensFrontBack));
        ASSERT_EQUALS(true, token.addValue(v1));
        ASSERT_EQUALS(true, token.addValue(v2));
        ASSERT_EQUALS(false, token.hasKnownIntValue());
    }

#define assert_tok(...) _assert_tok(__FILE__, __LINE__, __VA_ARGS__)
    void _assert_tok(const char* file, int line, const Token* tok, Token::Type t, bool l = false, bool std = false, bool ctrl = false) const
    {
        ASSERT_LOC_MSG(tok, "tok", file, line);
        ASSERT_EQUALS_ENUM_LOC_MSG(t, tok->tokType(), "tokType", file, line);
        ASSERT_EQUALS_LOC_MSG(l, tok->isLong(), "isLong", file, line);
        ASSERT_EQUALS_LOC_MSG(std, tok->isStandardType(), "isStandardType", file, line);
        ASSERT_EQUALS_LOC_MSG(ctrl, tok->isControlFlowKeyword(), "isControlFlowKeyword", file, line);
    }

    void _assert_tok(const char* file, int line, const std::string& s, Token::Type t, bool l = false, bool std = false, bool ctrl = false) const
    {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str(s);
        _assert_tok(file, line, &tok, t, l, std, ctrl);
    }

    void update_property_info() const
    {
        assert_tok("", Token::Type::eNone);
        assert_tok("true", Token::Type::eBoolean);
        assert_tok("false", Token::Type::eBoolean);
        assert_tok("\"\"", Token::Type::eString);
        assert_tok("L\"\"", Token::Type::eString, /*l=*/ true);
        assert_tok("'a'", Token::Type::eChar);
        assert_tok("L'a'", Token::Type::eChar, /*l=*/ true);
        // eVariable has separate test
        // eKeyword with specific languages and standards has separate tests
        assert_tok("sizeof", Token::Type::eKeyword);
        assert_tok("goto", Token::Type::eKeyword, /*l=*/ false, /*std=*/ false, /*ctrl=*/ true);
        assert_tok("asm", Token::Type::eKeyword);
        assert_tok("a", Token::Type::eName);
        assert_tok("_a", Token::Type::eName);
        assert_tok("$a", Token::Type::eName);
        assert_tok("bool2", Token::Type::eName);
        assert_tok("0", Token::Type::eNumber);
        assert_tok("-0", Token::Type::eNumber);
        assert_tok("+0", Token::Type::eNumber);
        assert_tok("0xa", Token::Type::eNumber);
        assert_tok("010", Token::Type::eNumber);
        assert_tok("0b0", Token::Type::eNumber);
        assert_tok("0.0", Token::Type::eNumber);
        assert_tok("0x0.3p10", Token::Type::eNumber);
        assert_tok("0z", Token::Type::eNumber); // TODO: not a valid number
        assert_tok("0_km", Token::Type::eLiteral); // user literal
        assert_tok("=", Token::Type::eAssignmentOp);
        assert_tok("<<=", Token::Type::eAssignmentOp);
        assert_tok(">>=", Token::Type::eAssignmentOp);
        assert_tok("+=", Token::Type::eAssignmentOp);
        assert_tok("-=", Token::Type::eAssignmentOp);
        assert_tok("*=", Token::Type::eAssignmentOp);
        assert_tok("/=", Token::Type::eAssignmentOp);
        assert_tok("%=", Token::Type::eAssignmentOp);
        assert_tok("&=", Token::Type::eAssignmentOp);
        assert_tok("|=", Token::Type::eAssignmentOp);
        assert_tok("^=", Token::Type::eAssignmentOp);
        assert_tok(",", Token::Type::eExtendedOp);
        assert_tok("[", Token::Type::eExtendedOp);
        assert_tok("]", Token::Type::eExtendedOp);
        assert_tok("(", Token::Type::eExtendedOp);
        assert_tok(")", Token::Type::eExtendedOp);
        assert_tok("?", Token::Type::eExtendedOp);
        assert_tok(":", Token::Type::eExtendedOp);
        assert_tok("<<", Token::Type::eArithmeticalOp);
        assert_tok(">>", Token::Type::eArithmeticalOp);
        assert_tok("+", Token::Type::eArithmeticalOp);
        assert_tok("-", Token::Type::eArithmeticalOp);
        assert_tok("*", Token::Type::eArithmeticalOp);
        assert_tok("/", Token::Type::eArithmeticalOp);
        assert_tok("%", Token::Type::eArithmeticalOp);
        assert_tok("&", Token::Type::eBitOp);
        assert_tok("|", Token::Type::eBitOp);
        assert_tok("^", Token::Type::eBitOp);
        assert_tok("~", Token::Type::eBitOp);
        assert_tok("&&", Token::Type::eLogicalOp);
        assert_tok("||", Token::Type::eLogicalOp);
        assert_tok("!", Token::Type::eLogicalOp);
        assert_tok("==", Token::Type::eComparisonOp);
        assert_tok("!=", Token::Type::eComparisonOp);
        assert_tok("<", Token::Type::eComparisonOp);
        assert_tok("<=", Token::Type::eComparisonOp);
        assert_tok(">", Token::Type::eComparisonOp);
        assert_tok(">=", Token::Type::eComparisonOp);
        // eComparisonOp with link has a separate test
        assert_tok("<=>", Token::Type::eComparisonOp);
        assert_tok("++", Token::Type::eIncDecOp);
        assert_tok("--", Token::Type::eIncDecOp);
        assert_tok("{", Token::Type::eBracket);
        assert_tok("}", Token::Type::eBracket);
        // < and > with link have a separate test
        assert_tok("...", Token::Type::eEllipsis);
        assert_tok(";", Token::Type::eOther);
        // eType with specific languages and standards has separate tests
        assert_tok("void", Token::Type::eType, /*l=*/ false, /*std=*/ true);
    }

    void update_property_info_evariable() const
    {
        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, std::move(tokensFrontBack));
            tok.str("var1");
            tok.varId(17);
            assert_tok(&tok, Token::Type::eVariable);
        }
    }

    void update_property_info_ekeyword_c() const
    {
        {
            const Settings s = settingsBuilder().c(Standards::cstd_t::C89).build();
            TokenList list_c{s, Standards::Language::C};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_c, std::move(tokensFrontBack));
            tok.str("alignas"); // not a C89 keyword
            assert_tok(&tok, Token::Type::eName);
        }
        {
            TokenList list_c{settingsDefault, Standards::Language::C};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_c, std::move(tokensFrontBack));
            tok.str("alignas"); // a C23 keyword
            assert_tok(&tok, Token::Type::eKeyword);
        }
        {
            TokenList list_c{settingsDefault, Standards::Language::C};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_c, std::move(tokensFrontBack));
            tok.str("and_eq"); // a C++ keyword
            assert_tok(&tok, Token::Type::eName);
        }
    }

    void update_property_info_ekeyword_cpp() const
    {
        {
            const Settings s = settingsBuilder().cpp(Standards::cppstd_t::CPP03).build();
            TokenList list_cpp{s, Standards::Language::CPP};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_cpp, std::move(tokensFrontBack));
            tok.str("consteval"); // not a C++03 keyword
            assert_tok(&tok, Token::Type::eName);
        }
        {
            TokenList list_cpp{settingsDefault, Standards::Language::CPP};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_cpp, std::move(tokensFrontBack));
            tok.str("consteval"); // a C++20 keyword
            assert_tok(&tok, Token::Type::eKeyword);
        }
        {
            TokenList list_cpp{settingsDefault, Standards::Language::CPP};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_cpp, std::move(tokensFrontBack));
            tok.str("typeof_unqual"); // a C keyword
            assert_tok(&tok, Token::Type::eName);
        }
    }

    void update_property_info_ebracket_link() const
    {
        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, tokensFrontBack);
            tok.str("<");

            Token tok2(list, std::move(tokensFrontBack));
            tok.link(&tok2);
            assert_tok(&tok, Token::Type::eBracket);
        }

        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, tokensFrontBack);

            Token tok2(list, std::move(tokensFrontBack));
            tok.link(&tok2);

            tok.str("<");
            assert_tok(&tok, Token::Type::eBracket);
        }
    }

    void update_property_info_ecomparisonop_link() const
    {
        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, tokensFrontBack);
            tok.str("==");

            Token tok2(list, std::move(tokensFrontBack));
            tok.link(&tok2); // TODO: does not (and probably should not) update
            assert_tok(&tok, Token::Type::eComparisonOp);
        }

        {
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list, tokensFrontBack);

            Token tok2(list, std::move(tokensFrontBack));
            tok.link(&tok2);

            tok.str("==");
            assert_tok(&tok, Token::Type::eOther); // TODO: this looks wrong
        }
    }

    void update_property_info_etype_c() const
    {
        {
            TokenList list_c{settingsDefault, Standards::Language::C};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_c, std::move(tokensFrontBack));
            tok.str("char"); // not treated as keyword in TokenList::isKeyword()
            assert_tok(&tok, Token::Type::eType, /*l=*/ false, /*std=*/ true);
        }
        {
            TokenList list_c{settingsDefault, Standards::Language::C};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_c, std::move(tokensFrontBack));
            tok.str("size_t"); // not treated as keyword in TokenList::isKeyword()
            assert_tok(&tok, Token::Type::eType, /*l=*/ false, /*std=*/ true);
        }
    }

    void update_property_info_etype_cpp() const
    {
        {
            TokenList list_cpp{settingsDefault, Standards::Language::CPP};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_cpp, std::move(tokensFrontBack));
            tok.str("bool"); // not treated as keyword in TokenList::isKeyword()
            assert_tok(&tok, Token::Type::eType, /*l=*/ false, /*std=*/ true);
        }
        {
            TokenList list_cpp{settingsDefault, Standards::Language::CPP};
            auto tokensFrontBack = std::make_shared<TokensFrontBack>();
            Token tok(list_cpp, std::move(tokensFrontBack));
            tok.str("size_t");
            assert_tok(&tok, Token::Type::eType, /*l=*/ false, /*std=*/ true);
        }
    }

    void update_property_info_replace() const // #13743
    {
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list, std::move(tokensFrontBack));
        tok.str("size_t");
        assert_tok(&tok, Token::Type::eType, false, true);
        tok.str("long");
        assert_tok(&tok, Token::Type::eType, false, true);
    }

    void varid_reset() const
    {
        TokenList list_c{settingsDefault, Standards::Language::C};
        auto tokensFrontBack = std::make_shared<TokensFrontBack>();
        Token tok(list_c, std::move(tokensFrontBack));
        tok.str("int"); // not treated as keyword in TokenList::isKeyword()
        assert_tok(&tok, Token::Type::eType, /*l=*/ false, /*std=*/ true);
        tok.varId(0);
        assert_tok(&tok, Token::Type::eType, /*l=*/ false, /*std=*/ true);
    }
#undef assert_tok
};

REGISTER_TEST(TestToken)
