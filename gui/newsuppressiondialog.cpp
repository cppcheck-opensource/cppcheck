/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "newsuppressiondialog.h"

#include "cppcheck.h"
#include "errorlogger.h"
#include "suppressions.h"

#include "ui_newsuppressiondialog.h"

#include <string>

#include <QComboBox>
#include <QLineEdit>
#include <QString>
#include <QStringList>

class QWidget;

NewSuppressionDialog::NewSuppressionDialog(QWidget *parent) :
    QDialog(parent),
    mUI(new Ui::NewSuppressionDialog)
{
    mUI->setupUi(this);

    class QErrorLogger : public ErrorLogger {
    public:
        void reportOut(const std::string & /*outmsg*/, Color /*c*/) override {}
        void reportErr(const ErrorMessage &msg) override {
            errorIds << QString::fromStdString(msg.id);
        }
        void reportMetric(const std::string &metric) override {
            (void) metric;
        }
        QStringList errorIds;
    };

    QErrorLogger errorLogger;
    CppCheck::getErrorMessages(errorLogger);
    errorLogger.errorIds.sort();

    mUI->mComboErrorId->addItems(errorLogger.errorIds);
    mUI->mComboErrorId->setCurrentIndex(-1);
    mUI->mComboErrorId->setCurrentText("");
}

NewSuppressionDialog::~NewSuppressionDialog()
{
    delete mUI;
}

SuppressionList::Suppression NewSuppressionDialog::getSuppression() const
{
    SuppressionList::Suppression ret;
    ret.errorId = mUI->mComboErrorId->currentText().toStdString();
    if (ret.errorId.empty())
        ret.errorId = "*";
    ret.fileName = mUI->mTextFileName->text().toStdString();
    if (!mUI->mTextLineNumber->text().isEmpty())
        ret.lineNumber = mUI->mTextLineNumber->text().toInt();
    ret.symbolName = mUI->mTextSymbolName->text().toStdString();
    return ret;
}

void NewSuppressionDialog::setSuppression(const SuppressionList::Suppression &suppression)
{
    setWindowTitle(tr("Edit suppression"));
    mUI->mComboErrorId->setCurrentText(QString::fromStdString(suppression.errorId));
    mUI->mTextFileName->setText(QString::fromStdString(suppression.fileName));
    mUI->mTextLineNumber->setText(suppression.lineNumber > 0 ? QString::number(suppression.lineNumber) : QString());
    mUI->mTextSymbolName->setText(QString::fromStdString(suppression.symbolName));
}
