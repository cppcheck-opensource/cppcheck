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

#include "librarydialog.h"

#include "common.h"
#include "libraryaddfunctiondialog.h"
#include "libraryeditargdialog.h"
#include "path.h"
#include "utils.h"

#include "ui_librarydialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QIODevice>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextStream>
#include <Qt>

class QWidget;

// TODO: get/compare functions from header

namespace {
    class FunctionListItem : public QListWidgetItem {
    public:
        FunctionListItem(QListWidget *view,
                         CppcheckLibraryData::Function *function,
                         bool selected)
            : QListWidgetItem(view), function(function) {
            setText(function->name);
            setFlags(flags() | Qt::ItemIsEditable);
            setSelected(selected);
        }
        CppcheckLibraryData::Function *function;
    };
}

LibraryDialog::LibraryDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::LibraryDialog)
{
    mUi->setupUi(this);
    mUi->buttonSave->setEnabled(false);
    mUi->buttonSaveAs->setEnabled(false);
    mUi->sortFunctions->setEnabled(false);
    mUi->filter->setEnabled(false);
    mUi->addFunction->setEnabled(false);

    //As no function selected, this disables function editing widgets
    selectFunction();
}

LibraryDialog::~LibraryDialog()
{
    delete mUi;
}

CppcheckLibraryData::Function *LibraryDialog::currentFunction()
{
    QList<QListWidgetItem *> selitems = mUi->functions->selectedItems();
    if (selitems.count() != 1)
        return nullptr;
    return dynamic_cast<FunctionListItem *>(selitems.first())->function;
}

void LibraryDialog::openCfg()
{
    const QString datadir = getDataDir();

    QString selectedFilter;
    const QString filter(tr("Library files (*.cfg)"));
    const QString selectedFile = QFileDialog::getOpenFileName(this,
                                                              tr("Open library file"),
                                                              datadir,
                                                              filter,
                                                              &selectedFilter);

    if (selectedFile.isEmpty())
        return;

    QFile file(selectedFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("Cannot open file %1.").arg(selectedFile),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }

    CppcheckLibraryData tempdata;
    const QString errmsg = tempdata.open(file);
    if (!errmsg.isNull()) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("Failed to load %1. %2.").arg(selectedFile).arg(errmsg),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }

    mIgnoreChanges = true;
    mData.swap(tempdata);
    mFileName = selectedFile;
    mUi->buttonSave->setEnabled(false);
    mUi->buttonSaveAs->setEnabled(true);
    mUi->filter->clear();
    mUi->functions->clear();
    for (CppcheckLibraryData::Function &function : mData.functions) {
        mUi->functions->addItem(new FunctionListItem(mUi->functions,
                                                     &function,
                                                     false));
    }
    mUi->sortFunctions->setEnabled(!mData.functions.empty());
    mUi->filter->setEnabled(!mData.functions.empty());
    mUi->addFunction->setEnabled(true);
    mIgnoreChanges = false;
}

void LibraryDialog::saveCfg()
{
    if (mFileName.isNull())
        return;
    QFile file(mFileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&file);
        ts << mData.toString() << '\n';
        mUi->buttonSave->setEnabled(false);
    } else {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("Cannot save file %1.").arg(mFileName),
                        QMessageBox::Ok,
                        this);
        msg.exec();
    }
}

void LibraryDialog::saveCfgAs()
{
    const QString filter(tr("Library files (*.cfg)"));
    const QString path = QString::fromStdString(Path::getPathFromFilename(mFileName.toStdString()));
    QString selectedFile = QFileDialog::getSaveFileName(this,
                                                        tr("Save the library as"),
                                                        path,
                                                        filter);
    if (selectedFile.isEmpty())
        return;

    if (!selectedFile.endsWith(".cfg", Qt::CaseInsensitive))
        selectedFile += ".cfg";

    mFileName = selectedFile;
    saveCfg();
}

void LibraryDialog::addFunction()
{
    auto *d = new LibraryAddFunctionDialog;
    if (d->exec() == QDialog::Accepted && !d->functionName().isEmpty()) {

        CppcheckLibraryData::Function f;
        f.name = d->functionName();
        const int args = d->numberOfArguments();

        for (int i = 1; i <= args; i++) {
            CppcheckLibraryData::Function::Arg arg;
            arg.nr = i;
            f.args.append(arg);
        }
        mData.functions.append(f);
        mUi->functions->addItem(new FunctionListItem(mUi->functions, &mData.functions.back(), false));
        mUi->buttonSave->setEnabled(true);
        mUi->sortFunctions->setEnabled(!mData.functions.empty());
        mUi->filter->setEnabled(!mData.functions.empty());
    }
    delete d;
}

void LibraryDialog::editFunctionName(QListWidgetItem* item)
{
    if (mIgnoreChanges)
        return;
    QString functionName = item->text();
    CppcheckLibraryData::Function * const function = dynamic_cast<FunctionListItem*>(item)->function;
    if (functionName != function->name) {
        const QRegularExpressionMatch matchRes = QRegularExpression("^" NAMES "$").match(functionName);
        if (matchRes.hasMatch()) {
            function->name = functionName;
            mUi->buttonSave->setEnabled(true);
        } else {
            mIgnoreChanges = true;
            item->setText(function->name);
            mIgnoreChanges = false;
        }
    }
}

void LibraryDialog::selectFunction()
{
    const CppcheckLibraryData::Function * const function = currentFunction();

    if (function == nullptr) {
        mUi->comments->clear();
        mUi->comments->setEnabled(false);

        mUi->noreturn->setCurrentIndex(0);
        mUi->noreturn->setEnabled(false);

        mUi->useretval->setChecked(false);
        mUi->useretval->setEnabled(false);

        mUi->leakignore->setChecked(false);
        mUi->leakignore->setEnabled(false);

        mUi->arguments->clear();
        mUi->arguments->setEnabled(false);

        mUi->editArgButton->setEnabled(false);
        return;
    }

    mIgnoreChanges = true;
    mUi->comments->setPlainText(function->comments);
    mUi->comments->setEnabled(true);

    mUi->noreturn->setCurrentIndex(function->noreturn);
    mUi->noreturn->setEnabled(true);

    mUi->useretval->setChecked(function->useretval);
    mUi->useretval->setEnabled(true);

    mUi->leakignore->setChecked(function->leakignore);
    mUi->leakignore->setEnabled(true);

    updateArguments(*function);
    mUi->arguments->setEnabled(true);

    mUi->editArgButton->setEnabled(true);
    mIgnoreChanges = false;
}

void LibraryDialog::sortFunctions(bool sort)
{
    if (sort) {
        mUi->functions->sortItems();
    } else {
        mIgnoreChanges = true;
        const CppcheckLibraryData::Function* selfunction = currentFunction();
        mUi->functions->clear();
        for (CppcheckLibraryData::Function &function : mData.functions) {
            mUi->functions->addItem(new FunctionListItem(mUi->functions,
                                                         &function,
                                                         selfunction == &function));
        }
        if (!mUi->filter->text().isEmpty())
            filterFunctions(mUi->filter->text());
        mIgnoreChanges = false;
    }
}

void LibraryDialog::filterFunctions(const QString& filter)
{
    QList<QListWidgetItem *> allItems = mUi->functions->findItems(QString(), Qt::MatchContains);

    if (filter.isEmpty()) {
        for (QListWidgetItem *item : utils::as_const(allItems)) {
            item->setHidden(false);
        }
    } else {
        for (QListWidgetItem *item : utils::as_const(allItems)) {
            item->setHidden(!item->text().startsWith(filter));
        }
    }
}

void LibraryDialog::changeFunction()
{
    if (mIgnoreChanges)
        return;

    CppcheckLibraryData::Function *function = currentFunction();
    if (!function)
        return;

    function->comments   = mUi->comments->toPlainText();
    function->noreturn   = static_cast<CppcheckLibraryData::Function::TrueFalseUnknown>(mUi->noreturn->currentIndex());
    function->useretval  = mUi->useretval->isChecked();
    function->leakignore = mUi->leakignore->isChecked();

    mUi->buttonSave->setEnabled(true);
}

void LibraryDialog::editArg()
{
    CppcheckLibraryData::Function *function = currentFunction();
    if (!function)
        return;

    if (mUi->arguments->selectedItems().count() != 1)
        return;
    CppcheckLibraryData::Function::Arg &arg = function->args[mUi->arguments->row(mUi->arguments->selectedItems().first())];

    LibraryEditArgDialog d(nullptr, arg);
    if (d.exec() == QDialog::Accepted) {
        const unsigned number = arg.nr;
        arg = d.getArg();
        arg.nr = number;
        mUi->arguments->selectedItems().first()->setText(getArgText(arg));
    }
    mUi->buttonSave->setEnabled(true);
}

QString LibraryDialog::getArgText(const CppcheckLibraryData::Function::Arg &arg)
{
    QString s("arg");
    if (arg.nr != CppcheckLibraryData::Function::Arg::ANY)
        s += QString::number(arg.nr);

    s += "\n    not bool: " + QString(bool_to_string(arg.notbool));
    s += "\n    not null: " +  QString(bool_to_string(arg.notnull));
    s += "\n    not uninit: " +  QString(bool_to_string(arg.notuninit));
    s += "\n    format string: " +  QString(bool_to_string(arg.formatstr));
    s += "\n    strz: " +  QString(bool_to_string(arg.strz));
    s += "\n    valid: " + (arg.valid.isEmpty() ? "any" : arg.valid);
    for (const CppcheckLibraryData::Function::Arg::MinSize &minsize : arg.minsizes) {
        s += "\n    minsize: " + minsize.type + " " + minsize.arg + " " + minsize.arg2;
    }
    return s;
}

void LibraryDialog::updateArguments(const CppcheckLibraryData::Function &function)
{
    mUi->arguments->clear();
    for (const CppcheckLibraryData::Function::Arg &arg : function.args) {
        mUi->arguments->addItem(getArgText(arg));
    }
}
