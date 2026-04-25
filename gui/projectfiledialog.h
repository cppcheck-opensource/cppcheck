/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#ifndef PROJECTFILE_DIALOG_H
#define PROJECTFILE_DIALOG_H

#include "suppressions.h"

#include <QDialog>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

class QModelIndex;
class QWidget;
namespace Ui {
    class ProjectFile;
}

extern const char ADDON_MISRA[];
extern const char CODING_STANDARD_MISRA_C_2023[];
extern const char CODING_STANDARD_MISRA_C_2025[];
extern const char CODING_STANDARD_MISRA_CPP_2008[];
extern const char CODING_STANDARD_MISRA_CPP_2023[];
extern const char CODING_STANDARD_CERT_C[];
extern const char CODING_STANDARD_CERT_CPP[];
extern const char CODING_STANDARD_AUTOSAR[];

/// @addtogroup GUI
/// @{


class ProjectFile;

/**
 * @brief A dialog for editing project file data.
 */
class ProjectFileDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProjectFileDialog(ProjectFile *projectFile, bool premium, QWidget *parent = nullptr);
    ~ProjectFileDialog() override;

private:
    void loadFromProjectFile(const ProjectFile *projectFile);
    void saveToProjectFile(ProjectFile *projectFile) const;

    /** Enable and disable widgets in the 'Paths and Defines' tab */
    void updatePathsAndDefines();

    /**
     * @brief Return project root path from the dialog control.
     * @return Project root path.
     */
    QString getRootPath() const;

    QStringList getProjectConfigurations() const;
    void setProjectConfigurations(const QStringList &configs);

    QString getImportProject() const;

    /** Get Cppcheck build dir */
    QString getBuildDir() const;

    /**
     * @brief Return include paths from the dialog control.
     * @return List of include paths.
     */
    QStringList getIncludePaths() const;

    /**
     * @brief Return define names from the dialog control.
     * @return List of define names.
     */
    QStringList getDefines() const;

    /**
     * @brief Return undefine names from the dialog control.
     * @return List of undefine names.
     */
    QStringList getUndefines() const;

    /**
     * @brief Return check paths from the dialog control.
     * @return List of check paths.
     */
    QStringList getCheckPaths() const;

    /**
     * @brief Return excluded paths from the dialog control.
     * @return List of excluded paths.
     */
    QStringList getExcludedPaths() const;

    /**
     * @brief Return selected libraries from the dialog control.
     * @return List of libraries.
     */
    QStringList getLibraries() const;

    /**
     * @brief Return suppressions from the dialog control.
     * @return List of suppressions.
     */
    const QList<SuppressionList::Suppression>& getSuppressions() const {
        return mSuppressions;
    }

    /**
     * @brief Set project root path to dialog control.
     * @param root Project root path to set to dialog control.
     */
    void setRootPath(const QString &root);

    /** Set build dir */
    void setBuildDir(const QString &buildDir);

    void setImportProject(const QString &importProject);

    /**
     * @brief Set include paths to dialog control.
     * @param includes List of include paths to set to dialog control.
     */
    void setIncludepaths(const QStringList &includes);

    /**
     * @brief Set define names to dialog control.
     * @param defines List of define names to set to dialog control.
     */
    void setDefines(const QStringList &defines);

    /**
     * @brief Set undefine names to dialog control.
     * @param undefines List of undefine names to set to dialog control.
     */
    void setUndefines(const QStringList &undefines);

    /**
     * @brief Set check paths to dialog control.
     * @param paths List of path names to set to dialog control.
     */
    void setCheckPaths(const QStringList &paths);

    /**
     * @brief Set excluded paths to dialog control.
     * @param paths List of path names to set to dialog control.
     */
    void setExcludedPaths(const QStringList &paths);

    /**
     * @brief Set libraries to dialog control.
     * @param libraries List of libraries to set to dialog control.
     */
    void setLibraries(const QStringList &libraries);

    /**
     * @brief Add a single suppression to dialog control.
     * @param suppression A suppressions to add to dialog control.
     */
    void addSingleSuppression(const SuppressionList::Suppression &suppression);

    /**
     * @brief Set suppressions to dialog control.
     * @param suppressions List of suppressions to set to dialog control.
     */
    void setSuppressions(const QList<SuppressionList::Suppression> &suppressions);

private slots:

    /** ok button pressed, save changes and accept */
    void ok();

    /**
     * @brief Browse for build dir.
     */
    void browseBuildDir();

    /**
     * @brief Clear 'import project'.
     */
    void clearImportProject();

    /**
     * @brief Browse for solution / project / compile database.
     */
    void browseImportProject();

    /**
     * @brief Browse for include file
     */
    void browseUserInclude();

    /**
     * @brief Add new path to check.
     */
    void addCheckPath();

    /**
     * @brief Edit path in the list.
     */
    void editCheckPath();

    /**
     * @brief Remove path from the list.
     */
    void removeCheckPath();

    /**
     * @brief Browse for include directory.
     * Allow user to add new include directory to the list.
     */
    void addIncludeDir();

    /**
     * @brief Remove include directory from the list.
     */
    void removeIncludeDir();

    /**
     * @brief Edit include directory in the list.
     */
    void editIncludeDir();

    /**
     * @brief Add new path to exclude list.
     */
    void addExcludePath();

    /**
     * @brief Add new file to exclude list.
     */
    void addExcludeFile();

    /**
     * @brief Edit excluded path in the list.
     */
    void editExcludePath();

    /**
     * @brief Remove excluded path from the list.
     */
    void removeExcludePath();

    /**
     * @brief Move include path up in the list.
     */
    void moveIncludePathUp();

    /**
     * @brief Move include path down in the list.
     */
    void moveIncludePathDown();

    /**
     * @brief Add suppression to the list
     */
    void addSuppression();

    /**
     * @brief Remove selected suppression from the list
     */
    void removeSuppression();

    /**
     * @brief Edit suppression (double clicking on suppression)
     */
    void editSuppression(const QModelIndex& /*index*/);

    /**
     * @brief Browse for misra file
     */
    void browseMisraFile();

    /**
     * @brief Check for all VS configurations
     */
    void checkAllVSConfigs();

protected:

    /**
     * @brief Save dialog settings.
     */
    void loadSettings();

    /**
     * @brief Load dialog settings.
     */
    void saveSettings() const;

    /**
     * @brief Add new indlude directory.
     * @param dir Directory to add.
     */
    void addIncludeDir(const QString &dir);

    /**
     * @brief Add new path to check.
     * @param path Path to add.
     */
    void addCheckPath(const QString &path);

    /**
     * @brief Add new path to ignore list.
     * @param path Path to add.
     */
    void addExcludePath(const QString &path);

    /**
     * @brief Get mSuppressions index that match the
     * given short text
     * @param shortText text as generated by Suppression::getText
     * @return index of matching suppression, -1 if not found
     */
    int getSuppressionIndex(const QString &shortText) const;

private:
    static QStringList getProjectConfigs(const QString &fileName);

    Ui::ProjectFile * const mUI;

    /**
     * @brief Projectfile path.
     */
    ProjectFile * const mProjectFile;

    /** Is this Cppcheck Premium? */
    const bool mPremium;

    QString getExistingDirectory(const QString &caption, bool trailingSlash);

    QList<SuppressionList::Suppression> mSuppressions;
};

/// @}
#endif // PROJECTFILE_DIALOG_H
