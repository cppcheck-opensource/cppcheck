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
#include "redirect.h"
#include "timer.h"

#include <chrono>

class TestTimer : public TestFixture {
public:
    TestTimer() : TestFixture("TestTimer") {}

private:
    void run() override {
        TEST_CASE(result);
    }

    void result() {
        REDIRECT;

        TimerResults t1;
        t1.addResults("call1", std::chrono::milliseconds{1230});
        t1.addResults("call2", std::chrono::milliseconds{1234});
        t1.addResults("call1", std::chrono::milliseconds{1235});
        t1.addResults("call1", std::chrono::milliseconds{1239});

        const auto results = t1.getResults();
        ASSERT_EQUALS(2, results.size());

        auto it = results.find("call1");
        ASSERT(it != results.cend());
        ASSERT_EQUALS(3, it->second.size());
        ASSERT_EQUALS(1230, it->second[0].count());
        ASSERT_EQUALS(1235, it->second[1].count());
        ASSERT_EQUALS(1239, it->second[2].count());

        it = results.find("call2");
        ASSERT(it != results.cend());
        ASSERT_EQUALS(1, it->second.size());
        ASSERT_EQUALS(1234, it->second[0].count());

        t1.showResults();
        ASSERT_EQUALS("call1: 3.704s (avg. 1.23467s / min 1.23s / max 1.239s - 3 result(s))\n"
                      "call2: 1.234s (avg. 1.234s / min 1.234s / max 1.234s - 1 result(s))\n", GET_REDIRECT_OUTPUT);

        t1.showResults(1);
        ASSERT_EQUALS("call1: 3.704s (avg. 1.23467s / min 1.23s / max 1.239s - 3 result(s))\n", GET_REDIRECT_OUTPUT);

        t1.showResults(1, false);
        ASSERT_EQUALS("call1: 3.704s\n", GET_REDIRECT_OUTPUT);
    }
};

REGISTER_TEST(TestTimer)
