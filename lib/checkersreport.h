/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#ifndef checkersReportH
#define checkersReportH

#include "config.h"

#include <set>
#include <string>

class Settings;

class CPPCHECKLIB CheckersReport {
public:
    CheckersReport(const Settings& settings, const std::set<std::string>& activeCheckers);

    int getActiveCheckersCount();
    int getAllCheckersCount();

    std::string getReport(const std::string& criticalErrors) const;
    std::string getXmlReport(const std::string& criticalErrors) const;

private:
    const Settings& mSettings;
    const std::set<std::string>& mActiveCheckers;

    void countCheckers();

    int mActiveCheckersCount = 0;
    int mAllCheckersCount = 0;
};

#endif
