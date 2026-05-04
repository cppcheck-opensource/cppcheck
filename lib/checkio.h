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
#ifndef checkioH
#define checkioH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <cstdint>
#include <ostream>
#include <string>

class Function;
class Settings;
class Token;
class Variable;
class ErrorLogger;
class Tokenizer;
enum class Severity : std::uint8_t;

/// @addtogroup Checks
/// @{

/** @brief %Check input output operations. */
class CPPCHECKLIB CheckIO : public Check {
    friend class TestIO;

public:
    /** @brief This constructor is used when registering CheckIO */
    CheckIO() : Check(myName()) {}

private:
    /** @brief This constructor is used when running checks. */
    CheckIO(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** @brief Run checks on the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** @brief %Check for missusage of std::cout */
    void checkCoutCerrMisusage();

    /** @brief %Check usage of files*/
    void checkFileUsage();

    /** @brief scanf can crash if width specifiers are not used */
    void invalidScanf();

    /** @brief %Checks type and number of arguments given to functions like printf or scanf*/
    void checkWrongPrintfScanfArguments();

    class ArgumentInfo {
    public:
        ArgumentInfo(const Token *arg, const Settings &settings, bool _isCPP);
        ~ArgumentInfo();

        ArgumentInfo(const ArgumentInfo &) = delete;
        ArgumentInfo& operator= (const ArgumentInfo &) = delete;

        bool isArrayOrPointer() const;
        bool isComplexType() const;
        bool isKnownType() const;
        bool isStdVectorOrString();
        bool isStdContainer(const Token *tok);
        bool isLibraryType(const Settings &settings) const;

        const Variable* variableInfo{};
        const Token* typeToken{};
        const Function* functionInfo{};
        Token* tempToken{};
        bool element{};
        bool _template{};
        bool address{};
        bool isCPP{};
    };

    void checkFormatString(const Token * tok,
                           const Token * formatStringTok,
                           const Token * argListTok,
                           bool scan,
                           bool scanf_s);

    // Reporting errors..
    void coutCerrMisusageError(const Token* tok, const std::string& streamName);
    void fflushOnInputStreamError(const Token *tok, const std::string &varname);
    void ioWithoutPositioningError(const Token *tok);
    void readWriteOnlyFileError(const Token *tok);
    void writeReadOnlyFileError(const Token *tok);
    void useClosedFileError(const Token *tok);
    void seekOnAppendedFileError(const Token *tok);
    void ftellFileError(const Token *tok);
    void incompatibleFileOpenError(const Token *tok, const std::string &filename);
    void invalidScanfError(const Token *tok);
    void wrongPrintfScanfArgumentsError(const Token* tok,
                                        const std::string &functionName,
                                        nonneg int numFormat,
                                        nonneg int numFunction);
    void wrongPrintfScanfPosixParameterPositionError(const Token* tok, const std::string& functionName,
                                                     nonneg int index, nonneg int numFunction);
    void invalidScanfArgTypeError_s(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidScanfArgTypeError_int(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo, bool isUnsigned);
    void invalidScanfArgTypeError_float(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_s(const Token* tok, nonneg int numFormat, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_n(const Token* tok, nonneg int numFormat, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_p(const Token* tok, nonneg int numFormat, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_uint(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_sint(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_float(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidLengthModifierError(const Token* tok, nonneg int numFormat, const std::string& modifier);
    void invalidScanfFormatWidthError(const Token* tok, nonneg int numFormat, int width, const Variable *var, const std::string& specifier);
    static void argumentType(std::ostream & os, const ArgumentInfo * argInfo);
    static Severity getSeverity(const ArgumentInfo *argInfo);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    static std::string myName() {
        return "IO using format string";
    }

    std::string classInfo() const override {
        return "Check format string input/output operations.\n"
               "- Bad usage of the function 'sprintf' (overlapping data)\n"
               "- Missing or wrong width specifiers in 'scanf' format string\n"
               "- Use a file that has been closed\n"
               "- File input/output without positioning results in undefined behaviour\n"
               "- Using 'ftell' on a file opened in text mode\n"
               "- Read to a file that has only been opened for writing (or vice versa)\n"
               "- Repositioning operation on a file opened in append mode\n"
               "- The same file can't be open for read and write at the same time on different streams\n"
               "- Using fflush() on an input stream\n"
               "- Invalid usage of output stream. For example: 'std::cout << std::cout;'\n"
               "- Wrong number of arguments given to 'printf' or 'scanf;'\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkioH
