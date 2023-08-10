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

//---------------------------------------------------------------------------
#ifndef checkH
#define checkH
//---------------------------------------------------------------------------

#include "config.h"

#include <list>
#include <string>
#include <utility>

namespace tinyxml2 {
    class XMLElement;
}

namespace CTU {
    class FileInfo;
}

class Settings;
class ErrorLogger;
class Tokenizer;

/** Use WRONG_DATA in checkers to mark conditions that check that data is correct */
#define WRONG_DATA(COND, TOK)  ((COND) && wrongData((TOK), #COND))

/// @addtogroup Core
/// @{

/**
 * @brief Interface class that cppcheck uses to communicate with the checks.
 * All checking classes must inherit from this class
 */
class CPPCHECKLIB Check {
protected:
    /** This constructor is used when registering the check */
    explicit Check(std::string aname)
        : mName(std::move(aname))
    {}
    virtual ~Check() = default;

public:
    Check(const Check &) = delete;
    Check& operator=(const Check &) = delete;

    /** run checks, the token list is not simplified */
    virtual void runChecks(const Tokenizer &, ErrorLogger *) = 0;

    /** get error messages */
    virtual void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const = 0;

    /** class name, used to generate documentation */
    const std::string& name() const {
        return mName;
    }

    /** get information about this class, used to generate documentation */
    virtual std::string classInfo() const = 0;

    /** Base class used for whole-program analysis */
    class CPPCHECKLIB FileInfo {
    public:
        explicit FileInfo(std::string f0 = {}) : file0(std::move(f0)) {}
        virtual ~FileInfo() = default;
        virtual std::string toString() const {
            return std::string();
        }
        std::string file0;
    };

    virtual FileInfo * getFileInfo(const Tokenizer& /*tokenizer*/, const Settings& /*settings*/, const std::string& /*currentConfig*/) const {
        return nullptr;
    }

    virtual FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const {
        (void)xmlElement;
        return nullptr;
    }

    // Return true if an error is reported.
    virtual bool analyseWholeProgram(const CTU::FileInfo& /*ctu*/, const std::list<FileInfo*>& /*fileInfo*/, const Settings& /*settings*/, ErrorLogger & /*errorLogger*/) {
        return false;
    }

private:
    const std::string mName;
};

/// @}
//---------------------------------------------------------------------------
#endif //  checkH
