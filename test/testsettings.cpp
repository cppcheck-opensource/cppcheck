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


#include "errortypes.h"
#include "settings.h"
#include "fixture.h"
#include "helpers.h"
#include "suppressions.h"

class TestSettings : public TestFixture {
public:
    TestSettings() : TestFixture("TestSettings") {}

private:
    void run() override {
        TEST_CASE(simpleEnableGroup);
        TEST_CASE(loadCppcheckCfg);
        TEST_CASE(loadCppcheckCfgSafety);
        TEST_CASE(getNameAndVersion);
        TEST_CASE(checkLevelDefault);

        TEST_CASE(getMaxConfigsDefault);
        TEST_CASE(getMaxConfigsOpt);
        TEST_CASE(getMaxConfigsForce);
        TEST_CASE(getMaxConfigsOptAndForce);
        TEST_CASE(getMaxConfigsDefines);
        TEST_CASE(getMaxConfigsDefinesAndOpt);
        TEST_CASE(getMaxConfigsOptAndProject);
    }

    void simpleEnableGroup() const {
        SimpleEnableGroup<Checks> group;

        ASSERT_EQUALS(0, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        group.fill();

        ASSERT_EQUALS(4294967295, group.intValue());
        ASSERT_EQUALS(true, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(true, group.isEnabled(Checks::internalCheck));

        group.clear();

        ASSERT_EQUALS(0, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        group.enable(Checks::unusedFunction);
        group.setEnabled(Checks::missingInclude, true);

        ASSERT_EQUALS(3, group.intValue());
        ASSERT_EQUALS(true, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        group.disable(Checks::unusedFunction);
        group.setEnabled(Checks::missingInclude, false);

        ASSERT_EQUALS(0, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        SimpleEnableGroup<Checks> newGroup;
        newGroup.enable(Checks::missingInclude);

        group.enable(newGroup);

        ASSERT_EQUALS(2, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        group.disable(newGroup);

        ASSERT_EQUALS(0, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));
    }

    void loadCppcheckCfg()
    {
        {
            Settings s;
            Suppressions supprs;
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            "{}\n");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            "{\n");
            ASSERT_EQUALS("not a valid JSON - syntax error at line 2 near: ", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"productName": ""}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS("", s.cppcheckCfgProductName);
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"productName": "product"}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS("product", s.cppcheckCfgProductName);
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"productName": 1}\n)");
            ASSERT_EQUALS("'productName' is not a string", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"about": ""}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS("", s.cppcheckCfgAbout);
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"about": "about"}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS("about", s.cppcheckCfgAbout);
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"about": 1}\n)");
            ASSERT_EQUALS("'about' is not a string", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": []}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS(0, s.addons.size());
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": 1}\n)");
            ASSERT_EQUALS("'addons' is not an array", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": ["addon"]}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS(1, s.addons.size());
            ASSERT_EQUALS("addon", *s.addons.cbegin());
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": [1]}\n)");
            ASSERT_EQUALS("'addons' array entry is not a string", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": []}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS(0, s.addons.size());
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": 1}\n)");
            ASSERT_EQUALS("'suppressions' is not an array", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": ["id"]}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS(1, supprs.nomsg.getSuppressions().size());
            ASSERT_EQUALS("id", supprs.nomsg.getSuppressions().cbegin()->errorId);
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": [""]}\n)");
            ASSERT_EQUALS("could not parse suppression '' - Failed to add suppression. No id.", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": [1]}\n)");
            ASSERT_EQUALS("'suppressions' array entry is not a string", Settings::loadCppcheckCfg(s, supprs));
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"manualUrl": "https://docs.notcppcheck.com/manual.pdf"})");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS("https://docs.notcppcheck.com/manual.pdf", s.manualUrl);
        }
        {
            Settings s;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg",
                            R"({"manualUrl": 0}\n)");
            ASSERT_EQUALS("'manualUrl' is not a string", Settings::loadCppcheckCfg(s, supprs));
        }

        // TODO: test with FILESDIR
    }

    void loadCppcheckCfgSafety() const
    {
        // Test the "safety" flag
        {
            Settings s;
            s.safety = false;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg", "{}");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS(false, s.safety);
        }

        {
            Settings s;
            s.safety = true;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg", "{\"safety\": false}");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS(false, s.safety);
        }

        {
            Settings s;
            s.safety = false;
            Suppressions supprs;
            ScopedFile file("cppcheck.cfg", "{\"safety\": true}");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, supprs));
            ASSERT_EQUALS(true, s.safety);
        }
    }

    void getNameAndVersion() const
    {
        {
            const auto nameVersion = Settings::getNameAndVersion("Cppcheck Premium 12.3.4");
            ASSERT_EQUALS("Cppcheck Premium", nameVersion.first);
            ASSERT_EQUALS("12.3.4", nameVersion.second);
        }

        {
            const auto nameVersion = Settings::getNameAndVersion("Cppcheck Premium 12.3.4s");
            ASSERT_EQUALS("Cppcheck Premium", nameVersion.first);
            ASSERT_EQUALS("12.3.4s", nameVersion.second);
        }
    }

    void checkLevelDefault() const
    {
        Settings s;
        ASSERT_EQUALS_ENUM(s.checkLevel, Settings::CheckLevel::exhaustive);
        ASSERT_EQUALS(s.vfOptions.maxIfCount, -1);
        ASSERT_EQUALS(s.vfOptions.maxSubFunctionArgs, 256);
        ASSERT_EQUALS(true, s.vfOptions.doConditionExpressionAnalysis);
        ASSERT_EQUALS(-1, s.vfOptions.maxForwardBranches);
    }

    void getMaxConfigsDefault() const {
        Settings s;
        ASSERT_EQUALS(12, s.getMaxConfigs());
    }

    void getMaxConfigsOpt() const {
        Settings s;
        s.maxConfigsOption = 1;
        ASSERT_EQUALS(1, s.getMaxConfigs());
    }

    void getMaxConfigsForce() const {
        Settings s;
        s.force = true;
        ASSERT(s.getMaxConfigs() > 1000);
    }

    void getMaxConfigsOptAndForce() const {
        Settings s;
        s.maxConfigsOption = 1;
        s.force = true;
        ASSERT(s.getMaxConfigs() > 1000);
    }

    void getMaxConfigsDefines() const {
        Settings s;
        s.userDefines = "X=1";
        ASSERT_EQUALS(1, s.getMaxConfigs());
    }

    void getMaxConfigsDefinesAndOpt() const {
        Settings s;
        s.userDefines = "X=1";
        s.maxConfigsOption = 3;
        ASSERT_EQUALS(3, s.getMaxConfigs());
    }

    void getMaxConfigsOptAndProject() const {
        Settings s;
        s.maxConfigsOption = 3;
        s.maxConfigsProject = 1;
        ASSERT_EQUALS(3, s.getMaxConfigs());
        s.maxConfigsProject = 10;
        ASSERT_EQUALS(3, s.getMaxConfigs());
    }
};

REGISTER_TEST(TestSettings)
