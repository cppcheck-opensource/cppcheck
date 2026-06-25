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

#ifndef TXT_REPORT_H
#define TXT_REPORT_H

#include "report.h"

#include <QString>
#include <QObject>
#include <QTextStream>

/// @addtogroup GUI
/// @{


/**
 * @brief Text file report.
 * This report mimics the output of the command line cppcheck.
 */
class TxtReport : public Report {
    Q_OBJECT

public:
    explicit TxtReport(const QString &filename);

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

private:

    /**
     * @brief Text stream writer for writing the report in text format.
     */
    QTextStream mTxtWriter;
};
/// @}
#endif // TXT_REPORT_H
