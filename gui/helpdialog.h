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

#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QObject>
#include <QTextBrowser>

class QHelpEngine;
class QWidget;
namespace Ui {
    class HelpDialog;
}

class HelpBrowser : public QTextBrowser {
public:
    explicit HelpBrowser(QWidget* parent = nullptr) : QTextBrowser(parent) {}
    HelpBrowser(const HelpBrowser&) = delete;
    HelpBrowser(HelpBrowser&&) = delete;
    HelpBrowser& operator=(const HelpBrowser&) = delete;
    HelpBrowser& operator=(HelpBrowser&&) = delete;
    void setHelpEngine(QHelpEngine *helpEngine);
    QVariant loadResource(int type, const QUrl& name) override;
private:
    QHelpEngine* mHelpEngine{};
};

class HelpDialog : public QDialog {
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);
    ~HelpDialog() override;

private:
    Ui::HelpDialog *mUi;
    QHelpEngine* mHelpEngine;
};

#endif // HELPDIALOG_H
