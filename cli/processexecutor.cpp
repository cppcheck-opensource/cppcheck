/*
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

#if defined(__CYGWIN__)
#define _BSD_SOURCE // required to have getloadavg()
#endif

#include "processexecutor.h"

#ifdef HAS_THREADING_MODEL_FORK

#include "cppcheck.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "filesettings.h"
#include "settings.h"
#include "suppressions.h"
#include "timer.h"
#include "utils.h"

#include <algorithm>
#include <numeric>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <fcntl.h>


#ifdef __SVR4  // Solaris
#include <sys/loadavg.h>
#endif

#if defined(__linux__)
#include <sys/prctl.h>
#endif

enum class Color : std::uint8_t;

// NOLINTNEXTLINE(misc-unused-using-decls) - required for FD_ZERO
using std::memset;


ProcessExecutor::ProcessExecutor(const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger, TimerResults* timerResults, CppCheck::ExecuteCmdFn executeCommand)
    : Executor(files, fileSettings, settings, suppressions, errorLogger, timerResults)
    , mExecuteCommand(std::move(executeCommand))
{
    assert(mSettings.jobs > 1);
}

namespace {
    class PipeWriter : public ErrorLogger {
    public:
        enum PipeSignal : std::uint8_t {REPORT_OUT='1',REPORT_ERROR='2',REPORT_SUPPR_INLINE='3',REPORT_SUPPR='4',CHILD_END='5',REPORT_METRIC='6'};

        explicit PipeWriter(int pipe) : mWpipe(pipe) {}

        void reportOut(const std::string &outmsg, Color c) override {
            writeToPipe(REPORT_OUT, static_cast<char>(c) + outmsg);
        }

        void reportErr(const ErrorMessage &msg) override {
            writeToPipe(REPORT_ERROR, msg.serialize());
        }

        void writeSuppr(const SuppressionList &supprs) const {
            for (const auto& suppr : supprs.getSuppressions())
            {
                if (suppr.isInline)
                    writeToPipe(REPORT_SUPPR_INLINE, suppressionToString(suppr));
                else if (suppr.checked)
                    writeToPipe(REPORT_SUPPR, suppressionToString(suppr));
            }
        }

        void reportMetric(const std::string &metric) override {
            writeToPipe(REPORT_METRIC, metric);
        }

        void writeEnd(const std::string& str) const {
            writeToPipe(CHILD_END, str);
        }

    private:
        static std::string suppressionToString(const SuppressionList::Suppression &suppr)
        {
            std::string suppr_str = suppr.toString();
            suppr_str += ";";
            suppr_str += suppr.checked ? "1" : "0";
            suppr_str += ";";
            suppr_str += suppr.matched ? "1" : "0";
            suppr_str += ";";
            suppr_str += suppr.extraComment;
            return suppr_str;
        }

        // TODO: how to log file name in error?
        void writeToPipeInternal(PipeSignal type, const void* data, std::size_t to_write) const
        {
            const ssize_t bytes_written = write(mWpipe, data, to_write);
            if (bytes_written <= 0) {
                const int err = errno;
                std::cerr << "#### ThreadExecutor::writeToPipeInternal() error for type " << static_cast<unsigned char>(type) << ": " << std::strerror(err) << std::endl;
                std::exit(EXIT_FAILURE);
            }
            // TODO: write until everything is written
            if (bytes_written != to_write) {
                std::cerr << "#### ThreadExecutor::writeToPipeInternal() error for type " << static_cast<unsigned char>(type) << ": insufficient data written (expected: " << to_write << " / got: " << bytes_written << ")" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }

        void writeToPipe(PipeSignal type, const std::string &data) const
        {
            {
                const auto t = static_cast<char>(type);
                writeToPipeInternal(type, &t, 1);
            }

            const auto len = static_cast<unsigned int>(data.length());
            {
                static constexpr std::size_t l_size = sizeof(unsigned int);
                writeToPipeInternal(type, &len, l_size);
            }

            if (len > 0) // TODO: unexpected - write a warning?
                writeToPipeInternal(type, data.c_str(), len);
        }

        const int mWpipe;
    };
}

bool ProcessExecutor::handleRead(int rpipe, unsigned int &result, const std::string& filename)
{
    std::size_t bytes_to_read;
    ssize_t bytes_read;

    char type = 0;
    bytes_to_read = sizeof(char);
    bytes_read = read(rpipe, &type, bytes_to_read);
    if (bytes_read <= 0) {
        if (errno == EAGAIN)
            return true;

        // TODO: log details about failure

        // need to increment so a missing pipe (i.e. premature exit of forked process) results in an error exitcode
        ++result;
        return false;
    }
    if (bytes_read != bytes_to_read) {
        std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") error (type): insufficient data read (expected: " << bytes_to_read << " / got: " << bytes_read << ")" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (type != PipeWriter::REPORT_OUT &&
        type != PipeWriter::REPORT_ERROR &&
        type != PipeWriter::REPORT_SUPPR_INLINE &&
        type != PipeWriter::REPORT_SUPPR &&
        type != PipeWriter::CHILD_END &&
        type != PipeWriter::REPORT_METRIC) {
        std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") invalid type " << int(type) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    unsigned int len = 0;
    bytes_to_read = sizeof(len);
    bytes_read = read(rpipe, &len, bytes_to_read);
    if (bytes_read <= 0) {
        const int err = errno;
        std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") error (len) for type " << int(type) << ": " << std::strerror(err) << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (bytes_read != bytes_to_read) {
        std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") error (len) for type" << int(type) << ": insufficient data read (expected: " << bytes_to_read << " / got: " << bytes_read << ")"  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::string buf(len, '\0');
    if (len > 0) { // TODO: unexpected - write a warning?
        char *data_start = &buf[0];
        bytes_to_read = len;
        do {
            bytes_read = read(rpipe, data_start, bytes_to_read);
            if (bytes_read <= 0) {
                const int err = errno;
                std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") error (buf) for type" << int(type) << ": " << std::strerror(err) << std::endl;
                std::exit(EXIT_FAILURE);
            }
            bytes_to_read -= bytes_read;
            data_start += bytes_read;
        } while (bytes_to_read != 0);
    }

    bool res = true;
    if (type == PipeWriter::REPORT_OUT) {
        // the first character is the color
        const auto c = static_cast<Color>(buf[0]);
        // TODO: avoid string copy
        mErrorLogger.reportOut(buf.substr(1), c);
    } else if (type == PipeWriter::REPORT_ERROR) {
        ErrorMessage msg;
        try {
            msg.deserialize(buf);
        } catch (const InternalError& e) {
            std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") internal error: " << e.errorMessage << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (hasToLog(msg))
            mErrorLogger.reportErr(msg);
    } else if (type == PipeWriter::REPORT_SUPPR_INLINE || type == PipeWriter::REPORT_SUPPR) {
        if (!buf.empty()) {
            // TODO: avoid string splitting
            auto parts = splitString(buf, ';');
            if (parts.size() < 4)
            {
                // TODO: make this non-fatal
                std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") adding of inline suppression failed - insufficient data" << std::endl;
                std::exit(EXIT_FAILURE);
            }
            auto suppr = SuppressionList::parseLine(parts[0]);
            suppr.isInline = (type == PipeWriter::REPORT_SUPPR_INLINE);
            suppr.checked = parts[1] == "1";
            suppr.matched = parts[2] == "1";
            suppr.extraComment = parts[3];
            for (std::size_t i = 4; i < parts.size(); i++) {
                suppr.extraComment += ";" + parts[i];
            }
            const std::string err = mSuppressions.nomsg.addSuppression(suppr);
            if (!err.empty()) {
                // TODO: only update state if it doesn't exist - otherwise propagate error
                mSuppressions.nomsg.updateSuppressionState(suppr); // TODO: check result
                // TODO: make this non-fatal
                //std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") adding of inline suppression failed - " << err << std::endl;
                //std::exit(EXIT_FAILURE);
            }
        }
    } else if (type == PipeWriter::CHILD_END) {
        result += std::stoi(buf);
        res = false;
    } else if (type == PipeWriter::REPORT_METRIC) {
        mErrorLogger.reportMetric(buf);
    }

    return res;
}

bool ProcessExecutor::checkLoadAverage(size_t nchildren)
{
#if defined(__QNX__) || defined(__HAIKU__) || defined(_AIX) // getloadavg() is unsupported on Qnx, Haiku, AIX.
    (void)nchildren;
    return true;
#else
    if (!nchildren || !mSettings.loadAverage) {
        return true;
    }

    double sample(0);
    if (getloadavg(&sample, 1) != 1) {
        // disable load average checking on getloadavg error
        return true;
    }
    if (sample < mSettings.loadAverage) {
        return true;
    }
    return false;
#endif
}

unsigned int ProcessExecutor::check()
{
    unsigned int fileCount = 0;
    unsigned int result = 0;

    const std::size_t totalfilesize = std::accumulate(mFiles.cbegin(), mFiles.cend(), std::size_t(0), [](std::size_t v, const FileWithDetails& p) {
        return v + p.size();
    });

    std::list<int> rpipes;
    std::map<pid_t, std::string> childFile;
    std::map<int, std::string> pipeFile;
    std::size_t processedsize = 0;
    auto iFile = mFiles.cbegin();
    auto iFileSettings = mFileSettings.cbegin();
    for (;;) {
        // Start a new child
        const size_t nchildren = childFile.size();
        if ((iFile != mFiles.cend() || iFileSettings != mFileSettings.cend()) && nchildren < mSettings.jobs && checkLoadAverage(nchildren)) {
            int pipes[2];
            if (pipe(pipes) == -1) {
                std::cerr << "#### ThreadExecutor::check, pipe() failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            const int flags = fcntl(pipes[0], F_GETFL, 0);
            if (flags < 0) {
                std::cerr << "#### ThreadExecutor::check, fcntl(F_GETFL) failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            if (fcntl(pipes[0], F_SETFL, flags) < 0) {
                std::cerr << "#### ThreadExecutor::check, fcntl(F_SETFL) failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            const pid_t pid = fork();
            if (pid < 0) {
                // Error
                std::cerr << "#### ThreadExecutor::check, Failed to create child process: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            } else if (pid == 0) {
#if defined(__linux__)
                prctl(PR_SET_PDEATHSIG, SIGHUP);
#endif
                close(pipes[0]);

                PipeWriter pipewriter(pipes[1]);
                CppCheck fileChecker(mSettings, mSuppressions, pipewriter, mTimerResults, false, mExecuteCommand);
                unsigned int resultOfCheck = 0;

                if (iFileSettings != mFileSettings.end()) {
                    resultOfCheck = fileChecker.check(*iFileSettings);
                } else {
                    // Read file from a file
                    resultOfCheck = fileChecker.check(*iFile);
                }

                pipewriter.writeSuppr(mSuppressions.nomsg);

                pipewriter.writeEnd(std::to_string(resultOfCheck));
                std::exit(EXIT_SUCCESS);
            }

            close(pipes[1]);
            rpipes.push_back(pipes[0]);
            if (iFileSettings != mFileSettings.end()) {
                childFile[pid] = iFileSettings->filename() + ' ' + iFileSettings->cfg;
                pipeFile[pipes[0]] = iFileSettings->filename() + ' ' + iFileSettings->cfg;
                ++iFileSettings;
            } else {
                childFile[pid] = iFile->path();
                pipeFile[pipes[0]] = iFile->path();
                ++iFile;
            }
        }
        if (!rpipes.empty()) {
            fd_set rfds;
            FD_ZERO(&rfds);
            for (auto rp = rpipes.cbegin(); rp != rpipes.cend(); ++rp)
                FD_SET(*rp, &rfds);
            timeval tv; // for every second polling of load average condition
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            const int r = select(*std::max_element(rpipes.cbegin(), rpipes.cend()) + 1, &rfds, nullptr, nullptr, &tv);

            if (r > 0) {
                auto rp = rpipes.cbegin();
                while (rp != rpipes.cend()) {
                    if (FD_ISSET(*rp, &rfds)) {
                        std::string name;
                        const auto p = utils::as_const(pipeFile).find(*rp);
                        if (p != pipeFile.cend()) {
                            name = p->second;
                        }
                        const bool readRes = handleRead(*rp, result, name);
                        if (!readRes) {
                            std::size_t size = 0;
                            if (p != pipeFile.cend()) {
                                pipeFile.erase(p);
                                const auto fs = std::find_if(mFiles.cbegin(), mFiles.cend(), [&name](const FileWithDetails& entry) {
                                    return entry.path() == name;
                                });
                                if (fs != mFiles.end()) {
                                    size = fs->size();
                                }
                            }

                            fileCount++;
                            processedsize += size;
                            if (!mSettings.quiet)
                                Executor::reportStatus(fileCount, mFiles.size() + mFileSettings.size(), processedsize, totalfilesize);

                            close(*rp);
                            rp = rpipes.erase(rp);
                        } else
                            ++rp;
                    } else
                        ++rp;
                }
            }
        }
        if (!childFile.empty()) {
            int stat = 0;
            const pid_t child = waitpid(0, &stat, WNOHANG);
            if (child > 0) {
                std::string childname;
                const auto c = utils::as_const(childFile).find(child);
                if (c != childFile.cend()) {
                    childname = c->second;
                    childFile.erase(c);
                }

                if (WIFEXITED(stat)) {
                    const int exitstatus = WEXITSTATUS(stat);
                    if (exitstatus != EXIT_SUCCESS) {
                        std::ostringstream oss;
                        oss << "Child process exited with " << exitstatus;
                        reportInternalChildErr(childname, oss.str());
                    }
                } else if (WIFSIGNALED(stat)) {
                    std::ostringstream oss;
                    oss << "Child process crashed with signal " << WTERMSIG(stat);
                    reportInternalChildErr(childname, oss.str());
                }
            }
        }
        if (iFile == mFiles.end() && iFileSettings == mFileSettings.end() && rpipes.empty() && childFile.empty()) {
            // All done
            break;
        }
    }

    // TODO: we need to get the timing information from the subprocess

    return result;
}

void ProcessExecutor::reportInternalChildErr(const std::string &childname, const std::string &msg)
{
    std::list<ErrorMessage::FileLocation> locations;
    locations.emplace_back(childname, 0, 0);
    const ErrorMessage errmsg(std::move(locations),
                              "",
                              Severity::error,
                              "Internal error: " + msg,
                              "cppcheckError",
                              Certainty::normal);

    if (!mSuppressions.nomsg.isSuppressed(errmsg, {}))
        mErrorLogger.reportErr(errmsg);
}

#endif // HAS_THREADING_MODEL_FORK
