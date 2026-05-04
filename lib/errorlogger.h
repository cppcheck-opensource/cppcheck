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
#ifndef errorloggerH
#define errorloggerH
//---------------------------------------------------------------------------

#include "config.h"
#include "errortypes.h"

#include <cstdint>
#include <ctime>
#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>

class Token;
class TokenList;
enum class ReportType : std::uint8_t;
enum class Color : std::uint8_t;

namespace tinyxml2 {
    class XMLElement;
}

/// @addtogroup Core
/// @{

/**
 * Wrapper for error messages, provided by reportErr()
 */
class CPPCHECKLIB ErrorMessage {
public:
    /**
     * File name and line number.
     * Internally paths are stored with / separator. When getting the filename
     * it is by default converted to native separators.
     */
    class CPPCHECKLIB WARN_UNUSED FileLocation {
    public:
        FileLocation(const std::string &file, int line, unsigned int column);
        FileLocation(const std::string &file, std::string info, int line, unsigned int column);
        FileLocation(const Token* tok, const TokenList* tokenList);
        FileLocation(const Token* tok, std::string info, const TokenList* tokenList);

        /**
         * Return the filename.
         * @param convert If true convert path to native separators.
         * @return filename.
         */
        std::string getfile(bool convert = true) const;

        /**
         * Filename with the whole path (no --rp)
         * @param convert If true convert path to native separators.
         * @return filename.
         */
        std::string getOrigFile(bool convert = true) const;

        /**
         * Set the filename.
         * @param file Filename to set.
         */
        void setfile(std::string file);

        /**
         * @return the location as a string. Format: [file:line]
         */
        std::string stringify(bool addcolumn = false) const;

        unsigned int fileIndex;
        int line; // TODO: should be unsigned
        unsigned int column;

        const std::string& getinfo() const {
            return mInfo;
        }

    private:
        std::string mOrigFileName;
        std::string mFileName;
        std::string mInfo;
    };

    ErrorMessage(std::list<FileLocation> callStack,
                 std::string file1,
                 Severity severity,
                 const std::string &msg,
                 std::string id, Certainty certainty);
    ErrorMessage(std::list<FileLocation> callStack,
                 std::string file1,
                 Severity severity,
                 const std::string &msg,
                 std::string id,
                 const CWE &cwe,
                 Certainty certainty);
    ErrorMessage(const std::list<const Token*>& callstack,
                 const TokenList* list,
                 Severity severity,
                 std::string id,
                 const std::string& msg,
                 Certainty certainty);
    ErrorMessage(const std::list<const Token*>& callstack,
                 const TokenList* list,
                 Severity severity,
                 std::string id,
                 const std::string& msg,
                 const CWE &cwe,
                 Certainty certainty);
    ErrorMessage(ErrorPath errorPath,
                 const TokenList *tokenList,
                 Severity severity,
                 const char id[],
                 const std::string &msg,
                 const CWE &cwe,
                 Certainty certainty);
    ErrorMessage();
    explicit ErrorMessage(const tinyxml2::XMLElement * errmsg);

    /**
     * Format the error message in XML format
     */
    std::string toXML() const;

    static std::string getXMLHeader(std::string productName, int xmlVersion = 2);
    static std::string getXMLFooter(int xmlVersion);

    /**
     * Format the error message into a string.
     * @param verbose use verbose message
     * @param templateFormat Empty string to use default output format
     * or template to be used. E.g. "{file}:{line},{severity},{id},{message}"
     * @param templateLocation Format Empty string to use default output format
     * or template to be used. E.g. "{file}:{line},{info}"
     * @return formatted string
     */
    std::string toString(bool verbose,
                         const std::string &templateFormat,
                         const std::string &templateLocation) const;

    std::string serialize() const;
    /**
     * @throws InternalError thrown if deserialization failed
     */
    void deserialize(const std::string &data);

    std::list<FileLocation> callStack;
    std::string id;

    /** For GUI rechecking; source file (not header) */
    std::string file0;

    Severity severity;
    CWE cwe;
    Certainty certainty;

    /** remark from REMARK comment */
    std::string remark;

    /** misra/autosar/certc classification/level */
    std::string classification;

    /** misra/autosar/certc guideline */
    std::string guideline;

    /** Warning hash */
    std::size_t hash{};

    /** set short and verbose messages */
    void setmsg(const std::string &msg);

    /** Short message (single line short message) */
    const std::string &shortMessage() const {
        return mShortMessage;
    }

    /** Verbose message (may be the same as the short message) */
    const std::string &verboseMessage() const {
        return mVerboseMessage;
    }

    /** Symbol names */
    const std::string &symbolNames() const {
        return mSymbolNames;
    }

    static ErrorMessage fromInternalError(const InternalError &internalError, const TokenList *tokenList, const std::string &filename, const std::string& msg = "");

private:
    static std::string fixInvalidChars(const std::string& raw);

    /** Short message */
    std::string mShortMessage;

    /** Verbose message */
    std::string mVerboseMessage;

    /** symbol names */
    std::string mSymbolNames;
};

/**
 * @brief This is an interface, which the class responsible of error logging
 * should implement.
 */
class CPPCHECKLIB ErrorLogger {
public:
    ErrorLogger() = default;
    virtual ~ErrorLogger() = default;

    /**
     * Information about progress is directed here.
     * Override this to receive the progress messages.
     *
     * @param outmsg Message to show e.g. "Checking main.cpp..."
     */
    virtual void reportOut(const std::string &outmsg, Color c) = 0;

    /**
     * Information about found errors and warnings is directed
     * here. Override this to receive the errormessages.
     *
     * @param msg Location and other information about the found error.
     */
    virtual void reportErr(const ErrorMessage &msg) = 0;

    /**
     * Information about file metrics reported by addons.
     *
     * @param metric The file metric to report, as an XML object.
     */
    virtual void reportMetric(const std::string &metric) = 0;

    /**
     * Report progress to client
     * @param filename main file that is checked
     * @param stage for example preprocess / tokenize / simplify / check
     * @param value progress value (0-100)
     */
    virtual void reportProgress(const std::string &filename, const char stage[], const std::size_t value) {
        (void)filename;
        (void)stage;
        (void)value;
    }

    static std::string callStackToString(const std::list<ErrorMessage::FileLocation> &callStack, bool addcolumn = false);

    /**
     * Convert XML-sensitive characters into XML entities
     * @param str The input string containing XML-sensitive characters
     * @return The output string containing XML entities
     */
    static std::string toxml(const std::string &str);

    static std::string plistHeader(const std::string &version, const std::vector<std::string> &files);
    static std::string plistData(const ErrorMessage &msg);
    static const char *plistFooter() {
        return " </array>\r\n"
               "</dict>\r\n"
               "</plist>";
    }

    static bool isCriticalErrorId(const std::string& id) {
        return mCriticalErrorIds.count(id) != 0;
    }

private:
    static const std::set<std::string> mCriticalErrorIds;
};

/// RAII class for reporting progress messages
class CPPCHECKLIB ProgressReporter {
public:
    ProgressReporter(ErrorLogger& e, int reportProgressInterval, std::string filename, std::string stage) :
        mErrorLogger(e),
        mReportProgressInterval(reportProgressInterval),
        mFilename(std::move(filename)),
        mStage(std::move(stage)) {
        report(0);
    }

    ~ProgressReporter() {
        if (mReportProgressInterval < 0)
            return;
        mErrorLogger.reportProgress(mFilename, mStage.c_str(), 100);
    }

    void report(int value) {
        if (mReportProgressInterval < 0 || value == mLastValue)
            return;
        const std::time_t t = std::time(nullptr);
        if (t >= mLastTime + mReportProgressInterval) {
            mErrorLogger.reportProgress(mFilename, mStage.c_str(), value);
            mLastTime = t;
            mLastValue = value;
        }
    }

private:
    ErrorLogger& mErrorLogger;
    const int mReportProgressInterval;
    const std::string mFilename;
    const std::string mStage;
    std::time_t mLastTime{0};
    int mLastValue{-1};
};

/** Replace substring. Example replaceStr("1,NR,3", "NR", "2") => "1,2,3" */
std::string replaceStr(std::string s, const std::string &from, const std::string &to);

/** replaces the static parts of the location template **/
CPPCHECKLIB void substituteTemplateFormatStatic(std::string& templateFormat, bool eraseColors = false);

/** replaces the static parts of the location template **/
CPPCHECKLIB void substituteTemplateLocationStatic(std::string& templateLocation, bool eraseColors = false);

/** Get a classification string from the given guideline and reporttype */
CPPCHECKLIB std::string getClassification(const std::string &guideline, ReportType reportType);

/** Get a guideline string froM the given error id, reporttype, mapping and severity */
CPPCHECKLIB std::string getGuideline(const std::string &errId, ReportType reportType,
                                     const std::map<std::string, std::string> &guidelineMapping,
                                     Severity severity);

/** Get a map from cppcheck error ids to guidelines matching the given report type */
CPPCHECKLIB std::map<std::string, std::string> createGuidelineMapping(ReportType reportType);

/// @}
//---------------------------------------------------------------------------
#endif // errorloggerH
