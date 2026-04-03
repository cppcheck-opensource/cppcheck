/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#include "testprojectfile.h"

#include "library.h"
#include "platform.h"
#include "projectfile.h"
#include "settings.h"
#include "suppressions.h"

#include <QFile>
#include <QIODevice>
#include <QList>
#include <QStringList>
#include <QTemporaryDir>
#include <QtTest>

// Mock...
const char Settings::SafeChecks::XmlRootName[] = "safe-checks";
const char Settings::SafeChecks::XmlClasses[] = "class-public";
const char Settings::SafeChecks::XmlExternalFunctions[] = "external-functions";
const char Settings::SafeChecks::XmlInternalFunctions[] = "internal-functions";
const char Settings::SafeChecks::XmlExternalVariables[] = "external-variables";
Settings::Settings() : executor(ExecutorType::Thread), pid(0) {}
Platform::Platform() = default;
Library::Library() = default;
Library::~Library() = default;
struct Library::LibraryData {};

void TestProjectFile::loadInexisting() const
{
    const QString filepath(QString(SRCDIR) + "/../data/projectfiles/foo.cppcheck");
    ProjectFile pfile(filepath);
    QCOMPARE(pfile.read(), false);
}

void TestProjectFile::loadSimple() const
{
    const QString filepath(QString(SRCDIR) + "/../data/projectfiles/simple.cppcheck");
    ProjectFile pfile(filepath);
    QVERIFY(pfile.read());
    QCOMPARE(pfile.getRootPath(), QString("../.."));
    QStringList includes = pfile.getIncludeDirs();
    QCOMPARE(includes.size(), 2);
    QCOMPARE(includes[0], QString("lib/"));
    QCOMPARE(includes[1], QString("cli/"));
    QStringList paths = pfile.getCheckPaths();
    QCOMPARE(paths.size(), 2);
    QCOMPARE(paths[0], QString("gui/"));
    QCOMPARE(paths[1], QString("test/"));
    QStringList excludes = pfile.getExcludedPaths();
    QCOMPARE(excludes.size(), 1);
    QCOMPARE(excludes[0], QString("gui/temp/"));
    QStringList defines = pfile.getDefines();
    QCOMPARE(defines.size(), 1);
    QCOMPARE(defines[0], QString("FOO"));
}

// Test that project file with old 'ignore' element works
void TestProjectFile::loadSimpleWithIgnore() const
{
    const QString filepath(QString(SRCDIR) + "/../data/projectfiles/simple_ignore.cppcheck");
    ProjectFile pfile(filepath);
    QVERIFY(pfile.read());
    QCOMPARE(pfile.getRootPath(), QString("../.."));
    QStringList includes = pfile.getIncludeDirs();
    QCOMPARE(includes.size(), 2);
    QCOMPARE(includes[0], QString("lib/"));
    QCOMPARE(includes[1], QString("cli/"));
    QStringList paths = pfile.getCheckPaths();
    QCOMPARE(paths.size(), 2);
    QCOMPARE(paths[0], QString("gui/"));
    QCOMPARE(paths[1], QString("test/"));
    QStringList excludes = pfile.getExcludedPaths();
    QCOMPARE(excludes.size(), 1);
    QCOMPARE(excludes[0], QString("gui/temp/"));
    QStringList defines = pfile.getDefines();
    QCOMPARE(defines.size(), 1);
    QCOMPARE(defines[0], QString("FOO"));
}

void TestProjectFile::loadSimpleNoroot() const
{
    const QString filepath(QString(SRCDIR) + "/../data/projectfiles/simple_noroot.cppcheck");
    ProjectFile pfile(filepath);
    QVERIFY(pfile.read());
    QCOMPARE(pfile.getRootPath(), QString());
    QStringList includes = pfile.getIncludeDirs();
    QCOMPARE(includes.size(), 2);
    QCOMPARE(includes[0], QString("lib/"));
    QCOMPARE(includes[1], QString("cli/"));
    QStringList paths = pfile.getCheckPaths();
    QCOMPARE(paths.size(), 2);
    QCOMPARE(paths[0], QString("gui/"));
    QCOMPARE(paths[1], QString("test/"));
    QStringList excludes = pfile.getExcludedPaths();
    QCOMPARE(excludes.size(), 1);
    QCOMPARE(excludes[0], QString("gui/temp/"));
    QStringList defines = pfile.getDefines();
    QCOMPARE(defines.size(), 1);
    QCOMPARE(defines[0], QString("FOO"));
}

void TestProjectFile::getAddonFilePath() const
{
    QTemporaryDir tempdir;
    QVERIFY(tempdir.isValid());
    const QString filepath(tempdir.path() + "/addon.py");

    QFile file(filepath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.close();

    // Relative path to addon
    QCOMPARE(ProjectFile::getAddonFilePath(tempdir.path(), "addon"), filepath);
    QCOMPARE(ProjectFile::getAddonFilePath(tempdir.path(), "not exist"), QString());

    // Absolute path to addon
    QCOMPARE(ProjectFile::getAddonFilePath("/not/exist", filepath), filepath);
    QCOMPARE(ProjectFile::getAddonFilePath(tempdir.path(), filepath), filepath);
}

void TestProjectFile::getSearchPaths() const
{
#ifdef FILESDIR
    const QString f(FILESDIR "\n"        // example: "/usr/local/share/cppcheck\n"
                    FILESDIR "/dir\n");  // example: "/usr/local/share/cppcheck/dir\n"
#else
    const QString f;
#endif

    QCOMPARE(ProjectFile::getSearchPaths("projectPath", "appPath", "datadir", "dir").join("\n"),
             "appPath\n"
             "appPath/dir\n"
             "projectPath\n" +
             f +
             "datadir\n"
             "datadir/dir");
}

void TestProjectFile::getInlineSuppressionDefaultValue() const
{
    ProjectFile projectFile;
    projectFile.setFilename("/some/path/123.cppcheck");
    QCOMPARE(projectFile.getInlineSuppression(), true);
}

void TestProjectFile::getInlineSuppression() const
{
    ProjectFile projectFile;
    projectFile.setFilename("/some/path/123.cppcheck");
    projectFile.setInlineSuppression(false);
    QCOMPARE(projectFile.getInlineSuppression(), false);
}

void TestProjectFile::getCheckingSuppressionsRelative() const
{
    const SuppressionList::Suppression suppression("*", "externals/*");
    const QList<SuppressionList::Suppression> suppressions{suppression};
    ProjectFile projectFile;
    projectFile.setFilename("/some/path/123.cppcheck");
    projectFile.setSuppressions(suppressions);
    QCOMPARE(projectFile.getCheckingSuppressions()[0].fileName, "/some/path/externals/*");
}

void TestProjectFile::getCheckingSuppressionsAbsolute() const
{
    const SuppressionList::Suppression suppression("*", "/some/path/1.h");
    const QList<SuppressionList::Suppression> suppressions{suppression};
    ProjectFile projectFile;
    projectFile.setFilename("/other/123.cppcheck");
    projectFile.setSuppressions(suppressions);
    QCOMPARE(projectFile.getCheckingSuppressions()[0].fileName, "/some/path/1.h");
}

void TestProjectFile::getCheckingSuppressionsStar() const
{
    const SuppressionList::Suppression suppression("*", "*.cpp");
    const QList<SuppressionList::Suppression> suppressions{suppression};
    ProjectFile projectFile;
    projectFile.setFilename("/some/path/123.cppcheck");
    projectFile.setSuppressions(suppressions);
    QCOMPARE(projectFile.getCheckingSuppressions()[0].fileName, "*.cpp");
}

QTEST_MAIN(TestProjectFile)

