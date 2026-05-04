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

#include "testcodeeditorstyle.h"

#include "codeeditorstyle.h"

#include <QDir>
#include <QSettings>
#include <QtTest>

QTEST_MAIN(TestCodeEditorStyle)

void TestCodeEditorStyle::defaultStyles_notEqual() const
{
    QVERIFY(defaultStyleLight != defaultStyleDark);
}

void TestCodeEditorStyle::equality_sameStyle() const
{
    QVERIFY(defaultStyleLight == defaultStyleLight);
    QVERIFY(defaultStyleDark == defaultStyleDark);
}

void TestCodeEditorStyle::equality_differentColor() const
{
    CodeEditorStyle style(defaultStyleLight);
    style.widgetFGColor = QColor(1, 2, 3);
    QVERIFY(style != defaultStyleLight);
}

void TestCodeEditorStyle::getSystemTheme_isSystemTheme() const
{
    const CodeEditorStyle sys = CodeEditorStyle::getSystemTheme();
    QVERIFY(sys.isSystemTheme());
}

void TestCodeEditorStyle::getSystemTheme_colorsMatchLight() const
{
    const CodeEditorStyle sys = CodeEditorStyle::getSystemTheme();
    QCOMPARE(sys.widgetFGColor,    defaultStyleLight.widgetFGColor);
    QCOMPARE(sys.widgetBGColor,    defaultStyleLight.widgetBGColor);
    QCOMPARE(sys.keywordColor,     defaultStyleLight.keywordColor);
    QCOMPARE(sys.keywordWeight,    defaultStyleLight.keywordWeight);
    QCOMPARE(sys.commentColor,     defaultStyleLight.commentColor);
}

void TestCodeEditorStyle::saveLoad_darkRoundtrip() const
{
    const QString settingsFile = QDir::tempPath() + "/test_codeeditorstyle_dark.ini";
    QFile::remove(settingsFile);

    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        CodeEditorStyle::saveSettings(&settings, defaultStyleDark);
    }

    QSettings settings(settingsFile, QSettings::IniFormat);
    const CodeEditorStyle loaded = CodeEditorStyle::loadSettings(&settings);
    QCOMPARE(loaded, defaultStyleDark);

    QFile::remove(settingsFile);
}

void TestCodeEditorStyle::saveLoad_customColors() const
{
    // Create a style that differs from both defaults so it is saved as "Custom"
    CodeEditorStyle custom(defaultStyleLight);
    custom.widgetFGColor = QColor(11, 22, 33);
    custom.keywordColor  = QColor(44, 55, 66);
    custom.commentColor  = QColor(77, 88, 99);

    const QString settingsFile = QDir::tempPath() + "/test_codeeditorstyle_custom.ini";
    QFile::remove(settingsFile);

    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        CodeEditorStyle::saveSettings(&settings, custom);
    }

    QSettings settings(settingsFile, QSettings::IniFormat);
    const CodeEditorStyle loaded = CodeEditorStyle::loadSettings(&settings);
    QCOMPARE(loaded.widgetFGColor, custom.widgetFGColor);
    QCOMPARE(loaded.keywordColor,  custom.keywordColor);
    QCOMPARE(loaded.commentColor,  custom.commentColor);

    QFile::remove(settingsFile);
}
