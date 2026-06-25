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

#include "testfilelist.h"

#include "filelist.h"

#include <QDir>
#include <QString>
#include <QStringList>
#include <QtTest>

void TestFileList::addFile() const
{
    // Accepted extensions: *.cpp, *.cxx, *.cc, *.c, *.c++, *.txx, *.tpp, *.ipp, *.ixx"
    FileList list;
    list.addFile(QString(SRCDIR) + "/../data/files/foo1.cpp");
    list.addFile(QString(SRCDIR) + "/../data/files/foo2.cxx");
    list.addFile(QString(SRCDIR) + "/../data/files/foo3.cc");
    list.addFile(QString(SRCDIR) + "/../data/files/foo4.c");
    list.addFile(QString(SRCDIR) + "/../data/files/foo5.c++");
    list.addFile(QString(SRCDIR) + "/../data/files/foo6.txx");
    list.addFile(QString(SRCDIR) + "/../data/files/foo7.tpp");
    list.addFile(QString(SRCDIR) + "/../data/files/foo8.ipp");
    list.addFile(QString(SRCDIR) + "/../data/files/foo9.ixx");
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 9);
}

void TestFileList::addPathList() const
{
    // Accepted extensions: *.cpp, *.cxx, *.cc, *.c, *.c++, *.txx, *.tpp, *.ipp, *.ixx"
    QStringList paths;
    paths << QString(SRCDIR) + "/../data/files/foo1.cpp";
    paths << QString(SRCDIR) + "/../data/files/foo2.cxx";
    paths << QString(SRCDIR) + "/../data/files/foo3.cc";
    paths << QString(SRCDIR) + "/../data/files/foo4.c";
    paths << QString(SRCDIR) + "/../data/files/foo5.c++";
    paths << QString(SRCDIR) + "/../data/files/foo6.txx";
    paths << QString(SRCDIR) + "/../data/files/foo7.tpp";
    paths << QString(SRCDIR) + "/../data/files/foo8.ipp";
    paths << QString(SRCDIR) + "/../data/files/foo9.ixx";
    FileList list;
    list.addPathList(paths);
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 9);
}

void TestFileList::addFile_notexist() const
{
    FileList list;
    list.addFile(QString(SRCDIR) + "/../data/files/bar1.cpp");
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 0);
}

void TestFileList::addFile_unknown() const
{
    FileList list;
    list.addFile(QString(SRCDIR) + "/../data/files/bar1");
    list.addFile(QString(SRCDIR) + "/../data/files/bar1.foo");
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 0);
}

void TestFileList::addDirectory() const
{
    FileList list;
    list.addDirectory(QString(SRCDIR) + "/../data/files");
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 9);
}

void TestFileList::addDirectory_recursive() const
{
    FileList list;
    list.addDirectory(QString(SRCDIR) + "/../data/files", true);
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 12);
    QDir dir(QString(SRCDIR) + "/../data/files");
    QString base = dir.canonicalPath();
    QVERIFY(files.contains(base + "/dir1/foo1.cpp"));
    QVERIFY(files.contains(base + "/dir1/dir11/foo11.cpp"));
    QVERIFY(files.contains(base + "/dir2/foo1.cpp"));
}

void TestFileList::filterFiles() const
{
    FileList list;
    QStringList filters;
    filters << "foo1.cpp" << "foo3.cc";
    list.addExcludeList(filters);
    list.addFile(QString(SRCDIR) + "/../data/files/foo1.cpp");
    list.addFile(QString(SRCDIR) + "/../data/files/foo2.cxx");
    list.addFile(QString(SRCDIR) + "/../data/files/foo3.cc");
    list.addFile(QString(SRCDIR) + "/../data/files/foo4.c");
    list.addFile(QString(SRCDIR) + "/../data/files/foo5.c++");
    list.addFile(QString(SRCDIR) + "/../data/files/foo6.txx");
    list.addFile(QString(SRCDIR) + "/../data/files/foo7.tpp");
    list.addFile(QString(SRCDIR) + "/../data/files/foo8.ipp");
    list.addFile(QString(SRCDIR) + "/../data/files/foo9.ixx");
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 7);
    QDir dir(QString(SRCDIR) + "/../data/files");
    QString base = dir.canonicalPath();
    QVERIFY(!files.contains(base + "/foo1.cpp"));
    QVERIFY(!files.contains(base + "/foo3.cpp"));
}

void TestFileList::filterFiles2() const
{
    FileList list;
    QStringList filters;
    filters << "foo1.cpp" << "foo3.cc";
    list.addExcludeList(filters);
    list.addDirectory(QString(SRCDIR) + "/../data/files");
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 7);
    QDir dir(QString(SRCDIR) + "/../data/files");
    QString base = dir.canonicalPath();
    QVERIFY(!files.contains(base + "/foo1.cpp"));
    QVERIFY(!files.contains(base + "/foo3.cpp"));
}

void TestFileList::filterFiles3() const
{
    FileList list;
    QStringList filters;
    filters << "foo1.cpp" << "foo3.cc";
    list.addExcludeList(filters);
    list.addDirectory(QString(SRCDIR) + "/../data/files", true);
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 8);
    QDir dir(QString(SRCDIR) + "/../data/files");
    QString base = dir.canonicalPath();
    QVERIFY(!files.contains(base + "/foo1.cpp"));
    QVERIFY(!files.contains(base + "/foo3.cpp"));
    QVERIFY(!files.contains(base + "/dir1/foo1.cpp"));
    QVERIFY(!files.contains(base + "/dir2/foo1.cpp"));
}

void TestFileList::filterFiles4() const
{
    FileList list;
    QStringList filters;
    filters << "dir1/";
    list.addExcludeList(filters);
    list.addDirectory(QString(SRCDIR) + "/../data/files", true);
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 10);
    QDir dir(QString(SRCDIR) + "/../data/files");
    QString base = dir.canonicalPath();
    QVERIFY(!files.contains(base + "/dir1/foo1.cpp"));
    QVERIFY(!files.contains(base + "/dir1/dir11/foo11.cpp"));
}

void TestFileList::filterFiles5() const
{
    FileList list;
    QStringList filters;
    filters << QDir(QString(SRCDIR) + "/../data/files/dir1/").absolutePath() + "/";
    list.addExcludeList(filters);
    list.addDirectory(QString(SRCDIR) + "/../data/files", true);
    QStringList files = list.getFileList();
    QCOMPARE(files.size(), 10);
    QDir dir(QString(SRCDIR) + "/../data/files");
    QString base = dir.canonicalPath();
    QVERIFY(!files.contains(base + "/dir1/foo1.cpp"));
    QVERIFY(!files.contains(base + "/dir1/dir11/foo11.cpp"));
}

QTEST_MAIN(TestFileList)
