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

#include "report.h"

#include <utility>

#include <QIODevice>

Report::Report(QString filename) :
    mFilename(std::move(filename))
{}

Report::~Report()
{
    close();
}

bool Report::create()
{
    bool succeed = false;
    if (!mFile.isOpen()) {
        mFile.setFileName(mFilename);
        succeed = mFile.open(QIODevice::WriteOnly | QIODevice::Text);
    }
    return succeed;
}

bool Report::open()
{
    bool succeed = false;
    if (!mFile.isOpen()) {
        mFile.setFileName(mFilename);
        succeed = mFile.open(QIODevice::ReadOnly | QIODevice::Text);
    }
    return succeed;
}

void Report::close()
{
    if (mFile.isOpen())
        mFile.close();
}

QFile* Report::getFile()
{
    return &mFile;
}
