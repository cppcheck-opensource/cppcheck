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

#ifndef SARIF_REPORT_H
#define SARIF_REPORT_H

#include "config.h"
#include "errorlogger.h"

#include <string>
#include <vector>

// Include picojson headers
#include "json.h"

class CPPCHECKLIB SarifReport {
public:
    SarifReport() = default;
    ~SarifReport() = default;

    void addFinding(ErrorMessage msg);
    std::string serialize(std::string productName) const;

private:
    // Implementation methods
    picojson::array serializeRules() const;
    static picojson::array serializeLocations(const ErrorMessage& finding);
    picojson::array serializeResults() const;
    picojson::value serializeRuns(const std::string& productName, const std::string& version) const;

    // Utility methods
    static std::string sarifSeverity(const ErrorMessage& errmsg);
    static std::string sarifPrecision(const ErrorMessage& errmsg);

    std::vector<ErrorMessage> mFindings;
};

#endif // SARIF_REPORT_H
