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

#include "cppchecklibrarydata.h"

#include <QObject>
#include <QString>

class TestCppcheckLibraryData : public QObject {
    Q_OBJECT

private slots:
    void init();

    void xmlReaderError();
    void unhandledElement();
    void mandatoryAttributeMissing();

    void podtypeValid();
    void typechecksValid();
    void smartPointerValid();
    void platformTypeValid();
    void memoryResourceValid();
    void defineValid();
    void undefineValid();
    void reflectionValid();
    void markupValid();
    void containerValid();

    void validateAllCfg();

private:
    static void loadCfgFile(const QString &filename, CppcheckLibraryData &data, QString &res, bool removeFile = false);
    static void saveCfgFile(const QString &filename, const CppcheckLibraryData &data);

    // cppcheck-suppress naming-privateMemberVariable - TODO: fix this
    CppcheckLibraryData libraryData;
    // cppcheck-suppress naming-privateMemberVariable - TODO: fix this
    CppcheckLibraryData fileLibraryData;
    // cppcheck-suppress naming-privateMemberVariable - TODO: fix this
    QString result;

    // cppcheck-suppress naming-privateMemberVariable - TODO: fix this
    static const QString TempCfgFile;
};
