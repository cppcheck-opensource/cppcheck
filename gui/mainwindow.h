/* -*- C++ -*-
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "library.h"
#include "platforms.h"

#include <cstdint>
#include <list>

#include <QFileDialog>
#include <QMainWindow>
#include <QObject>
#include <QString>
#include <QStringList>

class ThreadHandler;
class TranslationHandler;
class ScratchPad;
class ProjectFile;
class ApplicationList;
class QAction;
class QActionGroup;
class QSettings;
class QTimer;
class QLineEdit;
class ImportProject;
class QNetworkAccessManager;
class QNetworkReply;
class Settings;
struct Suppressions;
class FileWithDetails;
namespace Ui {
    class MainWindow;
}

/// @addtogroup GUI
/// @{

/**
 * @brief Main window for cppcheck-gui
 *
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:

    /**
     * @brief Maximum number of MRU project items in File-menu.
     */
    enum : std::uint8_t { MaxRecentProjects = 5 };

    MainWindow(TranslationHandler* th, QSettings* settings);
    MainWindow(const MainWindow &) = delete;
    ~MainWindow() override;
    MainWindow &operator=(const MainWindow &) = delete;

    /**
     * List of checked platforms.
     */
    Platforms mPlatforms;

    /**
     * @brief Analyze given code
     *
     * @param code Content of the (virtual) file to be analyzed
     * @param filename Name of the (virtual) file to be analyzed - determines language.
     */
    void analyzeCode(const QString& code, const QString& filename);

public slots:
    /** @brief Slot for analyze files menu item */
    void analyzeFiles();

    /** @brief Slot to reanalyze all files */
    void reAnalyzeAll();

    /** @brief Slot to reanalyze with checking library configuration */
    void checkLibrary();

    /** @brief Slot to check configuration */
    void checkConfiguration();

    /**
     * @brief Slot to reanalyze selected files
     * @param selectedFilesList list of selected files
     */
    void performSelectedFilesCheck(const QStringList &selectedFilesList);

    /** @brief Slot to reanalyze modified files */
    void reAnalyzeModified();

    /** @brief Slot to clear all search results */
    void clearResults();

    /** @brief Slot to open XML report file */
    void openResults();

    /**
     * @brief Show errors with type "style"
     * @param checked Should errors be shown (true) or hidden (false)
     */
    void showStyle(bool checked);

    /**
     * @brief Show errors with type "error"
     * @param checked Should errors be shown (true) or hidden (false)
     */
    void showErrors(bool checked);

    /**
     * @brief Show errors with type "warning"
     * @param checked Should errors be shown (true) or hidden (false)
     */
    void showWarnings(bool checked);

    /**
     * @brief Show errors with type "portability"
     * @param checked Should errors be shown (true) or hidden (false)
     */
    void showPortability(bool checked);

    /**
     * @brief Show errors with type "performance"
     * @param checked Should errors be shown (true) or hidden (false)
     */
    void showPerformance(bool checked);

    /**
     * @brief Show errors with type "information"
     * @param checked Should errors be shown (true) or hidden (false)
     */
    void showInformation(bool checked);

    /** @brief Slot to check all "Show errors" menu items */
    void checkAll();

    /** @brief Slot to uncheck all "Show errors" menu items */
    void uncheckAll();

    /** @brief Slot for analyze directory menu item */
    void analyzeDirectory();

    /** @brief Slot to open program's settings dialog */
    void programSettings();

    /** @brief Slot to open program's about dialog */
    void about();

    /** @brief Slot to to show license text */
    void showLicense();

    /** @brief Slot to to show authors list */
    void showAuthors();

    /** @brief Slot to to show EULA */
    void showEULA();

    /** @brief Slot to save results */
    void save();

    /** @brief Slot to create new project file */
    void newProjectFile();

    /** @brief Slot to open project file and start analyzing contained paths. */
    void openProjectFile();

    /** @brief Slot to show scratchpad. */
    void showScratchpad();

    /** @brief Slot to close open project file. */
    void closeProjectFile();

    /** @brief Slot to edit project file. */
    void editProjectFile();

    /** @brief Slot for showing the scan and project statistics. */
    void showStatistics();

    /** @brief Slot for showing the library editor */
    void showLibraryEditor();

    /** @brief Slot for showing the thread details window */
    void showThreadDetails();

private slots:

    /** @brief Slot for checkthread's done signal */
    void analysisDone();

    /** @brief Lock down UI while analyzing */
    void checkLockDownUI();

    /** @brief Slot for enabling save and clear button */
    void resultsAdded();

    /** @brief Slot for showing/hiding standard toolbar */
    void toggleMainToolBar();

    /** @brief Slot for showing/hiding Categories toolbar */
    void toggleViewToolBar();

    /** @brief Slot for showing/hiding Filter toolbar */
    void toggleFilterToolBar();

    /** @brief Slot for updating View-menu before it is shown. */
    void aboutToShowViewMenu();

    /** @brief Slot when stop analysis button is pressed */
    void stopAnalysis();

    /** @brief Open help file contents */
    void openHelpContents();

    /** @brief Filters the results in the result list. */
    void filterResults();

    /** @brief Opens recently opened project file. */
    void openRecentProject();

    /** @brief Selects the platform as analyzed platform. */
    void selectPlatform();

    /** Suppress error ids */
    void suppressIds(QStringList ids);

private slots:
    void replyFinished(QNetworkReply *reply);

    void hideInformation();

    void changeReportType();
private:

    bool isCppcheckPremium() const;

    /** Get filename for last results */
    QString getLastResults() const;

    /** @brief Reanalyzes files */
    void reAnalyze(bool all);

    /**
     * @brief Reanalyze selected files
     * @param files list of selected files
     */
    void reAnalyzeSelected(const QStringList& files);

    /**
     * @brief Analyze the project.
     * @param projectFile Pointer to the project to analyze.
     * @param recheckFiles files to recheck, empty => check all files
     * @param checkLibrary Flag to indicate if the library should be checked.
     * @param checkConfiguration Flag to indicate if the configuration should be checked.
     */
    void analyzeProject(const ProjectFile *projectFile, const QStringList& recheckFiles, bool checkLibrary = false, bool checkConfiguration = false);

    /**
     * @brief Set current language
     * @param code Language code of the language to set (e.g. "en").
     */
    void setLanguage(const QString &code);

    /** @brief Event coming when application is about to close. */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Helper function to toggle all show error menu items
     * @param checked Should all errors be shown (true) or hidden (false)
     */
    void toggleAllChecked(bool checked);

    /** @brief Helper function to enable/disable all check,recheck buttons */
    void enableCheckButtons(bool enable);

    /** @brief Helper function to enable/disable results buttons (clear,save,print) */
    void enableResultsButtons();

    /**
     * @brief Select files/or directory to analyze.
     * Helper function to open a dialog to ask user to select files or
     * directory to analyze. Use native dialogs instead of Qt:s own dialogs.
     *
     * @param mode Dialog open mode (files or directories)
     * @return QStringList of files or directories that were selected to analyze
     */
    QStringList selectFilesToAnalyze(QFileDialog::FileMode mode);

    /**
     * @brief Analyze project
     * @param p imported project
     * @param checkLibrary Flag to indicate if library should be checked
     * @param checkConfiguration Flag to indicate if the configuration should be checked.
     */
    void doAnalyzeProject(ImportProject p, bool checkLibrary = false, bool checkConfiguration = false);

    /**
     * @brief Analyze all files specified in parameter files
     *
     * @param files List of files and/or directories to analyze
     * @param checkLibrary Flag to indicate if library should be checked
     * @param checkConfiguration Flag to indicate if the configuration should be checked.
     */
    void doAnalyzeFiles(const QStringList &files, bool checkLibrary = false, bool checkConfiguration = false);

    /**
     * @brief Get our default cppcheck settings and read project file.
     */
    bool getCppcheckSettings(Settings& settings, Suppressions& supprs);

    /** @brief Load program settings */
    void loadSettings();

    /** @brief Save program settings */
    void saveSettings() const;

    /**
     * @brief Format main window title.
     * @param text Text added to end of the title.
     */
    void formatAndSetTitle(const QString &text = QString());

    /** @brief Show help contents */
    static void openOnlineHelp();

    /**
     * @brief Enable or disable project file actions.
     * Project editing and closing actions should be only enabled when project is
     * open and we are not analyzing files.
     * @param enable If true then actions are enabled.
     */
    void enableProjectActions(bool enable);

    /**
     * @brief Enable or disable project file actions.
     * Project opening and creating actions should be disabled when analyzing.
     * @param enable If true then actions are enabled.
     */
    void enableProjectOpenActions(bool enable);

    /**
     * @brief Add include directories.
     * @param includeDirs List of include directories to add.
     * @param result Settings class where include directories are added.
     */
    void addIncludeDirs(const QStringList &includeDirs, Settings &result);

    /**
     * @brief Handle command line parameters given to GUI.
     * @param params List of string given to command line.
     */
    void handleCLIParams(const QStringList &params);

    /**
     * @brief Load XML file to the GUI.
     * @param selectedFile Filename (inc. path) of XML file to load.
     */
    void loadResults(const QString &selectedFile);

    /**
     * @brief Load XML file to the GUI.
     * @param selectedFile Filename (inc. path) of XML file to load.
     * @param sourceDirectory Path to the directory that the results were generated for.
     */
    void loadResults(const QString &selectedFile, const QString &sourceDirectory);

    /**
     * @brief Load last project results to the GUI.
     * @return Returns true if last results was loaded
     */
    bool loadLastResults();

    /**
     * @brief Load project file to the GUI.
     * @param filePath Filename (inc. path) of project file to load.
     */
    void loadProjectFile(const QString &filePath);

    /**
     * @brief Load library file
     * @param library  library to use
     * @param filename filename (no path)
     * @return error code
     */
    Library::Error loadLibrary(Library &library, const QString &filename);

    /**
     * @brief Tries to load library file, prints message on error
     * @param library  library to use
     * @param filename filename (no path)
     * @return True if no error
     */
    bool tryLoadLibrary(Library &library, const QString& filename);

    QString loadAddon(Settings &settings, const QString &filesDir, const QString &pythonCmd, const QString& addon);

    /**
     * @brief Update project MRU items in File-menu.
     */
    void updateMRUMenuItems();

    /**
     * @brief Add project file (path) to the MRU list.
     * @param project Full path to the project file to add.
     */
    void addProjectMRU(const QString &project);

    /**
     * @brief Remove project file (path) from the MRU list.
     * @param project Full path of the project file to remove.
     */
    void removeProjectMRU(const QString &project);

    /** @brief Generate list of detailed files from list of filenames. */
    std::list<FileWithDetails> enrichFilesForAnalysis(const QStringList& fileNames, const Settings& settings) const;

    /** @brief Program settings */
    QSettings *mSettings;

    /** @brief Thread to analyze files */
    ThreadHandler *mThread;

    /** @brief List of user defined applications to open errors with */
    ApplicationList *mApplications;

    /** @brief Class to handle translation changes */
    TranslationHandler *mTranslation;

    /** @brief Class holding all UI components */
    Ui::MainWindow *mUI;

    /** @brief Current analyzed directory. */
    QString mCurrentDirectory;

    /** @brief Scratchpad. */
    ScratchPad* mScratchPad{};

    /** @brief Project (file). */
    ProjectFile* mProjectFile{};

    /** @brief Filter field in the Filter toolbar. */
    QLineEdit* mLineEditFilter;

    /** @brief Timer to delay filtering while typing. */
    QTimer* mFilterTimer;

    /** @brief GUI actions for selecting the analyzed platform. */
    QActionGroup *mPlatformActions;

    /** @brief GUI actions for selecting the coding standard. */
    QActionGroup *mCStandardActions, *mCppStandardActions;

    /** @brief GUI actions for selecting language. */
    QActionGroup *mSelectLanguageActions;

    /** @brief GUI actions for selecting report. */
    QActionGroup *mSelectReportActions;

    /**
     * @brief Are we exiting the cppcheck?
     * If this is true then the cppcheck is waiting for check threads to exit
     * so that the application can be closed.
     */
    bool mExiting{};

    /** @brief Set to true in case of loading log file. */
    bool mIsLogfileLoaded{};

    /**
     * @brief Project MRU menu actions.
     * List of MRU menu actions. Needs also to store the separator.
     */
    QAction *mRecentProjectActs[MaxRecentProjects + 1];

    QString mCppcheckCfgAbout;
    QString mCppcheckCfgProductName;

    QNetworkAccessManager *mNetworkAccessManager = nullptr;
};
/// @}
#endif // MAINWINDOW_H
