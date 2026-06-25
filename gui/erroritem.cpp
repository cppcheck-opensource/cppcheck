/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "erroritem.h"

#include "common.h"

#include <list>

QErrorPathItem::QErrorPathItem(const ErrorMessage::FileLocation &loc)
    : file(QString::fromStdString(loc.getfile(false)))
    , line(loc.line)
    , column(loc.column)
    , info(QString::fromStdString(loc.getinfo()))
{}

bool operator==(const QErrorPathItem &i1, const QErrorPathItem &i2)
{
    return i1.file == i2.file && i1.column == i2.column && i1.line == i2.line && i1.info == i2.info;
}

ErrorItem::ErrorItem()
    : severity(Severity::none)
    , inconclusive(false)
    , cwe(0)
    , hash(0)
{}

ErrorItem::ErrorItem(const ErrorMessage &errmsg)
    : file0(QString::fromStdString(errmsg.file0))
    , errorId(QString::fromStdString(errmsg.id))
    , severity(errmsg.severity)
    , inconclusive(errmsg.certainty == Certainty::inconclusive)
    , summary(QString::fromStdString(errmsg.shortMessage()))
    , message(QString::fromStdString(errmsg.verboseMessage()))
    , cwe(errmsg.cwe.id)
    , hash(errmsg.hash)
    , symbolNames(QString::fromStdString(errmsg.symbolNames()))
    , remark(QString::fromStdString(errmsg.remark))
{
    for (const auto& loc: errmsg.callStack)
        errorPath << QErrorPathItem(loc);
}

QString ErrorItem::tool() const
{
    if (errorId == CLANG_ANALYZER)
        return CLANG_ANALYZER;
    if (errorId.startsWith(CLANG_TIDY))
        return CLANG_TIDY;
    if (errorId.startsWith("clang-"))
        return "clang";
    return "cppcheck";
}

QString ErrorItem::toString() const
{
    const int i = getMainLocIndex();
    QString ret = errorPath[i].file + ":" + QString::number(errorPath[i].line) + ":" + QString::number(errorPath[i].column) + ":";
    ret += GuiSeverity::toString(severity);
    if (inconclusive)
        ret += ",inconclusive";
    ret += ": " + summary + " [" + errorId + "]";
    if (errorPath.size() >= 2) {
        for (const auto& e: errorPath)
            ret += "\n" + e.file + ":" + QString::number(e.line) + ":" + QString::number(e.column) + ":note: " + e.info;
    }
    return ret;
}

bool ErrorItem::same(const ErrorItem &errorItem1, const ErrorItem &errorItem2)
{
    if (errorItem1.hash && errorItem2.hash)
        return errorItem1.hash == errorItem2.hash;

    // fallback
    return errorItem1.errorId == errorItem2.errorId &&
           errorItem1.errorPath == errorItem2.errorPath &&
           errorItem1.file0 == errorItem2.file0 &&
           errorItem1.message == errorItem2.message &&
           errorItem1.inconclusive == errorItem2.inconclusive &&
           errorItem1.severity == errorItem2.severity;
}

bool ErrorItem::filterMatch(const QString& filter) const
{
    if (filter.isEmpty())
        return true;
    if (summary.contains(filter, Qt::CaseInsensitive) ||
        message.contains(filter, Qt::CaseInsensitive) ||
        errorId.contains(filter, Qt::CaseInsensitive) ||
        classification.contains(filter, Qt::CaseInsensitive))
        return true;
    return std::any_of(errorPath.cbegin(), errorPath.cend(),
                       [filter](const auto& e) {
        return e.file.contains(filter, Qt::CaseInsensitive) ||
               e.info.contains(filter, Qt::CaseInsensitive);
    });
}
