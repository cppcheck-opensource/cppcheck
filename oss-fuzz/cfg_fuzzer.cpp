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

// Fuzzing rationale (oss-fuzz-client-cfg):
//
// cppcheck loads library configuration files (.cfg XML) from user-controlled
// locations (--library=..., project files, addons) and parses them with its
// own dispatch code (lib/library.cpp, ~2.1k lines) on top of the bundled XML
// parser. The existing OSS-Fuzz harness never loads any library config (its
// own create_settings() says "TODO: load std.cfg"), so library.cpp is almost
// completely dark (276/1809 covered in the 2025-02-17 production report) and
// the suppressions/platform/standards config layers are similarly dark.
//
// This harness treats the fuzz input as an attacker-supplied .cfg file and
// drives it through the same public entry point the CLI uses
// (Library::load), then probes the loaded tables. A memory-safety or UB
// finding here is a real bug reachable by anyone running cppcheck with a
// malicious or corrupted library config file.

#include "library.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize);

namespace {
    // keeps runtime bounded; real-world .cfg files are up to ~1 MB
    constexpr size_t kMaxCfgSize = 1024 * 1024;

    const char* kProbeFunctions[] = {
        "malloc", "calloc", "realloc", "free", "fopen", "fclose",
        "printf", "memcpy", "strlen", "strdup", "read", "write"
    };
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize)
{
    if (dataSize < 8 || dataSize > kMaxCfgSize)
        return 0;

    char path[] = "/tmp/cppcheck_cfgfuzz_XXXXXX";
    const int fd = mkstemp(path);
    if (fd < 0)
        return 0;
    FILE* const f = fdopen(fd, "wb");
    if (!f) {
        close(fd);
        unlink(path);
        return 0;
    }
    const size_t written = fwrite(data, 1, dataSize, f);
    fclose(f);
    if (written != dataSize) {
        unlink(path);
        return 0;
    }

    try {
        Library lib;
        const Library::Error err = lib.load(nullptr, path);
        if (err.errorcode == Library::ErrorCode::OK) {
            // probe the parsed tables with fixed, well-known names
            for (const char* fn : kProbeFunctions) {
                (void)lib.getAllocFuncInfo(fn);
                (void)lib.getDeallocFuncInfo(fn);
                (void)lib.allocId(fn);
                (void)lib.deallocId(fn);
            }
            (void)lib.hasAnyTypeCheck("int");
            (void)lib.getTypeCheck("use", "int");
            (void)lib.getTypeCheck("suppress", "char*");
            (void)lib.functionwarn().size();
            (void)lib.platform_type("size_t", "unix64");
            (void)lib.platform_type("wchar_t", "win32W");
        }
    } catch (const std::exception&) {
        // Library::load throws on some malformed inputs; a fuzzer must not die
    } catch (...) {
    }

    unlink(path);
    return 0;
}
