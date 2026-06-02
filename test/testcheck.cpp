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

#include "check.h"
#include "checks.h"
#include "fixture.h"

#include <list>
#include <string>

class TestCheck : public TestFixture {
public:
    TestCheck() : TestFixture("TestCheck") {}

private:
    void run() override {
        TEST_CASE(classInfoFormat);
    }

    void classInfoFormat() const {
        for (const Check * const c : CheckInstances::get()) {
            const std::string info = c->classInfo();
            if (!info.empty()) {
                ASSERT('\n' != info[0]);         // No \n in the beginning
                ASSERT('\n' == info.back());     // \n at end
                if (info.size() > 1)
                    ASSERT('\n' != info[info.length()-2]); // Only one \n at end
            }
        }
    }
};

REGISTER_TEST(TestCheck)
