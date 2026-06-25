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

#include "showtypes.h"

#include "common.h"
#include "errortypes.h"

#include <QSettings>

ShowTypes::ShowTypes()
{
    load();
}

ShowTypes::~ShowTypes()
{
    save();
}

ShowTypes::ShowType ShowTypes::SeverityToShowType(Severity severity)
{
    switch (severity) {
    case Severity::none:
    case Severity::internal:
        return ShowTypes::ShowNone;
    case Severity::error:
        return ShowTypes::ShowErrors;
    case Severity::style:
        return ShowTypes::ShowStyle;
    case Severity::warning:
        return ShowTypes::ShowWarnings;
    case Severity::performance:
        return ShowTypes::ShowPerformance;
    case Severity::portability:
        return ShowTypes::ShowPortability;
    case Severity::information:
        return ShowTypes::ShowInformation;
    default:
        return ShowTypes::ShowNone;
    }
}

void ShowTypes::load()
{
    QSettings settings;
    mVisible[ShowStyle] = settings.value(SETTINGS_SHOW_STYLE, true).toBool();
    mVisible[ShowErrors] = settings.value(SETTINGS_SHOW_ERRORS, true).toBool();
    mVisible[ShowWarnings] = settings.value(SETTINGS_SHOW_WARNINGS, true).toBool();
    mVisible[ShowPortability] = settings.value(SETTINGS_SHOW_PORTABILITY, true).toBool();
    mVisible[ShowPerformance] = settings.value(SETTINGS_SHOW_PERFORMANCE, true).toBool();
    mVisible[ShowInformation] = settings.value(SETTINGS_SHOW_INFORMATION, true).toBool();
}

void ShowTypes::save() const
{
    QSettings settings;
    settings.setValue(SETTINGS_SHOW_STYLE, mVisible[ShowStyle]);
    settings.setValue(SETTINGS_SHOW_ERRORS, mVisible[ShowErrors]);
    settings.setValue(SETTINGS_SHOW_WARNINGS, mVisible[ShowWarnings]);
    settings.setValue(SETTINGS_SHOW_PORTABILITY, mVisible[ShowPortability]);
    settings.setValue(SETTINGS_SHOW_PERFORMANCE, mVisible[ShowPerformance]);
    settings.setValue(SETTINGS_SHOW_INFORMATION, mVisible[ShowInformation]);
}

bool ShowTypes::isShown(ShowTypes::ShowType category) const
{
    return mVisible[category];
}

bool ShowTypes::isShown(Severity severity) const
{
    return isShown(ShowTypes::SeverityToShowType(severity));
}

void ShowTypes::show(ShowTypes::ShowType category, bool showing)
{
    mVisible[category] = showing;
}
