/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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
#include "token.h"
#include "tokenrange.h"
#include "symboldatabase.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <sstream>
#include <string>


class TestTokenRange : public TestFixture {
public:
    TestTokenRange() : TestFixture("TestTokenRange") {}

private:
    void run() override {
        TEST_CASE(enumerationToEnd);
        TEST_CASE(untilHelperToEnd);
        TEST_CASE(untilHelperPartWay);
        TEST_CASE(partialEnumeration);
        TEST_CASE(scopeExample);
        TEST_CASE(exampleAlgorithms);
    }

    static std::string testTokenRange(ConstTokenRange range, const Token* start, const Token* end) {
        auto tokenToString = [](const Token* t) {
            return t ? t->str() : "<null>";
        };
        int index = 0;
        const Token* expected = start;
        for (const Token* t : range) {
            if (expected != t) {
                std::ostringstream message;
                message << "Failed to match token " << tokenToString(expected) << " at position " << index << ". Got " << tokenToString(t) << " instead";
                return message.str();
            }
            index++;
            expected = expected->next();
        }
        if (expected != end) {
            std::ostringstream message;
            message << "Failed to terminate on " << tokenToString(end) << ". Instead terminated on " << tokenToString(expected) << " at position " << index << ".";
            return message.str();
        }
        return "";
    }

    void enumerationToEnd() const {
        const char code[] = "void a(){} void main(){ if(true){a();} }";
        const SimpleTokenList tokenList(code);
        ASSERT_EQUALS("", testTokenRange(ConstTokenRange{ tokenList.front(), nullptr }, tokenList.front(), nullptr));
    }

    void untilHelperToEnd() const {
        const char code[] = "void a(){} void main(){ if(true){a();} }";
        const SimpleTokenList tokenList(code);
        ASSERT_EQUALS("", testTokenRange(tokenList.front()->until(nullptr), tokenList.front(), nullptr));
    }

    void untilHelperPartWay() const {
        const char code[] = "void a(){} void main(){ if(true){a();} }";
        const SimpleTokenList tokenList(code);
        const Token* start = tokenList.front()->tokAt(4);
        const Token* end = start->tokAt(8);
        ASSERT_EQUALS("", testTokenRange(start->until(end), start, end));
    }

    void partialEnumeration() const {
        const char code[] = "void a(){} void main(){ if(true){a();} }";
        const SimpleTokenList tokenList(code);
        const Token* start = tokenList.front()->tokAt(4);
        const Token* end = tokenList.front()->tokAt(10);
        ASSERT_EQUALS("", testTokenRange(ConstTokenRange{ start, end }, start, end));
    }

    void scopeExample() {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        const char code[] = "void a(){} void main(){ if(true){a();} }";
        ASSERT(tokenizer.tokenize(code));

        const SymbolDatabase* sd = tokenizer.getSymbolDatabase();
        const Scope& scope = *std::next(sd->scopeList.cbegin(), 3); //The scope of the if block

        std::string contents;
        for (const Token* t : ConstTokenRange{ scope.bodyStart->next(), scope.bodyEnd }) {
            contents += t->str();
        }
        ASSERT_EQUALS("a();", contents);
    }

    void exampleAlgorithms() const {
        const char code[] = "void a(){} void main(){ if(true){a();} }";
        const SimpleTokenList tokenList(code);
        ConstTokenRange range{ tokenList.front(), nullptr };
        ASSERT_EQUALS(true, std::all_of(range.begin(), range.end(), [](const Token*) {
            return true;
        }));
        ASSERT_EQUALS(true, std::any_of(range.begin(), range.end(), [](const Token* t) {
            return t->str() == "true";
        }));
        ASSERT_EQUALS("true", (*std::find_if(range.begin(), range.end(), [](const Token* t) {
            return t->str() == "true";
        }))->str());
        ASSERT_EQUALS(3, std::count_if(range.begin(), range.end(), [](const Token* t) {
            return t->str() == "{";
        }));
    }
};

REGISTER_TEST(TestTokenRange)
