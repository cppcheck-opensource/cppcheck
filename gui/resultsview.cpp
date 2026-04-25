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

#include "resultsview.h"

#include "checkstatistics.h"
#include "checkersreport.h"
#include "codeeditor.h"
#include "codeeditorstyle.h"
#include "common.h"
#include "csvreport.h"
#include "erroritem.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "path.h"
#include "printablereport.h"
#include "resultstree.h"
#include "settings.h"
#include "txtreport.h"
#include "xmlreport.h"
#include "xmlreportv2.h"

#include "ui_resultsview.h"

#include <set>
#include <string>

#include <QAbstractItemModel>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProgressBar>
#include <QSettings>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextStream>
#include <Qt>

enum class ReportType : std::uint8_t;

ResultsView::ResultsView(QWidget * parent) :
    QWidget(parent),
    mUI(new Ui::ResultsView),
    mStatistics(new CheckStatistics(this))
{
    mUI->setupUi(this);

    connect(mUI->mTree, &ResultsTree::resultsHidden, this, &ResultsView::resultsHidden);
    connect(mUI->mTree, &ResultsTree::checkSelected, this, &ResultsView::checkSelected);
    connect(mUI->mTree, &ResultsTree::treeSelectionChanged, this, &ResultsView::updateDetails);
    connect(mUI->mTree, &ResultsTree::suppressIds, this, &ResultsView::suppressIds);
    connect(this, &ResultsView::showResults, mUI->mTree, &ResultsTree::showResults);
    connect(this, &ResultsView::showCppcheckResults, mUI->mTree, &ResultsTree::showCppcheckResults);
    connect(this, &ResultsView::showClangResults, mUI->mTree, &ResultsTree::showClangResults);
    connect(this, &ResultsView::collapseAllResults, mUI->mTree, &ResultsTree::collapseAll);
    connect(this, &ResultsView::expandAllResults, mUI->mTree, &ResultsTree::expandAll);
    connect(this, &ResultsView::showHiddenResults, mUI->mTree, &ResultsTree::showHiddenResults);

    mUI->mListLog->setContextMenuPolicy(Qt::CustomContextMenu);
}

void ResultsView::initialize(QSettings *settings, ApplicationList *list, ThreadHandler *checkThreadHandler)
{
    mUI->mProgress->setMinimum(0);
    mUI->mProgress->setVisible(false);
    mUI->mLabelCriticalErrors->setVisible(false);

    CodeEditorStyle theStyle(CodeEditorStyle::loadSettings(settings));
    mUI->mCode->setStyle(theStyle);

    QByteArray state = settings->value(SETTINGS_MAINWND_SPLITTER_STATE).toByteArray();
    mUI->mVerticalSplitter->restoreState(state);
    mShowNoErrorsMessage = settings->value(SETTINGS_SHOW_NO_ERRORS, true).toBool();

    mUI->mTree->initialize(settings, list, checkThreadHandler);
}

ResultsView::~ResultsView()
{
    delete mUI;
    delete mCheckSettings;
}

void ResultsView::clear(bool results)
{
    if (results) {
        mUI->mTree->clear();
    }

    mUI->mDetails->setText(QString());
    mUI->mCode->clear();

    mStatistics->clear();
    delete mCheckSettings;
    mCheckSettings = nullptr;

    //Clear the progressbar
    mUI->mProgress->setMaximum(PROGRESS_MAX);
    mUI->mProgress->setValue(0);
    mUI->mProgress->setFormat("%p%");

    mUI->mLabelCriticalErrors->setVisible(false);
}

void ResultsView::clear(const QString &filename)
{
    mUI->mTree->clear(filename);
}

void ResultsView::clearRecheckFile(const QString &filename)
{
    mUI->mTree->clearRecheckFile(filename);
}

const ShowTypes & ResultsView::getShowTypes() const
{
    return mUI->mTree->mShowSeverities;
}

void ResultsView::setReportType(ReportType reportType) {
    mUI->mTree->setReportType(reportType);
}

void ResultsView::setResultsSource(ResultsTree::ResultsSource source)
{
    mUI->mTree->setResultsSource(source);
}

void ResultsView::filesCheckedProgress(int value, const QString& description)
{
    mUI->mProgress->setValue(value);
    mUI->mProgress->setFormat(QString("%p% (%1)").arg(description));
}

void ResultsView::error(const ErrorItem &item)
{
    if (item.severity == Severity::internal && (item.errorId == "logChecker" || item.errorId.endsWith("-logChecker"))) {
        mStatistics->addChecker(item.message);
        return;
    }

    handleCriticalError(item);

    if (item.severity == Severity::internal)
        return;

    if (mUI->mTree->addErrorItem(item)) {
        emit gotResults();
        mStatistics->addItem(item.tool(), ShowTypes::SeverityToShowType(item.severity));
    }
}

void ResultsView::filterResults(const QString& filter)
{
    mUI->mTree->filterResults(filter);
}

void ResultsView::saveStatistics(const QString &filename) const
{
    QFile f(filename);
    if (!f.open(QIODevice::Text | QIODevice::Append))
        return;
    QTextStream ts(&f);
    ts <<  '[' << QDate::currentDate().toString("dd.MM.yyyy") << "]\n";
    ts << QDateTime::currentMSecsSinceEpoch() << '\n';
    for (const QString& tool : mStatistics->getTools()) {
        ts << tool << "-error:" << mStatistics->getCount(tool, ShowTypes::ShowErrors) << '\n';
        ts << tool << "-warning:" << mStatistics->getCount(tool, ShowTypes::ShowWarnings) << '\n';
        ts << tool << "-style:" << mStatistics->getCount(tool, ShowTypes::ShowStyle) << '\n';
        ts << tool << "-performance:" << mStatistics->getCount(tool, ShowTypes::ShowPerformance) << '\n';
        ts << tool << "-portability:" << mStatistics->getCount(tool, ShowTypes::ShowPortability) << '\n';
    }
}

void ResultsView::updateFromOldReport(const QString &filename) const
{
    mUI->mTree->updateFromOldReport(filename);
}

void ResultsView::save(const QString &filename, Report::Type type, const QString& productName) const
{
    Report *report = nullptr;

    switch (type) {
    case Report::CSV:
        report = new CsvReport(filename);
        break;
    case Report::TXT:
        report = new TxtReport(filename);
        break;
    case Report::XMLV2:
        report = new XmlReportV2(filename, productName);
        break;
    }

    if (report) {
        if (report->create())
            mUI->mTree->saveResults(report);
        else {
            QMessageBox msgBox;
            msgBox.setText(tr("Failed to save the report."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
        delete report;
        report = nullptr;
    } else {
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to save the report."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

void ResultsView::print()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Report"));
    if (dialog.exec() != QDialog::Accepted)
        return;

    print(&printer);
}

void ResultsView::printPreview()
{
    QPrinter printer;
    QPrintPreviewDialog dialog(&printer, this);
    connect(&dialog, SIGNAL(paintRequested(QPrinter*)), SLOT(print(QPrinter*)));
    dialog.exec();
}

void ResultsView::print(QPrinter* printer) const
{
    if (!hasResults()) {
        QMessageBox msgBox;
        msgBox.setText(tr("No errors found, nothing to print."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    PrintableReport report;
    mUI->mTree->saveResults(&report);
    QTextDocument doc(report.getFormattedReportText());
    doc.print(printer);
}

void ResultsView::updateSettings(bool showFullPath,
                                 bool saveFullPath,
                                 bool saveAllErrors,
                                 bool showNoErrorsMessage,
                                 bool showErrorId,
                                 bool showInconclusive)
{
    mUI->mTree->updateSettings(showFullPath, saveFullPath, saveAllErrors, showErrorId, showInconclusive);
    mShowNoErrorsMessage = showNoErrorsMessage;
}

void ResultsView::updateStyleSetting(QSettings *settings)
{
    CodeEditorStyle theStyle(CodeEditorStyle::loadSettings(settings));
    mUI->mCode->setStyle(theStyle);
}

void ResultsView::setCheckDirectory(const QString &dir)
{
    mUI->mTree->setCheckDirectory(dir);
}

QString ResultsView::getCheckDirectory() const
{
    return mUI->mTree->getCheckDirectory();
}

void ResultsView::setCheckSettings(const Settings &settings)
{
    delete mCheckSettings;
    mCheckSettings = new Settings;
    *mCheckSettings = settings;
}

void ResultsView::checkingStarted(int count)
{
    mUI->mProgress->setVisible(true);
    mUI->mProgress->setMaximum(PROGRESS_MAX);
    mUI->mProgress->setValue(0);
    mUI->mProgress->setFormat(tr("%p% (%1 of %2 files checked)").arg(0).arg(count));
}

void ResultsView::checkingFinished()
{
    mUI->mProgress->setVisible(false);
    mUI->mProgress->setFormat("%p%");

    {
        Settings checkSettings;
        const std::set<std::string> activeCheckers = mStatistics->getActiveCheckers();
        CheckersReport checkersReport(mCheckSettings ? *mCheckSettings : checkSettings, activeCheckers);
        mStatistics->setCheckersReport(QString::fromStdString(checkersReport.getReport(mCriticalErrors.toStdString())));
    }

    // TODO: Items can be mysteriously hidden when checking is finished, this function
    // call should be redundant but it "unhides" the wrongly hidden items.
    mUI->mTree->refreshTree();

    //Should we inform user of non visible/not found errors?
    if (mShowNoErrorsMessage) {
        //Tell user that we found no errors
        if (!hasResults()) {
            QMessageBox msg(QMessageBox::Information,
                            tr("Cppcheck"),
                            tr("No errors found."),
                            QMessageBox::Ok,
                            this);

            msg.exec();
        } //If we have errors but they aren't visible, tell user about it
        else if (!mUI->mTree->hasVisibleResults()) {
            QString text = tr("Errors were found, but they are configured to be hidden.\n" \
                              "To toggle what kind of errors are shown, open view menu.");
            QMessageBox msg(QMessageBox::Information,
                            tr("Cppcheck"),
                            text,
                            QMessageBox::Ok,
                            this);

            msg.exec();
        }
    }
}

bool ResultsView::hasVisibleResults() const
{
    return mUI->mTree->hasVisibleResults();
}

bool ResultsView::hasResults() const
{
    return mUI->mTree->hasResults();
}

void ResultsView::saveSettings(QSettings *settings)
{
    mUI->mTree->saveSettings();
    QByteArray state = mUI->mVerticalSplitter->saveState();
    settings->setValue(SETTINGS_MAINWND_SPLITTER_STATE, state);
    mUI->mVerticalSplitter->restoreState(state);
}

void ResultsView::translate()
{
    mUI->retranslateUi(this);
    mUI->mTree->translate();
}

void ResultsView::readErrorsXml(const QString &filename)
{
    const int version = XmlReport::determineVersion(filename);
    if (version == 0) {
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to read the report."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    if (version == 1) {
        QMessageBox msgBox;
        msgBox.setText(tr("XML format version 1 is no longer supported."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    XmlReportV2 report(filename, QString());
    QList<ErrorItem> errors;
    if (report.open()) {
        errors = report.read();
    } else {
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to read the report."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }

    for (const ErrorItem& item : utils::as_const(errors)) {
        handleCriticalError(item);
        mUI->mTree->addErrorItem(item);
    }

    QString dir;
    if (!errors.isEmpty() && !errors[0].errorPath.isEmpty()) {
        QString relativePath = QFileInfo(filename).canonicalPath();
        if (QFileInfo::exists(relativePath + '/' + errors[0].errorPath[0].file))
            dir = relativePath;
    }

    mUI->mTree->setCheckDirectory(dir);
}

void ResultsView::updateDetails(const ResultItem* item)
{
    if (!item || !item->errorItem) {
        mUI->mCode->clear();
        mUI->mDetails->setText(QString());
        return;
    }

    // File item => No details can be shown
    if (item->getType() == ResultItem::Type::file) {
        mUI->mCode->clear();
        mUI->mDetails->setText(QString());
        return;
    }

    QString formattedMsg = item->errorItem->message;

    const QString file0 = item->errorItem->file0;
    if (!file0.isEmpty() && Path::isHeader(item->getErrorPathItem().file.toStdString()))
        formattedMsg += QString("\n\n%1: %2").arg(tr("First included by")).arg(QDir::toNativeSeparators(file0));

    if (item->errorItem->cwe > 0)
        formattedMsg.prepend("CWE: " + QString::number(item->errorItem->cwe) + "\n");
    if (mUI->mTree->showIdColumn())
        formattedMsg.prepend(tr("Id") + ": " + item->errorItem->errorId + "\n");
    mUI->mDetails->setText(formattedMsg);

    const int lineNumber = item->getErrorPathItem().line;

    QString filepath = item->getErrorPathItem().file;
    if (!QFileInfo::exists(filepath) && QFileInfo::exists(mUI->mTree->getCheckDirectory() + '/' + filepath))
        filepath = mUI->mTree->getCheckDirectory() + '/' + filepath;

    QStringList symbols;
    if (!item->errorItem->symbolNames.isEmpty())
        symbols = item->errorItem->symbolNames.split("\n");

    if (filepath == mUI->mCode->getFileName()) {
        mUI->mCode->setError(lineNumber, symbols);
        return;
    }

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        mUI->mCode->clear();
        return;
    }

    QTextStream in(&file);
    mUI->mCode->setError(in.readAll(), lineNumber, symbols);
    mUI->mCode->setFileName(filepath);
}

void ResultsView::log(const QString &str)
{
    mUI->mListLog->addItem(str);
}

void ResultsView::debugError(const ErrorItem &item)
{
    mUI->mListLog->addItem(item.toString());
}

void ResultsView::logClear()
{
    mUI->mListLog->clear();
}

void ResultsView::logCopyEntry()
{
    const QListWidgetItem * item = mUI->mListLog->currentItem();
    if (nullptr != item) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(item->text());
    }
}

void ResultsView::logCopyComplete()
{
    QString logText;
    for (int i=0; i < mUI->mListLog->count(); ++i) {
        const QListWidgetItem * item = mUI->mListLog->item(i);
        if (nullptr != item) {
            logText += item->text();
        }
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(logText);
}

void ResultsView::on_mListLog_customContextMenuRequested(const QPoint &pos)
{
    if (mUI->mListLog->count() <= 0)
        return;

    const QPoint globalPos = mUI->mListLog->mapToGlobal(pos);

    QMenu contextMenu;
    contextMenu.addAction(tr("Clear Log"), this, SLOT(logClear()));
    contextMenu.addAction(tr("Copy this Log entry"), this, SLOT(logCopyEntry()));
    contextMenu.addAction(tr("Copy complete Log"), this, SLOT(logCopyComplete()));

    contextMenu.exec(globalPos);
}

void ResultsView::stopAnalysis()
{
    mUI->mLabelCriticalErrors->setText(tr("Analysis was stopped"));
    mUI->mLabelCriticalErrors->setVisible(true);
}

void ResultsView::handleCriticalError(const ErrorItem &item)
{
    if (ErrorLogger::isCriticalErrorId(item.errorId.toStdString())) {
        if (!mCriticalErrors.contains(item.errorId)) {
            if (!mCriticalErrors.isEmpty())
                mCriticalErrors += ",";
            mCriticalErrors += item.errorId;
            if (item.severity == Severity::internal)
                mCriticalErrors += " (suppressed)";
        }
        QString msg = tr("There was a critical error with id '%1'").arg(item.errorId);
        if (!item.file0.isEmpty())
            msg += ", " + tr("when checking %1").arg(item.file0);
        else
            msg += ", " + tr("when checking a file");
        msg += ". " + tr("Analysis was aborted.");
        mUI->mLabelCriticalErrors->setText(msg);
        mUI->mLabelCriticalErrors->setVisible(true);
    }
}
