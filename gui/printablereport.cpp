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

#include "printablereport.h"

#include "erroritem.h"

#include <QDir>
#include <QList>

PrintableReport::PrintableReport() :
    Report(QString())
{}

bool PrintableReport::create()
{
    return true;
}

void PrintableReport::writeHeader()
{
    // No header for printable report
}

void PrintableReport::writeFooter()
{
    // No footer for printable report
}

void PrintableReport::writeError(const ErrorItem &error)
{
    const QString file = QDir::toNativeSeparators(error.errorPath.back().file);
    QString line = QString("%1,%2,").arg(file).arg(error.errorPath.back().line);
    line += QString("%1,%2").arg(GuiSeverity::toString(error.severity)).arg(error.summary);

    mFormattedReport += line;
    mFormattedReport += "\n";
}

const QString& PrintableReport::getFormattedReportText() const
{
    return mFormattedReport;
}

