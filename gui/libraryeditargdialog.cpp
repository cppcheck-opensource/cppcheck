/*
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

#include "libraryeditargdialog.h"

#include "ui_libraryeditargdialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QString>
#include <QStringList>

class QWidget;

LibraryEditArgDialog::LibraryEditArgDialog(QWidget *parent, const CppcheckLibraryData::Function::Arg &arg) :
    QDialog(parent),
    mUi(new Ui::LibraryEditArgDialog),
    mMinSizes(arg.minsizes)
{
    mUi->setupUi(this);
    mUi->notbool->setChecked(arg.notbool);
    mUi->notnull->setChecked(arg.notnull);
    mUi->notuninit->setChecked(arg.notuninit);
    mUi->strz->setChecked(arg.strz);
    mUi->formatstr->setChecked(arg.formatstr);
    mUi->valid->setText(arg.valid);

    mUi->minsize1type->setEnabled(true);
    mUi->minsize1arg->setEnabled(arg.minsizes.count() >= 1);
    mUi->minsize1arg2->setEnabled(arg.minsizes.count() >= 1 && arg.minsizes[0].type == "mul");
    mUi->minsize2type->setEnabled(arg.minsizes.count() >= 1);
    mUi->minsize2arg->setEnabled(arg.minsizes.count() >= 2);
    mUi->minsize2arg2->setEnabled(arg.minsizes.count() >= 2 && arg.minsizes[1].type == "mul");

    QStringList items;
    items << "None" << "argvalue" << "mul" << "sizeof" << "strlen";

    mUi->minsize1type->clear();
    mUi->minsize1type->addItems(items);
    if (arg.minsizes.count() >= 1) {
        mUi->minsize1type->setCurrentIndex(items.indexOf(mMinSizes[0].type));
        mUi->minsize1arg->setValue(mMinSizes[0].arg.toInt());
        if (arg.minsizes[0].type == "mul")
            mUi->minsize1arg2->setValue(mMinSizes[0].arg2.toInt());
        else
            mUi->minsize1arg2->setValue(0);
    } else {
        mUi->minsize1type->setCurrentIndex(0);
        mUi->minsize1arg->setValue(0);
        mUi->minsize1arg2->setValue(0);
    }

    mUi->minsize2type->clear();
    mUi->minsize2type->addItems(items);
    if (arg.minsizes.count() >= 2) {
        mUi->minsize2type->setCurrentIndex(items.indexOf(mMinSizes[1].type));
        mUi->minsize2arg->setValue(mMinSizes[1].arg.toInt());
        if (arg.minsizes[1].type == "mul")
            mUi->minsize2arg2->setValue(mMinSizes[1].arg2.toInt());
        else
            mUi->minsize2arg2->setValue(0);
    } else {
        mUi->minsize2type->setCurrentIndex(0);
        mUi->minsize2arg->setValue(0);
        mUi->minsize2arg2->setValue(0);
    }
}

LibraryEditArgDialog::~LibraryEditArgDialog()
{
    delete mUi;
}

CppcheckLibraryData::Function::Arg LibraryEditArgDialog::getArg() const
{
    CppcheckLibraryData::Function::Arg ret;
    ret.notbool   = mUi->notbool->isChecked();
    ret.notnull   = mUi->notnull->isChecked();
    ret.notuninit = mUi->notuninit->isChecked();
    ret.strz      = mUi->strz->isChecked();
    ret.formatstr = mUi->formatstr->isChecked();
    if (mUi->minsize1type->currentIndex() != 0) {
        CppcheckLibraryData::Function::Arg::MinSize minsize1;
        minsize1.type = mUi->minsize1type->currentText();
        minsize1.arg  = QString::number(mUi->minsize1arg->value());
        if (minsize1.type == "mul")
            minsize1.arg2 = QString::number(mUi->minsize1arg2->value());
        ret.minsizes.append(minsize1);

        if (mUi->minsize2type->currentIndex() != 0) {
            CppcheckLibraryData::Function::Arg::MinSize minsize2;
            minsize2.type = mUi->minsize2type->currentText();
            minsize2.arg  = QString::number(mUi->minsize2arg->value());
            if (minsize2.type == "mul")
                minsize2.arg2 = QString::number(mUi->minsize2arg2->value());
            ret.minsizes.append(minsize2);
        }
    }
    ret.valid     = mUi->valid->text();
    return ret;
}

void LibraryEditArgDialog::minsizeChanged()
{
    mUi->minsize1arg->setEnabled(mUi->minsize1type->currentIndex() != 0);
    mUi->minsize1arg2->setEnabled(mUi->minsize1type->currentText() == "mul");
    mUi->minsize2type->setEnabled(mUi->minsize1type->currentIndex() != 0);
    mUi->minsize2arg->setEnabled(mUi->minsize2type->currentIndex() != 0);
    mUi->minsize2arg2->setEnabled(mUi->minsize2type->currentText() == "mul");
}
