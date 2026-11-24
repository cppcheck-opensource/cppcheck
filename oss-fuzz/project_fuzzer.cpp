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

// Fuzzing rationale (oss-fuzz-client-project):
//
// ImportProject::import() parses project files that describe builds:
// compile_commands.json (picojson), Visual Studio .sln/.slnx/.vcxproj,
// Code::Blocks .cbp, Borland .bpr and cppcheck GUI .cppcheck projects.
// These files are attacker-controllable in practice: cppcheck is pointed
// at checkouts that carry them, and the XML/JSON parsers walk fully
// fuzz-derived structure and attribute data. In the 2025-02-17 production
// coverage report importproject.cpp is completely dark (0/1208 lines)
// because the existing harness never imports any project.
//
// This harness writes the fuzz bytes to a temporary file whose extension
// (selected from the first byte) picks the importer, drives the same
// public entry point the CLI uses (--project=...), and probes the
// imported file settings and error lists. A memory-safety or UB finding
// here is a real bug reachable by anyone running cppcheck on a malicious
// repository.

#include "importproject.h"
#include "settings.h"
#include "suppressions.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize);

namespace {
    constexpr size_t kMaxProjectSize = 1024 * 1024;
    const char* kExtensions[] = {".json", ".sln", ".vcxproj", ".cbp", ".cppcheck", ".bpr", ".slnx"};
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize)
{
    if (dataSize < 8 || dataSize > kMaxProjectSize)
        return 0;

    static unsigned counter = 0;
    char path[128];
    const int sel = data[0] % 7;
    snprintf(path, sizeof(path), "/tmp/cppcheck_projfuzz_%d_%u%s",
             static_cast<int>(getpid()), counter++, kExtensions[sel]);

    FILE* f = fopen(path, "wb");
    if (!f)
        return 0;
    const size_t written = fwrite(data + 1, 1, dataSize - 1, f);
    fclose(f);
    if (written != dataSize - 1) {
        unlink(path);
        return 0;
    }

    try {
        Settings settings;
        Suppressions supprs;
        ImportProject importer;
        (void)importer.import(path, &settings, &supprs);
        (void)importer.fileSettings.size();
        (void)importer.errors.size();
    } catch (const std::exception&) {
        // malformed project data must not kill the fuzzer
    } catch (...) {
    }

    unlink(path);
    return 0;
}
