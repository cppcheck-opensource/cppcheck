/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "libraryaddfunctiondialog.h"

#include "ui_libraryaddfunctiondialog.h"

#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSpinBox>

class QWidget;

LibraryAddFunctionDialog::LibraryAddFunctionDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::LibraryAddFunctionDialog)
{
    mUi->setupUi(this);
    static const QRegularExpression rx(NAMES);
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    mUi->functionName->setValidator(validator);
}

LibraryAddFunctionDialog::~LibraryAddFunctionDialog()
{
    delete mUi;
}

QString LibraryAddFunctionDialog::functionName() const
{
    return mUi->functionName->text();
}

int LibraryAddFunctionDialog::numberOfArguments() const
{
    return mUi->numberOfArguments->value();
}

