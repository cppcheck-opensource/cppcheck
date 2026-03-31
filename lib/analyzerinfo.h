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

//---------------------------------------------------------------------------
#ifndef analyzerinfoH
#define analyzerinfoH
//---------------------------------------------------------------------------

#include "config.h"

#include <cstddef>
#include <fstream>
#include <functional>
#include <list>
#include <string>

class ErrorMessage;
struct FileSettings;

namespace tinyxml2 {
    class XMLDocument;
    class XMLElement;
};

/// @addtogroup Core
/// @{

/**
 * @brief Analyzer information
 *
 * Store various analysis information:
 * - checksum
 * - error messages
 * - whole program analysis data
 *
 * The information can be used for various purposes. It allows:
 * - 'make' - only analyze TUs that are changed and generate full report
 * - should be possible to add distributed analysis later
 * - multi-threaded whole program analysis
 */
class CPPCHECKLIB AnalyzerInformation {
public:
    ~AnalyzerInformation();

    static void writeFilesTxt(const std::string &buildDir, const std::list<std::string> &sourcefiles, const std::list<FileSettings> &fileSettings);

    /** Close current TU.analyzerinfo file */
    void close();
    /**
     * @throws std::runtime_error thrown if the output file is already open or the output file cannot be opened
     */
    bool analyzeFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg, std::size_t fsFileId, std::size_t hash, std::list<ErrorMessage> &errors, bool debug = false);
    void reportErr(const ErrorMessage &msg);
    void setFileInfo(const std::string &check, const std::string &fileInfo);
    static std::string getAnalyzerInfoFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg, std::size_t fsFileId);

    static const char sep = ':';

    class CPPCHECKLIB Info {
    public:
        bool parse(const std::string& filesTxtLine);
        std::string afile;
        std::string cfg;
        std::size_t fsFileId = 0;
        std::string sourceFile;
    };

    static std::string processFilesTxt(const std::string& buildDir, const std::function<void(const char* checkattr, const tinyxml2::XMLElement* e, const Info& filesTxtInfo)>& handler, bool debug = false);

protected:
    static std::string getFilesTxt(const std::list<std::string> &sourcefiles, const std::list<FileSettings> &fileSettings);

    static std::string getAnalyzerInfoFileFromFilesTxt(std::istream& filesTxt, const std::string &sourcefile, const std::string &cfg, int fsFileId);

    static std::string skipAnalysis(const tinyxml2::XMLDocument &analyzerInfoDoc, std::size_t hash, std::list<ErrorMessage> &errors);

private:
    std::ofstream mOutputStream;
};

/// @}
//---------------------------------------------------------------------------
#endif // analyzerinfoH
