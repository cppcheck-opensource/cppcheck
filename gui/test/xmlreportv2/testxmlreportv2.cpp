/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#include "testxmlreportv2.h"

#include "erroritem.h"
#include "xmlreportv2.h"

#include <QList>
#include <QtTest>

void TestXmlReportV2::readXml() const
{
    const QString filepath(QString(SRCDIR) + "/../data/xmlfiles/xmlreport_v2.xml");
    XmlReportV2 report(filepath, QString());
    QVERIFY(report.open());
    QList<ErrorItem> errors = report.read();
    QCOMPARE(errors.size(), 6);

    const ErrorItem &item = errors[0];
    QCOMPARE(item.errorPath.size(), 1);
    QCOMPARE(item.errorPath[0].file, QString("test.cxx"));
    QCOMPARE(item.errorPath[0].line, 11);
    QCOMPARE(item.errorId, QString("unreadVariable"));
    QCOMPARE(GuiSeverity::toString(item.severity), QString("style"));
    QCOMPARE(item.summary, QString("Variable 'a' is assigned a value that is never used"));
    QCOMPARE(item.message, QString("Variable 'a' is assigned a value that is never used"));

    const ErrorItem &item2 = errors[3];
    QCOMPARE(item2.errorPath.size(), 2);
    QCOMPARE(item2.errorPath[0].file, QString("test.cxx"));
    QCOMPARE(item2.errorPath[0].line, 16);
    QCOMPARE(item2.errorPath[1].file, QString("test.cxx"));
    QCOMPARE(item2.errorPath[1].line, 32);
    QCOMPARE(item2.errorId, QString("mismatchAllocDealloc"));
    QCOMPARE(GuiSeverity::toString(item2.severity), QString("error"));
    QCOMPARE(item2.summary, QString("Mismatching allocation and deallocation: k"));
    QCOMPARE(item2.message, QString("Mismatching allocation and deallocation: k"));
}

QTEST_MAIN(TestXmlReportV2)
