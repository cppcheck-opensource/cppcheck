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

#ifndef LIBRARYDIALOG_H
#define LIBRARYDIALOG_H

#include "cppchecklibrarydata.h"

#include <QDialog>
#include <QObject>
#include <QString>

class QListWidgetItem;
class QWidget;
namespace Ui {
    class LibraryDialog;
}

class LibraryDialog : public QDialog {
    Q_OBJECT

public:
    explicit LibraryDialog(QWidget *parent = nullptr);
    LibraryDialog(const LibraryDialog &) = delete;
    ~LibraryDialog() override;
    LibraryDialog &operator=(const LibraryDialog &) = delete;

private slots:
    void openCfg();
    void saveCfg();
    void saveCfgAs();
    void addFunction();
    void changeFunction();
    void editArg();
    void editFunctionName(QListWidgetItem* item);
    void filterFunctions(const QString& filter);
    void selectFunction();
    void sortFunctions(bool sort);

private:
    Ui::LibraryDialog *mUi;
    CppcheckLibraryData mData;
    QString mFileName;
    bool mIgnoreChanges{};

    static QString getArgText(const CppcheckLibraryData::Function::Arg &arg);
    CppcheckLibraryData::Function *currentFunction();
    void updateArguments(const CppcheckLibraryData::Function &function);
};

#endif // LIBRARYDIALOG_H
