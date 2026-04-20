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
#ifndef cppcheckH
#define cppcheckH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

class TokenList;
struct FileSettings;
class CheckUnusedFunctions;
class Tokenizer;
class FileWithDetails;
class AnalyzerInformation;
class ErrorLogger;
class Settings;
struct Suppressions;
class Preprocessor;
class TimerResults;

namespace simplecpp {
    class TokenList;
    struct Output;
}

/// @addtogroup Core
/// @{

/**
 * @brief This is the base class which will use other classes to do
 * static code analysis for C and C++ code to find possible
 * errors or places that could be improved.
 * Usage: See check() for more info.
 */
class CPPCHECKLIB CppCheck {
    friend class TestCppcheck;

public:
    // exe, args, redirect, output
    using ExecuteCmdFn = std::function<int (std::string,std::vector<std::string>,std::string,std::string&)>;

    /**
     * @brief Constructor.
     */
    CppCheck(const Settings& settings,
             Suppressions& supprs,
             ErrorLogger &errorLogger,
             TimerResults* timerResults,
             bool useGlobalSuppressions,
             ExecuteCmdFn executeCommand);

    /**
     * @brief Destructor.
     */
    ~CppCheck();

    /**
     * @brief This starts the actual checking. Note that you must call
     * parseFromArgs() or settings() and addFile() before calling this.
     * @return amount of errors found or 0 if none were found.
     */

    /**
     * @brief Check the file.
     * This function checks one given file for errors.
     * @param file The file to check.
     * @return amount of errors found or 0 if none were found.
     * @note You must set settings before calling this function (by calling
     *  settings()).
     */
    unsigned int check(const FileWithDetails &file);
    unsigned int check(const FileSettings &fs);

    /**
     * @brief Check the file.
     * This function checks one "virtual" file. The file is not read from
     * the disk but the content is given in @p content. In errors the @p path
     * is used as a filename.
     * @param file The file to check.
     * @param data File content as a buffer.
     * @param size Size of buffer.
     * @return amount of errors found or 0 if none were found.
     * @note You must set settings before calling this function (by calling
     *  settings()).
     */
    unsigned int checkBuffer(const FileWithDetails &file, const char* data, std::size_t size);

    /**
     * @brief Returns current version number as a string.
     * @return version, e.g. "1.38"
     */
    RET_NONNULL static const char * version();

    /**
     * @brief Returns extra version info as a string.
     * This is for returning extra version info, like Git commit id, build
     * time/date etc.
     * @return extra version info, e.g. "04d42151" (Git commit id).
     */
    RET_NONNULL static const char * extraVersion();

    /**
     * @brief Call all "getErrorMessages" in all registered Check classes.
     * Also print out XML header and footer.
     */
    static void getErrorMessages(ErrorLogger &errorlogger);

    void tooManyConfigsError(const std::string &file, int numberOfConfigurations);

    /** Analyse whole program, run this after all TUs has been scanned.
     * This is deprecated and the plan is to remove this when
     * .analyzeinfo is good enough.
     * Return true if an error is reported.
     */
    bool analyseWholeProgram();

    /** analyse whole program use .analyzeinfo files or ctuinfo string */
    unsigned int analyseWholeProgram(const std::string &buildDir, const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const std::string& ctuInfo);

private:
    void purgedConfigurationMessage(const std::string &file, const std::string& configuration);

    bool isPremiumCodingStandardId(const std::string& id) const;

    /**
     * @brief Get dumpfile <rawtokens> contents, this is only public for testing purposes
     */
    std::string getDumpFileContentsRawTokens(const std::vector<std::string>& files, const simplecpp::TokenList& tokens1) const;

    std::string getLibraryDumpData() const;

    /** Analyze all files using clang-tidy */
    void analyseClangTidy(const FileSettings &fileSettings);

#ifdef HAVE_RULES
    /** Are there "simple" rules */
    bool hasRule(const std::string &tokenlist) const;
#endif

    /** @brief There has been an internal error => Report information message */
    void internalError(const std::string &filename, const std::string &msg);

    /**
     * @brief Calculate hash used to detect when a file needs to be reanalyzed.
     *
     * @param preprocessor  Preprocessor used to calculate the hash.
     * @return hash
     */
    std::size_t calculateHash(const Preprocessor &preprocessor, const std::string& filePath = {}) const;

    /**
     * @brief Check a file
     * @param file the file
     * @param cfgname  cfg name
     * @return number of errors found
     */
    unsigned int checkFile(const FileWithDetails& file, const std::string &cfgname);

    void checkPlistOutput(const FileWithDetails& file, const std::vector<std::string>& files);

    /**
     * @brief Check a file using buffer
     * @param file the file
     * @param cfgname  cfg name
     * @param data the data to be read
     * @param size the size of the data to be read
     * @return number of errors found
     */
    unsigned int checkBuffer(const FileWithDetails& file, const std::string &cfgname, const char* data, std::size_t size);

    // TODO: should use simplecpp::OutputList
    using CreateTokenListFn = std::function<simplecpp::TokenList (std::vector<std::string>&, std::list<simplecpp::Output>*)>;

    /**
     * @brief Check a file using stream
     * @param file the file
     * @param cfgname  cfg name
     * @param createTokenList a function to create the simplecpp::TokenList with
     * @return number of errors found
     */
    unsigned int checkInternal(const FileWithDetails& file, const std::string &cfgname, const CreateTokenListFn& createTokenList);

    /**
     * @brief Check normal tokens
     * @param tokenizer tokenizer instance
     * @param analyzerInformation the analyzer information
     */
    void checkNormalTokens(const Tokenizer &tokenizer, AnalyzerInformation* analyzerInformation, const std::string& currentConfig);

    /**
     * Execute addons
     */
    void executeAddons(const std::vector<std::string>& files, const std::string& file0);
    void executeAddons(const std::string &dumpFile, const FileWithDetails& file);

    /**
     * Execute addons
     */
    void executeAddonsWholeProgram(const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const std::string& ctuInfo);

#ifdef HAVE_RULES
    /**
     * @brief Execute rules, if any
     * @param tokenlist token list to use (define / normal / raw)
     * @param list token list
     */
    void executeRules(const std::string &tokenlist, const TokenList &list);
#endif

    unsigned int checkClang(const FileWithDetails &file);

    const Settings& mSettings;
    Suppressions& mSuppressions;

    class CppCheckLogger;
    std::unique_ptr<CppCheckLogger> mLogger;
    /** the internal ErrorLogger */
    ErrorLogger& mErrorLogger;
    /** the ErrorLogger provided to this instance */
    ErrorLogger& mErrorLoggerDirect;
    TimerResults* mTimerResults;

    bool mUseGlobalSuppressions;

    /** File info used for whole program analysis */
    std::list<Check::FileInfo*> mFileInfo;

    /** Callback for executing a shell command (exe, args, output) */
    ExecuteCmdFn mExecuteCommand;

    std::unique_ptr<CheckUnusedFunctions> mUnusedFunctionsCheck;
};

/// @}
//---------------------------------------------------------------------------
#endif // cppcheckH
