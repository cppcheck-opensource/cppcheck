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

// Fuzzing rationale (oss-fuzz-client-raw):
//
// The existing OSS-Fuzz harness (main.cpp) rewrites every fuzzer input through
// the type2 grammar generator before analysis, and that grammar never emits
// preprocessor directives (#include/#define/#if/...), raw string literals,
// line markers or many other C/C++ constructs. The preprocessor
// (lib/preprocessor.cpp + externals/simplecpp) and the diagnostic-formatting
// layer (lib/errorlogger.cpp) are therefore structurally unreachable for the
// mutator even though they parse fully attacker-controlled text in production
// (cppcheck is routinely run on untrusted source files by CI systems).
//
// This harness feeds the raw bytes straight into the same analysis entry
// point (CppCheck::checkBuffer) so the lexer, preprocessor and error
// formatting layers are exercised with unmodified attacker-controlled text.
// A memory-safety or UB finding here is a real bug reachable by anyone
// running cppcheck on untrusted code.
//
// Input format: byte 0 selects the language mode (odd bit: C++, else C);
// bytes 1.. are the source text passed through verbatim.

#include "cppcheck.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "filesettings.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace {
    class DummyErrorLogger : public ErrorLogger {
    public:
        void reportOut(const std::string& /*outmsg*/, Color /*c*/) override {}
        void reportErr(const ErrorMessage& /*msg*/) override {}
        void reportProgress(const std::string& /*filename*/,
                            const char /*stage*/[],
                            const std::size_t /*value*/) override {}
        void reportMetric(const std::string & /*metric*/) override {}
    };

    Settings create_settings()
    {
        Settings s;
        s.templateFormat = "{bold}{file}:{line}:{column}: {red}{inconclusive:{magenta}}{severity}:{inconclusive: inconclusive:}{default} {message} [{id}]{reset}\\n{code}";
        s.templateLocation = "{bold}{file}:{line}:{column}: {dim}note:{reset} {info}\\n{code}";
        s.addEnabled("all");
        s.certainty.setEnabled(Certainty::inconclusive, true);
        return s;
    }
} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize)
{
    // keep analysis cost bounded; mirrors the main.cpp input cap
    if (dataSize < 2 || dataSize >= 10000)
        return 0;

    const bool cppSource = (data[0] & 1) != 0;
    const std::string code(reinterpret_cast<const char*>(data + 1), dataSize - 1);

    static DummyErrorLogger s_errorLogger;
    Suppressions supprs;
    const Settings s_settings = create_settings();
    CppCheck cppcheck(s_settings, supprs, s_errorLogger, nullptr, false, nullptr);
    const FileWithDetails file(cppSource ? "test.cpp" : "test.c",
                               cppSource ? Standards::Language::CPP : Standards::Language::C,
                               code.size());
    cppcheck.checkBuffer(file, code.data(), code.size());
    return 0;
}
