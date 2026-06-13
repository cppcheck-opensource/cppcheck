/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2026 Cppcheck team.
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

#ifndef TESTSHOWTYPES_H
#define TESTSHOWTYPES_H

#include <QObject>

class TestShowTypes : public QObject {
    Q_OBJECT

private slots:
    void severityToShowType_allValues() const;
    void severityToShowType_noneAndInternal() const;
    void setShow_togglesVisibility();
    void isShown_bySeverity();
};

#endif // TESTSHOWTYPES_H
