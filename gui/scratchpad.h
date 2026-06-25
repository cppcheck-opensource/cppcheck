/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include <QDialog>
#include <QObject>

class MainWindow;
namespace Ui {
    class ScratchPad;
}

/// @addtogroup GUI
/// @{

/**
 * @brief A window with a text field that .
 */
class ScratchPad : public QDialog {
    Q_OBJECT
public:
    explicit ScratchPad(MainWindow& mainWindow);
    ~ScratchPad() override;

    /**
     * @brief Translate dialog
     */
    void translate();

private slots:
    /**
     * @brief Called when check button is clicked.
     */
    void checkButtonClicked();

private:
    Ui::ScratchPad *mUI;
    MainWindow& mMainWindow;
};

/// @}

#endif // SCRATCHPAD_H
