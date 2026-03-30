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

#include "codeeditorstyle.h"

#include <QList>
#include <QSettings>
#include <QVariant>

CodeEditorStyle::CodeEditorStyle(
    QColor ctrlFGColor, QColor ctrlBGColor,
    QColor hiLiBGColor,
    QColor lnNumFGColor, QColor lnNumBGColor,
    QColor keyWdFGColor, QFont::Weight keyWdWeight,
    QColor clsFGColor, QFont::Weight clsWeight,
    QColor qteFGColor, QFont::Weight qteWeight,
    QColor cmtFGColor, QFont::Weight cmtWeight,
    QColor symbFGColor, QColor symbBGColor,
    QFont::Weight symbWeight) :
    widgetFGColor(ctrlFGColor),
    widgetBGColor(ctrlBGColor),
    highlightBGColor(hiLiBGColor),
    lineNumFGColor(lnNumFGColor),
    lineNumBGColor(lnNumBGColor),
    keywordColor(keyWdFGColor),
    keywordWeight(keyWdWeight),
    classColor(clsFGColor),
    classWeight(clsWeight),
    quoteColor(qteFGColor),
    quoteWeight(qteWeight),
    commentColor(cmtFGColor),
    commentWeight(cmtWeight),
    symbolFGColor(symbFGColor),
    symbolBGColor(symbBGColor),
    symbolWeight(symbWeight)
{}

bool CodeEditorStyle::operator==(const CodeEditorStyle& rhs) const
{
    if (mSystemTheme != rhs.mSystemTheme) return false;
    if (widgetFGColor != rhs.widgetFGColor) return false;
    if (widgetBGColor != rhs.widgetBGColor) return false;
    if (highlightBGColor != rhs.highlightBGColor) return false;
    if (lineNumFGColor != rhs.lineNumFGColor) return false;
    if (lineNumBGColor != rhs.lineNumBGColor) return false;
    if (keywordColor != rhs.keywordColor) return false;
    if (keywordWeight != rhs.keywordWeight) return false;
    if (classColor != rhs.classColor) return false;
    if (classWeight != rhs.classWeight) return false;
    if (quoteColor != rhs.quoteColor) return false;
    if (quoteWeight != rhs.quoteWeight) return false;
    if (commentColor != rhs.commentColor) return false;
    if (commentWeight != rhs.commentWeight) return false;
    if (symbolFGColor != rhs.symbolFGColor) return false;
    if (symbolBGColor != rhs.symbolBGColor) return false;
    if (symbolWeight != rhs.symbolWeight) return false;
    return true;
}

bool CodeEditorStyle::operator!=(const CodeEditorStyle& rhs) const
{
    return !(*this == rhs);
}

CodeEditorStyle CodeEditorStyle::getSystemTheme()
{
    CodeEditorStyle theStyle(defaultStyleLight);
    theStyle.mSystemTheme = true;
    return theStyle;
}

CodeEditorStyle CodeEditorStyle::loadSettings(QSettings *settings)
{
    CodeEditorStyle theStyle(CodeEditorStyle::getSystemTheme());
    if (!settings)
        return theStyle;

    if (!settings->childGroups().contains(SETTINGS_STYLE_GROUP))
        return theStyle;

    // style section exists - load values
    settings->beginGroup(SETTINGS_STYLE_GROUP);
    QString type = settings->value(
        SETTINGS_STYLE_TYPE,
        QVariant(SETTINGS_STYLE_TYPE_LIGHT)
        ).toString();
    if (type == SETTINGS_STYLE_TYPE_LIGHT) {
        settings->endGroup();
        return theStyle;
    }
    if (type == SETTINGS_STYLE_TYPE_DARK) {
        theStyle = defaultStyleDark;
        settings->endGroup();
        return theStyle;
    }
    if (type == SETTINGS_STYLE_TYPE_CUSTOM) {
        theStyle.widgetFGColor = settings->value(
            SETTINGS_STYLE_WIDGETFG,
            QVariant(defaultStyleLight.widgetFGColor)).value<QColor>();
        theStyle.widgetBGColor = settings->value(
            SETTINGS_STYLE_WIDGETBG,
            QVariant(defaultStyleLight.widgetBGColor)).value<QColor>();
        theStyle.highlightBGColor = settings->value(
            SETTINGS_STYLE_HILIFG,
            QVariant(defaultStyleLight.highlightBGColor)).value<QColor>();
        theStyle.lineNumFGColor = settings->value(
            SETTINGS_STYLE_LINENUMFG,
            QVariant(defaultStyleLight.lineNumFGColor)).value<QColor>();
        theStyle.lineNumBGColor = settings->value(
            SETTINGS_STYLE_LINENUMBG,
            QVariant(defaultStyleLight.lineNumBGColor)).value<QColor>();
        theStyle.keywordColor = settings->value(
            SETTINGS_STYLE_KEYWORDFG,
            QVariant(defaultStyleLight.keywordColor)).value<QColor>();
        QVariant defKeyWWt(static_cast<int>(defaultStyleLight.keywordWeight));
        theStyle.keywordWeight = static_cast<QFont::Weight>(
            settings->value(SETTINGS_STYLE_KEYWORDWT, defKeyWWt).toInt());
        theStyle.classColor = settings->value(
            SETTINGS_STYLE_CLASSFG,
            QVariant(defaultStyleLight.classColor)).value<QColor>();
        QVariant defClsWt(static_cast<int>(defaultStyleLight.classWeight));
        theStyle.classWeight = static_cast<QFont::Weight>(
            settings->value(SETTINGS_STYLE_CLASSWT, defClsWt).toInt());
        theStyle.quoteColor = settings->value(
            SETTINGS_STYLE_QUOTEFG,
            QVariant(defaultStyleLight.quoteColor)).value<QColor>();
        QVariant defQteWt(static_cast<int>(defaultStyleLight.quoteWeight));
        theStyle.quoteWeight = static_cast<QFont::Weight>(
            settings->value(SETTINGS_STYLE_QUOTEWT, defQteWt).toInt());
        theStyle.commentColor = settings->value(
            SETTINGS_STYLE_COMMENTFG,
            QVariant(defaultStyleLight.commentColor)).value<QColor>();
        QVariant defCmtWt(static_cast<int>(defaultStyleLight.commentWeight));
        theStyle.commentWeight = static_cast<QFont::Weight>(
            settings->value(SETTINGS_STYLE_COMMENTWT, defCmtWt).toInt());
        theStyle.symbolFGColor = settings->value(
            SETTINGS_STYLE_SYMBOLFG,
            QVariant(defaultStyleLight.symbolFGColor)).value<QColor>();
        theStyle.symbolBGColor = settings->value(
            SETTINGS_STYLE_SYMBOLBG,
            QVariant(defaultStyleLight.symbolBGColor)).value<QColor>();
        QVariant defSymWt(static_cast<int>(defaultStyleLight.symbolWeight));
        theStyle.symbolWeight = static_cast<QFont::Weight>(
            settings->value(SETTINGS_STYLE_SYMBOLWT, defSymWt).toInt());
    }
    settings->endGroup();
    return theStyle;
}

void CodeEditorStyle::saveSettings(QSettings *settings,
                                   const CodeEditorStyle& theStyle)
{
    if (!settings)
        return;

    if (settings->childGroups().contains(SETTINGS_STYLE_GROUP)) {
        settings->remove(SETTINGS_STYLE_GROUP);
        if (theStyle.isSystemTheme())
            return;
    }

    settings->beginGroup(SETTINGS_STYLE_GROUP);
    const bool isDefaultLight = (defaultStyleLight == theStyle);
    const bool isDefaultDark = (defaultStyleDark == theStyle);
    if (isDefaultLight && !isDefaultDark) {
        settings->setValue(SETTINGS_STYLE_TYPE,
                           SETTINGS_STYLE_TYPE_LIGHT);
    } else if (!isDefaultLight && isDefaultDark) {
        settings->setValue(SETTINGS_STYLE_TYPE,
                           SETTINGS_STYLE_TYPE_DARK);
    } else {
        settings->setValue(SETTINGS_STYLE_TYPE,
                           SETTINGS_STYLE_TYPE_CUSTOM);
        settings->setValue(SETTINGS_STYLE_WIDGETFG,
                           QVariant(theStyle.widgetFGColor));
        settings->setValue(SETTINGS_STYLE_WIDGETBG,
                           QVariant(theStyle.widgetBGColor));
        settings->setValue(SETTINGS_STYLE_HILIFG,
                           QVariant(theStyle.highlightBGColor));
        settings->setValue(SETTINGS_STYLE_LINENUMFG,
                           QVariant(theStyle.lineNumFGColor));
        settings->setValue(SETTINGS_STYLE_LINENUMBG,
                           QVariant(theStyle.lineNumBGColor));
        settings->setValue(SETTINGS_STYLE_KEYWORDFG,
                           QVariant(theStyle.keywordColor));
        settings->setValue(SETTINGS_STYLE_KEYWORDWT,
                           QVariant(static_cast<int>(theStyle.keywordWeight)));
        settings->setValue(SETTINGS_STYLE_CLASSFG,
                           QVariant(theStyle.classColor));
        settings->setValue(SETTINGS_STYLE_CLASSWT,
                           QVariant(static_cast<int>(theStyle.classWeight)));
        settings->setValue(SETTINGS_STYLE_QUOTEFG,
                           QVariant(theStyle.quoteColor));
        settings->setValue(SETTINGS_STYLE_QUOTEWT,
                           QVariant(static_cast<int>(theStyle.quoteWeight)));
        settings->setValue(SETTINGS_STYLE_COMMENTFG,
                           QVariant(theStyle.commentColor));
        settings->setValue(SETTINGS_STYLE_COMMENTWT,
                           QVariant(static_cast<int>(theStyle.commentWeight)));
        settings->setValue(SETTINGS_STYLE_SYMBOLFG,
                           QVariant(theStyle.symbolFGColor));
        settings->setValue(SETTINGS_STYLE_SYMBOLBG,
                           QVariant(theStyle.symbolBGColor));
        settings->setValue(SETTINGS_STYLE_SYMBOLWT,
                           QVariant(static_cast<int>(theStyle.symbolWeight)));
    }
    settings->endGroup();
}
