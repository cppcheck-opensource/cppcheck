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


#include "addoninfo.h"
#include "checkersreport.h"
#include "fixture.h"
#include "settings.h"

class TestCheckersReport : public TestFixture {
public:
    TestCheckersReport() : TestFixture("TestCheckersReport") {}

private:
    void run() final {
        // AddonInfo::checkers
        TEST_CASE(addonInfoCheckers);
    }

    void addonInfoCheckers() const {
        AddonInfo a;
        a.name = "test";
        a.checkers["abcdefghijklmnopqrstuvwxyz::abcdefghijklmnopqrstuvwxyz"] = "123";
        Settings s;
        s.addonInfos.emplace_back(a);
        const std::set<std::string> activeCheckers;
        CheckersReport r(s, activeCheckers);
        const std::string report = r.getReport("");
        const auto pos = report.rfind("\n\n");
        ASSERT(pos != std::string::npos);

        const char expected[] =
            "test checkers\n"
            "-------------\n"
            "No   abcdefghijklmnopqrstuvwxyz::abcdefghijklmnopqrstuvwxyz    require:123\n";

        ASSERT_EQUALS(expected, report.substr(pos+2));
    }
};

REGISTER_TEST(TestCheckersReport)
