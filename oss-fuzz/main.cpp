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

#include "cppcheck.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "filesettings.h"
#include "settings.h"
#include "suppressions.h"

#include <string>

#ifndef NO_FUZZ
#include <cstddef>
#include <cstdint>

#include "type2.h"
#else
#include <cstdlib>
#include <fstream>
#include <sstream>
#endif

class DummyErrorLogger : public ErrorLogger {
public:
    void reportOut(const std::string& /*outmsg*/, Color /*c*/) override {}
    void reportErr(const ErrorMessage& /*msg*/) override {}
    void reportProgress(const std::string& /*filename*/,
                        const char /*stage*/[],
                        const std::size_t /*value*/) override {}
    void reportMetric(const std::string & /*metric*/) override {}
};

static Settings create_settings()
{
    // TODO: load std.cfg
    Settings s;
    s.templateFormat = "{bold}{file}:{line}:{column}: {red}{inconclusive:{magenta}}{severity}:{inconclusive: inconclusive:}{default} {message} [{id}]{reset}\\n{code}";
    s.templateLocation = "{bold}{file}:{line}:{column}: {dim}note:{reset} {info}\\n{code}";
    s.addEnabled("all");
    s.certainty.setEnabled(Certainty::inconclusive, true);
    return s;
}
static const Settings s_settings = create_settings();
static DummyErrorLogger s_errorLogger;
static const FileWithDetails s_file("test.cpp", Standards::Language::CPP, 0);

static void doCheck(const uint8_t *data, size_t dataSize)
{
    Suppressions supprs;
    CppCheck cppcheck(s_settings, supprs, s_errorLogger, nullptr, false, nullptr);
    cppcheck.checkBuffer(s_file, reinterpret_cast<const char*>(data), dataSize);
}

#ifndef NO_FUZZ
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t dataSize)
{
    if (dataSize < 10000) {
        const std::string code = generateCode2(data, dataSize);
        doCheck(reinterpret_cast<const unsigned char*>(code.data()), code.size());
    }
    return 0;
}
#else
int main(int argc, char * argv[])
{
    if (argc < 2 || argc > 3)
        return EXIT_FAILURE;

    std::ifstream f(argv[1]);
    if (!f.is_open())
        return EXIT_FAILURE;

    std::ostringstream oss;
    oss << f.rdbuf();

    if (!f.good())
        return EXIT_FAILURE;

    const int cnt = (argc == 3) ? std::stoi(argv[2]) : 1;

    const std::string code = oss.str();
    for (int i = 0; i < cnt; ++i)
        doCheck(reinterpret_cast<const unsigned char*>(code.data()), code.size());

    return EXIT_SUCCESS;
}
#endif


