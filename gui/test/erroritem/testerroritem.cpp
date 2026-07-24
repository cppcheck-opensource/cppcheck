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

#include "testerroritem.h"

#include "erroritem.h"
#include "errortypes.h"

#include <QtTest>

QTEST_MAIN(TestErrorItem)

void TestErrorItem::guiSeverity_toString() const
{
    QCOMPARE(GuiSeverity::toString(Severity::error),       QString("error"));
    QCOMPARE(GuiSeverity::toString(Severity::warning),     QString("warning"));
    QCOMPARE(GuiSeverity::toString(Severity::style),       QString("style"));
    QCOMPARE(GuiSeverity::toString(Severity::performance), QString("performance"));
    QCOMPARE(GuiSeverity::toString(Severity::portability), QString("portability"));
    QCOMPARE(GuiSeverity::toString(Severity::information), QString("information"));
    QCOMPARE(GuiSeverity::toString(Severity::none),        QString(""));
}

void TestErrorItem::guiSeverity_fromString() const
{
    QCOMPARE(GuiSeverity::fromString("error"),       Severity::error);
    QCOMPARE(GuiSeverity::fromString("warning"),     Severity::warning);
    QCOMPARE(GuiSeverity::fromString("style"),       Severity::style);
    QCOMPARE(GuiSeverity::fromString("performance"), Severity::performance);
    QCOMPARE(GuiSeverity::fromString("portability"), Severity::portability);
    QCOMPARE(GuiSeverity::fromString("information"), Severity::information);
    QCOMPARE(GuiSeverity::fromString("none"),        Severity::none);
    QCOMPARE(GuiSeverity::fromString("unknown"),     Severity::none);
}

void TestErrorItem::errorItem_tool() const
{
    ErrorItem item;
    item.errorId = "nullPointer";
    QCOMPARE(item.tool(), QString("cppcheck"));

    item.errorId = "clang-analyzer";
    QCOMPARE(item.tool(), QString("clang-analyzer"));

    item.errorId = "clang-tidy-readability-braces";
    QCOMPARE(item.tool(), QString("clang-tidy"));

    item.errorId = "clang-diagnostic-unused";
    QCOMPARE(item.tool(), QString("clang"));
}

void TestErrorItem::errorItem_toString() const
{
    ErrorItem item;
    item.errorId = "nullPointer";
    item.severity = Severity::error;
    item.summary = "Null pointer dereference";

    QErrorPathItem path;
    path.file = "test.cpp";
    path.line = 42;
    path.column = 5;
    item.errorPath << path;

    const QString result = item.toString();
    QVERIFY(result.startsWith("test.cpp:42:5:"));
    QVERIFY(result.contains("[nullPointer]"));
    QVERIFY(result.contains("Null pointer dereference"));
    QVERIFY(result.contains("error"));
}

void TestErrorItem::errorItem_same_byHash() const
{
    ErrorItem item1;
    item1.hash = 12345;
    ErrorItem item2;
    item2.hash = 12345;
    QVERIFY(ErrorItem::same(item1, item2));

    item2.hash = 99999;
    QVERIFY(!ErrorItem::same(item1, item2));
}

void TestErrorItem::errorItem_same_byFields() const
{
    ErrorItem item1;
    item1.errorId = "nullPointer";
    item1.severity = Severity::error;
    item1.message = "The pointer is null";
    item1.inconclusive = false;
    QErrorPathItem p;
    p.file = "test.cpp";
    p.line = 10;
    p.column = 3;
    item1.errorPath << p;

    ErrorItem item2 = item1;
    QVERIFY(ErrorItem::same(item1, item2));
}

void TestErrorItem::errorItem_same_different() const
{
    ErrorItem item1;
    item1.errorId = "nullPointer";
    item1.severity = Severity::error;

    ErrorItem item2;
    item2.errorId = "uninitVar";
    item2.severity = Severity::error;

    QVERIFY(!ErrorItem::same(item1, item2));
}

void TestErrorItem::errorItem_filterMatch() const
{
    ErrorItem item;
    item.errorId = "nullPointer";
    item.summary = "Null pointer dereference";
    item.message = "The pointer is null at this location";
    QErrorPathItem p;
    p.file = "main.cpp";
    p.info = "check here";
    item.errorPath << p;

    // empty filter matches everything
    QVERIFY(item.filterMatch(""));

    // matches in errorId (case insensitive)
    QVERIFY(item.filterMatch("nullPointer"));
    QVERIFY(item.filterMatch("NULLpointer"));

    // matches in summary
    QVERIFY(item.filterMatch("Null pointer"));
    QVERIFY(item.filterMatch("DEREFERENCE"));

    // matches in message
    QVERIFY(item.filterMatch("location"));

    // matches in errorPath file
    QVERIFY(item.filterMatch("main.cpp"));

    // matches in errorPath info
    QVERIFY(item.filterMatch("check here"));

    // no match
    QVERIFY(!item.filterMatch("xyz123abc"));
}
