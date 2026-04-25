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


#ifndef RESULTSVIEW_H
#define RESULTSVIEW_H

#include "report.h"
#include "resultstree.h"
#include "showtypes.h"

#include <cstdint>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>

class ErrorItem;
class Settings;
class ApplicationList;
class ThreadHandler;
class QPrinter;
class QSettings;
class CheckStatistics;
class QPoint;
class ResultItem;
enum class ReportType : std::uint8_t;
namespace Ui {
    class ResultsView;
}

/// @addtogroup GUI
/// @{

/**
 * @brief Widget to show cppcheck progressbar and result
 *
 */
class ResultsView : public QWidget {
    Q_OBJECT
public:

    explicit ResultsView(QWidget * parent = nullptr);
    void initialize(QSettings *settings, ApplicationList *list, ThreadHandler *checkThreadHandler);
    ResultsView(const ResultsView &) = delete;
    ~ResultsView() override;
    ResultsView &operator=(const ResultsView &) = delete;

    /**
     * @brief Clear results and statistics and reset progressinfo.
     * @param results Remove all the results from view?
     */
    void clear(bool results);

    /**
     * @brief Remove a file from the results.
     */
    void clear(const QString &filename);

    /**
     * @brief Remove a recheck file from the results.
     */
    void clearRecheckFile(const QString &filename);

    /**
     * @brief Write statistics in file
     *
     * @param filename Filename to save statistics to
     */
    void saveStatistics(const QString &filename) const;

    /**
     * @brief Save results to a file
     *
     * @param filename Filename to save results to
     * @param type Type of the report.
     * @param productName Custom product name
     */
    void save(const QString &filename, Report::Type type, const QString& productName) const;

    /**
     * @brief Update results from old report (tag, sinceDate)
     */
    void updateFromOldReport(const QString &filename) const;

    /**
     * @brief Update tree settings
     *
     * @param showFullPath Show full path of files in the tree
     * @param saveFullPath Save full path of files in reports
     * @param saveAllErrors Save all visible errors
     * @param showNoErrorsMessage Show "no errors"?
     * @param showErrorId Show error id?
     * @param showInconclusive Show inconclusive?
     */
    void updateSettings(bool showFullPath,
                        bool saveFullPath,
                        bool saveAllErrors,
                        bool showNoErrorsMessage,
                        bool showErrorId,
                        bool showInconclusive);

    /**
     * @brief Update Code Editor Style
     *
     * Function will read updated Code Editor styling from
     * stored program settings.
     *
     * @param settings Pointer to QSettings Object
     */
    void updateStyleSetting(QSettings *settings);

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

    QString getCheckDirectory() const;

    /**
     * Set settings used in checking
     */
    void setCheckSettings(const Settings& settings);

    /**
     * @brief Inform the view that checking has started
     *
     * @param count Count of files to be checked.
     */
    void checkingStarted(int count);

    /**
     * @brief Inform the view that checking finished.
     *
     */
    void checkingFinished();

    /**
     * @brief Do we have visible results to show?
     *
     * @return true if there is at least one warning/error to show.
     */
    bool hasVisibleResults() const;

    /**
     * @brief Do we have results from check?
     *
     * @return true if there is at least one warning/error, hidden or visible.
     */
    bool hasResults() const;

    /**
     * @brief Save View's settings
     *
     * @param settings program settings.
     */
    void saveSettings(QSettings *settings);

    /**
     * @brief Translate this view
     *
     */
    void translate();

    /**
     * @brief This function should be called when analysis is stopped
     */
    void stopAnalysis();

    /**
     * @brief Are there successful results?
     * @return true if analysis finished without critical errors etc
     */
    bool isSuccess() const;

    /**
     * @brief Read errors from report XML file.
     * @param filename Report file to read.
     *
     */
    void readErrorsXml(const QString &filename);

    /**
     * @brief Return checking statistics.
     * @return Pointer to checking statistics.
     */
    const CheckStatistics *getStatistics() const {
        return mStatistics;
    }

    /**
     * @brief Return Showtypes.
     * @return Pointer to Showtypes.
     */
    const ShowTypes & getShowTypes() const;

    void setReportType(ReportType reportType);

    /**
     * @brief Set the results source type for the results tree.
     *
     * @param source The results source type.
     */
    void setResultsSource(ResultsTree::ResultsSource source);

signals:

    /**
     * @brief Signal to be emitted when we have results
     *
     */
    void gotResults();

    /**
     * @brief Signal that results have been hidden or shown
     *
     * @param hidden true if there are some hidden results, or false if there are not
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void resultsHidden(bool hidden);

    /**
     * @brief Signal to perform recheck of selected files
     *
     * @param selectedFilesList list of selected files
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void checkSelected(QStringList selectedFilesList);

    /** Suppress Ids */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void suppressIds(QStringList ids);

    /**
     * @brief Show/hide certain type of errors
     * Refreshes the tree.
     *
     * @param type Type of error to show/hide
     * @param show Should specified errors be shown (true) or hidden (false)
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void showResults(ShowTypes::ShowType type, bool show);

    /**
     * @brief Show/hide cppcheck errors.
     * Refreshes the tree.
     *
     * @param show Should specified errors be shown (true) or hidden (false)
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void showCppcheckResults(bool show);

    /**
     * @brief Show/hide clang-tidy/clang-analyzer errors.
     * Refreshes the tree.
     *
     * @param show Should specified errors be shown (true) or hidden (false)
     */
    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
    void showClangResults(bool show);

    /**
     * @brief Collapse all results in the result list.
     */
    void collapseAllResults();

    /**
     * @brief Expand all results in the result list.
     */
    void expandAllResults();

    /**
     * @brief Show hidden results in the result list.
     */
    void showHiddenResults();

public slots:

    /**
     * @brief Slot for updating the checking progress
     *
     * @param value Current progress value
     * @param description Description to accompany the progress
     */
    void filesCheckedProgress(int value, const QString& description);

    /**
     * @brief Slot for new error to be displayed
     *
     * @param item Error data
     */
    void error(const ErrorItem &item);

    /**
     * @brief Filters the results in the result list.
     */
    void filterResults(const QString& filter);

    /**
     * @brief Update detailed message when selected item is changed.
     *
     * @param item selected item
     */
    void updateDetails(const ResultItem* item);

    /**
     * @brief Slot opening a print dialog to print the current report
     */
    void print();

    /**
     * @brief Slot printing the current report to the printer.
     * @param printer The printer used for printing the report.
     */
    void print(QPrinter* printer) const;

    /**
     * @brief Slot opening a print preview dialog
     */
    void printPreview();

    /**
     * \brief Log message
     */
    void log(const QString &str);

    /**
     * \brief debug message
     */
    void debugError(const ErrorItem &item);

    /**
     * \brief Clear log messages
     */
    void logClear();

    /**
     * \brief Copy selected log message entry
     */
    void logCopyEntry();

    /**
     * \brief Copy all log messages
     */
    void logCopyComplete();

private:

    /**
     * If provided ErrorItem is a critical error then display warning message
     * in the resultsview
     */
    void handleCriticalError(const ErrorItem& item);

    /**
     * @brief Should we show a "No errors found dialog" every time no errors were found?
     */
    bool mShowNoErrorsMessage = true;

    Ui::ResultsView *mUI;

    CheckStatistics *mStatistics;

    Settings* mCheckSettings = nullptr;

    /** Critical error ids */
    QString mCriticalErrors;

private slots:
    /**
     * @brief Custom context menu for Analysis Log
     * @param pos Mouse click position
     */
    void on_mListLog_customContextMenuRequested(const QPoint &pos);
};
/// @}
#endif // RESULTSVIEW_H
