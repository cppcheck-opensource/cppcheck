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
#include "fixture.h"

#ifdef HAS_THREADING_MODEL_THREAD
#include "filesettings.h"
#include "helpers.h"
#include "redirect.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "threadexecutor.h"
#include "timer.h"

#include <cstdlib>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#endif // HAS_THREADING_MODEL_THREAD

class TestThreadExecutorBase : public TestFixture {
public:
    TestThreadExecutorBase(const char * const name, bool useFS)
        : TestFixture(name)
#ifdef HAS_THREADING_MODEL_THREAD
        , useFS(useFS)
#endif // HAS_THREADING_MODEL_THREAD
    {
        (void)useFS;
    }

private:
#ifdef HAS_THREADING_MODEL_THREAD
    /*const*/ Settings settings;
    bool useFS;

    std::string fprefix() const
    {
        if (useFS)
            return "threadfs";
        return "thread";
    }

    struct CheckOptions
    {
        bool quiet = true;
        ShowTime showtime = ShowTime::NONE;
        const char* plistOutput = nullptr;
        std::vector<std::string> filesList;
    };

    /**
     * Execute check using n jobs for y files which are have
     * identical data, given within data.
     */
    void check(unsigned int jobs, int files, int result, const std::string &data, const CheckOptions& opt = make_default_obj{}) {
        std::list<FileSettings> fileSettings;

        std::list<FileWithDetails> filelist;
        if (opt.filesList.empty()) {
            for (int i = 1; i <= files; ++i) {
                std::string f_s = fprefix() + "_" + std::to_string(i) + ".c";
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
        s.jobs = jobs;
        s.showtime = opt.showtime;
        s.quiet = opt.quiet;
        if (opt.plistOutput)
            s.plistOutput = opt.plistOutput;
        s.templateFormat = "{callstack}: ({severity}) {inconclusive:inconclusive: }{message}"; // TODO: remove when we only longer rely on toString() in unique message handling?

        Suppressions supprs;
        TimerResults timerResults;

        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        auto executeFn = [](std::string,std::vector<std::string>,std::string,std::string&){
            return EXIT_SUCCESS;
        };

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filelist.size());
        for (auto i = filelist.cbegin(); i != filelist.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->path(), data));

        // clear files list so only fileSettings are used
        if (useFS)
            filelist.clear();

        ThreadExecutor executor(filelist, fileSettings, s, supprs, *this, &timerResults, executeFn);
        ASSERT_EQUALS(result, executor.check());
    }
#endif // HAS_THREADING_MODEL_THREAD

    void run() override {
        mNewTemplate = true;
#ifdef HAS_THREADING_MODEL_THREAD
        TEST_CASE(deadlock_with_many_errors);
        TEST_CASE(many_threads);
        TEST_CASE(many_threads_showtime);
        TEST_CASE(many_threads_plist);
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
#endif // HAS_THREADING_MODEL_THREAD
    }

#ifdef HAS_THREADING_MODEL_THREAD
    void deadlock_with_many_errors() {
        std::ostringstream oss;
        oss << "void f()\n"
            << "{\n";
        const int num_err = 1;
        for (int i = 0; i < num_err; i++) {
            oss << "  (void)(*((int*)0));\n";
        }
        oss << "}\n";
        const int num_files = 3;
        check(2, num_files, num_files, oss.str());
        ASSERT_EQUALS(1LL * num_err * num_files, cppcheck::count_all_of(errout_str(), "(error) Null pointer dereference: (int*)0"));
    }

    void many_threads() {
        const int num_files = 100;
        check(16, num_files, num_files,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}");
        ASSERT_EQUALS(num_files, cppcheck::count_all_of(errout_str(), "(error) Null pointer dereference: (int*)0"));
    }

    // #11249 - reports TSAN errors - only applies to threads not processes though
    void many_threads_showtime() {
        SUPPRESS;
        check(16, 100, 100,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}", dinit(CheckOptions, $.showtime = ShowTime::SUMMARY));
        // we are not interested in the results - so just consume them
        ignore_errout();
    }

    void many_threads_plist() {
        const std::string plistOutput = "plist_" + fprefix() + "/";
        ScopedFile plistFile("dummy", "", plistOutput);

        check(16, 100, 100,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}", dinit(CheckOptions, $.plistOutput = plistOutput.c_str()));
        // we are not interested in the results - so just consume them
        ignore_errout();
    }

    void no_errors_more_files() {
        check(2, 3, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_less_files() {
        check(2, 1, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_equal_amount_files() {
        check(2, 2, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void one_error_less_files() {
        check(2, 1, 1,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}");
        ASSERT_EQUALS("[" + fprefix() + "_1.c:3:12]: (error) Null pointer dereference: (int*)0 [nullPointer]\n", errout_str());
    }

    void one_error_several_files() {
        const int num_files = 20;
        check(2, num_files, num_files,
              "void f()\n"
              "{\n"
              "  (void)(*((int*)0));\n"
              "}");
        ASSERT_EQUALS(num_files, cppcheck::count_all_of(errout_str(), "(error) Null pointer dereference: (int*)0"));
    }

    // TODO: provide data which actually shows values above 0

    // TODO: should this be logged only once like summary?
    void showtime_top5_file() {
        REDIRECT; // should not cause TSAN failures as the showtime logging is synchronized
        check(2, 2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = ShowTime::TOP5_FILE));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        // for each file: top5 results + check time
        ASSERT_EQUALS((5 + 1) * 2LL, cppcheck::count_all_of(output_s, '\n'));
    }

    void showtime_file() {
        REDIRECT; // should not cause TSAN failures as the showtime logging is synchronized
        check(2, 2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = ShowTime::FILE));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        ASSERT_EQUALS(0, cppcheck::count_all_of(output_s, "Overall time:"));
    }

    void showtime_file_total() {
        REDIRECT; // should not cause TSAN failures as the showtime logging is synchronized
        check(2, 2, 0,
              "int main() {}",
              dinit(CheckOptions,
                    $.showtime = ShowTime::FILE_TOTAL));
        const std::string output_s = GET_REDIRECT_OUTPUT;
        ASSERT(output_s.find("Check time: " + fprefix() + "_1.c: ") != std::string::npos);
        ASSERT(output_s.find("Check time: " + fprefix() + "_2.c: ") != std::string::npos);
    }

    void suppress_error_library() {
        SUPPRESS;
        const Settings settingsOld = settings; // TODO: get rid of this
        const char xmldata[] = R"(<def format="2"><markup ext=".c" reporterrors="false"/></def>)";
        settings = settingsBuilder().libraryxml(xmldata).build();
        check(2, 1, 0,
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
        check(2, 2, 2,
              "#include \"" + inc_h.name() +"\"");
        // this is made unique by the executor
        ASSERT_EQUALS("[" + inc_h.name() + ":3:12]: (error) Null pointer dereference: (int*)0 [nullPointer]\n", errout_str());
    }

    // TODO: test whole program analysis
#endif // HAS_THREADING_MODEL_THREAD
};

class TestThreadExecutorFiles : public TestThreadExecutorBase {
public:
    TestThreadExecutorFiles() : TestThreadExecutorBase("TestThreadExecutorFiles", false) {}
};

class TestThreadExecutorFS : public TestThreadExecutorBase {
public:
    TestThreadExecutorFS() : TestThreadExecutorBase("TestThreadExecutorFS", true) {}
};

REGISTER_TEST(TestThreadExecutorFiles)
REGISTER_TEST(TestThreadExecutorFS)
