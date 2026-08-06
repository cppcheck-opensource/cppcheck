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
#include "refthunk.h"

#include <cstddef>
#include <string>

class TestRefThunk : public TestFixture {
public:
    TestRefThunk() : TestFixture("TestRefThunk") {}

private:
    void run() override {
        TEST_CASE(callAccess);
        TEST_CASE(getReference);
        TEST_CASE(referenceConversion);
        TEST_CASE(memberAccess);
        TEST_CASE(mutation);
        TEST_CASE(copyConstruct);
        TEST_CASE(copyAssign);
        TEST_CASE(rebind);
        TEST_CASE(convertToConst);
        TEST_CASE(functionArguments);
    }

    void callAccess() const {
        int x = 42;
        const RefThunk<int> p(x);
        ASSERT_EQUALS(42, p());
        ASSERT(&p() == &x);
    }

    void getReference() const {
        int x = 1;
        const RefThunk<int> p(x);
        ASSERT(&p.get() == &x);
        p.get() = 2;
        ASSERT_EQUALS(2, x);
    }

    void referenceConversion() const {
        int x = 1;
        const RefThunk<int> p(x);
        int& ref = p;
        ASSERT(&ref == &x);
        ref = 2;
        ASSERT_EQUALS(2, x);
    }

    void memberAccess() const {
        const std::string s = "abc";
        const RefThunk<const std::string> p(s);
        ASSERT_EQUALS(3, p().size());
        ASSERT_EQUALS("abc", p());
    }

    void mutation() const {
        std::string s = "abc";
        const RefThunk<std::string> p(s);
        p() += "d";
        p().push_back('e');
        ASSERT_EQUALS("abcde", s);
    }

    void copyConstruct() const {
        int x = 1;
        const RefThunk<int> p(x);
        const RefThunk<int> q(p);
        ASSERT(&q() == &x);
    }

    void copyAssign() const {
        int x = 1;
        int y = 2;
        RefThunk<int> p(x);
        const RefThunk<int> q(y);
        p = q;
        ASSERT(&p() == &y);
        ASSERT_EQUALS(2, p());
    }

    void rebind() const {
        int x = 1;
        int y = 2;
        RefThunk<int> p(x);
        // assignment rebinds, like std::reference_wrapper - it does not write through
        p = y;
        ASSERT(&p() == &y);
        ASSERT_EQUALS(1, x);
    }

    void convertToConst() const {
        int x = 1;
        const RefThunk<int> p(x);
        const RefThunk<const int> cp(p);
        ASSERT(&cp() == &x);
        const int& ref = cp;
        ASSERT(&ref == &x);
    }

    static std::size_t refArg(const std::string& s) {
        return s.size();
    }

    void functionArguments() const {
        std::string s = "abcd";
        const RefThunk<std::string> p(s);
        // implicit conversion to reference, like std::reference_wrapper
        ASSERT_EQUALS(4, refArg(p));
        // explicit access through operator()
        ASSERT_EQUALS(4, refArg(p()));
    }
};

REGISTER_TEST(TestRefThunk)
