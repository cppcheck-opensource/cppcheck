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

#ifndef SINGLEEXECUTOR_H
#define SINGLEEXECUTOR_H

#include "executor.h"

#include <list>

class ErrorLogger;
class Settings;
class CppCheck;
struct Suppressions;
struct FileSettings;
class FileWithDetails;
class TimerResults;

class SingleExecutor : public Executor
{
public:
    SingleExecutor(CppCheck &cppcheck, const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger, TimerResults* timerResults);
    SingleExecutor(const SingleExecutor &) = delete;
    SingleExecutor& operator=(const SingleExecutor &) = delete;

    unsigned int check() override;

private:
    CppCheck &mCppcheck;
};

#endif // SINGLEEXECUTOR_H
