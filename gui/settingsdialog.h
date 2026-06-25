/* -*- C++ -*-
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


#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QObject>
#include <QString>
#include <Qt>

class QSettings;
class QWidget;
class ApplicationList;
class TranslationHandler;
class CodeEditorStyle;
class QCheckBox;
namespace Ui {
    class Settings;
}

/// @addtogroup GUI
/// @{

/**
 * @brief Settings dialog
 *
 */
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(ApplicationList *list,
                   TranslationHandler *translator,
                   bool premium,
                   QWidget *parent = nullptr);
    SettingsDialog(const SettingsDialog &) = delete;
    ~SettingsDialog() override;
    SettingsDialog &operator=(const SettingsDialog &) = delete;

    /**
     * @brief Save all values to QSettings
     *
     */
    void saveSettingValues() const;

    /**
     * @brief Get checkbox value for mShowFullPath
     *
     * @return should full path of errors be shown in the tree
     */
    bool showFullPath() const;

    /**
     * @brief Get checkbox value for mSaveFullPath
     *
     * @return should full path of files be saved when creating a report
     */
    bool saveFullPath() const;


    /**
     * @brief Get checkbox value for mNoErrorsMessage
     *
     * @return Should "no errors message" be hidden
     */
    bool showNoErrorsMessage() const;

    /**
     * @brief Get checkbox value for mShowIdColumn
     *
     * @return Should error id column be displayed
     */
    bool showErrorId() const;


    /**
     * @brief Get checkbox value for mEnableInconclusive
     *
     * @return Should inconclusive column be displayed
     */
    bool showInconclusive() const;

    /**
     * @brief Get checkbox value for mSaveAllErrors
     *
     * @return should all errors be saved to report
     */
    bool saveAllErrors() const;

protected slots:
    /**
     * @brief Slot for clicking OK.
     *
     */
    void ok();

    /** @brief Slot for validating input value in @c editPythonPath */
    void validateEditPythonPath();

    /**
     * @brief Slot for adding a new application to the list
     *
     */
    void addApplication();

    /**
     * @brief Slot for deleting an application from the list
     *
     */
    void removeApplication();

    /**
     * @brief Slot for modifying an application in the list
     *
     */
    void editApplication();

    /**
     * @brief Slot for making the selected application as the default (first)
     *
     */
    void defaultApplication();

    /** @brief Slot for browsing for the python binary */
    void browsePythonPath();

    /** @brief Slot for browsing for the clang binary */
    void browseClangPath();

    /**
     * @brief Browse for MISRA file
     */
    void browseMisraFile();

    /**
     * @brief Set Code Editor Style to Default
     */
    void setCodeEditorStyleDefault();

    /**
     * @brief Edit Custom Code Editor Style
     */
    void editCodeEditorStyle();

protected:
    /**
     * @brief Clear all applications from the list and re insert them from mTempApplications
     *
     */
    void populateApplicationList();

    /**
     * @brief Load saved values
     * Loads dialog size and column widths.
     *
     */
    void loadSettings();

    /**
     * @brief Save settings
     * Save dialog size and column widths.
     */
    void saveSettings() const;

    /**
     * @brief Save a single checkboxes value
     *
     * @param settings Pointer to Settings.
     * @param box checkbox to save
     * @param name name for QSettings to store the value
     */
    static void saveCheckboxValue(QSettings *settings, const QCheckBox *box, const QString &name);

    /**
     * @brief Convert bool to Qt::CheckState
     *
     * @param yes value to convert
     * @return value converted to Qt::CheckState
     */
    static Qt::CheckState boolToCheckState(bool yes);

    /**
     * @brief Converts Qt::CheckState to bool
     *
     * @param state Qt::CheckState to convert
     * @return converted value
     */
    static bool checkStateToBool(Qt::CheckState state);

    /**
     * @brief Populate the translations list.
     */
    void initTranslationsList();

    /**
     * @brief Current Code Editor Style
     */
    CodeEditorStyle *mCurrentStyle;

    /**
     * @brief List of applications user has specified
     *
     */
    ApplicationList *mApplications;

    /**
     * @brief Temporary list of applications
     * This will be copied to actual list of applications (mApplications)
     * when user clicks ok.
     */
    ApplicationList *mTempApplications;

    /**
     * @brief List of translations.
     *
     */
    TranslationHandler *mTranslator;

    /**
     * @brief Dialog from UI designer
     *
     */
    Ui::Settings *mUI;
private:
    void manageStyleControls();

    static constexpr int mLangCodeRole = Qt::UserRole;

    bool mPremium;
};
/// @}
#endif // SETTINGSDIALOG_H
