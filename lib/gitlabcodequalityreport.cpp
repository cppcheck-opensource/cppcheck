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
#include "path.h"
#include "utils.h"

#include <cstdint>
#include <utility>

namespace {
    std::string locationPath(const ErrorMessage::FileLocation& location)
    {
        std::string path = location.getfile(false);
        while (startsWith(path, "./"))
            path.erase(0, 2);
        return path;
    }

    picojson::object serializeLocation(const ErrorMessage::FileLocation& location)
    {
        picojson::object lines;
        lines["begin"] = picojson::value(static_cast<int64_t>(location.line < 1 ? 1 : location.line));

        picojson::object result;
        result["path"] = picojson::value(locationPath(location));
        result["lines"] = picojson::value(lines);
        return result;
    }

    std::string fingerprint(const ErrorMessage& finding)
    {
        if (finding.hash != 0) {
            return std::to_string(finding.hash);
        }

        // Findings without token information have no warning hash.
        const ErrorMessage::FileLocation& location = finding.callStack.back();
        return finding.id + ':' + finding.shortMessage() + ':' + locationPath(location) + ':' +
               std::to_string(location.line);
    }
}

void GitlabCodeQualityReport::addFinding(ErrorMessage msg)
{
    if (!msg.callStack.empty() && Path::isAbsolute(locationPath(msg.callStack.back()))) {
        mHasAbsolutePath = true;
    }
    mFindings.push_back(std::move(msg));
}

picojson::object GitlabCodeQualityReport::serializeFinding(const ErrorMessage& finding)
{
    const ErrorMessage::FileLocation& location = finding.callStack.back();

    std::string description = finding.shortMessage();
    if (finding.cwe.id > 0) {
        description += " (CWE-" + std::to_string(finding.cwe.id) + ")";
    }

    picojson::object result;
    result["description"] = picojson::value(description);
    result["check_name"] = picojson::value(finding.id);
    result["fingerprint"] = picojson::value(fingerprint(finding));
    result["severity"] = picojson::value(codeQualitySeverity(finding));
    result["location"] = picojson::value(serializeLocation(location));
    return result;
}

std::string GitlabCodeQualityReport::serialize() const
{
    picojson::array findings;
    for (const ErrorMessage& finding : mFindings) {
        // GitLab requires a location for every code quality finding.
        if (finding.callStack.empty()) {
            continue;
        }
        findings.emplace_back(serializeFinding(finding));
    }

    return picojson::value(findings).serialize(true);
}

std::string GitlabCodeQualityReport::codeQualitySeverity(const ErrorMessage& finding)
{
    if (ErrorLogger::isCriticalErrorId(finding.id)) {
        return "critical";
    }

    switch (finding.severity) {
    case Severity::error:
        return "critical";
    case Severity::warning:
        return "major";
    case Severity::style:
    case Severity::performance:
    case Severity::portability:
        return "minor";
    case Severity::information:
    case Severity::internal:
    case Severity::debug:
    case Severity::none:
        return "info";
    }
    return "info";
}
