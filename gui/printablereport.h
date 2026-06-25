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

#ifndef PRINTABLE_REPORT_H
#define PRINTABLE_REPORT_H

#include "report.h"

#include <QString>

/// @addtogroup GUI
/// @{


/**
 * @brief Printable (in-memory) report.
 * This report formats results and exposes them for printing.
 */
class PrintableReport : public Report {
public:
    PrintableReport();

    /**
     * @brief Create the report (file).
     * @return true if succeeded, false if file could not be created.
     */
    bool create() override;

    /**
     * @brief Write report header.
     */
    void writeHeader() override;

    /**
     * @brief Write report footer.
     */
    void writeFooter() override;

    /**
     * @brief Write error to report.
     * @param error Error data.
     */
    void writeError(const ErrorItem &error) override;

    /**
     * @brief Returns the formatted report.
     */
    const QString& getFormattedReportText() const;

private:

    /**
     * @brief Stores the formatted report contents.
     */
    QString mFormattedReport;
};
/// @}
#endif // PRINTABLE_REPORT_H
