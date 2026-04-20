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

#include "mainwindow.h"

#include "addoninfo.h"
#include "applicationlist.h"
#include "aboutdialog.h"
#include "analyzerinfo.h"
#include "checkers.h"
#include "checkstatistics.h"
#include "checkthread.h"
#include "common.h"
#include "cppcheck.h"
#include "errortypes.h"
#include "filelist.h"
#include "filesettings.h"
#include "compliancereportdialog.h"
#include "fileviewdialog.h"
#include "helpdialog.h"
#include "importproject.h"
#include "librarydialog.h"
#include "path.h"
#include "platform.h"
#include "projectfile.h"
#include "projectfiledialog.h"
#include "report.h"
#include "resultstree.h"
#include "resultsview.h"
#include "scratchpad.h"
#include "settings.h"
#include "showtypes.h"
#include "statsdialog.h"
#include "settingsdialog.h"
#include "standards.h"
#include "suppressions.h"
#include "threadhandler.h"
#include "threadresult.h"
#include "translationhandler.h"
#include "utils.h"
#include "threaddetails.h"

#include "ui_mainwindow.h"

#include "frontend.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <QApplication>
#include <QAction>
#include <QActionGroup>
#include <QByteArray>
#include <QChar>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QSize>
#include <QTimer>
#include <QTemporaryFile>
#include <QToolBar>
#include <QUrl>
#include <QVariant>
#include <Qt>

#include "json.h"

static const QString compile_commands_json("compile_commands.json");

static QString fromNativePath(const QString& p) {
#ifdef Q_OS_WIN
    QString ret(p);
    ret.replace('\\', '/');
    return ret;
#else
    return p;
#endif
}

MainWindow::MainWindow(TranslationHandler* th, QSettings* settings) :
    mSettings(settings),
    mApplications(new ApplicationList(this)),
    mTranslation(th),
    mUI(new Ui::MainWindow),
    mPlatformActions(new QActionGroup(this)),
    mCStandardActions(new QActionGroup(this)),
    mCppStandardActions(new QActionGroup(this)),
    mSelectLanguageActions(new QActionGroup(this)),
    mSelectReportActions(new QActionGroup(this))
{
    {
        Settings tempSettings;
        tempSettings.exename = QCoreApplication::applicationFilePath().toStdString();
        Suppressions tempSupprs;
        Settings::loadCppcheckCfg(tempSettings, tempSupprs); // TODO: how to handle error?
        mCppcheckCfgProductName = QString::fromStdString(tempSettings.cppcheckCfgProductName);
        mCppcheckCfgAbout = QString::fromStdString(tempSettings.cppcheckCfgAbout);
    }

    mUI->setupUi(this);
    mThread = new ThreadHandler(this);
    mUI->mResults->initialize(mSettings, mApplications, mThread);

    // Filter timer to delay filtering results slightly while typing
    mFilterTimer = new QTimer(this);
    mFilterTimer->setInterval(500);
    mFilterTimer->setSingleShot(true);
    connect(mFilterTimer, &QTimer::timeout, this, &MainWindow::filterResults);

    // "Filter" toolbar
    mLineEditFilter = new QLineEdit(mUI->mToolBarFilter);
    mLineEditFilter->setPlaceholderText(tr("Quick Filter:"));
    mLineEditFilter->setClearButtonEnabled(true);
    mUI->mToolBarFilter->addWidget(mLineEditFilter);
    connect(mLineEditFilter, SIGNAL(textChanged(QString)), mFilterTimer, SLOT(start()));
    connect(mLineEditFilter, &QLineEdit::returnPressed, this, &MainWindow::filterResults);

    connect(mUI->mActionPrint, SIGNAL(triggered()), mUI->mResults, SLOT(print()));
    connect(mUI->mActionPrintPreview, SIGNAL(triggered()), mUI->mResults, SLOT(printPreview()));
    connect(mUI->mActionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(mUI->mActionAnalyzeFiles, &QAction::triggered, this, &MainWindow::analyzeFiles);
    connect(mUI->mActionAnalyzeDirectory, &QAction::triggered, this, &MainWindow::analyzeDirectory);
    connect(mUI->mActionSettings, &QAction::triggered, this, &MainWindow::programSettings);
    connect(mUI->mActionClearResults, &QAction::triggered, this, &MainWindow::clearResults);
    connect(mUI->mActionOpenXML, &QAction::triggered, this, &MainWindow::openResults);

    connect(mUI->mActionShowStyle, &QAction::toggled, this, &MainWindow::showStyle);
    connect(mUI->mActionShowErrors, &QAction::toggled, this, &MainWindow::showErrors);
    connect(mUI->mActionShowWarnings, &QAction::toggled, this, &MainWindow::showWarnings);
    connect(mUI->mActionShowPortability, &QAction::toggled, this, &MainWindow::showPortability);
    connect(mUI->mActionShowPerformance, &QAction::toggled, this, &MainWindow::showPerformance);
    connect(mUI->mActionShowInformation, &QAction::toggled, this, &MainWindow::showInformation);
    connect(mUI->mActionShowCppcheck, &QAction::toggled, mUI->mResults, &ResultsView::showCppcheckResults);
    connect(mUI->mActionShowClang, &QAction::toggled, mUI->mResults, &ResultsView::showClangResults);
    connect(mUI->mActionCheckAll, &QAction::triggered, this, &MainWindow::checkAll);
    connect(mUI->mActionUncheckAll, &QAction::triggered, this, &MainWindow::uncheckAll);
    connect(mUI->mActionCollapseAll, &QAction::triggered, mUI->mResults, &ResultsView::collapseAllResults);
    connect(mUI->mActionExpandAll, &QAction::triggered, mUI->mResults, &ResultsView::expandAllResults);
    connect(mUI->mActionShowHidden, &QAction::triggered, mUI->mResults, &ResultsView::showHiddenResults);
    connect(mUI->mActionViewStats, &QAction::triggered, this, &MainWindow::showStatistics);
    connect(mUI->mActionLibraryEditor, &QAction::triggered, this, &MainWindow::showLibraryEditor);
    connect(mUI->mActionShowThreadDetails, &QAction::triggered, this, &MainWindow::showThreadDetails);

    connect(mUI->mActionReanalyzeModified, &QAction::triggered, this, &MainWindow::reAnalyzeModified);
    connect(mUI->mActionReanalyzeAll, &QAction::triggered, this, &MainWindow::reAnalyzeAll);
    connect(mUI->mActionCheckLibrary, &QAction::triggered, this, &MainWindow::checkLibrary);
    connect(mUI->mActionCheckConfiguration, &QAction::triggered, this, &MainWindow::checkConfiguration);

    connect(mUI->mActionStop, &QAction::triggered, this, &MainWindow::stopAnalysis);
    connect(mUI->mActionSave, &QAction::triggered, this, &MainWindow::save);
    connect(mUI->mActionComplianceReport, &QAction::triggered, this, &MainWindow::complianceReport);

    // About menu
    connect(mUI->mActionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(mUI->mActionLicense, &QAction::triggered, this, &MainWindow::showLicense);
    mUI->mActionEULA->setVisible(isCppcheckPremium());
    connect(mUI->mActionEULA, &QAction::triggered, this, &MainWindow::showEULA);

    // View > Toolbar menu
    connect(mUI->mActionToolBarMain, SIGNAL(toggled(bool)), this, SLOT(toggleMainToolBar()));
    connect(mUI->mActionToolBarView, SIGNAL(toggled(bool)), this, SLOT(toggleViewToolBar()));
    connect(mUI->mActionToolBarFilter, SIGNAL(toggled(bool)), this, SLOT(toggleFilterToolBar()));

    connect(mUI->mActionAuthors, &QAction::triggered, this, &MainWindow::showAuthors);
    connect(mThread, &ThreadHandler::done, this, &MainWindow::analysisDone);
    connect(mThread, &ThreadHandler::log, mUI->mResults, &ResultsView::log);
    connect(mThread, &ThreadHandler::debugError, mUI->mResults, &ResultsView::debugError);
    connect(mUI->mResults, &ResultsView::gotResults, this, &MainWindow::resultsAdded);
    connect(mUI->mResults, &ResultsView::resultsHidden, mUI->mActionShowHidden, &QAction::setEnabled);
    connect(mUI->mResults, &ResultsView::checkSelected, this, &MainWindow::performSelectedFilesCheck);
    connect(mUI->mResults, &ResultsView::suppressIds, this, &MainWindow::suppressIds);
    connect(mUI->mMenuView, &QMenu::aboutToShow, this, &MainWindow::aboutToShowViewMenu);

    // Change report type
    connect(mUI->mActionReportNormal, &QAction::triggered, this, &MainWindow::changeReportType);
    connect(mUI->mActionReportAutosar, &QAction::triggered, this, &MainWindow::changeReportType);
    connect(mUI->mActionReportCertC, &QAction::triggered, this, &MainWindow::changeReportType);
    connect(mUI->mActionReportCertCpp, &QAction::triggered, this, &MainWindow::changeReportType);
    connect(mUI->mActionReportMisraC, &QAction::triggered, this, &MainWindow::changeReportType);
    connect(mUI->mActionReportMisraCpp2008, &QAction::triggered, this, &MainWindow::changeReportType);
    connect(mUI->mActionReportMisraCpp2023, &QAction::triggered, this, &MainWindow::changeReportType);

    // File menu
    connect(mUI->mActionNewProjectFile, &QAction::triggered, this, &MainWindow::newProjectFile);
    connect(mUI->mActionOpenProjectFile, &QAction::triggered, this, &MainWindow::openProjectFile);
    connect(mUI->mActionShowScratchpad, &QAction::triggered, this, &MainWindow::showScratchpad);
    connect(mUI->mActionCloseProjectFile, &QAction::triggered, this, &MainWindow::closeProjectFile);
    connect(mUI->mActionEditProjectFile, &QAction::triggered, this, &MainWindow::editProjectFile);

    connect(mUI->mActionHelpContents, &QAction::triggered, this, &MainWindow::openHelpContents);

    loadSettings();

    mThread->initialize(mUI->mResults);
    if (mProjectFile) {
        enableProjectActions(true);
        formatAndSetTitle(tr("Project:") + ' ' + mProjectFile->getFilename());
    } else {
        enableProjectActions(false);
        formatAndSetTitle();
    }

    mUI->mActionComplianceReport->setVisible(isCppcheckPremium());

    enableCheckButtons(true);

    mUI->mActionPrint->setShortcut(QKeySequence::Print);
    enableResultsButtons();
    enableProjectOpenActions(true);

    // Must setup MRU menu before CLI param handling as it can load a
    // project file and update MRU menu.
    for (int i = 0; i < MaxRecentProjects; ++i) {
        mRecentProjectActs[i] = new QAction(this);
        mRecentProjectActs[i]->setVisible(false);
        connect(mRecentProjectActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentProject()));
    }
    mRecentProjectActs[MaxRecentProjects] = nullptr; // The separator
    mUI->mActionProjectMRU->setVisible(false);
    updateMRUMenuItems();

    QStringList args = QCoreApplication::arguments();
    //Remove the application itself
    args.removeFirst();
    if (!args.isEmpty()) {
        handleCLIParams(args);
    }

    for (int i = 0; i < mPlatforms.getCount(); i++) {
        PlatformData platform = mPlatforms.mPlatforms[i];
        auto *action = new QAction(this);
        platform.mActMainWindow = action;
        mPlatforms.mPlatforms[i] = platform;
        action->setText(platform.mTitle);
        action->setData(platform.mType);
        action->setCheckable(true);
        action->setActionGroup(mPlatformActions);
        mUI->mMenuAnalyze->insertAction(mUI->mActionPlatforms, action);
        connect(action, SIGNAL(triggered()), this, SLOT(selectPlatform()));
    }

    mUI->mActionReportNormal->setActionGroup(mSelectReportActions);
    mUI->mActionReportAutosar->setActionGroup(mSelectReportActions);
    mUI->mActionReportCertC->setActionGroup(mSelectReportActions);
    mUI->mActionReportCertCpp->setActionGroup(mSelectReportActions);
    mUI->mActionReportMisraC->setActionGroup(mSelectReportActions);
    mUI->mActionReportMisraCpp2008->setActionGroup(mSelectReportActions);
    mUI->mActionReportMisraCpp2023->setActionGroup(mSelectReportActions);

    mUI->mActionC89->setActionGroup(mCStandardActions);
    mUI->mActionC99->setActionGroup(mCStandardActions);
    mUI->mActionC11->setActionGroup(mCStandardActions);
    //mUI->mActionC17->setActionGroup(mCStandardActions);
    //mUI->mActionC23->setActionGroup(mCStandardActions);
    //mUI->mActionC2Y->setActionGroup(mCStandardActions);

    mUI->mActionCpp03->setActionGroup(mCppStandardActions);
    mUI->mActionCpp11->setActionGroup(mCppStandardActions);
    mUI->mActionCpp14->setActionGroup(mCppStandardActions);
    mUI->mActionCpp17->setActionGroup(mCppStandardActions);
    mUI->mActionCpp20->setActionGroup(mCppStandardActions);
    //mUI->mActionCpp23->setActionGroup(mCppStandardActions);
    //mUI->mActionCpp26->setActionGroup(mCppStandardActions);

    mUI->mActionEnforceC->setActionGroup(mSelectLanguageActions);
    mUI->mActionEnforceCpp->setActionGroup(mSelectLanguageActions);
    mUI->mActionAutoDetectLanguage->setActionGroup(mSelectLanguageActions);

    // TODO: we no longer default to a Windows platform in CLI - so we should probably also get rid of this in the GUI
    // For Windows platforms default to Win32 checked platform.
    // For other platforms default to unspecified/default which means the
    // platform Cppcheck GUI was compiled on.
#if defined(_WIN32)
    constexpr Platform::Type defaultPlatform = Platform::Type::Win32W;
#else
    constexpr Platform::Type defaultPlatform = Platform::Type::Unspecified;
#endif
    PlatformData &platform = mPlatforms.get(static_cast<Platform::Type>(mSettings->value(SETTINGS_CHECKED_PLATFORM, defaultPlatform).toInt()));
    platform.mActMainWindow->setChecked(true);

    mNetworkAccessManager = new QNetworkAccessManager(this);
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished,
            this, &MainWindow::replyFinished);

    mUI->mLabelInformation->setVisible(false);
    mUI->mButtonHideInformation->setVisible(false);
    connect(mUI->mButtonHideInformation, &QPushButton::clicked,
            this, &MainWindow::hideInformation);

    if (mSettings->value(SETTINGS_CHECK_FOR_UPDATES, false).toBool()) {
        // Is there a new version?
        if (isCppcheckPremium()) {
            const QUrl url("https://files.cppchecksolutions.com/version.txt");
            mNetworkAccessManager->get(QNetworkRequest(url));
        } else {
            const QUrl url("https://cppcheck.sourceforge.io/version.txt");
            mNetworkAccessManager->get(QNetworkRequest(url));
        }
    } else {
        delete mUI->mLayoutInformation;
    }

    changeReportType();
}

MainWindow::~MainWindow()
{
    delete mProjectFile;
    delete mScratchPad;
    delete mUI;
}

void MainWindow::handleCLIParams(const QStringList &params)
{
    int index;
    if (params.contains("-p")) {
        index = params.indexOf("-p");
        if ((index + 1) < params.length())
            loadProjectFile(params[index + 1]);
    } else if (params.contains("-l")) {
        QString logFile;
        index = params.indexOf("-l");
        if ((index + 1) < params.length())
            logFile = params[index + 1];

        if (params.contains("-d")) {
            QString checkedDir;
            index = params.indexOf("-d");
            if ((index + 1) < params.length())
                checkedDir = params[index + 1];

            loadResults(logFile, checkedDir);
        } else {
            loadResults(logFile);
        }
    } else if ((index = params.indexOf(QRegularExpression(".*\\.cppcheck$", QRegularExpression::CaseInsensitiveOption))) >= 0 && index < params.length() && QFile(params[index]).exists()) {
        loadProjectFile(params[index]);
    } else if ((index = params.indexOf(QRegularExpression(".*\\.xml$", QRegularExpression::CaseInsensitiveOption))) >= 0 && index < params.length() && QFile(params[index]).exists()) {
        loadResults(params[index],QDir::currentPath());
    } else
        doAnalyzeFiles(params);
}

void MainWindow::loadSettings()
{
    // Window/dialog sizes
    if (mSettings->value(SETTINGS_WINDOW_MAXIMIZED, false).toBool()) {
        showMaximized();
    } else {
        resize(mSettings->value(SETTINGS_WINDOW_WIDTH, 800).toInt(),
               mSettings->value(SETTINGS_WINDOW_HEIGHT, 600).toInt());
    }

    const ReportType reportType = static_cast<ReportType>(mSettings->value(SETTINGS_REPORT_TYPE, static_cast<int>(ReportType::normal)).toInt());
    mUI->mActionReportNormal->setChecked(reportType <= ReportType::normal);
    mUI->mActionReportAutosar->setChecked(reportType == ReportType::autosar);
    mUI->mActionReportCertC->setChecked(reportType == ReportType::certC);
    mUI->mActionReportCertCpp->setChecked(reportType == ReportType::certCpp);
    mUI->mActionReportMisraC->setChecked(reportType == ReportType::misraC2012 ||
                                         reportType == ReportType::misraC2023 ||
                                         reportType == ReportType::misraC2025);
    mUI->mActionReportMisraCpp2008->setChecked(reportType == ReportType::misraCpp2008);
    mUI->mActionReportMisraCpp2023->setChecked(reportType == ReportType::misraCpp2023);

    const ShowTypes &types = mUI->mResults->getShowTypes();
    mUI->mActionShowStyle->setChecked(types.isShown(ShowTypes::ShowStyle));
    mUI->mActionShowErrors->setChecked(types.isShown(ShowTypes::ShowErrors));
    mUI->mActionShowWarnings->setChecked(types.isShown(ShowTypes::ShowWarnings));
    mUI->mActionShowPortability->setChecked(types.isShown(ShowTypes::ShowPortability));
    mUI->mActionShowPerformance->setChecked(types.isShown(ShowTypes::ShowPerformance));
    mUI->mActionShowInformation->setChecked(types.isShown(ShowTypes::ShowInformation));
    mUI->mActionShowCppcheck->setChecked(true);
    mUI->mActionShowClang->setChecked(true);

    Standards standards;
    standards.setC(mSettings->value(SETTINGS_STD_C, QString()).toString().toStdString());
    mUI->mActionC89->setChecked(standards.c == Standards::C89);
    mUI->mActionC99->setChecked(standards.c == Standards::C99);
    mUI->mActionC11->setChecked(standards.c == Standards::C11);
    //mUI->mActionC17->setChecked(standards.c == Standards::C17);
    //mUI->mActionC23->setChecked(standards.c == Standards::C23);
    //mUI->mActionC2Y->setChecked(standards.c == Standards::C2Y);
    standards.setCPP(mSettings->value(SETTINGS_STD_CPP, QString()).toString().toStdString());
    mUI->mActionCpp03->setChecked(standards.cpp == Standards::CPP03);
    mUI->mActionCpp11->setChecked(standards.cpp == Standards::CPP11);
    mUI->mActionCpp14->setChecked(standards.cpp == Standards::CPP14);
    mUI->mActionCpp17->setChecked(standards.cpp == Standards::CPP17);
    mUI->mActionCpp20->setChecked(standards.cpp == Standards::CPP20);
    //mUI->mActionCpp23->setChecked(standards.cpp == Standards::CPP23);
    //mUI->mActionCpp26->setChecked(standards.cpp == Standards::CPP26);

    // Main window settings
    const bool showMainToolbar = mSettings->value(SETTINGS_TOOLBARS_MAIN_SHOW, true).toBool();
    mUI->mActionToolBarMain->setChecked(showMainToolbar);
    mUI->mToolBarMain->setVisible(showMainToolbar);

    const bool showViewToolbar = mSettings->value(SETTINGS_TOOLBARS_VIEW_SHOW, true).toBool();
    mUI->mActionToolBarView->setChecked(showViewToolbar);
    mUI->mToolBarView->setVisible(showViewToolbar);

    const bool showFilterToolbar = mSettings->value(SETTINGS_TOOLBARS_FILTER_SHOW, true).toBool();
    mUI->mActionToolBarFilter->setChecked(showFilterToolbar);
    mUI->mToolBarFilter->setVisible(showFilterToolbar);

    const Standards::Language enforcedLanguage = static_cast<Standards::Language>(mSettings->value(SETTINGS_ENFORCED_LANGUAGE, 0).toInt());
    if (enforcedLanguage == Standards::Language::CPP)
        mUI->mActionEnforceCpp->setChecked(true);
    else if (enforcedLanguage == Standards::Language::C)
        mUI->mActionEnforceC->setChecked(true);
    else
        mUI->mActionAutoDetectLanguage->setChecked(true);

    const bool succeeded = mApplications->loadSettings();
    if (!succeeded) {
        const QString msg = tr("There was a problem with loading the editor application settings.\n\n"
                               "This is probably because the settings were changed between the Cppcheck versions. "
                               "Please check (and fix) the editor application settings, otherwise the editor "
                               "program might not start correctly.");
        QMessageBox msgBox(QMessageBox::Warning,
                           tr("Cppcheck"),
                           msg,
                           QMessageBox::Ok,
                           this);
        msgBox.exec();
    }

    const QString projectFile = mSettings->value(SETTINGS_OPEN_PROJECT, QString()).toString();
    if (!projectFile.isEmpty() && QCoreApplication::arguments().size()==1) {
        QFileInfo inf(projectFile);
        if (inf.exists() && inf.isReadable()) {
            setPath(SETTINGS_LAST_PROJECT_PATH, projectFile);
            mProjectFile = new ProjectFile(this);
            mProjectFile->setActiveProject();
            mProjectFile->read(projectFile);
            loadLastResults();
            QDir::setCurrent(inf.absolutePath());
        }
    }
}

static ReportType getMisraCReportType(const QStringList &standards)
{
    if (standards.contains(CODING_STANDARD_MISRA_C_2023))
        return ReportType::misraC2023;
    if (standards.contains(CODING_STANDARD_MISRA_C_2025))
        return ReportType::misraC2025;
    return ReportType::misraC2012;
}

void MainWindow::saveSettings() const
{
    // Window/dialog sizes
    mSettings->setValue(SETTINGS_WINDOW_WIDTH, size().width());
    mSettings->setValue(SETTINGS_WINDOW_HEIGHT, size().height());
    mSettings->setValue(SETTINGS_WINDOW_MAXIMIZED, isMaximized());

    const ReportType reportType = mUI->mActionReportAutosar->isChecked() ? ReportType::autosar :
                                  mUI->mActionReportCertC->isChecked() ? ReportType::certC :
                                  mUI->mActionReportCertCpp->isChecked() ? ReportType::certCpp :
                                  mUI->mActionReportMisraC->isChecked() ? (mProjectFile ? getMisraCReportType(mProjectFile->getCodingStandards()) : ReportType::misraC2012) :
                                  mUI->mActionReportMisraCpp2008->isChecked() ? ReportType::misraCpp2008 :
                                  mUI->mActionReportMisraCpp2023->isChecked() ? ReportType::misraCpp2023 :
                                  ReportType::normal;
    mSettings->setValue(SETTINGS_REPORT_TYPE, static_cast<int>(reportType));

    // Show * states
    mSettings->setValue(SETTINGS_SHOW_STYLE, mUI->mActionShowStyle->isChecked());
    mSettings->setValue(SETTINGS_SHOW_ERRORS, mUI->mActionShowErrors->isChecked());
    mSettings->setValue(SETTINGS_SHOW_WARNINGS, mUI->mActionShowWarnings->isChecked());
    mSettings->setValue(SETTINGS_SHOW_PORTABILITY, mUI->mActionShowPortability->isChecked());
    mSettings->setValue(SETTINGS_SHOW_PERFORMANCE, mUI->mActionShowPerformance->isChecked());
    mSettings->setValue(SETTINGS_SHOW_INFORMATION, mUI->mActionShowInformation->isChecked());

    if (mUI->mActionC89->isChecked())
        mSettings->setValue(SETTINGS_STD_C, "C89");
    if (mUI->mActionC99->isChecked())
        mSettings->setValue(SETTINGS_STD_C, "C99");
    if (mUI->mActionC11->isChecked())
        mSettings->setValue(SETTINGS_STD_C, "C11");
    //if (mUI->mActionC17->isChecked())
    //    mSettings->setValue(SETTINGS_STD_C, "C17");
    //if (mUI->mActionC23->isChecked())
    //    mSettings->setValue(SETTINGS_STD_C, "C23");
    //if (mUI->mActionC2Y->isChecked())
    //    mSettings->setValue(SETTINGS_STD_C, "C2Y");

    if (mUI->mActionCpp03->isChecked())
        mSettings->setValue(SETTINGS_STD_CPP, "C++03");
    if (mUI->mActionCpp11->isChecked())
        mSettings->setValue(SETTINGS_STD_CPP, "C++11");
    if (mUI->mActionCpp14->isChecked())
        mSettings->setValue(SETTINGS_STD_CPP, "C++14");
    if (mUI->mActionCpp17->isChecked())
        mSettings->setValue(SETTINGS_STD_CPP, "C++17");
    if (mUI->mActionCpp20->isChecked())
        mSettings->setValue(SETTINGS_STD_CPP, "C++20");
    //if (mUI.mActionCpp23->isChecked())
    //    mSettings->setValue(SETTINGS_STD_CPP, "C++23");
    //if (mUI.mActionCpp26->isChecked())
    //    mSettings->setValue(SETTINGS_STD_CPP, "C++26");

    // Main window settings
    mSettings->setValue(SETTINGS_TOOLBARS_MAIN_SHOW, mUI->mToolBarMain->isVisible());
    mSettings->setValue(SETTINGS_TOOLBARS_VIEW_SHOW, mUI->mToolBarView->isVisible());
    mSettings->setValue(SETTINGS_TOOLBARS_FILTER_SHOW, mUI->mToolBarFilter->isVisible());

    if (mUI->mActionEnforceCpp->isChecked())
        mSettings->setValue(SETTINGS_ENFORCED_LANGUAGE, Standards::Language::CPP);
    else if (mUI->mActionEnforceC->isChecked())
        mSettings->setValue(SETTINGS_ENFORCED_LANGUAGE, Standards::Language::C);
    else
        mSettings->setValue(SETTINGS_ENFORCED_LANGUAGE, Standards::Language::None);

    mApplications->saveSettings();

    mSettings->setValue(SETTINGS_LANGUAGE, mTranslation->getCurrentLanguage());

    mSettings->setValue(SETTINGS_OPEN_PROJECT, mProjectFile ? mProjectFile->getFilename() : QString());

    mUI->mResults->saveSettings(mSettings);
}

void MainWindow::doAnalyzeProject(ImportProject p, const bool checkLibrary, const bool checkConfiguration)
{
    Settings checkSettings;
    auto supprs = std::make_shared<Suppressions>();
    if (!getCppcheckSettings(checkSettings, *supprs))
        return;

    clearResults();

    mIsLogfileLoaded = false;
    if (mProjectFile) {
        std::vector<std::string> v;
        const QStringList excluded = mProjectFile->getExcludedPaths();
        std::transform(excluded.cbegin(), excluded.cend(), std::back_inserter(v), [](const QString& e) {
            return e.toStdString();
        });
        p.ignorePaths(v);

        if (!mProjectFile->getAnalyzeAllVsConfigs()) {
            const Platform::Type platform = static_cast<Platform::Type>(mSettings->value(SETTINGS_CHECKED_PLATFORM, 0).toInt());
            std::vector<std::string> configurations;
            const QStringList configs = mProjectFile->getVsConfigurations();
            std::transform(configs.cbegin(), configs.cend(), std::back_inserter(configurations), [](const QString& e) {
                return e.toStdString();
            });
            p.selectVsConfigurations(platform, configurations);
        }
    } else {
        enableProjectActions(false);
    }

    mUI->mResults->setResultsSource(ResultsTree::ResultsSource::Analysis);
    mThread->clearFiles();

    mUI->mResults->checkingStarted(p.fileSettings.size());

    QDir inf(mCurrentDirectory);
    const QString checkPath = inf.canonicalPath();
    setPath(SETTINGS_LAST_CHECK_PATH, checkPath);

    checkLockDownUI(); // lock UI while checking

    mUI->mResults->setCheckDirectory(checkPath);
    checkSettings.force = false;
    checkSettings.checkLibrary = checkLibrary;
    checkSettings.checkConfiguration = checkConfiguration;

    if (mProjectFile)
        qDebug() << "Checking project file" << mProjectFile->getFilename();

    if (!checkSettings.buildDir.empty()) {
        checkSettings.loadSummaries();
        std::list<std::string> sourcefiles;
        AnalyzerInformation::writeFilesTxt(checkSettings.buildDir, sourcefiles, p.fileSettings);
    }

    //mThread->SetanalyzeProject(true);
    if (mProjectFile) {
        mThread->setAddonsAndTools(mProjectFile->getAddonsAndTools());
        QString clangHeaders = mSettings->value(SETTINGS_VS_INCLUDE_PATHS).toString();
        mThread->setClangIncludePaths(clangHeaders.split(";"));
        mThread->setSuppressions(mProjectFile->getSuppressions());
    }

    const Standards::Language enforcedLang = static_cast<Standards::Language>(mSettings->value(SETTINGS_ENFORCED_LANGUAGE, 0).toInt());
    frontend::applyLang(p.fileSettings, checkSettings, enforcedLang);

    mThread->setProject(p);
    mThread->check(checkSettings, supprs);
    mUI->mResults->setCheckSettings(checkSettings);
}

void MainWindow::doAnalyzeFiles(const QStringList &files, const bool checkLibrary, const bool checkConfiguration)
{
    if (files.isEmpty())
        return;

    Settings checkSettings;
    auto supprs = std::make_shared<Suppressions>();
    if (!getCppcheckSettings(checkSettings, *supprs))
        return;

    clearResults();

    mIsLogfileLoaded = false;
    FileList pathList;
    pathList.addPathList(files);
    if (mProjectFile) {
        pathList.addExcludeList(mProjectFile->getExcludedPaths());
    } else {
        enableProjectActions(false);
    }
    QStringList fileNames = pathList.getFileList();

    mUI->mResults->clear(true);
    mUI->mResults->setResultsSource(ResultsTree::ResultsSource::Analysis);
    mThread->clearFiles();

    if (fileNames.isEmpty()) {
        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        tr("No suitable files found to analyze!"),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }

    std::list<FileWithDetails> fdetails = enrichFilesForAnalysis(fileNames, checkSettings);

    // TODO: lock UI here?
    mUI->mResults->checkingStarted(fdetails.size());
    mThread->setFiles(std::move(fdetails));
    if (mProjectFile && !checkConfiguration)
        mThread->setAddonsAndTools(mProjectFile->getAddonsAndTools());
    mThread->setSuppressions(mProjectFile ? mProjectFile->getCheckingSuppressions() : QList<SuppressionList::Suppression>());
    QDir inf(mCurrentDirectory);
    const QString checkPath = inf.canonicalPath();
    setPath(SETTINGS_LAST_CHECK_PATH, checkPath);

    checkLockDownUI(); // lock UI while checking

    mUI->mResults->setCheckDirectory(checkPath);
    checkSettings.checkLibrary = checkLibrary;
    checkSettings.checkConfiguration = checkConfiguration;

    if (mProjectFile)
        qDebug() << "Checking project file" << mProjectFile->getFilename();

    if (!checkSettings.buildDir.empty()) {
        checkSettings.loadSummaries();
        std::list<std::string> sourcefiles;
        std::transform(fileNames.cbegin(), fileNames.cend(), std::back_inserter(sourcefiles), [](const QString& s) {
            return s.toStdString();
        });
        AnalyzerInformation::writeFilesTxt(checkSettings.buildDir, sourcefiles, {});
    }

    mThread->setCheckFiles(true);
    mThread->check(checkSettings, supprs);
    mUI->mResults->setCheckSettings(checkSettings);
}

void MainWindow::analyzeCode(const QString& code, const QString& filename)
{
    Settings checkSettings;
    Suppressions supprs;
    if (!getCppcheckSettings(checkSettings, supprs))
        return;

    // TODO: split ErrorLogger from ThreadResult
    // Initialize dummy ThreadResult as ErrorLogger
    ThreadResult result;
    connect(&result, SIGNAL(progress(int,QString)),
            mUI->mResults, SLOT(progress(int,QString)));
    connect(&result, SIGNAL(error(ErrorItem)),
            mUI->mResults, SLOT(error(ErrorItem)));
    connect(&result, SIGNAL(log(QString)),
            mUI->mResults, SLOT(log(QString)));
    connect(&result, SIGNAL(debugError(ErrorItem)),
            mUI->mResults, SLOT(debugError(ErrorItem)));

    // Create CppCheck instance
    CppCheck cppcheck(checkSettings, supprs, result, nullptr, true, nullptr);

    // Check
    checkLockDownUI();
    clearResults();
    mUI->mResults->checkingStarted(1);
    {
        const std::string code_s = code.toStdString();
        // TODO: apply enforcedLanguage?
        cppcheck.checkBuffer(FileWithDetails(filename.toStdString(), Path::identify(filename.toStdString(), false), 0), code_s.data(), code_s.size());
    }
    analysisDone();

    // Expand results
    if (mUI->mResults->hasVisibleResults())
        mUI->mResults->expandAllResults();
}

QStringList MainWindow::selectFilesToAnalyze(QFileDialog::FileMode mode)
{
    if (mProjectFile) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Cppcheck"));
        const QString msg(tr("You must close the project file before selecting new files or directories!"));
        msgBox.setText(msg);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return QStringList();
    }

    QStringList selected;

    // NOTE: we use QFileDialog::getOpenFileNames() and
    // QFileDialog::getExistingDirectory() because they show native Windows
    // selection dialog which is a lot more usable than Qt:s own dialog.
    if (mode == QFileDialog::ExistingFiles) {
        QMap<QString,QString> filters;
        filters[tr("C/C++ Source")] = FileList::getDefaultFilters().join(" ");
        filters[tr("Compile database")] = compile_commands_json;
        filters[tr("Visual Studio")] = "*.sln *.slnx *.vcxproj";
        filters[tr("Borland C++ Builder 6")] = "*.bpr";
        QString lastFilter = mSettings->value(SETTINGS_LAST_ANALYZE_FILES_FILTER).toString();
        selected = QFileDialog::getOpenFileNames(this,
                                                 tr("Select files to analyze"),
                                                 getPath(SETTINGS_LAST_CHECK_PATH),
                                                 toFilterString(filters),
                                                 &lastFilter);
        mSettings->setValue(SETTINGS_LAST_ANALYZE_FILES_FILTER, lastFilter);

        if (selected.isEmpty())
            mCurrentDirectory.clear();
        else {
            QFileInfo inf(selected[0]);
            mCurrentDirectory = inf.absolutePath();
        }
        formatAndSetTitle();
    } else if (mode == QFileDialog::Directory) {
        QString dir = QFileDialog::getExistingDirectory(this,
                                                        tr("Select directory to analyze"),
                                                        getPath(SETTINGS_LAST_CHECK_PATH));
        if (!dir.isEmpty()) {
            qDebug() << "Setting current directory to: " << dir;
            mCurrentDirectory = dir;
            selected.append(dir);
            dir = QDir::toNativeSeparators(dir);
            formatAndSetTitle(dir);
        }
    }
    if (!mCurrentDirectory.isEmpty())
        setPath(SETTINGS_LAST_CHECK_PATH, mCurrentDirectory);

    return selected;
}

void MainWindow::analyzeFiles()
{
    Settings::terminate(false);

    QStringList selected = selectFilesToAnalyze(QFileDialog::ExistingFiles);

    const QString file0 = (!selected.empty() ? selected[0].toLower() : QString());
    if (file0.endsWith(".sln")
        || file0.endsWith(".slnx")
        || file0.endsWith(".vcxproj")
        || file0.endsWith(compile_commands_json)
        || file0.endsWith(".bpr")) {
        ImportProject p;
        p.import(selected[0].toStdString());

        if (file0.endsWith(".sln") || file0.endsWith(".slnx")) {
            QStringList configs;
            for (auto it = p.fileSettings.cbegin(); it != p.fileSettings.cend(); ++it) {
                const QString cfg(QString::fromStdString(it->cfg));
                if (!configs.contains(cfg))
                    configs.push_back(cfg);
            }
            configs.sort();

            bool ok = false;
            const QString cfg = QInputDialog::getItem(this, tr("Select configuration"), tr("Select the configuration that will be analyzed"), configs, 0, false, &ok);
            if (!ok)
                return;
            p.ignoreOtherConfigs(cfg.toStdString());
        }

        doAnalyzeProject(p); // TODO: avoid copy
        return;
    }

    doAnalyzeFiles(selected);
}

void MainWindow::analyzeDirectory()
{
    QStringList dir = selectFilesToAnalyze(QFileDialog::Directory);
    if (dir.isEmpty())
        return;

    QDir checkDir(dir[0]);
    QStringList filters;
    filters << "*.cppcheck";
    checkDir.setFilter(QDir::Files | QDir::Readable);
    checkDir.setNameFilters(filters);
    QStringList projFiles = checkDir.entryList();
    if (!projFiles.empty()) {
        if (projFiles.size() == 1) {
            // If one project file found, suggest loading it
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(tr("Cppcheck"));
            const QString msg(tr("Found project file: %1\n\nDo you want to "
                                 "load this project file instead?").arg(projFiles[0]));
            msgBox.setText(msg);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.addButton(QMessageBox::Yes);
            msgBox.addButton(QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            const int dlgResult = msgBox.exec();
            if (dlgResult == QMessageBox::Yes) {
                QString path = checkDir.canonicalPath();
                if (!path.endsWith("/"))
                    path += "/";
                path += projFiles[0];
                loadProjectFile(path);
            } else {
                doAnalyzeFiles(dir);
            }
        } else {
            // If multiple project files found inform that there are project
            // files also available.
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(tr("Cppcheck"));
            const QString msg(tr("Found project files from the directory.\n\n"
                                 "Do you want to proceed analysis without "
                                 "using any of these project files?"));
            msgBox.setText(msg);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.addButton(QMessageBox::Yes);
            msgBox.addButton(QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            const int dlgResult = msgBox.exec();
            if (dlgResult == QMessageBox::Yes) {
                doAnalyzeFiles(dir);
            }
        }
    } else {
        doAnalyzeFiles(dir);
    }
}

void MainWindow::addIncludeDirs(const QStringList &includeDirs, Settings &result)
{
    for (const QString& dir : includeDirs) {
        QString incdir;
        if (!QDir::isAbsolutePath(dir))
            incdir = mCurrentDirectory + "/";
        incdir += dir;
        incdir = QDir::cleanPath(incdir);

        // include paths must end with '/'
        if (!incdir.endsWith("/"))
            incdir += "/";
        result.includePaths.push_back(incdir.toStdString());
    }
}

Library::Error MainWindow::loadLibrary(Library &library, const QString &filename)
{
    Library::Error ret;

    // Try to load the library from the project folder..
    if (mProjectFile) {
        QString path = QFileInfo(mProjectFile->getFilename()).canonicalPath();
        QString libpath = path+"/"+filename;
        qDebug().noquote() << "looking for library '" + libpath + "'";
        ret = library.load(nullptr, libpath.toLatin1());
        if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
            return ret;
    }

    // Try to load the library from the application folder..
    const QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).canonicalPath();
    QString libpath = appPath+"/"+filename;
    qDebug().noquote() << "looking for library '" + libpath + "'";
    ret = library.load(nullptr, libpath.toLatin1());
    if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
        return ret;
    libpath = appPath+"/cfg/"+filename;
    qDebug().noquote() << "looking for library '" + libpath + "'";
    ret = library.load(nullptr, libpath.toLatin1());
    if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
        return ret;

#ifdef FILESDIR
    // Try to load the library from FILESDIR/cfg..
    const QString filesdir = FILESDIR;
    if (!filesdir.isEmpty()) {
        libpath = filesdir+"/cfg/"+filename;
        qDebug().noquote() << "looking for library '" + libpath + "'";
        ret = library.load(nullptr, libpath.toLatin1());
        if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
            return ret;
        libpath = filesdir+"/"+filename;
        qDebug().noquote() << "looking for library '" + libpath + "'";
        ret = library.load(nullptr, libpath.toLatin1());
        if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
            return ret;
    }
#endif

    // Try to load the library from the cfg subfolder..
    const QString datadir = getDataDir();
    if (!datadir.isEmpty()) {
        libpath = datadir+"/"+filename;
        qDebug().noquote() << "looking for library '" + libpath + "'";
        ret = library.load(nullptr, libpath.toLatin1());
        if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
            return ret;
        libpath = datadir+"/cfg/"+filename;
        qDebug().noquote() << "looking for library '" + libpath + "'";
        ret = library.load(nullptr, libpath.toLatin1());
        if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
            return ret;
    }

    qDebug().noquote() << "library not found: '" + filename + "'";

    return ret;
}

bool MainWindow::tryLoadLibrary(Library &library, const QString& filename)
{
    const Library::Error error = loadLibrary(library, filename);
    if (error.errorcode != Library::ErrorCode::OK) {
        if (error.errorcode == Library::ErrorCode::UNKNOWN_ELEMENT) {
            QMessageBox::information(this, tr("Information"), tr("The library '%1' contains unknown elements:\n%2").arg(filename).arg(error.reason.c_str()));
            return true;
        }

        QString errmsg;
        switch (error.errorcode) {
        case Library::ErrorCode::OK:
            break;
        case Library::ErrorCode::FILE_NOT_FOUND:
            errmsg = tr("File not found");
            break;
        case Library::ErrorCode::BAD_XML:
            errmsg = tr("Bad XML");
            break;
        case Library::ErrorCode::MISSING_ATTRIBUTE:
            errmsg = tr("Missing attribute");
            break;
        case Library::ErrorCode::BAD_ATTRIBUTE_VALUE:
            errmsg = tr("Bad attribute value");
            break;
        case Library::ErrorCode::UNSUPPORTED_FORMAT:
            errmsg = tr("Unsupported format");
            break;
        case Library::ErrorCode::DUPLICATE_PLATFORM_TYPE:
            errmsg = tr("Duplicate platform type");
            break;
        case Library::ErrorCode::PLATFORM_TYPE_REDEFINED:
            errmsg = tr("Platform type redefined");
            break;
        case Library::ErrorCode::DUPLICATE_DEFINE:
            errmsg = tr("Duplicate define");
            break;
        case Library::ErrorCode::UNKNOWN_ELEMENT:
            errmsg = tr("Unknown element");
            break;
        }
        if (!error.reason.empty())
            errmsg += " '" + QString::fromStdString(error.reason) + "'";
        QMessageBox::information(this, tr("Information"), tr("Failed to load the selected library '%1'.\n%2").arg(filename).arg(errmsg));
        return false;
    }
    return true;
}

QString MainWindow::loadAddon(Settings &settings, const QString &filesDir, const QString &pythonCmd, const QString& addon)
{
    const QString addonFilePath = fromNativePath(ProjectFile::getAddonFilePath(filesDir, addon));

    if (addonFilePath.isEmpty())
        return tr("File not found: '%1'").arg(addon);

    picojson::object obj;
    obj["script"] = picojson::value(addonFilePath.toStdString());
    if (!pythonCmd.isEmpty())
        obj["python"] = picojson::value(pythonCmd.toStdString());

    if (!isCppcheckPremium() && addon == "misra") {
        const QString misraFile = fromNativePath(mSettings->value(SETTINGS_MISRA_FILE).toString());
        if (!misraFile.isEmpty()) {
            QString arg;
            picojson::array arr;
            arg = "--rule-texts=" + misraFile;
            arr.emplace_back(arg.toStdString());
            obj["args"] = picojson::value(arr);
        }
    }

    const std::string& json_str = picojson::value(obj).serialize();

    AddonInfo addonInfo;
    const std::string errmsg = addonInfo.getAddonInfo(json_str, settings.exename);
    if (!errmsg.empty())
        return tr("Failed to load/setup addon %1: %2").arg(addon, QString::fromStdString(errmsg));
    settings.addonInfos.emplace_back(std::move(addonInfo));

    settings.addons.emplace(json_str);

    return "";
}

bool MainWindow::getCppcheckSettings(Settings& settings, Suppressions& supprs)
{
    saveSettings(); // Save settings

    Settings::terminate(true);

    settings.exename = QCoreApplication::applicationFilePath().toStdString();
    settings.templateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]";
    settings.reportProgress = 10;

    // default to --check-level=normal for GUI for now
    settings.setCheckLevel(Settings::CheckLevel::normal);

    const bool std = tryLoadLibrary(settings.library, "std.cfg");
    if (!std) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=<directory> at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.\n\nAnalysis is aborted.").arg("std.cfg"));
        return false;
    }

    const QString filesDir(getDataDir());
    const QString pythonCmd = fromNativePath(mSettings->value(SETTINGS_PYTHON_PATH).toString());

    {
        const QString cfgErr = QString::fromStdString(Settings::loadCppcheckCfg(settings, supprs));
        if (!cfgErr.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to load %1 - %2\n\nAnalysis is aborted.").arg("cppcheck.cfg").arg(cfgErr));
            return false;
        }

        settings.premium = startsWith(settings.cppcheckCfgProductName, "Cppcheck Premium");

        const auto cfgAddons = settings.addons;
        settings.addons.clear();
        for (const std::string& addon : cfgAddons) {
            // TODO: support addons which are a script and not a file
            const QString addonError = loadAddon(settings, filesDir, pythonCmd, QString::fromStdString(addon));
            if (!addonError.isEmpty()) {
                QMessageBox::critical(this, tr("Error"), tr("%1\n\nAnalysis is aborted.").arg(addonError));
                return false;
            }
        }
    }

    // If project file loaded, read settings from it
    if (mProjectFile) {
        QStringList dirs = mProjectFile->getIncludeDirs();
        addIncludeDirs(dirs, settings);

        settings.inlineSuppressions = mProjectFile->getInlineSuppression();

        const QStringList defines = mProjectFile->getDefines();
        for (const QString& define : defines) {
            if (!settings.userDefines.empty())
                settings.userDefines += ";";
            settings.userDefines += define.toStdString();
        }

        settings.clang = mProjectFile->clangParser;

        const QStringList undefines = mProjectFile->getUndefines();
        for (const QString& undefine : undefines)
            settings.userUndefs.insert(undefine.toStdString());

        settings.userIncludes.push_back(mProjectFile->getUserInclude().toStdString());

        const QStringList libraries = mProjectFile->getLibraries();
        for (const QString& library : libraries) {
            settings.libraries.emplace_back(library.toStdString());
            const QString filename = library + ".cfg";
            tryLoadLibrary(settings.library, filename);
        }

        for (const SuppressionList::Suppression &suppression : mProjectFile->getCheckingSuppressions()) {
            supprs.nomsg.addSuppression(suppression); // TODO: check result
        }

        // If importing a project, only check the given configuration
        if (mProjectFile->getImportProject().endsWith("json", Qt::CaseInsensitive))
            settings.maxConfigsProject = 1;

        const QString &buildDir = fromNativePath(mProjectFile->getBuildDir());
        if (!buildDir.isEmpty()) {
            if (QDir(buildDir).isAbsolute()) {
                settings.buildDir = buildDir.toStdString();
            } else {
                QString prjpath = QFileInfo(mProjectFile->getFilename()).absolutePath();
                settings.buildDir = (prjpath + '/' + buildDir).toStdString();
            }
        }

        const QString platform = mProjectFile->getPlatform();
        if (platform.endsWith(".xml")) {
            const std::vector<std::string> paths = {
                Path::getCurrentPath(), // TODO: do we want to look in CWD?
                QCoreApplication::applicationFilePath().toStdString(),
            };
            settings.platform.loadFromFile(paths, platform.toStdString());
        } else {
            for (int i = Platform::Type::Native; i <= Platform::Type::Unix64; i++) {
                const auto p = static_cast<Platform::Type>(i);
                if (platform == Platform::toString(p)) {
                    settings.platform.set(p);
                    break;
                }
            }
        }

        settings.maxCtuDepth = mProjectFile->getMaxCtuDepth();
        settings.maxTemplateRecursion = mProjectFile->getMaxTemplateRecursion();
        switch (mProjectFile->getCheckLevel()) {
        case ProjectFile::CheckLevel::reduced:
            settings.setCheckLevel(Settings::CheckLevel::reduced);
            break;
        case ProjectFile::CheckLevel::normal:
            settings.setCheckLevel(Settings::CheckLevel::normal);
            break;
        case ProjectFile::CheckLevel::exhaustive:
            settings.setCheckLevel(Settings::CheckLevel::exhaustive);
            break;
        }
        settings.checkHeaders = mProjectFile->getCheckHeaders();
        settings.checkUnusedTemplates = mProjectFile->getCheckUnusedTemplates();
        settings.safeChecks.classes = mProjectFile->safeChecks.classes;
        settings.safeChecks.externalFunctions = mProjectFile->safeChecks.externalFunctions;
        settings.safeChecks.internalFunctions = mProjectFile->safeChecks.internalFunctions;
        settings.safeChecks.externalVariables = mProjectFile->safeChecks.externalVariables;
        for (const QString& s : mProjectFile->getCheckUnknownFunctionReturn())
            settings.checkUnknownFunctionReturn.insert(s.toStdString());

        for (const QString& addon : mProjectFile->getAddons()) {
            if (isCppcheckPremium() && addon == "misra")
                continue;
            const QString addonError = loadAddon(settings, filesDir, pythonCmd, addon);
            if (!addonError.isEmpty()) {
                QMessageBox::critical(this, tr("Error"), tr("%1\n\nAnalysis is aborted.").arg(addonError));
                return false;
            }
        }

        if (isCppcheckPremium()) {
            QString premiumArgs;
            if (mProjectFile->getBughunting())
                premiumArgs += " --bughunting";
            if (mProjectFile->getCertIntPrecision() > 0)
                premiumArgs += " --cert-c-int-precision=" + QString::number(mProjectFile->getCertIntPrecision());
            for (const QString& c: mProjectFile->getCodingStandards())
                premiumArgs += " --" + c;
            if (!premiumArgs.contains("--misra-c-") && mProjectFile->getAddons().contains("misra"))
                premiumArgs += " --misra-c-2012";
            settings.premiumArgs = premiumArgs.mid(1).toStdString();
        }
    }
    else
        settings.inlineSuppressions = mSettings->value(SETTINGS_INLINE_SUPPRESSIONS, false).toBool();

    // Include directories (and files) are searched in listed order.
    // Global include directories must be added AFTER the per project include
    // directories so per project include directories can override global ones.
    const QString globalIncludes = mSettings->value(SETTINGS_GLOBAL_INCLUDE_PATHS).toString();
    if (!globalIncludes.isEmpty()) {
        QStringList includes = globalIncludes.split(";");
        addIncludeDirs(includes, settings);
    }

    settings.severity.enable(Severity::warning);
    settings.severity.enable(Severity::style);
    settings.severity.enable(Severity::performance);
    settings.severity.enable(Severity::portability);
    settings.severity.enable(Severity::information);
    settings.checks.enable(Checks::missingInclude);
    if (!settings.buildDir.empty())
        settings.checks.enable(Checks::unusedFunction);
    settings.debugwarnings = mSettings->value(SETTINGS_SHOW_DEBUG_WARNINGS, false).toBool();
    settings.quiet = false;
    settings.verbose = true;
    settings.force = mSettings->value(SETTINGS_CHECK_FORCE, 0).toBool();
    settings.outputFormat = Settings::OutputFormat::text;
    settings.jobs = mSettings->value(SETTINGS_CHECK_THREADS, 1).toInt();
    settings.certainty.setEnabled(Certainty::inconclusive, mSettings->value(SETTINGS_INCONCLUSIVE_ERRORS, false).toBool());
    if (!mProjectFile || settings.platform.type == Platform::Type::Unspecified)
        settings.platform.set(static_cast<Platform::Type>(mSettings->value(SETTINGS_CHECKED_PLATFORM, 0).toInt()));
    settings.standards.setCPP(mSettings->value(SETTINGS_STD_CPP, QString()).toString().toStdString());
    settings.standards.setC(mSettings->value(SETTINGS_STD_C, QString()).toString().toStdString());

    settings.jobs = std::max(settings.jobs, 1u);

    Settings::terminate(false);

    return true;
}

void MainWindow::analysisDone()
{
    if (mExiting) {
        close();
        return;
    }

    mUI->mResults->checkingFinished();
    enableCheckButtons(true);
    mUI->mActionSettings->setEnabled(true);
    mUI->mActionOpenXML->setEnabled(true);
    if (mProjectFile) {
        enableProjectActions(true);
    } else if (mIsLogfileLoaded) {
        mUI->mActionReanalyzeModified->setEnabled(false);
        mUI->mActionReanalyzeAll->setEnabled(false);
    }
    enableProjectOpenActions(true);
    mPlatformActions->setEnabled(true);
    mCStandardActions->setEnabled(true);
    mCppStandardActions->setEnabled(true);
    mSelectLanguageActions->setEnabled(true);
    mUI->mActionPosix->setEnabled(true);
    if (mScratchPad)
        mScratchPad->setEnabled(true);
    mUI->mActionViewStats->setEnabled(true);

    if (mProjectFile && !mProjectFile->getBuildDir().isEmpty()) {
        const QString prjpath = QFileInfo(mProjectFile->getFilename()).absolutePath();
        const QString buildDir = prjpath + '/' + mProjectFile->getBuildDir();
        if (QDir(buildDir).exists()) {
            mUI->mResults->saveStatistics(buildDir + "/statistics.txt");
            mUI->mResults->updateFromOldReport(buildDir + "/lastResults.xml");
            mUI->mResults->save(buildDir + "/lastResults.xml", Report::XMLV2, mCppcheckCfgProductName);
        }
    }

    enableResultsButtons();

    for (QAction* recentProjectAct : mRecentProjectActs) {
        if (recentProjectAct != nullptr)
            recentProjectAct->setEnabled(true);
    }

    // Notify user - if the window is not active - that check is ready
    QApplication::alert(this, 3000);
    if (mSettings->value(SETTINGS_SHOW_STATISTICS, false).toBool())
        showStatistics();
}

void MainWindow::checkLockDownUI()
{
    enableCheckButtons(false);
    mUI->mActionSettings->setEnabled(false);
    mUI->mActionOpenXML->setEnabled(false);
    enableProjectActions(false);
    enableProjectOpenActions(false);
    mPlatformActions->setEnabled(false);
    mCStandardActions->setEnabled(false);
    mCppStandardActions->setEnabled(false);
    mSelectLanguageActions->setEnabled(false);
    mUI->mActionPosix->setEnabled(false);
    if (mScratchPad)
        mScratchPad->setEnabled(false);

    for (QAction* recentProjectAct : mRecentProjectActs) {
        if (recentProjectAct != nullptr)
            recentProjectAct->setEnabled(false);
    }
}

void MainWindow::programSettings()
{
    SettingsDialog dialog(mApplications, mTranslation, isCppcheckPremium(), this);
    if (dialog.exec() == QDialog::Accepted) {
        dialog.saveSettingValues();
        mSettings->sync();
        mUI->mResults->updateSettings(dialog.showFullPath(),
                                      dialog.saveFullPath(),
                                      dialog.saveAllErrors(),
                                      dialog.showNoErrorsMessage(),
                                      dialog.showErrorId(),
                                      dialog.showInconclusive());
        mUI->mResults->updateStyleSetting(mSettings);
        const QString newLang = mSettings->value(SETTINGS_LANGUAGE, "en").toString();
        setLanguage(newLang);
    }
}

void MainWindow::reAnalyzeModified()
{
    reAnalyze(false);
}

void MainWindow::reAnalyzeAll()
{
    if (mProjectFile)
        analyzeProject(mProjectFile, QStringList());
    else
        reAnalyze(true);
}

void MainWindow::checkLibrary()
{
    if (mProjectFile)
        analyzeProject(mProjectFile, QStringList(), true);
}

void MainWindow::checkConfiguration()
{
    if (mProjectFile)
        analyzeProject(mProjectFile, QStringList(), false, true);
}

void MainWindow::reAnalyzeSelected(const QStringList& files)
{
    if (files.empty())
        return;
    if (mThread->isChecking())
        return;

    if (mProjectFile) {
        // Clear details, statistics and progress
        mUI->mResults->clear(false);
        for (int i = 0; i < files.size(); ++i)
            mUI->mResults->clearRecheckFile(files[i]);

        analyzeProject(mProjectFile, files);
        return;
    }

    Settings checkSettings;
    auto supprs = std::make_shared<Suppressions>();
    if (!getCppcheckSettings(checkSettings, *supprs))
        return;

    // Clear details, statistics and progress
    mUI->mResults->clear(false);
    for (int i = 0; i < files.size(); ++i)
        mUI->mResults->clearRecheckFile(files[i]);

    mCurrentDirectory = mUI->mResults->getCheckDirectory();
    FileList pathList;
    pathList.addPathList(files);
    if (mProjectFile)
        pathList.addExcludeList(mProjectFile->getExcludedPaths());

    std::list<FileWithDetails> fdetails = enrichFilesForAnalysis(pathList.getFileList(), checkSettings);

    checkLockDownUI(); // lock UI while checking
    mUI->mResults->checkingStarted(fdetails.size());
    mThread->setCheckFiles(std::move(fdetails));

    // Saving last check start time, otherwise unchecked modified files will not be
    // considered in "Modified Files Check"  performed after "Selected Files Check"
    // TODO: Should we store per file CheckStartTime?
    QDateTime saveCheckStartTime = mThread->getCheckStartTime();
    mThread->check(checkSettings, supprs);
    mUI->mResults->setCheckSettings(checkSettings);
    mThread->setCheckStartTime(std::move(saveCheckStartTime));
}

void MainWindow::reAnalyze(bool all)
{
    const std::list<FileWithDetails> files = mThread->getReCheckFiles(all);
    if (files.empty())
        return;

    Settings checkSettings;
    auto supprs = std::make_shared<Suppressions>();
    if (!getCppcheckSettings(checkSettings, *supprs))
        return;

    // Clear details, statistics and progress
    mUI->mResults->clear(all);

    // Clear results for changed files
    for (const auto& f : files)
        mUI->mResults->clear(QString::fromStdString(f.path()));

    checkLockDownUI(); // lock UI while checking
    mUI->mResults->checkingStarted(files.size());

    if (mProjectFile)
        qDebug() << "Rechecking project file" << mProjectFile->getFilename();

    mThread->setCheckFiles(all);
    mThread->check(checkSettings, supprs);
    mUI->mResults->setCheckSettings(checkSettings);
}

void MainWindow::clearResults()
{
    if (mProjectFile && !mProjectFile->getBuildDir().isEmpty()) {
        QDir dir(QFileInfo(mProjectFile->getFilename()).absolutePath() + '/' + mProjectFile->getBuildDir());
        for (const QString& f: dir.entryList(QDir::Files)) {
            if (!f.endsWith("files.txt")) {
                static const QRegularExpression rx("^.*.s[0-9]+$");
                if (!rx.match(f).hasMatch())
                    dir.remove(f);
            }
        }
    }
    mUI->mResults->clear(true);
    Q_ASSERT(false == mUI->mResults->hasResults());
    enableResultsButtons();
}

void MainWindow::openResults()
{
    if (mUI->mResults->hasResults()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Cppcheck"));
        const QString msg(tr("Current results will be cleared.\n\n"
                             "Opening a new XML file will clear current results.\n"
                             "Do you want to proceed?"));
        msgBox.setText(msg);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.addButton(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        const int dlgResult = msgBox.exec();
        if (dlgResult == QMessageBox::No) {
            return;
        }
    }

    QString selectedFilter;
    const QString filter(tr("XML files (*.xml)"));
    QString selectedFile = QFileDialog::getOpenFileName(this,
                                                        tr("Open the report file"),
                                                        getPath(SETTINGS_LAST_RESULT_PATH),
                                                        filter,
                                                        &selectedFilter);

    if (!selectedFile.isEmpty()) {
        loadResults(selectedFile);
    }
}

void MainWindow::loadResults(const QString &selectedFile)
{
    if (selectedFile.isEmpty())
        return;
    if (mProjectFile)
        closeProjectFile();
    mIsLogfileLoaded = true;
    mUI->mResults->clear(true);
    mUI->mResults->setResultsSource(ResultsTree::ResultsSource::Log);
    mUI->mActionReanalyzeModified->setEnabled(false);
    mUI->mActionReanalyzeAll->setEnabled(false);
    mUI->mResults->readErrorsXml(selectedFile);
    setPath(SETTINGS_LAST_RESULT_PATH, selectedFile);
    formatAndSetTitle(selectedFile);
}

void MainWindow::loadResults(const QString &selectedFile, const QString &sourceDirectory)
{
    loadResults(selectedFile);
    mUI->mResults->setCheckDirectory(sourceDirectory);
}

void MainWindow::enableCheckButtons(bool enable)
{
    mUI->mActionStop->setEnabled(!enable);
    mUI->mActionAnalyzeFiles->setEnabled(enable);

    if (mProjectFile) {
        mUI->mActionReanalyzeModified->setEnabled(enable);
        mUI->mActionReanalyzeAll->setEnabled(enable);
    } else if (!enable || mThread->hasPreviousFiles()) {
        mUI->mActionReanalyzeModified->setEnabled(enable);
        mUI->mActionReanalyzeAll->setEnabled(enable);
    }

    mUI->mActionAnalyzeDirectory->setEnabled(enable);

    if (isCppcheckPremium()) {
        mUI->mActionComplianceReport->setEnabled(enable && mProjectFile && (mProjectFile->getAddons().contains("misra") || !mProjectFile->getCodingStandards().empty()));
    }
}

void MainWindow::enableResultsButtons()
{
    mUI->mActionClearResults->setEnabled(mUI->mResults->hasResults());
    mUI->mActionSave->setEnabled(true);
    mUI->mActionPrint->setEnabled(true);
    mUI->mActionPrintPreview->setEnabled(true);
}

void MainWindow::showStyle(bool checked)
{
    mUI->mResults->showResults(ShowTypes::ShowStyle, checked);
}

void MainWindow::showErrors(bool checked)
{
    mUI->mResults->showResults(ShowTypes::ShowErrors, checked);
}

void MainWindow::showWarnings(bool checked)
{
    mUI->mResults->showResults(ShowTypes::ShowWarnings, checked);
}

void MainWindow::showPortability(bool checked)
{
    mUI->mResults->showResults(ShowTypes::ShowPortability, checked);
}

void MainWindow::showPerformance(bool checked)
{
    mUI->mResults->showResults(ShowTypes::ShowPerformance, checked);
}

void MainWindow::showInformation(bool checked)
{
    mUI->mResults->showResults(ShowTypes::ShowInformation, checked);
}

void MainWindow::checkAll()
{
    toggleAllChecked(true);
}

void MainWindow::uncheckAll()
{
    toggleAllChecked(false);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Check that we aren't checking files
    if (!mThread->isChecking()) {
        saveSettings();
        event->accept();
    } else {
        const QString text(tr("Analyzer is running.\n\n" \
                              "Do you want to stop the analysis and exit Cppcheck?"));

        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        text,
                        QMessageBox::Yes | QMessageBox::No,
                        this);

        msg.setDefaultButton(QMessageBox::No);
        const int rv = msg.exec();
        if (rv == QMessageBox::Yes) {
            // This isn't really very clean way to close threads but since the app is
            // exiting it doesn't matter.
            mThread->stop();
            saveSettings();
            mExiting = true;
        }
        event->ignore();
    }
}

void MainWindow::toggleAllChecked(bool checked)
{
    mUI->mActionShowStyle->setChecked(checked);
    showStyle(checked);
    mUI->mActionShowErrors->setChecked(checked);
    showErrors(checked);
    mUI->mActionShowWarnings->setChecked(checked);
    showWarnings(checked);
    mUI->mActionShowPortability->setChecked(checked);
    showPortability(checked);
    mUI->mActionShowPerformance->setChecked(checked);
    showPerformance(checked);
    mUI->mActionShowInformation->setChecked(checked);
    showInformation(checked);
}

void MainWindow::about()
{
    if (!mCppcheckCfgAbout.isEmpty()) {
        QMessageBox msg(QMessageBox::Information,
                        tr("About"),
                        mCppcheckCfgAbout,
                        QMessageBox::Ok,
                        this);
        msg.exec();
    }
    else {
        auto *dlg = new AboutDialog(CppCheck::version(), CppCheck::extraVersion(), this);
        dlg->exec();
    }
}

void MainWindow::showLicense()
{
    auto *dlg = new FileViewDialog(":COPYING", tr("License"), this);
    dlg->resize(570, 400);
    dlg->exec();
}

void MainWindow::showAuthors()
{
    auto *dlg = new FileViewDialog(":AUTHORS", tr("Authors"), this);
    dlg->resize(350, 400);
    dlg->exec();
}

void MainWindow::showEULA()
{
    QDesktopServices::openUrl(QUrl("https://www.cppcheck.com/EULA"));
}

void MainWindow::performSelectedFilesCheck(const QStringList &selectedFilesList)
{
    reAnalyzeSelected(selectedFilesList);
}

void MainWindow::save()
{
    QString selectedFilter;
    const QString filter(tr("XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)"));
    QString selectedFile = QFileDialog::getSaveFileName(this,
                                                        tr("Save the report file"),
                                                        getPath(SETTINGS_LAST_RESULT_PATH),
                                                        filter,
                                                        &selectedFilter);

    if (!selectedFile.isEmpty()) {
        Report::Type type = Report::TXT;
        if (selectedFilter == tr("XML files (*.xml)")) {
            type = Report::XMLV2;
            if (!selectedFile.endsWith(".xml", Qt::CaseInsensitive))
                selectedFile += ".xml";
        } else if (selectedFilter == tr("Text files (*.txt)")) {
            type = Report::TXT;
            if (!selectedFile.endsWith(".txt", Qt::CaseInsensitive))
                selectedFile += ".txt";
        } else if (selectedFilter == tr("CSV files (*.csv)")) {
            type = Report::CSV;
            if (!selectedFile.endsWith(".csv", Qt::CaseInsensitive))
                selectedFile += ".csv";
        } else {
            if (selectedFile.endsWith(".xml", Qt::CaseInsensitive))
                type = Report::XMLV2;
            else if (selectedFile.endsWith(".txt", Qt::CaseInsensitive))
                type = Report::TXT;
            else if (selectedFile.endsWith(".csv", Qt::CaseInsensitive))
                type = Report::CSV;
        }

        mUI->mResults->save(selectedFile, type, mCppcheckCfgProductName);
        setPath(SETTINGS_LAST_RESULT_PATH, selectedFile);
    }
}

void MainWindow::complianceReport()
{
    if (!mUI->mResults->isSuccess()) {
        QMessageBox m(QMessageBox::Critical,
                      "Cppcheck",
                      tr("Cannot generate a compliance report right now, an analysis must finish successfully. Try to reanalyze the code and ensure there are no critical errors."),
                      QMessageBox::Ok,
                      this);
        m.exec();
        return;
    }

    QTemporaryFile tempResults;
    (void)tempResults.open(); // TODO: check result
    tempResults.close();

    mUI->mResults->save(tempResults.fileName(), Report::XMLV2, mCppcheckCfgProductName);

    ComplianceReportDialog dlg(mProjectFile, tempResults.fileName(), mUI->mResults->getStatistics()->getCheckersReport());
    dlg.exec();
}

void MainWindow::resultsAdded()
{
    enableResultsButtons();
}

void MainWindow::toggleMainToolBar()
{
    mUI->mToolBarMain->setVisible(mUI->mActionToolBarMain->isChecked());
}

void MainWindow::toggleViewToolBar()
{
    mUI->mToolBarView->setVisible(mUI->mActionToolBarView->isChecked());
}

void MainWindow::toggleFilterToolBar()
{
    mUI->mToolBarFilter->setVisible(mUI->mActionToolBarFilter->isChecked());
    mLineEditFilter->clear(); // Clearing the filter also disables filtering
}

void MainWindow::formatAndSetTitle(const QString &text)
{
    QString nameWithVersion = QString("Cppcheck %1").arg(CppCheck::version());

    QString extraVersion = CppCheck::extraVersion();
    if (!extraVersion.isEmpty()) {
        nameWithVersion += " (" + extraVersion + ")";
    }

    // TODO: should not contain the version
    if (!mCppcheckCfgProductName.isEmpty())
        nameWithVersion = mCppcheckCfgProductName;

    QString title;
    if (text.isEmpty())
        title = nameWithVersion;
    else
        title = QString("%1 - %2").arg(nameWithVersion, text);
    setWindowTitle(title);
}

void MainWindow::setLanguage(const QString &code)
{
    const QString currentLang = mTranslation->getCurrentLanguage();
    if (currentLang == code)
        return;

    if (mTranslation->setLanguage(code)) {
        //Translate everything that is visible here
        mUI->retranslateUi(this);
        mUI->mResults->translate();
        mLineEditFilter->setPlaceholderText(QCoreApplication::translate("MainWindow", "Quick Filter:"));
        if (mProjectFile)
            formatAndSetTitle(tr("Project:") + ' ' + mProjectFile->getFilename());
        if (mScratchPad)
            mScratchPad->translate();
    }
}

void MainWindow::aboutToShowViewMenu()
{
    mUI->mActionToolBarMain->setChecked(mUI->mToolBarMain->isVisible());
    mUI->mActionToolBarView->setChecked(mUI->mToolBarView->isVisible());
    mUI->mActionToolBarFilter->setChecked(mUI->mToolBarFilter->isVisible());
}

void MainWindow::stopAnalysis()
{
    mThread->stop();
    mUI->mResults->stopAnalysis();
    const QString &lastResults = getLastResults();
    if (!lastResults.isEmpty()) {
        mUI->mResults->updateFromOldReport(lastResults);
    }
}

void MainWindow::openHelpContents()
{
    openOnlineHelp();
}

void MainWindow::openOnlineHelp()
{
    auto *helpDialog = new HelpDialog;
    helpDialog->showMaximized();
}

void MainWindow::openProjectFile()
{
    const QString filter = tr("Project files (*.cppcheck);;All files(*.*)");
    const QString filepath = QFileDialog::getOpenFileName(this,
                                                          tr("Select Project File"),
                                                          getPath(SETTINGS_LAST_PROJECT_PATH),
                                                          filter);

    if (!filepath.isEmpty()) {
        const QFileInfo fi(filepath);
        if (fi.exists() && fi.isFile() && fi.isReadable()) {
            setPath(SETTINGS_LAST_PROJECT_PATH, filepath);
            loadProjectFile(filepath);
        }
    }
}

void MainWindow::showScratchpad()
{
    if (!mScratchPad)
        mScratchPad = new ScratchPad(*this);

    mScratchPad->show();

    if (!mScratchPad->isActiveWindow())
        mScratchPad->activateWindow();
}

void MainWindow::loadProjectFile(const QString &filePath)
{
    QFileInfo inf(filePath);
    const QString filename = inf.fileName();
    formatAndSetTitle(tr("Project:") + ' ' + filename);
    addProjectMRU(filePath);

    mIsLogfileLoaded = false;
    mUI->mResults->setResultsSource(ResultsTree::ResultsSource::Analysis);
    mUI->mActionReanalyzeModified->setEnabled(true);
    mUI->mActionReanalyzeAll->setEnabled(true);
    enableProjectActions(true);
    delete mProjectFile;
    mProjectFile = new ProjectFile(filePath, this);
    mProjectFile->setActiveProject();
    if (!loadLastResults())
        analyzeProject(mProjectFile, QStringList());
}

QString MainWindow::getLastResults() const
{
    if (!mProjectFile || mProjectFile->getBuildDir().isEmpty())
        return QString();
    return QFileInfo(mProjectFile->getFilename()).absolutePath() + '/' + mProjectFile->getBuildDir() + "/lastResults.xml";
}

bool MainWindow::loadLastResults()
{
    const QString &lastResults = getLastResults();
    if (lastResults.isEmpty())
        return false;
    if (!QFileInfo::exists(lastResults))
        return false;
    mUI->mResults->clear(true);
    mUI->mResults->readErrorsXml(lastResults);
    mUI->mResults->setCheckDirectory(mSettings->value(SETTINGS_LAST_CHECK_PATH,QString()).toString());
    mUI->mActionViewStats->setEnabled(true);
    enableResultsButtons();
    return true;
}

void MainWindow::analyzeProject(const ProjectFile *projectFile, const QStringList& recheckFiles, const bool checkLibrary, const bool checkConfiguration)
{
    Settings::terminate(false);

    QFileInfo inf(projectFile->getFilename());
    const QString& rootpath = projectFile->getRootPath();

    if (isCppcheckPremium() && !projectFile->getLicenseFile().isEmpty()) {
        if (rootpath.isEmpty() || rootpath == ".")
            QDir::setCurrent(inf.absolutePath());
        else if (QDir(rootpath).isAbsolute())
            QDir::setCurrent(rootpath);
        else
            QDir::setCurrent(inf.absolutePath() + "/" + rootpath);

        QString licenseFile = projectFile->getLicenseFile();
        if (!QFileInfo(licenseFile).isAbsolute() && !rootpath.isEmpty())
            licenseFile = inf.absolutePath() + "/" + licenseFile;

#ifdef Q_OS_WIN
        const QString premiumaddon = QCoreApplication::applicationDirPath() + "/premiumaddon.exe";
#else
        const QString premiumaddon = QCoreApplication::applicationDirPath() + "/premiumaddon";
#endif
        const std::vector<std::string> args{"--check-loc-license", licenseFile.toStdString()};
        std::string output;
        CheckThread::executeCommand(premiumaddon.toStdString(), args, "", output);
        std::ofstream fout(inf.absolutePath().toStdString() + "/cppcheck-premium-loc");
        fout << output;
    }

    QDir::setCurrent(inf.absolutePath());

    mThread->setAddonsAndTools(projectFile->getAddonsAndTools());

    // If the root path is not given or is not "current dir", use project
    // file's location directory as root path
    if (rootpath.isEmpty() || rootpath == ".")
        mCurrentDirectory = inf.canonicalPath();
    else if (rootpath.startsWith("./"))
        mCurrentDirectory = inf.canonicalPath() + rootpath.mid(1);
    else
        mCurrentDirectory = rootpath;

    if (!projectFile->getBuildDir().isEmpty()) {
        QString buildDir = projectFile->getBuildDir();
        if (!QDir::isAbsolutePath(buildDir))
            buildDir = inf.canonicalPath() + '/' + buildDir;
        if (!QDir(buildDir).exists()) {
            QMessageBox msg(QMessageBox::Question,
                            tr("Cppcheck"),
                            tr("Build dir '%1' does not exist, create it?").arg(buildDir),
                            QMessageBox::Yes | QMessageBox::No,
                            this);
            if (msg.exec() == QMessageBox::Yes) {
                QDir().mkpath(buildDir);
            } else if (!projectFile->getAddons().isEmpty()) {
                QMessageBox m(QMessageBox::Critical,
                              tr("Cppcheck"),
                              tr("To check the project using addons, you need a build directory."),
                              QMessageBox::Ok,
                              this);
                m.exec();
                return;
            }
        }
    }

    if (!projectFile->getImportProject().isEmpty()) {
        ImportProject p;
        QString prjfile;

        if (QFileInfo(projectFile->getImportProject()).isAbsolute()) {
            prjfile = projectFile->getImportProject();
        } else {
            prjfile = inf.canonicalPath() + '/' + projectFile->getImportProject();
        }
        try {

            const ImportProject::Type result = p.import(prjfile.toStdString());

            QString errorMessage;
            switch (result) {
            case ImportProject::Type::COMPILE_DB:
            case ImportProject::Type::VS_SLN:
            case ImportProject::Type::VS_SLNX:
            case ImportProject::Type::VS_VCXPROJ:
            case ImportProject::Type::BORLAND:
            case ImportProject::Type::CPPCHECK_GUI:
                // Loading was successful
                break;
            case ImportProject::Type::MISSING:
                errorMessage = tr("Failed to open file");
                break;
            case ImportProject::Type::UNKNOWN:
                errorMessage = tr("Unknown project file format");
                break;
            case ImportProject::Type::FAILURE:
                errorMessage = tr("Failed to import project file");
                break;
            case ImportProject::Type::NONE:
                // can never happen
                break;
            }

            if (!p.errors.empty())
                errorMessage += ": \n";

            for (const auto &error : p.errors)
                errorMessage += "\n - " + QString::fromStdString(error);

            if (!errorMessage.isEmpty()) {
                QMessageBox msg(QMessageBox::Critical,
                                tr("Cppcheck"),
                                tr("Failed to import '%1': %2\n\nAnalysis is stopped.").arg(prjfile).arg(errorMessage),
                                QMessageBox::Ok,
                                this);
                msg.exec();
                return;
            }
        } catch (InternalError &e) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Cppcheck"),
                            tr("Failed to import '%1' (%2), analysis is stopped").arg(prjfile).arg(QString::fromStdString(e.errorMessage)),
                            QMessageBox::Ok,
                            this);
            msg.exec();
            return;
        }
        doAnalyzeProject(p, checkLibrary, checkConfiguration);  // TODO: avoid copy
        return;
    }

    QStringList paths = recheckFiles.isEmpty() ? projectFile->getCheckPaths() : recheckFiles;

    // If paths not given then check the root path (which may be the project
    // file's location, see above). This is to keep the compatibility with
    // old "silent" project file loading when we checked the director where the
    // project file was located.
    if (paths.isEmpty()) {
        paths << mCurrentDirectory;
    }
    doAnalyzeFiles(paths, checkLibrary, checkConfiguration);
}

void MainWindow::newProjectFile()
{
    const QString filter = tr("Project files (*.cppcheck)");
    QString filepath = QFileDialog::getSaveFileName(this,
                                                    tr("Select Project Filename"),
                                                    getPath(SETTINGS_LAST_PROJECT_PATH),
                                                    filter);

    if (filepath.isEmpty())
        return;
    if (!filepath.endsWith(".cppcheck", Qt::CaseInsensitive))
        filepath += ".cppcheck";

    setPath(SETTINGS_LAST_PROJECT_PATH, filepath);

    QFileInfo inf(filepath);
    const QString filename = inf.fileName();
    formatAndSetTitle(tr("Project:") + QString(" ") + filename);

    delete mProjectFile;
    mProjectFile = new ProjectFile(this);
    mProjectFile->setActiveProject();
    mProjectFile->setFilename(filepath);
    mProjectFile->setProjectName(filename.left(filename.indexOf(".")));
    mProjectFile->setBuildDir(filename.left(filename.indexOf(".")) + "-cppcheck-build-dir");

    ProjectFileDialog dlg(mProjectFile, isCppcheckPremium(), this);
    if (dlg.exec() == QDialog::Accepted) {
        addProjectMRU(filepath);
        analyzeProject(mProjectFile, QStringList());
    } else {
        closeProjectFile();
    }
}

void MainWindow::closeProjectFile()
{
    delete mProjectFile;
    mProjectFile = nullptr;
    mUI->mResults->clear(true);
    enableProjectActions(false);
    enableProjectOpenActions(true);
    formatAndSetTitle();
}

void MainWindow::editProjectFile()
{
    if (!mProjectFile) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("No project file loaded"),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }

    ProjectFileDialog dlg(mProjectFile, isCppcheckPremium(), this);
    if (dlg.exec() == QDialog::Accepted) {
        mProjectFile->write();
        analyzeProject(mProjectFile, QStringList());
    }
}

void MainWindow::showStatistics()
{
    StatsDialog statsDialog(this);

    // Show a dialog with the previous scan statistics and project information
    statsDialog.setProject(mProjectFile);
    statsDialog.setPathSelected(mCurrentDirectory);
    statsDialog.setNumberOfFilesScanned(mThread->getPreviousFilesCount());
    statsDialog.setScanDuration(mThread->getPreviousScanDuration() / 1000.0);
    statsDialog.setStatistics(mUI->mResults->getStatistics());

    statsDialog.exec();
}

void MainWindow::showLibraryEditor()
{
    LibraryDialog libraryDialog(this);
    libraryDialog.exec();
}

void MainWindow::showThreadDetails()
{
    if (ThreadDetails::instance())
        return;
    auto* threadDetails = new ThreadDetails(this);
    connect(mThread, &ThreadHandler::threadDetailsUpdated,
            threadDetails, &ThreadDetails::threadDetailsUpdated, Qt::QueuedConnection);
    connect(mThread, &ThreadHandler::progress,
            threadDetails, &ThreadDetails::progress, Qt::QueuedConnection);
    threadDetails->setAttribute(Qt::WA_DeleteOnClose);
    threadDetails->show();
    mThread->emitThreadDetailsUpdated();
}

void MainWindow::filterResults()
{
    mUI->mResults->filterResults(mLineEditFilter->text());
}

void MainWindow::enableProjectActions(bool enable)
{
    mUI->mActionCloseProjectFile->setEnabled(enable);
    mUI->mActionEditProjectFile->setEnabled(enable);
    mUI->mActionCheckLibrary->setEnabled(enable);
    mUI->mActionCheckConfiguration->setEnabled(enable);
}

void MainWindow::enableProjectOpenActions(bool enable)
{
    mUI->mActionNewProjectFile->setEnabled(enable);
    mUI->mActionOpenProjectFile->setEnabled(enable);
}

void MainWindow::openRecentProject()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    const QString project = action->data().toString();
    QFileInfo inf(project);
    if (inf.exists()) {
        if (inf.suffix() == "xml")
            loadResults(project);
        else {
            loadProjectFile(project);
            loadLastResults();
        }
    } else {
        const QString text(tr("The project file\n\n%1\n\n could not be found!\n\n"
                              "Do you want to remove the file from the recently "
                              "used projects -list?").arg(project));

        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        text,
                        QMessageBox::Yes | QMessageBox::No,
                        this);

        msg.setDefaultButton(QMessageBox::No);
        const int rv = msg.exec();
        if (rv == QMessageBox::Yes) {
            removeProjectMRU(project);
        }
    }
}

void MainWindow::updateMRUMenuItems()
{
    for (QAction* recentProjectAct : mRecentProjectActs) {
        if (recentProjectAct != nullptr)
            mUI->mMenuFile->removeAction(recentProjectAct);
    }

    QStringList projects = mSettings->value(SETTINGS_MRU_PROJECTS).toStringList();

    // Do a sanity check - remove duplicates and non-existing projects
    int removed = projects.removeDuplicates();
    for (int i = projects.size() - 1; i >= 0; i--) {
        if (!QFileInfo::exists(projects[i])) {
            projects.removeAt(i);
            removed++;
        }
    }

    if (removed)
        mSettings->setValue(SETTINGS_MRU_PROJECTS, projects);

    const int numRecentProjects = qMin(projects.size(), static_cast<int>(MaxRecentProjects));
    for (int i = 0; i < numRecentProjects; i++) {
        const QString filename = QFileInfo(projects[i]).fileName();
        const QString text = QString("&%1 %2").arg(i + 1).arg(filename);
        mRecentProjectActs[i]->setText(text);
        mRecentProjectActs[i]->setData(projects[i]);
        mRecentProjectActs[i]->setVisible(true);
        mUI->mMenuFile->insertAction(mUI->mActionProjectMRU, mRecentProjectActs[i]);
    }

    if (numRecentProjects > 1)
        mRecentProjectActs[numRecentProjects] = mUI->mMenuFile->insertSeparator(mUI->mActionProjectMRU);
}

void MainWindow::addProjectMRU(const QString &project)
{
    QStringList files = mSettings->value(SETTINGS_MRU_PROJECTS).toStringList();
    files.removeAll(project);
    files.prepend(project);
    while (files.size() > MaxRecentProjects)
        files.removeLast();

    mSettings->setValue(SETTINGS_MRU_PROJECTS, files);
    updateMRUMenuItems();
}

void MainWindow::removeProjectMRU(const QString &project)
{
    QStringList files = mSettings->value(SETTINGS_MRU_PROJECTS).toStringList();
    files.removeAll(project);

    mSettings->setValue(SETTINGS_MRU_PROJECTS, files);
    updateMRUMenuItems();
}

void MainWindow::selectPlatform()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (action) {
        const Platform::Type platform = static_cast<Platform::Type>(action->data().toInt());
        mSettings->setValue(SETTINGS_CHECKED_PLATFORM, platform);
    }
}

void MainWindow::suppressIds(QStringList ids)
{
    if (!mProjectFile)
        return;
    ids.removeDuplicates();

    QList<SuppressionList::Suppression> suppressions = mProjectFile->getSuppressions();
    for (const QString& id : ids) {
        // Remove all matching suppressions
        std::string id2 = id.toStdString();
        for (int i = 0; i < suppressions.size();) {
            if (suppressions[i].errorId == id2)
                suppressions.removeAt(i);
            else
                ++i;
        }

        SuppressionList::Suppression newSuppression;
        newSuppression.errorId = id2;
        suppressions << newSuppression;
    }

    mProjectFile->setSuppressions(suppressions);
    mProjectFile->write();
}

static int getVersion(const QString& nameWithVersion) {
    int ret = 0;
    int v = 0;
    int dot = 0;
    for (const auto c: nameWithVersion) {
        if (c == '\n' || c == '\r')
            break;
        if (c == ' ') {
            if (ret > 0 && dot == 1 && nameWithVersion.endsWith(" dev"))
                return (ret * 1000000) + (v * 1000) + 500;
            dot = ret = v = 0;
        }
        else if (c == '.') {
            ++dot;
            ret = (ret * 1000) + v;
            v = 0;
        } else if (c >= '0' && c <= '9')
            v = (v * 10) + (c.toLatin1() - '0');
    }
    ret = (ret * 1000) + v;
    while (dot < 2) {
        ++dot;
        ret *= 1000;
    }
    return ret;
}

void MainWindow::replyFinished(QNetworkReply *reply) {
    reply->deleteLater();
    if (reply->error()) {
        mUI->mLayoutInformation->deleteLater();
        qDebug() << "Response: ERROR";
        return;
    }
    const QString str = reply->readAll();
    qDebug() << "Response: " << str;
    if (reply->url().fileName() == "version.txt") {
        // TODO: lacks extra version
        QString nameWithVersion = QString("Cppcheck %1").arg(CppCheck::version());
        // TODO: this should not contain the version
        if (!mCppcheckCfgProductName.isEmpty())
            nameWithVersion = mCppcheckCfgProductName;
        const int appVersion = getVersion(nameWithVersion);
        const int latestVersion = getVersion(str.trimmed());
        if (appVersion < latestVersion) {
            if (mSettings->value(SETTINGS_CHECK_VERSION, 0).toInt() != latestVersion) {
                QString install;
                if (isCppcheckPremium()) {
#ifdef Q_OS_WIN
                    const QString url("https://cppchecksolutions.com/cppcheck-premium-installation");
#else
                    const QString url("https://cppchecksolutions.com/cppcheck-premium-linux-installation");
#endif
                    install = "<a href=\"" + url + "\">" + tr("Install") + "</a>";
                }
                mUI->mButtonHideInformation->setVisible(true);
                mUI->mLabelInformation->setVisible(true);
                mUI->mLabelInformation->setText(tr("New version available: %1. %2").arg(str.trimmed()).arg(install));
            }
        }
    }
    if (!mUI->mLabelInformation->isVisible()) {
        mUI->mLayoutInformation->deleteLater();
    }
}

void MainWindow::hideInformation() {
    int version = getVersion(mUI->mLabelInformation->text());
    mSettings->setValue(SETTINGS_CHECK_VERSION, version);
    mUI->mLabelInformation->setVisible(false);
    mUI->mButtonHideInformation->setVisible(false);
    mUI->mLayoutInformation->deleteLater();
}

bool MainWindow::isCppcheckPremium() const {
    return mCppcheckCfgProductName.startsWith("Cppcheck Premium ");
}

void MainWindow::changeReportType() {
    const ReportType reportType = mUI->mActionReportAutosar->isChecked() ? ReportType::autosar :
                                  mUI->mActionReportCertC->isChecked() ? ReportType::certC :
                                  mUI->mActionReportCertCpp->isChecked() ? ReportType::certCpp :
                                  mUI->mActionReportMisraC->isChecked() ? (mProjectFile ? getMisraCReportType(mProjectFile->getCodingStandards()) : ReportType::misraC2012) :
                                  mUI->mActionReportMisraCpp2008->isChecked() ? ReportType::misraCpp2008 :
                                  mUI->mActionReportMisraCpp2023->isChecked() ? ReportType::misraCpp2023 :
                                  ReportType::normal;

    mUI->mResults->setReportType(reportType);

    auto setTextAndHint = [](QAction* a, const QString& s) {
        a->setVisible(!s.isEmpty());
        a->setText(s);
        a->setToolTip(s);
    };

    const QString showMandatory = tr("Show Mandatory");
    const QString showRequired = tr("Show Required");
    const QString showAdvisory = tr("Show Advisory");
    const QString showDocument = tr("Show Document");

    if (mUI->mActionReportAutosar->isChecked()) {
        setTextAndHint(mUI->mActionShowErrors, "");
        setTextAndHint(mUI->mActionShowWarnings, showRequired);
        setTextAndHint(mUI->mActionShowStyle, showAdvisory);
        setTextAndHint(mUI->mActionShowPortability, "");
        setTextAndHint(mUI->mActionShowPerformance, "");
        setTextAndHint(mUI->mActionShowInformation, "");
    } else if (mUI->mActionReportMisraC->isChecked() || mUI->mActionReportMisraCpp2008->isChecked() || mUI->mActionReportMisraCpp2023->isChecked()) {
        setTextAndHint(mUI->mActionShowErrors, mUI->mActionReportMisraCpp2008->isChecked() ? "" : showMandatory);
        setTextAndHint(mUI->mActionShowWarnings, showRequired);
        setTextAndHint(mUI->mActionShowStyle, showAdvisory);
        setTextAndHint(mUI->mActionShowPortability, "");
        setTextAndHint(mUI->mActionShowPerformance, "");
        setTextAndHint(mUI->mActionShowInformation, mUI->mActionReportMisraCpp2008->isChecked() ? showDocument : QString());
    } else if (mUI->mActionReportCertC->isChecked() || mUI->mActionReportCertCpp->isChecked()) {
        setTextAndHint(mUI->mActionShowErrors, tr("Show L1"));
        setTextAndHint(mUI->mActionShowWarnings, tr("Show L2"));
        setTextAndHint(mUI->mActionShowStyle, tr("Show L3"));
        setTextAndHint(mUI->mActionShowPortability, "");
        setTextAndHint(mUI->mActionShowPerformance, "");
        setTextAndHint(mUI->mActionShowInformation, "");
    } else {
        setTextAndHint(mUI->mActionShowErrors, tr("Show errors"));
        setTextAndHint(mUI->mActionShowWarnings, tr("Show warnings"));
        setTextAndHint(mUI->mActionShowStyle, tr("Show style"));
        setTextAndHint(mUI->mActionShowPortability, tr("Show portability"));
        setTextAndHint(mUI->mActionShowPerformance, tr("Show performance"));
        setTextAndHint(mUI->mActionShowInformation, tr("Show information"));
    }
}

std::list<FileWithDetails> MainWindow::enrichFilesForAnalysis(const QStringList& fileNames, const Settings& settings) const {
    std::list<FileWithDetails> fdetails;
    std::transform(fileNames.cbegin(), fileNames.cend(), std::back_inserter(fdetails), [](const QString& f) {
        return FileWithDetails{f.toStdString(), Standards::Language::None, static_cast<std::size_t>(QFile(f).size())};
    });
    const Standards::Language enforcedLang = static_cast<Standards::Language>(mSettings->value(SETTINGS_ENFORCED_LANGUAGE, 0).toInt());
    frontend::applyLang(fdetails, settings, enforcedLang);
    return fdetails;
}
