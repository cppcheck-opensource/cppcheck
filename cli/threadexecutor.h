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

#ifndef THREADEXECUTOR_H
#define THREADEXECUTOR_H

#include "config.h"

#ifdef HAS_THREADING_MODEL_THREAD

#include "cppcheck.h"
#include "executor.h"

#include <list>

class Settings;
class ErrorLogger;
struct Suppressions;
struct FileSettings;
class FileWithDetails;
class TimerResults;

/// @addtogroup CLI
/// @{

/**
 * This class will take a list of filenames and settings and check then
 * all files using threads.
 */
class ThreadExecutor : public Executor {
    friend class SyncLogForwarder;

public:
    ThreadExecutor(const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger, TimerResults* timerResults, CppCheck::ExecuteCmdFn executeCommand);
    ThreadExecutor(const ThreadExecutor &) = delete;
    ThreadExecutor& operator=(const ThreadExecutor &) = delete;

    unsigned int check() override;

    CppCheck::ExecuteCmdFn mExecuteCommand;
};

/// @}

#endif // HAS_THREADING_MODEL_THREAD

#endif // THREADEXECUTOR_H
