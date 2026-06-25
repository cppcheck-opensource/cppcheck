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

#include "cppcheck.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "settings.h"

#include <cstdint>
#include <set>
#include <sstream>
#include <utility>

static const char sarifVersion[] = "2.1.0";
static const char sarifSchema[] = "https://docs.oasis-open.org/sarif/sarif/v2.1.0/errata01/os/schemas/sarif-schema-2.1.0.json";

void SarifReport::addFinding(ErrorMessage msg)
{
    mFindings.push_back(std::move(msg));
}

picojson::array SarifReport::serializeRules() const
{
    picojson::array ret;
    std::set<std::string> ruleIds;
    for (const auto& finding : mFindings) {
        // github only supports findings with locations
        if (finding.callStack.empty())
            continue;
        if (ruleIds.insert(finding.id).second) {
            // setting name and description to empty strings will make github default
            // to the instance specific violation message and not rule description,
            // this makes it so not all the violations have the same description.
            picojson::object rule;
            rule["id"] = picojson::value(finding.id);
            // rule.name
            rule["name"] = picojson::value("");
            // rule.shortDescription.text
            picojson::object shortDescription;
            shortDescription["text"] = picojson::value("");
            rule["shortDescription"] = picojson::value(shortDescription);
            // rule.fullDescription.text
            picojson::object fullDescription;
            fullDescription["text"] = picojson::value("");
            rule["fullDescription"] = picojson::value(fullDescription);
            // rule.help.text
            picojson::object help;
            help["text"] = picojson::value("");
            rule["help"] = picojson::value(help);
            // rule.properties.precision, rule.properties.problem.severity
            picojson::object properties;
            properties["precision"] = picojson::value(sarifPrecision(finding));
            // rule.properties.security-severity, rule.properties.tags
            picojson::array tags;

            // If we have a CWE ID, treat it as security-related (CWE is the authoritative source for security weaknesses)
            if (finding.cwe.id > 0) {
                double securitySeverity = 0;
                if (finding.severity == Severity::error && !ErrorLogger::isCriticalErrorId(finding.id)) {
                    securitySeverity = 9.9; // critical = 9.0+
                }
                else if (finding.severity == Severity::warning) {
                    securitySeverity = 8.5; // high = 7.0 to 8.9
                }
                else if (finding.severity == Severity::performance || finding.severity == Severity::portability ||
                         finding.severity == Severity::style) {
                    securitySeverity = 5.5; // medium = 4.0 to 6.9
                }
                else if (finding.severity == Severity::information || finding.severity == Severity::internal ||
                         finding.severity == Severity::debug || finding.severity == Severity::none) {
                    securitySeverity = 2.0; // low = 0.1 to 3.9
                }
                if (securitySeverity > 0.0) {
                    std::ostringstream ss;
                    ss << securitySeverity;
                    properties["security-severity"] = picojson::value(ss.str());
                    tags.emplace_back("external/cwe/cwe-" + std::to_string(finding.cwe.id));
                    tags.emplace_back("security");
                }
            }

            // Add tags array if it has any content
            if (!tags.empty()) {
                properties["tags"] = picojson::value(tags);
            }

            // Set problem.severity for use with github
            const std::string problemSeverity = sarifSeverity(finding);
            properties["problem.severity"] = picojson::value(problemSeverity);
            rule["properties"] = picojson::value(properties);
            // rule.defaultConfiguration.level
            picojson::object defaultConfiguration;
            defaultConfiguration["level"] = picojson::value(sarifSeverity(finding));
            rule["defaultConfiguration"] = picojson::value(defaultConfiguration);

            ret.emplace_back(rule);
        }
    }
    return ret;
}

picojson::array SarifReport::serializeLocations(const ErrorMessage& finding)
{
    picojson::array ret;
    for (const auto& location : finding.callStack) {
        picojson::object physicalLocation;
        picojson::object artifactLocation;
        artifactLocation["uri"] = picojson::value(location.getfile(false));
        physicalLocation["artifactLocation"] = picojson::value(artifactLocation);
        picojson::object region;
        region["startLine"] = picojson::value(static_cast<int64_t>(location.line < 1 ? 1 : location.line));
        region["startColumn"] = picojson::value(static_cast<int64_t>(location.column < 1 ? 1 : location.column));
        region["endLine"] = region["startLine"];
        region["endColumn"] = region["startColumn"];
        physicalLocation["region"] = picojson::value(region);
        picojson::object loc;
        loc["physicalLocation"] = picojson::value(physicalLocation);
        ret.emplace_back(loc);
    }
    return ret;
}

picojson::array SarifReport::serializeResults() const
{
    picojson::array results;
    for (const auto& finding : mFindings) {
        // github only supports findings with locations
        if (finding.callStack.empty())
            continue;
        picojson::object res;
        res["level"] = picojson::value(sarifSeverity(finding));
        res["locations"] = picojson::value(serializeLocations(finding));
        picojson::object message;
        message["text"] = picojson::value(finding.shortMessage());
        res["message"] = picojson::value(message);
        res["ruleId"] = picojson::value(finding.id);
        if (finding.hash != 0) {
            picojson::object partialFingerprints;
            partialFingerprints["hash/v1"] = picojson::value(std::to_string(finding.hash));
            res["partialFingerprints"] = picojson::value(partialFingerprints);
        }
        results.emplace_back(res);
    }
    return results;
}

picojson::value SarifReport::serializeRuns(const std::string& productName, const std::string& version) const
{
    picojson::object driver;
    driver["name"] = picojson::value(productName);
    driver["semanticVersion"] = picojson::value(version);
    driver["informationUri"] = picojson::value("https://cppcheck.sourceforge.io");
    driver["rules"] = picojson::value(serializeRules());
    picojson::object tool;
    tool["driver"] = picojson::value(driver);
    picojson::object run;
    run["tool"] = picojson::value(tool);
    run["results"] = picojson::value(serializeResults());
    picojson::array runs{picojson::value(run)};
    return picojson::value(runs);
}

std::string SarifReport::serialize(std::string productName) const
{
    const auto nameAndVersion = Settings::getNameAndVersion(productName);
    productName = nameAndVersion.first.empty() ? "Cppcheck" : nameAndVersion.first;
    std::string version = nameAndVersion.first.empty() ? CppCheck::version() : nameAndVersion.second;
    if (version.find(' ') != std::string::npos)
        version.erase(version.find(' '), std::string::npos);

    picojson::object doc;
    doc["$schema"] = picojson::value(sarifSchema);
    doc["runs"] = serializeRuns(productName, version);

    // Insert "version" property at the start.
    // From SARIF specification (https://docs.oasis-open.org/sarif/sarif/v2.1.0/errata01/os/sarif-v2.1.0-errata01-os-complete.html#_Toc141790730):
    // Although the order in which properties appear in a JSON object value is not semantically significant, the version property SHOULD appear first.

    return "{\n  \"version\": \"" + std::string(sarifVersion) + "\"," + picojson::value(doc).serialize(true).substr(1);
}

std::string SarifReport::sarifSeverity(const ErrorMessage& errmsg)
{
    if (ErrorLogger::isCriticalErrorId(errmsg.id))
        return "error";
    switch (errmsg.severity) {
    case Severity::error:
    case Severity::warning:
        return "error";
    case Severity::style:
    case Severity::portability:
    case Severity::performance:
        return "warning";
    case Severity::information:
    case Severity::internal:
    case Severity::debug:
    case Severity::none:
        return "note";
    }
    return "note";
}

std::string SarifReport::sarifPrecision(const ErrorMessage& errmsg)
{
    if (errmsg.certainty == Certainty::inconclusive)
        return "medium";
    return "high";
}
