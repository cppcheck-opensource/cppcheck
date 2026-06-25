/* -*- C++ -*-
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

#ifndef filelisterH
#define filelisterH

#include <list>
#include <set>
#include <string>

class PathMatch;
class FileWithDetails;

/// @addtogroup CLI
/// @{

/** @brief Cross-platform FileLister */
class FileLister {
public:
    /**
     * @brief Recursively add source files to a map.
     * Add source files from given directory and all subdirectries to the
     * given map. Only files with accepted extensions
     * (*.c;*.cpp;*.cxx;*.c++;*.cc;*.txx) are added.
     * @param files output list that associates the size of each file with its name
     * @param path root path
     * @param extra Extra file extensions
     * @param ignored ignored paths
     * @param debug log if path was ignored
     * @return On success, an empty string is returned. On error, a error message is returned.
     */
    static std::string recursiveAddFiles(std::list<FileWithDetails> &files, const std::string &path, const std::set<std::string> &extra, const PathMatch& ignored, bool debug = false);

    /**
     * @brief (Recursively) add source files to a map.
     * Add source files from given directory and all subdirectries to the
     * given map. Only files with accepted extensions
     * (*.c;*.cpp;*.cxx;*.c++;*.cc;*.txx) are added.
     * @param files output list that associates the size of each file with its name
     * @param path root path
     * @param extra Extra file extensions
     * @param recursive Enable recursion
     * @param ignored ignored paths
     * @param debug log when a path was ignored
     * @return On success, an empty string is returned. On error, a error message is returned.
     */
    static std::string addFiles(std::list<FileWithDetails> &files, const std::string &path, const std::set<std::string> &extra, bool recursive, const PathMatch& ignored, bool debug = false);
};

/// @}

#endif // #ifndef filelisterH
