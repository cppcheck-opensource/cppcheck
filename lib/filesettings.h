/* -*- C++ -*-
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

#ifndef fileSettingsH
#define fileSettingsH

#include "config.h"
#include "path.h"
#include "platform.h"
#include "standards.h"

#include <list>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

class FileWithDetails
{
public:
    /**
     * @throws std::runtime_error thrown if given path is empty
     */
    FileWithDetails(std::string path, Standards::Language lang, std::size_t size)
        : mLang(lang)
        , mSize(size)
    {
        setPath(std::move(path));
        if (mPath.empty())
            throw std::runtime_error("empty path specified");
    }

    void setPath(std::string path)
    {
        mPath = std::move(path);
        mPathSimplified = Path::simplifyPath(mPath);
        mPathAbsolute.clear();
    }

    const std::string& path() const
    {
        return mPath;
    }

    const std::string& spath() const
    {
        return mPathSimplified;
    }

    const std::string& abspath() const
    {
        // use delayed resolution as it will fail for files which do not exist
        if (mPathAbsolute.empty())
            mPathAbsolute = Path::getAbsoluteFilePath(mPath);
        return mPathAbsolute;
    }

    std::size_t size() const
    {
        return mSize;
    }

    void setLang(Standards::Language lang)
    {
        mLang = lang;
    }

    Standards::Language lang() const
    {
        return mLang;
    }

    std::size_t fsFileId() const
    {
        return mFsFileId;
    }

    void setFsFileId(std::size_t fsFileId)
    {
        mFsFileId = fsFileId;
    }
private:
    std::string mPath;
    std::string mPathSimplified;
    mutable std::string mPathAbsolute;
    Standards::Language mLang = Standards::Language::None;
    std::size_t mSize;
    std::size_t mFsFileId{0};
};

/** File settings. Multiple configurations for a file is allowed. */
struct CPPCHECKLIB FileSettings {
    FileSettings(std::string path, Standards::Language lang, std::size_t size)
        : file(std::move(path), lang, size)
    {}

    std::string cfg;
    FileWithDetails file;
    const std::string& filename() const
    {
        return file.path();
    }
    const std::string& sfilename() const
    {
        return file.spath();
    }
    std::string defines;
    // TODO: handle differently
    std::string cppcheckDefines() const {
        std::ostringstream oss;
        oss << defines;

        if (msc) {
            oss << ";_MSC_VER=1900";
        }
        if (useMfc) {
            oss << ";__AFXWIN_H__=1";
        }

        // Add Y2038 specific flags to configuration
        if (timeBitsDefined) {
            oss << ";_TIME_BITS=" << timeBitsValue;
        }
        if (fileOffsetBitsDefined) {
            oss << ";_FILE_OFFSET_BITS=" << fileOffsetBitsValue;
        }
        if (useTimeBits64Defined) {
            oss << ";_USE_TIME_BITS64";
        }

        return oss.str();
    }
    std::set<std::string> undefs;
    std::list<std::string> includePaths;
    // only used by clang mode
    std::list<std::string> systemIncludePaths;
    std::string standard;
    Platform::Type platformType = Platform::Type::Unspecified;
    // TODO: get rid of these
    bool msc{};
    bool useMfc{};

    // Y2038 specific configuration flags
    bool timeBitsDefined{};
    int timeBitsValue{};
    bool useTimeBits64Defined{};
    bool fileOffsetBitsDefined{};
    int fileOffsetBitsValue{};
};

#endif // fileSettingsH
