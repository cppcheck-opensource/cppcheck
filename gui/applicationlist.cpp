/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "applicationlist.h"

#include "application.h"
#include "common.h"

#include <QFileInfo>
#include <QSettings>
#include <QStringList>
#include <QVariant>

ApplicationList::ApplicationList(QObject *parent) :
    QObject(parent)
{
    //ctor
}

ApplicationList::~ApplicationList()
{
    clear();
}

bool ApplicationList::loadSettings()
{
    QSettings settings;
    QStringList names = settings.value(SETTINGS_APPLICATION_NAMES, QStringList()).toStringList();
    QStringList paths = settings.value(SETTINGS_APPLICATION_PATHS, QStringList()).toStringList();
    QStringList params = settings.value(SETTINGS_APPLICATION_PARAMS, QStringList()).toStringList();
    int defapp = settings.value(SETTINGS_APPLICATION_DEFAULT, -1).toInt();

    // Params will be empty first time starting with the new setting.
    // Return false and inform user about problem with application settings.
    bool succeeded = true;
    if (!names.empty() && !paths.empty() && params.empty()) {
        for (int i = 0; i < paths.length(); i++)
            params << QString();
        succeeded = false;
    }

    if (names.empty() && paths.empty() && params.empty()) {
#ifndef _WIN32
        // use as default for gnome environments
        if (QFileInfo("/usr/bin/gedit").isExecutable()) {
            Application app;
            app.setName("gedit");
            app.setPath("/usr/bin/gedit");
            app.setParameters("+(line) (file)");
            addApplication(app);
            defapp = 0;
        }
        checkAndAddApplication("/usr/bin/geany","geany","+(line) (file)");
        checkAndAddApplication("/usr/bin/qtcreator","Qt Creator","-client (file):(line)");
        // use as default for kde environments
        if (QFileInfo("/usr/bin/kate").isExecutable()) {
            Application app;
            app.setName("kate");
            app.setPath("/usr/bin/kate");
            app.setParameters("-l(line) (file)");
            addApplication(app);
            defapp = 0;
        }
#else
        if (findDefaultWindowsEditor()) {
            defapp = 0;
        }
#endif
    } else if (names.size() == paths.size()) {
        for (int i = 0; i < names.size(); i++) {
            const Application app(names[i], paths[i], params[i]);
            addApplication(app);
        }
    }

    if (defapp == -1)
        mDefaultApplicationIndex = 0;
    else if (defapp < names.size())
        mDefaultApplicationIndex = defapp;
    else
        mDefaultApplicationIndex = 0;

    return succeeded;
}

void ApplicationList::saveSettings() const
{
    QSettings settings;
    QStringList names;
    QStringList paths;
    QStringList params;

    for (int i = 0; i < getApplicationCount(); i++) {
        const Application& app = getApplication(i);
        names << app.getName();
        paths << app.getPath();
        params << app.getParameters();
    }

    settings.setValue(SETTINGS_APPLICATION_NAMES, names);
    settings.setValue(SETTINGS_APPLICATION_PATHS, paths);
    settings.setValue(SETTINGS_APPLICATION_PARAMS, params);
    settings.setValue(SETTINGS_APPLICATION_DEFAULT, mDefaultApplicationIndex);
}

int ApplicationList::getApplicationCount() const
{
    return mApplications.size();
}

Application& ApplicationList::getApplication(const int index)
{
    if (index >= 0 && index < mApplications.size()) {
        return mApplications[index];
    }

    static Application dummy; // TODO: Throw exception instead?
    return dummy;
}

const Application& ApplicationList::getApplication(const int index) const
{
    if (index >= 0 && index < mApplications.size()) {
        return mApplications[index];
    }

    static const Application dummy; // TODO: Throw exception instead?
    return dummy;
}

void ApplicationList::addApplication(const Application &app)
{
    if (app.getName().isEmpty() || app.getPath().isEmpty()) {
        return;
    }
    mApplications << app;
}

void ApplicationList::removeApplication(const int index)
{
    mApplications.removeAt(index);
}

void ApplicationList::setDefault(const int index)
{
    if (index < mApplications.size() && index >= 0) {
        mDefaultApplicationIndex = index;
    }
}

void ApplicationList::copy(const ApplicationList *list)
{
    if (!list) {
        return;
    }

    clear();
    for (int i = 0; i < list->getApplicationCount(); i++) {
        const Application& app = list->getApplication(i);
        addApplication(app);
    }
    mDefaultApplicationIndex = list->getDefaultApplication();
}

void ApplicationList::clear()
{
    mApplications.clear();
    mDefaultApplicationIndex = -1;
}

bool ApplicationList::checkAndAddApplication(const QString& appPath, const QString& name, const QString& parameters)
{
    if (QFileInfo::exists(appPath) && QFileInfo(appPath).isExecutable()) {
        Application app;
        app.setName(name);
        app.setPath("\"" + appPath + "\"");
        app.setParameters(parameters);
        addApplication(app);
        return true;
    }
    return false;
}

#ifdef _WIN32
bool ApplicationList::findDefaultWindowsEditor()
{
    bool foundOne = false;
#ifdef WIN64 // As long as we do support 32-bit XP, we cannot be sure that the environment variable "ProgramFiles(x86)" exists
    const QString appPathx86(getenv("ProgramFiles(x86)"));
#else
    const QString appPathx86(getenv("ProgramFiles"));
#endif
    const QString appPathx64(getenv("ProgramW6432"));
    const QString windowsPath(getenv("windir"));

    if (checkAndAddApplication(appPathx86 + "\\Notepad++\\notepad++.exe", "Notepad++", "-n(line) (file)"))
        foundOne = true;
    else if (checkAndAddApplication(appPathx64 + "\\Notepad++\\notepad++.exe", "Notepad++", "-n(line) (file)"))
        foundOne = true;

    if (checkAndAddApplication(appPathx86 + "\\Notepad2\\Notepad2.exe", "Notepad2", "/g (line) (file)"))
        foundOne = true;
    else if (checkAndAddApplication(appPathx64 + "\\Notepad2\\Notepad2.exe", "Notepad2", "/g (line) (file)"))
        foundOne = true;

    if (checkAndAddApplication(windowsPath + "\\system32\\notepad.exe", "Notepad", "(file)"))
        foundOne = true;

    QString regPath = "HKEY_CLASSES_ROOT\\Applications\\QtProject.QtCreator.pro\\shell\\Open\\command";
    QSettings registry(regPath, QSettings::NativeFormat);
    QString qtCreatorRegistry = registry.value("Default", QString()).toString();
    QString qtCreatorPath = qtCreatorRegistry.left(qtCreatorRegistry.indexOf(".exe") + 4);
    if (!qtCreatorRegistry.isEmpty() && checkAndAddApplication(qtCreatorPath, "Qt Creator", "-client (file):(line)")) {
        foundOne = true;
    }

    const QString regPathUEdit32 = "HKEY_CLASSES_ROOT\\Applications\\Uedit32.exe\\shell\\open\\Command";
    const QSettings registryUEdit32(regPathUEdit32, QSettings::NativeFormat);
    const QString uedit32Registry = registryUEdit32.value("Default", QString()).toString();
    if (!uedit32Registry.isEmpty()) {
        // Extract path to executable and make sure there is no single quotation mark at the beginning
        const QString uedit32Path = uedit32Registry.left(uedit32Registry.indexOf(".exe") + 4).replace("\"", "");
        if (checkAndAddApplication(uedit32Path, "UltraEdit 32", "(file)/(line)")) {
            foundOne = true;
        }
    }

    const QString regPathUEdit64 = "HKEY_CLASSES_ROOT\\Applications\\uedit64.exe\\shell\\open\\Command";
    const QSettings registryUEdit64(regPathUEdit64, QSettings::NativeFormat);
    const QString uedit64Registry = registryUEdit64.value("Default", QString()).toString();
    if (!uedit64Registry.isEmpty()) {
        // Extract path to executable and make sure there is no single quotation mark at the beginning
        const QString uedit64Path = uedit64Registry.left(uedit64Registry.indexOf(".exe") + 4).replace("\"", "");
        if (checkAndAddApplication(uedit64Path, "UltraEdit 64", "(file)/(line)")) {
            foundOne = true;
        }
    }

    const QString regPathMSVSCode = "HKEY_CLASSES_ROOT\\Applications\\Code.exe\\shell\\open\\command";
    const QSettings registryMSVSCode(regPathMSVSCode, QSettings::NativeFormat);
    const QString msvscodeRegistry = registryMSVSCode.value("Default", QString()).toString();
    if (!msvscodeRegistry.isEmpty()) {
        const QString msvscodePath = msvscodeRegistry.left(msvscodeRegistry.indexOf(".exe") + 4).replace("\"", "");
        if (checkAndAddApplication(msvscodePath, "Microsoft VS Code", "-g (file):(line)")) {
            foundOne = true;
        }
    }

    return foundOne;
}
#endif
