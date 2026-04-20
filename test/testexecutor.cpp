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

#include "config.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "executor.h"
#include "filesettings.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"

#include <list>
#include <stdexcept>
#include <string>
#include <utility>

class DummyExecutor : public Executor
{
public:
    DummyExecutor(const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger)
        : Executor(files, fileSettings, settings, suppressions, errorLogger, nullptr)
    {}

    NORETURN unsigned int check() override
    {
        throw std::runtime_error("not implemented");
    }

    bool hasToLog_(const ErrorMessage &msg)
    {
        return hasToLog(msg);
    }
};

class TestExecutor : public TestFixture {
public:
    TestExecutor() : TestFixture("TestExecutor") {}

private:
    void run() override {
        TEST_CASE(hasToLogSimple);
    }

    void hasToLogSimple() {
        const std::list<FileWithDetails> files{FileWithDetails{"test.c", Standards::Language::C, 0}};
        const std::list<FileSettings> fileSettings;
        // this is the "simple" format
        const auto settings = dinit(Settings,
                                    $.templateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]");
        Suppressions supprs;
        DummyExecutor executor(files, fileSettings, settings, supprs, *this);

        ErrorMessage::FileLocation loc1("test.c", 1, 2);
        ErrorMessage msg({std::move(loc1)}, "test.c", Severity::error, "error", "id", Certainty::normal);

        ASSERT(executor.hasToLog_(msg));
        ASSERT(!executor.hasToLog_(msg));

        ErrorMessage::FileLocation loc2("test.c", 1, 12);
        msg.callStack = {std::move(loc2)};

        ASSERT(executor.hasToLog_(msg));

        msg.id = "id2";

        ASSERT(executor.hasToLog_(msg));
    }
};

REGISTER_TEST(TestExecutor)
