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

#include "resultstree.h"

#include "application.h"
#include "applicationlist.h"
#include "checkers.h"
#include "common.h"
#include "erroritem.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "path.h"
#include "projectfile.h"
#include "report.h"
#include "resultitem.h"
#include "showtypes.h"
#include "suppressions.h"
#include "threadhandler.h"
#include "utils.h"
#include "xmlreportv2.h"

#include <algorithm>
#include <utility>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QObject>
#include <QProcess>
#include <QSet>
#include <QSettings>
#include <QSignalMapper>
#include <QStandardItemModel>
#include <QUrl>
#include <Qt>
#include <QHeaderView>

// These must match column headers given in ResultsTree::translate()
static constexpr int COLUMN_FILE                  = 0;
static constexpr int COLUMN_LINE                  = 1;
static constexpr int COLUMN_SEVERITY              = 2;
static constexpr int COLUMN_MISRA_CLASSIFICATION  = 3;
static constexpr int COLUMN_CERT_LEVEL            = 4;
static constexpr int COLUMN_INCONCLUSIVE          = 5;
static constexpr int COLUMN_SUMMARY               = 6;
static constexpr int COLUMN_ID                    = 7;
static constexpr int COLUMN_MISRA_GUIDELINE       = 8;
static constexpr int COLUMN_CERT_RULE             = 9;
static constexpr int COLUMN_SINCE_DATE            = 10;
static constexpr int COLUMN_TAGS                  = 11;
static constexpr int COLUMN_CWE                   = 12;

static QString getGuideline(ReportType reportType, const std::map<std::string, std::string> &guidelineMapping,
                            const QString& errorId, Severity severity) {
    return QString::fromStdString(getGuideline(errorId.toStdString(),
                                               reportType, guidelineMapping,
                                               severity));
}

static QString getClassification(ReportType reportType, const QString& guideline) {
    return QString::fromStdString(getClassification(guideline.toStdString(), reportType));
}

static Severity getSeverityFromClassification(const QString &c) {
    if (c == checkers::Man)
        return Severity::error;
    if (c == checkers::Req)
        return Severity::warning;
    if (c == checkers::Adv)
        return Severity::style;
    if (c == checkers::Doc)
        return Severity::information;
    if (c == "L1")
        return Severity::error;
    if (c == "L2")
        return Severity::warning;
    if (c == "L3")
        return Severity::style;
    return Severity::none;
}

static QStringList getLabels() {
    return QStringList{
        QObject::tr("File"),
        QObject::tr("Line"),
        QObject::tr("Severity"),
        QObject::tr("Classification"),
        QObject::tr("Level"),
        QObject::tr("Inconclusive"),
        QObject::tr("Summary"),
        QObject::tr("Id"),
        QObject::tr("Guideline"),
        QObject::tr("Rule"),
        QObject::tr("Since date"),
        QObject::tr("Tags"),
        QObject::tr("CWE")};
}

static Severity getSeverity(ReportType reportType, const ErrorItem& errorItem) {
    return reportType == ReportType::normal ? errorItem.severity : getSeverityFromClassification(errorItem.classification);
}

ResultsTree::ResultsTree(QWidget * parent) :
    QTreeView(parent),
    mModel(new QStandardItemModel)
{
    setModel(mModel);
    translate(); // Adds columns to grid
    clear();
    setExpandsOnDoubleClick(false);
    setSortingEnabled(true);

    connect(this, &ResultsTree::doubleClicked, this, &ResultsTree::quickStartApplication);
}

void ResultsTree::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        quickStartApplication(this->currentIndex());
    }
    QTreeView::keyPressEvent(event);
}

void ResultsTree::setReportType(ReportType reportType) {
    mReportType = reportType;

    mGuideline = createGuidelineMapping(reportType);

    for (int i = 0; i < mModel->rowCount(); ++i) {
        auto *fileItem = dynamic_cast<ResultItem*>(mModel->item(i, COLUMN_FILE));
        if (!fileItem)
            continue;
        for (int j = 0; j < fileItem->rowCount(); ++j) {
            QSharedPointer<ErrorItem>& errorItem = dynamic_cast<ResultItem*>(fileItem->child(j,0))->errorItem;
            errorItem->guideline = getGuideline(mReportType, mGuideline, errorItem->errorId, errorItem->severity);
            errorItem->classification = getClassification(mReportType, errorItem->guideline);
            dynamic_cast<ResultItem*>(fileItem->child(j, COLUMN_FILE))->setIconFileName(severityToIcon(getSeverity(reportType, *errorItem)));
            fileItem->child(j, COLUMN_CERT_LEVEL)->setText(errorItem->classification);
            fileItem->child(j, COLUMN_CERT_RULE)->setText(errorItem->guideline);
            fileItem->child(j, COLUMN_MISRA_CLASSIFICATION)->setText(errorItem->classification);
            fileItem->child(j, COLUMN_MISRA_GUIDELINE)->setText(errorItem->guideline);
        }
    }

    if (isAutosarMisraReport()) {
        showColumn(COLUMN_MISRA_CLASSIFICATION);
        showColumn(COLUMN_MISRA_GUIDELINE);
    } else {
        hideColumn(COLUMN_MISRA_CLASSIFICATION);
        hideColumn(COLUMN_MISRA_GUIDELINE);
    }

    if (isCertReport()) {
        showColumn(COLUMN_CERT_LEVEL);
        showColumn(COLUMN_CERT_RULE);
    } else {
        hideColumn(COLUMN_CERT_LEVEL);
        hideColumn(COLUMN_CERT_RULE);
    }

    if (mReportType == ReportType::normal) {
        showColumn(COLUMN_SEVERITY);
    } else {
        hideColumn(COLUMN_SEVERITY);
    }

    refreshTree();
}

void ResultsTree::initialize(QSettings *settings, ApplicationList *list, ThreadHandler *checkThreadHandler)
{
    mSettings = settings;
    mApplications = list;
    mThread = checkThreadHandler;
    loadSettings();
}

ResultItem *ResultsTree::createNormalItem(const QString &text, QSharedPointer<ErrorItem> errorItem, ResultItem::Type type, int errorPathIndex)
{
    auto *item = new ResultItem(std::move(errorItem), type, errorPathIndex);
    item->setText(text);
    item->setEditable(false);
    return item;
}

ResultItem *ResultsTree::createFilenameItem(const QSharedPointer<ErrorItem>& errorItem, ResultItem::Type type, int errorPathIndex)
{
    auto *item = new ResultItem(errorItem, type, errorPathIndex);
    item->setText(QDir::toNativeSeparators(stripPath(errorItem->errorPath[errorPathIndex].file, false)));
    item->setEditable(false);
    return item;
}

ResultItem *ResultsTree::createCheckboxItem(bool checked, QSharedPointer<ErrorItem> errorItem, ResultItem::Type type, int errorPathIndex)
{
    auto *item = new ResultItem(std::move(errorItem), type, errorPathIndex);
    item->setCheckable(true);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    item->setEnabled(false);
    return item;
}

ResultItem *ResultsTree::createLineNumberItem(int linenumber, QSharedPointer<ErrorItem> errorItem, ResultItem::Type type, int errorPathIndex)
{
    auto *item = new ResultItem(std::move(errorItem), type, errorPathIndex);
    item->setText(QString::number(linenumber));
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    item->setEditable(false);
    return item;
}

bool ResultsTree::addErrorItem(const ErrorItem& errorItem)
{
    if (errorItem.errorPath.isEmpty())
        return false;

    const QString s = errorItem.toString();
    if (mErrorList.contains(s))
        return false;
    mErrorList.append(s);

    QSharedPointer<ErrorItem> errorItemPtr{new ErrorItem(errorItem)};

    if (mReportType != ReportType::normal) {
        errorItemPtr->guideline = getGuideline(mReportType, mGuideline, errorItemPtr->errorId, errorItemPtr->severity);
        errorItemPtr->classification = getClassification(mReportType, errorItemPtr->guideline);
    }

    const bool showItem = !isErrorItemHidden(errorItemPtr);

    // if there is at least one error that is not hidden, we have a visible error
    mVisibleErrors |= showItem;

    if (const ProjectFile *activeProject = ProjectFile::getActiveProject())
        errorItemPtr->tags = activeProject->getWarningTags(errorItemPtr->hash);
    //Create the base item for the error and ensure it has a proper
    //file item as a parent
    ResultItem* fileItem = ensureFileItem(errorItemPtr, !showItem);
    ResultItem* stditem = addBacktraceFiles(fileItem,
                                            errorItemPtr,
                                            !showItem,
                                            severityToIcon(getSeverity(mReportType, *errorItemPtr)),
                                            ResultItem::Type::message,
                                            errorItemPtr->getMainLocIndex());

    if (!stditem)
        return false;

    //Add backtrace files as children
    if (errorItemPtr->errorPath.size() > 1) {
        for (int i = 0; i < errorItemPtr->errorPath.size(); i++) {
            addBacktraceFiles(stditem,
                              errorItemPtr,
                              false,
                              ":images/go-down.png",
                              ResultItem::Type::note,
                              i);
        }
    }

    return true;
}

ResultItem *ResultsTree::addBacktraceFiles(ResultItem *parent,
                                           const QSharedPointer<ErrorItem>& errorItem,
                                           const bool hide,
                                           const QString &icon,
                                           ResultItem::Type type,
                                           int errorPathIndex)
{
    if (!parent)
        return nullptr;

    //TODO message has parameter names so we'll need changes to the core
    //cppcheck so we can get proper translations

    const bool childOfMessage = (type == ResultItem::Type::note);
    const QString itemSeverity = childOfMessage ? tr("note") : severityToTranslatedString(errorItem->severity);

    const auto& loc = errorItem->errorPath[errorPathIndex];

    // Check for duplicate rows and don't add them if found
    for (int i = 0; i < errorPathIndex; i++) {
        // The first column is the file name and is always the same
        const auto& e = errorItem->errorPath[i];
        if (loc.line == e.line && loc.info == e.info)
            return nullptr;
    }

    const QString text = childOfMessage ? loc.info : errorItem->summary;

    const int numberOfColumns = getLabels().size();
    QList<ResultItem*> columns(numberOfColumns);
    columns[COLUMN_FILE] = createFilenameItem(errorItem, type, errorPathIndex);
    columns[COLUMN_FILE]->setIconFileName(icon);
    columns[COLUMN_LINE] = createLineNumberItem(loc.line, errorItem, type, errorPathIndex);
    columns[COLUMN_SEVERITY] = createNormalItem(itemSeverity, errorItem, type, errorPathIndex);
    columns[COLUMN_SUMMARY] = createNormalItem(text, errorItem, type, errorPathIndex);
    if (type == ResultItem::Type::message) {
        columns[COLUMN_CERT_LEVEL] = createNormalItem(errorItem->classification, errorItem, type, errorPathIndex);
        columns[COLUMN_CERT_RULE] = createNormalItem(errorItem->guideline, errorItem, type, errorPathIndex);
        columns[COLUMN_CWE] = createNormalItem(errorItem->cwe > 0 ? QString::number(errorItem->cwe) : QString(), errorItem, type, errorPathIndex);
        columns[COLUMN_ID] = createNormalItem(errorItem->errorId, errorItem, type, errorPathIndex);
        columns[COLUMN_INCONCLUSIVE] = createCheckboxItem(errorItem->inconclusive, errorItem, type, errorPathIndex);
        columns[COLUMN_MISRA_CLASSIFICATION] = createNormalItem(errorItem->classification, errorItem, type, errorPathIndex);
        columns[COLUMN_MISRA_GUIDELINE] = createNormalItem(errorItem->guideline, errorItem, type, errorPathIndex);
        columns[COLUMN_SINCE_DATE] = createNormalItem(errorItem->sinceDate, errorItem, type, errorPathIndex);
        columns[COLUMN_TAGS] = createNormalItem(errorItem->tags, errorItem, type, errorPathIndex);
    }

    QList<QStandardItem*> list;
    for (int i = 0; i < numberOfColumns; ++i)
        list << (columns[i] ? columns[i] : createNormalItem(QString(), errorItem, type, errorPathIndex));

    parent->appendRow(list);

    setRowHidden(parent->rowCount() - 1, parent->index(), hide);

    return columns[COLUMN_FILE];
}

QString ResultsTree::severityToTranslatedString(Severity severity)
{
    switch (severity) {
    case Severity::style:
        return tr("style");

    case Severity::error:
        return tr("error");

    case Severity::warning:
        return tr("warning");

    case Severity::performance:
        return tr("performance");

    case Severity::portability:
        return tr("portability");

    case Severity::information:
        return tr("information");

    case Severity::debug:
        return tr("debug");

    case Severity::internal:
        return tr("internal");

    case Severity::none:
        return QString();
    }

    cppcheck::unreachable();
}

ResultItem *ResultsTree::findFileItem(const QString &name) const
{
    // The first column contains the file name. In Windows we can get filenames
    // "header.h" and "Header.h" and must compare them as identical.

    for (int i = 0; i < mModel->rowCount(); i++) {
#ifdef _WIN32
        if (QString::compare(mModel->item(i, COLUMN_FILE)->text(), name, Qt::CaseInsensitive) == 0)
#else
        if (mModel->item(i, COLUMN_FILE)->text() == name)
#endif
            return dynamic_cast<ResultItem*>(mModel->item(i, COLUMN_FILE));
    }
    return nullptr;
}

void ResultsTree::clear()
{
    mErrorList.clear();

    mModel->removeRows(0, mModel->rowCount());

    if (const ProjectFile *activeProject = ProjectFile::getActiveProject()) {
        hideColumn(COLUMN_SINCE_DATE);
        if (activeProject->getTags().isEmpty())
            hideColumn(COLUMN_TAGS);
        else
            showColumn(COLUMN_TAGS);
    } else {
        hideColumn(COLUMN_SINCE_DATE);
        hideColumn(COLUMN_TAGS);
    }
}

void ResultsTree::clear(const QString &filename)
{
    const QString stripped = QDir::toNativeSeparators(stripPath(filename, false));

    for (int i = 0; i < mModel->rowCount(); ++i) {
        const auto *fileItem = dynamic_cast<ResultItem*>(mModel->item(i, COLUMN_FILE));
        if (!fileItem)
            continue;

        if (stripped == fileItem->text() ||
            filename == fileItem->errorItem->file0) {
            mModel->removeRow(i);
            mErrorList.removeAll(fileItem->errorItem->toString());
            break;
        }
    }
}

void ResultsTree::clearRecheckFile(const QString &filename)
{
    for (int i = 0; i < mModel->rowCount(); ++i) {
        const auto *fileItem = dynamic_cast<ResultItem*>(mModel->item(i, COLUMN_FILE));
        if (!fileItem)
            continue;

        QString actualfile((!mCheckPath.isEmpty() && filename.startsWith(mCheckPath)) ? filename.mid(mCheckPath.length() + 1) : filename);
        QString storedfile = fileItem->getErrorPathItem().file;
        storedfile = ((!mCheckPath.isEmpty() && storedfile.startsWith(mCheckPath)) ? storedfile.mid(mCheckPath.length() + 1) : storedfile);
        if (actualfile == storedfile) {
            mModel->removeRow(i);
            mErrorList.removeAll(fileItem->errorItem->toString());
            break;
        }
    }
}


void ResultsTree::loadSettings()
{
    for (int i = 0; i < mModel->columnCount(); i++) {
        QString temp = QString(SETTINGS_RESULT_COLUMN_WIDTH).arg(i);
        setColumnWidth(i, qMax(20, mSettings->value(temp, 800 / mModel->columnCount()).toInt()));
    }

    mSaveFullPath = mSettings->value(SETTINGS_SAVE_FULL_PATH, false).toBool();
    mSaveAllErrors = mSettings->value(SETTINGS_SAVE_ALL_ERRORS, false).toBool();
    mShowFullPath = mSettings->value(SETTINGS_SHOW_FULL_PATH, false).toBool();

    showIdColumn(mSettings->value(SETTINGS_SHOW_ERROR_ID, true).toBool());
    showInconclusiveColumn(mSettings->value(SETTINGS_INCONCLUSIVE_ERRORS, false).toBool());
}

void ResultsTree::saveSettings() const
{
    for (int i = 0; i < mModel->columnCount(); i++) {
        QString temp = QString(SETTINGS_RESULT_COLUMN_WIDTH).arg(i);
        mSettings->setValue(temp, columnWidth(i));
    }
}

void ResultsTree::showResults(ShowTypes::ShowType type, bool show)
{
    if (type != ShowTypes::ShowNone && mShowSeverities.isShown(type) != show) {
        mShowSeverities.show(type, show);
        refreshTree();
    }
}

void ResultsTree::showCppcheckResults(bool show)
{
    mShowCppcheck = show;
    refreshTree();
}

void ResultsTree::showClangResults(bool show)
{
    mShowClang = show;
    refreshTree();
}

void ResultsTree::filterResults(const QString& filter)
{
    mFilter = filter;
    refreshTree();
}

void ResultsTree::showHiddenResults()
{
    //Clear the "hide" flag for each item
    mHiddenMessageId.clear();
    refreshTree();
    emit resultsHidden(false);
}


void ResultsTree::refreshTree()
{
    mVisibleErrors = false;
    //Get the amount of files in the tree
    const int filecount = mModel->rowCount();

    for (int i = 0; i < filecount; i++) {
        //Get file i
        auto *fileItem = dynamic_cast<ResultItem*>(mModel->item(i, 0));
        if (!fileItem) {
            continue;
        }

        //Get the amount of errors this file contains
        const int errorcount = fileItem->rowCount();

        //By default it shouldn't be visible
        bool showFile = false;

        for (int j = 0; j < errorcount; j++) {
            //Get the error itself
            const auto *child = dynamic_cast<ResultItem*>(fileItem->child(j, 0));
            if (!child) {
                continue;
            }

            //Check if this error should be hidden
            const bool hide = child->hidden || isErrorItemHidden(child->errorItem);

            if (!hide) {
                showFile = true;
                mVisibleErrors = true;
            }

            //Hide/show accordingly
            setRowHidden(j, fileItem->index(), hide);
        }

        // Show the file if any of it's errors are visible
        setRowHidden(i, QModelIndex(), !showFile);
    }
    sortByColumn(header()->sortIndicatorSection(), header()->sortIndicatorOrder());
}

bool ResultsTree::isErrorItemHidden(const QSharedPointer<ErrorItem>& errorItem) const {
    //Check if this error should be hidden
    if (mHiddenMessageId.contains(errorItem->errorId))
        return true;

    bool hide;
    if (mReportType == ReportType::normal)
        hide = !mShowSeverities.isShown(errorItem->severity);
    else
        hide = errorItem->classification.isEmpty() || !mShowSeverities.isShown(getSeverityFromClassification(errorItem->classification));

    // If specified, filter on summary, message, filename, and id
    if (!hide && !mFilter.isEmpty())
        hide = !errorItem->filterMatch(mFilter);

    // Tool filter
    if (!hide) {
        if (errorItem->isClangResult())
            hide = !mShowClang;
        else
            hide = !mShowCppcheck;
    }

    return hide;
}

ResultItem *ResultsTree::ensureFileItem(const QSharedPointer<ErrorItem>& errorItem, bool hide)
{
    QString name = QDir::toNativeSeparators(stripPath(errorItem->getFile(), false));
    // Since item has path with native separators we must use path with
    // native separators to find it.
    ResultItem *fileItem = findFileItem(name);

    if (fileItem) {
        if (!hide)
            setRowHidden(fileItem->row(), QModelIndex(), hide);
        return fileItem;
    }

    // Ensure shown path is with native separators
    fileItem = createFilenameItem(errorItem, ResultItem::Type::file, errorItem->getMainLocIndex());
    fileItem->setIcon(QIcon(":images/text-x-generic.png"));

    mModel->appendRow(fileItem);

    setRowHidden(fileItem->row(), QModelIndex(), hide);

    return fileItem;
}

void ResultsTree::contextMenuEvent(QContextMenuEvent * e)
{
    QModelIndex index = indexAt(e->pos());
    if (index.isValid()) {
        bool multipleSelection = false;

        mSelectionModel = selectionModel();
        if (mSelectionModel->selectedRows().count() > 1)
            multipleSelection = true;

        mContextItem = dynamic_cast<ResultItem*>(mModel->itemFromIndex(index));

        //Create a new context menu
        QMenu menu(this);

        //Create a signal mapper so we don't have to store data to class
        //member variables
        QSignalMapper signalMapper;

        if (mContextItem && mApplications->getApplicationCount() > 0 && mContextItem->parent()) {
            //Create an action for the application
            int defaultApplicationIndex = mApplications->getDefaultApplication();
            defaultApplicationIndex = std::max(defaultApplicationIndex, 0);
            const Application& app = mApplications->getApplication(defaultApplicationIndex);
            auto *start = new QAction(app.getName(), &menu);
            if (multipleSelection)
                start->setDisabled(true);

            //Add it to context menu
            menu.addAction(start);

            //Connect the signal to signal mapper
            connect(start, &QAction::triggered, &signalMapper, QOverload<>::of(&QSignalMapper::map));

            //Add a new mapping
            signalMapper.setMapping(start, defaultApplicationIndex);

            connect(&signalMapper, SIGNAL(mappedInt(int)),
                    this, SLOT(context(int)));
        }

        // Add popup menuitems
        if (mContextItem) {
            if (mApplications->getApplicationCount() > 0) {
                menu.addSeparator();
            }

            int selectedFiles = 0;
            int selectedResults = 0;

            for (auto row : mSelectionModel->selectedRows()) {
                auto *item = mModel->itemFromIndex(row);
                if (!item->parent())
                    selectedFiles++;
                else if (!item->parent()->parent())
                    selectedResults++;
            }

            //Create an action for the application
            auto *recheckAction          = new QAction(tr("Recheck %1 file(s)").arg(selectedFiles), &menu);
            auto *copyAction             = new QAction(tr("Copy"), &menu);
            auto *hide                   = new QAction(tr("Hide %1 result(s)").arg(selectedResults), &menu);
            auto *hideallid              = new QAction(tr("Hide all with id"), &menu);
            auto *opencontainingfolder   = new QAction(tr("Open containing folder"), &menu);

            if (selectedFiles == 0 || mThread->isChecking() || mResultsSource == ResultsSource::Log)
                recheckAction->setDisabled(true);

            if (selectedResults == 0)
                hide->setDisabled(true);

            if (selectedResults == 0 || multipleSelection)
                hideallid->setDisabled(true);

            if (multipleSelection || mResultsSource == ResultsSource::Log)
                opencontainingfolder->setDisabled(true);

            menu.addAction(recheckAction);
            menu.addSeparator();
            menu.addAction(copyAction);
            menu.addSeparator();
            menu.addAction(hide);
            menu.addAction(hideallid);

            auto *suppress = new QAction(tr("Suppress selected id(s)"), &menu);
            {
                if (selectedResults == 0 || ErrorLogger::isCriticalErrorId(mContextItem->errorItem->errorId.toStdString()))
                    suppress->setDisabled(true);
            }
            menu.addAction(suppress);
            connect(suppress, &QAction::triggered, this, &ResultsTree::suppressSelectedIds);

            menu.addSeparator();
            menu.addAction(opencontainingfolder);

            connect(recheckAction, &QAction::triggered, this, &ResultsTree::recheckSelectedFiles);
            connect(copyAction, &QAction::triggered, this, &ResultsTree::copy);
            connect(hide, &QAction::triggered, this, &ResultsTree::hideResult);
            connect(hideallid, &QAction::triggered, this, &ResultsTree::hideAllIdResult);
            connect(opencontainingfolder, &QAction::triggered, this, &ResultsTree::openContainingFolder);

            const ProjectFile *currentProject = ProjectFile::getActiveProject();
            if (currentProject && !currentProject->getTags().isEmpty()) {
                menu.addSeparator();
                QMenu *tagMenu = menu.addMenu(tr("Tag"));
                {
                    auto *action = new QAction(tr("No tag"), tagMenu);
                    tagMenu->addAction(action);
                    connect(action, &QAction::triggered, [this]() {
                        tagSelectedItems(QString());
                    });
                }

                for (const QString& tagstr : currentProject->getTags()) {
                    auto *action = new QAction(tagstr, tagMenu);
                    tagMenu->addAction(action);
                    connect(action, &QAction::triggered, [tagstr, this]() {
                        tagSelectedItems(tagstr);
                    });
                }
            }
        }

        //Start the menu
        menu.exec(e->globalPos());
        index = indexAt(e->pos());
        if (index.isValid())
            mContextItem = dynamic_cast<ResultItem*>(mModel->itemFromIndex(index));
    }
}

void ResultsTree::startApplication(const ResultItem *target, int application)
{
    //If there are no applications specified, tell the user about it
    if (mApplications->getApplicationCount() == 0) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("No editor application configured.\n\n"
                           "Configure the editor application for Cppcheck in preferences/Applications."),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }

    if (application == -1)
        application = mApplications->getDefaultApplication();

    if (application == -1) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("No default editor application selected.\n\n"
                           "Please select the default editor application in preferences/Applications."),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }

    if (target && application >= 0 && application < mApplications->getApplicationCount() && target->parent()) {
        const auto& errorPathItem = target->getErrorPathItem();

        //Replace (file) with filename
        QString file = QDir::toNativeSeparators(errorPathItem.file);
        qDebug() << "Opening file: " << file;

        const QFileInfo info(file);
        if (!info.exists()) {
            if (info.isAbsolute()) {
                QMessageBox msgbox(this);
                msgbox.setWindowTitle("Cppcheck");
                msgbox.setText(tr("Could not find the file!"));
                msgbox.setIcon(QMessageBox::Critical);
                msgbox.exec();
            } else {
                QDir checkdir(mCheckPath);
                if (checkdir.isAbsolute() && checkdir.exists()) {
                    file = mCheckPath + "/" + file;
                } else {
                    QString dir = askFileDir(file);
                    dir += '/';
                    file = dir + file;
                }
            }
        }

        if (file.indexOf(" ") > -1) {
            file.insert(0, "\"");
            file.append("\"");
        }

        const Application& app = mApplications->getApplication(application);
        QString params = app.getParameters();
        params.replace("(file)", file, Qt::CaseInsensitive);

        params.replace("(line)", QString::number(errorPathItem.line), Qt::CaseInsensitive);

        params.replace("(message)", target->errorItem->message, Qt::CaseInsensitive);
        params.replace("(severity)", severityToTranslatedString(target->errorItem->severity), Qt::CaseInsensitive);

        QString program = app.getPath();

        // In Windows we must surround paths including spaces with quotation marks.
#ifdef Q_OS_WIN
        if (program.indexOf(" ") > -1) {
            if (!program.startsWith('"') && !program.endsWith('"')) {
                program.insert(0, "\"");
                program.append("\"");
            }
        }
#endif // Q_OS_WIN

        const bool success = QProcess::startDetached(program, QProcess::splitCommand(params));
        if (!success) {
            QString text = tr("Could not start %1\n\nPlease check the application path and parameters are correct.").arg(program);

            QMessageBox msgbox(this);
            msgbox.setWindowTitle("Cppcheck");
            msgbox.setText(text);
            msgbox.setIcon(QMessageBox::Critical);

            msgbox.exec();
        }
    }
}

QString ResultsTree::askFileDir(const QString &file)
{
    QString text = tr("Could not find file:") + '\n' + file + '\n';
    QString title;
    if (file.indexOf('/')) {
        QString folderName = file.mid(0, file.indexOf('/'));
        text += tr("Please select the folder '%1'").arg(folderName);
        title = tr("Select Directory '%1'").arg(folderName);
    } else {
        text += tr("Please select the directory where file is located.");
        title = tr("Select Directory");
    }

    QMessageBox msgbox(this);
    msgbox.setWindowTitle("Cppcheck");
    msgbox.setText(text);
    msgbox.setIcon(QMessageBox::Warning);
    msgbox.exec();

    QString dir = QFileDialog::getExistingDirectory(this, title,
                                                    getPath(SETTINGS_LAST_SOURCE_PATH),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return QString();

    // User selected root path
    if (QFileInfo::exists(dir + '/' + file))
        mCheckPath = dir;

    // user selected checked folder
    else if (file.indexOf('/') > 0) {
        dir += '/';
        QString folderName = file.mid(0, file.indexOf('/'));
        if (dir.indexOf('/' + folderName + '/'))
            dir = dir.mid(0, dir.lastIndexOf('/' + folderName + '/'));
        if (QFileInfo::exists(dir + '/' + file))
            mCheckPath = dir;
    }

    // Otherwise; return
    else
        return QString();

    setPath(SETTINGS_LAST_SOURCE_PATH, mCheckPath);
    return mCheckPath;
}

void ResultsTree::copy()
{
    if (!mSelectionModel)
        return;

    QString text;
    for (const QModelIndex& index : mSelectionModel->selectedRows()) {
        const auto *item = dynamic_cast<ResultItem*>(mModel->itemFromIndex(index));
        if (!item)
            continue;
        if (item->getType() == ResultItem::Type::file)
            text += item->text() + '\n';
        else if (item->getType() == ResultItem::Type::message)
            text += item->errorItem->toString() + '\n';
        else if (item->getType() == ResultItem::Type::note) {
            const auto e = item->getErrorPathItem();
            text += e.file + ":" + QString::number(e.line) + ":" + QString::number(e.column) + ":note: " + e.info + '\n';
        }
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void ResultsTree::hideResult()
{
    if (!mSelectionModel)
        return;
    bool hide = false;
    for (const QModelIndex& index : mSelectionModel->selectedRows()) {
        auto *item = dynamic_cast<ResultItem*>(mModel->itemFromIndex(index));
        if (item && item->getType() == ResultItem::Type::message)
            hide = item->hidden = true;
    }
    if (hide) {
        refreshTree();
        emit resultsHidden(true);
    }
}

void ResultsTree::recheckSelectedFiles()
{
    if (!mSelectionModel)
        return;

    QStringList selectedItems;
    for (const QModelIndex& index : mSelectionModel->selectedRows()) {
        const auto *item = dynamic_cast<ResultItem*>(mModel->itemFromIndex(index));
        while (item->parent())
            item = dynamic_cast<const ResultItem*>(item->parent());
        const auto e = item->getErrorPathItem();
        const QString currentFile = e.file;
        if (!currentFile.isEmpty()) {
            QString fileNameWithCheckPath;
            const QFileInfo curfileInfo(currentFile);
            if (!curfileInfo.exists() && !mCheckPath.isEmpty() && currentFile.indexOf(mCheckPath) != 0)
                fileNameWithCheckPath = mCheckPath + "/" + currentFile;
            else
                fileNameWithCheckPath = currentFile;
            const QFileInfo fileInfo(fileNameWithCheckPath);
            if (!fileInfo.exists()) {
                askFileDir(currentFile);
                return;
            }
            if (Path::isHeader(currentFile.toStdString())) {
                if (!item->errorItem->file0.isEmpty() && !selectedItems.contains(item->errorItem->file0)) {
                    selectedItems << ((!mCheckPath.isEmpty() && (item->errorItem->file0.indexOf(mCheckPath) != 0)) ? (mCheckPath + "/" + item->errorItem->file0) : item->errorItem->file0);
                    if (!selectedItems.contains(fileNameWithCheckPath))
                        selectedItems << fileNameWithCheckPath;
                }
            } else if (!selectedItems.contains(fileNameWithCheckPath))
                selectedItems << fileNameWithCheckPath;
        }
    }
    emit checkSelected(std::move(selectedItems));
}

void ResultsTree::hideAllIdResult()
{
    if (!mContextItem || mContextItem->getType() == ResultItem::Type::file)
        return;

    mHiddenMessageId.append(mContextItem->errorItem->errorId);

    refreshTree();
    emit resultsHidden(true);
}

void ResultsTree::suppressSelectedIds()
{
    if (!mSelectionModel)
        return;

    QSet<QString> selectedIds;
    for (const QModelIndex& index : mSelectionModel->selectedRows()) {
        const auto *item = dynamic_cast<ResultItem*>(mModel->itemFromIndex(index));
        if (!item || item->getType() == ResultItem::Type::file || !item->errorItem)
            continue;
        selectedIds << item->errorItem->errorId;
    }

    // delete all errors with selected message Ids
    for (int i = 0; i < mModel->rowCount(); i++) {
        QStandardItem * const file = mModel->item(i, 0);
        for (int j = 0; j < file->rowCount();) {
            const auto *errorItem = dynamic_cast<ResultItem*>(file->child(j, 0));
            if (errorItem && errorItem->errorItem && selectedIds.contains(errorItem->errorItem->errorId)) {
                file->removeRow(j);
            } else {
                j++;
            }
        }
        if (file->rowCount() == 0)
            mModel->removeRow(file->row());
    }

    if (!selectedIds.isEmpty()) {
        refreshTree(); // If all visible warnings was suppressed then the file item should be hidden
        emit suppressIds(selectedIds.values());
    }
}

void ResultsTree::suppressHash()
{
    if (!mSelectionModel)
        return;

    bool changed = false;
    ProjectFile *projectFile = ProjectFile::getActiveProject();

    for (QModelIndex index : mSelectionModel->selectedRows()) {
        auto *item = dynamic_cast<ResultItem *>(mModel->itemFromIndex(index));
        if (!item || item->getType() == ResultItem::Type::file)
            continue;
        if (item->getType() == ResultItem::Type::note)
            item = dynamic_cast<ResultItem *>(item->parent());

        // Suppress
        if (projectFile && item->errorItem->hash > 0) {
            SuppressionList::Suppression suppression;
            suppression.hash = item->errorItem->hash;
            projectFile->addSuppression(suppression);
            changed = true;
        }

        // Remove item
        QStandardItem *fileItem = item->parent();
        fileItem->removeRow(item->row());
        if (fileItem->rowCount() == 0)
            mModel->removeRow(fileItem->row());
    }

    if (changed)
        projectFile->write();
}

void ResultsTree::openContainingFolder()
{
    if (!mContextItem)
        return;
    QString filePath = mContextItem->getErrorPathItem().file;
    if (!filePath.isEmpty()) {
        filePath = QFileInfo(filePath).absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
}

void ResultsTree::tagSelectedItems(const QString &tag)
{
    if (!mSelectionModel)
        return;
    bool isTagged = false;
    ProjectFile *currentProject = ProjectFile::getActiveProject();
    for (QModelIndex index : mSelectionModel->selectedRows()) {
        auto *item = dynamic_cast<ResultItem*>(mModel->itemFromIndex(index));
        if (item && item->getType() != ResultItem::Type::file) {
            if (item->getType() == ResultItem::Type::note)
                item = dynamic_cast<ResultItem*>(item->parent());
            item->errorItem->tags = tag;
            item->parent()->child(index.row(), COLUMN_TAGS)->setText(tag);
            if (currentProject && item->errorItem->hash > 0) {
                isTagged = true;
                currentProject->setWarningTags(item->errorItem->hash, tag);
            }
        }
    }
    if (isTagged)
        currentProject->write();
}

void ResultsTree::context(int application)
{
    startApplication(mContextItem, application);
}

void ResultsTree::quickStartApplication(const QModelIndex &index)
{
    startApplication(dynamic_cast<ResultItem*>(mModel->itemFromIndex(index)));
}

QString ResultsTree::severityToIcon(Severity severity)
{
    switch (severity) {
    case Severity::error:
        return ":images/dialog-error.png";
    case Severity::style:
        return ":images/applications-development.png";
    case Severity::warning:
        return ":images/dialog-warning.png";
    case Severity::portability:
        return ":images/applications-system.png";
    case Severity::performance:
        return ":images/utilities-system-monitor.png";
    case Severity::information:
        return ":images/dialog-information.png";
    default:
        return QString();
    }
}

void ResultsTree::saveResults(Report *report) const
{
    report->writeHeader();

    for (int i = 0; i < mModel->rowCount(); i++) {
        if (mSaveAllErrors || !isRowHidden(i, QModelIndex()))
            saveErrors(report, dynamic_cast<ResultItem*>(mModel->item(i, 0)));
    }

    report->writeFooter();
}

void ResultsTree::saveErrors(Report *report, const ResultItem *fileItem) const
{
    if (!fileItem) {
        return;
    }

    for (int i = 0; i < fileItem->rowCount(); i++) {
        const auto *error = dynamic_cast<ResultItem*>(fileItem->child(i, 0));

        if (!error) {
            continue;
        }

        if (isRowHidden(i, fileItem->index()) && !mSaveAllErrors) {
            continue;
        }

        report->writeError(*error->errorItem);
    }
}

void ResultsTree::updateFromOldReport(const QString &filename)
{
    showColumn(COLUMN_SINCE_DATE);

    QList<ErrorItem> oldErrors;
    XmlReportV2 oldReport(filename, QString());
    if (oldReport.open()) {
        oldErrors = oldReport.read();
        oldReport.close();
    }

    // Read current results..
    for (int i = 0; i < mModel->rowCount(); i++) {
        auto *fileItem = dynamic_cast<ResultItem*>(mModel->item(i,COLUMN_FILE));
        for (int j = 0; j < fileItem->rowCount(); j++) {
            auto *error = dynamic_cast<ResultItem*>(fileItem->child(j,COLUMN_FILE));
            if (!error)
                // FIXME..
                continue;
            const auto it = std::find_if(oldErrors.cbegin(),
                                         oldErrors.cend(),
                                         [error](const ErrorItem& err) {
                return ErrorItem::same(err, *error->errorItem);
            });
            const ErrorItem* oldError = (it == oldErrors.cend()) ? nullptr : &*it;

            // New error .. set the "sinceDate" property
            if (oldError && !oldError->sinceDate.isEmpty()) {
                error->errorItem->sinceDate = oldError->sinceDate;
                fileItem->child(j, COLUMN_SINCE_DATE)->setText(error->errorItem->sinceDate);
            } else if (oldError == nullptr || error->errorItem->sinceDate.isEmpty()) {
                const QString sinceDate = QLocale::system().toString(QDate::currentDate(), QLocale::ShortFormat);
                error->errorItem->sinceDate = sinceDate;
                fileItem->child(j, COLUMN_SINCE_DATE)->setText(sinceDate);
            }

            if (oldError && error->errorItem->tags.isEmpty())
                error->errorItem->tags = oldError->tags;
        }
    }
}

void ResultsTree::updateSettings(bool showFullPath,
                                 bool saveFullPath,
                                 bool saveAllErrors,
                                 bool showErrorId,
                                 bool showInconclusive)
{
    if (mShowFullPath != showFullPath) {
        mShowFullPath = showFullPath;
        refreshFilePaths();
    }

    mSaveFullPath = saveFullPath;
    mSaveAllErrors = saveAllErrors;

    showIdColumn(showErrorId);
    showInconclusiveColumn(showInconclusive);
}

void ResultsTree::setCheckDirectory(const QString &dir)
{
    mCheckPath = dir;
}

const QString& ResultsTree::getCheckDirectory() const
{
    return mCheckPath;
}

void ResultsTree::setResultsSource(ResultsSource source)
{
    mResultsSource = source;
}

QString ResultsTree::stripPath(const QString &path, bool saving) const
{
    if ((!saving && mShowFullPath) || (saving && mSaveFullPath)) {
        return QString(path);
    }

    QDir dir(mCheckPath);
    return dir.relativeFilePath(path);
}

void ResultsTree::refreshFilePaths(ResultItem *fileItem)
{
    if (!fileItem)
        return;

    auto refreshItem = [this](ResultItem* item) {
        item->setText(QDir::toNativeSeparators(stripPath(item->getErrorPathItem().file, false)));
    };

    refreshItem(fileItem);

    //Loop through all errors within this file
    for (int i = 0; i < fileItem->rowCount(); i++) {
        //Get error i
        auto *error = dynamic_cast<ResultItem*>(fileItem->child(i, COLUMN_FILE));

        if (!error) {
            continue;
        }

        //Update this error's text
        refreshItem(error);

        //Loop through all files within the error
        for (int j = 0; j < error->rowCount(); j++) {
            //Get file
            auto *child = dynamic_cast<ResultItem*>(error->child(j, COLUMN_FILE));
            if (child) {
                //Update file's path
                refreshItem(child);
            }
        }
    }
}

void ResultsTree::refreshFilePaths()
{
    qDebug("Refreshing file paths");

    //Go through all file items (these are parent items that contain the errors)
    for (int row = 0; row < mModel->rowCount(); row++) {
        refreshFilePaths(dynamic_cast<ResultItem*>(mModel->item(row, COLUMN_FILE)));
    }
}

bool ResultsTree::hasVisibleResults() const
{
    return mVisibleErrors;
}

bool ResultsTree::hasResults() const
{
    return mModel->rowCount() > 0;
}

void ResultsTree::translate()
{
    mModel->setHorizontalHeaderLabels(getLabels());
    //TODO go through all the errors in the tree and translate severity and message
}

void ResultsTree::showIdColumn(bool show)
{
    mShowErrorId = show;
    if (show)
        showColumn(COLUMN_ID);
    else
        hideColumn(COLUMN_ID);
}

void ResultsTree::showInconclusiveColumn(bool show)
{
    if (show)
        showColumn(COLUMN_INCONCLUSIVE);
    else
        hideColumn(COLUMN_INCONCLUSIVE);
}

void ResultsTree::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    const auto *item = dynamic_cast<ResultItem*>(mModel->itemFromIndex(current));
    emit treeSelectionChanged(item);
}

bool ResultsTree::isCertReport() const {
    return mReportType == ReportType::certC || mReportType == ReportType::certCpp;
}

bool ResultsTree::isAutosarMisraReport() const {
    return mReportType == ReportType::autosar ||
           mReportType == ReportType::misraC2012 ||
           mReportType == ReportType::misraC2023 ||
           mReportType == ReportType::misraC2025 ||
           mReportType == ReportType::misraCpp2008 ||
           mReportType == ReportType::misraCpp2023;
}
