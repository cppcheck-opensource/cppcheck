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

#ifndef LIBRARYEDITARGDIALOG_H
#define LIBRARYEDITARGDIALOG_H

#include "cppchecklibrarydata.h"

#include <QDialog>
#include <QList>
#include <QObject>

class QWidget;
namespace Ui {
    class LibraryEditArgDialog;
}

class LibraryEditArgDialog : public QDialog {
    Q_OBJECT

public:
    LibraryEditArgDialog(QWidget *parent, const CppcheckLibraryData::Function::Arg &arg);
    LibraryEditArgDialog(const LibraryEditArgDialog &) = delete;
    ~LibraryEditArgDialog() override;
    LibraryEditArgDialog &operator=(const LibraryEditArgDialog &) = delete;

    CppcheckLibraryData::Function::Arg getArg() const;

private slots:
    void minsizeChanged();

private:
    Ui::LibraryEditArgDialog *mUi;

    QList<CppcheckLibraryData::Function::Arg::MinSize> mMinSizes;
};

#endif // LIBRARYEDITARGDIALOG_H
