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

#include "fixture.h"

#include "errortypes.h"
#include "helpers.h"
#include "library.h"
#include "options.h"
#include "redirect.h"
#include "timer.h"

#include <algorithm>
#include <cstdio>
#include <cctype>
#include <exception>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "xml.h"

/**
 * TestRegistry
 **/
namespace {
    struct CompareFixtures {
        bool operator()(const TestInstance* lhs, const TestInstance* rhs) const {
            return lhs->classname < rhs->classname;
        }
    };
}
using TestSet = std::set<TestInstance*, CompareFixtures>;
namespace {
    class TestRegistry {
        TestSet _tests;
    public:

        static TestRegistry &theInstance() {
            static TestRegistry testreg;
            return testreg;
        }

        void addTest(TestInstance *t) {
            _tests.insert(t);
        }

        const TestSet &tests() const {
            return _tests;
        }
    };
}

TestInstance::TestInstance(const char * _name)
    : classname(_name)
{
    TestRegistry::theInstance().addTest(this);
}


/**
 * TestFixture
 **/

std::ostringstream TestFixture::errmsg;
unsigned int TestFixture::countTests;

std::size_t TestFixture::fails_counter = 0;
std::size_t TestFixture::todos_counter = 0;
std::size_t TestFixture::succeeded_todos_counter = 0;

TestFixture::TestFixture(const char * const _name)
    : classname(_name)
{}

TestFixture::~TestFixture() = default;

bool TestFixture::prepareTest(const char testname[])
{
    mTemplateFormat.clear();
    mTemplateLocation.clear();

    prepareTestInternal();

    // Check if tests should be executed
    if (!testsToRun.empty()) {
        const bool match = testsToRun.count(testname);
        if ((match && exclude_tests) || (!match && !exclude_tests))
            return false;
    }

    // Tests will be executed - prepare them
    mTestname = testname;
    ++countTests;
    std::string fullTestName =  classname + "::" + mTestname;
    if (quiet_tests) {
        std::putchar('.'); // Use putchar to write through redirection of std::cout/cerr
        std::fflush(stdout);
    } else {
        std::cout << fullTestName << std::endl;
    }
    if (timer_results)
        mTimer.reset(new Timer(fullTestName, ShowTime::TOP5_SUMMARY, timer_results));
    return !dry_run;
}

void TestFixture::teardownTest()
{
    teardownTestInternal();

    if (mTimer)
        mTimer->stop();

    {
        const std::string s = errout_str();
        if (!s.empty())
            throw std::runtime_error("unconsumed ErrorLogger err: " + s);
    }
    {
        const std::string s = output_str();
        if (!s.empty())
            throw std::runtime_error("unconsumed ErrorLogger out: " + s);
    }
}

std::string TestFixture::getLocationStr(const char * const filename, const unsigned int linenr) const
{
    return std::string(filename) + ':' + std::to_string(linenr) + '(' + classname + "::" + mTestname + ')';
}

static std::string writestr(const std::string &str, bool gccStyle = false)
{
    std::ostringstream ostr;
    if (gccStyle)
        ostr << '\"';
    for (auto i = str.cbegin(); i != str.cend(); ++i) {
        if (*i == '\n') {
            ostr << "\\n";
            if ((i+1) != str.end() && !gccStyle)
                ostr << std::endl;
        } else if (*i == '\t')
            ostr << "\\t";
        else if (*i == '\"')
            ostr << "\\\"";
        else if (std::isprint(static_cast<unsigned char>(*i)))
            ostr << *i;
        else
            ostr << "\\x" << std::hex << short{*i};
    }
    if (!str.empty() && !gccStyle)
        ostr << std::endl;
    else if (gccStyle)
        ostr << '\"';
    return ostr.str();
}

void TestFixture::assert_(const char * const filename, const unsigned int linenr, const bool condition, const std::string& msg) const
{
    if (!condition) {
        ++fails_counter;
        errmsg << getLocationStr(filename, linenr) << ": Assertion failed." << std::endl << "_____" << std::endl;
        if (!msg.empty())
            errmsg << "Hint:" << std::endl << msg << std::endl;
        throw AssertFailedError();
    }
}

void TestFixture::assertFailure(const char* const filename, const unsigned int linenr, const std::string& expected, const std::string& actual, const std::string& msg) const
{
    ++fails_counter;
    errmsg << getLocationStr(filename, linenr) << ": Assertion failed. " << std::endl
           << "Expected: " << std::endl
           << writestr(expected) << std::endl
           << "Actual: " << std::endl
           << writestr(actual) << std::endl;
    if (!msg.empty())
        errmsg << "Hint:" << std::endl << msg << std::endl;
    errmsg << "_____" << std::endl;
    throw AssertFailedError();
}

void TestFixture::assertEquals(const char * const filename, const unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg) const
{
    if (expected != actual) {
        assertFailure(filename, linenr, expected, actual, msg);
    }
}

std::string TestFixture::deleteLineNumber(const std::string &message)
{
    std::string result(message);
    // delete line number in "...:NUMBER:..."
    std::string::size_type pos = 0;
    while ((pos = result.find(':', pos)) != std::string::npos) {
        // get number
        if (pos + 1 == result.find_first_of("0123456789", pos + 1)) {
            const std::string::size_type after = result.find_first_not_of("0123456789", pos + 1);
            if (after != std::string::npos
                && result.at(after) == ':') {
                // erase NUMBER
                result.erase(pos + 1, after - pos - 1);
                pos = after;
            } else {
                ++pos;
            }
        } else {
            ++pos;
        }
    }
    return result;
}

void TestFixture::assertEqualsWithoutLineNumbers(const char * const filename, const unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg) const
{
    assertEquals(filename, linenr, deleteLineNumber(expected), deleteLineNumber(actual), msg);
}

void TestFixture::assertEquals(const char * const filename, const unsigned int linenr, const char expected[], const std::string& actual, const std::string &msg) const
{
    assertEquals(filename, linenr, std::string(expected), actual, msg);
}

void TestFixture::assertEquals(const char * const filename, const unsigned int linenr, const char expected[], const char actual[], const std::string &msg) const
{
    assertEquals(filename, linenr, std::string(expected), std::string(actual), msg);
}

void TestFixture::assertEquals(const char * const filename, const unsigned int linenr, const std::string& expected, const char actual[], const std::string &msg) const
{
    assertEquals(filename, linenr, expected, std::string(actual), msg);
}

void TestFixture::assertEquals(const char * const filename, const unsigned int linenr, const long long expected, const long long actual, const std::string &msg) const
{
    if (expected != actual) {
        assertEquals(filename, linenr, std::to_string(expected), std::to_string(actual), msg);
    }
}

void TestFixture::assertEqualsDouble(const char * const filename, const unsigned int linenr, const double expected, const double actual, const double tolerance, const std::string &msg) const
{
    if (expected < (actual - tolerance) || expected > (actual + tolerance)) {
        std::ostringstream ostr1;
        ostr1 << expected;
        std::ostringstream ostr2;
        ostr2 << actual;
        assertEquals(filename, linenr, ostr1.str(), ostr2.str(), msg);
    }
}

void TestFixture::todoAssertEquals(const char * const filename, const unsigned int linenr,
                                   const std::string &wanted,
                                   const std::string &current,
                                   const std::string &actual) const
{
    if (wanted == actual) {
        errmsg << getLocationStr(filename, linenr) << ": Assertion succeeded unexpectedly. "
               << "Result: " << writestr(wanted, true)  << std::endl << "_____" << std::endl;

        ++succeeded_todos_counter;
    } else {
        assertEquals(filename, linenr, current, actual);
        ++todos_counter;
    }
}

void TestFixture::todoAssertEquals(const char* const filename, const unsigned int linenr,
                                   const char wanted[],
                                   const char current[],
                                   const std::string& actual) const
{
    todoAssertEquals(filename, linenr, std::string(wanted), std::string(current), actual);
}


void TestFixture::todoAssertEquals(const char * const filename, const unsigned int linenr, const long long wanted, const long long current, const long long actual) const
{
    todoAssertEquals(filename, linenr, std::to_string(wanted), std::to_string(current), std::to_string(actual));
}

void TestFixture::assertThrow(const char * const filename, const unsigned int linenr) const
{
    ++fails_counter;
    errmsg << getLocationStr(filename, linenr) << ": Assertion succeeded. "
           << "The expected exception was thrown" << std::endl << "_____" << std::endl;
    throw AssertFailedError();
}

void TestFixture::assertThrowFail(const char * const filename, const unsigned int linenr) const
{
    ++fails_counter;
    errmsg << getLocationStr(filename, linenr) << ": Assertion failed. "
           << "The expected exception was not thrown"  << std::endl << "_____" << std::endl;
    throw AssertFailedError();
}

void TestFixture::assertNoThrowFail(const char * const filename, const unsigned int linenr, bool bailout) const
{
    std::string ex_msg;

    try {
        // cppcheck-suppress rethrowNoCurrentException
        throw;
    }
    catch (const AssertFailedError&) {
        return;
    }
    catch (const InternalError& e) {
        ex_msg = e.errorMessage;
    }
    catch (const std::exception& e) {
        ex_msg = e.what();
    }
    catch (...) {
        ex_msg = "unknown exception";
    }

    ++fails_counter;
    errmsg << getLocationStr(filename, linenr) << ": Assertion failed. "
           << "Unexpected exception was thrown: " << ex_msg << std::endl << "_____" << std::endl;
    if (bailout)
        throw AssertFailedError();
}

void TestFixture::printHelp()
{
    std::cout << "Testrunner - run Cppcheck tests\n"
        "\n"
        "Syntax:\n"
        "    testrunner [OPTIONS] [TestClass::TestCase...]\n"
        "    run all test cases:\n"
        "        testrunner\n"
        "    run all test cases in TestClass:\n"
        "        testrunner TestClass\n"
        "    run TestClass::TestCase:\n"
        "        testrunner TestClass::TestCase\n"
        "    run all test cases in TestClass1 and TestClass2::TestCase:\n"
        "        testrunner TestClass1 TestClass2::TestCase\n"
        "\n"
        "Options:\n"
        "    -q                   Do not print the test cases that have run.\n"
        "    -h, --help           Print this help.\n"
        "    -n                   Print no summaries.\n"
        "    -d                   Do not execute any tests (dry run).\n"
        "    -x                   Exclude the specified tests.\n";
}

void TestFixture::run(const std::set<std::string> &tests)
{
    testsToRun = tests;
    try {
        if (quiet_tests) {
            std::cout << '\n' << classname << ':';
            SUPPRESS;
            run();
        }
        else
            run();
    }
    catch (const InternalError& e) {
        ++fails_counter;
        errmsg << classname << "::" << mTestname << " - InternalError: " << e.errorMessage << std::endl;
    }
    catch (const std::exception& error) {
        ++fails_counter;
        errmsg << classname << "::" << mTestname << " - Exception: " << error.what() << std::endl;
    }
    catch (...) {
        ++fails_counter;
        errmsg << classname << "::" << mTestname << " - Unknown exception" << std::endl;
    }
}

void TestFixture::processOptions(const options& args)
{
    quiet_tests = args.quiet();
    dry_run = args.dry_run();
    exclude_tests = args.exclude_tests();
    exename = args.exe();
    timer_results = args.timer_results();
}

std::size_t TestFixture::runTests(const options& args)
{
    countTests = 0;
    errmsg.str("");

    const auto& which_tests = args.which_tests();
    const auto exclude_tests = args.exclude_tests();

    // TODO: bail out when given class/test is not found?
    for (TestInstance * test : TestRegistry::theInstance().tests())
    {
        std::set<std::string> tests;
        if (!which_tests.empty()) {
            const auto it = which_tests.find(test->classname);
            const bool match = it != which_tests.cend();
            if (match && exclude_tests && it->second.empty()) // only bailout when the whole fixture is excluded
                continue;
            if (!match && !exclude_tests)
                continue;
            if (match)
                tests = it->second;
        }

        TestFixture* fixture;
        const auto f = [&](){
            fixture = test->create();
        };
        // TODO: Timer::run() needs proper handling if no results should be collected
        if (args.timer_results())
            Timer::run(test->classname + " - create", ShowTime::TOP5_SUMMARY, args.timer_results(), f);
        else
            f();
        fixture->processOptions(args);
        fixture->run(tests);
    }

    if (args.summary() && !args.dry_run()) {
        std::cout << "\n\nTesting Complete\nNumber of tests: " << countTests << std::endl;
        std::cout << "Number of todos: " << todos_counter;
        if (succeeded_todos_counter > 0)
            std::cout << " (" << succeeded_todos_counter << " succeeded)";
        std::cout << std::endl;
    }
    // calling flush here, to do all output before the error messages (in case the output is buffered)
    std::cout.flush();

    if (args.summary() && !args.dry_run()) {
        std::cerr << "Tests failed: " << fails_counter << std::endl << std::endl;
    }
    std::cerr << errmsg.str();

    std::cerr.flush();
    return fails_counter + succeeded_todos_counter;
}

void TestFixture::reportOut(const std::string & outmsg, Color /*c*/)
{
    mOutput << outmsg << std::endl;
}

void TestFixture::reportErr(const ErrorMessage &msg)
{
    if (msg.severity == Severity::internal)
        return;
    if (msg.severity == Severity::information && msg.id == "normalCheckLevelMaxBranches")
        return;
    std::string errormessage;
    if (!mTemplateFormat.empty()) {
        errormessage = msg.toString(false, mTemplateFormat, mTemplateLocation);
    }
    else {
        if (!msg.callStack.empty()) {
            errormessage += ErrorLogger::callStackToString(msg.callStack, mNewTemplate);
            errormessage += ": ";
        }
        if (msg.severity != Severity::none) {
            errormessage += '(';
            errormessage += severityToString(msg.severity);
            if (msg.certainty == Certainty::inconclusive)
                errormessage += ", inconclusive";
            errormessage += ") ";
        }
        errormessage += msg.shortMessage();
        if (mNewTemplate) {
            errormessage += " [";
            errormessage += msg.id;
            errormessage += "]";
        }
    }
    mErrout << errormessage << std::endl;
}

void TestFixture::setTemplateFormat(const std::string &templateFormat)
{
    if (templateFormat == "multiline") {
        mTemplateFormat = "[{file}:{line}:{column}]: {severity}:{inconclusive:inconclusive:} {message} [{id}]";
        mTemplateLocation = "[{file}:{line}:{column}]: note: {info}";
    }
    else if (templateFormat == "simple") { // TODO: use the existing one in CmdLineParser
        mTemplateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]";
        mTemplateLocation = "";
    }
    else {
        mTemplateFormat = templateFormat;
        mTemplateLocation = "";
    }
}

TestFixture::SettingsBuilder& TestFixture::SettingsBuilder::checkLevel(Settings::CheckLevel level) {
    settings.setCheckLevel(level);
    return *this;
}

TestFixture::SettingsBuilder& TestFixture::SettingsBuilder::library(const char lib[]) {
    if (REDUNDANT_CHECK && std::find(settings.libraries.cbegin(), settings.libraries.cend(), lib) != settings.libraries.cend())
        throw std::runtime_error("redundant setting: libraries (" + std::string(lib) + ")");
    // TODO: exename is not yet set
    const Library::ErrorCode lib_error = settings.library.load(fixture.exename.c_str(), lib).errorcode;
    if (lib_error != Library::ErrorCode::OK)
        throw std::runtime_error("loading library '" + std::string(lib) + "' failed - " + std::to_string(static_cast<int>(lib_error)));
    // strip extension
    std::string lib_s(lib);
    const std::string ext(".cfg");
    const auto pos = lib_s.find(ext);
    if (pos != std::string::npos)
        lib_s.erase(pos, ext.size());
    settings.libraries.emplace_back(lib_s);
    return *this;
}

TestFixture::SettingsBuilder& TestFixture::SettingsBuilder::platform(Platform::Type type)
{
    const std::string platformStr = Platform::toString(type);

    if (REDUNDANT_CHECK && settings.platform.type == type)
        throw std::runtime_error("redundant setting: platform (" + platformStr + ")");

    std::string errstr;
    // TODO: exename is not yet set
    if (!settings.platform.set(platformStr, errstr, {fixture.exename}))
        throw std::runtime_error("platform '" + platformStr + "' not found");
    return *this;
}

TestFixture::SettingsBuilder& TestFixture::SettingsBuilder::libraryxml(const char xmldata[], std::size_t len)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError xml_error = doc.Parse(xmldata, len);
    if (tinyxml2::XML_SUCCESS != xml_error)
        throw std::runtime_error(std::string("loading library XML data failed - ") + tinyxml2::XMLDocument::ErrorIDToName(xml_error));
    const Library::ErrorCode lib_error = LibraryHelper::loadxmldoc(settings.library, doc).errorcode;
    if (lib_error != Library::ErrorCode::OK)
        throw std::runtime_error("loading library XML failed - " + std::to_string(static_cast<int>(lib_error)));
    return *this;
}
