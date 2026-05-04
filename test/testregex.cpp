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

#ifdef HAVE_RULES

#include "fixture.h"
#include "regex.h"

#include <list>
#include <memory>
#include <string>
#include <utility>

class TestRegExBase : public TestFixture {
public:
    TestRegExBase(const char * const name, Regex::Engine engine) : TestFixture(name), mEngine(engine) {}

private:
    Regex::Engine mEngine;

    void run() override {
        TEST_CASE(match);
        TEST_CASE(nomatch);
        TEST_CASE(compileError);
        TEST_CASE(copy);
        TEST_CASE(multimatch);
        TEST_CASE(partialmatch);
        TEST_CASE(exactmatch);
    }

#define assertRegex(...) assertRegex_(__FILE__, __LINE__, __VA_ARGS__)
    std::shared_ptr<Regex> assertRegex_(const char* file, int line, std::string pattern, const std::string& exp_err = "") const {
        std::string regex_err;
        auto r = Regex::create(std::move(pattern), mEngine, regex_err);
        if (exp_err.empty())
            ASSERT_LOC(!!r.get(), file, line);
        else
            ASSERT_LOC(!r.get(), file, line); // only not set if we encountered an error
        ASSERT_EQUALS_LOC(exp_err, regex_err, file, line);
        return r;
    }

    void match() const {
        const auto r = assertRegex("begin.*end");
        int called = 0;
        int s = -1;
        int e = -1;
        auto f = [&](int start, int end) {
            ++called;
            s = start;
            e = end;
        };
        ASSERT_EQUALS("", r->match("begin-123-end", std::move(f)));
        ASSERT_EQUALS(1, called);
        ASSERT_EQUALS(0, s);
        ASSERT_EQUALS(13, e);
    }

    void nomatch() const {
        const auto r = assertRegex("begin.*end");
        int called = 0;
        auto f = [&](int /*start*/, int /*end*/) {
            ++called;
        };
        ASSERT_EQUALS("", r->match("end-123-begin", std::move(f)));
        ASSERT_EQUALS(0, called);
    }

    void compileError() const {
        std::string exp;
        if (mEngine == Regex::Engine::Pcre)
            exp = "missing terminating ] for character class";
        else if (mEngine == Regex::Engine::Std)
        {
#if defined(_MSC_VER)
            exp = "regex_error(error_brack): The expression contained mismatched [ and ].";
#elif defined(_LIBCPP_VERSION)
            exp = "The expression contained mismatched [ and ].";
#elif defined(__clang__)
            exp = "Unexpected character within '[...]' in regular expression";
#else
            exp = "Unexpected character in bracket expression.";
#endif
        }

        (void)assertRegex("[", exp);
    }

    void copy() const {
        const auto r = assertRegex("begin.*end");

        int called = 0;
        int s = -1;
        int e = -1;
        auto f = [&](int start, int end) {
            ++called;
            s = start;
            e = end;
        };

        {
            // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
            auto r2 = r;
            ASSERT_EQUALS("", r2->match("begin-123-end", f));
            ASSERT_EQUALS(1, called);
            ASSERT_EQUALS(0, s);
            ASSERT_EQUALS(13, e);
        }

        called = 0;
        s = -1;
        e = -1;
        ASSERT_EQUALS("", r->match("begin-123-end", f));
        ASSERT_EQUALS(1, called);
        ASSERT_EQUALS(0, s);
        ASSERT_EQUALS(13, e);
    }

    void multimatch() const {
        const auto r = assertRegex("info:.*");

        std::string input =
            "info: start\n"
            "info: init\n"
            "warn: missing\n"
            "warn: invalid\n"
            "info: done\n"
            "error: notclean\n";

        std::list<std::string> matches;
        auto f = [&](int start, int end) {
            matches.push_back(input.substr(start, end - start));
        };
        ASSERT_EQUALS("", r->match(input, std::move(f)));
        ASSERT_EQUALS(3, matches.size());
        auto it = matches.cbegin();
        ASSERT_EQUALS("info: start", *it);
        ASSERT_EQUALS("info: init", *(++it));
        ASSERT_EQUALS("info: done", *(++it));
    }

    void partialmatch() const {
        const auto r = assertRegex("123");
        int called = 0;
        int s = -1;
        int e = -1;
        auto f = [&](int start, int end) {
            ++called;
            s = start;
            e = end;
        };
        ASSERT_EQUALS("", r->match("begin-123-end", std::move(f)));
        ASSERT_EQUALS(1, called);
        ASSERT_EQUALS(6, s);
        ASSERT_EQUALS(9, e);
    }

    void exactmatch() const {
        const auto r = assertRegex("^123$");

        int called = 0;
        int s = -1;
        int e = -1;
        auto f = [&](int start, int end) {
            ++called;
            s = start;
            e = end;
        };

        ASSERT_EQUALS("", r->match("begin-123-end", f));
        ASSERT_EQUALS(0, called);
        ASSERT_EQUALS(-1, s);
        ASSERT_EQUALS(-1, e);

        ASSERT_EQUALS("", r->match("123\n123", f));
        ASSERT_EQUALS(0, called);
        ASSERT_EQUALS(-1, s);
        ASSERT_EQUALS(-1, e);

        ASSERT_EQUALS("", r->match("123123", f));
        ASSERT_EQUALS(0, called);
        ASSERT_EQUALS(-1, s);
        ASSERT_EQUALS(-1, e);

        ASSERT_EQUALS("", r->match("123", f));
        ASSERT_EQUALS(1, called);
        ASSERT_EQUALS(0, s);
        ASSERT_EQUALS(3, e);
    }

    // TODO: how to provoke a match() error?

#undef assertRegex
};

class TestRegExPcre : public TestRegExBase {
public:
    TestRegExPcre() : TestRegExBase("TestRegExPcre", Regex::Engine::Pcre) {}
};

class TestRegExStd : public TestRegExBase {
public:
    TestRegExStd() : TestRegExBase("TestRegExStd", Regex::Engine::Std) {}
};

REGISTER_TEST(TestRegExPcre)
REGISTER_TEST(TestRegExStd)

#endif // HAVE_RULES
