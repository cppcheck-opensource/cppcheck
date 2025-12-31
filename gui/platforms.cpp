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

#include "platforms.h"

Platforms::Platforms(QObject *parent)
    : QObject(parent)
{
    init();
}

void Platforms::add(const QString &title, Platform::Type platform)
{
    PlatformData plat;
    plat.mTitle = title;
    plat.mType = platform;
    plat.mActMainWindow = nullptr;
    mPlatforms << plat;
}

void Platforms::init()
{
    add(tr("Native"), Platform::Type::Native);
    add("Unix 32-bit", Platform::Type::Unix32);
    add("Unix 64-bit", Platform::Type::Unix64);
    add("Windows 32-bit ANSI", Platform::Type::Win32A);
    add("Windows 32-bit Unicode", Platform::Type::Win32W);
    add("Windows 64-bit", Platform::Type::Win64);
}

int Platforms::getCount() const
{
    return mPlatforms.count();
}

PlatformData& Platforms::get(Platform::Type platform)
{
    auto iter = mPlatforms.begin();
    while (iter != mPlatforms.end()) {
        if (iter->mType == platform) {
            return *iter;
        }
        ++iter;
    }
    return mPlatforms.first();
}
