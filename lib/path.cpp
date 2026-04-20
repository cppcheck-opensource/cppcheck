/*
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

//#define LOG_EMACS_MARKER

#if defined(__CYGWIN__)
#define _POSIX_C_SOURCE 200112L // required to have readlink()
#define _BSD_SOURCE // required to have realpath()
#endif

#include "path.h"
#include "utils.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#ifdef LOG_EMACS_MARKER
#include <iostream>
#endif
#include <sys/stat.h>
#include <unordered_set>
#include <utility>

#include <simplecpp.h>

#ifndef _WIN32
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#else
#include <direct.h>
#include <windows.h>
#endif
#if defined(__CYGWIN__)
#include <strings.h>
#endif
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif


/** Is the filesystem case insensitive? */
static constexpr bool caseInsensitiveFilesystem()
{
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
    // Windows is case insensitive
    // MacOS is case insensitive by default (also supports case sensitivity)
    return true;
#else
    // TODO: Non-windows filesystems might be case insensitive
    // needs to be determined per filesystem and location - e.g. /sys/fs/ext4/features/casefold
    return false;
#endif
}

std::string Path::toNativeSeparators(std::string path)
{
#if defined(_WIN32)
    constexpr char separ = '/';
    constexpr char native = '\\';
#else
    constexpr char separ = '\\';
    constexpr char native = '/';
#endif
    std::replace(path.begin(), path.end(), separ, native);
    return path;
}

std::string Path::fromNativeSeparators(std::string path)
{
    constexpr char nonnative = '\\';
    constexpr char newsepar = '/';
    std::replace(path.begin(), path.end(), nonnative, newsepar);
    return path;
}

std::string Path::simplifyPath(std::string originalPath)
{
    return simplecpp::simplifyPath(std::move(originalPath));
}

std::string Path::getPathFromFilename(const std::string &filename)
{
    const std::size_t pos = filename.find_last_of("\\/");

    if (pos != std::string::npos)
        return filename.substr(0, 1 + pos);

    return "";
}

bool Path::sameFileName(const std::string &fname1, const std::string &fname2)
{
    return caseInsensitiveFilesystem() ? (caseInsensitiveStringCompare(fname1, fname2) == 0) : (fname1 == fname2);
}

std::string Path::removeQuotationMarks(std::string path)
{
    path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());
    return path;
}

std::string Path::getFilenameExtension(const std::string &path, bool lowercase)
{
    const std::string::size_type dotLocation = path.find_last_of('.');
    if (dotLocation == std::string::npos)
        return "";

    std::string extension = path.substr(dotLocation);
    if (lowercase || caseInsensitiveFilesystem()) {
        // on a case insensitive filesystem the case doesn't matter so
        // let's return the extension in lowercase
        strTolower(extension);
    }
    return extension;
}

std::string Path::getFilenameExtensionInLowerCase(const std::string &path)
{
    return getFilenameExtension(path, true);
}

std::string Path::getCurrentPath()
{
    char currentPath[4096];

#ifndef _WIN32
    if (getcwd(currentPath, 4096) != nullptr)
#else
    if (_getcwd(currentPath, 4096) != nullptr)
#endif
        return std::string(currentPath);

    return "";
}

std::string Path::getCurrentExecutablePath(const char* fallback)
{
    char buf[4096] = {};
    bool success{};
#ifdef _WIN32
    success = (GetModuleFileNameA(nullptr, buf, sizeof(buf)) < sizeof(buf));
#elif defined(__APPLE__)
    uint32_t size = sizeof(buf);
    success = (_NSGetExecutablePath(buf, &size) == 0);
#else
    const char* procPath =
#ifdef __SVR4 // Solaris
        "/proc/self/path/a.out";
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
        "/proc/curproc/file";
#else // Linux
        "/proc/self/exe";
#endif
    // readlink does not null-terminate the string if the buffer is too small, therefore write bufsize - 1
    success = (readlink(procPath, buf, sizeof(buf) - 1) != -1);
#endif
    return success ? std::string(buf) : std::string(fallback);
}

bool Path::isAbsolute(const std::string& path)
{
#ifdef _WIN32
    if (path.length() < 2)
        return false;

    if ((path[0] == '\\' || path[0] == '/') && (path[1] == '\\' || path[1] == '/'))
        return true;

    // On Windows, 'C:\foo\bar' is an absolute path, while 'C:foo\bar' is not
    return std::isalpha(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/');
#else
    return !path.empty() && path[0] == '/';
#endif
}

bool Path::isRelative(const std::string& path)
{
    const std::string p = fromNativeSeparators(path);
    return (p.find('/') != std::string::npos) && !isAbsolute(path);
}

std::string Path::getRelativePath(const std::string& absolutePath, const std::vector<std::string>& basePaths)
{
    for (const std::string &bp : basePaths) {
        if (absolutePath == bp || bp.empty()) // Seems to be a file, or path is empty
            continue;

        if (absolutePath.compare(0, bp.length(), bp) != 0)
            continue;

        if (endsWith(bp,'/'))
            return absolutePath.substr(bp.length());
        if (absolutePath.size() > bp.size() && absolutePath[bp.length()] == '/')
            return absolutePath.substr(bp.length() + 1);
    }
    return absolutePath;
}

static const std::unordered_set<std::string> cpp_src_exts = {
    ".cpp", ".cxx", ".cc", ".c++", ".tpp", ".txx", ".ipp", ".ixx"
};

static const std::unordered_set<std::string> c_src_exts = {
    ".c", ".cl"
};

static const std::unordered_set<std::string> header_exts = {
    ".h", ".hpp", ".h++", ".hxx", ".hh"
};

bool Path::acceptFile(const std::string &path, const std::set<std::string> &extra, Standards::Language* lang)
{
    bool header = false;
    Standards::Language l = identify(path, false, &header);
    if (lang)
        *lang = l;
    return (l != Standards::Language::None && !header) || extra.find(getFilenameExtension(path)) != extra.end();
}

static bool hasEmacsCppMarker(const char* path)
{
    // TODO: identify is called three times for each file
    // Preprocessor::loadFiles() -> createDUI()
    // Preprocessor::preprocess() -> createDUI()
    // TokenList::createTokens() -> TokenList::determineCppC()
#ifdef LOG_EMACS_MARKER
    std::cout << path << '\n';
#endif

    FILE *fp = fopen(path, "rt");
    if (!fp)
        return false;
    std::string buf(128, '\0');
    {
#if __cplusplus >= 201703L
        // C++17 provides an overload with non-const data()
        #define CONST_CAST(x) (x)
#else
        #define CONST_CAST(x) const_cast<char*>(x)
#endif
        // TODO: read the whole first line only
        const char * const res = fgets(CONST_CAST(buf.data()), buf.size(), fp);
#undef CONST_CAST
        fclose(fp);
        fp = nullptr;
        if (!res)
            return false; // failed to read file
    }
    // TODO: replace with regular expression
    const auto pos1 = buf.find("-*-");
    if (pos1 == std::string::npos)
        return false; // no start marker
    const auto pos_nl = buf.find_first_of("\r\n");
    if (pos_nl != std::string::npos && (pos_nl < pos1)) {
#ifdef LOG_EMACS_MARKER
        std::cout << path << " - Emacs marker not on the first line" << '\n';
#endif
        return false; // not on first line
    }
    const auto pos2 = buf.find("-*-", pos1 + 3);
    // TODO: make sure we have read the whole line before bailing out
    if (pos2 == std::string::npos) {
#ifdef LOG_EMACS_MARKER
        std::cout << path << " - Emacs marker not terminated" << '\n';
#endif
        return false; // no end marker
    }
#ifdef LOG_EMACS_MARKER
    std::cout << "Emacs marker: '"  << buf.substr(pos1, (pos2 + 3) - pos1) << "'" << '\n';
#endif
    // TODO: support /* */ comments
    const std::string buf_trim = trim(buf); // trim whitespaces
    if (buf_trim[0] == '/' && buf_trim[1] == '*') {
        const auto pos_cmt = buf.find("*/", 2);
        if (pos_cmt != std::string::npos && pos_cmt < (pos2 + 3))
        {
 #ifdef LOG_EMACS_MARKER
            std::cout << path << " - Emacs marker not contained in C-style comment block: '"  << buf.substr(pos1, (pos2 + 3) - pos1) << "'" << '\n';
 #endif
            return false; // not in a comment
        }
    }
    else if (buf_trim[0] != '/' || buf_trim[1] != '/') {
#ifdef LOG_EMACS_MARKER
        std::cout << path << " - Emacs marker not in a comment: '"  << buf.substr(pos1, (pos2 + 3) - pos1) << "'" << '\n';
#endif
        return false; // not in a comment
    }

    // there are more variations with lowercase and no whitespaces
    // -*- C++ -*-
    // -*- Mode: C++; -*-
    // -*- Mode: C++; c-basic-offset: 8 -*-
    std::string marker = trim(buf.substr(pos1 + 3, pos2 - pos1 - 3), " ;");
    // cut off additional attributes
    const auto pos_semi = marker.find(';');
    if (pos_semi != std::string::npos)
        marker.resize(pos_semi);
    findAndReplace(marker, "mode:", "");
    findAndReplace(marker, "Mode:", "");
    marker = trim(marker);
    if (marker == "C++" || marker == "c++") {
        // NOLINTNEXTLINE(readability-simplify-boolean-expr) - TODO: FP
        return true; // C++ marker found
    }

    //if (marker == "C" || marker == "c")
    //    return false;
#ifdef LOG_EMACS_MARKER
    std::cout << path << " - unmatched Emacs marker: '"  << marker << "'" << '\n';
#endif

    return false; // marker is not a C++ one
}

Standards::Language Path::identify(const std::string &path, bool cppHeaderProbe, bool *header)
{
    if (header)
        *header = false;

    std::string ext = getFilenameExtension(path);
    // standard library headers have no extension
    if (cppHeaderProbe && ext.empty()) {
        if (hasEmacsCppMarker(path.c_str())) {
            if (header)
                *header = true;
            return Standards::Language::CPP;
        }
        return Standards::Language::None;
    }
    if (ext == ".C")
        return Standards::Language::CPP;
    if (c_src_exts.find(ext) != c_src_exts.end())
        return Standards::Language::C;
    if (!caseInsensitiveFilesystem())
        strTolower(ext);
    if (ext == ".h") {
        if (header)
            *header = true;
        if (cppHeaderProbe && hasEmacsCppMarker(path.c_str()))
            return Standards::Language::CPP;
        return Standards::Language::C;
    }
    if (cpp_src_exts.find(ext) != cpp_src_exts.end())
        return Standards::Language::CPP;
    if (header_exts.find(ext) != header_exts.end()) {
        if (header)
            *header = true;
        return Standards::Language::CPP;
    }
    return Standards::Language::None;
}

bool Path::isHeader(const std::string &path)
{
    bool header;
    (void)identify(path, false, &header);
    return header;
}

std::string Path::getAbsoluteFilePath(const std::string& filePath)
{
    if (filePath.empty())
        return "";

    std::string absolute_path;
#ifdef _WIN32
    char absolute[_MAX_PATH];
    if (_fullpath(absolute, filePath.c_str(), _MAX_PATH))
        absolute_path = absolute;
    if (!absolute_path.empty() && absolute_path.back() == '\\')
        absolute_path.pop_back();
#elif defined(__linux__) || defined(__sun) || defined(__hpux) || defined(__GNUC__) || defined(__CPPCHECK__)
    // simplify the path since any non-existent part has to exist even if discarded by ".."
    std::string spath = Path::simplifyPath(filePath);
    char * absolute = realpath(spath.c_str(), nullptr);
    if (absolute)
        absolute_path = absolute;
    free(absolute);
    // only throw on realpath() failure to resolve a path when the given one was non-existent
    if (!spath.empty() && absolute_path.empty() && !exists(spath))
        throw std::runtime_error("path '" + filePath + "' does not exist");
#else
#error Platform absolute path function needed
#endif
    return absolute_path;
}

std::string Path::stripDirectoryPart(const std::string &file)
{
#if defined(_WIN32) && !defined(__MINGW32__)
    constexpr char native = '\\';
#else
    constexpr char native = '/';
#endif

    const std::string::size_type p = file.rfind(native);
    if (p != std::string::npos) {
        return file.substr(p + 1);
    }
    return file;
}

#ifdef _WIN32
using mode_t = unsigned short;
#endif

static mode_t file_type(const std::string &path)
{
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) == -1)
        return 0;
    return file_stat.st_mode & S_IFMT;
}

bool Path::isFile(const std::string &path)
{
    return file_type(path) == S_IFREG;
}

bool Path::isDirectory(const std::string &path)
{
    return file_type(path) == S_IFDIR;
}

bool Path::exists(const std::string &path, bool* isdir)
{
    const auto type = file_type(path);
    if (type == S_IFDIR)
    {
        if (isdir)
            *isdir = true;
        return true;
    }
    if (isdir)
        *isdir = false;
    return type == S_IFREG;
}

std::string Path::join(std::string path1, std::string path2)
{
    path1 = fromNativeSeparators(std::move(path1));
    path2 = fromNativeSeparators(std::move(path2));
    if (path1.empty() || path2.empty())
        return path1 + path2;
    // this matches the behavior of std::filesystem::path::operator/=() and os.path.join()
    if (path2.front() == '/')
        return path2;
    return ((path1.back() == '/') ? path1 : (path1 + "/")) + path2;
}

std::string Path::join(std::string path1, std::string path2, std::string path3)
{
    return Path::join(Path::join(std::move(path1), std::move(path2)), std::move(path3));
}
