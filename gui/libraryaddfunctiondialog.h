/* -*- C++ -*-
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

#ifndef LIBRARYADDFUNCTIONDIALOG_H
#define LIBRARYADDFUNCTIONDIALOG_H

#include <QDialog>
#include <QObject>
#include <QString>

class QWidget;
namespace Ui {
    class LibraryAddFunctionDialog;
}

#define SIMPLENAME     "[_a-zA-Z][_a-zA-Z0-9]*"            // just a name
#define SCOPENAME      SIMPLENAME "(::" SIMPLENAME ")*"    // names with optional scope
#define NAMES          SCOPENAME "(," SCOPENAME ")*"       // names can be separated by comma

class LibraryAddFunctionDialog : public QDialog {
    Q_OBJECT

public:
    explicit LibraryAddFunctionDialog(QWidget *parent = nullptr);
    LibraryAddFunctionDialog(const LibraryAddFunctionDialog &) = delete;
    ~LibraryAddFunctionDialog() override;
    LibraryAddFunctionDialog &operator=(const LibraryAddFunctionDialog &) = delete;

    QString functionName() const;
    int     numberOfArguments() const;

private:
    Ui::LibraryAddFunctionDialog *mUi;
};

#endif // LIBRARYADDFUNCTIONDIALOG_H
