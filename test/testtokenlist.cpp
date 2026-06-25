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

#include "settings.h"
#include "fixture.h"
#include "helpers.h"
#include "path.h"
#include "platform.h"
#include "preprocessor.h"
#include "standards.h"
#include "token.h"
#include "tokenlist.h"

#include <stack>
#include <string>
#include <utility>
#include <vector>

#include <simplecpp.h>

class TestTokenList : public TestFixture {
public:
    TestTokenList() : TestFixture("TestTokenList")
    {}

private:
    void run() override {
        TEST_CASE(testaddtoken1);
        TEST_CASE(testaddtoken2);
        TEST_CASE(inc);
        TEST_CASE(isKeyword);
        TEST_CASE(notokens);
        TEST_CASE(ast1);
    }

    // inspired by #5895
    void testaddtoken1() const {
        const std::string code = "0x89504e470d0a1a0a";
        TokenList tokenlist(settingsDefault, Standards::Language::CPP);
        tokenlist.addtoken(code, 1, 1, false);
        ASSERT_EQUALS("0x89504e470d0a1a0a", tokenlist.front()->str());
    }

    void testaddtoken2() const {
        const std::string code = "0xF0000000";
        const Settings settings1 = dinit(Settings, $.platform.int_bit = 32);
        TokenList tokenlist(settings1, Standards::Language::CPP);
        tokenlist.addtoken(code, 1, 1, false);
        ASSERT_EQUALS("0xF0000000", tokenlist.front()->str());
    }

    void inc() const {
        const char code[] = "a++1;1++b;";

        const SimpleTokenList tokenlist(code);
        ASSERT(Token::simpleMatch(tokenlist.front(), "a + + 1 ; 1 + + b ;"));
    }

    void isKeyword() const {

        const char code[] = "for a int delete true";

        {
            const SimpleTokenList tokenlist(code, Standards::Language::C);

            ASSERT_EQUALS(true, tokenlist.front()->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->next()->isKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->next()->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(2)->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(2)->tokType() == Token::eType);
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(2)->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(3)->isKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(3)->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(4)->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(4)->isLiteral());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(4)->isControlFlowKeyword());
        }
        {
            const SimpleTokenList tokenlist(code);

            ASSERT_EQUALS(true, tokenlist.front()->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->next()->isKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->next()->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(2)->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(2)->tokType() == Token::eType);
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(2)->isControlFlowKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(3)->isKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(3)->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(4)->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(4)->isLiteral());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(4)->isControlFlowKeyword());
        }

        {
            const char code2[] = "_Generic"; // C11 keyword
            const SimpleTokenList tokenlist(code2); // default settings use latest standard
            ASSERT_EQUALS(false, tokenlist.front()->isKeyword());
        }

        {
            const char code2[] = "_Generic"; // C11 keyword
            const SimpleTokenList tokenlist(code2, Standards::Language::C); // default settings use latest standard
            ASSERT_EQUALS(true, tokenlist.front()->isKeyword());
        }

        {
            const char code2[] = "_Generic"; // C11 keyword
            const Settings s = settingsBuilder().c(Standards::C89).build();
            TokenList tokenlist(s, Standards::Language::C);
            tokenlist.appendFileIfNew("a.c");
            ASSERT(tokenlist.createTokensFromString(code2));
            ASSERT_EQUALS(false, tokenlist.front()->isKeyword());
        }

        {
            const char code2[] = "co_return"; // C++20 keyword
            const SimpleTokenList tokenlist(code2); // default settings use latest standard
            ASSERT_EQUALS(true, tokenlist.front()->isKeyword());
        }

        {
            const char code2[] = "co_return"; // C++20 keyword
            const SimpleTokenList tokenlist(code2, Standards::Language::C); // default settings use latest standard
            ASSERT_EQUALS(false, tokenlist.front()->isKeyword());
        }

        {
            const char code2[] = "noexcept"; // C++11 keyword
            const Settings s = settingsBuilder().cpp(Standards::CPP03).build();
            TokenList tokenlist(s, Standards::Language::CPP);
            tokenlist.appendFileIfNew("a.cpp");
            ASSERT(tokenlist.createTokensFromString(code2));
            ASSERT_EQUALS(false, tokenlist.front()->isKeyword());
        }
    }

    void notokens() {
        // analyzing /usr/include/poll.h caused Path::identify() to be called with an empty filename from
        // TokenList::determineCppC() because there are no tokens
        const char code[] = "#include <sys/poll.h>";
        std::vector<std::string> files;
        simplecpp::TokenList tokens1(code, files, "poll.h", nullptr);
        Preprocessor preprocessor(tokens1, settingsDefault, *this, Path::identify(tokens1.getFiles()[0], false));
        simplecpp::OutputList outputList_pp;
        simplecpp::TokenList tokensP = preprocessor.preprocess("", files, outputList_pp);
        ASSERT(!preprocessor.reportOutput(outputList_pp, true));
        TokenList tokenlist(settingsDefault, Standards::Language::C); // headers are treated as C files
        tokenlist.createTokens(std::move(tokensP)); // do not assert
    }

    void ast1() const {
        const char code[] = "('Release|x64' == 'Release|x64');";

        TokenList tokenlist(settingsDefault, Standards::Language::C);
        ASSERT(tokenlist.createTokensFromString(code));
        // TODO: put this logic in TokenList
        // generate links
        {
            std::stack<Token*> lpar;
            for (Token* tok2 = tokenlist.front(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    lpar.push(tok2);
                else if (tok2->str() == ")") {
                    if (lpar.empty())
                        break;
                    Token::createMutualLinks(lpar.top(), tok2);
                    lpar.pop();
                }
            }
        }
        tokenlist.createAst(); // do not crash
    }
};

REGISTER_TEST(TestTokenList)
