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
#ifndef checkbufferoverrunH
#define checkbufferoverrunH
//---------------------------------------------------------------------------

#include "check.h"
#include "checkimpl.h"
#include "config.h"
#include "ctu.h"

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>

class ErrorLogger;
class Settings;
class Token;
class Tokenizer;
class Variable;
struct Dimension;
enum class Certainty : std::uint8_t;

namespace ValueFlow
{
    class Value;
}

/// @addtogroup Checks
/// @{

/**
 * @brief buffer overruns and array index out of bounds
 *
 * Buffer overrun and array index out of bounds are pretty much the same.
 * But I generally use 'array index' if the code contains []. And the given
 * index is out of bounds.
 * I generally use 'buffer overrun' if you for example call a strcpy or
 * other function and pass a buffer and reads or writes too much data.
 */
class CPPCHECKLIB CheckBufferOverrun : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckBufferOverrun() : Check("Bounds checking") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger& errorLogger) override;

    /** @brief Parse current TU and extract file info */
    const Check::FileInfo *getFileInfo(const Tokenizer &tokenizer, const Settings &settings, const std::string& /*currentConfig*/) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::FileInfo &ctu, const std::list<const Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) override;

    void getErrorMessages(ErrorLogger& errorLogger, const Settings &settings) const override;

    const Check::FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement, const std::string& file0) const override;

    std::string classInfo() const override {
        return "Out of bounds checking:\n"
               "- Array index out of bounds\n"
               "- Pointer arithmetic overflow\n"
               "- Buffer overflow\n"
               "- Dangerous usage of strncat()\n"
               "- Using array index before checking it\n"
               "- Partial string write that leads to buffer that is not zero terminated.\n"
               "- Check for large enough arrays being passed to functions\n"
               "- Allocating memory with a negative size\n";
    }

    static bool analyseWholeProgram1(const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> &callsMap, const CTU::FileInfo::UnsafeUsage &unsafeUsage,
                                     int type, ErrorLogger &errorLogger, int maxCtuDepth, const std::string& file0);
};

class CPPCHECKLIB CheckBufferOverrunImpl : public CheckImpl
{
public:
    /** This constructor is used when running checks. */
    CheckBufferOverrunImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger &errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    void arrayIndex();
    void arrayIndexError(const Token* tok,
                         const std::vector<Dimension>& dimensions,
                         const std::vector<ValueFlow::Value>& indexes);
    void negativeIndexError(const Token* tok,
                            const std::vector<Dimension>& dimensions,
                            const std::vector<ValueFlow::Value>& indexes);

    void pointerArithmetic();
    void pointerArithmeticError(const Token *tok, const Token *indexToken, const ValueFlow::Value *indexValue);

    void bufferOverflow();
    void bufferOverflowError(const Token *tok, const ValueFlow::Value *value, Certainty certainty);

    void arrayIndexThenCheck();
    void arrayIndexThenCheckError(const Token *tok, const std::string &indexName);

    void stringNotZeroTerminated();
    void terminateStrncpyError(const Token *tok, const std::string &varname);

    void argumentSize();
    void argumentSizeError(const Token *tok, const std::string &functionName, nonneg int paramIndex, const std::string &paramExpression, const Variable *paramVar, const Variable *functionArg);

    void negativeArraySize();
    void negativeArraySizeError(const Token* tok);
    void negativeMemoryAllocationSizeError(const Token* tok, const ValueFlow::Value* value); // provide a negative value to memory allocation function

    void objectIndex();
    void objectIndexError(const Token *tok, const ValueFlow::Value *v, bool known);

    ValueFlow::Value getBufferSize(const Token *bufTok, const Settings& settings) const;

    // CTU
    static bool isCtuUnsafeBufferUsage(const Settings &settings, const Token *argtok, CTU::FileInfo::Value *offset, int type);
    static bool isCtuUnsafeArrayIndex(const Settings &settings, const Token *argtok, CTU::FileInfo::Value *offset);
    static bool isCtuUnsafePointerArith(const Settings &settings, const Token *argtok, CTU::FileInfo::Value *offset);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkbufferoverrunH
