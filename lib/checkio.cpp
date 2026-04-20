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

//---------------------------------------------------------------------------
#include "checkio.h"

#include "astutils.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "utils.h"
#include "vfvalue.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

//---------------------------------------------------------------------------

// Register CheckIO..
namespace {
    CheckIO instance;
}

// CVE ID used:
static const CWE CWE119(119U);  // Improper Restriction of Operations within the Bounds of a Memory Buffer
static const CWE CWE398(398U);  // Indicator of Poor Code Quality
static const CWE CWE664(664U);  // Improper Control of a Resource Through its Lifetime
static const CWE CWE685(685U);  // Function Call With Incorrect Number of Arguments
static const CWE CWE686(686U);  // Function Call With Incorrect Argument Type
static const CWE CWE687(687U);  // Function Call With Incorrectly Specified Argument Value
static const CWE CWE704(704U);  // Incorrect Type Conversion or Cast
static const CWE CWE910(910U);  // Use of Expired File Descriptor

//---------------------------------------------------------------------------
//    std::cout << std::cout;
//---------------------------------------------------------------------------
void CheckIO::checkCoutCerrMisusage()
{
    if (mTokenizer->isC())
        return;

    logChecker("CheckIO::checkCoutCerrMisusage"); // c

    const SymbolDatabase * const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "std :: cout|cerr !!.") && tok->next()->astParent() && tok->next()->astParent()->astOperand1() == tok->next()) {
                const Token* tok2 = tok->next();
                while (tok2->astParent() && tok2->astParent()->str() == "<<") {
                    tok2 = tok2->astParent();
                    if (tok2->astOperand2() && Token::Match(tok2->astOperand2()->previous(), "std :: cout|cerr"))
                        coutCerrMisusageError(tok, tok2->astOperand2()->strAt(1));
                }
            }
        }
    }
}

void CheckIO::coutCerrMisusageError(const Token* tok, const std::string& streamName)
{
    reportError(tok, Severity::error, "coutCerrMisusage", "Invalid usage of output stream: '<< std::" + streamName + "'.", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
// fflush(stdin) <- fflush only applies to output streams in ANSI C
// fread(); fwrite(); <- consecutive read/write statements require repositioning in between
// fopen("","r"); fwrite(); <- write to read-only file (or vice versa)
// fclose(); fread(); <- Use closed file
//---------------------------------------------------------------------------
enum class OpenMode : std::uint8_t { CLOSED, READ_MODE, WRITE_MODE, RW_MODE, UNKNOWN_OM };
static OpenMode getMode(const std::string& str)
{
    if (str.find('+', 1) != std::string::npos)
        return OpenMode::RW_MODE;
    if (str.find('w') != std::string::npos || str.find('a') != std::string::npos)
        return OpenMode::WRITE_MODE;
    if (str.find('r') != std::string::npos)
        return OpenMode::READ_MODE;
    return OpenMode::UNKNOWN_OM;
}

namespace {
    struct Filepointer {
        OpenMode mode;
        nonneg int mode_indent{};
        enum class Operation : std::uint8_t {NONE, UNIMPORTANT, READ, WRITE, POSITIONING, OPEN, CLOSE, UNKNOWN_OP} lastOperation = Operation::NONE;
        nonneg int op_indent{};
        enum class AppendMode : std::uint8_t { UNKNOWN_AM, APPEND, APPEND_EX };
        AppendMode append_mode = AppendMode::UNKNOWN_AM;
        std::string filename;
        explicit Filepointer(OpenMode mode_ = OpenMode::UNKNOWN_OM)
            : mode(mode_) {}
    };

    const std::unordered_set<std::string> whitelist = { "clearerr", "feof", "ferror", "fgetpos", "ftell", "setbuf", "setvbuf", "ungetc", "ungetwc" };
}

void CheckIO::checkFileUsage()
{
    const bool windows = mSettings->platform.isWindows();
    const bool printPortability = mSettings->severity.isEnabled(Severity::portability);
    const bool printWarnings = mSettings->severity.isEnabled(Severity::warning);

    std::map<int, Filepointer> filepointers;

    logChecker("CheckIO::checkFileUsage");

    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Variable* var : symbolDatabase->variableList()) {
        if (!var || !var->declarationId() || var->isArray() || !Token::simpleMatch(var->typeStartToken(), "FILE *"))
            continue;

        if (var->isLocal()) {
            if (var->nameToken()->strAt(1) == "(") // initialize by calling "ctor"
                filepointers.emplace(var->declarationId(), Filepointer(OpenMode::UNKNOWN_OM));
            else
                filepointers.emplace(var->declarationId(), Filepointer(OpenMode::CLOSED));
        } else {
            filepointers.emplace(var->declarationId(), Filepointer(OpenMode::UNKNOWN_OM));
            // TODO: If all fopen calls we find open the file in the same type, we can set Filepointer::mode
        }
    }

    for (const Scope * scope : symbolDatabase->functionScopes) {
        int indent = 0;
        for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%name% (") && isUnevaluated(tok)) {
                tok = tok->linkAt(1);
                continue;
            }
            if (tok->str() == "{")
                indent++;
            else if (tok->str() == "}") {
                indent--;
                for (std::pair<const int, Filepointer>& filepointer : filepointers) {
                    if (indent < filepointer.second.mode_indent) {
                        filepointer.second.mode_indent = 0;
                        filepointer.second.mode = OpenMode::UNKNOWN_OM;
                    }
                    if (indent < filepointer.second.op_indent) {
                        filepointer.second.op_indent = 0;
                        filepointer.second.lastOperation = Filepointer::Operation::UNKNOWN_OP;
                    }
                }
            } else if (tok->str() == "return" || tok->str() == "continue" || tok->str() == "break" || mSettings->library.isnoreturn(tok)) { // Reset upon return, continue or break
                for (std::pair<const int, Filepointer>& filepointer : filepointers) {
                    filepointer.second.mode_indent = 0;
                    filepointer.second.mode = OpenMode::UNKNOWN_OM;
                    filepointer.second.op_indent = 0;
                    filepointer.second.lastOperation = Filepointer::Operation::UNKNOWN_OP;
                }
            } else if (Token::Match(tok, "%var% =") &&
                       (tok->strAt(2) != "fopen" && tok->strAt(2) != "freopen" && tok->strAt(2) != "tmpfile" &&
                        (windows ? (tok->str() != "_wfopen" && tok->str() != "_wfreopen") : true))) {
                const auto i = filepointers.find(tok->varId());
                if (i != filepointers.end()) {
                    i->second.mode = OpenMode::UNKNOWN_OM;
                    i->second.lastOperation = Filepointer::Operation::UNKNOWN_OP;
                }
            } else if (Token::Match(tok, "%name% (") && tok->previous() && (!tok->previous()->isName() || Token::Match(tok->previous(), "return|throw"))) {
                std::string mode;
                const Token* fileTok = nullptr;
                const Token* fileNameTok = nullptr;
                Filepointer::Operation operation = Filepointer::Operation::NONE;

                if ((tok->str() == "fopen" || tok->str() == "freopen" || tok->str() == "tmpfile" ||
                     (windows && (tok->str() == "_wfopen" || tok->str() == "_wfreopen"))) &&
                    tok->strAt(-1) == "=") {
                    if (tok->str() != "tmpfile") {
                        const Token* modeTok = tok->tokAt(2)->nextArgument();
                        if (modeTok && modeTok->tokType() == Token::eString)
                            mode = modeTok->strValue();
                    } else
                        mode = "wb+";
                    fileTok = tok->tokAt(-2);
                    operation = Filepointer::Operation::OPEN;
                    if (Token::Match(tok, "fopen ( %str% ,"))
                        fileNameTok = tok->tokAt(2);
                } else if (windows && Token::Match(tok, "fopen_s|freopen_s|_wfopen_s|_wfreopen_s ( & %name%")) {
                    const Token* modeTok = tok->tokAt(2)->nextArgument()->nextArgument();
                    if (modeTok && modeTok->tokType() == Token::eString)
                        mode = modeTok->strValue();
                    fileTok = tok->tokAt(3);
                    operation = Filepointer::Operation::OPEN;
                } else if ((tok->str() == "rewind" || tok->str() == "fseek" || tok->str() == "fsetpos" || tok->str() == "fflush") ||
                           (windows && tok->str() == "_fseeki64")) {
                    fileTok = tok->tokAt(2);
                    if (printPortability && fileTok && tok->str() == "fflush") {
                        if (fileTok->str() == "stdin")
                            fflushOnInputStreamError(tok, fileTok->str());
                        else {
                            const Filepointer& f = filepointers[fileTok->varId()];
                            if (f.mode == OpenMode::READ_MODE)
                                fflushOnInputStreamError(tok, fileTok->str());
                        }
                    }
                    operation = Filepointer::Operation::POSITIONING;
                } else if (tok->str() == "fgetc" || tok->str() == "fgetwc" ||
                           tok->str() == "fgets" || tok->str() == "fgetws" || tok->str() == "fread" ||
                           tok->str() == "fscanf" || tok->str() == "fwscanf" || tok->str() == "getc" ||
                           (windows && (tok->str() == "fscanf_s" || tok->str() == "fwscanf_s"))) {
                    if (tok->str().find("scanf") != std::string::npos)
                        fileTok = tok->tokAt(2);
                    else
                        fileTok = tok->linkAt(1)->previous();
                    operation = Filepointer::Operation::READ;
                } else if (tok->str() == "fputc" || tok->str() == "fputwc" ||
                           tok->str() == "fputs" || tok->str() == "fputws" || tok->str() == "fwrite" ||
                           tok->str() == "fprintf" || tok->str() == "fwprintf" || tok->str() == "putcc" ||
                           (windows && (tok->str() == "fprintf_s" || tok->str() == "fwprintf_s"))) {
                    if (tok->str().find("printf") != std::string::npos)
                        fileTok = tok->tokAt(2);
                    else
                        fileTok = tok->linkAt(1)->previous();
                    operation = Filepointer::Operation::WRITE;
                } else if (tok->str() == "fclose") {
                    fileTok = tok->tokAt(2);
                    operation = Filepointer::Operation::CLOSE;
                } else if (whitelist.find(tok->str()) != whitelist.end()) {
                    fileTok = tok->tokAt(2);
                    if ((tok->str() == "ungetc" || tok->str() == "ungetwc") && fileTok)
                        fileTok = fileTok->nextArgument();
                    operation = Filepointer::Operation::UNIMPORTANT;
                } else if (!Token::Match(tok, "if|for|while|catch|switch") && !mSettings->library.isFunctionConst(tok->str(), true)) {
                    const Token* const end2 = tok->linkAt(1);
                    if (scope->functionOf && scope->functionOf->isClassOrStruct() && !scope->function->isStatic() && ((tok->strAt(-1) != "::" && tok->strAt(-1) != ".") || tok->strAt(-2) == "this")) {
                        if (!tok->function() || (tok->function()->nestedIn && tok->function()->nestedIn->isClassOrStruct())) {
                            for (std::pair<const int, Filepointer>& filepointer : filepointers) {
                                const Variable* var = symbolDatabase->getVariableFromVarId(filepointer.first);
                                if (!var || !(var->isLocal() || var->isGlobal() || var->isStatic())) {
                                    filepointer.second.mode = OpenMode::UNKNOWN_OM;
                                    filepointer.second.mode_indent = 0;
                                    filepointer.second.op_indent = indent;
                                    filepointer.second.lastOperation = Filepointer::Operation::UNKNOWN_OP;
                                }
                            }
                            continue;
                        }
                    }
                    for (const Token* tok2 = tok->tokAt(2); tok2 != end2; tok2 = tok2->next()) {
                        if (tok2->varId() && filepointers.find(tok2->varId()) != filepointers.end()) {
                            fileTok = tok2;
                            operation = Filepointer::Operation::UNKNOWN_OP; // Assume that repositioning was last operation and that the file is opened now
                            break;
                        }
                    }
                }

                while (Token::Match(fileTok, "%name% ."))
                    fileTok = fileTok->tokAt(2);

                if (!fileTok || !fileTok->varId() || fileTok->strAt(1) == "[")
                    continue;

                // function call indicates: Its a File
                filepointers.emplace(fileTok->varId(), Filepointer(OpenMode::UNKNOWN_OM));

                Filepointer& f = filepointers[fileTok->varId()];

                switch (operation) {
                case Filepointer::Operation::OPEN:
                    if (fileNameTok) {
                        for (auto it = filepointers.cbegin(); it != filepointers.cend(); ++it) {
                            const Filepointer &fptr = it->second;
                            if (fptr.filename == fileNameTok->str() && (fptr.mode == OpenMode::RW_MODE || fptr.mode == OpenMode::WRITE_MODE))
                                incompatibleFileOpenError(tok, fileNameTok->str());
                        }

                        f.filename = fileNameTok->str();
                    }

                    f.mode = getMode(mode);
                    if (mode.find('a') != std::string::npos) {
                        if (f.mode == OpenMode::RW_MODE)
                            f.append_mode = Filepointer::AppendMode::APPEND_EX;
                        else
                            f.append_mode = Filepointer::AppendMode::APPEND;
                    } else
                        f.append_mode = Filepointer::AppendMode::UNKNOWN_AM;
                    f.mode_indent = indent;
                    break;
                case Filepointer::Operation::POSITIONING:
                    if (f.mode == OpenMode::CLOSED)
                        useClosedFileError(tok);
                    else if (f.append_mode == Filepointer::AppendMode::APPEND && tok->str() != "fflush" && printWarnings)
                        seekOnAppendedFileError(tok);
                    break;
                case Filepointer::Operation::READ:
                    if (f.mode == OpenMode::CLOSED)
                        useClosedFileError(tok);
                    else if (f.mode == OpenMode::WRITE_MODE)
                        readWriteOnlyFileError(tok);
                    else if (f.lastOperation == Filepointer::Operation::WRITE)
                        ioWithoutPositioningError(tok);
                    break;
                case Filepointer::Operation::WRITE:
                    if (f.mode == OpenMode::CLOSED)
                        useClosedFileError(tok);
                    else if (f.mode == OpenMode::READ_MODE)
                        writeReadOnlyFileError(tok);
                    else if (f.lastOperation == Filepointer::Operation::READ)
                        ioWithoutPositioningError(tok);
                    break;
                case Filepointer::Operation::CLOSE:
                    if (f.mode == OpenMode::CLOSED)
                        useClosedFileError(tok);
                    else
                        f.mode = OpenMode::CLOSED;
                    f.mode_indent = indent;
                    break;
                case Filepointer::Operation::UNIMPORTANT:
                    if (f.mode == OpenMode::CLOSED)
                        useClosedFileError(tok);
                    break;
                case Filepointer::Operation::UNKNOWN_OP:
                    f.mode = OpenMode::UNKNOWN_OM;
                    f.mode_indent = 0;
                    break;
                default:
                    break;
                }
                if (operation != Filepointer::Operation::NONE && operation != Filepointer::Operation::UNIMPORTANT) {
                    f.op_indent = indent;
                    f.lastOperation = operation;
                }
            }
        }
        for (std::pair<const int, Filepointer>& filepointer : filepointers) {
            filepointer.second.op_indent = 0;
            filepointer.second.mode = OpenMode::UNKNOWN_OM;
            filepointer.second.lastOperation = Filepointer::Operation::UNKNOWN_OP;
        }
    }
}

void CheckIO::fflushOnInputStreamError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::portability,
                "fflushOnInputStream", "fflush() called on input stream '" + varname + "' may result in undefined behaviour on non-linux systems.", CWE398, Certainty::normal);
}

void CheckIO::ioWithoutPositioningError(const Token *tok)
{
    reportError(tok, Severity::error,
                "IOWithoutPositioning", "Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush in between result in undefined behaviour.", CWE664, Certainty::normal);
}

void CheckIO::readWriteOnlyFileError(const Token *tok)
{
    reportError(tok, Severity::error,
                "readWriteOnlyFile", "Read operation on a file that was opened only for writing.", CWE664, Certainty::normal);
}

void CheckIO::writeReadOnlyFileError(const Token *tok)
{
    reportError(tok, Severity::error,
                "writeReadOnlyFile", "Write operation on a file that was opened only for reading.", CWE664, Certainty::normal);
}

void CheckIO::useClosedFileError(const Token *tok)
{
    reportError(tok, Severity::error,
                "useClosedFile", "Used file that is not opened.", CWE910, Certainty::normal);
}

void CheckIO::seekOnAppendedFileError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "seekOnAppendedFile", "Repositioning operation performed on a file opened in append mode has no effect.", CWE398, Certainty::normal);
}

void CheckIO::incompatibleFileOpenError(const Token *tok, const std::string &filename)
{
    reportError(tok, Severity::warning,
                "incompatibleFileOpen", "The file '" + filename + "' is opened for read and write access at the same time on different streams", CWE664, Certainty::normal);
}


//---------------------------------------------------------------------------
// scanf without field width limits can crash with huge input data
//---------------------------------------------------------------------------
void CheckIO::invalidScanf()
{
    if (!mSettings->severity.isEnabled(Severity::warning) && !mSettings->isPremiumEnabled("invalidscanf"))
        return;

    logChecker("CheckIO::invalidScanf");

    const SymbolDatabase * const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            const Token *formatToken = nullptr;
            if (Token::Match(tok, "scanf|vscanf ( %str% ,"))
                formatToken = tok->tokAt(2);
            else if (Token::Match(tok, "sscanf|vsscanf|fscanf|vfscanf (")) {
                const Token* nextArg = tok->tokAt(2)->nextArgument();
                if (nextArg && nextArg->tokType() == Token::eString)
                    formatToken = nextArg;
                else
                    continue;
            } else
                continue;

            bool format = false;

            // scan the string backwards, so we do not need to keep states
            const std::string &formatstr(formatToken->str());
            for (std::size_t i = 1; i < formatstr.length(); i++) {
                if (formatstr[i] == '%')
                    format = !format;

                else if (!format)
                    continue;

                else if (std::isdigit(formatstr[i]) || formatstr[i] == '*') {
                    format = false;
                }

                else if (std::isalpha(static_cast<unsigned char>(formatstr[i])) || formatstr[i] == '[') {
                    if (formatstr[i] == 's' || formatstr[i] == '[' || formatstr[i] == 'S' || (formatstr[i] == 'l' && formatstr[i+1] == 's'))  // #3490 - field width limits are only necessary for string input
                        invalidScanfError(tok);
                    format = false;
                }
            }
        }
    }
}

void CheckIO::invalidScanfError(const Token *tok)
{
    const std::string fname = (tok ? tok->str() : std::string("scanf"));
    reportError(tok, Severity::warning,
                "invalidscanf", fname + "() without field width limits can crash with huge input data.\n" +
                fname + "() without field width limits can crash with huge input data. Add a field width "
                "specifier to fix this problem.\n"
                "\n"
                "Sample program that can crash:\n"
                "\n"
                "#include <stdio.h>\n"
                "int main()\n"
                "{\n"
                "    char c[5];\n"
                "    scanf(\"%s\", c);\n"
                "    return 0;\n"
                "}\n"
                "\n"
                "Typing in 5 or more characters may make the program crash. The correct usage "
                "here is 'scanf(\"%4s\", c);', as the maximum field width does not include the "
                "terminating null byte.\n"
                "Source: http://linux.die.net/man/3/scanf\n"
                "Source: http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/libkern/stdio/scanf.c",
                CWE119, Certainty::normal);
}

//---------------------------------------------------------------------------
//    printf("%u", "xyz"); // Wrong argument type
//    printf("%u%s", 1); // Too few arguments
//    printf("", 1); // Too much arguments
//---------------------------------------------------------------------------

static bool findFormat(nonneg int arg, const Token *firstArg,
                       const Token *&formatStringTok, const Token *&formatArgTok)
{
    const Token* argTok = firstArg;

    for (int i = 0; i < arg && argTok; ++i)
        argTok = argTok->nextArgument();

    if (Token::Match(argTok, "%str% [,)]")) {
        formatArgTok = argTok->nextArgument();
        formatStringTok = argTok;
        return true;
    }
    if (Token::Match(argTok, "%var% [,)]") &&
        argTok->variable() &&
        Token::Match(argTok->variable()->typeStartToken(), "char|wchar_t") &&
        (argTok->variable()->isPointer() ||
         (argTok->variable()->dimensions().size() == 1 &&
          argTok->variable()->dimensionKnown(0) &&
          argTok->variable()->dimension(0) != 0))) {
        formatArgTok = argTok->nextArgument();
        if (!argTok->values().empty()) {
            const auto value = std::find_if(
                argTok->values().cbegin(), argTok->values().cend(), std::mem_fn(&ValueFlow::Value::isTokValue));
            if (value != argTok->values().cend() && value->isTokValue() && value->tokvalue &&
                value->tokvalue->tokType() == Token::eString) {
                formatStringTok = value->tokvalue;
            }
        }
        return true;
    }
    return false;
}

// Utility function returning whether iToTest equals iTypename or iOptionalPrefix+iTypename
static inline bool typesMatch(const std::string& iToTest, const std::string& iTypename, const std::string& iOptionalPrefix = "std::")
{
    return (iToTest == iTypename) || (iToTest == iOptionalPrefix + iTypename);
}

void CheckIO::checkWrongPrintfScanfArguments()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    const bool isWindows = mSettings->platform.isWindows();

    logChecker("CheckIO::checkWrongPrintfScanfArguments");

    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!tok->isName()) continue;

            const Token* argListTok = nullptr; // Points to first va_list argument
            const Token* formatStringTok = nullptr; // Points to format string token

            bool scan = false;
            bool scanf_s = false;
            int formatStringArgNo = -1;

            if (tok->strAt(1) == "(" && mSettings->library.formatstr_function(tok)) {
                formatStringArgNo = mSettings->library.formatstr_argno(tok);
                scan = mSettings->library.formatstr_scan(tok);
                scanf_s = mSettings->library.formatstr_secure(tok);
            }

            if (formatStringArgNo >= 0) {
                // formatstring found in library. Find format string and first argument belonging to format string.
                if (!findFormat(formatStringArgNo, tok->tokAt(2), formatStringTok, argListTok))
                    continue;
            } else if (Token::simpleMatch(tok, "swprintf (")) {
                if (Token::Match(tok->tokAt(2)->nextArgument(), "%str%")) {
                    // Find third parameter and format string
                    if (!findFormat(1, tok->tokAt(2), formatStringTok, argListTok))
                        continue;
                } else {
                    // Find fourth parameter and format string
                    if (!findFormat(2, tok->tokAt(2), formatStringTok, argListTok))
                        continue;
                }
            } else if (isWindows && Token::Match(tok, "sprintf_s|swprintf_s (")) {
                // template <size_t size> int sprintf_s(char (&buffer)[size], const char *format, ...);
                if (findFormat(1, tok->tokAt(2), formatStringTok, argListTok)) {
                    if (!formatStringTok)
                        continue;
                }
                // int sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ...);
                else if (findFormat(2, tok->tokAt(2), formatStringTok, argListTok)) {
                    if (!formatStringTok)
                        continue;
                }
            } else if (isWindows && Token::Match(tok, "_snprintf_s|_snwprintf_s (")) {
                // template <size_t size> int _snprintf_s(char (&buffer)[size], size_t count, const char *format, ...);
                if (findFormat(2, tok->tokAt(2), formatStringTok, argListTok)) {
                    if (!formatStringTok)
                        continue;
                }
                // int _snprintf_s(char *buffer, size_t sizeOfBuffer, size_t count, const char *format, ...);
                else if (findFormat(3, tok->tokAt(2), formatStringTok, argListTok)) {
                    if (!formatStringTok)
                        continue;
                }
            } else {
                continue;
            }

            if (!formatStringTok)
                continue;

            checkFormatString(tok, formatStringTok, argListTok, scan, scanf_s);
        }
    }
}

void CheckIO::checkFormatString(const Token * const tok,
                                const Token * const formatStringTok,
                                const Token *       argListTok,
                                const bool scan,
                                const bool scanf_s)
{
    const bool isWindows = mSettings->platform.isWindows();
    const bool printWarning = mSettings->severity.isEnabled(Severity::warning);
    const std::string &formatString = formatStringTok->str();

    // Count format string parameters..
    int numFormat = 0;
    int numSecure = 0;
    bool percent = false;
    const Token* argListTok2 = argListTok;
    std::set<int> parameterPositionsUsed;
    for (auto i = formatString.cbegin(); i != formatString.cend(); ++i) {
        if (*i == '%') {
            percent = !percent;
        } else if (percent && *i == '[') {
            while (i != formatString.cend()) {
                if (*i == ']') {
                    numFormat++;
                    if (argListTok)
                        argListTok = argListTok->nextArgument();
                    percent = false;
                    break;
                }
                ++i;
            }
            if (scanf_s) {
                numSecure++;
                if (argListTok) {
                    argListTok = argListTok->nextArgument();
                }
            }
            if (i == formatString.cend())
                break;
        } else if (percent) {
            percent = false;

            bool _continue = false;
            bool skip = false;
            std::string width;
            int parameterPosition = 0;
            bool hasParameterPosition = false;
            while (i != formatString.cend() && *i != '[' && !std::isalpha(static_cast<unsigned char>(*i))) {
                if (*i == '*') {
                    skip = true;
                    if (scan)
                        _continue = true;
                    else {
                        numFormat++;
                        if (argListTok)
                            argListTok = argListTok->nextArgument();
                    }
                } else if (std::isdigit(*i)) {
                    width += *i;
                } else if (*i == '$') {
                    parameterPosition = strToInt<int>(width);
                    hasParameterPosition = true;
                    width.clear();
                }
                ++i;
            }
            auto bracketBeg = formatString.cend();
            if (i != formatString.cend() && *i == '[') {
                bracketBeg = i;
                while (i != formatString.cend()) {
                    if (*i == ']')
                        break;

                    ++i;
                }
                if (scanf_s && !skip) {
                    if (argListTok) {
                        argListTok = argListTok->nextArgument();
                    }
                }
            }
            if (i == formatString.cend())
                break;
            if (_continue)
                continue;

            if (scan || *i != 'm') { // %m is a non-standard extension that requires no parameter on print functions.
                ++numFormat;

                // Handle parameter positions (POSIX extension) - Ticket #4900
                if (hasParameterPosition) {
                    if (parameterPositionsUsed.find(parameterPosition) == parameterPositionsUsed.end())
                        parameterPositionsUsed.insert(parameterPosition);
                    else // Parameter already referenced, hence don't consider it a new format
                        --numFormat;
                }

                // Perform type checks
                ArgumentInfo argInfo(argListTok, *mSettings, mTokenizer->isCPP());

                if ((argInfo.typeToken && !argInfo.isLibraryType(*mSettings)) || *i == ']') {
                    if (scan) {
                        std::string specifier;
                        bool done = false;
                        while (!done) {
                            switch (*i) {
                            case 's':
                            case ']': // charset
                                specifier += (*i == 's' || bracketBeg == formatString.end()) ? std::string{ 's' } : std::string{ bracketBeg, i + 1 };
                                if (argInfo.variableInfo && argInfo.isKnownType() && argInfo.variableInfo->isArray() && (argInfo.variableInfo->dimensions().size() == 1) && argInfo.variableInfo->dimensions()[0].known) {
                                    if (!width.empty()) {
                                        const int numWidth = strToInt<int>(width);
                                        if (numWidth != (argInfo.variableInfo->dimension(0) - 1))
                                            invalidScanfFormatWidthError(tok, numFormat, numWidth, argInfo.variableInfo, specifier);
                                    }
                                }
                                if (argListTok && argListTok->tokType() != Token::eString && argInfo.typeToken &&
                                    argInfo.isKnownType() && argInfo.isArrayOrPointer() &&
                                    (!Token::Match(argInfo.typeToken, "char|wchar_t") ||
                                     argInfo.typeToken->strAt(-1) == "const")) {
                                    if (!(argInfo.isArrayOrPointer() && argInfo.element && !argInfo.typeToken->isStandardType()))
                                        invalidScanfArgTypeError_s(tok, numFormat, specifier, &argInfo);
                                }
                                if (scanf_s) {
                                    numSecure++;
                                    if (argListTok) {
                                        argListTok = argListTok->nextArgument();
                                    }
                                }
                                done = true;
                                break;
                            case 'c':
                                if (argInfo.variableInfo && argInfo.isKnownType() && argInfo.variableInfo->isArray() && (argInfo.variableInfo->dimensions().size() == 1) && argInfo.variableInfo->dimensions()[0].known) {
                                    if (!width.empty()) {
                                        const int numWidth = strToInt<int>(width);
                                        if (numWidth > argInfo.variableInfo->dimension(0))
                                            invalidScanfFormatWidthError(tok, numFormat, numWidth, argInfo.variableInfo, std::string(1, *i));
                                    }
                                }
                                if (scanf_s) {
                                    numSecure++;
                                    if (argListTok) {
                                        argListTok = argListTok->nextArgument();
                                    }
                                }
                                done = true;
                                break;
                            case 'x':
                            case 'X':
                            case 'u':
                            case 'o':
                                specifier += *i;
                                if (argInfo.typeToken->tokType() == Token::eString)
                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                else if (argInfo.isKnownType()) {
                                    if (!Token::Match(argInfo.typeToken, "char|short|int|long")) {
                                        if (argInfo.typeToken->isStandardType() || !argInfo.element)
                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                    } else if (!argInfo.typeToken->isUnsigned() ||
                                               !argInfo.isArrayOrPointer() ||
                                               argInfo.typeToken->strAt(-1) == "const") {
                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                    } else {
                                        switch (specifier[0]) {
                                        case 'h':
                                            if (specifier[1] == 'h') {
                                                if (argInfo.typeToken->str() != "char")
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            } else if (argInfo.typeToken->str() != "short")
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            break;
                                        case 'l':
                                            if (specifier[1] == 'l') {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                         typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                         typesMatch(argInfo.typeToken->originalName(), "uintmax_t"))
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                     typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                     typesMatch(argInfo.typeToken->originalName(), "uintmax_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            break;
                                        case 'I':
                                            if (specifier.find("I64") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            } else if (specifier.find("I32") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            } else if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            break;
                                        case 'j':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "uintmax_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            break;
                                        case 'z':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            break;
                                        case 't':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            break;
                                        case 'L':
                                            if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                     typesMatch(argInfo.typeToken->originalName(), "uintmax_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            break;
                                        default:
                                            if (argInfo.typeToken->str() != "int")
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                     typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                     typesMatch(argInfo.typeToken->originalName(), "uintmax_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            break;
                                        }
                                    }
                                }
                                done = true;
                                break;
                            case 'n':
                            case 'd':
                            case 'i':
                                specifier += *i;
                                if (argInfo.typeToken->tokType() == Token::eString)
                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                else if (argInfo.isKnownType()) {
                                    if (!Token::Match(argInfo.typeToken, "char|short|int|long")) {
                                        if (argInfo.typeToken->isStandardType() || !argInfo.element)
                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                    } else if (argInfo.typeToken->isUnsigned() ||
                                               !argInfo.isArrayOrPointer() ||
                                               argInfo.typeToken->strAt(-1) == "const") {
                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                    } else {
                                        switch (specifier[0]) {
                                        case 'h':
                                            if (specifier[1] == 'h') {
                                                if (argInfo.typeToken->str() != "char")
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            } else if (argInfo.typeToken->str() != "short")
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            break;
                                        case 'l':
                                            if (specifier[1] == 'l') {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                         typesMatch(argInfo.typeToken->originalName(), "intmax_t"))
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                     typesMatch(argInfo.typeToken->originalName(), "intmax_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            break;
                                        case 'I':
                                            if (specifier.find("I64") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            } else if (specifier.find("I32") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            } else if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            break;
                                        case 'j':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "intmax_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            break;
                                        case 'z':
                                            if (!(typesMatch(argInfo.typeToken->originalName(), "ssize_t") ||
                                                  (isWindows && typesMatch(argInfo.typeToken->originalName(), "SSIZE_T"))))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            break;
                                        case 't':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            break;
                                        case 'L':
                                            if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            break;
                                        default:
                                            if (argInfo.typeToken->str() != "int")
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                     argInfo.typeToken->originalName() == "intmax_t")
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            break;
                                        }
                                    }
                                }
                                done = true;
                                break;
                            case 'e':
                            case 'E':
                            case 'f':
                            case 'g':
                            case 'G':
                            case 'a':
                                specifier += *i;
                                if (argInfo.typeToken->tokType() == Token::eString)
                                    invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                else if (argInfo.isKnownType()) {
                                    if (!Token::Match(argInfo.typeToken, "float|double")) {
                                        if (argInfo.typeToken->isStandardType())
                                            invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                    } else if (!argInfo.isArrayOrPointer() ||
                                               argInfo.typeToken->strAt(-1) == "const") {
                                        invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                    } else {
                                        switch (specifier[0]) {
                                        case 'l':
                                            if (argInfo.typeToken->str() != "double" || argInfo.typeToken->isLong())
                                                invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'L':
                                            if (argInfo.typeToken->str() != "double" || !argInfo.typeToken->isLong())
                                                invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                            break;
                                        default:
                                            if (argInfo.typeToken->str() != "float")
                                                invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                            break;
                                        }
                                    }
                                }
                                done = true;
                                break;
                            case 'I':
                                if ((i+1 != formatString.cend() && *(i+1) == '6' &&
                                     i+2 != formatString.cend() && *(i+2) == '4') ||
                                    (i+1 != formatString.cend() && *(i+1) == '3' &&
                                     i+2 != formatString.cend() && *(i+2) == '2')) {
                                    specifier += *i++;
                                    specifier += *i++;
                                    if ((i+1) != formatString.cend() && !isalpha(*(i+1))) {
                                        specifier += *i;
                                        invalidLengthModifierError(tok, numFormat, specifier);
                                        done = true;
                                    } else {
                                        specifier += *i++;
                                    }
                                } else {
                                    if ((i+1) != formatString.cend() && !isalpha(*(i+1))) {
                                        specifier += *i;
                                        invalidLengthModifierError(tok, numFormat, specifier);
                                        done = true;
                                    } else {
                                        specifier += *i++;
                                    }
                                }
                                break;
                            case 'h':
                            case 'l':
                                if (i+1 != formatString.cend() && *(i+1) == *i)
                                    specifier += *i++;
                                FALLTHROUGH;
                            case 'j':
                            case 'q':
                            case 't':
                            case 'z':
                            case 'L':
                                // Expect an alphabetical character after these specifiers
                                if ((i + 1) != formatString.end() && !isalpha(*(i+1))) {
                                    specifier += *i;
                                    invalidLengthModifierError(tok, numFormat, specifier);
                                    done = true;
                                } else {
                                    specifier += *i++;
                                }
                                break;
                            default:
                                done = true;
                                break;
                            }
                        }
                    } else if (printWarning) {
                        std::string specifier;
                        bool done = false;
                        while (!done) {
                            if (i == formatString.end()) {
                                break;
                            }
                            switch (*i) {
                            case 's':
                                if (argListTok->tokType() != Token::eString &&
                                    argInfo.isKnownType() && !argInfo.isArrayOrPointer()) {
                                    if (!Token::Match(argInfo.typeToken, "char|wchar_t")) {
                                        if (!argInfo.element)
                                            invalidPrintfArgTypeError_s(tok, numFormat, &argInfo);
                                    }
                                }
                                done = true;
                                break;
                            case 'n':
                                if ((argInfo.isKnownType() && (!argInfo.isArrayOrPointer() || argInfo.typeToken->strAt(-1) == "const")) || argListTok->tokType() == Token::eString)
                                    invalidPrintfArgTypeError_n(tok, numFormat, &argInfo);
                                done = true;
                                break;
                            case 'c':
                            case 'x':
                            case 'X':
                            case 'o':
                                specifier += *i;
                                if (argInfo.typeToken->tokType() == Token::eString)
                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                else if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                    // use %p on pointers and arrays
                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                } else if (argInfo.isKnownType()) {
                                    if (!Token::Match(argInfo.typeToken, "bool|short|long|int|char|wchar_t")) {
                                        if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                    } else {
                                        switch (specifier[0]) {
                                        case 'h':
                                            if (specifier[1] == 'h') {
                                                if (!(argInfo.typeToken->str() == "char" && argInfo.typeToken->isUnsigned()))
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (!(argInfo.typeToken->str() == "short" && argInfo.typeToken->isUnsigned()))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'l':
                                            if (specifier[1] == 'l') {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                         argInfo.typeToken->originalName() == "uintmax_t")
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                     argInfo.typeToken->originalName() == "uintmax_t")
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'j':
                                            if (argInfo.typeToken->originalName() != "uintmax_t")
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'z':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 't':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'I':
                                            if (specifier.find("I64") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (specifier.find("I32") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (!(typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                         argInfo.typeToken->originalName() == "WPARAM" ||
                                                         argInfo.typeToken->originalName() == "UINT_PTR" ||
                                                         argInfo.typeToken->originalName() == "LONG_PTR" ||
                                                         argInfo.typeToken->originalName() == "LPARAM" ||
                                                         argInfo.typeToken->originalName() == "LRESULT"))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        default:
                                            if (!Token::Match(argInfo.typeToken, "bool|char|short|wchar_t|int"))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        }
                                    }
                                }
                                done = true;
                                break;
                            case 'd':
                            case 'i':
                                specifier += *i;
                                if (argInfo.typeToken->tokType() == Token::eString) {
                                    invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                } else if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                    // use %p on pointers and arrays
                                    invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                } else if (argInfo.isKnownType()) {
                                    if (argInfo.typeToken->isUnsigned() && !Token::Match(argInfo.typeToken, "char|short")) {
                                        if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                            invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                    } else if (!Token::Match(argInfo.typeToken, "bool|char|short|int|long")) {
                                        if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                            invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                    } else {
                                        switch (specifier[0]) {
                                        case 'h':
                                            if (specifier[1] == 'h') {
                                                if (!(argInfo.typeToken->str() == "char" && !argInfo.typeToken->isUnsigned()))
                                                    invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            } else if (!(argInfo.typeToken->str() == "short" && !argInfo.typeToken->isUnsigned()))
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'l':
                                            if (specifier[1] == 'l') {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                         argInfo.typeToken->originalName() == "intmax_t")
                                                    invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                     argInfo.typeToken->originalName() == "intmax_t")
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'j':
                                            if (argInfo.typeToken->originalName() != "intmax_t")
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 't':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'I':
                                            if (specifier.find("I64") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            } else if (specifier.find("I32") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                    invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            } else if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'z':
                                            if (!(typesMatch(argInfo.typeToken->originalName(), "ssize_t") ||
                                                  (isWindows && typesMatch(argInfo.typeToken->originalName(), "SSIZE_T"))))
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'L':
                                            if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        default:
                                            if (!Token::Match(argInfo.typeToken, "bool|char|short|int"))
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                     argInfo.typeToken->originalName() == "intmax_t")
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        }
                                    }
                                }
                                done = true;
                                break;
                            case 'u':
                                specifier += *i;
                                if (argInfo.typeToken->tokType() == Token::eString) {
                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                } else if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                    // use %p on pointers and arrays
                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                } else if (argInfo.isKnownType()) {
                                    if (!argInfo.typeToken->isUnsigned() && !Token::Match(argInfo.typeToken, "bool|_Bool")) {
                                        if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                    } else if (!Token::Match(argInfo.typeToken, "bool|char|short|long|int")) {
                                        if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                    } else {
                                        switch (specifier[0]) {
                                        case 'h':
                                            if (specifier[1] == 'h') {
                                                if (!(argInfo.typeToken->str() == "char" && argInfo.typeToken->isUnsigned()))
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (!(argInfo.typeToken->str() == "short" && argInfo.typeToken->isUnsigned()))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'l':
                                            if (specifier[1] == 'l') {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                         argInfo.typeToken->originalName() == "uintmax_t")
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                     argInfo.typeToken->originalName() == "uintmax_t")
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'j':
                                            if (argInfo.typeToken->originalName() != "uintmax_t")
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'z':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 't':
                                            if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'I':
                                            if (specifier.find("I64") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (specifier.find("I32") != std::string::npos) {
                                                if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        case 'L':
                                            if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        default:
                                            if (!Token::Match(argInfo.typeToken, "bool|char|short|int"))
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                     argInfo.typeToken->originalName() == "intmax_t")
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            break;
                                        }
                                    }
                                }
                                done = true;
                                break;
                            case 'p':
                                if (argInfo.typeToken->tokType() == Token::eString)
                                    ; // string literals are passed as pointers to literal start, okay
                                else if (argInfo.isKnownType() && !argInfo.isArrayOrPointer())
                                    invalidPrintfArgTypeError_p(tok, numFormat, &argInfo);
                                done = true;
                                break;
                            case 'e':
                            case 'E':
                            case 'f':
                            case 'g':
                            case 'G':
                                specifier += *i;
                                if (argInfo.typeToken->tokType() == Token::eString)
                                    invalidPrintfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                else if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                    // use %p on pointers and arrays
                                    invalidPrintfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                } else if (argInfo.isKnownType()) {
                                    if (!Token::Match(argInfo.typeToken, "float|double")) {
                                        if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                            invalidPrintfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                    } else if ((specifier[0] == 'L' && (!argInfo.typeToken->isLong() || argInfo.typeToken->str() != "double")) ||
                                               (specifier[0] != 'L' && argInfo.typeToken->isLong()))
                                        invalidPrintfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                }
                                done = true;
                                break;
                            case 'h': // Can be 'hh' (signed char or unsigned char) or 'h' (short int or unsigned short int)
                            case 'l': { // Can be 'll' (long long int or unsigned long long int) or 'l' (long int or unsigned long int)
                                // If the next character is the same (which makes 'hh' or 'll') then expect another alphabetical character
                                if ((i + 1) != formatString.end() && *(i + 1) == *i) {
                                    if ((i + 2) != formatString.end() && !isalpha(*(i + 2))) {
                                        std::string modifier;
                                        modifier += *i;
                                        modifier += *(i + 1);
                                        invalidLengthModifierError(tok, numFormat, modifier);
                                        done = true;
                                    } else {
                                        specifier = *i++;
                                        specifier += *i++;
                                    }
                                } else {
                                    if ((i + 1) != formatString.end() && !isalpha(*(i + 1))) {
                                        std::string modifier;
                                        modifier += *i;
                                        invalidLengthModifierError(tok, numFormat, modifier);
                                        done = true;
                                    } else {
                                        specifier = *i++;
                                    }
                                }
                            }
                            break;
                            case 'I': // Microsoft extension: I for size_t and ptrdiff_t, I32 for __int32, and I64 for __int64
                                if ((*(i+1) == '3' && *(i+2) == '2') ||
                                    (*(i+1) == '6' && *(i+2) == '4')) {
                                    specifier += *i++;
                                    specifier += *i++;
                                }
                                FALLTHROUGH;
                            case 'j': // intmax_t or uintmax_t
                            case 'z': // size_t
                            case 't': // ptrdiff_t
                            case 'L': // long double
                                // Expect an alphabetical character after these specifiers
                                if ((i + 1) != formatString.end() && !isalpha(*(i+1))) {
                                    specifier += *i;
                                    invalidLengthModifierError(tok, numFormat, specifier);
                                    done = true;
                                } else {
                                    specifier += *i++;
                                }
                                break;
                            default:
                                done = true;
                                break;
                            }
                        }
                    }
                }

                if (argListTok)
                    argListTok = argListTok->nextArgument(); // Find next argument
            }
        }
    }

    // Count printf/scanf parameters..
    int numFunction = 0;
    while (argListTok2) {
        if (Token::Match(argListTok2, "%name% ...")) // bailout for parameter pack
            return;
        numFunction++;
        argListTok2 = argListTok2->nextArgument(); // Find next argument
    }

    if (printWarning) {
        // Check that all parameter positions reference an actual parameter
        for (const int i : parameterPositionsUsed) {
            if ((i == 0) || (i > numFormat))
                wrongPrintfScanfPosixParameterPositionError(tok, tok->str(), i, numFormat);
        }
    }

    // Mismatching number of parameters => warning
    if ((numFormat + numSecure) != numFunction)
        wrongPrintfScanfArgumentsError(tok, tok->originalName().empty() ? tok->str() : tok->originalName(), numFormat + numSecure, numFunction);
}

// We currently only support string literals, variables, and functions.
/// @todo add non-string literals, and generic expressions

CheckIO::ArgumentInfo::ArgumentInfo(const Token * arg, const Settings &settings, bool _isCPP)
    : isCPP(_isCPP)
{
    if (!arg)
        return;

    // Use AST type info
    // TODO: This is a bailout so that old code is used in simple cases. Remove the old code and always use the AST type.
    if (!Token::Match(arg, "%str% ,|)") && !(arg->variable() && arg->variable()->isArray())) {
        const Token *top = arg;
        while (top->str() == "(" && !top->isCast())
            top = top->next();
        while (top->astParent() && top->astParent()->str() != "," && top->astParent() != arg->previous())
            top = top->astParent();
        const ValueType *valuetype = top->argumentType();
        if (valuetype && valuetype->type >= ValueType::Type::BOOL) {
            typeToken = tempToken = new Token(top);
            if (valuetype->pointer && valuetype->constness & 1) {
                tempToken->str("const");
                tempToken->insertToken("a");
                tempToken = tempToken->next();
            }
            if (valuetype->type == ValueType::BOOL)
                tempToken->str("bool");
            else if (valuetype->type == ValueType::CHAR)
                tempToken->str("char");
            else if (valuetype->type == ValueType::SHORT)
                tempToken->str("short");
            else if (valuetype->type == ValueType::WCHAR_T)
                tempToken->str("wchar_t");
            else if (valuetype->type == ValueType::INT)
                tempToken->str("int");
            else if (valuetype->type == ValueType::LONG)
                tempToken->str("long");
            else if (valuetype->type == ValueType::LONGLONG) {
                tempToken->str("long");
                tempToken->isLong(true);
            } else if (valuetype->type == ValueType::FLOAT)
                tempToken->str("float");
            else if (valuetype->type == ValueType::DOUBLE)
                tempToken->str("double");
            else if (valuetype->type == ValueType::LONGDOUBLE) {
                tempToken->str("double");
                tempToken->isLong(true);
            }
            if (valuetype->isIntegral()) {
                if (valuetype->sign == ValueType::Sign::UNSIGNED)
                    tempToken->isUnsigned(true);
                else if (valuetype->sign == ValueType::Sign::SIGNED)
                    tempToken->isSigned(true);
            }
            if (!valuetype->originalTypeName.empty())
                tempToken->originalName(valuetype->originalTypeName);
            for (int p = 0; p < valuetype->pointer; p++)
                tempToken->insertToken("*");
            tempToken = const_cast<Token*>(typeToken);
            if (top->isBinaryOp() && Token::Match(top, "[+-]") && valuetype->pointer == 1 && (valuetype->type == ValueType::CHAR || valuetype->type == ValueType::WCHAR_T))
                tempToken->tokType(Token::eString);
            return;
        }
    }


    if (arg->tokType() == Token::eString) {
        typeToken = arg;
        return;
    }
    if (arg->str() == "&" || arg->tokType() == Token::eVariable ||
        arg->tokType() == Token::eFunction || Token::Match(arg, "%type% ::") ||
        (Token::Match(arg, "static_cast|reinterpret_cast|const_cast <") &&
         Token::simpleMatch(arg->linkAt(1), "> (") &&
         Token::Match(arg->linkAt(1)->linkAt(1), ") ,|)"))) {
        if (Token::Match(arg, "static_cast|reinterpret_cast|const_cast")) {
            typeToken = arg->tokAt(2);
            while (typeToken->str() == "const" || typeToken->str() == "extern")
                typeToken = typeToken->next();
            return;
        }
        if (arg->str() == "&") {
            address = true;
            arg = arg->next();
        }
        while (Token::Match(arg, "%type% ::"))
            arg = arg->tokAt(2);
        if (!arg || !(arg->tokType() == Token::eVariable || arg->tokType() == Token::eFunction))
            return;
        const Token *varTok = nullptr;
        const Token *tok1 = arg->next();
        for (; tok1; tok1 = tok1->next()) {
            if (tok1->str() == "," || tok1->str() == ")") {
                if (tok1->strAt(-1) == "]") {
                    varTok = tok1->linkAt(-1)->previous();
                    if (varTok->str() == ")" && varTok->link()->previous()->tokType() == Token::eFunction) {
                        const Function * function = varTok->link()->previous()->function();
                        if (function && function->retType && function->retType->isEnumType()) {
                            if (function->retType->classScope->enumType)
                                typeToken = function->retType->classScope->enumType;
                            else {
                                tempToken = new Token(tok1);
                                tempToken->str("int");
                                typeToken = tempToken;
                            }
                        } else if (function && function->retDef) {
                            typeToken = function->retDef;
                            while (typeToken->str() == "const" || typeToken->str() == "extern")
                                typeToken = typeToken->next();
                            functionInfo = function;
                            element = true;
                        }
                        return;
                    }
                } else if (tok1->strAt(-1) == ")" && tok1->linkAt(-1)->previous()->tokType() == Token::eFunction) {
                    const Function * function = tok1->linkAt(-1)->previous()->function();
                    if (function && function->retType && function->retType->isEnumType()) {
                        if (function->retType->classScope->enumType)
                            typeToken = function->retType->classScope->enumType;
                        else {
                            tempToken = new Token(tok1);
                            tempToken->str("int");
                            typeToken = tempToken;
                        }
                    } else if (function && function->retDef) {
                        typeToken = function->retDef;
                        while (typeToken->str() == "const" || typeToken->str() == "extern")
                            typeToken = typeToken->next();
                        functionInfo = function;
                        element = false;
                    }
                    return;
                } else
                    varTok = tok1->previous();
                break;
            }
            if (tok1->str() == "(" || tok1->str() == "{" || tok1->str() == "[")
                tok1 = tok1->link();
            else if (tok1->link() && tok1->str() == "<")
                tok1 = tok1->link();

            // check for some common well known functions
            else if (isCPP && ((Token::Match(tok1->previous(), "%var% . size|empty|c_str ( ) [,)]") && isStdContainer(tok1->previous())) ||
                               (Token::Match(tok1->previous(), "] . size|empty|c_str ( ) [,)]") && isStdContainer(tok1->linkAt(-1)->previous())))) {
                tempToken = new Token(tok1);
                if (tok1->strAt(1) == "size") {
                    // size_t is platform dependent
                    if (settings.platform.sizeof_size_t == 8) {
                        tempToken->str("long");
                        if (settings.platform.sizeof_long != 8)
                            tempToken->isLong(true);
                    } else if (settings.platform.sizeof_size_t == 4) {
                        if (settings.platform.sizeof_long == 4) {
                            tempToken->str("long");
                        } else {
                            tempToken->str("int");
                        }
                    }

                    tempToken->originalName("size_t");
                    tempToken->isUnsigned(true);
                } else if (tok1->strAt(1) == "empty") {
                    tempToken->str("bool");
                } else if (tok1->strAt(1) == "c_str") {
                    tempToken->str("const");
                    tempToken->insertToken("*");
                    if (typeToken->strAt(2) == "string")
                        tempToken->insertToken("char");
                    else
                        tempToken->insertToken("wchar_t");
                }
                typeToken = tempToken;
                return;
            }

            // check for std::vector::at() and std::string::at()
            else if (Token::Match(tok1->previous(), "%var% . at (") &&
                     Token::Match(tok1->linkAt(2), ") [,)]")) {
                varTok = tok1->previous();
                variableInfo = varTok->variable();

                if (!variableInfo || !isStdVectorOrString()) {
                    variableInfo = nullptr;
                    typeToken = nullptr;
                }

                return;
            } else if (!(tok1->str() == "." || tok1->tokType() == Token::eVariable || tok1->tokType() == Token::eFunction))
                return;
        }

        if (varTok) {
            variableInfo = varTok->variable();
            element = tok1->strAt(-1) == "]";

            // look for std::vector operator [] and use template type as return type
            if (variableInfo) {
                if (element && isStdVectorOrString()) { // isStdVectorOrString sets type token if true
                    element = false;    // not really an array element
                } else if (variableInfo->isEnumType()) {
                    if (variableInfo->type() && variableInfo->type()->classScope && variableInfo->type()->classScope->enumType)
                        typeToken = variableInfo->type()->classScope->enumType;
                    else {
                        tempToken = new Token(tok1);
                        tempToken->str("int");
                        typeToken = tempToken;
                    }
                } else
                    typeToken = variableInfo->typeStartToken();
            }

            return;
        }
    }
}

CheckIO::ArgumentInfo::~ArgumentInfo()
{
    if (tempToken) {
        while (tempToken->next())
            tempToken->deleteNext();

        delete tempToken;
    }
}

namespace {
    const std::set<std::string> stl_vector = { "array", "vector" };
    const std::set<std::string> stl_string = { "string", "u16string", "u32string", "wstring" };
}

bool CheckIO::ArgumentInfo::isStdVectorOrString()
{
    if (!isCPP)
        return false;
    if (variableInfo->isStlType(stl_vector)) {
        typeToken = variableInfo->typeStartToken()->tokAt(4);
        _template = true;
        return true;
    }
    if (variableInfo->isStlType(stl_string)) {
        tempToken = new Token(variableInfo->typeStartToken());
        if (variableInfo->typeStartToken()->strAt(2) == "string")
            tempToken->str("char");
        else
            tempToken->str("wchar_t");
        typeToken = tempToken;
        return true;
    }
    if (variableInfo->type() && !variableInfo->type()->derivedFrom.empty()) {
        const std::vector<Type::BaseInfo>& derivedFrom = variableInfo->type()->derivedFrom;
        for (const Type::BaseInfo & i : derivedFrom) {
            const Token* nameTok = i.nameTok;
            if (Token::Match(nameTok, "std :: vector|array <")) {
                typeToken = nameTok->tokAt(4);
                _template = true;
                return true;
            }
            if (Token::Match(nameTok, "std :: string|wstring")) {
                tempToken = new Token(variableInfo->typeStartToken());
                if (nameTok->strAt(2) == "string")
                    tempToken->str("char");
                else
                    tempToken->str("wchar_t");
                typeToken = tempToken;
                return true;
            }
        }
    } else if (variableInfo->type()) {
        const Scope * classScope = variableInfo->type()->classScope;
        if (classScope) {
            for (const Function &func : classScope->functionList) {
                if (func.name() == "operator[]") {
                    if (Token::Match(func.retDef, "%type% &")) {
                        typeToken = func.retDef;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

static const std::set<std::string> stl_container = {
    "array", "bitset", "deque", "forward_list",
    "hash_map", "hash_multimap", "hash_set",
    "list", "map", "multimap", "multiset",
    "priority_queue", "queue", "set", "stack",
    "unordered_map", "unordered_multimap", "unordered_multiset", "unordered_set", "vector"
};

bool CheckIO::ArgumentInfo::isStdContainer(const Token *tok)
{
    if (!isCPP)
        return false;
    if (tok && tok->variable()) {
        const Variable* variable = tok->variable();
        if (variable->isStlType(stl_container)) {
            typeToken = variable->typeStartToken()->tokAt(4);
            return true;
        }
        if (variable->isStlType(stl_string)) {
            typeToken = variable->typeStartToken();
            return true;
        }
        if (variable->type() && !variable->type()->derivedFrom.empty()) {
            for (const Type::BaseInfo &baseInfo : variable->type()->derivedFrom) {
                const Token* nameTok = baseInfo.nameTok;
                if (Token::Match(nameTok, "std :: vector|array|bitset|deque|list|forward_list|map|multimap|multiset|priority_queue|queue|set|stack|hash_map|hash_multimap|hash_set|unordered_map|unordered_multimap|unordered_set|unordered_multiset <")) {
                    typeToken = nameTok->tokAt(4);
                    return true;
                }
                if (Token::Match(nameTok, "std :: string|wstring")) {
                    typeToken = nameTok;
                    return true;
                }
            }
        }
    }

    return false;
}

bool CheckIO::ArgumentInfo::isArrayOrPointer() const
{
    if (address)
        return true;
    if (variableInfo && !_template)
        return variableInfo->isArrayOrPointer();

    const Token *tok = typeToken;
    while (Token::Match(tok, "const|struct"))
        tok = tok->next();
    return tok && tok->strAt(1) == "*";
}

bool CheckIO::ArgumentInfo::isComplexType() const
{
    if (variableInfo->type())
        return true;

    const Token* varTypeTok = typeToken;
    if (varTypeTok->str() == "std")
        varTypeTok = varTypeTok->tokAt(2);

    return ((variableInfo->isStlStringType() || (varTypeTok->strAt(1) == "<" && varTypeTok->linkAt(1) && varTypeTok->linkAt(1)->strAt(1) != "::")) && !variableInfo->isArrayOrPointer());
}

bool CheckIO::ArgumentInfo::isKnownType() const
{
    if (variableInfo)
        return (typeToken->isStandardType() || typeToken->next()->isStandardType() || isComplexType());
    if (functionInfo)
        return (typeToken->isStandardType() || functionInfo->retType || Token::Match(typeToken, "std :: string|wstring"));

    return typeToken->isStandardType() || Token::Match(typeToken, "std :: string|wstring");
}

bool CheckIO::ArgumentInfo::isLibraryType(const Settings &settings) const
{
    return typeToken && typeToken->isStandardType() && settings.library.podtype(typeToken->str());
}

void CheckIO::wrongPrintfScanfArgumentsError(const Token* tok,
                                             const std::string &functionName,
                                             nonneg int numFormat,
                                             nonneg int numFunction)
{
    const Severity severity = numFormat > numFunction ? Severity::error : Severity::warning;
    if (severity != Severity::error && !mSettings->severity.isEnabled(Severity::warning) && !mSettings->isPremiumEnabled("wrongPrintfScanfArgNum"))
        return;

    std::ostringstream errmsg;
    errmsg << functionName
           << " format string requires "
           << numFormat
           << " parameter" << (numFormat != 1 ? "s" : "") << " but "
           << (numFormat > numFunction ? "only " : "")
           << numFunction
           << (numFunction != 1 ? " are" : " is")
           << " given.";

    reportError(tok, severity, "wrongPrintfScanfArgNum", errmsg.str(), CWE685, Certainty::normal);
}

void CheckIO::wrongPrintfScanfPosixParameterPositionError(const Token* tok, const std::string& functionName,
                                                          nonneg int index, nonneg int numFunction)
{
    if (!mSettings->severity.isEnabled(Severity::warning) && !mSettings->isPremiumEnabled("wrongPrintfScanfParameterPositionError"))
        return;
    std::ostringstream errmsg;
    errmsg << functionName << ": ";
    if (index == 0) {
        errmsg << "parameter positions start at 1, not 0";
    } else {
        errmsg << "referencing parameter " << index << " while " << numFunction << " arguments given";
    }
    reportError(tok, Severity::warning, "wrongPrintfScanfParameterPositionError", errmsg.str(), CWE685, Certainty::normal);
}

void CheckIO::invalidScanfArgTypeError_s(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    const Severity severity = getSeverity(argInfo);
    if (!mSettings->severity.isEnabled(severity))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires a \'";
    if (specifier[0] == 's')
        errmsg << "char";
    else if (specifier[0] == 'S')
        errmsg << "wchar_t";
    errmsg << " *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, severity, "invalidScanfArgType_s", errmsg.str(), CWE686, Certainty::normal);
}
void CheckIO::invalidScanfArgTypeError_int(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo, bool isUnsigned)
{
    const Severity severity = getSeverity(argInfo);
    if (!mSettings->severity.isEnabled(severity))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires \'";
    if (specifier[0] == 'h') {
        if (specifier[1] == 'h')
            errmsg << (isUnsigned ? "unsigned " : "") << "char";
        else
            errmsg << (isUnsigned ? "unsigned " : "") << "short";
    } else if (specifier[0] == 'l') {
        if (specifier[1] == 'l')
            errmsg << (isUnsigned ? "unsigned " : "") << "long long";
        else
            errmsg << (isUnsigned ? "unsigned " : "") << "long";
    } else if (specifier.find("I32") != std::string::npos) {
        errmsg << (isUnsigned ? "unsigned " : "") << "__int32";
    } else if (specifier.find("I64") != std::string::npos) {
        errmsg << (isUnsigned ? "unsigned " : "") << "__int64";
    } else if (specifier[0] == 'I') {
        errmsg << (isUnsigned ? "size_t" : "ptrdiff_t");
    } else if (specifier[0] == 'j') {
        if (isUnsigned)
            errmsg << "uintmax_t";
        else
            errmsg << "intmax_t";
    } else if (specifier[0] == 'z') {
        if (specifier[1] == 'd' || specifier[1] == 'i')
            errmsg << "ssize_t";
        else
            errmsg << "size_t";
    } else if (specifier[0] == 't') {
        errmsg << (isUnsigned ? "unsigned " : "") << "ptrdiff_t";
    } else if (specifier[0] == 'L') {
        errmsg << (isUnsigned ? "unsigned " : "") << "long long";
    } else {
        errmsg << (isUnsigned ? "unsigned " : "") << "int";
    }
    errmsg << " *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, severity, "invalidScanfArgType_int", errmsg.str(), CWE686, Certainty::normal);
}
void CheckIO::invalidScanfArgTypeError_float(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    const Severity severity = getSeverity(argInfo);
    if (!mSettings->severity.isEnabled(severity))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires \'";
    if (specifier[0] == 'l' && specifier[1] != 'l')
        errmsg << "double";
    else if (specifier[0] == 'L')
        errmsg << "long double";
    else
        errmsg << "float";
    errmsg << " *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, severity, "invalidScanfArgType_float", errmsg.str(), CWE686, Certainty::normal);
}

void CheckIO::invalidPrintfArgTypeError_s(const Token* tok, nonneg int numFormat, const ArgumentInfo* argInfo)
{
    const Severity severity = getSeverity(argInfo);
    if (!mSettings->severity.isEnabled(severity))
        return;
    std::ostringstream errmsg;
    errmsg << "%s in format string (no. " << numFormat << ") requires \'char *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, severity, "invalidPrintfArgType_s", errmsg.str(), CWE686, Certainty::normal);
}
void CheckIO::invalidPrintfArgTypeError_n(const Token* tok, nonneg int numFormat, const ArgumentInfo* argInfo)
{
    const Severity severity = getSeverity(argInfo);
    if (!mSettings->severity.isEnabled(severity))
        return;
    std::ostringstream errmsg;
    errmsg << "%n in format string (no. " << numFormat << ") requires \'int *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, severity, "invalidPrintfArgType_n", errmsg.str(), CWE686, Certainty::normal);
}
void CheckIO::invalidPrintfArgTypeError_p(const Token* tok, nonneg int numFormat, const ArgumentInfo* argInfo)
{
    const Severity severity = getSeverity(argInfo);
    if (!mSettings->severity.isEnabled(severity))
        return;
    std::ostringstream errmsg;
    errmsg << "%p in format string (no. " << numFormat << ") requires an address but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, severity, "invalidPrintfArgType_p", errmsg.str(), CWE686, Certainty::normal);
}
static void printfFormatType(std::ostream& os, const std::string& specifier, bool isUnsigned)
{
    os << "\'";
    if (specifier[0] == 'l') {
        if (specifier[1] == 'l')
            os << (isUnsigned ? "unsigned " : "") << "long long";
        else
            os << (isUnsigned ? "unsigned " : "") << "long";
    } else if (specifier[0] == 'h') {
        if (specifier[1] == 'h')
            os << (isUnsigned ? "unsigned " : "") << "char";
        else
            os << (isUnsigned ? "unsigned " : "") << "short";
    } else if (specifier.find("I32") != std::string::npos) {
        os << (isUnsigned ? "unsigned " : "") << "__int32";
    } else if (specifier.find("I64") != std::string::npos) {
        os << (isUnsigned ? "unsigned " : "") << "__int64";
    } else if (specifier[0] == 'I') {
        os << (isUnsigned ? "size_t" : "ptrdiff_t");
    } else if (specifier[0] == 'j') {
        if (isUnsigned)
            os << "uintmax_t";
        else
            os << "intmax_t";
    } else if (specifier[0] == 'z') {
        if (specifier[1] == 'd' || specifier[1] == 'i')
            os << "ssize_t";
        else
            os << "size_t";
    } else if (specifier[0] == 't') {
        os << (isUnsigned ? "unsigned " : "") << "ptrdiff_t";
    } else if (specifier[0] == 'L') {
        os << (isUnsigned ? "unsigned " : "") << "long long";
    } else {
        os << (isUnsigned ? "unsigned " : "") << "int";
    }
    os << "\'";
}

void CheckIO::invalidPrintfArgTypeError_uint(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    const Severity severity = getSeverity(argInfo);
    if (!mSettings->severity.isEnabled(severity))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires ";
    printfFormatType(errmsg, specifier, true);
    errmsg << " but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, severity, "invalidPrintfArgType_uint", errmsg.str(), CWE686, Certainty::normal);
}

void CheckIO::invalidPrintfArgTypeError_sint(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    const Severity severity = getSeverity(argInfo);
    if (!mSettings->severity.isEnabled(severity))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires ";
    printfFormatType(errmsg, specifier, false);
    errmsg << " but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, severity, "invalidPrintfArgType_sint", errmsg.str(), CWE686, Certainty::normal);
}
void CheckIO::invalidPrintfArgTypeError_float(const Token* tok, nonneg int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    const Severity severity = getSeverity(argInfo);
    if (!mSettings->severity.isEnabled(severity))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires \'";
    if (specifier[0] == 'L')
        errmsg << "long ";
    errmsg << "double\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, severity, "invalidPrintfArgType_float", errmsg.str(), CWE686, Certainty::normal);
}

Severity CheckIO::getSeverity(const CheckIO::ArgumentInfo *argInfo)
{
    return (argInfo && argInfo->typeToken && !argInfo->typeToken->originalName().empty()) ? Severity::portability : Severity::warning;
}

void CheckIO::argumentType(std::ostream& os, const ArgumentInfo * argInfo)
{
    if (argInfo) {
        os << "\'";
        const Token *type = argInfo->typeToken;
        if (type->tokType() == Token::eString) {
            if (type->isLong())
                os << "const wchar_t *";
            else
                os << "const char *";
        } else {
            if (type->originalName().empty()) {
                if (type->strAt(-1) == "const")
                    os << "const ";
                while (Token::Match(type, "const|struct")) {
                    os << type->str() << " ";
                    type = type->next();
                }
                while (Token::Match(type, "%any% ::")) {
                    os << type->str() << "::";
                    type = type->tokAt(2);
                }
                os << type->stringify(false, true, false);
                if (type->strAt(1) == "*" && !argInfo->element)
                    os << " *";
                else if (argInfo->variableInfo && !argInfo->element && argInfo->variableInfo->isArray())
                    os << " *";
                else if (type->strAt(1) == "*" && argInfo->variableInfo && argInfo->element && argInfo->variableInfo->isArray())
                    os << " *";
                if (argInfo->address)
                    os << " *";
            } else {
                if (type->isUnsigned()) {
                    if (type->originalName() == "__int64" || type->originalName() == "__int32" || type->originalName() == "ptrdiff_t")
                        os << "unsigned ";
                }
                os << type->originalName();
                if (type->strAt(1) == "*" || argInfo->address)
                    os << " *";
                os << " {aka " << type->stringify(false, true, false);
                if (type->strAt(1) == "*" || argInfo->address)
                    os << " *";
                os << "}";
            }
        }
        os << "\'";
    } else
        os << "Unknown";
}

void CheckIO::invalidLengthModifierError(const Token* tok, nonneg int numFormat, const std::string& modifier)
{
    if (!mSettings->severity.isEnabled(Severity::warning) && !mSettings->isPremiumEnabled("invalidLengthModifierError"))
        return;
    std::ostringstream errmsg;
    errmsg << "'" << modifier << "' in format string (no. " << numFormat << ") is a length modifier and cannot be used without a conversion specifier.";
    reportError(tok, Severity::warning, "invalidLengthModifierError", errmsg.str(), CWE704, Certainty::normal);
}

void CheckIO::invalidScanfFormatWidthError(const Token* tok, nonneg int numFormat, int width, const Variable *var, const std::string& specifier)
{
    MathLib::bigint arrlen = 0;
    std::string varname;

    if (var) {
        arrlen = var->dimension(0);
        varname = var->name();
    }

    std::ostringstream errmsg;
    if (arrlen > width) {
        if (tok != nullptr && (!mSettings->certainty.isEnabled(Certainty::inconclusive) || !mSettings->severity.isEnabled(Severity::warning)))
            return;
        errmsg << "Width " << width << " given in format string (no. " << numFormat << ") is smaller than destination buffer"
               << " '" << varname << "[" << arrlen << "]'.";
        reportError(tok, Severity::warning, "invalidScanfFormatWidth_smaller", errmsg.str(), CWE(0U), Certainty::inconclusive);
    } else {
        errmsg << "Width " << width << " given in format string (no. " << numFormat << ") is larger than destination buffer '"
               << varname << "[" << arrlen << "]', use %" << (specifier == "c" ? arrlen : (arrlen - 1)) << specifier << " to prevent overflowing it.";
        reportError(tok, Severity::error, "invalidScanfFormatWidth", errmsg.str(), CWE687, Certainty::normal);
    }
}

void CheckIO::runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger)
{
    CheckIO checkIO(&tokenizer, &tokenizer.getSettings(), errorLogger);

    checkIO.checkWrongPrintfScanfArguments();
    checkIO.checkCoutCerrMisusage();
    checkIO.checkFileUsage();
    checkIO.invalidScanf();
}

void CheckIO::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const
{
    CheckIO c(nullptr, settings, errorLogger);
    c.coutCerrMisusageError(nullptr,  "cout");
    c.fflushOnInputStreamError(nullptr,  "stdin");
    c.ioWithoutPositioningError(nullptr);
    c.readWriteOnlyFileError(nullptr);
    c.writeReadOnlyFileError(nullptr);
    c.useClosedFileError(nullptr);
    c.seekOnAppendedFileError(nullptr);
    c.incompatibleFileOpenError(nullptr, "tmp");
    c.invalidScanfError(nullptr);
    c.wrongPrintfScanfArgumentsError(nullptr, "printf",3,2);
    c.invalidScanfArgTypeError_s(nullptr,  1, "s", nullptr);
    c.invalidScanfArgTypeError_int(nullptr,  1, "d", nullptr, false);
    c.invalidScanfArgTypeError_float(nullptr,  1, "f", nullptr);
    c.invalidPrintfArgTypeError_s(nullptr,  1, nullptr);
    c.invalidPrintfArgTypeError_n(nullptr,  1, nullptr);
    c.invalidPrintfArgTypeError_p(nullptr,  1, nullptr);
    c.invalidPrintfArgTypeError_uint(nullptr,  1, "u", nullptr);
    c.invalidPrintfArgTypeError_sint(nullptr,  1, "i", nullptr);
    c.invalidPrintfArgTypeError_float(nullptr,  1, "f", nullptr);
    c.invalidLengthModifierError(nullptr,  1, "I");
    c.invalidScanfFormatWidthError(nullptr,  10, 5, nullptr, "s");
    c.invalidScanfFormatWidthError(nullptr,  99, -1, nullptr, "s");
    c.wrongPrintfScanfPosixParameterPositionError(nullptr,  "printf", 2, 1);
}
