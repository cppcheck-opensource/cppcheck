/* -*- C++ -*-
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

#ifndef CODEEDITORSTYLE_H
#define CODEEDITORSTYLE_H

#include <QColor>
#include <QFont>
#include <QString>
#include <Qt>

const QString SETTINGS_STYLE_GROUP("EditorStyle");
const QString SETTINGS_STYLE_TYPE("StyleType");
const QString SETTINGS_STYLE_TYPE_LIGHT("DefaultLight");
const QString SETTINGS_STYLE_TYPE_DARK("DefaultDark");
const QString SETTINGS_STYLE_TYPE_CUSTOM("Custom");
const QString SETTINGS_STYLE_WIDGETFG("StyleWidgetFG");
const QString SETTINGS_STYLE_WIDGETBG("StyleWidgetBG");
const QString SETTINGS_STYLE_HILIFG("StyleHighlightFG");
const QString SETTINGS_STYLE_LINENUMFG("StyleLineNumFG");
const QString SETTINGS_STYLE_LINENUMBG("StyleLineNumBG");
const QString SETTINGS_STYLE_KEYWORDFG("StyleKeywordFG");
const QString SETTINGS_STYLE_KEYWORDWT("StyleKeywordWeight");
const QString SETTINGS_STYLE_CLASSFG("StyleClassFG");
const QString SETTINGS_STYLE_CLASSWT("StyleClassWeight");
const QString SETTINGS_STYLE_QUOTEFG("StyleQuoteFG");
const QString SETTINGS_STYLE_QUOTEWT("StyleQuoteWeight");
const QString SETTINGS_STYLE_COMMENTFG("StyleCommentFG");
const QString SETTINGS_STYLE_COMMENTWT("StyleCommentWeight");
const QString SETTINGS_STYLE_SYMBOLFG("StyleSymbolFG");
const QString SETTINGS_STYLE_SYMBOLBG("StyleSymbolBG");
const QString SETTINGS_STYLE_SYMBOLWT("StyleSymbolWeight");

class QSettings;

class CodeEditorStyle {
public:
    explicit CodeEditorStyle(
        QColor ctrlFGColor, QColor ctrlBGColor,
        QColor hiLiBGColor,
        QColor lnNumFGColor, QColor lnNumBGColor,
        QColor keyWdFGColor, QFont::Weight keyWdWeight,
        QColor clsFGColor, QFont::Weight clsWeight,
        QColor qteFGColor, QFont::Weight qteWeight,
        QColor cmtFGColor, QFont::Weight cmtWeight,
        QColor symbFGColor, QColor symbBGColor,
        QFont::Weight symbWeight);

    bool operator==(const CodeEditorStyle& rhs) const;
    bool operator!=(const CodeEditorStyle& rhs) const;

    bool isSystemTheme() const {
        return mSystemTheme;
    }

    static CodeEditorStyle getSystemTheme();
    static CodeEditorStyle loadSettings(QSettings *settings);
    static void saveSettings(QSettings *settings, const CodeEditorStyle& theStyle);

public:
    bool mSystemTheme{};
    QColor widgetFGColor;
    QColor widgetBGColor;
    QColor highlightBGColor;
    QColor lineNumFGColor;
    QColor lineNumBGColor;
    QColor keywordColor;
    QFont::Weight keywordWeight;
    QColor classColor;
    QFont::Weight classWeight;
    QColor quoteColor;
    QFont::Weight quoteWeight;
    QColor commentColor;
    QFont::Weight commentWeight;
    QColor symbolFGColor;
    QColor symbolBGColor;
    QFont::Weight symbolWeight;
};

static const CodeEditorStyle defaultStyleLight(
    /* editor FG/BG */ Qt::black, QColor(240, 240, 240),
    /* highlight BG */ QColor(255, 220, 220),
    /* line number FG/BG */ Qt::black, QColor(240, 240, 240),
    /* keyword FG/Weight */ Qt::darkBlue, QFont::Bold,
    /* class FG/Weight */ Qt::darkMagenta, QFont::Bold,
    /* quote FG/Weight */ Qt::darkGreen, QFont::Normal,
    /* comment FG/Weight */ Qt::gray, QFont::Normal,
    /* Symbol FG/BG/Weight */ Qt::red, QColor(220, 220, 255), QFont::Normal
    );

// Styling derived from Eclipse Color Theme - 'RecognEyes'
// http://www.eclipsecolorthemes.org/?view=theme&id=30
static const CodeEditorStyle defaultStyleDark(
    /* editor FG/BG */ QColor(218, 218, 218), QColor(16, 16, 32),
    /* highlight BG */ QColor(64, 64, 64),
    /* line number FG/BG */ QColor(43, 145, 175), QColor(16, 16, 32),
    /* keyword FG/Weight */ QColor(0, 204, 204), QFont::Bold,
    /* class FG/Weight */ QColor(218, 0, 218), QFont::Bold,
    /* quote FG/Weight */ QColor(0, 204, 0), QFont::Normal,
    /* comment FG/Weight */ QColor(180, 180, 180), QFont::Normal,
    /* Symbol FG/BG/Weight */ QColor(218, 32, 32), QColor(32, 32, 108), QFont::Normal
    );

#endif /* CODEEDITORSTYLE_H */
