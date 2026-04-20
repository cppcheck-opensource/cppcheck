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

#ifndef ERRORITEM_H
#define ERRORITEM_H

#include "errorlogger.h"
#include "errortypes.h"

#include <QList>
#include <QString>

/// @addtogroup GUI
/// @{


/**
 * @brief GUI versions of severity conversions.
 * GUI needs wrappers for conversion functions since GUI uses Qt's QString
 * instead of the std::string used by lib/cli.
 */
class GuiSeverity {
public:
    static QString toString(Severity severity) {
        return QString::fromStdString(severityToString(severity));
    }

    static Severity fromString(const QString &severity) {
        return severityFromString(severity.toStdString());
    }
};

/**
 * @brief A class containing data for one error path item
 */
class QErrorPathItem {
public:
    QErrorPathItem() : line(0), column(-1) {}
    explicit QErrorPathItem(const ErrorMessage::FileLocation &loc);
    QString file;
    int line;
    int column;
    QString info;
};

bool operator==(const QErrorPathItem &i1, const QErrorPathItem &i2);

/**
 * @brief A class containing error data for one error.
 *
 * The paths are stored with internal ("/") separators. Only when we show the
 * path or copy if for user (to clipboard) we convert to native separators.
 * Full path is stored instead of relative path for flexibility. It is easy
 * to get the relative path from full path when needed.
 */
class ErrorItem {
public:
    ErrorItem();
    explicit ErrorItem(const ErrorMessage &errmsg);

    /**
     * @brief Convert error item to string.
     * @return Error item as string.
     */
    QString toString() const;
    QString tool() const;

    int getMainLocIndex() const {
        return isClangResult() ? 0 : errorPath.size() - 1;
    }

    QString getFile() const {
        return errorPath.isEmpty() ? QString() : errorPath[getMainLocIndex()].file;
    }

    bool isClangResult() const {
        return errorId.startsWith("clang");
    }

    bool filterMatch(const QString& filter) const;

    QString file0;
    QString errorId;
    Severity severity;
    bool inconclusive;
    QString summary;
    QString message;
    int cwe;
    unsigned long long hash;
    QList<QErrorPathItem> errorPath;
    QString symbolNames;
    QString remark;
    QString classification; // misra/cert/etc: classification/level
    QString guideline; // misra/cert/etc: guideline/rule

    // Special GUI properties
    QString sinceDate;
    QString tags;

    /**
     * Compare Hash and fields
     */
    static bool same(const ErrorItem &errorItem1, const ErrorItem &errorItem2);
};
/// @}
#endif // ERRORITEM_H
