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

#ifndef XML_REPORTV2_H
#define XML_REPORTV2_H

#include "erroritem.h"
#include "xmlreport.h"

#include <QString>

class QXmlStreamReader;
class QXmlStreamWriter;

/// @addtogroup GUI
/// @{


/**
 * @brief XML file report version 2.
 * This report outputs XML-formatted report. The XML format must match command
 * line version's XML output.
 */
class XmlReportV2 : public XmlReport {
public:
    explicit XmlReportV2(const QString &filename, QString productName);
    ~XmlReportV2() override;

    /**
     * @brief Create the report (file).
     * @return true if succeeded, false if file could not be created.
     */
    bool create() override;

    /**
     * @brief Open existing report file.
     */
    bool open() override;

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
     * @brief Read contents of the report file.
     */
    QList<ErrorItem> read() override;

protected:
    /**
     * @brief Read and parse error item from XML stream.
     * @param reader XML stream reader to use.
     */
    ErrorItem readError(const QXmlStreamReader *reader);

private:
    /** Product name read from cppcheck.cfg */
    const QString mProductName;

    /**
     * @brief XML stream reader for reading the report in XML format.
     */
    QXmlStreamReader *mXmlReader;

    /**
     * @brief XML stream writer for writing the report in XML format.
     */
    QXmlStreamWriter *mXmlWriter;
};
/// @}
#endif // XML_REPORTV2_H
