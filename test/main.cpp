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

#include "color.h"
#include "options.h"
#include "preprocessor.h"
#include "fixture.h"

#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
    Preprocessor::macroChar = '$';     // While macroChar is char(1) per default outside test suite, we require it to be a human-readable character here.
    gDisableColors = true;

    options args(argc, argv);

    if (args.help()) {
        TestFixture::printHelp();
        return EXIT_SUCCESS;
    }
    if (!args.errors().empty()) {
        for (const auto& error : args.errors())
        {
            std::cout << "error: " << error << '\n';
        }
        return EXIT_FAILURE;
    }
    const std::size_t failedTestsCount = TestFixture::runTests(args);
    return (failedTestsCount == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
