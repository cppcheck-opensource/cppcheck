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

#include "frontend.h"

#include "filesettings.h"
#include "library.h"
#include "path.h"
#include "settings.h"

#include <cassert>

namespace frontend {
    void applyLang(std::list<FileSettings>& fileSettings, const Settings& settings, Standards::Language enforcedLang)
    {
        if (enforcedLang != Standards::Language::None)
        {
            // apply enforced language
            for (auto& fs : fileSettings)
            {
                if (settings.library.markupFile(fs.filename()))
                    continue;
                fs.file.setLang(enforcedLang);
            }
        }
        else
        {
            // identify files
            for (auto& fs : fileSettings)
            {
                if (settings.library.markupFile(fs.filename()))
                    continue;
                assert(fs.file.lang() == Standards::Language::None);
                bool header = false;
                fs.file.setLang(Path::identify(fs.filename(), settings.cppHeaderProbe, &header));
                // unknown extensions default to C++
                if (!header && fs.file.lang() == Standards::Language::None)
                    fs.file.setLang(Standards::Language::CPP);
            }
        }

        // enforce the language since markup files are special and do not adhere to the enforced language
        for (auto& fs : fileSettings)
        {
            if (settings.library.markupFile(fs.filename())) {
                assert(fs.file.lang() == Standards::Language::None);
                fs.file.setLang(Standards::Language::C);
            }
        }
    }

    void applyLang(std::list<FileWithDetails>& files, const Settings& settings, Standards::Language enforcedLang)
    {
        if (enforcedLang != Standards::Language::None)
        {
            // apply enforced language
            for (auto& f : files)
            {
                if (settings.library.markupFile(f.path()))
                    continue;
                f.setLang(enforcedLang);
            }
        }
        else
        {
            // identify remaining files
            for (auto& f : files)
            {
                if (f.lang() != Standards::Language::None)
                    continue;
                if (settings.library.markupFile(f.path()))
                    continue;
                bool header = false;
                f.setLang(Path::identify(f.path(), settings.cppHeaderProbe, &header));
                // unknown extensions default to C++
                if (!header && f.lang() == Standards::Language::None)
                    f.setLang(Standards::Language::CPP);
            }
        }

        // enforce the language since markup files are special and do not adhere to the enforced language
        for (auto& f : files)
        {
            if (settings.library.markupFile(f.path())) {
                f.setLang(Standards::Language::C);
            }
        }
    }
}
