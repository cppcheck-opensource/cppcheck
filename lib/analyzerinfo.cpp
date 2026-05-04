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

#include "analyzerinfo.h"

#include "errorlogger.h"
#include "filesettings.h"
#include "path.h"
#include "utils.h"

#include <array>
#include <cstring>
#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "xml.h"

AnalyzerInformation::~AnalyzerInformation()
{
    close();
}

static std::string getFilename(const std::string &fullpath)
{
    std::string::size_type pos1 = fullpath.find_last_of("/\\");
    pos1 = (pos1 == std::string::npos) ? 0U : (pos1 + 1U);
    std::string::size_type pos2 = fullpath.rfind('.');
    if (pos2 < pos1)
        pos2 = std::string::npos;
    if (pos2 != std::string::npos)
        pos2 = pos2 - pos1;
    return fullpath.substr(pos1,pos2);
}

void AnalyzerInformation::writeFilesTxt(const std::string &buildDir, const std::list<std::string> &sourcefiles, const std::list<FileSettings> &fileSettings)
{
    const std::string filesTxt(buildDir + "/files.txt");
    std::ofstream fout(filesTxt);
    fout << getFilesTxt(sourcefiles, fileSettings);
}

std::string AnalyzerInformation::getFilesTxt(const std::list<std::string> &sourcefiles, const std::list<FileSettings> &fileSettings) {
    std::ostringstream ret;

    std::map<std::string, unsigned int> fileCount;

    for (const std::string &f : sourcefiles) {
        const std::string afile = getFilename(f);
        ret << afile << ".a" << (++fileCount[afile]) << sep << sep << sep << Path::simplifyPath(f) << '\n';
    }

    for (const FileSettings &fs : fileSettings) {
        const std::string afile = getFilename(fs.filename());
        const std::string id = fs.file.fsFileId() > 0 ? std::to_string(fs.file.fsFileId()) : "";
        ret << afile << ".a" << (++fileCount[afile]) << sep << fs.cfg << sep << id << sep << Path::simplifyPath(fs.filename()) << std::endl;
    }

    return ret.str();
}

void AnalyzerInformation::close()
{
    if (mOutputStream.is_open()) {
        mOutputStream << "</analyzerinfo>\n";
        mOutputStream.close();
    }
}

std::string AnalyzerInformation::skipAnalysis(const tinyxml2::XMLDocument &analyzerInfoDoc, std::size_t hash, std::list<ErrorMessage> &errors)
{
    const tinyxml2::XMLElement * const rootNode = analyzerInfoDoc.FirstChildElement();
    if (rootNode == nullptr)
        return "no root node found";

    if (strcmp(rootNode->Name(), "analyzerinfo") != 0)
        return "unexpected root node";

    const char * const attr = rootNode->Attribute("hash");
    if (!attr)
        return "no 'hash' attribute found";
    if (attr != std::to_string(hash))
        return "hash mismatch";

    for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "error") != 0)
            continue;

        // TODO: discarding results on internalError doesn't make sense since that won't fix itself
        // Check for invalid license error or internal error, in which case we should retry analysis
        static const std::array<const char*, 3> s_ids{
            "premium-invalidLicense",
            "premium-internalError",
            "internalError"
        };
        for (const auto* id : s_ids)
        {
            // cppcheck-suppress useStlAlgorithm
            if (e->Attribute("id", id)) {
                errors.clear();
                return std::string("'") + id + "' encountered";
            }
        }

        errors.emplace_back(e);
    }

    return "";
}

std::string AnalyzerInformation::getAnalyzerInfoFileFromFilesTxt(std::istream& filesTxt, const std::string &sourcefile, const std::string &cfg, int fsFileId)
{
    std::string line;
    while (std::getline(filesTxt,line)) {
        AnalyzerInformation::Info filesTxtInfo;
        if (!filesTxtInfo.parse(line))
            continue; // TODO: report error?
        if (endsWith(sourcefile, filesTxtInfo.sourceFile) && filesTxtInfo.cfg == cfg && filesTxtInfo.fsFileId == fsFileId)
            return filesTxtInfo.afile;
    }
    return "";
}

std::string AnalyzerInformation::getAnalyzerInfoFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg, std::size_t fsFileId)
{
    std::ifstream fin(Path::join(buildDir, "files.txt"));
    if (fin.is_open()) {
        const std::string& ret = getAnalyzerInfoFileFromFilesTxt(fin, sourcefile, cfg, fsFileId);
        if (!ret.empty())
            return Path::join(buildDir, ret);
    }

    const std::string::size_type pos = sourcefile.rfind('/');
    std::string filename;
    if (pos == std::string::npos)
        filename = sourcefile;
    else
        filename = sourcefile.substr(pos + 1);
    // TODO: is this correct? the above code will return files ending in '.aN'. Also does not consider the ID
    return Path::join(buildDir, std::move(filename)) + ".analyzerinfo";
}

bool AnalyzerInformation::analyzeFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg, std::size_t fsFileId, std::size_t hash, std::list<ErrorMessage> &errors, bool debug)
{
    if (mOutputStream.is_open())
        throw std::runtime_error("analyzer information file is already open");

    if (buildDir.empty() || sourcefile.empty())
        return true;

    const std::string analyzerInfoFile = AnalyzerInformation::getAnalyzerInfoFile(buildDir,sourcefile,cfg,fsFileId);

    {
        tinyxml2::XMLDocument analyzerInfoDoc;
        const tinyxml2::XMLError xmlError = analyzerInfoDoc.LoadFile(analyzerInfoFile.c_str());
        if (xmlError == tinyxml2::XML_SUCCESS) {
            const std::string err = skipAnalysis(analyzerInfoDoc, hash, errors);
            if (err.empty()) {
                if (debug)
                    std::cout << "skipping analysis - loaded " << errors.size() << " cached finding(s) from '" << analyzerInfoFile << "' for '" << sourcefile <<  "'" << std::endl;
                return false;
            }
            if (debug) {
                std::cout << "discarding cached result from '" << analyzerInfoFile << "' for '" << sourcefile << "' - " << err << std::endl;
            }
        }
        else if (xmlError != tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
            if (debug)
                std::cout << "discarding cached result - failed to load '" << analyzerInfoFile << "' for '" << sourcefile << "' (" << tinyxml2::XMLDocument::ErrorIDToName(xmlError) << ")" << std::endl;
        }
        else if (debug)
            std::cout << "no cached result '" << analyzerInfoFile << "' for '" << sourcefile << "' found" << std::endl;
    }

    mOutputStream.open(analyzerInfoFile);
    if (!mOutputStream.is_open())
        throw std::runtime_error("failed to open '" + analyzerInfoFile + "'");
    mOutputStream << "<?xml version=\"1.0\"?>\n";
    mOutputStream << "<analyzerinfo hash=\"" << hash << "\">\n";

    return true;
}

void AnalyzerInformation::reportErr(const ErrorMessage &msg)
{
    if (mOutputStream.is_open())
        mOutputStream << msg.toXML() << '\n';
}

void AnalyzerInformation::setFileInfo(const std::string &check, const std::string &fileInfo)
{
    if (mOutputStream.is_open() && !fileInfo.empty())
        mOutputStream << "  <FileInfo check=\"" << check << "\">\n" << fileInfo << "  </FileInfo>\n";
}

// TODO: report detailed errors?
bool AnalyzerInformation::Info::parse(const std::string& filesTxtLine) {
    const std::string::size_type sep1 = filesTxtLine.find(sep);
    if (sep1 == std::string::npos)
        return false;
    const std::string::size_type sep2 = filesTxtLine.find(sep, sep1+1);
    if (sep2 == std::string::npos)
        return false;
    const std::string::size_type sep3 = filesTxtLine.find(sep, sep2+1);
    if (sep3 == std::string::npos)
        return false;

    if (sep3 == sep2 + 1)
        fsFileId = 0;
    else {
        try {
            fsFileId = std::stoi(filesTxtLine.substr(sep2+1, sep3-sep2-1));
        } catch (const std::exception&) {
            return false;
        }
    }

    afile = filesTxtLine.substr(0, sep1);
    cfg = filesTxtLine.substr(sep1+1, sep2-sep1-1);
    sourceFile = filesTxtLine.substr(sep3+1);
    return true;
}

std::string AnalyzerInformation::processFilesTxt(const std::string& buildDir, const std::function<void(const char* checkattr, const tinyxml2::XMLElement* e, const Info& filesTxtInfo)>& handler, bool debug)
{
    const std::string filesTxt(buildDir + "/files.txt");
    std::ifstream fin(filesTxt.c_str());
    std::string filesTxtLine;
    while (std::getline(fin, filesTxtLine)) {
        AnalyzerInformation::Info filesTxtInfo;
        if (!filesTxtInfo.parse(filesTxtLine))
            return "failed to parse '" + filesTxtLine + "' from '" + filesTxt + "'";

        if (filesTxtInfo.afile.empty())
            return "empty afile from '" + filesTxt + "'";

        const std::string xmlfile = buildDir + '/' + filesTxtInfo.afile;

        tinyxml2::XMLDocument doc;
        const tinyxml2::XMLError error = doc.LoadFile(xmlfile.c_str());
        if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
            /* FIXME: this can currently not be reported as an error because:
             * - --clang does not generate any analyzer information - see #14456
             * - markup files might not generate analyzer information
             * - files with preprocessor errors might not generate analyzer information
             */
            if (debug)
                std::cout << "'" + xmlfile + "' from '" + filesTxt + "' not found";
            continue;
        }

        if (error != tinyxml2::XML_SUCCESS)
            return "failed to load '" + xmlfile + "' from '" + filesTxt + "'";

        const tinyxml2::XMLElement * const rootNode = doc.FirstChildElement();
        if (rootNode == nullptr)
            return "no root node found in '" + xmlfile + "' from '" + filesTxt + "'";

        if (strcmp(rootNode->Name(), "analyzerinfo") != 0)
            return "unexpected root node in '" + xmlfile + "' from '" + filesTxt + "'";

        for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
            if (std::strcmp(e->Name(), "FileInfo") != 0)
                continue;
            const char *checkattr = e->Attribute("check");
            if (checkattr == nullptr) {
                if (debug)
                    std::cout << "'check' attribute missing in 'FileInfo' in '" << xmlfile << "' from '" << filesTxt + "'";
                continue;
            }
            handler(checkattr, e, filesTxtInfo);
        }
    }

    // TODO: error on empty file?
    return "";
}
