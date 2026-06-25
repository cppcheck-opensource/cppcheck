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

#ifndef xmlH
#define xmlH

#include "config.h"
#include "path.h"

#if defined(__GNUC__) && (__GNUC__ >= 14)
SUPPRESS_WARNING_GCC_PUSH("-Wsuggest-attribute=returns_nonnull")
#endif
SUPPRESS_WARNING_GCC_PUSH("-Wuseless-cast")
SUPPRESS_WARNING_CLANG_PUSH("-Wzero-as-null-pointer-constant")
SUPPRESS_WARNING_CLANG_PUSH("-Wsuggest-destructor-override")
SUPPRESS_WARNING_CLANG_PUSH("-Winconsistent-missing-destructor-override")

#include <tinyxml2.h> // IWYU pragma: export

SUPPRESS_WARNING_CLANG_POP
SUPPRESS_WARNING_CLANG_POP
SUPPRESS_WARNING_CLANG_POP
SUPPRESS_WARNING_GCC_POP
#if defined(__GNUC__) && (__GNUC__ >= 14)
SUPPRESS_WARNING_GCC_POP
#endif

inline static tinyxml2::XMLError xml_LoadFile(tinyxml2::XMLDocument& doc, const char* filename)
{
    // tinyxml2 will fail with a misleading XML_ERROR_FILE_READ_ERROR when you try to load a directory as a XML file
    if (Path::isDirectory(filename))
        return tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND;
    return doc.LoadFile(filename);
}

#endif // xmlH
