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


#ifndef RESULTSTREE_H
#define RESULTSTREE_H

#include "showtypes.h"
#include "checkers.h"
#include "resultitem.h"

#include <cstdint>
#include <map>
#include <string>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTreeView>

class ApplicationList;
class Report;
class ErrorItem;
class QModelIndex;
class QWidget;
class QItemSelectionModel;
class ThreadHandler;
class QSettings;
class QStandardItemModel;
enum class Severity : std::uint8_t;

/// @addtogroup GUI
/// @{

/**
 * @brief Cppcheck's results are shown in this tree
 *
 */
class ResultsTree : public QTreeView {
    Q_OBJECT
public:
    explicit ResultsTree(QWidget * parent = nullptr);

    void initialize(QSettings *settings, ApplicationList *list, ThreadHandler *checkThreadHandler);

    /**
     * @brief Add a new item to the tree
     *
     * @param errorItem Error item data
     */
    bool addErrorItem(const ErrorItem& errorItem);

    /**
     * @brief Clear all errors from the tree
     *
     */
    void clear();

    /**
     * @brief Clear errors for a specific file from the tree
     */
    void clear(const QString &filename);

    /**
     * @brief Clear errors of a file selected for recheck
     */
    void clearRecheckFile(const QString &filename);

    /**
     * @brief Function to filter the displayed list of errors.
     * Refreshes the tree.
     *
     * @param filter String that must be found in the summary, description, file or id
     */
    void filterResults(const QString& filter);

    /**
     * @brief Function to show results that were previous hidden with HideResult()
     */
    void showHiddenResults();

    /**
     * @brief Refresh tree by checking which of the items should be shown
     * and which should be hidden
     */
    void refreshTree();

    /**
     * @brief Save results to a text stream
     *
     */
    void saveResults(Report *report) const;

    /**
     * @brief Update items from old report (tag, sinceDate)
     */
    void updateFromOldReport(const QString &filename);

    /**
     * @brief Update tree settings
     *
     * @param showFullPath Show full path of files in the tree
     * @param saveFullPath Save full path of files in reports
     * @param saveAllErrors Save all visible errors
     * @param showErrorId Show error id
     * @param showInconclusive Show inconclusive column
     */
    void updateSettings(bool showFullPath, bool saveFullPath, bool saveAllErrors, bool showErrorId, bool showInconclusive);

    /**
     * @brief Set the directory we are checking
     *
     * This is used to split error file path to relative if necessary
     * @param dir Directory we are checking
     */
    void setCheckDirectory(const QString &dir);

    /**
     * @brief Get the directory we are checking
     *
     * @return Directory containing source files
     */

    const QString& getCheckDirectory() const;

    /**
     * @brief Results source for analysis results in the results tree.
     */
    enum class ResultsSource : std::uint8_t {
        /** Results from a project, files, or directory check */
        Analysis,
        /** Saved results from a log file */
        Log,
    };

    /**
     * @brief Set the source type of the current results. This
     * affects the actions that are allowed on them.
     *
     * @param source The results source type.
     */
    void setResultsSource(ResultsSource source);

    /**
     * @brief Check if there are any visible results in view.
     * @return true if there is at least one visible warning/error.
     */
    bool hasVisibleResults() const;

    /**
     * @brief Do we have results from check?
     * @return true if there is at least one warning/error, hidden or visible.
     */
    bool hasResults() const;

    /**
     * @brief Save all settings
     * Column widths
     */
    void saveSettings() const;

    /**
     * @brief Change all visible texts language
     *
     */
    void translate();

    /**
     * @brief Show optional column "Id"
     */
    void showIdColumn(bool show);

    /**
     * @brief Show optional column "Inconclusve"
     */
    void showInconclusiveColumn(bool show);

    /**
     * @brief Returns true if column "Id" is shown
     */
    bool showIdColumn() const {
        return mShowErrorId;
    }

    /**
     * @brief GUI severities.
     */
    ShowTypes mShowSeverities;

    void setReportType(ReportType reportType);

    /**
     * @brief should errorItem be hidden by filter/severity/etc?
     * @param errorItem error item
     * @return true if error item should be hidden
     */
    bool isErrorItemHidden(const QSharedPointer<ErrorItem>& errorItem) const;

signals:
    /**
     * @brief Signal that results have been hidden or shown
     *
     * @param hidden true if there are some hidden results, or false if there are not
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void resultsHidden(bool hidden);

    /**
     * @brief Signal to perform selected files recheck
     *
     * @param selectedItems list of selected files
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void checkSelected(QStringList selectedItems);

    /**
     * @brief Signal for selection change in result tree.
     * @param selectedItem item that was selected
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void treeSelectionChanged(const ResultItem *selectedItem);

    /** Suppress Ids */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void suppressIds(QStringList ids);

public slots:

    /**
     * @brief Function to show/hide certain type of errors
     * Refreshes the tree.
     *
     * @param type Type of error to show/hide
     * @param show Should specified errors be shown (true) or hidden (false)
     */
    void showResults(ShowTypes::ShowType type, bool show);


    /**
     * @brief Show/hide cppcheck errors.
     * Refreshes the tree.
     *
     * @param show Should specified errors be shown (true) or hidden (false)
     */
    void showCppcheckResults(bool show);

    /**
     * @brief Show/hide clang-tidy/clang-analyzer errors.
     * Refreshes the tree.
     *
     * @param show Should specified errors be shown (true) or hidden (false)
     */
    void showClangResults(bool show);

protected slots:
    /**
     * @brief Slot to quickstart an error with default application
     *
     * @param index Model index to specify which error item to open
     */
    void quickStartApplication(const QModelIndex &index);

    /**
     * @brief Slot for context menu item to open an error with specified application
     *
     * @param application Index of the application to open the error
     */
    void context(int application);

    /**
     * @brief Slot for context menu item to copy selection to clipboard
     */
    void copy();

    /**
     * @brief Slot for context menu item to hide the current error message
     *
     */
    void hideResult();

    /**
     * @brief Slot for rechecking selected files
     *
     */
    void recheckSelectedFiles();

    /**
     * @brief Slot for context menu item to hide all messages with the current message Id
     *
     */
    void hideAllIdResult();

    /** Slot for context menu item to suppress all messages with the current message id */
    void suppressSelectedIds();

    /** Slot for context menu item to suppress message with hash */
    void suppressHash();

    /**
     * @brief Slot for context menu item to open the folder containing the current file.
     */
    void openContainingFolder();

    /**
     * @brief Slot for selection change in the results tree.
     *
     * @param current Model index to specify new selected item.
     * @param previous Model index to specify previous selected item.
     */
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

protected:

    /**
     * @brief Hides/shows full file path on all error file items according to mShowFullPath
     *
     */
    void refreshFilePaths();

    /**
     * @brief Hides/shows full file path on all error file items according to mShowFullPath
     * @param fileItem Parent item whose children's paths to change
     */
    void refreshFilePaths(ResultItem *fileItem);


    /**
     * @brief Removes checking directory from given path if mShowFullPath is false
     *
     * @param path Path to remove checking directory
     * @param saving are we saving? Check mSaveFullPath instead
     * @return Path that has checking directory removed
     */
    QString stripPath(const QString &path, bool saving) const;


    /**
     * @brief Save all errors under specified item
     * @param report Report that errors are saved to
     * @param fileItem Item whose errors to save
     */
    void saveErrors(Report *report, const ResultItem *fileItem) const;

    /**
     * @brief Convert a severity string to a icon filename
     *
     * @param severity Severity
     */
    static QString severityToIcon(Severity severity);

    /**
     * @brief Helper function to open an error within target with application*
     *
     * @param target Error tree item to open
     * @param application Index of the application to open with. Giving -1
     *  (default value) will open the default application.
     */
    void startApplication(const ResultItem *target, int application = -1);

    /**
     * @brief Context menu event (user right clicked on the tree)
     *
     * @param e Event
     */
    void contextMenuEvent(QContextMenuEvent * e) override;

    /**
     * @brief Add a new error item beneath a file or a backtrace item beneath an error
     *
     * @param parent Parent for the item. Either a file item or an error item
     * @param errorItem Error item
     * @param hide Should this be hidden (true) or shown (false)
     * @param icon Should a default backtrace item icon be added
     * @param type type of items to create file/message/note
     * @param errorPathIndex errorPathIndex
     * @return newly created ResultItem *
     */
    ResultItem *addBacktraceFiles(ResultItem *parent,
                                  const QSharedPointer<ErrorItem>& errorItem,
                                  bool hide,
                                  const QString &icon,
                                  ResultItem::Type type,
                                  int errorPathIndex);

    /**
     * @brief Convert Severity to translated string for GUI.
     * @param severity Severity to convert
     * @return Severity as translated string
     */
    static QString severityToTranslatedString(Severity severity);

    /**
     * @brief Load all settings
     * Column widths
     */
    void loadSettings();

    /**
     * @brief Ask directory where file is located.
     * @param file File name.
     * @return Directory user chose.
     */
    QString askFileDir(const QString &file);

    /**
     * @brief Create new normal item.
     *
     * Normal item has left alignment.
     * @param text text for the item
     * @param errorItem errorItem pointer
     * @param type (file/message)
     * @param errorPathIndex error path index
     * @return new ResultItem
     */
    static ResultItem *createNormalItem(const QString &text, QSharedPointer<ErrorItem> errorItem, ResultItem::Type type, int errorPathIndex);

    /**
     * @brief Create filename item.
     *
     * filename item has left alignment. Path is stripped and converted to native path separators.
     * @param errorItem errorItem pointer
     * @param type (file/message)
     * @param errorPathIndex error path index
     *
     * @return new ResultItem
     */
    ResultItem *createFilenameItem(const QSharedPointer<ErrorItem>& errorItem, ResultItem::Type type, int errorPathIndex);

    /**
     * @brief Create new checkbox item.
     *
     * Checkbox item can be checked or unchecked.
     * @param checked checked
     * @param errorItem errorItem pointer
     * @param type (file/message)
     * @param errorPathIndex error path index
     * @return new ResultItem
     */
    static ResultItem *createCheckboxItem(bool checked, QSharedPointer<ErrorItem> errorItem, ResultItem::Type type, int errorPathIndex);

    /**
     * @brief Create new line number item.
     *
     * Line number item has right align.
     * @param linenumber line number
     * @param errorItem errorItem pointer
     * @param type (file/message)
     * @param errorPathIndex error path index
     * @return new ResultItem
     */
    static ResultItem *createLineNumberItem(int linenumber, QSharedPointer<ErrorItem> errorItem, ResultItem::Type type, int errorPathIndex);

    /**
     * @brief Finds a file item
     *
     * @param name name of the file item to find
     * @return pointer to file item or null if none found
     */
    ResultItem *findFileItem(const QString &name) const;

    /**
     * @brief Ensures there's a item in the model for the specified file
     *
     * @param errorItem Error item
     * @param hide is the error (we want this file item for) hidden?
     * @return ResultItem to be used as a parent for all errors for specified file
     */
    ResultItem *ensureFileItem(const QSharedPointer<ErrorItem>& errorItem, bool hide);

    /**
     * @brief Item model for tree
     *
     */
    QStandardItemModel* mModel;

    /**
     * @brief Program settings
     *
     */
    QSettings* mSettings{};

    /**
     * @brief A string used to filter the results for display.
     *
     */
    QString mFilter;

    /**
     * @brief List of applications to open errors with
     *
     */
    ApplicationList* mApplications{};

    /**
     * @brief Right clicked item (used by context menu slots)
     *
     */
    ResultItem* mContextItem{};

    /**
     * @brief Should full path of files be shown (true) or relative (false)
     *
     */
    bool mShowFullPath{};

    /**
     * @brief Should full path of files be saved
     *
     */
    bool mSaveFullPath{};

    /**
     * @brief Save all errors (true) or only visible (false)
     *
     */
    bool mSaveAllErrors = true;

    /**
     * @brief true if optional column "Id" is shown
     *
     */
    bool mShowErrorId{};

    /**
     * @brief Path we are currently checking
     *
     */
    QString mCheckPath;

    /**
     * @brief The type of source of the current results
     *
     */
    ResultsSource mResultsSource{ResultsSource::Analysis};

    /**
     * @brief Are there any visible errors
     *
     */
    bool mVisibleErrors{};

private:
    void keyPressEvent(QKeyEvent *event) override;

    /** tag selected items */
    void tagSelectedItems(const QString &tag);

    bool isCertReport() const;

    bool isAutosarMisraReport() const;

    QStringList mHiddenMessageId;

    // List of existing errors so we can avoid duplicates
    QStringList mErrorList;

    QItemSelectionModel* mSelectionModel{};
    ThreadHandler *mThread{};

    bool mShowCppcheck = true;
    bool mShowClang = true;

    ReportType mReportType = ReportType::normal;

    std::map<std::string, std::string> mGuideline;
};
/// @}
#endif // RESULTSTREE_H
