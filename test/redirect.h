// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2025 Cppcheck team.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef REDIRECT_H
#define REDIRECT_H

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

/**
 * @brief Utility class for capturing cout and cerr to ostringstream buffers
 * for later use. Uses RAII to stop redirection when the object goes out of
 * scope.
 * NOTE: This is *not* thread-safe.
 */
class RedirectOutputError {
public:
    /** Set up redirection, flushing anything in the pipes. */
    RedirectOutputError() {
        // flush all old output
        std::cout.flush();
        std::cerr.flush();

        _oldCout = std::cout.rdbuf(); // back up cout's streambuf
        _oldCerr = std::cerr.rdbuf(); // back up cerr's streambuf

        std::cout.rdbuf(_out.rdbuf()); // assign streambuf to cout
        std::cerr.rdbuf(_err.rdbuf()); // assign streambuf to cerr
    }

    // TODO: if an assert failed previously this will mask that failure
    /** Revert cout and cerr behaviour */
    ~RedirectOutputError() noexcept(false) {
        std::cout.rdbuf(_oldCout); // restore cout's original streambuf
        std::cerr.rdbuf(_oldCerr); // restore cerrs's original streambuf

        {
            const std::string s = _out.str();
            if (!s.empty())
                throw std::runtime_error("unconsumed stdout: " + s);
        }

        {
            const std::string s = _err.str();
            if (!s.empty())
                throw std::runtime_error("consumed stderr: " + s);
        }
    }

    /** Return what would be printed to cout. */
    std::string getOutput() {
        std::string s = _out.str();
        _out.str("");
        return s;
    }

    /** Return what would be printed to cerr. */
    std::string getErrout() {
        std::string s = _err.str();
        _err.str("");
        return s;
    }

private:
    std::ostringstream _out;
    std::ostringstream _err;
    std::streambuf *_oldCout;
    std::streambuf *_oldCerr;
};

class SuppressOutput {
public:
    /** Set up suppression, flushing anything in the pipes. */
    SuppressOutput() {
        // flush all old output
        std::cout.flush();
        std::cerr.flush();

        _oldCout = std::cout.rdbuf(); // back up cout's streambuf
        _oldCerr = std::cerr.rdbuf(); // back up cerr's streambuf

        std::cout.rdbuf(nullptr); // disable cout
        std::cerr.rdbuf(nullptr); // disable cerr
    }

    /** Revert cout and cerr behaviour */
    ~SuppressOutput() {
        std::cout.rdbuf(_oldCout); // restore cout's original streambuf
        std::cerr.rdbuf(_oldCerr); // restore cerrs's original streambuf
    }

private:
    std::streambuf *_oldCout;
    std::streambuf *_oldCerr;
};

#define REDIRECT RedirectOutputError redir
#define GET_REDIRECT_OUTPUT redir.getOutput()
#define GET_REDIRECT_ERROUT redir.getErrout()

#define SUPPRESS SuppressOutput supprout


class RedirectInput {
public:
    explicit RedirectInput(const std::string &input) : _in(input) {
        _oldCin = std::cin.rdbuf(); // back up cin's streambuf
        std::cin.rdbuf(_in.rdbuf()); // assign streambuf to cin
    }
    ~RedirectInput() noexcept {
        std::cin.rdbuf(_oldCin); // restore cin's original streambuf
    }
private:
    std::istringstream _in;
    std::streambuf* _oldCin;
};

#endif
