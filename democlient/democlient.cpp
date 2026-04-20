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
#include "version.h"

#include <algorithm>
#include <ctime>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

enum class Color : std::uint8_t;

static void unencode(const char *src, char *dest)
{
    for (; *src; src++, dest++) {
        if (*src == '+')
            *dest = ' ';
        else if (*src == '%') {
            unsigned int code;
            if (std::sscanf(src+1, "%2x", &code) != 1)
                code = '?';
            *dest = code;
            src += 2;
        } else
            *dest = *src;
    }
    *dest = '\0';
}

static FILE *logfile = nullptr;

class CppcheckExecutor : public ErrorLogger {
private:
    const std::time_t stoptime;
    Suppressions supprs;
    CppCheck cppcheck;

public:
    CppcheckExecutor(const Settings& settings)
        : stoptime(std::time(nullptr)+2U)
        , cppcheck(settings, supprs, *this, nullptr, false, nullptr)
    {}

    void run(const char* code) {
        cppcheck.checkBuffer(FileWithDetails("test.cpp", Standards::Language::CPP, 0), code, strlen(code));
    }

    void reportOut(const std::string & /*outmsg*/, Color /*c*/) override {}
    void reportErr(const ErrorMessage &msg) override {
        static const std::string templateFormat = "{bold}{file}:{line}:{column}: {red}{inconclusive:{magenta}}{severity}:{inconclusive: inconclusive:}{default} {message} [{id}]{reset}\\n{code}";
        static const std::string templateLocation = "{bold}{file}:{line}:{column}: {dim}note:{reset} {info}\\n{code}";
        const std::string s = msg.toString(true, templateFormat, templateLocation);

        std::cout << s << std::endl;

        if (logfile != nullptr)
            std::fprintf(logfile, "%s\n", s.c_str());
    }
    void reportMetric(const std::string & /*metric*/) override {}

    void reportProgress(const std::string& /*filename*/,
                        const char /*stage*/[],
                        const std::size_t /*value*/) override {
        if (std::time(nullptr) >= stoptime) {
            std::cout << "Time to analyse the code exceeded 2 seconds. Terminating.\n\n";
            Settings::terminate();
        }
    }
};


int main()
{
    std::cout << "Content-type: text/html\r\n\r\n"
              << "<!DOCTYPE html>\n";

    char data[4096] = {0};

    const char *query_string = std::getenv("QUERY_STRING");
    if (query_string)
        std::strncpy(data, query_string, sizeof(data)-2);

    const char *lenstr = std::getenv("CONTENT_LENGTH");
    if (lenstr) {
        int len = std::min(1 + std::atoi(lenstr), (int)(sizeof(data) - 2));
        std::fgets(data, len, stdin);
    }

    if (data[4000] != '\0') {
        std::cout << "<html><body>For performance reasons the code must be shorter than 1000 chars.</body></html>";
        return EXIT_SUCCESS;
    }

    const char *pdata = data;
    if (std::strncmp(pdata, "code=", 5)==0)
        pdata += 5;

    char code[4096] = {0};
    unencode(pdata, code);

    logfile = std::fopen("democlient.log", "at");
    if (logfile != nullptr)
        std::fprintf(logfile, "===========================================================\n%s\n", code);

    std::cout << "<html><body>Cppcheck " CPPCHECK_VERSION_STRING "<pre>";

    Settings s;
    s.addEnabled("all");
    s.certainty.enable(Certainty::inconclusive);
    CppcheckExecutor cppcheckExecutor(s);
    cppcheckExecutor.run(code);

    std::fclose(logfile);

    std::cout << "</pre>Done!</body></html>";

    return EXIT_SUCCESS;
}
