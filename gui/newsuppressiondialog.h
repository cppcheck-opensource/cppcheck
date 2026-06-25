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

#ifndef NEWSUPPRESSIONDIALOG_H
#define NEWSUPPRESSIONDIALOG_H

#include "suppressions.h"

#include <QDialog>
#include <QObject>

class QWidget;
namespace Ui {
    class NewSuppressionDialog;
}

class NewSuppressionDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewSuppressionDialog(QWidget *parent = nullptr);
    NewSuppressionDialog(const NewSuppressionDialog &) = delete;
    ~NewSuppressionDialog() override;
    NewSuppressionDialog &operator=(const NewSuppressionDialog &) = delete;

    /**
     * @brief Translate the user input in the GUI into a suppression
     * @return Cppcheck suppression
     */
    SuppressionList::Suppression getSuppression() const;

    /**
     * @brief Update the GUI so it corresponds with the given
     * Cppcheck suppression
     * @param suppression Cppcheck suppression
     */
    void setSuppression(const SuppressionList::Suppression &suppression);

private:
    Ui::NewSuppressionDialog *mUI;
};

#endif // NEWSUPPRESSIONDIALOG_H
