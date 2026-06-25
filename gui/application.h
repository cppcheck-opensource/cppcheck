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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QString>

/**
 * @brief A class containing information of the application to execute.
 *
 * Each application has a name and a path. Name is displayed to the user
 * and has no other meaning. It isn't used to start the application.
 * Path contains the full path to the application containing the executable name.
 * Parameters contains the command line arguments for the executable.
 *
 * User can also specify certain predefined strings to parameters. These strings
 * will be replaced with appropriate values concerning the error. Strings are:
 * (file) - Filename containing the error
 * (line) - Line number containing the error
 * (message) - Error message
 * (severity) - Error severity
 *
 * Example opening a file with Kate and make Kate scroll to the correct line.
 * Executable: kate
 * Parameters: -l(line) (file)
 */
class Application {
public:
    Application() = default;
    Application(QString name, QString path, QString params);

    /**
     * @brief Get application name.
     * @return Application name.
     */
    const QString& getName() const {
        return mName;
    }

    /**
     * @brief Get application path.
     * @return Application path.
     */
    const QString& getPath() const {
        return mPath;
    }

    /**
     * @brief Get application command line parameters.
     * @return Application command line parameters.
     */
    const QString& getParameters() const {
        return mParameters;
    }

    /**
     * @brief Set application name.
     * @param name Application name.
     */
    void setName(const QString &name) {
        mName = name;
    }

    /**
     * @brief Set application path.
     * @param path Application path.
     */
    void setPath(const QString &path) {
        mPath = path;
    }

    /**
     * @brief Set application command line parameters.
     * @param parameters Application command line parameters.
     */
    void setParameters(const QString &parameters) {
        mParameters = parameters;
    }

private:

    /**
     * @brief Application's name
     */
    QString mName;

    /**
     * @brief Application's path
     */
    QString mPath;

    /**
     * @brief Application's parameters
     */
    QString mParameters;
};

#endif // APPLICATION_H
