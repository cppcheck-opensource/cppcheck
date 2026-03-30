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

#include "helpdialog.h"

#include "common.h"
#include "utils.h"

#include "ui_helpdialog.h"

#include <QApplication>
#include <QFileInfo>
#include <QHelpEngine>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVBoxLayout>

class QWidget;

void HelpBrowser::setHelpEngine(QHelpEngine *helpEngine)
{
    mHelpEngine = helpEngine;
}

QVariant HelpBrowser::loadResource(int type, const QUrl &name)
{
    if (name.scheme() == "qthelp") {
        QString url(name.toString());
        while (url.indexOf("/./") > 0)
            url.remove(url.indexOf("/./"), 2);
        return QVariant(mHelpEngine->fileData(QUrl(url)));
    }
    return QTextBrowser::loadResource(type, name);
}

static QString getHelpFile()
{
    const QString datadir = getDataDir();

    QStringList paths;
    paths << (datadir + "/help")
          << datadir
          << (QApplication::applicationDirPath() + "/help")
          << QApplication::applicationDirPath();
#ifdef FILESDIR
    const QString filesdir = FILESDIR;
    paths << (filesdir + "/help")
          << filesdir;
#endif
    for (const QString &p: utils::as_const(paths)) {
        QString filename = p + "/online-help.qhc";
        if (QFileInfo::exists(filename))
            return filename;
    }
    return QString();
}

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::HelpDialog)
{
    mUi->setupUi(this);

    QString helpFile = getHelpFile();
    if (helpFile.isEmpty()) {
        const QString msg = tr("Helpfile '%1' was not found").arg("online-help.qhc");
        QMessageBox msgBox(QMessageBox::Warning,
                           tr("Cppcheck"),
                           msg,
                           QMessageBox::Ok,
                           this);
        msgBox.exec();
        mHelpEngine = nullptr;
        return;
    }

    mHelpEngine = new QHelpEngine(helpFile);
    // Disable the timestamp check of online-help.qhc by setting _q_readonly
    mHelpEngine->setProperty("_q_readonly", QVariant::fromValue<bool>(true));
    mHelpEngine->setupData();

    mUi->contents->addWidget(mHelpEngine->contentWidget());
    mUi->index->addWidget(mHelpEngine->indexWidget());

    mUi->textBrowser->setHelpEngine(mHelpEngine);

    mUi->textBrowser->setSource(QUrl("qthelp://cppcheck.sourceforge.io/doc/index.html"));
    connect(mHelpEngine->contentWidget(),
            SIGNAL(linkActivated(QUrl)),
            mUi->textBrowser,
            SLOT(setSource(QUrl)));

    connect(mHelpEngine->indexWidget(),
            SIGNAL(linkActivated(QUrl,QString)),
            mUi->textBrowser,
            SLOT(setSource(QUrl)));
}

HelpDialog::~HelpDialog()
{
    delete mUi;
    delete mHelpEngine;
}
