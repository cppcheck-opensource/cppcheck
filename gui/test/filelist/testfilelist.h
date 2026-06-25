/* -*- C++ -*-
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

#include <QObject>

class TestFileList : public QObject {
    Q_OBJECT

private slots:
    void addFile() const;
    void addPathList() const;
    void addFile_notexist() const;
    void addFile_unknown() const;
    void addDirectory() const;
    void addDirectory_recursive() const;
    void filterFiles() const;
    void filterFiles2() const;
    void filterFiles3() const;
    void filterFiles4() const;
    void filterFiles5() const;
};
