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

#include "checkstatistics.h"

#include "utils.h"

#include <QDebug>
#include <QList>
#include <QSet>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#include <QtLogging>
#endif

CheckStatistics::CheckStatistics(QObject *parent)
    : QObject(parent)
{
    clear();
}

static void addItem(QMap<QString,unsigned> &m, const QString &key)
{
    m[key]++;
}

void CheckStatistics::addItem(const QString &tool, ShowTypes::ShowType type)
{
    const QString lower = tool.toLower();
    switch (type) {
    case ShowTypes::ShowStyle:
        ::addItem(mStyle, lower);
        break;
    case ShowTypes::ShowWarnings:
        ::addItem(mWarning, lower);
        break;
    case ShowTypes::ShowPerformance:
        ::addItem(mPerformance, lower);
        break;
    case ShowTypes::ShowPortability:
        ::addItem(mPortability, lower);
        break;
    case ShowTypes::ShowErrors:
        ::addItem(mError, lower);
        break;
    case ShowTypes::ShowInformation:
        ::addItem(mInformation, lower);
        break;
    case ShowTypes::ShowNone:
        qDebug() << "Unknown error type - not added to statistics.";
        break;
    }
}

void CheckStatistics::addChecker(const QString &checker)
{
    mActiveCheckers.insert(checker.toStdString());
}

void CheckStatistics::clear()
{
    mStyle.clear();
    mWarning.clear();
    mPerformance.clear();
    mPortability.clear();
    mInformation.clear();
    mError.clear();
    mActiveCheckers.clear();
    mCheckersReport.clear();
}

unsigned CheckStatistics::getCount(const QString &tool, ShowTypes::ShowType type) const
{
    const QString lower = tool.toLower();
    switch (type) {
    case ShowTypes::ShowStyle:
        return mStyle.value(lower,0);
    case ShowTypes::ShowWarnings:
        return mWarning.value(lower,0);
    case ShowTypes::ShowPerformance:
        return mPerformance.value(lower,0);
    case ShowTypes::ShowPortability:
        return mPortability.value(lower,0);
    case ShowTypes::ShowErrors:
        return mError.value(lower,0);
    case ShowTypes::ShowInformation:
        return mInformation.value(lower,0);
    case ShowTypes::ShowNone:
        qDebug() << "Unknown error type - returning zero statistics.";
        return 0;
    }
    cppcheck::unreachable();
}

QStringList CheckStatistics::getTools() const
{
    QSet<QString> ret;
    for (const QString& tool: mStyle.keys()) ret.insert(tool);
    for (const QString& tool: mWarning.keys()) ret.insert(tool);
    for (const QString& tool: mPerformance.keys()) ret.insert(tool);
    for (const QString& tool: mPortability.keys()) ret.insert(tool);
    for (const QString& tool: mError.keys()) ret.insert(tool);
    return ret.values();
}
