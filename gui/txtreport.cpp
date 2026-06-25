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

#include "txtreport.h"

#include "erroritem.h"

#include <QDir>
#include <QFile>
#include <QList>
#include <QtGlobal>

TxtReport::TxtReport(const QString &filename) :
    Report(filename)
{}

bool TxtReport::create()
{
    if (Report::create()) {
        mTxtWriter.setDevice(Report::getFile());
        return true;
    }
    return false;
}

void TxtReport::writeHeader()
{
    // No header for txt report
}

void TxtReport::writeFooter()
{
    // No footer for txt report
}

void TxtReport::writeError(const ErrorItem &error)
{
    /*
       Error example from the core program in text
       [gui/test.cpp:23] -> [gui/test.cpp:14]: (error) Mismatching allocation and deallocation: k
     */

    QString line;

    for (int i = 0; i < error.errorPath.size(); i++) {
        const QString file = QDir::toNativeSeparators(error.errorPath[i].file);
        line += QString("[%1:%2]").arg(file).arg(error.errorPath[i].line);
        if (i < error.errorPath.size() - 1) {
            line += " -> ";
        }

        if (i == error.errorPath.size() - 1) {
            line += ": ";
        }
    }
    QString temp = "(%1";
    if (error.inconclusive) {
        temp += ", ";
        temp += tr("inconclusive");
    }
    temp += ") ";
    line += temp.arg(GuiSeverity::toString(error.severity));
    line += error.summary;

    mTxtWriter << line << Qt::endl;
}
