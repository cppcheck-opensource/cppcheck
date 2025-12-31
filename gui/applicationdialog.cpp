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

#include "applicationdialog.h"

#include "application.h"
#include "common.h"

#include "ui_applicationdialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>


ApplicationDialog::ApplicationDialog(const QString &title,
                                     Application &app,
                                     QWidget *parent) :
    QDialog(parent),
    mUI(new Ui::ApplicationDialog),
    mApplication(app)
{
    mUI->setupUi(this);

    connect(mUI->mButtonBrowse, &QPushButton::clicked, this, &ApplicationDialog::browse);
    connect(mUI->mButtons, &QDialogButtonBox::accepted, this, &ApplicationDialog::ok);
    connect(mUI->mButtons, &QDialogButtonBox::rejected, this, &ApplicationDialog::reject);
    mUI->mPath->setText(app.getPath());
    mUI->mName->setText(app.getName());
    mUI->mParameters->setText(app.getParameters());
    setWindowTitle(title);
    adjustSize();
}


ApplicationDialog::~ApplicationDialog()
{
    delete mUI;
}

void ApplicationDialog::browse()
{
    QString filter;
#ifdef Q_OS_WIN
    // In Windows (almost) all executables have .exe extension
    // so it does not make sense to show everything.
    filter += tr("Executable files (*.exe);;All files(*.*)");
#endif // Q_OS_WIN
    QString selectedFile = QFileDialog::getOpenFileName(this,
                                                        tr("Select viewer application"),
                                                        getPath(SETTINGS_LAST_APP_PATH),
                                                        filter);

    if (!selectedFile.isEmpty()) {
        setPath(SETTINGS_LAST_APP_PATH, selectedFile);
        QString path(QDir::toNativeSeparators(selectedFile));
        mUI->mPath->setText(path);
    }
}

void ApplicationDialog::ok()
{
    if (mUI->mName->text().isEmpty() || mUI->mPath->text().isEmpty()) {
        QMessageBox msg(QMessageBox::Warning,
                        "Cppcheck",
                        tr("You must specify a name, a path and optionally parameters for the application!"),
                        QMessageBox::Ok,
                        this);

        msg.exec();

        reject();
    } else {
        // Convert possible native (Windows) path to internal presentation format
        mApplication.setName(mUI->mName->text());
        mApplication.setPath(QDir::fromNativeSeparators(mUI->mPath->text()));
        mApplication.setParameters(mUI->mParameters->text());

        accept();
    }
}
