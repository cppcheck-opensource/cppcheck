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
#include "utils.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

class TestUtils : public TestFixture {
public:
    TestUtils() : TestFixture("TestUtils") {}

private:
    void run() override {
        TEST_CASE(isValidGlobPattern);
        TEST_CASE(matchglob);
        TEST_CASE(isStringLiteral);
        TEST_CASE(isCharLiteral);
        TEST_CASE(strToInt);
        TEST_CASE(id_string);
        TEST_CASE(startsWith);
        TEST_CASE(trim);
        TEST_CASE(findAndReplace);
        TEST_CASE(replaceEscapeSequences);
        TEST_CASE(splitString);
        TEST_CASE(as_const);
        TEST_CASE(memoize);
        TEST_CASE(endsWith);
    }

    void isValidGlobPattern() const {
        ASSERT_EQUALS(true, ::isValidGlobPattern("*"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("*x"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x*"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("*/x/*"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x/*/z"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("**"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("**x"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x**"));

        ASSERT_EQUALS(true, ::isValidGlobPattern("?"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("?x"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x?"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("?/x/?"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x/?/z"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("??"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("????"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("??x"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x??"));

        ASSERT_EQUALS(true, ::isValidGlobPattern("?*"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("?**"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("?***"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("???*"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("???**"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("???***"));

        ASSERT_EQUALS(false, ::isValidGlobPattern("*?"));
        ASSERT_EQUALS(true,  ::isValidGlobPattern("*x?"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("**?"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("***"));
        ASSERT_EQUALS(true,  ::isValidGlobPattern("**x*"));
    }

    void matchglob() const {
        ASSERT_EQUALS(true, ::matchglob("*", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("x*", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("*z", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("*y*", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("*y*", "yz"));
        ASSERT_EQUALS(false, ::matchglob("*y*", "abc"));
        ASSERT_EQUALS(true, ::matchglob("*", "x/y/z"));
        ASSERT_EQUALS(true, ::matchglob("*/y/z", "x/y/z"));

        ASSERT_EQUALS(true, ::matchglob("**", ""));
        ASSERT_EQUALS(true, ::matchglob("**", "abcdefg"));
        ASSERT_EQUALS(true, ::matchglob("**", "abcde/foo/bar"));
        ASSERT_EQUALS(false, ::matchglob("*/**", "abcde"));
        ASSERT_EQUALS(true, ::matchglob("*/**", "abcde/foo"));
        ASSERT_EQUALS(true, ::matchglob("*/**", "abcde/foo/bar"));

        ASSERT_EQUALS(false, ::matchglob("?", "xyz"));
        ASSERT_EQUALS(false, ::matchglob("x?", "xyz"));
        ASSERT_EQUALS(false, ::matchglob("?z", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("?y?", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("?/?/?", "x/y/z"));

        ASSERT_EQUALS(true, ::matchglob("??", "xy"));
        ASSERT_EQUALS(false, ::matchglob("??", "x"));
        ASSERT_EQUALS(false, ::matchglob("??", "xyz"));

        ASSERT_EQUALS(true, ::matchglob("????", "wxyz"));
        ASSERT_EQUALS(false, ::matchglob("????", "xyz"));
        ASSERT_EQUALS(false, ::matchglob("????", "vwxyz"));
    }

    void isStringLiteral() const {
        // empty
        ASSERT_EQUALS(false, ::isStringLiteral(""));

        // no literals
        ASSERT_EQUALS(false, ::isStringLiteral("u8"));
        ASSERT_EQUALS(false, ::isStringLiteral("u"));
        ASSERT_EQUALS(false, ::isStringLiteral("U"));
        ASSERT_EQUALS(false, ::isStringLiteral("L"));

        // incomplete string literals
        ASSERT_EQUALS(false, ::isStringLiteral("\""));
        ASSERT_EQUALS(false, ::isStringLiteral("u8\""));
        ASSERT_EQUALS(false, ::isStringLiteral("u\""));
        ASSERT_EQUALS(false, ::isStringLiteral("U\""));
        ASSERT_EQUALS(false, ::isStringLiteral("L\""));

        // valid string literals
        ASSERT_EQUALS(true, ::isStringLiteral("\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u8\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("U\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("L\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u8\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("U\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("L\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("\"test\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u8\"test\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u\"test\""));
        ASSERT_EQUALS(true, ::isStringLiteral("U\"test\""));
        ASSERT_EQUALS(true, ::isStringLiteral("L\"test\""));

        // incomplete char literals
        ASSERT_EQUALS(false, ::isStringLiteral("'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u8'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u'"));
        ASSERT_EQUALS(false, ::isStringLiteral("U'"));
        ASSERT_EQUALS(false, ::isStringLiteral("L'"));

        // valid char literals
        ASSERT_EQUALS(false, ::isStringLiteral("'t'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u8't'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u't'"));
        ASSERT_EQUALS(false, ::isStringLiteral("U't'"));
        ASSERT_EQUALS(false, ::isStringLiteral("L't'"));
        ASSERT_EQUALS(false, ::isStringLiteral("'test'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u8'test'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u'test'"));
        ASSERT_EQUALS(false, ::isStringLiteral("U'test'"));
        ASSERT_EQUALS(false, ::isStringLiteral("L'test'"));
    }

    void isCharLiteral() const {
        // empty
        ASSERT_EQUALS(false, ::isCharLiteral(""));

        // no literals
        ASSERT_EQUALS(false, ::isCharLiteral("u8"));
        ASSERT_EQUALS(false, ::isCharLiteral("u"));
        ASSERT_EQUALS(false, ::isCharLiteral("U"));
        ASSERT_EQUALS(false, ::isCharLiteral("L"));

        // incomplete string literals
        ASSERT_EQUALS(false, ::isCharLiteral("\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u8\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u\""));
        ASSERT_EQUALS(false, ::isCharLiteral("U\""));
        ASSERT_EQUALS(false, ::isCharLiteral("L\""));

        // valid string literals
        ASSERT_EQUALS(false, ::isCharLiteral("\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u8\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("U\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("L\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u8\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("U\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("L\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("\"test\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u8\"test\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u\"test\""));
        ASSERT_EQUALS(false, ::isCharLiteral("U\"test\""));
        ASSERT_EQUALS(false, ::isCharLiteral("L\"test\""));

        // incomplete char literals
        ASSERT_EQUALS(false, ::isCharLiteral("'"));
        ASSERT_EQUALS(false, ::isCharLiteral("u8'"));
        ASSERT_EQUALS(false, ::isCharLiteral("u'"));
        ASSERT_EQUALS(false, ::isCharLiteral("U'"));
        ASSERT_EQUALS(false, ::isCharLiteral("L'"));

        // valid char literals
        ASSERT_EQUALS(true, ::isCharLiteral("'t'"));
        ASSERT_EQUALS(true, ::isCharLiteral("u8't'"));
        ASSERT_EQUALS(true, ::isCharLiteral("u't'"));
        ASSERT_EQUALS(true, ::isCharLiteral("U't'"));
        ASSERT_EQUALS(true, ::isCharLiteral("L't'"));
        ASSERT_EQUALS(true, ::isCharLiteral("'test'"));
        ASSERT_EQUALS(true, ::isCharLiteral("u8'test'"));
        ASSERT_EQUALS(true, ::isCharLiteral("u'test'"));
        ASSERT_EQUALS(true, ::isCharLiteral("U'test'"));
        ASSERT_EQUALS(true, ::isCharLiteral("L'test'"));
    }

    void strToInt() {
        ASSERT_EQUALS(1, ::strToInt<int>("1"));
        ASSERT_EQUALS(-1, ::strToInt<int>("-1"));
        ASSERT_EQUALS(1, ::strToInt<std::size_t>("1"));
        ASSERT_THROW_EQUALS(::strToInt<int>(""), std::runtime_error, "converting '' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<std::size_t>(""), std::runtime_error, "converting '' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<int>(" "), std::runtime_error, "converting ' ' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<std::size_t>(" "), std::runtime_error, "converting ' ' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<unsigned int>("-1"), std::runtime_error, "converting '-1' to integer failed - needs to be positive");
        ASSERT_THROW_EQUALS(::strToInt<int>("1ms"), std::runtime_error, "converting '1ms' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<int>("1.0"), std::runtime_error, "converting '1.0' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<int>("one"), std::runtime_error, "converting 'one' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<unsigned int>("1ms"), std::runtime_error, "converting '1ms' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<unsigned int>("1.0"), std::runtime_error, "converting '1.0' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<unsigned int>("one"), std::runtime_error, "converting 'one' to integer failed - not an integer");
        ASSERT_THROW_EQUALS(::strToInt<int>(std::to_string(static_cast<int64_t>(std::numeric_limits<int>::max()) + 1)), std::runtime_error, "converting '2147483648' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS(::strToInt<int>(std::to_string(static_cast<int64_t>(std::numeric_limits<int>::min()) - 1)), std::runtime_error, "converting '-2147483649' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS(::strToInt<int8_t>(std::to_string(static_cast<int64_t>(std::numeric_limits<int8_t>::max()) + 1)), std::runtime_error, "converting '128' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS(::strToInt<int8_t>(std::to_string(static_cast<int64_t>(std::numeric_limits<int8_t>::min()) - 1)), std::runtime_error, "converting '-129' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS(::strToInt<unsigned int>(std::to_string(static_cast<uint64_t>(std::numeric_limits<unsigned int>::max()) + 1)), std::runtime_error, "converting '4294967296' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS(::strToInt<int>("9223372036854775808"), std::runtime_error, "converting '9223372036854775808' to integer failed - out of range (stoll)"); // LLONG_MAX + 1
        ASSERT_THROW_EQUALS(::strToInt<std::size_t>("18446744073709551616"), std::runtime_error, "converting '18446744073709551616' to integer failed - out of range (stoull)"); // ULLONG_MAX + 1

        {
            long tmp;
            ASSERT(::strToInt("1", tmp));
            ASSERT_EQUALS(1, tmp);
        }
        {
            long tmp;
            ASSERT(::strToInt("-1", tmp));
            ASSERT_EQUALS(-1, tmp);
        }
        {
            std::size_t tmp;
            ASSERT(::strToInt("1", tmp));
        }
        {
            std::size_t tmp;
            ASSERT(!::strToInt("-1", tmp));
        }
        {
            long tmp;
            ASSERT(!::strToInt("1ms", tmp));
        }
        {
            unsigned long tmp;
            ASSERT(!::strToInt("1ms", tmp));
        }
        {
            long tmp;
            ASSERT(!::strToInt("", tmp));
        }
        {
            unsigned long tmp;
            ASSERT(!::strToInt("", tmp));
        }
        {
            long tmp;
            ASSERT(!::strToInt(" ", tmp));
        }
        {
            unsigned long tmp;
            ASSERT(!::strToInt(" ", tmp));
        }

        {
            long tmp;
            std::string err;
            ASSERT(::strToInt("1", tmp, &err));
            ASSERT_EQUALS(1, tmp);
            ASSERT_EQUALS("", err);
        }
        {
            std::size_t tmp;
            std::string err;
            ASSERT(::strToInt("1", tmp, &err));
            ASSERT_EQUALS(1, tmp);
            ASSERT_EQUALS("", err);
        }
        {
            unsigned long tmp;
            std::string err;
            ASSERT(!::strToInt("-1", tmp, &err));
            ASSERT_EQUALS("needs to be positive", err);
        }
        {
            long tmp;
            std::string err;
            ASSERT(!::strToInt("1ms", tmp, &err));
            ASSERT_EQUALS("not an integer", err);
        }
        {
            long tmp;
            std::string err;
            ASSERT(!::strToInt("1.0", tmp, &err));
            ASSERT_EQUALS("not an integer", err);
        }
        {
            long tmp;
            std::string err;
            ASSERT(!::strToInt("one", tmp, &err));
            ASSERT_EQUALS("not an integer", err);
        }
        {
            std::size_t tmp;
            std::string err;
            ASSERT(!::strToInt("1ms", tmp, &err));
            ASSERT_EQUALS("not an integer", err);
        }
        {
            long tmp;
            std::string err;
            ASSERT(!::strToInt("9223372036854775808", tmp, &err)); // LLONG_MAX + 1
            ASSERT_EQUALS("out of range (stoll)", err);
        }
        {
            int tmp;
            std::string err;
            ASSERT(!::strToInt(std::to_string(static_cast<int64_t>(std::numeric_limits<int>::max()) + 1), tmp, &err));
            ASSERT_EQUALS("out of range (limits)", err);
        }
        {
            int tmp;
            std::string err;
            ASSERT(!::strToInt(std::to_string(static_cast<int64_t>(std::numeric_limits<int>::min()) - 1), tmp, &err));
            ASSERT_EQUALS("out of range (limits)", err);
        }
        {
            int8_t tmp;
            std::string err;
            ASSERT(!::strToInt(std::to_string(static_cast<int64_t>(std::numeric_limits<int8_t>::max()) + 1), tmp, &err));
            ASSERT_EQUALS("out of range (limits)", err);
        }
        {
            int8_t tmp;
            std::string err;
            ASSERT(!::strToInt(std::to_string(static_cast<int64_t>(std::numeric_limits<int8_t>::min()) - 1), tmp, &err));
            ASSERT_EQUALS("out of range (limits)", err);
        }
        {
            std::size_t tmp;
            std::string err;
            ASSERT(!::strToInt("18446744073709551616", tmp, &err)); // ULLONG_MAX + 1
            ASSERT_EQUALS("out of range (stoull)", err);
        }
    }

    void id_string() const
    {
        ASSERT_EQUALS("0", id_string_i(0));
        ASSERT_EQUALS("f1", id_string_i(0xF1));
        ASSERT_EQUALS("123", id_string_i(0x123));
        ASSERT_EQUALS("1230", id_string_i(0x1230));
        ASSERT_EQUALS(std::string(8,'f'), id_string_i(0xffffffffU));
        if (sizeof(void*) == 8) {
            ASSERT_EQUALS(std::string(16,'f'), id_string_i(~0ULL));
        }
    }

    void startsWith() const
    {
        ASSERT(::startsWith("test", "test"));
        ASSERT(::startsWith("test2", "test"));
        ASSERT(::startsWith("test test", "test"));
        ASSERT(::startsWith("test", "t"));
        ASSERT(!::startsWith("2test", "test"));
        ASSERT(!::startsWith("", "test"));
        ASSERT(!::startsWith("tes", "test"));
        ASSERT(!::startsWith("2test", "t"));
        ASSERT(!::startsWith("t", "test"));
    }

    void trim() const
    {
        ASSERT_EQUALS("test", ::trim("test"));
        ASSERT_EQUALS("test", ::trim(" test"));
        ASSERT_EQUALS("test", ::trim("test "));
        ASSERT_EQUALS("test", ::trim(" test "));
        ASSERT_EQUALS("test", ::trim("  test"));
        ASSERT_EQUALS("test", ::trim("test  "));
        ASSERT_EQUALS("test", ::trim("  test  "));
        ASSERT_EQUALS("test", ::trim("\ttest"));
        ASSERT_EQUALS("test", ::trim("test\t"));
        ASSERT_EQUALS("test", ::trim("\ttest\t"));
        ASSERT_EQUALS("test", ::trim("\t\ttest"));
        ASSERT_EQUALS("test", ::trim("test\t\t"));
        ASSERT_EQUALS("test", ::trim("\t\ttest\t\t"));
        ASSERT_EQUALS("test", ::trim(" \ttest"));
        ASSERT_EQUALS("test", ::trim("test\t "));
        ASSERT_EQUALS("test", ::trim(" \ttest\t"));
        ASSERT_EQUALS("test", ::trim("\t \ttest"));
        ASSERT_EQUALS("test", ::trim("test\t \t"));
        ASSERT_EQUALS("test", ::trim("\t \ttest\t \t"));

        ASSERT_EQUALS("test test", ::trim("test test"));
        ASSERT_EQUALS("test test", ::trim(" test test"));
        ASSERT_EQUALS("test test", ::trim("test test "));
        ASSERT_EQUALS("test test", ::trim(" test test "));
        ASSERT_EQUALS("test\ttest", ::trim(" test\ttest"));
        ASSERT_EQUALS("test\ttest", ::trim("test\ttest "));
        ASSERT_EQUALS("test\ttest", ::trim(" test\ttest "));

        ASSERT_EQUALS("test", ::trim("\ntest", "\n"));
        ASSERT_EQUALS("test", ::trim("test\n", "\n"));
        ASSERT_EQUALS("test", ::trim("\ntest\n", "\n"));
    }

    void findAndReplace() const {
        {
            std::string s{"test"};
            ::findAndReplace(s, "test", "tset");
            ASSERT_EQUALS("tset", s);
        }
        {
            std::string s{"testtest"};
            ::findAndReplace(s, "test", "tset");
            ASSERT_EQUALS("tsettset", s);
        }
        {
            std::string s{"1test1test1"};
            ::findAndReplace(s, "test", "tset");
            ASSERT_EQUALS("1tset1tset1", s);
        }
        {
            std::string s{"1test1test1"};
            ::findAndReplace(s, "test", "");
            ASSERT_EQUALS("111", s);
        }
        {
            std::string s{"111"};
            ::findAndReplace(s, "test", "tset");
            ASSERT_EQUALS("111", s);
        }
        {
            std::string s;
            ::findAndReplace(s, "test", "tset");
            ASSERT_EQUALS("", s);
        }
    }

    void replaceEscapeSequences() const {
        ASSERT_EQUALS("\x30", ::replaceEscapeSequences("\\x30"));
        ASSERT_EQUALS("\030", ::replaceEscapeSequences("\\030"));
        ASSERT_EQUALS("\r", ::replaceEscapeSequences("\\r"));
        ASSERT_EQUALS("\n", ::replaceEscapeSequences("\\n"));
        ASSERT_EQUALS("\t", ::replaceEscapeSequences("\\t"));
        ASSERT_EQUALS("\\", ::replaceEscapeSequences("\\\\"));
        ASSERT_EQUALS("\"", ::replaceEscapeSequences("\\\""));
    }

    void splitString() const {
        {
            const auto l = ::splitString("test", ',');
            ASSERT_EQUALS(1, l.size());
            ASSERT_EQUALS("test", *l.cbegin());
        }
        {
            const auto l = ::splitString("test,test", ';');
            ASSERT_EQUALS(1, l.size());
            ASSERT_EQUALS("test,test", *l.cbegin());
        }
        {
            const auto l = ::splitString("test,test", ',');
            ASSERT_EQUALS(2, l.size());
            auto it = l.cbegin();
            ASSERT_EQUALS("test", *it++);
            ASSERT_EQUALS("test", *it);
        }
        {
            const auto l = ::splitString("test,test,", ',');
            ASSERT_EQUALS(3, l.size());
            auto it = l.cbegin();
            ASSERT_EQUALS("test", *it++);
            ASSERT_EQUALS("test", *it++);
            ASSERT_EQUALS("", *it);
        }
        {
            const auto l = ::splitString("test,,test", ',');
            ASSERT_EQUALS(3, l.size());
            auto it = l.cbegin();
            ASSERT_EQUALS("test", *it++);
            ASSERT_EQUALS("", *it++);
            ASSERT_EQUALS("test", *it);
        }
        {
            const auto l = ::splitString(",test,test", ',');
            ASSERT_EQUALS(3, l.size());
            auto it = l.cbegin();
            ASSERT_EQUALS("", *it++);
            ASSERT_EQUALS("test", *it++);
            ASSERT_EQUALS("test", *it);
        }
    }

    void as_const() const {
        struct C {
            bool written = false;
            void f() {
                written = true;
            }
            void f() const {}
        };

        {
            C c;
            c.f();
            ASSERT(c.written);
        }
        {
            C c;
            utils::as_const(c).f();
            ASSERT(!c.written);
        }
        {
            C c;
            C* cp = &c;
            cp->f();
            ASSERT(c.written);
        }
        {
            int i = 0;
            auto cr = utils::as_const(i); // TODO: does nothing and should not be allowed
            (void)cr;
        }
    }

    void memoize() const {
        int count = 0;
        auto f = [&count]() {
            ++count;
            return count;
        };
        const auto callF = utils::memoize([&]() {
            return f();
        });
        ASSERT_EQUALS(0, count);
        ASSERT_EQUALS(1, callF());
        ASSERT_EQUALS(1, count);
        ASSERT_EQUALS(1, callF());
        ASSERT_EQUALS(1, count);
    }

    void endsWith() const
    {
        ASSERT(::endsWith("test", "test"));
        ASSERT(::endsWith("test2", "2"));
        ASSERT(::endsWith("test test", "test"));
        ASSERT(::endsWith("test", "t"));
        ASSERT(!::endsWith("", "test"));
        ASSERT(!::endsWith("tes", "test"));
        ASSERT(!::endsWith("2test", "2"));
    }
};

REGISTER_TEST(TestUtils)
