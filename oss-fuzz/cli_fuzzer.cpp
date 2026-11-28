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

// Fuzzing rationale (oss-fuzz-client-cli):
//
// The entire cli/ front end (cli/cmdlineparser.cpp alone is ~2.3k lines) is
// not part of the OSS-Fuzz build at all: the existing harness links only
// lib/ + the grammar generator, so the command-line parser - which parses
// fully attacker-controlled text (users routinely run cppcheck on untrusted
// repositories whose build scripts embed crafted arguments, and CI systems
// pass user-controlled option strings) - has zero coverage by construction.
//
// This harness feeds NUL-separated argument strings through the same public
// entry point the executor uses (CmdLineParser::fillSettingsFromArgs),
// which returns a Result instead of exiting, and probes the parsed file
// lists. Options such as --library/--project/--suppressions-list/--rule
// additionally drive the library/project/suppression loading code paths
// with attacker-controlled names. Findings here are real bugs reachable
// from the command line.
//
// Input format: NUL-separated argument strings (argv[0] "cppcheck" is
// synthesized by the harness; the input holds only the arguments).

#include "cmdlineparser.h"
#include "cmdlinelogger.h"
#include "settings.h"
#include "suppressions.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {
    // argument strings are short; keep runtime bounded
    constexpr size_t kMaxInputSize = 4096;
    constexpr int kMaxArgs = 64;

    class NullLogger : public CmdLineLogger {
    public:
        void printMessage(const std::string&) override {}
        void printError(const std::string&) override {}
        void printRaw(const std::string&) override {}
    };
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize)
{
    if (dataSize < 2 || dataSize > kMaxInputSize)
        return 0;

    std::vector<std::string> args;
    args.push_back("cppcheck");
    {
        size_t start = 0;
        for (size_t i = 0; i <= dataSize && args.size() < kMaxArgs; ++i) {
            if (i == dataSize || data[i] == 0) {
                if (i > start) {
                    const size_t len = (i - start) > 1024 ? 1024 : (i - start);
                    args.push_back(std::string(reinterpret_cast<const char*>(data) + start, len));
                }
                start = i + 1;
            }
        }
    }

    std::vector<const char*> argv;
    argv.reserve(args.size());
    for (const std::string& a : args)
        argv.push_back(a.c_str());

    try {
        Settings settings;
        Suppressions supprs;
        NullLogger logger;
        CmdLineParser parser(logger, settings, supprs);
        (void)parser.fillSettingsFromArgs(static_cast<int>(argv.size()), argv.data());
        // probe the parse results cheaply
        (void)parser.getFiles().size();
        (void)parser.getFileSettings().size();
    } catch (const std::exception&) {
        // options such as --rule=... may throw on bad regexes; a fuzzer must not die
    } catch (...) {
    }
    return 0;
}
