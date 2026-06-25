/*
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


#include "common.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QSettings>
#include <QStringList>
#include <QVariant>
#include <Qt>


QString getPath(const QString &type)
{
    QSettings settings;
    QString path = settings.value(type, QString()).toString();
    if (path.isEmpty()) {
        // if not set, fallback to last check path hoping that it will be close enough
        path = settings.value(SETTINGS_LAST_CHECK_PATH, QString()).toString();
        if (path.isEmpty())
            // if not set, return user's home directory as the best we can do for now
            return QDir::homePath();
    }
    return path;
}

void setPath(const QString &type, const QString &value)
{
    QSettings settings;
    settings.setValue(type, value);
}

QString toFilterString(const QMap<QString,QString>& filters, bool addAllSupported, bool addAll)
{
    QStringList entries;

    if (addAllSupported) {
        entries << QCoreApplication::translate("toFilterString", "All supported files (%1)")
            .arg(filters.values().join(" "));
    }

    if (addAll) {
        entries << QCoreApplication::translate("toFilterString", "All files (%1)").arg("*.*");
    }

    // We're using the description of the filters as the map keys, the file
    // name patterns are our values. The generated filter string list will
    // thus be sorted alphabetically over the descriptions.
    for (const auto& k: filters.keys()) {
        entries << QString("%1 (%2)").arg(k).arg(filters.value(k));
    }

    return entries.join(";;");
}

QString getDataDir()
{
    QSettings settings;
    const QString dataDir = settings.value("DATADIR", QString()).toString();
    if (!dataDir.isEmpty())
        return dataDir;
    const QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).canonicalPath();
    if (QFileInfo::exists(appPath + "/std.cfg"))
        return appPath;
    if (appPath.indexOf("/cppcheck/", 0, Qt::CaseInsensitive) > 0)
        return appPath.left(appPath.indexOf("/cppcheck/", 0, Qt::CaseInsensitive) + 9);
    return appPath;
}
