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

#include "testxmlreportv2.h"

#include "erroritem.h"
#include "xmlreportv2.h"

#include <QFile>
#include <QList>
#include <QTemporaryDir>
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

void TestXmlReportV2::writeXmlEscaping() const
{
    QTemporaryDir tmpdir;
    const QString path = tmpdir.path() + "/results.xml";

    ErrorItem item;
    item.errorId = "unusedVariable";
    item.severity = Severity::style;
    item.summary = "Unused variable: \"x\" & <y>";
    item.message = item.summary;

    QErrorPathItem loc;
    loc.file = "test.cpp";
    loc.line = 1;
    loc.column = 1;
    loc.info = item.summary;
    item.errorPath << loc;
    loc.line = 2;
    item.errorPath << loc;

    {
        XmlReportV2 report(path, QString());
        QVERIFY(report.create());
        report.writeHeader();
        report.writeError(item);
        report.writeFooter();
    }

    // Each special character must be escaped exactly once
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));

    const QString xml = QString::fromUtf8(file.readAll());
    QVERIFY(xml.contains("msg=\"Unused variable: &quot;x&quot; &amp; &lt;y&gt;\""));
    QVERIFY(xml.contains("info=\"Unused variable: &quot;x&quot; &amp; &lt;y&gt;\""));
    QVERIFY(!xml.contains("&amp;quot;"));

    XmlReportV2 report(path, QString());
    QVERIFY(report.open());

    const QList<ErrorItem> errors = report.read();
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].summary, item.summary);
    QCOMPARE(errors[0].message, item.message);
    QCOMPARE(errors[0].errorPath.size(), 2);
    QCOMPARE(errors[0].errorPath[0].info, item.summary);
}

QTEST_MAIN(TestXmlReportV2)
