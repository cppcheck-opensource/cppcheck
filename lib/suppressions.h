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
#ifndef suppressionsH
#define suppressionsH
//---------------------------------------------------------------------------

#include "config.h"

#include <cstddef>
#include <cstdint>
#include <istream>
#include <list>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>

class TokenList;
class ErrorMessage;
enum class Certainty : std::uint8_t;
class FileWithDetails;
class Settings;

/// @addtogroup Core
/// @{

/** @brief class for handling suppressions */
class CPPCHECKLIB SuppressionList {
public:

    enum class Type : std::uint8_t {
        unique, file, block, blockBegin, blockEnd, macro
    };

    struct CPPCHECKLIB ErrorMessage {
        std::size_t hash;
        std::string errorId;
        void setFileName(std::string s);
        const std::string &getFileName() const {
            return mFileName;
        }
        int lineNumber; // TODO: need to be unsigned
        Certainty certainty;
        std::string symbolNames;
        std::set<std::string> macroNames;

        static SuppressionList::ErrorMessage fromErrorMessage(const ::ErrorMessage &msg, const std::set<std::string> &macroNames);
    private:
        std::string mFileName;
    };

    struct CPPCHECKLIB Suppression {
        Suppression() = default;
        Suppression(std::string id, std::string file, int line=NO_LINE) : errorId(std::move(id)), fileName(std::move(file)), lineNumber(line) {}

        bool operator<(const Suppression &other) const {
            if (errorId != other.errorId)
                return errorId < other.errorId;
            if (lineNumber < other.lineNumber)
                return true;
            if (fileName != other.fileName)
                return fileName < other.fileName;
            if (symbolName != other.symbolName)
                return symbolName < other.symbolName;
            if (macroName != other.macroName)
                return macroName < other.macroName;
            if (hash != other.hash)
                return hash < other.hash;
            if (thisAndNextLine != other.thisAndNextLine)
                return thisAndNextLine;
            return false;
        }

        bool operator==(const Suppression &other) const {
            if (errorId != other.errorId)
                return false;
            if (lineNumber < other.lineNumber)
                return false;
            if (fileName != other.fileName)
                return false;
            if (symbolName != other.symbolName)
                return false;
            if (macroName != other.macroName)
                return false;
            if (hash != other.hash)
                return false;
            if (type != other.type)
                return false;
            if (lineBegin != other.lineBegin)
                return false;
            if (lineEnd != other.lineEnd)
                return false;
            return true;
        }

        /**
         * Parse inline suppression in comment
         * @param comment the full comment text
         * @param errorMessage output parameter for error message (wrong suppression attribute)
         * @return true if it is a inline comment.
         */
        bool parseComment(std::string comment, std::string *errorMessage);

        enum class Result : std::uint8_t {
            None,
            Checked,
            Matched
        };

        Result isSuppressed(const ErrorMessage &errmsg) const;

        bool isMatch(const ErrorMessage &errmsg);

        bool isWildcard() const {
            return fileName.find_first_of("?*") != std::string::npos;
        }

        bool isLocal() const {
            return !fileName.empty() && !isWildcard();
        }

        bool isSameParameters(const Suppression &other) const {
            return errorId == other.errorId &&
                   fileName == other.fileName &&
                   lineNumber == other.lineNumber &&
                   symbolName == other.symbolName &&
                   hash == other.hash &&
                   thisAndNextLine == other.thisAndNextLine;
        }

        std::string toString() const;

        std::string errorId;
        std::string fileName;
        std::string extraComment;
        // TODO: use simplecpp::Location?
        int fileIndex{};
        int lineNumber = NO_LINE; // TODO: needs to be unsigned
        int lineBegin = NO_LINE;
        int lineEnd = NO_LINE;
        int column{};
        Type type = Type::unique;
        std::string symbolName;
        std::string macroName;
        std::size_t hash{};
        bool thisAndNextLine{}; // Special case for backwards compatibility: { // cppcheck-suppress something
        bool matched{}; /** This suppression was fully matched in an isSuppressed() call */
        bool checked{}; /** This suppression applied to code which was being analyzed but did not match the error in an isSuppressed() call */
        bool isInline{};
        bool isPolyspace{};

        enum : std::int8_t { NO_LINE = -1 };
    };

    /**
     * @brief Don't show errors listed in the file.
     * @param istr Open file stream where errors can be read.
     * @return error message. empty upon success
     */
    std::string parseFile(std::istream &istr);

    /**
     * @brief Don't show errors listed in the file.
     * @param filename file name
     * @return error message. empty upon success
     */
    std::string parseXmlFile(const char *filename);

    /**
     * Parse multi inline suppression in comment
     * @param comment the full comment text
     * @param errorMessage output parameter for error message (wrong suppression attribute)
     * @return empty vector if something wrong.
     */
    static std::vector<Suppression> parseMultiSuppressComment(const std::string &comment, std::string *errorMessage);

    /**
     * Create a Suppression object from a suppression line
     * @param line The line to parse.
     * @return a suppression object
     */
    static Suppression parseLine(const std::string &line);

    /**
     * @brief Don't show the given error.
     * @param line Description of error to suppress (in id:file:line format).
     * @return error message. empty upon success
     */
    std::string addSuppressionLine(const std::string &line);

    /**
     * @brief Don't show this error. File and/or line are optional. In which case
     * the errorId alone is used for filtering.
     * @param suppression suppression details
     * @return error message. empty upon success
     */
    std::string addSuppression(Suppression suppression);

    /**
     * @brief Combine list of suppressions into the current suppressions.
     * @param suppressions list of suppression details
     * @return error message. empty upon success
     */
    std::string addSuppressions(std::list<Suppression> suppressions);

    /**
     * @brief Updates the state of the given suppression.
     * @param suppression the suppression to update
     * @return true if suppression to update was found
     */
    bool updateSuppressionState(const SuppressionList::Suppression& suppression);

    /**
     * @brief Returns true if this message should not be shown to the user.
     * @param errmsg error message
     * @param global use global suppressions
     * @return true if this error is suppressed.
     */
    bool isSuppressed(const ErrorMessage &errmsg, bool global = true);

    /**
     * @brief Returns true if this message is "explicitly" suppressed. The suppression "id" must match textually exactly.
     * @param errmsg error message
     * @param global use global suppressions
     * @return true if this error is explicitly suppressed.
     */
    bool isSuppressedExplicitly(const ErrorMessage &errmsg, bool global = true);

    /**
     * @brief Returns true if this message should not be shown to the user.
     * @param errmsg error message
     * @return true if this error is suppressed.
     */
    bool isSuppressed(const ::ErrorMessage &errmsg, const std::set<std::string>& macroNames);

    /**
     * @brief Create an xml dump of suppressions
     * @param out stream to write XML to
     */
    void dump(std::ostream &out, const std::string& filePath = {}) const;

    /**
     * @brief Returns list of unmatched local (per-file) suppressions.
     * @return list of unmatched suppressions
     */
    std::list<Suppression> getUnmatchedLocalSuppressions(const FileWithDetails &file) const;

    /**
     * @brief Returns list of unmatched global (glob pattern) suppressions.
     * @return list of unmatched suppressions
     */
    std::list<Suppression> getUnmatchedGlobalSuppressions() const;

    /**
     * @brief Returns list of unmatched inline suppressions.
     * @return list of unmatched suppressions
     */
    std::list<Suppression> getUnmatchedInlineSuppressions() const;

    /**
     * @brief Returns list of all suppressions.
     * @return list of suppressions
     */
    std::list<Suppression> getSuppressions() const;

    /**
     * @brief Marks Inline Suppressions as checked if source line is in the token stream
     */
    void markUnmatchedInlineSuppressionsAsChecked(const TokenList &tokenlist);

private:
    mutable std::mutex mSuppressionsSync;
    /** @brief List of error which the user doesn't want to see. */
    std::list<Suppression> mSuppressions;
};

struct Suppressions
{
    /** @brief suppress message (--suppressions) */
    SuppressionList nomsg;
    /** @brief suppress exitcode */
    SuppressionList nofail;
};

namespace polyspace {

    enum class CommentKind : std::uint8_t {
        Invalid, Regular, Begin, End,
    };

    class CPPCHECKLIB Parser {
    public:
        Parser() = delete;
        explicit Parser(const Settings &settings);

        std::list<SuppressionList::Suppression> parse(const std::string &comment, int line, const std::string &filename) const;

        static int parseRange(const std::string& comment, std::string::size_type& pos);
        static std::vector<std::pair<std::string, std::string>> parseFamilyRules(const std::string& comment, std::string::size_type& pos);

    private:
        std::set<std::string> parseIds(const std::string& comment, std::string::size_type& pos) const;

        static CommentKind parseKind(const std::string& comment, std::string::size_type& pos);

        std::map<std::string, std::string> mFamilyMap;
    };

    bool CPPCHECKLIB isPolyspaceComment(const std::string &comment);

}

/// @}
//---------------------------------------------------------------------------
#endif // suppressionsH
