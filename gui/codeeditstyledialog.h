/* -*- C++ -*-
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

#ifndef CODEEDITSTYLEDIALOG_H
#define CODEEDITSTYLEDIALOG_H

#include "codeeditorstyle.h"

#include <QColor>
#include <QDialog>
#include <QFont>
#include <QObject>
#include <QStringList>

class CodeEditor;
class SelectColorButton;
class SelectFontWeightCombo;
class QPushButton;
class QWidget;
class QString;

class StyleEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit StyleEditDialog(const CodeEditorStyle& newStyle,
                             QWidget *parent = nullptr);

    CodeEditorStyle getStyle() const;

private:
    void updateControls();
    void updateStyle();

public slots:
    void resetStyle();
    void setStyleDefaultLight();
    void setStyleDefaultDark();
    void colorChangedWidgetFG(const QColor& newColor);
    void colorChangedWidgetBG(const QColor& newColor);
    void colorChangedHighlightBG(const QColor& newColor);
    void colorChangedLineNumFG(const QColor& newColor);
    void colorChangedLineNumBG(const QColor& newColor);
    void colorChangedKeywordFG(const QColor& newColor);
    void weightChangedKeyword(QFont::Weight newWeight);
    void colorChangedClassFG(const QColor& newColor);
    void weightChangedClass(QFont::Weight newWeight);
    void colorChangedQuoteFG(const QColor& newColor);
    void weightChangedQuote(QFont::Weight newWeight);
    void colorChangedCommentFG(const QColor& newColor);
    void weightChangedComment(QFont::Weight newWeight);
    void colorChangedSymbolFG(const QColor& newColor);
    void colorChangedSymbolBG(const QColor& newColor);
    void weightChangedSymbol(QFont::Weight newWeight);

private:
    CodeEditorStyle mStyleIncoming;
    CodeEditorStyle mStyleOutgoing;

    CodeEditor              *mSampleEditor;

    SelectColorButton       *mBtnWidgetColorFG;
    SelectColorButton       *mBtnWidgetColorBG;
    SelectColorButton       *mBtnHighlightBG;
    SelectColorButton       *mBtnLineNumFG;
    SelectColorButton       *mBtnLineNumBG;
    SelectColorButton       *mBtnKeywordFG;
    SelectFontWeightCombo   *mCBKeywordWeight;
    SelectColorButton       *mBtnClassFG;
    SelectFontWeightCombo   *mCBClassWeight;
    SelectColorButton       *mBtnQuoteFG;
    SelectFontWeightCombo   *mCBQuoteWeight;
    SelectColorButton       *mBtnCommentFG;
    SelectFontWeightCombo   *mCBCommentWeight;
    SelectColorButton       *mBtnSymbolFG;
    SelectColorButton       *mBtnSymbolBG;
    SelectFontWeightCombo   *mCBSymbolWeight;

    QPushButton             *mBtnDefaultLight;
    QPushButton             *mBtnDefaultDark;

    static const QString mSampleDocument;
    static const QStringList mErrSymbolsList;
    static const int mErrLineNum;
};

#endif  //CODEEDITSTYLEDIALOG_H

