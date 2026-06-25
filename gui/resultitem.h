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

#ifndef RESULTITEM_H
#define RESULTITEM_H

#include "erroritem.h"
#include <QStandardItem>
#include <QSharedPointer>

class ResultItem : public QStandardItem
{
public:
    enum class Type: std::uint8_t {file, message, note};

    ResultItem(QSharedPointer<ErrorItem> errorItem, Type type, int errorPathIndex);
    QSharedPointer<ErrorItem> errorItem;
    bool hidden{};

    QErrorPathItem getErrorPathItem() const {
        if (!errorItem || mErrorPathIndex < 0 || mErrorPathIndex >= errorItem->errorPath.size())
            return {};
        return errorItem->errorPath[mErrorPathIndex];
    }

    Type getType() const {
        return mType;
    }

    void setIconFileName(const QString& iconFileName) {
        mIconFileName = iconFileName;
        if (!mIconFileName.isEmpty())
            setIcon(QIcon(iconFileName));
    }

    // cppcheck-suppress unusedFunction; used in test-resultstree
    const QString& getIconFileName() const {
        return mIconFileName;
    }
private:
    const Type mType;
    const int mErrorPathIndex;
    QString mIconFileName;
};

#endif // RESULTITEM_H
