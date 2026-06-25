/*
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

#include "threadresult.h"

#include "common.h"
#include "erroritem.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "importproject.h"

#include <numeric>
#include <utility>

#include <QFile>

void ThreadResult::reportOut(const std::string &outmsg, Color /*c*/)
{
    emit log(QString::fromStdString(outmsg));
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ThreadResult::finishCheck(CheckThread::Details details)
{
    std::lock_guard<std::mutex> locker(mutex);

    mCheckedFileSize += QFile(details.file).size();
    mFilesChecked++;

    if (mTotalFileSize > 0) {
        const int value = static_cast<int>(PROGRESS_MAX * mCheckedFileSize / mTotalFileSize);
        const QString description = tr("%1 of %2 files checked").arg(mFilesChecked).arg(mTotalFiles);

        emit filesCheckedProgress(value, description);
    }
}

void ThreadResult::reportErr(const ErrorMessage &msg)
{
    std::lock_guard<std::mutex> locker(mutex);
    const ErrorItem item(msg);
    if (msg.severity != Severity::debug)
        emit error(item);
    else
        emit debugError(item);
}

// NOLINTNEXTLINE(readability-avoid-const-params-in-decls) - false positive this is an overload
void ThreadResult::reportProgress(const std::string &filename, const char stage[], const std::size_t value) {
    emit progress(QString::fromStdString(filename), stage, value);
}

void ThreadResult::getNextFile(const FileWithDetails*& file)
{
    std::lock_guard<std::mutex> locker(mutex);
    file = nullptr;
    if (mItNextFile == mFiles.cend()) {
        return;
    }
    file = &(*mItNextFile);
    ++mItNextFile;
}

void ThreadResult::getNextFileSettings(const FileSettings*& fs)
{
    std::lock_guard<std::mutex> locker(mutex);
    fs = nullptr;
    if (mItNextFileSettings == mFileSettings.cend()) {
        return;
    }
    fs = &(*mItNextFileSettings);
    ++mItNextFileSettings;
}

void ThreadResult::setFiles(std::list<FileWithDetails> files)
{
    std::lock_guard<std::mutex> locker(mutex);
    mTotalFiles = files.size();
    mFiles = std::move(files);
    mItNextFile = mFiles.cbegin();
    mCheckedFileSize = 0;
    mFilesChecked = 0;

    // Determine the total size of all of the files to check, so that we can
    // show an accurate progress estimate
    quint64 sizeOfFiles = std::accumulate(mFiles.cbegin(), mFiles.cend(), 0, [](quint64 total, const FileWithDetails& file) {
        return total + file.size();
    });
    mTotalFileSize = sizeOfFiles;
}

void ThreadResult::setProject(const ImportProject &prj)
{
    std::lock_guard<std::mutex> locker(mutex);
    mFiles.clear();
    mItNextFile = mFiles.cbegin();
    mFileSettings = prj.fileSettings;
    mItNextFileSettings = mFileSettings.cbegin();
    mCheckedFileSize = 0;
    mFilesChecked = 0;
    mTotalFiles = prj.fileSettings.size();

    // Determine the total size of all of the files to check, so that we can
    // show an accurate progress estimate
    mTotalFileSize = std::accumulate(prj.fileSettings.begin(), prj.fileSettings.end(), quint64{ 0 }, [](quint64 v, const FileSettings& fs) {
        return v + QFile(QString::fromStdString(fs.filename())).size();
    });
}

void ThreadResult::clearFiles()
{
    std::lock_guard<std::mutex> locker(mutex);
    mFiles.clear();
    mFileSettings.clear();
    mItNextFileSettings = mFileSettings.cend();
    mFilesChecked = 0;
    mTotalFiles = 0;
}

int ThreadResult::getFileCount() const
{
    std::lock_guard<std::mutex> locker(mutex);
    return mFiles.size() + mFileSettings.size();
}
