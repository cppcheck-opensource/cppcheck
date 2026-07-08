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
#include "nonnullptr.h"

#include <cstddef>
#include <string>

class TestNonNullPtr : public TestFixture {
public:
    TestNonNullPtr() : TestFixture("TestNonNullPtr") {}

private:
    void run() override {
        TEST_CASE(getPointer);
        TEST_CASE(pointerConversion);
        TEST_CASE(referenceConversion);
        TEST_CASE(dereference);
        TEST_CASE(memberAccess);
        TEST_CASE(mutation);
        TEST_CASE(copyConstruct);
        TEST_CASE(copyAssign);
        TEST_CASE(rebind);
        TEST_CASE(convertToConst);
        TEST_CASE(functionArguments);
    }

    void getPointer() const {
        int x = 1;
        const NonNullPtr<int> p(x);
        ASSERT(p.get() == &x);
    }

    void pointerConversion() const {
        int x = 1;
        const NonNullPtr<int> p(x);
        int* raw = p;
        ASSERT(raw == &x);
        *raw = 2;
        ASSERT_EQUALS(2, x);
    }

    void referenceConversion() const {
        int x = 1;
        const NonNullPtr<int> p(x);
        int& ref = p;
        ASSERT(&ref == &x);
        ref = 2;
        ASSERT_EQUALS(2, x);
    }

    void dereference() const {
        int x = 42;
        const NonNullPtr<int> p(x);
        ASSERT_EQUALS(42, *p);
        ASSERT(&*p == &x);
    }

    void memberAccess() const {
        const std::string s = "abc";
        const NonNullPtr<const std::string> p(s);
        ASSERT_EQUALS(3, p->size());
        ASSERT_EQUALS("abc", *p);
    }

    void mutation() const {
        std::string s = "abc";
        const NonNullPtr<std::string> p(s);
        *p += "d";
        p->push_back('e');
        ASSERT_EQUALS("abcde", s);
    }

    void copyConstruct() const {
        int x = 1;
        const NonNullPtr<int> p(x);
        const NonNullPtr<int> q(p);
        ASSERT(q.get() == &x);
    }

    void copyAssign() const {
        int x = 1;
        int y = 2;
        NonNullPtr<int> p(x);
        const NonNullPtr<int> q(y);
        p = q;
        ASSERT(p.get() == &y);
        ASSERT_EQUALS(2, *p);
    }

    void rebind() const {
        int x = 1;
        int y = 2;
        NonNullPtr<int> p(x);
        // assignment rebinds, like std::reference_wrapper - it does not write through
        p = y;
        ASSERT(p.get() == &y);
        ASSERT_EQUALS(1, x);
    }

    void convertToConst() const {
        int x = 1;
        const NonNullPtr<int> p(x);
        const NonNullPtr<const int> cp(p);
        ASSERT(cp.get() == &x);
        const int* raw = cp;
        ASSERT(raw == &x);
    }

    static std::size_t refArg(const std::string& s) {
        return s.size();
    }

    static std::size_t ptrArg(const std::string* s) {
        return s->size();
    }

    void functionArguments() const {
        std::string s = "abcd";
        const NonNullPtr<std::string> p(s);
        // implicit conversion to reference, like std::reference_wrapper
        ASSERT_EQUALS(4, refArg(p));
        // implicit conversion to pointer, like gsl::not_null
        ASSERT_EQUALS(4, ptrArg(p));
    }
};

REGISTER_TEST(TestNonNullPtr)
