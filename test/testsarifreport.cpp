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

#include "sarifreport.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "fixture.h"

#include <cmath>
#include <set>
#include <string>
#include <vector>

#include "json.h"

class TestSarifReport : public TestFixture
{
public:
    TestSarifReport() : TestFixture("TestSarifReport")
    {}

private:
    void run() override
    {
        TEST_CASE(emptyReport);
        TEST_CASE(singleError);
        TEST_CASE(multipleErrors);
        TEST_CASE(errorWithoutLocation);
        TEST_CASE(errorWithMultipleLocations);
        TEST_CASE(differentSeverityLevels);
        TEST_CASE(securityRelatedErrors);
        TEST_CASE(cweTagsPresent);
        TEST_CASE(noCweNoSecurity);
        TEST_CASE(inconclusiveCertainty);
        TEST_CASE(criticalErrorId);
        TEST_CASE(emptyDescriptions);
        TEST_CASE(locationBoundaryValues);
        TEST_CASE(duplicateRuleIds);
        TEST_CASE(customProductName);
        TEST_CASE(versionHandling);
        TEST_CASE(securitySeverityMapping);
        TEST_CASE(versionWithSpace);
        TEST_CASE(customProductNameAndVersion);
        TEST_CASE(normalizeLineColumnToOne);
        TEST_CASE(internalAndDebugSeverity);
        TEST_CASE(problemSeverityMapping);
        TEST_CASE(mixedLocationAndNoLocation);
    }

    // Helper to create an ErrorMessage
    static ErrorMessage createErrorMessage(const std::string& id,
                                           Severity severity,
                                           const std::string& msg,
                                           const std::string& file = "test.cpp",
                                           int line                = 10,
                                           int column              = 5,
                                           int cweId               = 0,
                                           Certainty certainty     = Certainty::normal)
    {
        ErrorMessage::FileLocation loc(file, line, column);
        ErrorMessage errorMessage({loc}, file, severity, msg, id, certainty);
        if (cweId > 0)
        {
            errorMessage.cwe = CWE(cweId);
        }
        return errorMessage;
    }

    // Helper to parse JSON and validate structure
    static bool parseAndValidateJson(const std::string& json, picojson::value& root)
    {
        std::string parseError = picojson::parse(root, json);
        return parseError.empty() && root.is<picojson::object>();
    }

    void emptyReport()
    {
        SarifReport report;
        std::string sarif = report.serialize("TestProduct");

        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root = json.get<picojson::object>();
        ASSERT_EQUALS("2.1.0", root.at("version").get<std::string>());
        ASSERT(root.at("$schema").get<std::string>().find("sarif-schema-2.1.0") != std::string::npos);

        // From SARIF specification (https://docs.oasis-open.org/sarif/sarif/v2.1.0/errata01/os/sarif-v2.1.0-errata01-os-complete.html#_Toc141790730):
        // Although the order in which properties appear in a JSON object value is not semantically significant, the version property SHOULD appear first.
        ASSERT_EQUALS("{\n  \"version\": \"2.1.0\"", sarif.substr(0,22));

        const picojson::array& runs = root.at("runs").get<picojson::array>();
        ASSERT_EQUALS(1U, runs.size());

        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::array& results = cur_run.at("results").get<picojson::array>();
        ASSERT_EQUALS(0U, results.size());

        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();
        ASSERT_EQUALS(0U, rules.size());
    }

    void singleError()
    {
        SarifReport report;
        report.addFinding(createErrorMessage("nullPointer", Severity::error, "Null pointer dereference"));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root = json.get<picojson::object>();
        const picojson::array& runs  = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run  = runs[0].get<picojson::object>();

        // Check results
        const picojson::array& results = cur_run.at("results").get<picojson::array>();
        ASSERT_EQUALS(1U, results.size());

        const picojson::object& result = results[0].get<picojson::object>();
        ASSERT_EQUALS("nullPointer", result.at("ruleId").get<std::string>());
        ASSERT_EQUALS("error", result.at("level").get<std::string>());

        const picojson::object& message = result.at("message").get<picojson::object>();
        ASSERT_EQUALS("Null pointer dereference", message.at("text").get<std::string>());

        // Check rules
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();
        ASSERT_EQUALS(1U, rules.size());

        const picojson::object& rule = rules[0].get<picojson::object>();
        ASSERT_EQUALS("nullPointer", rule.at("id").get<std::string>());
    }

    void multipleErrors()
    {
        SarifReport report;
        report.addFinding(createErrorMessage("error1", Severity::error, "Error 1", "file1.cpp", 10, 5));
        report.addFinding(createErrorMessage("error2", Severity::warning, "Error 2", "file2.cpp", 20, 10));
        report.addFinding(createErrorMessage("error3", Severity::style, "Error 3", "file3.cpp", 30, 15));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root = json.get<picojson::object>();
        const picojson::array& runs  = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run  = runs[0].get<picojson::object>();

        const picojson::array& results = cur_run.at("results").get<picojson::array>();
        ASSERT_EQUALS(3U, results.size());

        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();
        ASSERT_EQUALS(3U, rules.size());
    }

    void errorWithoutLocation()
    {
        SarifReport report;

        // Create error without location (empty callStack)
        ErrorMessage errorMessage(
            {}, "test.cpp", Severity::error, "Error without location", "testError", Certainty::normal);
        report.addFinding(errorMessage);

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root = json.get<picojson::object>();
        const picojson::array& runs  = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run  = runs[0].get<picojson::object>();

        // Should have no results (GitHub doesn't support findings without locations)
        const picojson::array& results = cur_run.at("results").get<picojson::array>();
        ASSERT_EQUALS(0U, results.size());

        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();
        ASSERT_EQUALS(0U, rules.size());
    }

    void errorWithMultipleLocations()
    {
        SarifReport report;

        ErrorMessage::FileLocation loc1("test1.cpp", 10, 5);
        ErrorMessage::FileLocation loc2("test2.cpp", 20, 10);
        ErrorMessage::FileLocation loc3("test3.cpp", 30, 15);

        ErrorMessage errorMessage({loc1, loc2, loc3},
                                  "test1.cpp",
                                  Severity::error,
                                  "Error with multiple locations",
                                  "multiLocError",
                                  Certainty::normal);
        report.addFinding(errorMessage);

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::array& results = cur_run.at("results").get<picojson::array>();

        ASSERT_EQUALS(1U, results.size());

        const picojson::object& result   = results[0].get<picojson::object>();
        const picojson::array& locations = result.at("locations").get<picojson::array>();
        ASSERT_EQUALS(3U, locations.size());

        // Verify each location
        const picojson::object& loc1Obj  = locations[0].get<picojson::object>();
        const picojson::object& physLoc1 = loc1Obj.at("physicalLocation").get<picojson::object>();
        const picojson::object& region1  = physLoc1.at("region").get<picojson::object>();
        ASSERT_EQUALS(10, static_cast<int>(region1.at("startLine").get<double>()));
        ASSERT_EQUALS(5, static_cast<int>(region1.at("startColumn").get<double>()));
    }

    void differentSeverityLevels()
    {
        SarifReport report;

        report.addFinding(createErrorMessage("error1", Severity::error, "Error severity"));
        report.addFinding(createErrorMessage("warning1", Severity::warning, "Warning severity"));
        report.addFinding(createErrorMessage("style1", Severity::style, "Style severity"));
        report.addFinding(createErrorMessage("perf1", Severity::performance, "Performance severity"));
        report.addFinding(createErrorMessage("port1", Severity::portability, "Portability severity"));
        report.addFinding(createErrorMessage("info1", Severity::information, "Information severity"));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root = json.get<picojson::object>();
        const picojson::array& runs  = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run  = runs[0].get<picojson::object>();

        const picojson::array& results = cur_run.at("results").get<picojson::array>();
        ASSERT_EQUALS(6U, results.size());

        // Check severity mappings
        ASSERT_EQUALS("error", results[0].get<picojson::object>().at("level").get<std::string>());    // error
        ASSERT_EQUALS("error", results[1].get<picojson::object>().at("level").get<std::string>());    // warning
        ASSERT_EQUALS("warning", results[2].get<picojson::object>().at("level").get<std::string>());  // style
        ASSERT_EQUALS("warning", results[3].get<picojson::object>().at("level").get<std::string>());  // performance
        ASSERT_EQUALS("warning", results[4].get<picojson::object>().at("level").get<std::string>());  // portability
        ASSERT_EQUALS("note", results[5].get<picojson::object>().at("level").get<std::string>());     // information
    }

    void securityRelatedErrors()
    {
        SarifReport report;

        // Add errors with CWE IDs
        report.addFinding(createErrorMessage("nullPointer", Severity::error, "Null pointer", "test.cpp", 10, 5, 476));
        report.addFinding(
            createErrorMessage("bufferOverflow", Severity::error, "Buffer overflow", "test.cpp", 20, 5, 121));
        report.addFinding(createErrorMessage("memleak", Severity::warning, "Memory leak", "test.cpp", 30, 5, 401));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        for (const auto& rule : rules)
        {
            const picojson::object& r     = rule.get<picojson::object>();
            const picojson::object& props = r.at("properties").get<picojson::object>();

            // Should have security-severity
            ASSERT(props.find("security-severity") != props.end());

            // Should have tags with security and CWE
            ASSERT(props.find("tags") != props.end());
            const picojson::array& tags = props.at("tags").get<picojson::array>();

            bool hasSecurityTag = false;
            bool hasCweTag      = false;
            for (const auto& tag : tags)
            {
                const std::string& tagStr = tag.get<std::string>();
                if (tagStr == "security")
                    hasSecurityTag = true;
                if (tagStr.find("external/cwe/cwe-") == 0)
                    hasCweTag = true;
            }

            ASSERT(hasSecurityTag);
            ASSERT(hasCweTag);
        }
    }

    void cweTagsPresent()
    {
        SarifReport report;

        report.addFinding(createErrorMessage("testError", Severity::error, "Test error", "test.cpp", 10, 5, 119));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        ASSERT_EQUALS(1U, rules.size());

        const picojson::object& rule  = rules[0].get<picojson::object>();
        const picojson::object& props = rule.at("properties").get<picojson::object>();
        const picojson::array& tags   = props.at("tags").get<picojson::array>();

        bool foundCwe119 = false;
        for (const auto& tag : tags)
        {
            if (tag.get<std::string>() == "external/cwe/cwe-119")
                foundCwe119 = true;
        }

        ASSERT(foundCwe119);
    }

    void noCweNoSecurity()
    {
        SarifReport report;

        // Error without CWE ID should not have security properties
        report.addFinding(createErrorMessage("styleError", Severity::style, "Style error", "test.cpp", 10, 5, 0));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        ASSERT_EQUALS(1U, rules.size());

        const picojson::object& rule  = rules[0].get<picojson::object>();
        const picojson::object& props = rule.at("properties").get<picojson::object>();

        // Should NOT have security-severity
        ASSERT(props.find("security-severity") == props.end());

        // Should NOT have tags (or if present, no security tag)
        if (props.find("tags") != props.end())
        {
            const picojson::array& tags = props.at("tags").get<picojson::array>();
            for (const auto& tag : tags)
            {
                ASSERT(tag.get<std::string>() != "security");
            }
        }
    }

    void inconclusiveCertainty()
    {
        SarifReport report;

        report.addFinding(
            createErrorMessage("test1", Severity::error, "Conclusive", "test.cpp", 10, 5, 0, Certainty::normal));
        report.addFinding(createErrorMessage(
                              "test2", Severity::error, "Inconclusive", "test.cpp", 20, 5, 0, Certainty::inconclusive));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        ASSERT_EQUALS(2U, rules.size());

        // Check precision values
        const picojson::object& rule1  = rules[0].get<picojson::object>();
        const picojson::object& props1 = rule1.at("properties").get<picojson::object>();
        ASSERT_EQUALS("high", props1.at("precision").get<std::string>());

        const picojson::object& rule2  = rules[1].get<picojson::object>();
        const picojson::object& props2 = rule2.at("properties").get<picojson::object>();
        ASSERT_EQUALS("medium", props2.at("precision").get<std::string>());
    }

    void criticalErrorId()
    {
        SarifReport report;

        // Use a critical error ID (from ErrorLogger::isCriticalErrorId)
        report.addFinding(createErrorMessage("syntaxError", Severity::error, "Syntax error"));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::array& results = cur_run.at("results").get<picojson::array>();

        ASSERT_EQUALS(1U, results.size());

        const picojson::object& result = results[0].get<picojson::object>();
        // Critical errors should always map to "error" level
        ASSERT_EQUALS("error", result.at("level").get<std::string>());
    }

    void emptyDescriptions()
    {
        SarifReport report;

        report.addFinding(createErrorMessage("testError", Severity::error, "Test error"));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        ASSERT_EQUALS(1U, rules.size());

        const picojson::object& rule = rules[0].get<picojson::object>();

        // All descriptions should be empty for GitHub integration
        ASSERT_EQUALS("", rule.at("name").get<std::string>());
        ASSERT_EQUALS("", rule.at("shortDescription").get<picojson::object>().at("text").get<std::string>());
        ASSERT_EQUALS("", rule.at("fullDescription").get<picojson::object>().at("text").get<std::string>());
        ASSERT_EQUALS("", rule.at("help").get<picojson::object>().at("text").get<std::string>());
    }

    void locationBoundaryValues()
    {
        SarifReport report;

        // Test with line/column values that are 0
        // Note: Negative values don't work correctly if FileLocation uses unsigned types
        ErrorMessage::FileLocation loc1("test.cpp", 0, 0);
        ErrorMessage::FileLocation loc2("test.cpp", 1, 1);

        ErrorMessage errorMessage1({loc1}, "test.cpp", Severity::error, "Error at 0,0", "error1", Certainty::normal);
        ErrorMessage errorMessage2({loc2}, "test.cpp", Severity::error, "Error at 1,1", "error2", Certainty::normal);

        report.addFinding(errorMessage1);
        report.addFinding(errorMessage2);

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::array& results = cur_run.at("results").get<picojson::array>();

        ASSERT_EQUALS(2U, results.size());

        // Check first result (0,0 should be normalized to 1,1)
        {
            const picojson::object& res      = results[0].get<picojson::object>();
            const picojson::array& locations = res.at("locations").get<picojson::array>();
            const picojson::object& loc      = locations[0].get<picojson::object>();
            const picojson::object& physLoc  = loc.at("physicalLocation").get<picojson::object>();
            const picojson::object& region   = physLoc.at("region").get<picojson::object>();

            int line   = static_cast<int>(region.at("startLine").get<double>());
            int column = static_cast<int>(region.at("startColumn").get<double>());

            // 0 should be normalized to 1
            ASSERT_EQUALS(1, line);
            ASSERT_EQUALS(1, column);
        }

        // Check second result (1,1 should stay as 1,1)
        {
            const picojson::object& res      = results[1].get<picojson::object>();
            const picojson::array& locations = res.at("locations").get<picojson::array>();
            const picojson::object& loc      = locations[0].get<picojson::object>();
            const picojson::object& physLoc  = loc.at("physicalLocation").get<picojson::object>();
            const picojson::object& region   = physLoc.at("region").get<picojson::object>();

            int line   = static_cast<int>(region.at("startLine").get<double>());
            int column = static_cast<int>(region.at("startColumn").get<double>());

            ASSERT_EQUALS(1, line);
            ASSERT_EQUALS(1, column);
        }
    }

    void duplicateRuleIds()
    {
        SarifReport report;

        // Add multiple errors with the same rule ID
        report.addFinding(createErrorMessage("duplicateId", Severity::error, "First error", "file1.cpp", 10, 5));
        report.addFinding(createErrorMessage("duplicateId", Severity::error, "Second error", "file2.cpp", 20, 10));
        report.addFinding(createErrorMessage("duplicateId", Severity::error, "Third error", "file3.cpp", 30, 15));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root = json.get<picojson::object>();
        const picojson::array& runs  = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run  = runs[0].get<picojson::object>();

        // Should have 3 results but only 1 rule
        const picojson::array& results = cur_run.at("results").get<picojson::array>();
        ASSERT_EQUALS(3U, results.size());

        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();
        ASSERT_EQUALS(1U, rules.size());

        const picojson::object& rule = rules[0].get<picojson::object>();
        ASSERT_EQUALS("duplicateId", rule.at("id").get<std::string>());
    }

    void customProductName()
    {
        SarifReport report;
        report.addFinding(createErrorMessage("testError", Severity::error, "Test error"));

        // Test with custom product name
        std::string sarif = report.serialize("CustomChecker");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();

        // Should use "Cppcheck" as default when custom name doesn't parse
        ASSERT_EQUALS("Cppcheck", driver.at("name").get<std::string>());
    }

    void versionHandling()
    {
        SarifReport report;
        report.addFinding(createErrorMessage("testError", Severity::error, "Test error"));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();

        // Should have a semantic version
        ASSERT(driver.find("semanticVersion") != driver.end());
        const std::string version = driver.at("semanticVersion").get<std::string>();

        // Version should not contain spaces (they should be stripped)
        ASSERT(version.find(' ') == std::string::npos);
    }

    void securitySeverityMapping()
    {
        // Test the detailed security-severity mapping for different severity levels with CWE
        SarifReport report;

        // Error with CWE should get 9.9 (critical)
        report.addFinding(createErrorMessage("error1", Severity::error, "Error with CWE", "test.cpp", 10, 5, 119));

        // Warning with CWE should get 8.5 (high)
        report.addFinding(
            createErrorMessage("warning1", Severity::warning, "Warning with CWE", "test.cpp", 20, 5, 120));

        // Style/Performance/Portability with CWE should get 5.5 (medium)
        report.addFinding(createErrorMessage("style1", Severity::style, "Style with CWE", "test.cpp", 30, 5, 398));
        report.addFinding(
            createErrorMessage("perf1", Severity::performance, "Performance with CWE", "test.cpp", 40, 5, 407));
        report.addFinding(
            createErrorMessage("port1", Severity::portability, "Portability with CWE", "test.cpp", 50, 5, 562));

        // Information with CWE should get 2.0 (low)
        report.addFinding(createErrorMessage("info1", Severity::information, "Info with CWE", "test.cpp", 60, 5, 561));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        // Check each rule's security-severity value
        for (const auto& rule : rules)
        {
            const picojson::object& r     = rule.get<picojson::object>();
            const std::string& id         = r.at("id").get<std::string>();
            const picojson::object& props = r.at("properties").get<picojson::object>();

            ASSERT(props.find("security-severity") != props.end());
            const std::string& severity = props.at("security-severity").get<std::string>();
            double severityValue        = std::stod(severity);

            if (id == "error1")
            {
                // Use a tolerance for floating-point comparison to avoid warning
                ASSERT(std::abs(severityValue - 9.9) < 0.01);
            }
            else if (id == "warning1")
            {
                ASSERT(std::abs(severityValue - 8.5) < 0.01);
            }
            else if (id == "style1" || id == "perf1" || id == "port1")
            {
                ASSERT(std::abs(severityValue - 5.5) < 0.01);
            }
            else if (id == "info1")
            {
                ASSERT(std::abs(severityValue - 2.0) < 0.01);
            }
        }
    }

    void versionWithSpace()
    {
        // Test that version strings with spaces are properly truncated
        SarifReport report;
        report.addFinding(createErrorMessage("testError", Severity::error, "Test error"));

        // This test would need a way to inject a version with a space
        // The current implementation gets version from CppCheck::version()
        // This test verifies the space-trimming logic works
        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();

        const std::string& version = driver.at("semanticVersion").get<std::string>();
        // Version should not contain any spaces
        ASSERT(version.find(' ') == std::string::npos);
    }

    void customProductNameAndVersion()
    {
        // Test custom product name that includes version info
        SarifReport report;
        report.addFinding(createErrorMessage("testError", Severity::error, "Test error"));

        // Test with product name that might parse differently
        std::string sarif = report.serialize("MyChecker-1.0.0");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();

        // Should have a name (either parsed or default)
        ASSERT(driver.find("name") != driver.end());
        ASSERT(driver.find("semanticVersion") != driver.end());
    }

    void normalizeLineColumnToOne()
    {
        SarifReport report;

        // Test with 0 values
        ErrorMessage::FileLocation loc0("test.cpp", 0, 0);
        ErrorMessage errorMessage0({loc0}, "test.cpp", Severity::error, "Error at 0", "error0", Certainty::normal);
        report.addFinding(errorMessage0);

        // Test with positive values
        ErrorMessage::FileLocation locPos("test.cpp", 10, 5);
        ErrorMessage errorMessagePos(
            {locPos}, "test.cpp", Severity::error, "Error at positive", "errorPos", Certainty::normal);
        report.addFinding(errorMessagePos);

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::array& results = cur_run.at("results").get<picojson::array>();

        ASSERT_EQUALS(2U, results.size());

        // Check first result with 0,0
        const picojson::object& res0      = results[0].get<picojson::object>();
        const picojson::array& locations0 = res0.at("locations").get<picojson::array>();
        const picojson::object& loc0_obj  = locations0[0].get<picojson::object>();
        const picojson::object& physLoc0  = loc0_obj.at("physicalLocation").get<picojson::object>();
        const picojson::object& region0   = physLoc0.at("region").get<picojson::object>();

        int line0   = static_cast<int>(region0.at("startLine").get<double>());
        int column0 = static_cast<int>(region0.at("startColumn").get<double>());

        // 0 values should be normalized to 1
        ASSERT(line0 == 1);
        ASSERT(column0 == 1);

        // Check second result with positive values
        const picojson::object& res1      = results[1].get<picojson::object>();
        const picojson::array& locations1 = res1.at("locations").get<picojson::array>();
        const picojson::object& loc1_obj  = locations1[0].get<picojson::object>();
        const picojson::object& physLoc1  = loc1_obj.at("physicalLocation").get<picojson::object>();
        const picojson::object& region1   = physLoc1.at("region").get<picojson::object>();

        ASSERT_EQUALS(10, static_cast<int>(region1.at("startLine").get<double>()));
        ASSERT_EQUALS(5, static_cast<int>(region1.at("startColumn").get<double>()));
    }

    void internalAndDebugSeverity()
    {
        // Test internal and debug severity levels
        // Based on the implementation in sarifSeverity():
        // - internal -> error
        // - debug -> note
        // - none -> note
        SarifReport report;

        // Create errors with internal and debug severities
        ErrorMessage::FileLocation loc1("test.cpp", 10, 5);
        ErrorMessage::FileLocation loc2("test.cpp", 20, 10);
        ErrorMessage::FileLocation loc3("test.cpp", 30, 15);

        ErrorMessage internal(
            {loc1}, "test.cpp", Severity::internal, "Internal message", "internalError", Certainty::normal);
        ErrorMessage debug({loc2}, "test.cpp", Severity::debug, "Debug message", "debugError", Certainty::normal);
        ErrorMessage none({loc3}, "test.cpp", Severity::none, "None message", "noneError", Certainty::normal);

        report.addFinding(internal);
        report.addFinding(debug);
        report.addFinding(none);

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::array& results = cur_run.at("results").get<picojson::array>();

        ASSERT_EQUALS(3U, results.size());

        // Check the actual mapping
        const picojson::object& res0 = results[0].get<picojson::object>();
        const picojson::object& res1 = results[1].get<picojson::object>();
        const picojson::object& res2 = results[2].get<picojson::object>();

        const std::string level0 = res0.at("level").get<std::string>();
        const std::string level1 = res1.at("level").get<std::string>();
        const std::string level2 = res2.at("level").get<std::string>();

        // Actual implementation behavior:
        ASSERT_EQUALS("error", level0);  // internal -> error
        ASSERT_EQUALS("note", level1);   // debug -> note
        ASSERT_EQUALS("note", level2);   // none -> note
    }

    void problemSeverityMapping()
    {
        // Test that problem.severity property matches the SARIF severity
        SarifReport report;

        report.addFinding(createErrorMessage("error1", Severity::error, "Error"));
        report.addFinding(createErrorMessage("warning1", Severity::warning, "Warning"));
        report.addFinding(createErrorMessage("style1", Severity::style, "Style"));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        for (const auto& rule : rules)
        {
            const picojson::object& r             = rule.get<picojson::object>();
            const picojson::object& props         = r.at("properties").get<picojson::object>();
            const picojson::object& defaultConfig = r.at("defaultConfiguration").get<picojson::object>();

            // problem.severity should match defaultConfiguration.level
            const std::string& problemSeverity = props.at("problem.severity").get<std::string>();
            const std::string& defaultLevel    = defaultConfig.at("level").get<std::string>();

            ASSERT_EQUALS(defaultLevel, problemSeverity);
        }
    }

    void mixedLocationAndNoLocation()
    {
        // Test a mix of findings with and without locations
        SarifReport report;

        // Add findings with locations
        report.addFinding(createErrorMessage("withLoc1", Severity::error, "Error with location", "test.cpp", 10, 5));
        report.addFinding(
            createErrorMessage("withLoc2", Severity::warning, "Warning with location", "test.cpp", 20, 5));

        // Add findings without locations
        ErrorMessage noLoc1({}, "test.cpp", Severity::error, "Error without location", "noLoc1", Certainty::normal);
        ErrorMessage noLoc2({}, "test.cpp", Severity::warning, "Warning without location", "noLoc2", Certainty::normal);

        report.addFinding(noLoc1);
        report.addFinding(noLoc2);

        // Add more with locations
        report.addFinding(createErrorMessage("withLoc3", Severity::style, "Style with location", "test.cpp", 30, 5));

        std::string sarif = report.serialize("Cppcheck");
        picojson::value json;
        ASSERT(parseAndValidateJson(sarif, json));

        const picojson::object& root = json.get<picojson::object>();
        const picojson::array& runs  = root.at("runs").get<picojson::array>();
        const picojson::object& cur_run  = runs[0].get<picojson::object>();

        // Should only have results for findings with locations
        const picojson::array& results = cur_run.at("results").get<picojson::array>();
        ASSERT_EQUALS(3U, results.size());

        // Should only have rules for findings with locations
        const picojson::object& tool   = cur_run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();
        ASSERT_EQUALS(3U, rules.size());

        // Verify the rule IDs are only for findings with locations
        std::set<std::string> ruleIds;
        for (const auto& rule : rules)
        {
            const picojson::object& r = rule.get<picojson::object>();
            ruleIds.insert(r.at("id").get<std::string>());
        }

        ASSERT(ruleIds.find("withLoc1") != ruleIds.end());
        ASSERT(ruleIds.find("withLoc2") != ruleIds.end());
        ASSERT(ruleIds.find("withLoc3") != ruleIds.end());
        ASSERT(ruleIds.find("noLoc1") == ruleIds.end());
        ASSERT(ruleIds.find("noLoc2") == ruleIds.end());
    }
};

REGISTER_TEST(TestSarifReport)
