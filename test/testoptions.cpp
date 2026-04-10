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
#include "options.h"
#include "utils.h"

#include <set>
#include <string>


class TestOptions : public TestFixture {
public:
    TestOptions()
        : TestFixture("TestOptions") {}


private:
    void run() override {
        TEST_CASE(which_test);
        TEST_CASE(which_test_method);
        TEST_CASE(no_test_method);
        TEST_CASE(defaults);
        TEST_CASE(quiet);
        TEST_CASE(help);
        TEST_CASE(help_long);
        TEST_CASE(multiple_testcases);
        TEST_CASE(multiple_testcases_ignore_duplicates);
        TEST_CASE(invalid_switches);
        TEST_CASE(summary);
        TEST_CASE(dry_run);
        TEST_CASE(exclude_tests);
        TEST_CASE(timer_results);
    }


    void which_test() const {
        const char* argv[] = {"./test_runner", "TestClass"};
        options args(getArrayLength(argv), argv);
        const std::map<std::string, std::set<std::string>> expected{
            { "TestClass", {} }
        };
        ASSERT(expected == args.which_tests());
        ASSERT(args.errors().empty());
    }


    void which_test_method() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod"};
        options args(getArrayLength(argv), argv);
        const std::map<std::string, std::set<std::string>> expected{
            { "TestClass", {"TestMethod"} }
        };
        ASSERT(expected == args.which_tests());
        ASSERT(args.errors().empty());
    }


    void no_test_method() const {
        const char* argv[] = {"./test_runner"};
        options args(getArrayLength(argv), argv);
        const std::map<std::string, std::set<std::string>> expected{};
        ASSERT(expected == args.which_tests());
        ASSERT(args.errors().empty());
    }


    void defaults() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod"};
        options args(getArrayLength(argv), argv);
        ASSERT_EQUALS(false, args.quiet());
        ASSERT_EQUALS(false, args.help());
        ASSERT_EQUALS(true, args.summary());
        ASSERT_EQUALS(false, args.dry_run());
        ASSERT_EQUALS(false, args.exclude_tests());
        ASSERT(args.errors().empty());
    }

    void quiet() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-q"};
        options args(getArrayLength(argv), argv);
        ASSERT_EQUALS(true, args.quiet());
        ASSERT(args.errors().empty());
    }

    void help() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-h"};
        options args(getArrayLength(argv), argv);
        ASSERT_EQUALS(true, args.help());
        ASSERT(args.errors().empty());
    }


    void help_long() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "--help"};
        options args(getArrayLength(argv), argv);
        ASSERT_EQUALS(true, args.help());
        ASSERT(args.errors().empty());
    }

    void multiple_testcases() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "TestClass::AnotherTestMethod"};
        options args(getArrayLength(argv), argv);
        const std::map<std::string, std::set<std::string>> expected{
            { "TestClass", { "TestMethod", "AnotherTestMethod" } }
        };
        ASSERT(expected == args.which_tests());
        ASSERT(args.errors().empty());
    }

    void multiple_testcases_ignore_duplicates() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "TestClass"};
        options args(getArrayLength(argv), argv);
        const std::map<std::string, std::set<std::string>> expected{
            { "TestClass", {} }
        };
        ASSERT(expected == args.which_tests());
        ASSERT(args.errors().empty());
    }

    void invalid_switches() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-a", "-v", "-q"};
        options args(getArrayLength(argv), argv);
        const std::map<std::string, std::set<std::string>> expected {
            { "TestClass", { "TestMethod" } }
        };
        ASSERT(expected == args.which_tests());
        ASSERT_EQUALS(true, args.quiet());
        ASSERT_EQUALS(2, args.errors().size());
        auto it = args.errors().cbegin();
        ASSERT_EQUALS("unknown option '-a'", *it);
        ++it;
        ASSERT_EQUALS("unknown option '-v'", *it);
    }

    void summary() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-n"};
        options args(getArrayLength(argv), argv);
        ASSERT_EQUALS(false, args.summary());
        ASSERT(args.errors().empty());
    }

    void dry_run() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-d"};
        options args(getArrayLength(argv), argv);
        ASSERT_EQUALS(true, args.dry_run());
        ASSERT(args.errors().empty());
    }

    void exclude_tests() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-x"};
        options args(getArrayLength(argv), argv);
        ASSERT_EQUALS(true, args.exclude_tests());
        ASSERT(args.errors().empty());
    }

    void timer_results() const {
        const char* argv[] = {"./test_runner", "TestClass::TestMethod", "-t"};
        options args(getArrayLength(argv), argv);
        ASSERT(!!args.timer_results());
    }
};

REGISTER_TEST(TestOptions)
