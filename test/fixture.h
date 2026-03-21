/* -*- C++ -*-
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


#ifndef fixtureH
#define fixtureH

#include "check.h"
#include "color.h"
#include "config.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <list>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

class options;
class Tokenizer;
class Timer;
class TimerResultsIntf;

class TestFixture : public ErrorLogger {
private:
    static std::ostringstream errmsg;
    static unsigned int countTests;
    static std::size_t fails_counter;
    static std::size_t todos_counter;
    static std::size_t succeeded_todos_counter;
    std::string mTemplateFormat;
    std::string mTemplateLocation;
    std::string mTestname;

protected:
    std::string exename;
    std::set<std::string> testsToRun;
    bool quiet_tests{};
    bool dry_run{};
    bool exclude_tests{};
    TimerResultsIntf* timer_results{};
    bool mNewTemplate{};

    virtual void run() = 0;

    bool prepareTest(const char testname[]);
    virtual void prepareTestInternal() {}
    void teardownTest();
    virtual void teardownTestInternal() {}
    std::string getLocationStr(const char * filename, unsigned int linenr) const;

    class AssertFailedError : public std::exception {};

    void assert_(const char * filename, unsigned int linenr, bool condition, const std::string& msg = "") const;

    template<typename T>
    void assertEquals(const char* const filename, const unsigned int linenr, const T& expected, const T& actual, const std::string& msg = "") const {
        if (expected != actual) {
            std::ostringstream expectedStr;
            // NOLINTNEXTLINE(bugprone-unintended-char-ostream-output) - chars being treated as integers is acceptable here
            expectedStr << expected;
            std::ostringstream actualStr;
            // NOLINTNEXTLINE(bugprone-unintended-char-ostream-output) - chars being treated as integers is acceptable here
            actualStr << actual;

            assertFailure(filename, linenr, expectedStr.str(), actualStr.str(), msg);
        }
    }

    template<typename T>
    void assertEqualsEnum(const char* const filename, const unsigned int linenr, const T& expected, const T& actual, const std::string& msg = "") const {
        if (std::is_unsigned<T>())
            assertEquals(filename, linenr, static_cast<std::uint64_t>(expected), static_cast<std::uint64_t>(actual), msg);
        else
            assertEquals(filename, linenr, static_cast<std::int64_t>(expected), static_cast<std::int64_t>(actual), msg);
    }

    template<typename T>
    void todoAssertEqualsEnum(const char* const filename, const unsigned int linenr, const T& wanted, const T& current, const T& actual) const {
        if (std::is_unsigned<T>())
            todoAssertEquals(filename, linenr, static_cast<std::uint64_t>(wanted), static_cast<std::uint64_t>(current), static_cast<std::uint64_t>(actual));
        else
            todoAssertEquals(filename, linenr, static_cast<std::int64_t>(wanted), static_cast<std::int64_t>(current), static_cast<std::int64_t>(actual));
    }

    void assertEquals(const char * filename, unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg = "") const;
    void assertEqualsWithoutLineNumbers(const char * filename, unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg = "") const;
    void assertEquals(const char * filename, unsigned int linenr, const char expected[], const std::string& actual, const std::string &msg = "") const;
    void assertEquals(const char * filename, unsigned int linenr, const char expected[], const char actual[], const std::string &msg = "") const;
    void assertEquals(const char * filename, unsigned int linenr, const std::string& expected, const char actual[], const std::string &msg = "") const;
    void assertEquals(const char * filename, unsigned int linenr, long long expected, long long actual, const std::string &msg = "") const;
    void assertEqualsDouble(const char * filename, unsigned int linenr, double expected, double actual, double tolerance, const std::string &msg = "") const;

    void todoAssertEquals(const char * filename, unsigned int linenr, const std::string &wanted,
                          const std::string &current, const std::string &actual) const;
    void todoAssertEquals(const char * filename, unsigned int linenr, const char wanted[],
                          const char current[], const std::string &actual) const;
    void todoAssertEquals(const char * filename, unsigned int linenr, long long wanted,
                          long long current, long long actual) const;
    NORETURN void assertThrow(const char * filename, unsigned int linenr) const;
    NORETURN void assertThrowFail(const char * filename, unsigned int linenr) const;
    void assertNoThrowFail(const char * filename, unsigned int linenr, bool bailout) const;
    static std::string deleteLineNumber(const std::string &message);

    void setTemplateFormat(const std::string &templateFormat);

    void setMultiline() {
        setTemplateFormat("multiline");
    }

    void processOptions(const options& args);

    template<typename T>
    static T& getCheck()
    {
        for (Check *check : Check::instances()) {
            //cppcheck-suppress useStlAlgorithm
            if (T* c = dynamic_cast<T*>(check))
                return *c;
        }
        throw std::runtime_error("instance not found");
    }

    template<typename T>
    static void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
    {
        Check& check = getCheck<T>();
        check.runChecks(tokenizer, errorLogger);
    }

    class SettingsBuilder
    {
    public:
        explicit SettingsBuilder(const TestFixture &fixture) : fixture(fixture) {}
        SettingsBuilder(const TestFixture &fixture, Settings settings) : fixture(fixture), settings(std::move(settings)) {}

        SettingsBuilder& severity(Severity sev, bool b = true) {
            if (REDUNDANT_CHECK && settings.severity.isEnabled(sev) == b)
                throw std::runtime_error("redundant setting: severity - " + severityToString(sev));
            settings.severity.setEnabled(sev, b);
            return *this;
        }

        SettingsBuilder& certainty(Certainty cert, bool b = true) {
            if (REDUNDANT_CHECK && settings.certainty.isEnabled(cert) == b)
                throw std::runtime_error("redundant setting: certainty");
            settings.certainty.setEnabled(cert, b);
            return *this;
        }

        SettingsBuilder& clang() {
            if (REDUNDANT_CHECK && settings.clang)
                throw std::runtime_error("redundant setting: clang");
            settings.clang = true;
            return *this;
        }

        SettingsBuilder& checkLibrary() {
            if (REDUNDANT_CHECK && settings.checkLibrary)
                throw std::runtime_error("redundant setting: checkLibrary");
            settings.checkLibrary = true;
            return *this;
        }

        SettingsBuilder& checkUnusedTemplates(bool b = true) {
            if (REDUNDANT_CHECK && settings.checkUnusedTemplates == b)
                throw std::runtime_error("redundant setting: checkUnusedTemplates");
            settings.checkUnusedTemplates = b;
            return *this;
        }

        SettingsBuilder& debugwarnings(bool b = true) {
            if (REDUNDANT_CHECK && settings.debugwarnings == b)
                throw std::runtime_error("redundant setting: debugwarnings");
            settings.debugwarnings = b;
            return *this;
        }

        SettingsBuilder& c(Standards::cstd_t std) {
            // TODO: CLatest and C23 are the same - handle differently?
            //if (REDUNDANT_CHECK && settings.standards.c == std)
            //    throw std::runtime_error("redundant setting: standards.c");
            settings.standards.c = std;
            return *this;
        }

        SettingsBuilder& cpp(Standards::cppstd_t std) {
            // TODO: CPPLatest and CPP26 are the same - handle differently?
            //if (REDUNDANT_CHECK && settings.standards.cpp == std)
            //    throw std::runtime_error("redundant setting: standards.cpp");
            settings.standards.cpp = std;
            return *this;
        }

        SettingsBuilder& checkLevel(Settings::CheckLevel level);

        SettingsBuilder& library(const char lib[]);

        template<size_t size>
        SettingsBuilder& libraryxml(const char (&xmldata)[size]) {
            return libraryxml(xmldata, size-1);
        }

        SettingsBuilder& platform(Platform::Type type);

        SettingsBuilder& checkConfiguration() {
            if (REDUNDANT_CHECK && settings.checkConfiguration)
                throw std::runtime_error("redundant setting: checkConfiguration");
            settings.checkConfiguration = true;
            return *this;
        }

        SettingsBuilder& checkHeaders(bool b = true) {
            if (REDUNDANT_CHECK && settings.checkHeaders == b)
                throw std::runtime_error("redundant setting: checkHeaders");
            settings.checkHeaders = b;
            return *this;
        }

        Settings build() {
            return std::move(settings);
        }
    private:
        SettingsBuilder& libraryxml(const char xmldata[], std::size_t len);

        const TestFixture &fixture;
        Settings settings;

        static constexpr bool REDUNDANT_CHECK = false;
    };

    SettingsBuilder settingsBuilder() const {
        return SettingsBuilder(*this);
    }

    SettingsBuilder settingsBuilder(Settings settings) const {
        return SettingsBuilder(*this, std::move(settings));
    }

    std::string output_str() {
        std::string s = mOutput.str();
        mOutput.str("");
        return s;
    }

    std::string errout_str() {
        std::string s = mErrout.str();
        mErrout.str("");
        return s;
    }

    void ignore_errout() {
        if (errout_str().empty())
            throw std::runtime_error("no errout to ignore");
    }

    const Settings settingsDefault;

private:
    //Helper function to be called when an assertEquals assertion fails.
    //Writes the appropriate failure message to errmsg and increments fails_counter
    NORETURN void assertFailure(const char* filename, unsigned int linenr, const std::string& expected, const std::string& actual, const std::string& msg) const;

    std::ostringstream mOutput;
    std::ostringstream mErrout;

    std::unique_ptr<Timer> mTimer;

    void reportOut(const std::string &outmsg, Color c = Color::Reset) override;
    void reportErr(const ErrorMessage &msg) override;
    void reportMetric(const std::string &metric) override
    {
        (void) metric;
    }
    void run(const std::set<std::string> &tests);

public:
    static void printHelp();
    const std::string classname;

    explicit TestFixture(const char * _name);
    ~TestFixture() override;

    static std::size_t runTests(const options& args);
};

class TestInstance {
public:
    explicit TestInstance(const char * _name);
    virtual ~TestInstance() = default;

    virtual TestFixture* create() = 0;

    const std::string classname;
protected:
    std::unique_ptr<TestFixture> impl;
};

#define TEST_CASE( NAME ) do { if (prepareTest(#NAME)) { try { NAME(); teardownTest(); } catch (const AssertFailedError&) {} catch (...) { assertNoThrowFail(__FILE__, __LINE__, false); } } } while (false)

#define ASSERT( CONDITION ) assert_(__FILE__, __LINE__, (CONDITION))
#define ASSERT_MSG( CONDITION, MSG ) assert_(__FILE__, __LINE__, (CONDITION), MSG)
#define ASSERT_LOC( CONDITION, FILE_, LINE_ ) assert_(FILE_, LINE_, (CONDITION))
#define ASSERT_LOC_MSG( CONDITION, MSG, FILE_, LINE_ ) assert_(FILE_, LINE_, (CONDITION), MSG)
// *INDENT-OFF*
#define ASSERT_EQUALS( EXPECTED, ACTUAL ) do { try { assertEquals(__FILE__, __LINE__, (EXPECTED), (ACTUAL)); } catch(const AssertFailedError&) { throw; } catch (...) { assertNoThrowFail(__FILE__, __LINE__, true); } } while (false)
// *INDENT-ON*
#define ASSERT_EQUALS_WITHOUT_LINENUMBERS( EXPECTED, ACTUAL ) assertEqualsWithoutLineNumbers(__FILE__, __LINE__, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_DOUBLE( EXPECTED, ACTUAL, TOLERANCE ) assertEqualsDouble(__FILE__, __LINE__, EXPECTED, ACTUAL, TOLERANCE)
#define ASSERT_EQUALS_LOC( EXPECTED, ACTUAL, FILE_, LINE_ ) assertEquals(FILE_, LINE_, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_LOC_MSG( EXPECTED, ACTUAL, MSG, FILE_, LINE_ ) assertEquals(FILE_, LINE_, EXPECTED, ACTUAL, MSG)
#define ASSERT_EQUALS_MSG( EXPECTED, ACTUAL, MSG ) assertEquals(__FILE__, __LINE__, EXPECTED, ACTUAL, MSG)
#define ASSERT_EQUALS_ENUM( EXPECTED, ACTUAL ) assertEqualsEnum(__FILE__, __LINE__, (EXPECTED), (ACTUAL))
#define ASSERT_EQUALS_ENUM_LOC( EXPECTED, ACTUAL, FILE_, LINE_ ) assertEqualsEnum(FILE_, LINE_, (EXPECTED), (ACTUAL))
#define ASSERT_EQUALS_ENUM_LOC_MSG( EXPECTED, ACTUAL, MSG, FILE_, LINE_ ) assertEqualsEnum(FILE_, LINE_, (EXPECTED), (ACTUAL), MSG)
#define TODO_ASSERT_EQUALS_ENUM( WANTED, CURRENT, ACTUAL ) todoAssertEqualsEnum(__FILE__, __LINE__, WANTED, CURRENT, ACTUAL)
#define ASSERT_THROW_EQUALS( CMD, EXCEPTION, EXPECTED ) do { try { (void)(CMD); assertThrowFail(__FILE__, __LINE__); } catch (const AssertFailedError&) { throw; } catch (const EXCEPTION&e) { assertEquals(__FILE__, __LINE__, EXPECTED, e.what()); } catch (...) { assertThrowFail(__FILE__, __LINE__); } } while (false)
#define ASSERT_THROW_INTERNAL( CMD, TYPE ) do { try { (void)(CMD); assertThrowFail(__FILE__, __LINE__); } catch (const AssertFailedError&) { throw; } catch (const InternalError& e) { assertEqualsEnum(__FILE__, __LINE__, InternalError::TYPE, e.type); } catch (...) { assertThrowFail(__FILE__, __LINE__); } } while (false)
#define ASSERT_THROW_INTERNAL_EQUALS( CMD, TYPE, EXPECTED ) do { try { (void)(CMD); assertThrowFail(__FILE__, __LINE__); } catch (const AssertFailedError&) { throw; } catch (const InternalError& e) { assertEqualsEnum(__FILE__, __LINE__, InternalError::TYPE, e.type); assertEquals(__FILE__, __LINE__, EXPECTED, e.errorMessage); } catch (...) { assertThrowFail(__FILE__, __LINE__); } } while (false)
#define ASSERT_NO_THROW( CMD ) do { try { (void)(CMD); } catch (const AssertFailedError&) { throw; } catch (...) { assertNoThrowFail(__FILE__, __LINE__, true); } } while (false)
#define TODO_ASSERT_THROW( CMD, EXCEPTION ) do { try { (void)(CMD); } catch (const AssertFailedError&) { throw; } catch (const EXCEPTION&) {} catch (...) { assertThrow(__FILE__, __LINE__); } } while (false)
#define TODO_ASSERT( CONDITION ) do { const bool condition=(CONDITION); todoAssertEquals(__FILE__, __LINE__, true, false, condition); } while (false)
#define TODO_ASSERT_EQUALS( WANTED, CURRENT, ACTUAL ) todoAssertEquals(__FILE__, __LINE__, WANTED, CURRENT, ACTUAL)

// *INDENT-OFF*
#define REGISTER_TEST( CLASSNAME ) namespace { class CLASSNAME ## Instance : public TestInstance { public: CLASSNAME ## Instance() : TestInstance(#CLASSNAME) {} TestFixture* create() override { impl.reset(new CLASSNAME); return impl.get(); } }; CLASSNAME ## Instance instance_ ## CLASSNAME; }
// *INDENT-ON*

#define PLATFORM( P, T ) do { std::string errstr; assertEquals(__FILE__, __LINE__, true, P.set(Platform::toString(T), errstr, {exename}), errstr); } while (false)

#endif // fixtureH
