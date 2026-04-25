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
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cppcheck.h"
#include "filesettings.h"
#include "fixture.h"
#include "helpers.h"
#include "redirect.h"
#include "settings.h"
#include "singleexecutor.h"
#include "standards.h"
#include "suppressions.h"
#include "timer.h"

#include <cstdlib>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class TestSingleExecutorBase : public TestFixture {
protected:
    TestSingleExecutorBase(const char * const name, bool useFS) : TestFixture(name), useFS(useFS) {}

private:
    /*const*/ Settings settings;
    bool useFS;

    std::string fprefix() const
    {
        if (useFS)
            return "singlefs";
        return "single";
    }

    static std::string zpad3(int i)
    {
        if (i < 10)
            return "00" + std::to_string(i);
        if (i < 100)
            return "0" + std::to_string(i);
        return std::to_string(i);
    }

    struct CheckOptions
    {
        bool quiet = true;
        ShowTime showtime = ShowTime::NONE;
        const char* plistOutput = nullptr;
        std::vector<std::string> filesList;
    };

    void check(int files, int result, const std::string &data, const CheckOptions& opt = make_default_obj{}) {
        std::list<FileSettings> fileSettings;

        std::list<FileWithDetails> filelist;
        if (opt.filesList.empty()) {
            for (int i = 1; i <= files; ++i) {
                std::string f_s = fprefix() + "_" + zpad3(i) + ".c";
                filelist.emplace_back(f_s, Standards::Language::C, data.size());
                if (useFS) {
                    fileSettings.emplace_back(std::move(f_s), Standards::Language::C, data.size());
                }
            }
        }
        else {
            for (const auto& f : opt.filesList)
            {
                filelist.emplace_back(f, Standards::Language::C, data.size());
                if (useFS) {
                    fileSettings.emplace_back(f, Standards::Language::C, data.size());
                }
            }
        }

        /*const*/ Settings s = settings;
        s.showtime = opt.showtime;
        s.quiet = opt.quiet;
        if (opt.plistOutput)
            s.plistOutput = opt.plistOutput;
        s.templateFormat = "{callstack}: ({severity}) {inconclusive:inconclusive: }{message}"; // TODO: remove when we only longer rely on toString() in unique message handling?

        Suppressions supprs;
        TimerResults timerResults;

        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        CppCheck cppcheck(s, supprs, *this, &timerResults, true, [](std::string,std::vector<std::string>,std::string,std::string&){
            return EXIT_SUCCESS;
        });

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filelist.size());
        for (auto i = filelist.cbegin(); i != filelist.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->path(), data));

        // clear files list so only fileSettings are used
        if (useFS)
            filelist.clear();

        SingleExecutor executor(cppcheck, filelist, fileSettings, s, supprs, *this, &timerResults);
        ASSERT_EQUALS(result, executor.check());
    }

    void run() override {
        mNewTemplate = true;
        TEST_CASE(many_files);
        TEST_CASE(many_files_showtime);
        TEST_CASE(many_files_plist);
        TEST_CASE(no_errors_more_files);
        TEST_CASE(no_errors_less_files);
        TEST_CASE(no_errors_equal_amount_files);
        TEST_CASE(one_error_less_files);
        TEST_CASE(one_error_several_files);
        TEST_CASE(showtime_top5_file);
        TEST_CASE(showtime_file);
        TEST_CASE(showtime_file_total);
        TEST_CASE(suppress_error_library);
        TEST_CASE(unique_errors);
    }

    void many_files() {
        const int num_files = 100;
        check(num_files, num_files,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}", dinit(CheckOptions,
                         $.quiet = false));
        {
            std::string expected;
            for (int i = 1; i <= num_files; ++i) {
                expected += "Checking " + fprefix() + "_" + zpad3(i) + ".c ...\n";
                expected += std::to_string(i) + "/100 files checked " + std::to_string(i) + "% done\n";
            }
            ASSERT_EQUALS(expected, output_str());
        }
        {
            std::string expected;
            for (int i = 1; i <= num_files; ++i) {
                expected += "[" + fprefix() + "_" + zpad3(i) + ".c:3:12]: (error) Null pointer dereference: (int*)0 [nullPointer]\n";
            }
            ASSERT_EQUALS(expected, errout_str());
        }
    }

    void many_files_showtime() {
        SUPPRESS;
        check(100, 100,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}", dinit(CheckOptions, $.showtime = ShowTime::SUMMARY));
        // we are not interested in the results - so just consume them
        ignore_errout();
    }

    void many_files_plist() {
        const std::string plistOutput = "plist_" + fprefix() + "/";
        ScopedFile plistFile("dummy", "", plistOutput);

        check(100, 100,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}", dinit(CheckOptions, $.plistOutput = plistOutput.c_str()));
        // we are not interested in the results - so just consume them
        ignore_errout();
    }

    void no_errors_more_files() {
        check(3, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_less_files() {
        check(1, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_equal_amount_files() {
        check(2, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void one_error_less_files() {
        check(1, 1,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}");
        ASSERT_EQUALS("[" + fprefix() + "_" + zpad3(1) + ".c:3:12]: (error) Null pointer dereference: (int*)0 [nullPointer]\n", errout_str());
    }

    void one_error_several_files() {
        const int num_files = 20;
        check(num_files, num_files,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}");
        {
            std::string expected;
            for (int i = 1; i <= num_files; ++i) {
                expected += "[" + fprefix() + "_" + zpad3(i) + ".c:3:12]: (error) Null pointer dereference: (int*)0 [nullPointer]\n";
            }
            ASSERT_EQUALS(expected, errout_str());
        }
    }

// TODO: provide data which actually shows values above 0

    void showtime_top5_file() {
        REDIRECT;
        check(2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = ShowTime::TOP5_FILE));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        // for each file: top5 results + check time
        ASSERT_EQUALS((5 + 1) * 2LL, cppcheck::count_all_of(output_s, '\n'));
    }

    void showtime_file() {
        REDIRECT;
        check(2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = ShowTime::FILE));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        ASSERT_EQUALS(0, cppcheck::count_all_of(output_s, "Overall time:"));
    }

    void showtime_file_total() {
        REDIRECT;
        check(2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = ShowTime::FILE_TOTAL));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        ASSERT(output_s.find("Check time: " + fprefix() + "_" + zpad3(1) + ".c: ") != std::string::npos);
        ASSERT(output_s.find("Check time: " + fprefix() + "_" + zpad3(2) + ".c: ") != std::string::npos);
    }

    void suppress_error_library() {
        SUPPRESS;
        const Settings settingsOld = settings; // TODO: get rid of this
        const char xmldata[] = R"(<def format="2"><markup ext=".c" reporterrors="false"/></def>)";
        settings = settingsBuilder().libraryxml(xmldata).build();
        check(1, 0,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}");
        ASSERT_EQUALS("", errout_str());
        settings = settingsOld;
    }

    void unique_errors() {
        SUPPRESS;
        ScopedFile inc_h(fprefix() + ".h",
                         "inline void f()\n"
                         "{\n"
                         "  (void)(*((int*)0));\n"
                         "}");
        check(2, 2,
              "#include \"" + inc_h.name() + "\"");
        // these are not actually made unique by the implementation. That needs to be done by the given ErrorLogger
        ASSERT_EQUALS(
            "[" + inc_h.name() + ":3:12]: (error) Null pointer dereference: (int*)0 [nullPointer]\n"
            "[" + inc_h.name() + ":3:12]: (error) Null pointer dereference: (int*)0 [nullPointer]\n",
            errout_str());
    }

    // TODO: test whole program analysis
    // TODO: test unique errors
};

class TestSingleExecutorFiles : public TestSingleExecutorBase {
public:
    TestSingleExecutorFiles() : TestSingleExecutorBase("TestSingleExecutorFiles", false) {}
};

class TestSingleExecutorFS : public TestSingleExecutorBase {
public:
    TestSingleExecutorFS() : TestSingleExecutorBase("TestSingleExecutorFS", true) {}
};

REGISTER_TEST(TestSingleExecutorFiles)
REGISTER_TEST(TestSingleExecutorFS)
