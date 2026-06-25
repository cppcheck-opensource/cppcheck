/*
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

#include "scratchpad.h"

#include "codeeditor.h"
#include "mainwindow.h"

#include "ui_scratchpad.h"

#include <QLineEdit>
#include <QPushButton>
#include <QString>

ScratchPad::ScratchPad(MainWindow& mainWindow)
    : QDialog(&mainWindow)
    , mUI(new Ui::ScratchPad)
    , mMainWindow(mainWindow)
{
    mUI->setupUi(this);

    connect(mUI->mCheckButton, &QPushButton::clicked, this, &ScratchPad::checkButtonClicked);
}

ScratchPad::~ScratchPad()
{
    delete mUI;
}

void ScratchPad::translate()
{
    mUI->retranslateUi(this);
}

void ScratchPad::checkButtonClicked()
{
    QString filename = mUI->lineEdit->text();
    if (filename.isEmpty())
        filename = "test.cpp";
    mMainWindow.analyzeCode(mUI->plainTextEdit->toPlainText(), filename);
}
