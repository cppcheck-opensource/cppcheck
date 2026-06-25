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

#include "fixture.h"
#include "helpers.h"
#include "summaries.h"

#include <cstddef>
#include <string>

class TestSummaries : public TestFixture {
public:
    TestSummaries() : TestFixture("TestSummaries") {}

private:
    void run() override {

        TEST_CASE(createSummaries1);
        TEST_CASE(createSummariesGlobal);
        TEST_CASE(createSummariesNoreturn);
    }

#define createSummaries(...) createSummaries_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string createSummaries_(const char* file, int line, const char (&code)[size]) {
        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        return Summaries::create(tokenizer, "", 0);
    }

    void createSummaries1() {
        ASSERT_EQUALS("foo\n", createSummaries("void foo() {}"));
    }

    void createSummariesGlobal() {
        ASSERT_EQUALS("foo global:[x]\n", createSummaries("int x; void foo() { x=0; }"));
    }

    void createSummariesNoreturn() {
        ASSERT_EQUALS("foo call:[bar] noreturn:[bar]\n", createSummaries("void foo() { bar(); }"));
    }
};

REGISTER_TEST(TestSummaries)
