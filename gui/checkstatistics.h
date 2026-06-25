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

#ifndef CHECKSTATISTICS_H
#define CHECKSTATISTICS_H

#include "showtypes.h"

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include <set>
#include <string>
#include <utility>

/// @addtogroup GUI
/// @{

/**
 * A class for check statistics.
 */
class CheckStatistics : public QObject {
public:
    explicit CheckStatistics(QObject *parent = nullptr);

    /**
     * @brief Add new checked item to statistics.
     *
     * @param tool Tool.
     * @param type Type of the item to add.
     */
    void addItem(const QString &tool, ShowTypes::ShowType type);

    /**
     * @brief Add checker to statistics
     */
    void addChecker(const QString& checker);

    /**
     * @brief Clear the statistics.
     *
     */
    void clear();

    /**
     * @brief Return statistics for given type.
     *
     * @param tool Tool.
     * @param type Type for which the statistics are returned.
     * @return Number of items of given type.
     */
    unsigned getCount(const QString &tool, ShowTypes::ShowType type) const;

    const std::set<std::string>& getActiveCheckers() const {
        return mActiveCheckers;
    }

    int getNumberOfActiveCheckers() const {
        return mActiveCheckers.size();
    }

    /** Get tools with results */
    QStringList getTools() const;

    void setCheckersReport(QString report) {
        mCheckersReport = std::move(report);
    }
    const QString& getCheckersReport() const {
        return mCheckersReport;
    }

private:
    QMap<QString, unsigned> mStyle;
    QMap<QString, unsigned> mWarning;
    QMap<QString, unsigned> mPerformance;
    QMap<QString, unsigned> mPortability;
    QMap<QString, unsigned> mInformation;
    QMap<QString, unsigned> mError;
    std::set<std::string> mActiveCheckers;
    QString mCheckersReport;
};

/// @}

#endif // CHECKSTATISTICS_H
