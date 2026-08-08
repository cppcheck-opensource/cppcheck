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

#include "testplatforms.h"

#include "platform.h"
#include "platforms.h"

#include <QtTest>

QTEST_MAIN(TestPlatforms)

void TestPlatforms::init_populatesAllPlatforms() const
{
    const Platforms platforms;
    QCOMPARE(platforms.getCount(), 6);
}

void TestPlatforms::get_validPlatform() const
{
    Platforms platforms;
    const PlatformData& native = platforms.get(Platform::Type::Native);
    QVERIFY(!native.mTitle.isEmpty());
    QCOMPARE(native.mType, Platform::Type::Native);
}

void TestPlatforms::platformTypes_allPresent() const
{
    Platforms platforms;
    const QList<Platform::Type> expectedTypes = {
        Platform::Type::Native,
        Platform::Type::Unix32,
        Platform::Type::Unix64,
        Platform::Type::Win32A,
        Platform::Type::Win32W,
        Platform::Type::Win64,
    };
    for (const Platform::Type type : expectedTypes) {
        bool found = false;
        for (const PlatformData& pd : platforms.mPlatforms) {
            if (pd.mType == type) {
                found = true;
                break;
            }
        }
        QVERIFY2(found, qPrintable(QString("Platform type %1 not found").arg(static_cast<int>(type))));
    }
}

void TestPlatforms::platformTitles_nonEmpty() const
{
    const Platforms platforms;
    for (const PlatformData& pd : platforms.mPlatforms) {
        QVERIFY2(!pd.mTitle.isEmpty(),
                 qPrintable(QString("Platform type %1 has empty title").arg(static_cast<int>(pd.mType))));
    }
}

void TestPlatforms::add_increasesCount() const
{
    Platforms platforms;
    const int before = platforms.getCount();
    platforms.add("Custom", Platform::Type::Native);
    QCOMPARE(platforms.getCount(), before + 1);
}
