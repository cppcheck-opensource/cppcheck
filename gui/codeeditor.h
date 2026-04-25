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

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <cstdint>

#include <QList>
#include <QObject>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QWidget>

class CodeEditorStyle;
class QPaintEvent;
class QRect;
class QTextDocument;

class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent,
                         CodeEditorStyle *widgetStyle);

    void setSymbols(const QStringList &symbols);

    void setStyle(const CodeEditorStyle &newStyle);

protected:
    void highlightBlock(const QString &text) override;

private:
    enum RuleRole : std::uint8_t {
        Keyword = 1,
        Class   = 2,
        Comment = 3,
        Quote   = 4,
        Symbol  = 5
    };
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
        RuleRole ruleRole;
    };

    void applyFormat(HighlightingRule &rule) const;

    QList<HighlightingRule> mHighlightingRules;
    QList<HighlightingRule> mHighlightingRulesWithSymbols;

    QRegularExpression mCommentStartExpression;
    QRegularExpression mCommentEndExpression;

    QTextCharFormat mKeywordFormat;
    QTextCharFormat mClassFormat;
    QTextCharFormat mSingleLineCommentFormat;
    QTextCharFormat mMultiLineCommentFormat;
    QTextCharFormat mQuotationFormat;
    QTextCharFormat mSymbolFormat;

    CodeEditorStyle *mWidgetStyle;
};

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent);
    CodeEditor(const CodeEditor &) = delete;
    CodeEditor &operator=(const CodeEditor &) = delete;
    ~CodeEditor() override;

    void lineNumberAreaPaintEvent(const QPaintEvent *event);
    int lineNumberAreaWidth();
    void setStyle(const CodeEditorStyle& newStyle);

    /**
     * Set source code to show, goto error line and highlight that line.
     * \param code         The source code.
     * \param errorLine    line number
     * \param symbols      the related symbols, these are marked
     */
    void setError(const QString &code, int errorLine, const QStringList &symbols);

    /**
     * Goto another error in existing source file
     * \param errorLine    line number
     * \param symbols      the related symbols, these are marked
     */
    void setError(int errorLine, const QStringList &symbols);

    void setFileName(const QString &fileName) {
        mFileName = fileName;
    }

    const QString& getFileName() const {
        return mFileName;
    }

    // NOLINTNEXTLINE(bugprone-derived-method-shadowing-base-method) - TODO: fix this
    void clear() {
        mFileName.clear();
        setPlainText(QString());
    }

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int /*newBlockCount*/);
    void highlightErrorLine();
    void updateLineNumberArea(const QRect& rect, int dy);

private:
    QString generateStyleString();

private:
    QWidget *mLineNumberArea;
    Highlighter *mHighlighter;
    CodeEditorStyle *mWidgetStyle;
    int mErrorPosition;
    QString mFileName;
};


class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        mCodeEditor = editor;
    }

    QSize sizeHint() const override {
        return QSize(mCodeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        mCodeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *mCodeEditor;
};

#endif // CODEEDITOR_H
