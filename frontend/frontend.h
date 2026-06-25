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

#ifndef FRONTEND_H
#define FRONTEND_H

#include "standards.h"

#include <list>

struct FileSettings;
class Settings;
class FileWithDetails;

namespace frontend
{
    /**
        Applies the enforced language as all as identifying remaining files - also taking markup files into consideration.
     */
    void applyLang(std::list<FileSettings> &fileSettings, const Settings &settings, Standards::Language enforcedLang);
    void applyLang(std::list<FileWithDetails> &files, const Settings &settings, Standards::Language enforcedLang);
}

#endif // FRONTEND_H
