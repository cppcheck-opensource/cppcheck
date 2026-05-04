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

#include "testshowtypes.h"

#include "errortypes.h"
#include "showtypes.h"

#include <QtTest>

QTEST_MAIN(TestShowTypes)

void TestShowTypes::severityToShowType_allValues() const
{
    QCOMPARE(ShowTypes::SeverityToShowType(Severity::error),       ShowTypes::ShowErrors);
    QCOMPARE(ShowTypes::SeverityToShowType(Severity::warning),     ShowTypes::ShowWarnings);
    QCOMPARE(ShowTypes::SeverityToShowType(Severity::style),       ShowTypes::ShowStyle);
    QCOMPARE(ShowTypes::SeverityToShowType(Severity::performance), ShowTypes::ShowPerformance);
    QCOMPARE(ShowTypes::SeverityToShowType(Severity::portability), ShowTypes::ShowPortability);
    QCOMPARE(ShowTypes::SeverityToShowType(Severity::information), ShowTypes::ShowInformation);
}

void TestShowTypes::severityToShowType_noneAndInternal() const
{
    QCOMPARE(ShowTypes::SeverityToShowType(Severity::none),     ShowTypes::ShowNone);
    QCOMPARE(ShowTypes::SeverityToShowType(Severity::internal), ShowTypes::ShowNone);
}

void TestShowTypes::setShow_togglesVisibility()
{
    ShowTypes st;

    // hide each type and verify
    st.show(ShowTypes::ShowErrors, false);
    QVERIFY(!st.isShown(ShowTypes::ShowErrors));

    // restore and verify
    st.show(ShowTypes::ShowErrors, true);
    QVERIFY(st.isShown(ShowTypes::ShowErrors));

    // hide a different type
    st.show(ShowTypes::ShowWarnings, false);
    QVERIFY(!st.isShown(ShowTypes::ShowWarnings));
    QVERIFY(st.isShown(ShowTypes::ShowErrors)); // others unaffected

    st.show(ShowTypes::ShowStyle, false);
    QVERIFY(!st.isShown(ShowTypes::ShowStyle));

    st.show(ShowTypes::ShowPerformance, false);
    QVERIFY(!st.isShown(ShowTypes::ShowPerformance));

    st.show(ShowTypes::ShowPortability, false);
    QVERIFY(!st.isShown(ShowTypes::ShowPortability));

    st.show(ShowTypes::ShowInformation, false);
    QVERIFY(!st.isShown(ShowTypes::ShowInformation));
}

void TestShowTypes::isShown_bySeverity()
{
    ShowTypes st;

    // ensure errors are visible, then test severity overload
    st.show(ShowTypes::ShowErrors, true);
    QVERIFY(st.isShown(Severity::error));

    // hide errors via ShowType, verify Severity overload reflects it
    st.show(ShowTypes::ShowErrors, false);
    QVERIFY(!st.isShown(Severity::error));

    // hide warnings, verify
    st.show(ShowTypes::ShowWarnings, false);
    QVERIFY(!st.isShown(Severity::warning));

    // ShowNone severities always return false (array is only indexed 0..ShowNone-1)
    // Severity::none maps to ShowNone which is out of bounds for mVisible,
    // so we only test mapped severities here
}
