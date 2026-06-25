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

#ifndef SHOWTYPES_H
#define SHOWTYPES_H

#include <cstdint>

enum class Severity : std::uint8_t;

/// @addtogroup GUI
/// @{

/**
 * @brief A class for different show types we have.
 * This class contains enum type for the different show types we have. Each
 * show type presents one severity selectable in the GUI. In addition there
 * are several supporting functions.
 *
 * Notice that the "visibility" settings are automatically loaded when the
 * class is constructed and saved when the class is destroyed.
 */
class ShowTypes {
public:

    /**
     * @brief Show types we have (i.e. severities in the GUI).
     */
    enum ShowType : std::uint8_t {
        ShowStyle = 0,
        ShowWarnings,
        ShowPerformance,
        ShowPortability,
        ShowInformation,
        ShowErrors, // Keep this as last real item
        ShowNone
    };

    /**
     * @brief Constructor.
     * @note Loads visibility settings.
     */
    ShowTypes();

    /**
     * @brief Destructor.
     * @note Saves visibility settings.
     */
    ~ShowTypes();

    /**
     * @brief Load visibility settings from the platform's settings storage.
     */
    void load();

    /**
     * @brief Save visibility settings to the platform's settings storage.
     */
    void save() const;

    /**
     * @brief Is the showtype visible in the GUI?
     * @param category Showtype to check.
     * @return true if the showtype is visible.
     */
    bool isShown(ShowTypes::ShowType category) const;

    /**
     * @brief Is the severity visible in the GUI?
     * @param severity severity to check.
     * @return true if the severity is visible.
     */
    bool isShown(Severity severity) const;

    /**
     * @brief Show/hide the showtype.
     * @param category Showtype whose visibility to set.
     * @param showing true if the severity is set visible.
     */
    void show(ShowTypes::ShowType category, bool showing);

    /**
     * @brief Convert severity string to ShowTypes value
     * @param severity Error severity
     * @return Severity converted to ShowTypes value
     */
    static ShowTypes::ShowType SeverityToShowType(Severity severity);

    bool mVisible[ShowNone];
};


/// @}

#endif // SHOWTYPES_H
