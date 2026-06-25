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

#ifndef XML_REPORT_H
#define XML_REPORT_H

#include "report.h"

#include <QString>
#include <QList>

class ErrorItem;

/// @addtogroup GUI
/// @{


/**
 * @brief Base class for XML report classes.
 */
class XmlReport : public Report {
public:
    explicit XmlReport(const QString &filename);

    /**
     * @brief Read contents of the report file.
     */
    virtual QList<ErrorItem> read() = 0;

    /**
     * @brief Quote the message.
     * @param message Message to quote.
     * @return quoted message.
     */
    static QString quoteMessage(const QString &message);

    /**
     * @brief Unquote the message.
     * @param message Message to quote.
     * @return quoted message.
     */
    static QString unquoteMessage(const QString &message);

    /**
     * @brief Get the XML report format version from the file.
     * @param filename Filename of the report file.
     * @return XML report format version or 0 if error happened.
     */
    static int determineVersion(const QString &filename);
};
/// @}

#endif // XML_REPORT_H
