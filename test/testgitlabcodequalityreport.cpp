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

#include "gitlabcodequalityreport.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "fixture.h"

#include <cstdint>
#include <string>

#include "json.h"

class TestGitlabCodeQualityReport : public TestFixture
{
public:
    TestGitlabCodeQualityReport() : TestFixture("TestGitlabCodeQualityReport")
    {}

private:
    void run() override
    {
        TEST_CASE(emptyReport);
        TEST_CASE(singleError);
        TEST_CASE(multipleErrors);
        TEST_CASE(errorWithoutLocation);
        TEST_CASE(severityMapping);
        TEST_CASE(fingerprint);
        TEST_CASE(pathWithoutDotSlash);
    }

    static ErrorMessage createErrorMessage(const std::string& id,
                                           Severity severity,
                                           const std::string& msg,
                                           const std::string& file = "test.cpp",
                                           int line = 10)
    {
        ErrorMessage::FileLocation loc(file, line, 1);
        return ErrorMessage({loc}, file, severity, msg, id, Certainty::normal);
    }

    static bool parseJson(const std::string& data, picojson::value& root)
    {
        const std::string error = picojson::parse(root, data);
        return error.empty() && root.is<picojson::array>();
    }

    static const picojson::object& findingAt(const picojson::value& root, std::size_t index)
    {
        return root.get<picojson::array>()[index].get<picojson::object>();
    }

    std::string severityOf(Severity severity)
    {
        GitlabCodeQualityReport report;
        report.addFinding(createErrorMessage("testError", severity, "Test error"));
        picojson::value json;
        ASSERT(parseJson(report.serialize(), json));
        return findingAt(json, 0).at("severity").get<std::string>();
    }

    void emptyReport()
    {
        GitlabCodeQualityReport report;
        picojson::value json;
        ASSERT(parseJson(report.serialize(), json));
        ASSERT_EQUALS(0U, json.get<picojson::array>().size());
    }

    void singleError()
    {
        GitlabCodeQualityReport report;
        report.addFinding(createErrorMessage("nullPointer", Severity::error, "Null pointer dereference", "src/test.cpp", 42));

        picojson::value json;
        ASSERT(parseJson(report.serialize(), json));
        ASSERT_EQUALS(1U, json.get<picojson::array>().size());

        const picojson::object& finding = findingAt(json, 0);
        ASSERT_EQUALS("Null pointer dereference", finding.at("description").get<std::string>());
        ASSERT_EQUALS("nullPointer", finding.at("check_name").get<std::string>());
        ASSERT_EQUALS("critical", finding.at("severity").get<std::string>());
        ASSERT(!finding.at("fingerprint").get<std::string>().empty());

        const picojson::object& location = finding.at("location").get<picojson::object>();
        ASSERT_EQUALS("src/test.cpp", location.at("path").get<std::string>());
        const picojson::object& lines = location.at("lines").get<picojson::object>();
        ASSERT_EQUALS(42, static_cast<int>(lines.at("begin").get<double>()));
    }

    void multipleErrors()
    {
        GitlabCodeQualityReport report;
        report.addFinding(createErrorMessage("error1", Severity::error, "Error 1", "file1.cpp", 10));
        report.addFinding(createErrorMessage("error2", Severity::warning, "Error 2", "file2.cpp", 20));
        report.addFinding(createErrorMessage("error3", Severity::style, "Error 3", "file3.cpp", 30));

        picojson::value json;
        ASSERT(parseJson(report.serialize(), json));
        ASSERT_EQUALS(3U, json.get<picojson::array>().size());
    }

    void errorWithoutLocation()
    {
        GitlabCodeQualityReport report;
        report.addFinding(ErrorMessage({}, "test.cpp", Severity::error, "Error without location", "testError", Certainty::normal));

        picojson::value json;
        ASSERT(parseJson(report.serialize(), json));
        ASSERT_EQUALS(0U, json.get<picojson::array>().size());
    }

    void severityMapping()
    {
        ASSERT_EQUALS("critical", severityOf(Severity::error));
        ASSERT_EQUALS("major", severityOf(Severity::warning));
        ASSERT_EQUALS("minor", severityOf(Severity::style));
        ASSERT_EQUALS("minor", severityOf(Severity::performance));
        ASSERT_EQUALS("minor", severityOf(Severity::portability));
        ASSERT_EQUALS("info", severityOf(Severity::information));
        ASSERT_EQUALS("info", severityOf(Severity::debug));
        ASSERT_EQUALS("info", severityOf(Severity::none));

        GitlabCodeQualityReport report;
        report.addFinding(createErrorMessage("syntaxError", Severity::style, "Syntax error"));
        picojson::value json;
        ASSERT(parseJson(report.serialize(), json));
        ASSERT_EQUALS("critical", findingAt(json, 0).at("severity").get<std::string>());
    }

    void fingerprint()
    {
        ErrorMessage errorMessage = createErrorMessage("testError", Severity::error, "Test error");
        errorMessage.hash = 12345;
        GitlabCodeQualityReport report;
        report.addFinding(errorMessage);

        picojson::value json;
        ASSERT(parseJson(report.serialize(), json));
        ASSERT_EQUALS("12345", findingAt(json, 0).at("fingerprint").get<std::string>());
    }

    void pathWithoutDotSlash()
    {
        GitlabCodeQualityReport report;
        report.addFinding(createErrorMessage("testError", Severity::error, "Test error", "./src/test.cpp", 10));

        picojson::value json;
        ASSERT(parseJson(report.serialize(), json));
        const picojson::object& location = findingAt(json, 0).at("location").get<picojson::object>();
        ASSERT_EQUALS("src/test.cpp", location.at("path").get<std::string>());
    }
};

REGISTER_TEST(TestGitlabCodeQualityReport)
