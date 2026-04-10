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

#include "projectfiledialog.h"

#include "checkthread.h"
#include "common.h"
#include "importproject.h"
#include "library.h"
#include "newsuppressiondialog.h"
#include "path.h"
#include "platform.h"
#include "platforms.h"
#include "projectfile.h"
#include "settings.h"
#include "utils.h"

#include "ui_projectfile.h"

#include <array>
#include <list>
#include <string>
#include <utility>

#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSettings>
#include <QSize>
#include <QSpinBox>
#include <QVariant>

const char ADDON_MISRA[]   = "misra";
const char CODING_STANDARD_MISRA_C_2023[] = "misra-c-2023";
const char CODING_STANDARD_MISRA_C_2025[] = "misra-c-2025";
const char CODING_STANDARD_MISRA_CPP_2008[] = "misra-cpp-2008";
const char CODING_STANDARD_MISRA_CPP_2023[] = "misra-cpp-2023";
const char CODING_STANDARD_CERT_C[] = "cert-c-2016";
const char CODING_STANDARD_CERT_CPP[] = "cert-cpp-2016";
const char CODING_STANDARD_AUTOSAR[] = "autosar";
const char CODING_STANDARD_SAFETY_PROFILES[] = "safety-profiles";

/** Return paths from QListWidget */
static QStringList getPaths(const QListWidget *list)
{
    const int count = list->count();
    QStringList paths;
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = list->item(i);
        paths << QDir::fromNativeSeparators(item->text());
    }
    return paths;
}

/** Platforms shown in the platform combobox */
static const std::array<Platform::Type, 6> builtinPlatforms = {
    Platform::Type::Native,
    Platform::Type::Win32A,
    Platform::Type::Win32W,
    Platform::Type::Win64,
    Platform::Type::Unix32,
    Platform::Type::Unix64
};

static std::string suppressionAsText(const SuppressionList::Suppression& s)
{
    std::string ret;
    if (!s.errorId.empty())
        ret = s.errorId;
    if (!s.fileName.empty())
        ret += " fileName=" + s.fileName;
    if (s.lineNumber != SuppressionList::Suppression::NO_LINE)
        ret += " lineNumber=" + std::to_string(s.lineNumber);
    if (!s.symbolName.empty())
        ret += " symbolName=" + s.symbolName;
    if (s.hash > 0)
        ret += " hash=" + std::to_string(s.hash);
    if (startsWith(ret," "))
        return ret.substr(1);
    return ret;
}

QStringList ProjectFileDialog::getProjectConfigs(const QString &fileName)
{
    if (!fileName.endsWith(".sln") && !fileName.endsWith(".slnx") && !fileName.endsWith(".vcxproj"))
        return QStringList();
    QStringList ret;
    ImportProject importer;
    Settings projSettings;
    Suppressions projSupprs;
    importer.import(fileName.toStdString(), &projSettings, &projSupprs);
    for (const std::string &cfg : importer.getVSConfigs())
        ret << QString::fromStdString(cfg);
    return ret;
}

ProjectFileDialog::ProjectFileDialog(ProjectFile *projectFile, bool premium, QWidget *parent)
    : QDialog(parent)
    , mUI(new Ui::ProjectFile)
    , mProjectFile(projectFile)
    , mPremium(premium)
{
    mUI->setupUi(this);

    mUI->mToolClangAnalyzer->hide();

    const QFileInfo inf(projectFile->getFilename());
    QString filename = inf.fileName();
    QString title = tr("Project file: %1").arg(filename);
    setWindowTitle(title);
    loadSettings();

    mUI->premiumLicense->setVisible(false);

    QStringList libs;
    // Search the std.cfg first since other libraries could depend on it
    QString stdLibraryFilename;
    for (const QString &sp : projectFile->getSearchPaths("cfg")) {
        QDir dir(sp);
        dir.setSorting(QDir::Name);
        dir.setNameFilters(QStringList("*.cfg"));
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo& item : dir.entryInfoList()) {
            QString library = item.fileName();
            if (library.compare("std.cfg", Qt::CaseInsensitive) != 0)
                continue;
            Library lib;
            const QString fullfilename = sp + "/" + library;
            const Library::Error err = lib.load(nullptr, fullfilename.toLatin1());
            if (err.errorcode != Library::ErrorCode::OK)
                continue;
            // Working std.cfg found
            stdLibraryFilename = fullfilename;
            break;
        }
        if (!stdLibraryFilename.isEmpty())
            break;
    }
    // Search other libraries
    for (const QString &sp : projectFile->getSearchPaths("cfg")) {
        QDir dir(sp);
        dir.setSorting(QDir::Name);
        dir.setNameFilters(QStringList("*.cfg"));
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo& item : dir.entryInfoList()) {
            QString library = item.fileName();
            {
                Library lib;
                const QString fullfilename = sp + "/" + library;
                Library::Error err = lib.load(nullptr, fullfilename.toLatin1());
                if (err.errorcode != Library::ErrorCode::OK) {
                    // Some libraries depend on std.cfg so load it first and test again
                    lib.load(nullptr, stdLibraryFilename.toLatin1());
                    err = lib.load(nullptr, fullfilename.toLatin1());
                }
                if (err.errorcode != Library::ErrorCode::OK)
                    continue;
            }
            library.chop(4);
            if (library.compare("std", Qt::CaseInsensitive) == 0)
                continue;
            if (libs.indexOf(library) == -1)
                libs << library;
        }
    }
    libs.sort();
    mUI->mLibraries->clear();
    for (const QString &lib : utils::as_const(libs)) {
        auto* item = new QListWidgetItem(lib, mUI->mLibraries);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Unchecked); // AND initialize check state
    }

    // Platforms..
    Platforms platforms;
    for (const Platform::Type builtinPlatform : builtinPlatforms)
        mUI->mComboBoxPlatform->addItem(platforms.get(builtinPlatform).mTitle);
    QStringList platformFiles;
    for (const QString& sp : projectFile->getSearchPaths("platforms")) {
        QDir dir(sp);
        dir.setSorting(QDir::Name);
        dir.setNameFilters(QStringList("*.xml"));
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo& item : dir.entryInfoList()) {
            const QString platformFile = item.fileName();
            Platform plat2;
            if (!plat2.loadFromFile({sp.toStdString()}, platformFile.toStdString()))
                continue;

            if (platformFiles.indexOf(platformFile) == -1)
                platformFiles << platformFile;
        }
    }
    platformFiles.sort();
    mUI->mComboBoxPlatform->addItems(platformFiles);

    // integer. allow empty.
    mUI->mEditCertIntPrecision->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*"),this));

    mUI->mEditTags->setValidator(new QRegularExpressionValidator(QRegularExpression("[a-zA-Z0-9 ;]*"),this));

    const QRegularExpression undefRegExp("\\s*([a-zA-Z_][a-zA-Z0-9_]*[; ]*)*");
    mUI->mEditUndefines->setValidator(new QRegularExpressionValidator(undefRegExp, this));

    connect(mUI->mButtons, &QDialogButtonBox::accepted, this, &ProjectFileDialog::ok);
    connect(mUI->mBtnBrowseBuildDir, &QPushButton::clicked, this, &ProjectFileDialog::browseBuildDir);
    connect(mUI->mBtnClearImportProject, &QPushButton::clicked, this, &ProjectFileDialog::clearImportProject);
    connect(mUI->mBtnBrowseImportProject, &QPushButton::clicked, this, &ProjectFileDialog::browseImportProject);
    connect(mUI->mBtnAddCheckPath, SIGNAL(clicked()), this, SLOT(addCheckPath()));
    connect(mUI->mBtnEditCheckPath, &QPushButton::clicked, this, &ProjectFileDialog::editCheckPath);
    connect(mUI->mBtnRemoveCheckPath, &QPushButton::clicked, this, &ProjectFileDialog::removeCheckPath);
    connect(mUI->mBtnAddInclude, SIGNAL(clicked()), this, SLOT(addIncludeDir()));
    connect(mUI->mBtnEditInclude, &QPushButton::clicked, this, &ProjectFileDialog::editIncludeDir);
    connect(mUI->mBtnRemoveInclude, &QPushButton::clicked, this, &ProjectFileDialog::removeIncludeDir);
    connect(mUI->mBtnAddIgnorePath, SIGNAL(clicked()), this, SLOT(addExcludePath()));
    connect(mUI->mBtnAddIgnoreFile, SIGNAL(clicked()), this, SLOT(addExcludeFile()));
    connect(mUI->mBtnEditIgnorePath, &QPushButton::clicked, this, &ProjectFileDialog::editExcludePath);
    connect(mUI->mBtnRemoveIgnorePath, &QPushButton::clicked, this, &ProjectFileDialog::removeExcludePath);
    connect(mUI->mBtnIncludeUp, &QPushButton::clicked, this, &ProjectFileDialog::moveIncludePathUp);
    connect(mUI->mBtnIncludeDown, &QPushButton::clicked, this, &ProjectFileDialog::moveIncludePathDown);
    connect(mUI->mBtnAddSuppression, &QPushButton::clicked, this, &ProjectFileDialog::addSuppression);
    connect(mUI->mBtnRemoveSuppression, &QPushButton::clicked, this, &ProjectFileDialog::removeSuppression);
    connect(mUI->mListSuppressions, &QListWidget::doubleClicked, this, &ProjectFileDialog::editSuppression);
    connect(mUI->mBtnBrowseMisraFile, &QPushButton::clicked, this, &ProjectFileDialog::browseMisraFile);
    connect(mUI->mChkAllVsConfigs, &QCheckBox::clicked, this, &ProjectFileDialog::checkAllVSConfigs);
    loadFromProjectFile(projectFile);
}

ProjectFileDialog::~ProjectFileDialog()
{
    saveSettings();
    delete mUI;
}

void ProjectFileDialog::loadSettings()
{
    QSettings settings;
    resize(settings.value(SETTINGS_PROJECT_DIALOG_WIDTH, 470).toInt(),
           settings.value(SETTINGS_PROJECT_DIALOG_HEIGHT, 330).toInt());
}

void ProjectFileDialog::saveSettings() const
{
    QSettings settings;
    settings.setValue(SETTINGS_PROJECT_DIALOG_WIDTH, size().width());
    settings.setValue(SETTINGS_PROJECT_DIALOG_HEIGHT, size().height());
}

static void updateAddonCheckBox(QCheckBox *cb, const ProjectFile *projectFile, const QString &dataDir, const QString &addon)
{
    if (projectFile)
        cb->setChecked(projectFile->getAddons().contains(addon));
    if (ProjectFile::getAddonFilePath(dataDir, addon).isEmpty()) {
        cb->setEnabled(false);
        cb->setText(cb->text() + QObject::tr(" (Not found)"));
    }
}

void ProjectFileDialog::checkAllVSConfigs()
{
    if (mUI->mChkAllVsConfigs->isChecked()) {
        for (int row = 0; row < mUI->mListVsConfigs->count(); ++row) {
            QListWidgetItem *item = mUI->mListVsConfigs->item(row);
            item->setCheckState(Qt::Checked);
        }
    }
    mUI->mListVsConfigs->setEnabled(!mUI->mChkAllVsConfigs->isChecked());
}

void ProjectFileDialog::loadFromProjectFile(const ProjectFile *projectFile)
{
    setRootPath(projectFile->getRootPath());
    setBuildDir(projectFile->getBuildDir());
    setIncludepaths(projectFile->getIncludeDirs());
    setDefines(projectFile->getDefines());
    setUndefines(projectFile->getUndefines());
    setCheckPaths(projectFile->getCheckPaths());
    setImportProject(projectFile->getImportProject());
    mUI->mChkAllVsConfigs->setChecked(projectFile->getAnalyzeAllVsConfigs());
    setProjectConfigurations(getProjectConfigs(mUI->mEditImportProject->text()));
    for (int row = 0; row < mUI->mListVsConfigs->count(); ++row) {
        QListWidgetItem *item = mUI->mListVsConfigs->item(row);
        if (projectFile->getAnalyzeAllVsConfigs() || projectFile->getVsConfigurations().contains(item->text()))
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
    }
    switch (projectFile->getCheckLevel()) {
    case ProjectFile::CheckLevel::reduced:
        mUI->mCheckLevelReduced->setChecked(true);
        break;
    case ProjectFile::CheckLevel::normal:
        mUI->mCheckLevelNormal->setChecked(true);
        break;
    case ProjectFile::CheckLevel::exhaustive:
        mUI->mCheckLevelExhaustive->setChecked(true);
        break;
    }
    mUI->mCheckHeaders->setChecked(projectFile->getCheckHeaders());
    mUI->mCheckUnusedTemplates->setChecked(projectFile->getCheckUnusedTemplates());
    mUI->mInlineSuppressions->setChecked(projectFile->getInlineSuppression());
    mUI->mMaxCtuDepth->setValue(projectFile->getMaxCtuDepth());
    mUI->mMaxTemplateRecursion->setValue(projectFile->getMaxTemplateRecursion());
    if (projectFile->clangParser)
        mUI->mBtnClangParser->setChecked(true);
    else
        mUI->mBtnCppcheckParser->setChecked(true);
    mUI->mBtnSafeClasses->setChecked(projectFile->safeChecks.classes);
    setExcludedPaths(projectFile->getExcludedPaths());
    setLibraries(projectFile->getLibraries());
    const QString& platform = projectFile->getPlatform();
    if (platform.endsWith(".xml")) {
        int i;
        for (i = builtinPlatforms.size(); i < mUI->mComboBoxPlatform->count(); ++i) {
            if (mUI->mComboBoxPlatform->itemText(i) == platform)
                break;
        }
        if (i < mUI->mComboBoxPlatform->count())
            mUI->mComboBoxPlatform->setCurrentIndex(i);
        else {
            mUI->mComboBoxPlatform->addItem(platform);
            mUI->mComboBoxPlatform->setCurrentIndex(i);
        }
    } else {
        int i;
        for (i = 0; i < builtinPlatforms.size(); ++i) {
            const Platform::Type p = builtinPlatforms[i];
            if (platform == Platform::toString(p))
                break;
        }
        if (i < builtinPlatforms.size())
            mUI->mComboBoxPlatform->setCurrentIndex(i);
        else
            mUI->mComboBoxPlatform->setCurrentIndex(-1);
    }

    mUI->mComboBoxPlatform->setCurrentText(projectFile->getPlatform());
    setSuppressions(projectFile->getSuppressions());

    // TODO
    // Human knowledge..
    /*
       mUI->mListUnknownFunctionReturn->clear();
       mUI->mListUnknownFunctionReturn->addItem("rand()");
       for (int row = 0; row < mUI->mListUnknownFunctionReturn->count(); ++row) {
        QListWidgetItem *item = mUI->mListUnknownFunctionReturn->item(row);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        const bool unknownValues = projectFile->getCheckUnknownFunctionReturn().contains(item->text());
        item->setCheckState(unknownValues ? Qt::Checked : Qt::Unchecked); // AND initialize check state
       }
       mUI->mCheckSafeClasses->setChecked(projectFile->getSafeChecks().classes);
       mUI->mCheckSafeExternalFunctions->setChecked(projectFile->getSafeChecks().externalFunctions);
       mUI->mCheckSafeInternalFunctions->setChecked(projectFile->getSafeChecks().internalFunctions);
       mUI->mCheckSafeExternalVariables->setChecked(projectFile->getSafeChecks().externalVariables);
     */

    // Addons..
    QSettings settings;
    const QString dataDir = getDataDir();
    updateAddonCheckBox(mUI->mAddonThreadSafety, projectFile, dataDir, "threadsafety");
    updateAddonCheckBox(mUI->mAddonY2038, projectFile, dataDir, "y2038");

    // Misra checkbox..
    if (mPremium)
        mUI->mMisraC->setText("Misra C");
    else {
        mUI->mMisraC->setText("Misra C 2012  " + tr("Note: Open source Cppcheck does not fully implement Misra C 2012"));
    }
    updateAddonCheckBox(mUI->mMisraC, projectFile, dataDir, ADDON_MISRA);
    mUI->mMisraVersion->setEnabled(mUI->mMisraC->isChecked());
    connect(mUI->mMisraC, &QCheckBox::toggled, mUI->mMisraVersion, &QComboBox::setEnabled);

    const QString &misraFile = settings.value(SETTINGS_MISRA_FILE, QString()).toString();
    mUI->mEditMisraFile->setText(misraFile);
    mUI->mMisraVersion->setVisible(mPremium);
    if (projectFile->getCodingStandards().contains(CODING_STANDARD_MISRA_C_2023))
        mUI->mMisraVersion->setCurrentIndex(1);
    else if (projectFile->getCodingStandards().contains(CODING_STANDARD_MISRA_C_2025))
        mUI->mMisraVersion->setCurrentIndex(2);
    else if (projectFile->getAddons().contains(ADDON_MISRA))
        mUI->mMisraVersion->setCurrentIndex(0);
    else
        mUI->mMisraVersion->setCurrentIndex(-1);
    if (mPremium) {
        mUI->mLabelMisraFile->setVisible(false);
        mUI->mEditMisraFile->setVisible(false);
        mUI->mBtnBrowseMisraFile->setVisible(false);
    } else if (!mUI->mMisraC->isEnabled()) {
        mUI->mEditMisraFile->setEnabled(false);
        mUI->mBtnBrowseMisraFile->setEnabled(false);
    }

    mUI->mMisraCpp->setEnabled(mPremium);
    mUI->mMisraCppVersion->setVisible(mPremium);
    if (projectFile->getCodingStandards().contains(CODING_STANDARD_MISRA_CPP_2008)) {
        mUI->mMisraCpp->setChecked(true);
        mUI->mMisraCppVersion->setCurrentIndex(0);
    }
    else if (projectFile->getCodingStandards().contains(CODING_STANDARD_MISRA_CPP_2023)) {
        mUI->mMisraCpp->setChecked(true);
        mUI->mMisraCppVersion->setCurrentIndex(1);
    } else {
        mUI->mMisraCpp->setChecked(false);
        mUI->mMisraCppVersion->setCurrentIndex(1);
    }

    mUI->mMisraCppVersion->setEnabled(mUI->mMisraCpp->isChecked());
    connect(mUI->mMisraCpp, &QCheckBox::toggled, mUI->mMisraCppVersion, &QComboBox::setEnabled);

    mUI->mCertC2016->setChecked(mPremium && projectFile->getCodingStandards().contains(CODING_STANDARD_CERT_C));
    mUI->mCertCpp2016->setChecked(mPremium && projectFile->getCodingStandards().contains(CODING_STANDARD_CERT_CPP));
    mUI->mAutosar->setChecked(mPremium && projectFile->getCodingStandards().contains(CODING_STANDARD_AUTOSAR));
    mUI->mSafetyProfiles->setChecked(mPremium && projectFile->getCodingStandards().contains(CODING_STANDARD_SAFETY_PROFILES));

    if (projectFile->getCertIntPrecision() <= 0)
        mUI->mEditCertIntPrecision->setText(QString());
    else
        mUI->mEditCertIntPrecision->setText(QString::number(projectFile->getCertIntPrecision()));

    mUI->mCertC2016->setEnabled(mPremium);
    mUI->mCertCpp2016->setEnabled(mPremium);
    mUI->mAutosar->setEnabled(mPremium);
    mUI->mSafetyProfiles->setEnabled(mPremium);
    mUI->mLabelCertIntPrecision->setVisible(mPremium);
    mUI->mEditCertIntPrecision->setVisible(mPremium);
    mUI->mBughunting->setChecked(mPremium && projectFile->getBughunting());
    mUI->mBughunting->setEnabled(mPremium);

    mUI->mToolClangAnalyzer->setChecked(projectFile->getClangAnalyzer());
    mUI->mToolClangTidy->setChecked(projectFile->getClangTidy());
    if (CheckThread::clangTidyCmd().isEmpty()) {
        mUI->mToolClangTidy->setText(tr("Clang-tidy (not found)"));
        mUI->mToolClangTidy->setEnabled(false);
    }
    mUI->mEditTags->setText(projectFile->getTags().join(';'));
    mUI->mEditLicenseFile->setText(projectFile->getLicenseFile());
    updatePathsAndDefines();
}

void ProjectFileDialog::saveToProjectFile(ProjectFile *projectFile) const
{
    projectFile->setRootPath(getRootPath());
    projectFile->setBuildDir(getBuildDir());
    projectFile->setImportProject(getImportProject());
    projectFile->setAnalyzeAllVsConfigs(mUI->mChkAllVsConfigs->isChecked());
    projectFile->setVSConfigurations(getProjectConfigurations());
    projectFile->setCheckHeaders(mUI->mCheckHeaders->isChecked());
    projectFile->setCheckUnusedTemplates(mUI->mCheckUnusedTemplates->isChecked());
    projectFile->setInlineSuppression(mUI->mInlineSuppressions->isChecked());
    projectFile->setMaxCtuDepth(mUI->mMaxCtuDepth->value());
    projectFile->setMaxTemplateRecursion(mUI->mMaxTemplateRecursion->value());
    projectFile->setIncludes(getIncludePaths());
    projectFile->setDefines(getDefines());
    projectFile->setUndefines(getUndefines());
    projectFile->setCheckPaths(getCheckPaths());
    projectFile->setExcludedPaths(getExcludedPaths());
    projectFile->setLibraries(getLibraries());
    if (mUI->mCheckLevelReduced->isChecked())
        projectFile->setCheckLevel(ProjectFile::CheckLevel::reduced);
    else if (mUI->mCheckLevelNormal->isChecked())
        projectFile->setCheckLevel(ProjectFile::CheckLevel::normal);
    else // if (mUI->mCheckLevelExhaustive->isChecked())
        projectFile->setCheckLevel(ProjectFile::CheckLevel::exhaustive);
    projectFile->clangParser = mUI->mBtnClangParser->isChecked();
    projectFile->safeChecks.classes = mUI->mBtnSafeClasses->isChecked();
    if (mUI->mComboBoxPlatform->currentText().endsWith(".xml"))
        projectFile->setPlatform(mUI->mComboBoxPlatform->currentText());
    else {
        const int i = mUI->mComboBoxPlatform->currentIndex();
        if (i>=0 && i < builtinPlatforms.size())
            projectFile->setPlatform(Platform::toString(builtinPlatforms[i]));
        else
            projectFile->setPlatform(QString());
    }
    projectFile->setSuppressions(getSuppressions());
    // Human knowledge
    /*
       QStringList unknownReturnValues;
       for (int row = 0; row < mUI->mListUnknownFunctionReturn->count(); ++row) {
        QListWidgetItem *item = mUI->mListUnknownFunctionReturn->item(row);
        if (item->checkState() == Qt::Checked)
            unknownReturnValues << item->text();
       }
       projectFile->setCheckUnknownFunctionReturn(unknownReturnValues);
       ProjectFile::SafeChecks safeChecks;
       safeChecks.classes = mUI->mCheckSafeClasses->isChecked();
       safeChecks.externalFunctions = mUI->mCheckSafeExternalFunctions->isChecked();
       safeChecks.internalFunctions = mUI->mCheckSafeInternalFunctions->isChecked();
       safeChecks.externalVariables = mUI->mCheckSafeExternalVariables->isChecked();
       projectFile->setSafeChecks(safeChecks);
     */
    // Addons
    QStringList addons;
    if (mUI->mAddonThreadSafety->isChecked())
        addons << "threadsafety";
    if (mUI->mAddonY2038->isChecked())
        addons << "y2038";
    if (mUI->mMisraC->isChecked())
        addons << ADDON_MISRA;
    projectFile->setAddons(addons);
    QStringList codingStandards;
    if (mUI->mCertC2016->isChecked())
        codingStandards << CODING_STANDARD_CERT_C;
    if (mUI->mCertCpp2016->isChecked())
        codingStandards << CODING_STANDARD_CERT_CPP;
    if (mPremium && mUI->mMisraVersion->currentIndex() == 1)
        codingStandards << CODING_STANDARD_MISRA_C_2023;
    if (mPremium && mUI->mMisraVersion->currentIndex() == 2)
        codingStandards << CODING_STANDARD_MISRA_C_2025;
    if (mUI->mMisraCpp->isChecked() && mUI->mMisraCppVersion->currentIndex() == 0)
        codingStandards << CODING_STANDARD_MISRA_CPP_2008;
    if (mUI->mMisraCpp->isChecked() && mUI->mMisraCppVersion->currentIndex() == 1)
        codingStandards << CODING_STANDARD_MISRA_CPP_2023;
    if (mUI->mAutosar->isChecked())
        codingStandards << CODING_STANDARD_AUTOSAR;
    if (mUI->mSafetyProfiles->isChecked())
        codingStandards << CODING_STANDARD_SAFETY_PROFILES;
    projectFile->setCodingStandards(std::move(codingStandards));
    projectFile->setCertIntPrecision(mUI->mEditCertIntPrecision->text().toInt());
    projectFile->setBughunting(mUI->mBughunting->isChecked());
    projectFile->setClangAnalyzer(mUI->mToolClangAnalyzer->isChecked());
    projectFile->setClangTidy(mUI->mToolClangTidy->isChecked());
    projectFile->setLicenseFile(mUI->mEditLicenseFile->text());
    projectFile->setTags(mUI->mEditTags->text().split(";", Qt::SkipEmptyParts));
}

void ProjectFileDialog::ok()
{
    saveToProjectFile(mProjectFile);
    mProjectFile->write();
    accept();
}

QString ProjectFileDialog::getExistingDirectory(const QString &caption, bool trailingSlash)
{
    const QFileInfo inf(mProjectFile->getFilename());
    const QString projectPath = inf.absolutePath();
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                                                            caption,
                                                            projectPath);

    if (selectedDir.isEmpty())
        return QString();

    // Check if the path is relative to project file's path and if so
    // make it a relative path instead of absolute path.
    const QDir dir(projectPath);
    const QString relpath(dir.relativeFilePath(selectedDir));
    if (!relpath.startsWith("../.."))
        selectedDir = relpath;

    // Trailing slash..
    if (trailingSlash && !selectedDir.endsWith('/'))
        selectedDir += '/';

    return selectedDir;
}

void ProjectFileDialog::browseBuildDir()
{
    const QString dir(getExistingDirectory(tr("Select Cppcheck build dir"), false));
    if (!dir.isEmpty())
        mUI->mEditBuildDir->setText(dir);
}

void ProjectFileDialog::updatePathsAndDefines()
{
    const QString &fileName = mUI->mEditImportProject->text();
    const bool importProject = !fileName.isEmpty();
    const bool hasConfigs = fileName.endsWith(".sln") || fileName.endsWith(".slnx") || fileName.endsWith(".vcxproj");
    mUI->mBtnClearImportProject->setEnabled(importProject);
    mUI->mListCheckPaths->setEnabled(!importProject);
    mUI->mListIncludeDirs->setEnabled(!importProject);
    mUI->mBtnAddCheckPath->setEnabled(!importProject);
    mUI->mBtnEditCheckPath->setEnabled(!importProject);
    mUI->mBtnRemoveCheckPath->setEnabled(!importProject);
    mUI->mEditUndefines->setEnabled(!importProject);
    mUI->mBtnAddInclude->setEnabled(!importProject);
    mUI->mBtnEditInclude->setEnabled(!importProject);
    mUI->mBtnRemoveInclude->setEnabled(!importProject);
    mUI->mBtnIncludeUp->setEnabled(!importProject);
    mUI->mBtnIncludeDown->setEnabled(!importProject);
    mUI->mChkAllVsConfigs->setEnabled(hasConfigs);
    mUI->mListVsConfigs->setEnabled(hasConfigs && !mUI->mChkAllVsConfigs->isChecked());
    if (!hasConfigs)
        mUI->mListVsConfigs->clear();
}

void ProjectFileDialog::clearImportProject()
{
    mUI->mEditImportProject->clear();
    updatePathsAndDefines();
}

void ProjectFileDialog::browseImportProject()
{
    const QFileInfo inf(mProjectFile->getFilename());
    const QDir &dir = inf.absoluteDir();
    QMap<QString,QString> filters;
    filters[tr("Visual Studio")] = "*.sln *.slnx *.vcxproj";
    filters[tr("Compile database")] = "compile_commands.json";
    filters[tr("Borland C++ Builder 6")] = "*.bpr";
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Project"),
                                                    dir.canonicalPath(),
                                                    toFilterString(filters));
    if (!fileName.isEmpty()) {
        mUI->mEditImportProject->setText(dir.relativeFilePath(fileName));
        updatePathsAndDefines();
        setProjectConfigurations(getProjectConfigs(fileName));
        for (int row = 0; row < mUI->mListVsConfigs->count(); ++row) {
            QListWidgetItem *item = mUI->mListVsConfigs->item(row);
            item->setCheckState(Qt::Checked);
        }
    }
}

QStringList ProjectFileDialog::getProjectConfigurations() const
{
    QStringList configs;
    for (int row = 0; row < mUI->mListVsConfigs->count(); ++row) {
        QListWidgetItem *item = mUI->mListVsConfigs->item(row);
        if (item->checkState() == Qt::Checked)
            configs << item->text();
    }
    return configs;
}

void ProjectFileDialog::setProjectConfigurations(const QStringList &configs)
{
    mUI->mListVsConfigs->clear();
    mUI->mListVsConfigs->setEnabled(!configs.isEmpty() && !mUI->mChkAllVsConfigs->isChecked());
    for (const QString &cfg : configs) {
        auto* item = new QListWidgetItem(cfg, mUI->mListVsConfigs);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Unchecked);
    }
}

QString ProjectFileDialog::getImportProject() const
{
    return mUI->mEditImportProject->text();
}

void ProjectFileDialog::addIncludeDir(const QString &dir)
{
    if (dir.isEmpty())
        return;

    const QString newdir = QDir::toNativeSeparators(dir);
    auto *item = new QListWidgetItem(newdir);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI->mListIncludeDirs->addItem(item);
}

void ProjectFileDialog::addCheckPath(const QString &path)
{
    if (path.isEmpty())
        return;

    const QString newpath = QDir::toNativeSeparators(path);
    auto *item = new QListWidgetItem(newpath);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI->mListCheckPaths->addItem(item);
}

void ProjectFileDialog::addExcludePath(const QString &path)
{
    if (path.isEmpty())
        return;

    const QString newpath = QDir::toNativeSeparators(path);
    auto *item = new QListWidgetItem(newpath);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI->mListExcludedPaths->addItem(item);
}

QString ProjectFileDialog::getRootPath() const
{
    QString root = mUI->mEditProjectRoot->text();
    root = root.trimmed();
    root = QDir::fromNativeSeparators(root);
    return root;
}

QString ProjectFileDialog::getBuildDir() const
{
    return mUI->mEditBuildDir->text();
}

QStringList ProjectFileDialog::getIncludePaths() const
{
    return getPaths(mUI->mListIncludeDirs);
}

QStringList ProjectFileDialog::getDefines() const
{
    return mUI->mEditDefines->text().trimmed().split(QRegularExpression("\\s*;\\s*"), Qt::SkipEmptyParts);
}

QStringList ProjectFileDialog::getUndefines() const
{
    const QString undefine = mUI->mEditUndefines->text().trimmed();
    QStringList undefines = undefine.split(QRegularExpression("\\s*;\\s*"), Qt::SkipEmptyParts);
    undefines.removeDuplicates();
    return undefines;
}

QStringList ProjectFileDialog::getCheckPaths() const
{
    return getPaths(mUI->mListCheckPaths);
}

QStringList ProjectFileDialog::getExcludedPaths() const
{
    return getPaths(mUI->mListExcludedPaths);
}

QStringList ProjectFileDialog::getLibraries() const
{
    QStringList libraries;
    for (int row = 0; row < mUI->mLibraries->count(); ++row) {
        QListWidgetItem *item = mUI->mLibraries->item(row);
        if (item->checkState() == Qt::Checked)
            libraries << item->text();
    }
    return libraries;
}

void ProjectFileDialog::setRootPath(const QString &root)
{
    mUI->mEditProjectRoot->setText(QDir::toNativeSeparators(root));
}

void ProjectFileDialog::setBuildDir(const QString &buildDir)
{
    mUI->mEditBuildDir->setText(buildDir);
}

void ProjectFileDialog::setImportProject(const QString &importProject)
{
    mUI->mEditImportProject->setText(importProject);
}

void ProjectFileDialog::setIncludepaths(const QStringList &includes)
{
    for (const QString& dir : includes) {
        addIncludeDir(dir);
    }
}

void ProjectFileDialog::setDefines(const QStringList &defines)
{
    mUI->mEditDefines->setText(defines.join(";"));
}

void ProjectFileDialog::setUndefines(const QStringList &undefines)
{
    mUI->mEditUndefines->setText(undefines.join(";"));
}

void ProjectFileDialog::setCheckPaths(const QStringList &paths)
{
    for (const QString& path : paths) {
        addCheckPath(path);
    }
}

void ProjectFileDialog::setExcludedPaths(const QStringList &paths)
{
    for (const QString& path : paths) {
        addExcludePath(path);
    }
}

void ProjectFileDialog::setLibraries(const QStringList &libraries)
{
    for (int row = 0; row < mUI->mLibraries->count(); ++row) {
        QListWidgetItem *item = mUI->mLibraries->item(row);
        item->setCheckState(libraries.contains(item->text()) ? Qt::Checked : Qt::Unchecked);
    }
}

void ProjectFileDialog::addSingleSuppression(const SuppressionList::Suppression &suppression)
{
    mSuppressions += suppression;
    mUI->mListSuppressions->addItem(QString::fromStdString(suppressionAsText(suppression)));
}

void ProjectFileDialog::setSuppressions(const QList<SuppressionList::Suppression> &suppressions)
{
    mUI->mListSuppressions->clear();
    QList<SuppressionList::Suppression> new_suppressions = suppressions;
    mSuppressions.clear();
    for (const SuppressionList::Suppression &suppression : new_suppressions) {
        addSingleSuppression(suppression);
    }
    mUI->mListSuppressions->sortItems();
}

void ProjectFileDialog::addCheckPath()
{
    QString dir = getExistingDirectory(tr("Select a directory to check"), false);
    if (!dir.isEmpty())
        addCheckPath(dir);
}

void ProjectFileDialog::editCheckPath()
{
    QListWidgetItem *item = mUI->mListCheckPaths->currentItem();
    mUI->mListCheckPaths->editItem(item);
}

void ProjectFileDialog::removeCheckPath()
{
    const int row = mUI->mListCheckPaths->currentRow();
    QListWidgetItem *item = mUI->mListCheckPaths->takeItem(row);
    delete item;
}

void ProjectFileDialog::addIncludeDir()
{
    const QString dir = getExistingDirectory(tr("Select include directory"), true);
    if (!dir.isEmpty())
        addIncludeDir(dir);
}

void ProjectFileDialog::removeIncludeDir()
{
    const int row = mUI->mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI->mListIncludeDirs->takeItem(row);
    delete item;
}

void ProjectFileDialog::editIncludeDir()
{
    QListWidgetItem *item = mUI->mListIncludeDirs->currentItem();
    mUI->mListIncludeDirs->editItem(item);
}

void ProjectFileDialog::addExcludePath()
{
    addExcludePath(getExistingDirectory(tr("Select directory to ignore"), true));
}

void ProjectFileDialog::addExcludeFile()
{
    const QFileInfo inf(mProjectFile->getFilename());
    const QDir &dir = inf.absoluteDir();
    QMap<QString,QString> filters;
    filters[tr("Source files")] = "*.c *.cpp";
    filters[tr("All files")] = "*.*";
    addExcludePath(QFileDialog::getOpenFileName(this, tr("Exclude file"), dir.canonicalPath(), toFilterString(filters)));
}

void ProjectFileDialog::editExcludePath()
{
    QListWidgetItem *item = mUI->mListExcludedPaths->currentItem();
    mUI->mListExcludedPaths->editItem(item);
}

void ProjectFileDialog::removeExcludePath()
{
    const int row = mUI->mListExcludedPaths->currentRow();
    QListWidgetItem *item = mUI->mListExcludedPaths->takeItem(row);
    delete item;
}

void ProjectFileDialog::moveIncludePathUp()
{
    int row = mUI->mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI->mListIncludeDirs->takeItem(row);
    row = row > 0 ? row - 1 : 0;
    mUI->mListIncludeDirs->insertItem(row, item);
    mUI->mListIncludeDirs->setCurrentItem(item);
}

void ProjectFileDialog::moveIncludePathDown()
{
    int row = mUI->mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI->mListIncludeDirs->takeItem(row);
    const int count = mUI->mListIncludeDirs->count();
    row = row < count ? row + 1 : count;
    mUI->mListIncludeDirs->insertItem(row, item);
    mUI->mListIncludeDirs->setCurrentItem(item);
}

void ProjectFileDialog::addSuppression()
{
    NewSuppressionDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        addSingleSuppression(dlg.getSuppression());
    }
}

void ProjectFileDialog::removeSuppression()
{
    const int row = mUI->mListSuppressions->currentRow();
    QListWidgetItem *item = mUI->mListSuppressions->takeItem(row);
    if (!item)
        return;

    const int suppressionIndex = getSuppressionIndex(item->text());
    if (suppressionIndex >= 0)
        mSuppressions.removeAt(suppressionIndex);
    delete item;
}

void ProjectFileDialog::editSuppression(const QModelIndex & /*index*/)
{
    const int row = mUI->mListSuppressions->currentRow();
    QListWidgetItem *item = mUI->mListSuppressions->item(row);
    const int suppressionIndex = getSuppressionIndex(item->text());
    if (suppressionIndex >= 0) { // TODO what if suppression is not found?
        NewSuppressionDialog dlg;
        dlg.setSuppression(mSuppressions[suppressionIndex]);
        if (dlg.exec() == QDialog::Accepted) {
            mSuppressions[suppressionIndex] = dlg.getSuppression();
            setSuppressions(mSuppressions);
        }
    }
}

int ProjectFileDialog::getSuppressionIndex(const QString &shortText) const
{
    const std::string s = shortText.toStdString();
    auto it = std::find_if(mSuppressions.cbegin(), mSuppressions.cend(), [&](const SuppressionList::Suppression& sup) {
        return suppressionAsText(sup) == s;
    });
    return it == mSuppressions.cend() ? -1 : static_cast<int>(std::distance(mSuppressions.cbegin(), it));
}

void ProjectFileDialog::browseMisraFile()
{
    const QString fileName = QFileDialog::getOpenFileName(this,
                                                          tr("Select MISRA rule texts file"),
                                                          QDir::homePath(),
                                                          tr("MISRA rule texts file (%1)").arg("*.txt"));
    if (!fileName.isEmpty()) {
        QSettings settings;
        mUI->mEditMisraFile->setText(fileName);
        settings.setValue(SETTINGS_MISRA_FILE, fileName);

        mUI->mMisraC->setText("MISRA C 2012");
        mUI->mMisraC->setEnabled(true);
        updateAddonCheckBox(mUI->mMisraC, nullptr, getDataDir(), ADDON_MISRA);
    }
}
