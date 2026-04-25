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

#include "addoninfo.h"
#include "config.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "filesettings.h"
#include "fixture.h"
#include "helpers.h"
#include "processexecutor.h"
#include "settings.h"
#include "singleexecutor.h"
#include "standards.h"
#include "suppressions.h"
#include "threadexecutor.h"

#include <cstring>
#include <initializer_list>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class TestSuppressions : public TestFixture {
public:
    TestSuppressions() : TestFixture("TestSuppressions") {}

private:

    class CppCheckExecutorTest final : public CppCheckExecutor
    {
        friend class TestSuppressions;
    };

    const std::string templateFormat{"{callstack}: ({severity}) {inconclusive:inconclusive: }{message}"};

    void run() override {
        mNewTemplate = true;
        TEST_CASE(parseLine);
        TEST_CASE(parseLineInvalid);
        TEST_CASE(suppressionsBadId1);
        TEST_CASE(suppressionsDosFormat);     // Ticket #1836
        TEST_CASE(suppressionsFileNameWithColon);    // Ticket #1919 - filename includes colon
        TEST_CASE(suppressionsGlob);
        TEST_CASE(suppressionsGlobId);
        TEST_CASE(suppressionsFileNameWithExtraPath);
        TEST_CASE(suppressionsSettingsFiles);
        TEST_CASE(suppressionsSettingsFS);
#ifdef HAS_THREADING_MODEL_THREAD
        TEST_CASE(suppressionsSettingsThreadsFiles);
        TEST_CASE(suppressionsSettingsThreadsFS);
#endif // HAS_THREADING_MODEL_THREAD
#ifdef HAS_THREADING_MODEL_FORK
        TEST_CASE(suppressionsSettingsProcessesFiles);
        TEST_CASE(suppressionsSettingsProcessesFS);
#endif // HAS_THREADING_MODEL_FORK
        TEST_CASE(suppressionsMultiFileFiles);
        TEST_CASE(suppressionsMultiFileFS);
        TEST_CASE(suppressionsPathSeparator);
        TEST_CASE(suppressionsLine0);
        TEST_CASE(suppressionsFileComment);

        TEST_CASE(inlinesuppress);
        TEST_CASE(inlinesuppress_symbolname_Files);
        TEST_CASE(inlinesuppress_symbolname_FS);
        TEST_CASE(inlinesuppress_comment);
        TEST_CASE(inlinesuppress_unchecked);

        TEST_CASE(multi_inlinesuppress);
        TEST_CASE(multi_inlinesuppress_comment);

        TEST_CASE(globalSuppressions); // Testing that global suppressions work (#8515)

        TEST_CASE(inlinesuppress_unusedFunction); // #4210 - unusedFunction
        TEST_CASE(globalsuppress_unusedFunction); // #4946
        TEST_CASE(suppressionWithRelativePaths); // #4733
        TEST_CASE(suppressingSyntaxErrorsFiles); // #7076
        TEST_CASE(suppressingSyntaxErrorsFS); // #7076
        TEST_CASE(suppressingSyntaxErrorsInlineFiles); // #5917
        TEST_CASE(suppressingSyntaxErrorsInlineFS); // #5917
        TEST_CASE(suppressingSimplecppErrorsWhileFileReadFiles); // PR #1333
        TEST_CASE(suppressingSimplecppErrorsWhileFileReadFS); // PR #1333
        TEST_CASE(symbol);

        TEST_CASE(unusedFunctionFiles);
        TEST_CASE(unusedFunctionFS);

        TEST_CASE(suppressingSyntaxErrorAndExitCodeFiles);
        TEST_CASE(suppressingSyntaxErrorAndExitCodeFS);
        TEST_CASE(suppressingSyntaxErrorAndExitCodeMultiFileFiles);
        TEST_CASE(suppressingSyntaxErrorAndExitCodeMultiFileFS);
        TEST_CASE(suppressLocal);

        TEST_CASE(suppressUnmatchedSuppressions);
        TEST_CASE(addSuppressionDuplicate);
        TEST_CASE(updateSuppressionState);
        TEST_CASE(addSuppressionLineMultiple);

        TEST_CASE(suppressionsParseXmlFile);

        TEST_CASE(toString);

        TEST_CASE(suppressionFromErrorMessage);

        TEST_CASE(suppressionWildcard);

        TEST_CASE(polyspaceParseRange);
        TEST_CASE(polyspaceParseIds);

        TEST_CASE(polyspaceMisraC2012);
        TEST_CASE(polyspacePremiumMisraC2012);
        TEST_CASE(polyspaceMisraC2023);
        TEST_CASE(polyspaceMisraCpp2008);
        TEST_CASE(polyspaceMisraCpp2023);
        TEST_CASE(polyspaceCertC);
        TEST_CASE(polyspaceCertCpp);
        TEST_CASE(polyspaceAutosar);
        TEST_CASE(polyspaceIgnored);
        TEST_CASE(polyspaceMultiple1);
        TEST_CASE(polyspaceMultiple2);
        TEST_CASE(polyspaceMultiple3);
        TEST_CASE(polyspaceRange);
        TEST_CASE(polyspaceBlock);
        TEST_CASE(polyspaceExtraComments);
    }


    void parseLine() const {
        ASSERT_EQUALS("bad", SuppressionList::parseLine("bad").toString());
        ASSERT_EQUALS("bad:test.c", SuppressionList::parseLine("bad:test.c").toString());
        ASSERT_EQUALS("bad:test.c:1", SuppressionList::parseLine("bad:test.c:1").toString());

        // symbol
        ASSERT_EQUALS("bad\nsymbol=x", SuppressionList::parseLine("bad\nsymbol=x").toString());
        ASSERT_EQUALS("bad:test.c\nsymbol=x", SuppressionList::parseLine("bad:test.c\nsymbol=x").toString());
        ASSERT_EQUALS("bad:test.c:1\nsymbol=x", SuppressionList::parseLine("bad:test.c:1\nsymbol=x").toString());

        // empty symbol
        ASSERT_EQUALS("bad", SuppressionList::parseLine("bad\nsymbol=").toString());
        ASSERT_EQUALS("bad:test.c", SuppressionList::parseLine("bad:test.c\nsymbol=").toString());
        ASSERT_EQUALS("bad:test.c:1", SuppressionList::parseLine("bad:test.c:1\nsymbol=").toString());

        // polyspace
        ASSERT_EQUALS("bad\npolyspace=1", SuppressionList::parseLine("bad\npolyspace=1").toString());
        ASSERT_EQUALS("bad:test.c\npolyspace=1", SuppressionList::parseLine("bad:test.c\npolyspace=1").toString());
        ASSERT_EQUALS("bad:test.c:1\npolyspace=1", SuppressionList::parseLine("bad:test.c:1\npolyspace=1").toString());

        // symbol + polyspace
        ASSERT_EQUALS("bad\nsymbol=x\npolyspace=1", SuppressionList::parseLine("bad\nsymbol=x\npolyspace=1").toString());
        ASSERT_EQUALS("bad:test.c\nsymbol=x\npolyspace=1", SuppressionList::parseLine("bad:test.c\nsymbol=x\npolyspace=1").toString());
        ASSERT_EQUALS("bad:test.c:1\nsymbol=x\npolyspace=1", SuppressionList::parseLine("bad:test.c:1\nsymbol=x\npolyspace=1").toString());

        // polyspace + symbol
        ASSERT_EQUALS("bad\nsymbol=x\npolyspace=1", SuppressionList::parseLine("bad\npolyspace=1\nsymbol=x").toString());
        ASSERT_EQUALS("bad:test.c\nsymbol=x\npolyspace=1", SuppressionList::parseLine("bad:test.c\npolyspace=1\nsymbol=x").toString());
        ASSERT_EQUALS("bad:test.c:1\nsymbol=x\npolyspace=1", SuppressionList::parseLine("bad:test.c:1\npolyspace=1\nsymbol=x").toString());
    }

    void parseLineInvalid() const {
        // missing filename
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:"), std::runtime_error, "filename is missing");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:\n"), std::runtime_error, "filename is missing");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:\n1.c"), std::runtime_error, "filename is missing");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:#1.c"), std::runtime_error, "filename is missing"); // TODO: looks like a valid filename
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id://1.c"), std::runtime_error, "filename is missing");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id::"), std::runtime_error, "filename is missing");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id::1"), std::runtime_error, "filename is missing");

        // missing/invalid line
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:"), std::runtime_error, "invalid line number (converting '' to integer failed - not an integer (invalid_argument))");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:\n"), std::runtime_error, "invalid line number (converting '' to integer failed - not an integer (invalid_argument))");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:\n1"), std::runtime_error, "invalid line number (converting '' to integer failed - not an integer (invalid_argument))");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:#1"), std::runtime_error, "invalid line number (converting '' to integer failed - not an integer (invalid_argument))"); // TODO: looks like a valid filename
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c://1"), std::runtime_error, "invalid line number (converting '' to integer failed - not an integer (invalid_argument))");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:zero"), std::runtime_error, "invalid line number (converting 'zero' to integer failed - not an integer (invalid_argument))");

        // invalid extras
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:1\n"), std::runtime_error, "unexpected extra ''");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:1\nsym=x"), std::runtime_error, "unexpected extra 'sym=x'");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:1\nsymbol:x"), std::runtime_error, "unexpected extra 'symbol:x'");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:1\npolyspace=0"), std::runtime_error, "unexpected extra 'polyspace=0'");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c:1\npolyspace:1"), std::runtime_error, "unexpected extra 'polyspace:1'");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id\n:"), std::runtime_error, "unexpected extra ':'");
        ASSERT_THROW_EQUALS(SuppressionList::parseLine("id:1.c\n:"), std::runtime_error, "unexpected extra ':'");
    }

    void suppressionsBadId1() const {
        SuppressionList suppressions;
        std::istringstream s1("123");
        ASSERT_EQUALS("Failed to add suppression. Invalid id \"123\"", suppressions.parseFile(s1));

        std::istringstream s2("obsoleteFunctionsrand_r");
        ASSERT_EQUALS("", suppressions.parseFile(s2));
    }

    static SuppressionList::ErrorMessage errorMessage(const std::string &errorId) {
        SuppressionList::ErrorMessage ret;
        ret.errorId = errorId;
        ret.hash = 0;
        ret.lineNumber = 0;
        ret.certainty = Certainty::normal;
        return ret;
    }

    static SuppressionList::ErrorMessage errorMessage(const std::string &errorId, const std::string &file, int line) {
        SuppressionList::ErrorMessage ret;
        ret.errorId = errorId;
        ret.setFileName(file);
        ret.lineNumber = line;
        return ret;
    }

    void suppressionsDosFormat() const {
        SuppressionList suppressions;
        std::istringstream s("abc\r\n"
                             "def\r\n");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("abc")));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("def")));
    }

    void suppressionsFileNameWithColon() const {
        SuppressionList suppressions;
        std::istringstream s("errorid:c:\\foo.cpp\n"
                             "errorid:c:\\bar.cpp:12");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "c:/foo.cpp", 1111)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "c:/bar.cpp", 10)));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "c:/bar.cpp", 12)));
    }

    void suppressionsGlob() const {
        // Check for syntax errors in glob
        {
            SuppressionList suppressions;
            std::istringstream s("errorid:*?.cpp\n");
            ASSERT_EQUALS("Failed to add suppression. Invalid glob pattern '*?.cpp'.", suppressions.parseFile(s));
        }

        // Check that globbing works
        {
            SuppressionList suppressions;
            std::istringstream s("errorid:x*.cpp\n"
                                 "errorid:y?.cpp\n"
                                 "errorid:test.c*\n"
                                 "errorid:dir/**\n"
                                 "errorid:**/abc**/xyz-??*.?pp\n");
            ASSERT_EQUALS("", suppressions.parseFile(s));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "xyz.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "xyz.cpp.cpp", 1)));
            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "abc.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "ya.cpp", 1)));
            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "y.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "test.c", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "test.cpp", 1)));

            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "dir/test.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "dir/deep/nested/test.cpp", 1)));
            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "yellow.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "foo/sub/abc/sub/xyz-22aaa.cpp", 1)));
            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "foo/sub/abc-xyz-22.cpp", 1)));
            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "x/abcxyz-11.cpp", 1)));
        }

        // Check that both a filename match and a glob match apply
        {
            SuppressionList suppressions;
            std::istringstream s("errorid:x*.cpp\n"
                                 "errorid:xyz.cpp:1\n"
                                 "errorid:a*.cpp:1\n"
                                 "errorid:abc.cpp:2");
            ASSERT_EQUALS("", suppressions.parseFile(s));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "xyz.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "xyz.cpp", 2)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "abc.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "abc.cpp", 2)));
        }
    }

    void suppressionsGlobId() const {
        SuppressionList suppressions;
        std::istringstream s("a*\n");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("abc", "xyz.cpp", 1)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("def", "xyz.cpp", 1)));
    }

    void suppressionsFileNameWithExtraPath() const {
        // Ticket #2797
        SuppressionList suppressions;
        ASSERT_EQUALS("", suppressions.addSuppressionLine("errorid:./a.c:123"));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "a.c", 123)));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "x/../a.c", 123)));
    }

    unsigned int checkSuppressionFiles(const char code[], const std::string &suppression = "") {
        return _checkSuppression(code, false, suppression);
    }

    unsigned int checkSuppressionFS(const char code[], const std::string &suppression = "") {
        return _checkSuppression(code, true, suppression);
    }

    // Check the suppression
    unsigned int _checkSuppression(const char code[], bool useFS, const std::string &suppression = "") {
        std::map<std::string, std::string> files;
        files["test.cpp"] = code;

        return _checkSuppression(files, useFS, suppression);
    }

    unsigned int checkSuppressionFiles(std::map<std::string, std::string> &f, const std::string &suppression = "") {
        return _checkSuppression(f, false, suppression);
    }

    unsigned int checkSuppressionFS(std::map<std::string, std::string> &f, const std::string &suppression = "") {
        return _checkSuppression(f, true, suppression);
    }

    // Check the suppression for multiple files
    unsigned int _checkSuppression(std::map<std::string, std::string> &f, bool useFS, const std::string &suppression = "") {
        std::list<FileSettings> fileSettings;

        std::list<FileWithDetails> filelist;
        for (auto i = f.cbegin(); i != f.cend(); ++i) {
            filelist.emplace_back(i->first, Standards::Language::CPP, i->second.size());
            if (useFS) {
                fileSettings.emplace_back(i->first, Standards::Language::CPP, i->second.size());
            }
        }

        Suppressions supprs;
        if (!suppression.empty()) {
            ASSERT_EQUALS("", supprs.nomsg.addSuppressionLine(suppression));
        }

        Settings settings;
        settings.jobs = 1;
        settings.quiet = true;
        settings.inlineSuppressions = true;
        settings.severity.enable(Severity::information);
        if (suppression == "unusedFunction")
            settings.checks.setEnabled(Checks::unusedFunction, true);
        settings.templateFormat = templateFormat;

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filelist.size());
        for (auto i = f.cbegin(); i != f.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, i->second));

        // clear files list so only fileSettings are used
        if (useFS)
            filelist.clear();

        CppCheck cppCheck(settings, supprs, *this, nullptr, true, nullptr);
        SingleExecutor executor(cppCheck, filelist, fileSettings, settings, supprs, *this, nullptr);
        unsigned int exitCode = executor.check();

        const bool err = CppCheckExecutorTest::reportUnmatchedSuppressions(settings, supprs.nomsg, filelist, fileSettings, *this);
        if (err && exitCode == 0)
            exitCode = 1;

        return exitCode;
    }

#ifdef HAS_THREADING_MODEL_THREAD
    unsigned int checkSuppressionThreadsFiles(const char code[], const std::string &suppression = "") {
        return _checkSuppressionThreads(code, false, suppression);
    }

    unsigned int checkSuppressionThreadsFS(const char code[], const std::string &suppression = "") {
        return _checkSuppressionThreads(code, true, suppression);
    }

    unsigned int _checkSuppressionThreads(const char code[], bool useFS, const std::string &suppression = "") {
        std::list<FileSettings> fileSettings;

        std::list<FileWithDetails> filelist;
        filelist.emplace_back("test.cpp", Standards::Language::CPP, strlen(code));
        if (useFS) {
            fileSettings.emplace_back("test.cpp", Standards::Language::CPP, strlen(code));
        }

        const auto settings = dinit(Settings,
                                    $.jobs = 2,
                                        $.quiet = true,
                                        $.inlineSuppressions = true,
                                        $.severity.enable (Severity::information),
                                        $.templateFormat = templateFormat);

        Suppressions supprs;
        if (!suppression.empty()) {
            ASSERT_EQUALS("", supprs.nomsg.addSuppressionLine(suppression));
        }

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filelist.size());
        for (auto i = filelist.cbegin(); i != filelist.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->path(), code));

        // clear files list so only fileSettings are used
        if (useFS)
            filelist.clear();

        ThreadExecutor executor(filelist, fileSettings, settings, supprs, *this, nullptr, CppCheckExecutorTest::executeCommand);
        unsigned int exitCode = executor.check();

        const bool err = CppCheckExecutorTest::reportUnmatchedSuppressions(settings, supprs.nomsg, filelist, fileSettings, *this);
        if (err && exitCode == 0)
            exitCode = 1;

        return exitCode;
    }
#endif // HAS_THREADING_MODEL_THREAD

#ifdef HAS_THREADING_MODEL_FORK
    unsigned int checkSuppressionProcessesFiles(const char code[], const std::string &suppression = "") {
        return _checkSuppressionProcesses(code, false, suppression);
    }

    unsigned int checkSuppressionProcessesFS(const char code[], const std::string &suppression = "") {
        return _checkSuppressionProcesses(code, true, suppression);
    }

    unsigned int _checkSuppressionProcesses(const char code[], bool useFS, const std::string &suppression = "") {
        std::list<FileSettings> fileSettings;

        std::list<FileWithDetails> filelist;
        filelist.emplace_back("test.cpp", Standards::Language::CPP, strlen(code));
        if (useFS) {
            fileSettings.emplace_back("test.cpp", Standards::Language::CPP, strlen(code));
        }

        /*const*/ auto settings = dinit(Settings,
                                        $.jobs = 2,
                                            $.quiet = true,
                                            $.inlineSuppressions = true);
        settings.severity.enable(Severity::information);
        settings.templateFormat = templateFormat;

        Suppressions supprs;
        if (!suppression.empty()) {
            ASSERT_EQUALS("", supprs.nomsg.addSuppressionLine(suppression));
        }

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filelist.size());
        for (auto i = filelist.cbegin(); i != filelist.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->path(), code));

        // clear files list so only fileSettings are used
        if (useFS)
            filelist.clear();

        ProcessExecutor executor(filelist, fileSettings, settings, supprs, *this, nullptr, CppCheckExecutorTest::executeCommand);
        unsigned int exitCode = executor.check();

        const bool err = CppCheckExecutorTest::reportUnmatchedSuppressions(settings, supprs.nomsg, filelist, fileSettings, *this);
        if (err && exitCode == 0)
            exitCode = 1;

        return exitCode;
    }
#endif // HAS_THREADING_MODEL_FORK

    void runChecks(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        // check to make sure the appropriate errors are present
        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:3:5]: (error) Uninitialized variable: a [uninitvar]\n"
                      "[test.cpp:5:5]: (error) Uninitialized variable: b [uninitvar]\n", errout_str());

        // suppress uninitvar globally
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "uninitvar"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    // cppcheck-suppress-file uninitvar\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:2:5]: (error) File suppression should be at the top of the file [invalidSuppression]\n"
                      "[test.cpp:4:5]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n"
                                        "// cppcheck-suppress-file uninitvar\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:5:1]: (error) File suppression should be at the top of the file [invalidSuppression]\n"
                      "[test.cpp:3:5]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        ASSERT_EQUALS(0, (this->*check)("// cppcheck-suppress-file uninitvar\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(0, (this->*check)("/* Fake file description\n"
                                        " * End\n"
                                        " */\n"
                                        "\n"
                                        "// cppcheck-suppress-file uninitvar\n"
                                        "\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(0, (this->*check)("// cppcheck-suppress-file uninitvar\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar globally, without error present
        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    b++;\n"
                                        "}\n",
                                        "uninitvar"));
        ASSERT_EQUALS("(information) Unmatched suppression: uninitvar [unmatchedSuppression]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("// cppcheck-suppress-file uninitvar\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:1:1]: (information) Unmatched suppression: uninitvar [unmatchedSuppression]\n", errout_str());

        // suppress uninitvar for this file only
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "uninitvar:test.cpp"));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar for this file only, without error present
        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    b++;\n"
                                        "}\n",
                                        "uninitvar:test.cpp"));
        ASSERT_EQUALS("[test.cpp:0:0]: (information) Unmatched suppression: uninitvar [unmatchedSuppression]\n", errout_str());

        // suppress all for this file only
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "*:test.cpp"));
        ASSERT_EQUALS("", errout_str());

        // suppress all for this file only, without error present
        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    b++;\n"
                                        "}\n",
                                        "*:test.cpp"));
        ASSERT_EQUALS("[test.cpp:0:0]: (information) Unmatched suppression: * [unmatchedSuppression]\n", errout_str());

        // suppress uninitvar for this file and line
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "uninitvar:test.cpp:3"));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar for this file and line, without error present
        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    b++;\n"
                                        "}\n",
                                        "uninitvar:test.cpp:3"));
        ASSERT_EQUALS("[test.cpp:3:0]: (information) Unmatched suppression: uninitvar [unmatchedSuppression]\n", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;// cppcheck-suppress uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress uninitvar */\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress uninitvar */\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;/* cppcheck-suppress uninitvar */\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress[uninitvar]\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress[uninitvar]\n"
                                        "    a++;\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;// cppcheck-suppress[uninitvar]\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress[uninitvar]*/\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress[uninitvar]*/\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;/* cppcheck-suppress[uninitvar]*/\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline, with asm before (#6813)
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    __asm {\n"
                                        "        foo\n"
                                        "    }"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    a++;\n"
                                        "}",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline, without error present
        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:4:5]: (information) Unmatched suppression: uninitvar [unmatchedSuppression]\n", errout_str());

        // suppress block inline checks
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:2:5]: (error) Suppress Begin: No matching end [invalidSuppression]\n"
                      "[test.cpp:4:5]: (error) Uninitialized variable: a [uninitvar]\n"
                      "[test.cpp:6:5]: (error) Uninitialized variable: b [uninitvar]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:6:5]: (error) Suppress End: No matching begin [invalidSuppression]\n"
                      "[test.cpp:3:5]: (error) Uninitialized variable: a [uninitvar]\n"
                      "[test.cpp:5:5]: (error) Uninitialized variable: b [uninitvar]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("// cppcheck-suppress: id\n"
                                        "// cppcheck-suppress-unknown id\n"
                                        "// cppcheck-suppress-begin-unknown id\n"
                                        "// cppcheck-suppress-begin id4\n"
                                        "void f() {}\n"
                                        "// cppcheck-suppress-end-unknown id4\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:1:1]: (error) unknown suppression type 'cppcheck-suppress:' [invalidSuppression]\n"
                      "[test.cpp:2:1]: (error) unknown suppression type 'cppcheck-suppress-unknown' [invalidSuppression]\n"
                      "[test.cpp:3:1]: (error) unknown suppression type 'cppcheck-suppress-begin-unknown' [invalidSuppression]\n"
                      "[test.cpp:6:1]: (error) unknown suppression type 'cppcheck-suppress-end-unknown' [invalidSuppression]\n"
                      "[test.cpp:4:1]: (error) Suppress Begin: No matching end [invalidSuppression]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("// cppcheck-suppress-file\n"
                                        "// cppcheck-suppress\n"
                                        "// cppcheck-suppress \n"
                                        "// cppcheck-suppress\t\n"
                                        "// cppcheck-suppress []\n" // TODO
                                        "// cppcheck-suppress-macro\n"
                                        "// cppcheck-suppress-begin\n"
                                        "// cppcheck-suppress-begin id0\n"
                                        "void f() {}\n"
                                        "// cppcheck-suppress-end\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:1:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:2:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:3:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:4:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:6:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:7:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:10:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:8:1]: (error) Suppress Begin: No matching end [invalidSuppression]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("// cppcheck-suppress:\n"
                                        "// cppcheck-suppress-unknown\n"
                                        "// cppcheck-suppress-begin-unknown\n"
                                        "// cppcheck-suppress-begin\n"
                                        "void f() {}\n"
                                        "// cppcheck-suppress-end-unknown\n",
                                        ""));
        // TODO: actually these are all invalid types
        ASSERT_EQUALS("[test.cpp:1:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:2:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:3:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:4:1]: (error) suppression without error ID [invalidSuppression]\n"
                      "[test.cpp:6:1]: (error) suppression without error ID [invalidSuppression]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Uninitialized variable: b [uninitvar]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Uninitialized variable: b [uninitvar]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin[uninitvar]\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end[uninitvar]\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Uninitialized variable: b [uninitvar]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin [uninitvar]\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end [uninitvar]\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:7:5]: (error) Uninitialized variable: b [uninitvar]\n", errout_str());

        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    int b;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    // cppcheck-suppress-begin [uninitvar]\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    int b;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    // cppcheck-suppress-end [uninitvar]\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    // cppcheck-suppress-begin [uninitvar, syntaxError]\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    int b;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    // cppcheck-suppress-end [uninitvar, syntaxError]\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:2:5]: (information) Unmatched suppression: syntaxError [unmatchedSuppression]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("// cppcheck-suppress-begin [uninitvar, syntaxError]\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    int b;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n"
                                        "// cppcheck-suppress-end [uninitvar, syntaxError]\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:1:1]: (information) Unmatched suppression: syntaxError [unmatchedSuppression]\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("// cppcheck-suppress-begin [uninitvar, syntaxError]\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    int b;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n"
                                        "// cppcheck-suppress-end [uninitvar, syntaxError]",
                                        ""));
        ASSERT_EQUALS("[test.cpp:1:1]: (information) Unmatched suppression: syntaxError [unmatchedSuppression]\n", errout_str());

        // test of multiple suppression types
        ASSERT_EQUALS(0, (this->*check)("// cppcheck-suppress-file uninitvar\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(0, (this->*check)("// cppcheck-suppress-file uninitvar\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(0, (this->*check)("// cppcheck-suppress-file uninitvar\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(1, (this->*check)("// cppcheck-suppress-file uninitvar\n"
                                        "void f() {\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:4:5]: (information) Unmatched suppression: uninitvar [unmatchedSuppression]\n", errout_str());

        // #5746 - exitcode
        ASSERT_EQUALS(1U,
                      (this->*check)("int f() {\n"
                                     "  int a; return a;\n"
                                     "}\n",
                                     ""));
        ASSERT_EQUALS("[test.cpp:2:17]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());

        ASSERT_EQUALS(0U,
                      (this->*check)("int f() {\n"
                                     "  int a; return a;\n"
                                     "}\n",
                                     "uninitvar"));
        ASSERT_EQUALS("", errout_str());

        // TODO: check result
        (this->*check)("// cppcheck-suppress-macro zerodiv\n"
                       "#define DIV(A,B) A/B\n"
                       "a = DIV(10,0);\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        // TODO: check result
        (this->*check)("// cppcheck-suppress-macro abc\n"
                       "#define DIV(A,B) A/B\n"
                       "a = DIV(10,1);\n",
                       "");
        ASSERT_EQUALS("[test.cpp:2:1]: (information) Unmatched suppression: abc [unmatchedSuppression]\n", errout_str());
    }

    void suppressionsSettingsFiles() {
        runChecks(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressionsSettingsFS() {
        runChecks(&TestSuppressions::checkSuppressionFS);
    }

#ifdef HAS_THREADING_MODEL_THREAD
    void suppressionsSettingsThreadsFiles() {
        runChecks(&TestSuppressions::checkSuppressionThreadsFiles);
    }

    void suppressionsSettingsThreadsFS() {
        runChecks(&TestSuppressions::checkSuppressionThreadsFS);
    }
#endif // HAS_THREADING_MODEL_THREAD

#ifdef HAS_THREADING_MODEL_FORK
    void suppressionsSettingsProcessesFiles() {
        runChecks(&TestSuppressions::checkSuppressionProcessesFiles);
    }

    void suppressionsSettingsProcessesFS() {
        runChecks(&TestSuppressions::checkSuppressionProcessesFS);
    }
#endif // HAS_THREADING_MODEL_FORK

    void suppressionsMultiFileInternal(unsigned int (TestSuppressions::*check)(std::map<std::string, std::string> &f, const std::string &)) {
        std::map<std::string, std::string> files;
        files["abc.cpp"] = "void f() {\n"
                           "}\n";
        files["xyz.cpp"] = "void f() {\n"
                           "    int a;\n"
                           "    a++;\n"
                           "}\n";

        // suppress uninitvar for this file and line
        ASSERT_EQUALS(0, (this->*check)(files, "uninitvar:xyz.cpp:3"));
        ASSERT_EQUALS("", errout_str());
    }

    void suppressionsMultiFileFiles() {
        suppressionsMultiFileInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressionsMultiFileFS() {
        suppressionsMultiFileInternal(&TestSuppressions::checkSuppressionFS);
    }

    // TODO: this tests an internal function - should it be private?
    void suppressionsPathSeparator() const {
        const SuppressionList::Suppression s1("*", "test/foo/*");
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::Matched, s1.isSuppressed(errorMessage("someid", "test/foo/bar.cpp", 142)));

        const SuppressionList::Suppression s2("abc", "include/1.h");
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::Matched, s2.isSuppressed(errorMessage("abc", "include/1.h", 142)));
    }

    void suppressionsLine0() const {
        SuppressionList suppressions;
        ASSERT_EQUALS("", suppressions.addSuppressionLine("syntaxError:*:0"));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("syntaxError", "test.cpp", 0)));
    }

    void suppressionsFileComment() const {
        std::istringstream file1("# comment\n"
                                 "abc");
        SuppressionList suppressions1;
        ASSERT_EQUALS("", suppressions1.parseFile(file1));
        ASSERT_EQUALS(true, suppressions1.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file2("// comment\n"
                                 "abc");
        SuppressionList suppressions2;
        ASSERT_EQUALS("", suppressions2.parseFile(file2));
        ASSERT_EQUALS(true, suppressions2.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file3("abc // comment");
        SuppressionList suppressions3;
        ASSERT_EQUALS("", suppressions3.parseFile(file3));
        ASSERT_EQUALS(true, suppressions3.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file4("abc\t\t # comment");
        SuppressionList suppressions4;
        ASSERT_EQUALS("", suppressions4.parseFile(file4));
        ASSERT_EQUALS(true, suppressions4.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file5("abc:test.cpp\t\t # comment");
        SuppressionList suppressions5;
        ASSERT_EQUALS("", suppressions5.parseFile(file5));
        ASSERT_EQUALS(true, suppressions5.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file6("abc:test.cpp:123\t\t # comment with . inside");
        SuppressionList suppressions6;
        ASSERT_EQUALS("", suppressions6.parseFile(file6));
        ASSERT_EQUALS(true, suppressions6.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file7(" // comment\n" // #11450
                                 "abc");
        SuppressionList suppressions7;
        ASSERT_EQUALS("", suppressions7.parseFile(file7));
        ASSERT_EQUALS(true, suppressions7.isSuppressed(errorMessage("abc", "test.cpp", 123)));
    }

    void inlinesuppress() const {
        SuppressionList::Suppression s;
        std::string msg;

        // Suppress without attribute
        ASSERT_EQUALS(false, s.parseComment("/* some text */", &msg));
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress */", &msg));
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress-file  */", &msg));
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress-begin */", &msg));
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress-end */", &msg));

        // Correct suppress
        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress id */", &msg));
        ASSERT_EQUALS("", msg);
        ASSERT_EQUALS("", s.extraComment);

        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress id ; extra */", &msg));
        ASSERT_EQUALS("", msg);
        ASSERT_EQUALS("extra", s.extraComment);

        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress id // extra */", &msg));
        ASSERT_EQUALS("", msg);
        ASSERT_EQUALS("extra", s.extraComment);

        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-file id */", &msg));
        ASSERT_EQUALS("", msg);

        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-begin id */", &msg));
        ASSERT_EQUALS("", msg);

        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-end id */", &msg));
        ASSERT_EQUALS("", msg);

        // Bad cppcheck-suppress comment
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress-beggin id */", &msg));

        // Bad attribute construction
        const std::string badSuppressionAttribute = "Bad suppression attribute 'some'. You can write comments in the comment after a ; or //. Valid suppression attributes; symbolName=sym";

        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress id some text */", &msg));
        ASSERT_EQUALS(badSuppressionAttribute, msg);
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-file id some text */", &msg));
        ASSERT_EQUALS(badSuppressionAttribute, msg);
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-begin id some text */", &msg));
        ASSERT_EQUALS(badSuppressionAttribute, msg);
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-end id some text */", &msg));
        ASSERT_EQUALS(badSuppressionAttribute, msg);
    }

    void inlinesuppress_symbolname_Internal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress uninitvar symbolName=a */\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a,b;\n"
                                        "    /* cppcheck-suppress uninitvar symbolName=b */\n"
                                        "    a++; b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:4:5]: (error) Uninitialized variable: a [uninitvar]\n", errout_str());
    }

    void inlinesuppress_symbolname_Files() {
        inlinesuppress_symbolname_Internal(&TestSuppressions::checkSuppressionFiles);
    }

    void inlinesuppress_symbolname_FS() {
        inlinesuppress_symbolname_Internal(&TestSuppressions::checkSuppressionFS);
    }

    void inlinesuppress_comment() const {
        SuppressionList::Suppression s;
        std::string errMsg;
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc ; some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc // some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc -- some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
    }

    // TODO: tests internal function - should it be private?
    void inlinesuppress_unchecked() const {
        SuppressionList::Suppression s;
        std::string errMsg;
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc", &errMsg));
        ASSERT_EQUALS("", errMsg);
        s.lineNumber = 5;

        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::None, s.isSuppressed(errorMessage("id", "test.cpp", 11)));
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::Checked, s.isSuppressed(errorMessage("id", "test.cpp", 5)));
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::Matched, s.isSuppressed(errorMessage("abc", "test.cpp", 5)));
    }

    void multi_inlinesuppress() const {
        std::vector<SuppressionList::Suppression> suppressions;
        std::string errMsg;

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-begin[errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-begin [errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-end[errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-end [errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-file[errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-file [errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId symbolName=arr]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("arr", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId symbolName=]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId1, errorId2 symbolName=arr]", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS("errorId1", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("errorId2", suppressions[1].errorId);
        ASSERT_EQUALS("arr", suppressions[1].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress errorId", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId1 errorId2 symbolName=arr]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId1, errorId2 symbol=arr]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId1, errorId2 symbolName]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());
    }

    void multi_inlinesuppress_comment() const {
        std::vector<SuppressionList::Suppression> suppressions;
        std::string errMsg;

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("//cppcheck-suppress[errorId1, errorId2 symbolName=arr]", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("//cppcheck-suppress[errorId1, errorId2 symbolName=arr] some text", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("/*cppcheck-suppress[errorId1, errorId2 symbolName=arr]*/", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());
    }

    void globalSuppressions() { // Testing that Cppcheck::useGlobalSuppressions works (#8515)
        const auto settings = dinit(Settings,
                                    $.quiet = true,
                                        $.exitCode = 1,
                                        $.templateFormat = templateFormat);

        Suppressions supprs;
        ASSERT_EQUALS("", supprs.nomsg.addSuppressionLine("uninitvar"));

        CppCheck cppCheck(settings, supprs, *this, nullptr, false, nullptr); // <- do not "use global suppressions". pretend this is a thread that just checks a file.

        const char code[] = "int f() { int a; return a; }";
        ASSERT_EQUALS(0, cppCheck.checkBuffer(FileWithDetails("test.c", Standards::Language::C, 0),code, sizeof(code))); // <- no unsuppressed error is seen
        ASSERT_EQUALS("[test.c:1:25]: (error) Uninitialized variable: a [uninitvar]\n", errout_str()); // <- report error so ThreadExecutor can suppress it and make sure the global suppression is matched.
    }

    void inlinesuppress_unusedFunction() const { // #4210, #4946 - wrong report of "unmatchedSuppression" for "unusedFunction"
        SuppressionList suppressions;
        SuppressionList::Suppression suppression("unusedFunction", "test.c", 3);
        suppression.checked = true; // have to do this because fixes for #5704
        ASSERT_EQUALS("", suppressions.addSuppression(std::move(suppression)));
        ASSERT_EQUALS(true, !suppressions.getUnmatchedLocalSuppressions(FileWithDetails("test.c", Standards::Language::C, 0)).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedGlobalSuppressions().empty());
    }

    void globalsuppress_unusedFunction() const { // #4946 - wrong report of "unmatchedSuppression" for "unusedFunction"
        SuppressionList suppressions;
        ASSERT_EQUALS("", suppressions.addSuppressionLine("unusedFunction:*"));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid")));
        ASSERT_EQUALS(false, !suppressions.getUnmatchedLocalSuppressions(FileWithDetails("test.c", Standards::Language::C, 0)).empty());
        ASSERT_EQUALS(true, !suppressions.getUnmatchedGlobalSuppressions().empty());
    }

    void suppressionWithRelativePaths() {
        Suppressions supprs;

        Settings settings;
        settings.quiet = true;
        settings.severity.enable(Severity::style);
        settings.inlineSuppressions = true;
        settings.relativePaths = true;
        settings.basePaths.emplace_back("/somewhere");
        settings.templateFormat = templateFormat;
        const char code[] =
            "struct Point\n"
            "{\n"
            "    // cppcheck-suppress unusedStructMember\n"
            "    int x;\n"
            "    // cppcheck-suppress unusedStructMember\n"
            "    int y;\n"
            "};";
        CppCheck cppCheck(settings, supprs, *this, nullptr, true, nullptr);
        ASSERT_EQUALS(0, cppCheck.checkBuffer(FileWithDetails("/somewhere/test.cpp", Standards::Language::CPP, 0), code, sizeof(code)));
        ASSERT_EQUALS("",errout_str());
    }

    void suppressingSyntaxErrorsInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) { // syntaxErrors should be suppressible (#7076)
        const char code[] = "if if\n";

        ASSERT_EQUALS(0, (this->*check)(code, "syntaxError:test.cpp:1"));
        ASSERT_EQUALS("", errout_str());
    }

    void suppressingSyntaxErrorsFiles() {
        suppressingSyntaxErrorsInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSyntaxErrorsFS() {
        suppressingSyntaxErrorsInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSyntaxErrorsInlineInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) { // syntaxErrors should be suppressible (#5917)
        const char code[] = "double result(0.0);\n"
                            "_asm\n"
                            "{\n"
                            "   // cppcheck-suppress syntaxError\n"
                            "   push  EAX               ; save EAX for callers\n"
                            "   mov   EAX,Real10        ; get the address pointed to by Real10\n"
                            "   fld   TBYTE PTR [EAX]   ; load an extended real (10 bytes)\n"
                            "   fstp  QWORD PTR result  ; store a double (8 bytes)\n"
                            "   pop   EAX               ; restore EAX\n"
                            "}";
        ASSERT_EQUALS(0, (this->*check)(code, ""));
        ASSERT_EQUALS("", errout_str());
    }

    void suppressingSyntaxErrorsInlineFiles() {
        suppressingSyntaxErrorsInlineInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSyntaxErrorsInlineFS() {
        suppressingSyntaxErrorsInlineInternal(&TestSuppressions::checkSuppressionFS);
    }

    void suppressingSimplecppErrorsWhileFileReadInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) { // syntaxError while file read should be suppressible (PR #1333)
        const char code[] = "CONST (genType, KS_CONST) genService[KS_CFG_NR_OF_NVM_BLOCKS] =\n"
                            "{\n"
                            "[!VAR \"BC\" = \"$BC + 1\"!][!//\n"
                            "[!IF \"(as:modconf('Ks')[1]/KsGeneral/KsType = 'KS_CFG_TYPE_KS_MASTER') and\n"
                            "      (as:modconf('Ks')[1]/KsGeneral/KsUseShe = 'true')\"!][!//\n"
                            "  {\n"
                            "      &varNB_GetErrorStatus,\n"
                            "      &varNB_WriteBlock,\n"
                            "      &varNB_ReadBlock\n"
                            "  },\n"
                            "[!VAR \"BC\" = \"$BC + 1\"!][!//\n"
                            "[!ENDIF!][!//\n"
                            "};";
        ASSERT_EQUALS(0, (this->*check)(code, "syntaxError:test.cpp:4"));
        ASSERT_EQUALS("", errout_str());
    }

    void suppressingSimplecppErrorsWhileFileReadFiles() {
        suppressingSimplecppErrorsWhileFileReadInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSimplecppErrorsWhileFileReadFS() {
        suppressingSimplecppErrorsWhileFileReadInternal(&TestSuppressions::checkSuppressionFiles);
    }

    // TODO: this tests an internal function - should it be private?
    void symbol() const {
        SuppressionList::Suppression s;
        s.fileName = "test.cpp";
        s.errorId = "foo";
        s.symbolName = "array*";

        SuppressionList::ErrorMessage errorMsg;
        errorMsg.errorId = "foo";
        errorMsg.symbolNames = "";
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::None, s.isSuppressed(errorMsg));
        errorMsg.setFileName("test.cpp");
        errorMsg.lineNumber = 123;
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::Checked, s.isSuppressed(errorMsg));
        errorMsg.symbolNames = "x\n";
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::Checked, s.isSuppressed(errorMsg));
        errorMsg.symbolNames = "array1\n";
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::Matched, s.isSuppressed(errorMsg));
        errorMsg.symbolNames = "x\n"
                               "array2\n";
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::Matched, s.isSuppressed(errorMsg));
        errorMsg.symbolNames = "array3\n"
                               "x\n";
        ASSERT_EQUALS_ENUM(SuppressionList::Suppression::Result::Matched, s.isSuppressed(errorMsg));
    }

    void unusedFunctionInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        ASSERT_EQUALS(0, (this->*check)("void f() {}", "unusedFunction"));
    }

    void unusedFunctionFiles() {
        unusedFunctionInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void unusedFunctionFS() {
        unusedFunctionInternal(&TestSuppressions::checkSuppressionFS);
    }

    void suppressingSyntaxErrorAndExitCodeInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        const char code[] = "fi if;";

        ASSERT_EQUALS(0, (this->*check)(code, "*:test.cpp"));
        ASSERT_EQUALS("", errout_str());

        // TODO: this test never worked
        // multi error in file, but only suppression one error
        //const char code2[] = "fi fi\n"
        //                     "if if;";
        //ASSERT_EQUALS(1, (this->*check)(code2, "*:test.cpp:1"));  // suppress all error at line 1 of test.cpp
        //ASSERT_EQUALS("[test.cpp:2]: (error) syntax error\n", errout_str());

        // multi error in file, but only suppression one error (2)
        const char code3[] = "void f(int x, int y){\n"
                             "    int a = x/0;\n"
                             "    int b = y/0;\n"
                             "}\n"
                             "f(0, 1);\n";
        ASSERT_EQUALS(1, (this->*check)(code3, "zerodiv:test.cpp:3"));  // suppress 'zerodiv' at line 3 of test.cpp
        ASSERT_EQUALS("[test.cpp:2:14]: (error) Division by zero. [zerodiv]\n", errout_str());
    }

    void suppressingSyntaxErrorAndExitCodeFiles() {
        suppressingSyntaxErrorAndExitCodeInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSyntaxErrorAndExitCodeFS() {
        suppressingSyntaxErrorAndExitCodeInternal(&TestSuppressions::checkSuppressionFS);
    }

    void suppressingSyntaxErrorAndExitCodeMultiFileInternal(unsigned int (TestSuppressions::*check)(std::map<std::string, std::string> &f, const std::string &)) {
        // multi files, but only suppression one
        std::map<std::string, std::string> mfiles;
        mfiles["test.cpp"] = "fi if;";
        mfiles["test2.cpp"] = "fi if";
        ASSERT_EQUALS(1, (this->*check)(mfiles, "*:test.cpp"));
        ASSERT_EQUALS("[test2.cpp:1:4]: (error) syntax error [syntaxError]\n", errout_str());
    }

    void suppressingSyntaxErrorAndExitCodeMultiFileFiles() {
        suppressingSyntaxErrorAndExitCodeMultiFileInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSyntaxErrorAndExitCodeMultiFileFS() {
        suppressingSyntaxErrorAndExitCodeMultiFileInternal(&TestSuppressions::checkSuppressionFS);
    }

    void suppressLocal() const {
        SuppressionList suppressions;
        std::istringstream s("errorid:test.cpp\n"
                             "errorid2");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "test.cpp", 1)));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "test.cpp", 1), false));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "test2.cpp", 1)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "test2.cpp", 1), false));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid2", "test.cpp", 1)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid2", "test.cpp", 1), false));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid2", "test2.cpp", 1)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid2", "test2.cpp", 1), false));
    }

    static void addCheckedSuppression(SuppressionList& supprs, SuppressionList::Suppression suppr)
    {
        suppr.checked = true;
        supprs.addSuppression(std::move(suppr));
    }

    void suppressUnmatchedSuppressions() {
        const std::list<FileWithDetails> files = { {"a.c", Standards::Language::C, 0}};
        const std::list<FileSettings> fs;

        // No unmatched suppression
        {
            SuppressionList suppressions;
            ASSERT_EQUALS(false, CppCheckExecutorTest::reportUnmatchedSuppressions(settingsDefault, suppressions, files, fs, *this));
            ASSERT_EQUALS("", errout_str());
        }

        // suppress all unmatchedSuppression
        {
            SuppressionList suppressions;
            addCheckedSuppression(suppressions, {"abc", "a.c", 10U});
            addCheckedSuppression(suppressions, {"unmatchedSuppression", "*", SuppressionList::Suppression::NO_LINE});
            ASSERT_EQUALS(false, CppCheckExecutorTest::reportUnmatchedSuppressions(settingsDefault, suppressions, files, fs, *this));
            ASSERT_EQUALS("", errout_str());
        }

        // suppress all unmatchedSuppression (corresponds to "--suppress=unmatchedSuppression")
        {
            SuppressionList suppressions;
            addCheckedSuppression(suppressions, {"abc", "a.c", 10U});
            addCheckedSuppression(suppressions, {"unmatchedSuppression", "", SuppressionList::Suppression::NO_LINE});
            ASSERT_EQUALS(false, CppCheckExecutorTest::reportUnmatchedSuppressions(settingsDefault, suppressions, files, fs, *this));
            ASSERT_EQUALS("", errout_str());
        }

        // suppress all unmatchedSuppression in a.c
        {
            SuppressionList suppressions;
            addCheckedSuppression(suppressions, {"abc", "a.c", 10U});
            addCheckedSuppression(suppressions, {"unmatchedSuppression", "a.c", SuppressionList::Suppression::NO_LINE});
            ASSERT_EQUALS(false, CppCheckExecutorTest::reportUnmatchedSuppressions(settingsDefault, suppressions, files, fs, *this));
            ASSERT_EQUALS("", errout_str());
        }

        // suppress unmatchedSuppression in a.c at line 10
        {
            SuppressionList suppressions;
            addCheckedSuppression(suppressions, {"abc", "a.c", 10U});
            addCheckedSuppression(suppressions, {"unmatchedSuppression", "a.c", 10U});
            ASSERT_EQUALS(false, CppCheckExecutorTest::reportUnmatchedSuppressions(settingsDefault, suppressions, files, fs, *this));
            ASSERT_EQUALS("", errout_str());
        }

        // don't suppress unmatchedSuppression when file is mismatching
        {
            SuppressionList suppressions;
            addCheckedSuppression(suppressions, {"abc", "a.c", 10U});
            addCheckedSuppression(suppressions, {"unmatchedSuppression", "b.c", SuppressionList::Suppression::NO_LINE});
            ASSERT_EQUALS(true, CppCheckExecutorTest::reportUnmatchedSuppressions(settingsDefault, suppressions, files, fs, *this));
            ASSERT_EQUALS("[a.c:10:0]: (information) Unmatched suppression: abc [unmatchedSuppression]\n", errout_str());
        }

        // don't suppress unmatchedSuppression when line is mismatching
        {
            SuppressionList suppressions;
            addCheckedSuppression(suppressions, {"abc", "a.c", 10U});
            addCheckedSuppression(suppressions, {"unmatchedSuppression", "a.c", 1U});
            ASSERT_EQUALS(true, CppCheckExecutorTest::reportUnmatchedSuppressions(settingsDefault, suppressions, files, fs, *this));
            ASSERT_EQUALS("[a.c:10:0]: (information) Unmatched suppression: abc [unmatchedSuppression]\n", errout_str());
        }
    }

    void suppressionsParseXmlFile() const {
        {
            ScopedFile file("suppressparsexml.xml",
                            "<suppressions>\n"
                            "<suppress>\n"
                            "<id>uninitvar</id>\n"
                            "<fileName>file.c</fileName>\n"
                            "<lineNumber>10</lineNumber>\n"
                            "<symbolName>sym</symbolName>\n"
                            "</suppress>\n"
                            "</suppressions>");

            SuppressionList supprList;
            ASSERT_EQUALS("", supprList.parseXmlFile(file.path().c_str()));
            const auto& supprs = supprList.getSuppressions();
            ASSERT_EQUALS(1, supprs.size());
            const auto& suppr = *supprs.cbegin();
            ASSERT_EQUALS("uninitvar", suppr.errorId);
            ASSERT_EQUALS("file.c", suppr.fileName);
            ASSERT_EQUALS(10, suppr.lineNumber);
            ASSERT_EQUALS("sym", suppr.symbolName);
        }

        // no file specified
        {
            SuppressionList supprList;
            ASSERT_EQUALS("failed to load suppressions XML '' (XML_ERROR_FILE_NOT_FOUND).", supprList.parseXmlFile(""));
        }

        // missing file
        {
            SuppressionList supprList;
            ASSERT_EQUALS("failed to load suppressions XML 'suppressparsexml.xml' (XML_ERROR_FILE_NOT_FOUND).", supprList.parseXmlFile("suppressparsexml.xml"));
        }

        // empty file
        {
            ScopedFile file("suppressparsexml.xml",
                            "");

            SuppressionList supprList;
            ASSERT_EQUALS("failed to load suppressions XML 'suppressparsexml.xml' (XML_ERROR_EMPTY_DOCUMENT).", supprList.parseXmlFile(file.path().c_str()));
        }

        // wrong root node
        {
            ScopedFile file("suppressparsexml.xml",
                            "<suppress/>\n");

            SuppressionList supprList;
            ASSERT_EQUALS("", supprList.parseXmlFile(file.path().c_str()));
        }

        // no root node
        {
            ScopedFile file("suppressparsexml.xml",
                            "<?xml version=\"1.0\"?>\n");

            SuppressionList supprList;
            ASSERT_EQUALS("failed to load suppressions XML 'suppressparsexml.xml' (no root node found).", supprList.parseXmlFile(file.path().c_str()));
        }

        // unknown element
        {
            ScopedFile file("suppressparsexml.xml",
                            "<suppressions>\n"
                            "<suppress>\n"
                            "<eid>uninitvar</eid>\n"
                            "</suppress>\n"
                            "</suppressions>");

            SuppressionList supprList;
            ASSERT_EQUALS("unknown element 'eid' in suppressions XML 'suppressparsexml.xml', expected id/fileName/lineNumber/symbolName/hash.", supprList.parseXmlFile(file.path().c_str()));
        }
    }

    void addSuppressionDuplicate() const {
        SuppressionList supprs;

        SuppressionList::Suppression s;
        s.errorId = "uninitvar";

        ASSERT_EQUALS("", supprs.addSuppression(s));
        ASSERT_EQUALS("suppression 'uninitvar' already exists", supprs.addSuppression(s));
    }

    void updateSuppressionState() const {
        {
            SuppressionList supprs;

            SuppressionList::Suppression s;
            s.errorId = "uninitVar";
            ASSERT_EQUALS(false, supprs.updateSuppressionState(s));
        }
        {
            SuppressionList supprs;

            SuppressionList::Suppression s;
            s.errorId = "uninitVar";

            ASSERT_EQUALS("", supprs.addSuppression(s));

            ASSERT_EQUALS(true, supprs.updateSuppressionState(s));

            const std::list<SuppressionList::Suppression> l = supprs.getUnmatchedGlobalSuppressions();
            ASSERT_EQUALS(1, l.size());
        }
        {
            SuppressionList supprs;

            SuppressionList::Suppression s;
            s.errorId = "uninitVar";
            s.matched = false;

            ASSERT_EQUALS("", supprs.addSuppression(s));

            s.matched = true;
            ASSERT_EQUALS(true, supprs.updateSuppressionState(s));

            const std::list<SuppressionList::Suppression> l = supprs.getUnmatchedGlobalSuppressions();
            ASSERT_EQUALS(0, l.size());
        }
    }

    void addSuppressionLineMultiple() const {
        SuppressionList supprlist;

        ASSERT_EQUALS("", supprlist.addSuppressionLine("syntaxError"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("uninitvar:1.c"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("memleak:1.c"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("uninitvar:2.c"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("memleak:3.c:12 # first"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("memleak:3.c:22 // second"));

        const auto& supprs = supprlist.getSuppressions();
        ASSERT_EQUALS(6, supprs.size());

        auto it = supprs.cbegin();

        ASSERT_EQUALS("syntaxError", it->errorId);
        ASSERT_EQUALS("", it->fileName);
        ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, it->lineNumber);
        ++it;

        ASSERT_EQUALS("uninitvar", it->errorId);
        ASSERT_EQUALS("1.c", it->fileName);
        ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, it->lineNumber);
        ++it;

        ASSERT_EQUALS("memleak", it->errorId);
        ASSERT_EQUALS("1.c", it->fileName);
        ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, it->lineNumber);
        ++it;

        ASSERT_EQUALS("uninitvar", it->errorId);
        ASSERT_EQUALS("2.c", it->fileName);
        ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, it->lineNumber);
        ++it;

        ASSERT_EQUALS("memleak", it->errorId);
        ASSERT_EQUALS("3.c", it->fileName);
        ASSERT_EQUALS(12, it->lineNumber);
        ++it;

        ASSERT_EQUALS("memleak", it->errorId);
        ASSERT_EQUALS("3.c", it->fileName);
        ASSERT_EQUALS(22, it->lineNumber);
    }

    void toString() const
    {
        {
            SuppressionList::Suppression s;
            s.errorId = "unitvar";
            ASSERT_EQUALS("unitvar", s.toString());
        }
        {
            SuppressionList::Suppression s;
            s.errorId = "unitvar";
            s.fileName = "test.cpp";
            ASSERT_EQUALS("unitvar:test.cpp", s.toString());
        }
        {
            SuppressionList::Suppression s;
            s.errorId = "unitvar";
            s.fileName = "test.cpp";
            s.lineNumber = 12;
            ASSERT_EQUALS("unitvar:test.cpp:12", s.toString());
        }
        {
            SuppressionList::Suppression s;
            s.errorId = "unitvar";
            s.symbolName = "sym";
            ASSERT_EQUALS("unitvar\nsymbol=sym", s.toString());
        }
    }

    void suppressionFromErrorMessage() const {
        {
            const ErrorMessage msg({}, "test1.cpp", Severity::information, "msg", "id", Certainty::inconclusive);
            const auto msg_s = SuppressionList::ErrorMessage::fromErrorMessage(msg, {"m1", "m2"});
            ASSERT_EQUALS("test1.cpp", msg_s.getFileName());
            ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, msg_s.lineNumber);
            ASSERT_EQUALS("id", msg_s.errorId);
            ASSERT_EQUALS_ENUM(Certainty::inconclusive, msg_s.certainty);
            ASSERT_EQUALS("", msg_s.symbolNames);
            ASSERT_EQUALS(2, msg_s.macroNames.size());
            auto it = msg_s.macroNames.cbegin();
            ASSERT_EQUALS("m1", *it);
            ++it;
            ASSERT_EQUALS("m2", *it);
        }
        {
            std::list<ErrorMessage::FileLocation> loc;
            loc.emplace_back("test1.cpp", 1, 1);
            const ErrorMessage msg(std::move(loc), "test1.cpp", Severity::information, "msg", "id", Certainty::normal);
            const auto msg_s = SuppressionList::ErrorMessage::fromErrorMessage(msg, {});
            ASSERT_EQUALS("test1.cpp", msg_s.getFileName());
            ASSERT_EQUALS(1, msg_s.lineNumber);
        }
        {
            std::list<ErrorMessage::FileLocation> loc;
            loc.emplace_back("test1.cpp", 1, 1);
            loc.emplace_back("test2.cpp", 2, 2);
            loc.emplace_back("test3.cpp", 3, 3);
            const ErrorMessage msg(std::move(loc), "test1.cpp", Severity::information, "msg", "id", Certainty::normal);
            const auto msg_s = SuppressionList::ErrorMessage::fromErrorMessage(msg, {});
            ASSERT_EQUALS("test3.cpp", msg_s.getFileName());
            ASSERT_EQUALS(3, msg_s.lineNumber);
        }
    }

    void suppressionWildcard() const {
        {
            SuppressionList suppressions;
            ASSERT_EQUALS("", suppressions.addSuppressionLine("id:test*.cpp"));

            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("abc", "xyz.cpp", 1)));
            {
                const auto supprs = suppressions.getSuppressions();
                const auto suppr = supprs.cbegin();
                ASSERT(!suppr->checked);
                ASSERT(!suppr->matched);
            }
            ASSERT(suppressions.getUnmatchedGlobalSuppressions().empty());
        }

        {
            SuppressionList suppressions;
            ASSERT_EQUALS("", suppressions.addSuppressionLine("id:test*.cpp"));

            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("abc", "test.cpp", 1)));
            {
                const auto supprs = suppressions.getSuppressions();
                const auto suppr = supprs.cbegin();
                ASSERT(suppr->checked);
                ASSERT(!suppr->matched);
            }
            ASSERT(!suppressions.getUnmatchedGlobalSuppressions().empty());
        }

        {
            SuppressionList suppressions;
            ASSERT_EQUALS("", suppressions.addSuppressionLine("id:test*.cpp"));

            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("id2", "test.cpp", 1)));
            {
                const auto supprs = suppressions.getSuppressions();
                const auto suppr = supprs.cbegin();
                ASSERT(suppr->checked);
                ASSERT(!suppr->matched);
            }
            ASSERT(!suppressions.getUnmatchedGlobalSuppressions().empty());
        }

        {
            SuppressionList suppressions;
            ASSERT_EQUALS("", suppressions.addSuppressionLine("id:test*.cpp"));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("id", "test.cpp", 1)));
            {
                const auto supprs = suppressions.getSuppressions();
                const auto suppr = supprs.cbegin();
                ASSERT(suppr->checked);
                ASSERT(suppr->matched);
            }
            ASSERT(suppressions.getUnmatchedGlobalSuppressions().empty());
        }

        {
            SuppressionList suppressions;
            ASSERT_EQUALS("", suppressions.addSuppressionLine("*:test*.cpp"));
            // the empty ID should be disallowed but it is use as a hack to mark wildcards on error IDs as checked
            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("", "test.cpp", 1)));
            {
                const auto supprs = suppressions.getSuppressions();
                const auto suppr = supprs.cbegin();
                ASSERT(suppr->checked);
                ASSERT(!suppr->matched);
            }
            ASSERT(!suppressions.getUnmatchedGlobalSuppressions().empty());
        }
    }

    static std::string polyspaceParseIdsResults(const std::string& comment, std::string::size_type pos) {
        std::string ret;
        for (const auto& fr: polyspace::Parser::parseFamilyRules(comment,pos)) {
            if (!fr.second.empty())
                ret += ',' + fr.first + ':' + fr.second;
        }
        return ret.empty() ? ret : ret.substr(1);
    }

    void polyspaceParseRange() const {
        std::string::size_type pos;

        // Happy case
        pos = 0;
        ASSERT_EQUALS(12, polyspace::Parser::parseRange(" +12 ",pos));
        ASSERT_EQUALS(4U, pos);

        // Invalid range => pos will point at the token
        pos = 0;
        ASSERT_EQUALS(0, polyspace::Parser::parseRange(" ",pos));
        ASSERT_EQUALS(std::string::npos, pos);

        pos = 0;
        ASSERT_EQUALS(0, polyspace::Parser::parseRange(" test ",pos));
        ASSERT_EQUALS(1U, pos);

        pos = 0;
        ASSERT_EQUALS(0, polyspace::Parser::parseRange(" +",pos));
        ASSERT_EQUALS(1U, pos);

        pos = 0;
        ASSERT_EQUALS(0, polyspace::Parser::parseRange(" +12",pos));
        ASSERT_EQUALS(1U, pos);

        pos = 0;
        ASSERT_EQUALS(0, polyspace::Parser::parseRange(" +A ",pos));
        ASSERT_EQUALS(1U, pos);
    }

    void polyspaceParseIds() const {
        ASSERT_EQUALS("test:12", polyspaceParseIdsResults("abc test:12",3));
        ASSERT_EQUALS("test:12", polyspaceParseIdsResults("abc test:12 [12]",3));
        ASSERT_EQUALS("test:12", polyspaceParseIdsResults("abc test:12 */",3));
        ASSERT_EQUALS("test:1*", polyspaceParseIdsResults("abc test:1* */",3));
        ASSERT_EQUALS("test:*", polyspaceParseIdsResults("// abc test:*",6));
        // -_.
        ASSERT_EQUALS("test:1-2.3", polyspaceParseIdsResults("abc test:1-2.3",3));
        ASSERT_EQUALS("t-e_s.t:12", polyspaceParseIdsResults("abc t-e_s.t : 12",3));
        // multiple ids
        ASSERT_EQUALS("d:1,d:2", polyspaceParseIdsResults("abc d:1,2",3));
        ASSERT_EQUALS("d:1,d:2,e:3,e:4,f:6", polyspaceParseIdsResults("abc d:1,2 e: 3 , 4 f : 6 ",3));
    }

    struct PolyspaceComment {
        std::string text;
        int line;

        PolyspaceComment(const std::string &&text, int line)
            : text(text)
            , line(line)
        {}
    };

    struct PolyspaceParseResult {
        std::string errorId;
        int lineNumber;
        std::string extraComment;
        SuppressionList::Type type = SuppressionList::Type::unique;
        int lineBegin = SuppressionList::Suppression::NO_LINE;
        int lineEnd = SuppressionList::Suppression::NO_LINE;

        PolyspaceParseResult(const std::string &&errorId,
                             int lineNumber,
                             const std::string &&extraComment = "",
                             SuppressionList::Type type = SuppressionList::Type::unique,
                             int lineBegin = SuppressionList::Suppression::NO_LINE,
                             int lineEnd = SuppressionList::Suppression::NO_LINE)
            : errorId(errorId)
            , lineNumber(lineNumber)
            , extraComment(extraComment)
            , type(type)
            , lineBegin(lineBegin)
            , lineEnd(lineEnd)
        {}
    };

    void testPolyspaceSuppression(const std::string& addon,
                                  const std::string& premiumArgs,
                                  const PolyspaceComment& comment,
                                  std::initializer_list<PolyspaceParseResult> results) const
    {
        SuppressionList list;
        Settings settings;
        if (!addon.empty()) {
            AddonInfo info;
            info.name = addon;
            settings.addonInfos.push_back(info);
        }
        settings.premiumArgs = premiumArgs;
        polyspace::Parser parser(settings);

        const std::string fileName = "file.c";
        const auto supprs = parser.parse(comment.text, comment.line, fileName);

        ASSERT_EQUALS(results.size(), supprs.size());

        auto supprIt = supprs.cbegin();
        const auto *resultIt = results.begin();

        for (; supprIt != supprs.cend(); supprIt++, resultIt++) {
            ASSERT(supprIt->isPolyspace);
            ASSERT(supprIt->isInline);
            ASSERT_EQUALS(fileName, supprIt->fileName);
            ASSERT_EQUALS(resultIt->errorId, supprIt->errorId);
            ASSERT_EQUALS(resultIt->extraComment, supprIt->extraComment);
            ASSERT_EQUALS_ENUM(resultIt->type, supprIt->type);
            ASSERT_EQUALS(resultIt->lineNumber, supprIt->lineNumber);
            ASSERT_EQUALS(resultIt->lineBegin, supprIt->lineBegin);
            ASSERT_EQUALS(resultIt->lineEnd, supprIt->lineEnd);
        }
    }

    void polyspaceMisraC2012() const {
        testPolyspaceSuppression(
            "misra", "",
            { "/* polyspace MISRA2012 : 2.7 */", 1 },
            { { "misra-c2012-2.7", 1 } }
            );
    }

    void polyspacePremiumMisraC2012() const {
        testPolyspaceSuppression(
            "", "--misra-c-2012",
            { "/* polyspace MISRA2012 : 2.7 */", 1 },
            { { "premium-misra-c-2012-2.7", 1 } }
            );
    }

    void polyspaceMisraC2023() const {
        testPolyspaceSuppression(
            "", "--misra-c-2023",
            { "// polyspace MISRA-C-2023 : *", 2 },
            { { "premium-misra-c-2023-*", 2 } }
            );
    }

    void polyspaceMisraCpp2008() const {
        testPolyspaceSuppression(
            "", "--misra-cpp-2008",
            { "// polyspace MISRA-CPP : 7-1-1", 1 },
            { { "premium-misra-cpp-2008-7-1-1", 1 } }
            );
    }

    void polyspaceMisraCpp2023() const {
        testPolyspaceSuppression(
            "", "--misra-cpp-2023",
            { "// polyspace MISRA-CPP-2023 : 4.6.1", 1 },
            { { "premium-misra-cpp-2023-4.6.1", 1 } }
            );
    }

    void polyspaceCertC() const {
        testPolyspaceSuppression(
            "", "--cert-c",
            { "// polyspace CERT-C : PRE30", 1 },
            { { "premium-cert-c-PRE30", 1 } }
            );
    }

    void polyspaceCertCpp() const {
        testPolyspaceSuppression(
            "", "--cert-cpp",
            { "// polyspace CERT-CPP : CTR51", 1 },
            { { "premium-cert-cpp-CTR51", 1 } }
            );
    }

    void polyspaceAutosar() const {
        testPolyspaceSuppression(
            "", "--autosar",
            { "// polyspace AUTOSAR-CPP14 : a2-10-1", 1 },
            { { "premium-autosar-a2-10-1", 1 } }
            );
    }

    void polyspaceIgnored() const {
        testPolyspaceSuppression(
            "", "",
            { "// polyspace DEFECT : INT_OVFL AUTOSAR-CPP14 : a2-10-1", 1 },
            {}
            );
    }

    void polyspaceMultiple1() const {
        testPolyspaceSuppression(
            "", "--misra-c-2012",
            { "/* polyspace MISRA2012 : 2.7, 9.1 */", 1 },
            { { "premium-misra-c-2012-2.7", 1 },
                { "premium-misra-c-2012-9.1", 1 } }
            );
    }

    void polyspaceMultiple2() const {
        testPolyspaceSuppression(
            "", "--misra-c-2012 --misra-cpp-2008",
            { "/* polyspace MISRA2012 : 2.7 MISRA-CPP : 7-1-1 */", 1 },
            { { "premium-misra-c-2012-2.7", 1 },
                { "premium-misra-cpp-2008-7-1-1", 1 } }
            );
    }

    void polyspaceMultiple3() const {
        testPolyspaceSuppression(
            "", "--misra-c-2012 --misra-cpp-2008",
            { "/* polyspace MISRA2012 : 2.7 [Justified:Low] \"comment 1\" polyspace MISRA-CPP : 7-1-1 \"comment 2\"*/", 1 },
            { { "premium-misra-c-2012-2.7", 1, "comment 1" },
                { "premium-misra-cpp-2008-7-1-1", 1, "comment 2" }, }
            );
    }

    void polyspaceRange() const {
        testPolyspaceSuppression(
            "", "--misra-c-2012",
            { "/* polyspace +3 MISRA2012 : 2.7 */", 1 },
            { { "premium-misra-c-2012-2.7", 1, "", SuppressionList::Type::block, 1, 4 } }
            );
    }

    void polyspaceBlock() const {
        testPolyspaceSuppression(
            "", "--misra-c-2012",
            { "/* polyspace-begin MISRA2012 : 2.7 */", 1 },
            { { "premium-misra-c-2012-2.7", 1, "", SuppressionList::Type::blockBegin } }
            );

        testPolyspaceSuppression(
            "", "--misra-c-2012",
            { "/* polyspace-end MISRA2012 : 2.7 */", 1 },
            { { "premium-misra-c-2012-2.7", 1, "", SuppressionList::Type::blockEnd } }
            );

    }

    void polyspaceExtraComments() const {
        testPolyspaceSuppression(
            "", "--misra-c-2012 --misra-cpp-2008",
            { "/* polyspace MISRA2012 : 2.7 MISRA-CPP : 7-1-1 \"comment 1\" polyspace MISRA2012 : 8.1, 8.3 \"comment 2\" */", 1 },
            { { "premium-misra-c-2012-2.7", 1, "comment 1" },
                { "premium-misra-cpp-2008-7-1-1", 1, "comment 1" },
                { "premium-misra-c-2012-8.1", 1, "comment 2" },
                { "premium-misra-c-2012-8.3", 1, "comment 2" }, }
            );
    }
};

REGISTER_TEST(TestSuppressions)
