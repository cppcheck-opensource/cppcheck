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

#ifndef GITLAB_CODE_QUALITY_REPORT_H
#define GITLAB_CODE_QUALITY_REPORT_H

#include "config.h"
#include "errorlogger.h"

#include <string>
#include <vector>

#include "json.h"

/**
 * @brief Generates a GitLab Code Quality report.
 *
 * The report is a JSON array in the GitLab Code Quality report format.
 *
 * Each finding must contain:
 *
 * - description: human-readable description of the violation
 * - check_name: unique name of the check or rule
 * - fingerprint: unique fingerprint for the violation
 * - severity: info, minor, major, critical, or blocker
 * - location.path: repository-relative path to the source file
 * - location.lines.begin (or location.positions.begin.line): line number
 *
 * Example:
 * @code{.json}
 * [
 *   {
 *     "description": "Division by zero.",
 *     "check_name": "zerodiv",
 *     "fingerprint": "12532254259410460725",
 *     "severity": "critical",
 *     "location": {
 *       "path": "src/main.cpp",
 *       "lines": {
 *         "begin": 4
 *       }
 *     }
 *   }
 * ]
 * @endcode
 *
 * @see https://docs.gitlab.com/ci/testing/code_quality/#code-quality-report-format
 */
class CPPCHECKLIB GitlabCodeQualityReport {
public:
    GitlabCodeQualityReport() = default;
    ~GitlabCodeQualityReport() = default;

    void addFinding(ErrorMessage msg);
    std::string serialize() const;

private:
    static picojson::object serializeFinding(const ErrorMessage& finding);
    static std::string codeQualitySeverity(const ErrorMessage& finding);

    std::vector<ErrorMessage> mFindings;
};

#endif // GITLAB_CODE_QUALITY_REPORT_H
