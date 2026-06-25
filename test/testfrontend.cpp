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

#include <filesettings.h>

#include "fixture.h"
#include "frontend.h"
#include "settings.h"
#include "standards.h"

#include <list>
#include <string>

class TestFrontend : public TestFixture {
public:
    TestFrontend() : TestFixture("TestFrontend") {}

private:
    void run() override {
        TEST_CASE(applyLangFS);
        TEST_CASE(applyLangFiles);
    }

    void applyLangFS() const {
        const char xmldata[] = R"(<def format="2"><markup ext=".ml" reporterrors="false"/></def>)";
        const Settings s = settingsBuilder().libraryxml(xmldata).build();

        const std::list<FileSettings> fs = {
            {"nolang", Standards::Language::None, 0 },
            {"c", Standards::Language::None, 0 }, // TODO: should be C - FileSettings are currently expected to not be pre-identified
            {"cpp", Standards::Language::None, 0 }, // TODO: should be CPP - FileSettings arecurrently expected to not be pre-identified
            {"nolang.c", Standards::Language::None, 0 },
            {"nolang.cpp", Standards::Language::None, 0 },
            {"nolang.ml", Standards::Language::None, 0 }
        };

        // no language to enforce - identify only
        {
            std::list<FileSettings> fs1 = fs;
            frontend::applyLang(fs1, s, Standards::Language::None);
            auto it = fs1.cbegin();
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::CPP); // unknown defaults to C++
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::CPP); // unknown defaults to C++
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::CPP); // unknown defaults to C++
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::C); // markup files are always C
        }

        // language to enforce (C)
        {
            std::list<FileSettings> fs1 = fs;
            frontend::applyLang(fs1, s, Standards::Language::C);
            auto it = fs1.cbegin();
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::C); // markup files are always C
        }

        // language to enforce (C++)
        {
            std::list<FileSettings> fs1 = fs;
            frontend::applyLang(fs1, s, Standards::Language::CPP);
            auto it = fs1.cbegin();
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->file.lang(), Standards::Language::C); // markup files are always C
        }
    }

    void applyLangFiles() const
    {
        const char xmldata[] = R"(<def format="2"><markup ext=".ml" reporterrors="false"/></def>)";
        const Settings s = settingsBuilder().libraryxml(xmldata).build();

        const std::list<FileWithDetails> fs = {
            {"nolang", Standards::Language::None, 0 },
            {"c", Standards::Language::C, 0 },
            {"cpp", Standards::Language::CPP, 0 },
            {"nolang.c", Standards::Language::None, 0 },
            {"nolang.cpp", Standards::Language::None, 0 },
            {"nolang.ml", Standards::Language::None, 0 }
        };

        // no language to enforce - identify only
        {
            std::list<FileWithDetails> fs1 = fs;
            frontend::applyLang(fs1, s, Standards::Language::None);
            auto it = fs1.cbegin();
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::CPP); // unknown defaults to C++
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C); // markup files are always C
        }

        // language to enforce (C)
        {
            std::list<FileWithDetails> fs1 = fs;
            frontend::applyLang(fs1, s, Standards::Language::C);
            auto it = fs1.cbegin();
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C); // markup files are always C
        }

        // language to enforce (C++)
        {
            std::list<FileWithDetails> fs1 = fs;
            frontend::applyLang(fs1, s, Standards::Language::CPP);
            auto it = fs1.cbegin();
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::CPP);
            ASSERT_EQUALS_ENUM((it++)->lang(), Standards::Language::C); // markup files are always C
        }
    }
};

REGISTER_TEST(TestFrontend)
