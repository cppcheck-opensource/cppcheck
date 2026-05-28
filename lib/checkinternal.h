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
#ifndef checkinternalH
#define checkinternalH
//---------------------------------------------------------------------------

#ifdef CHECK_INTERNAL

#include "check.h"
#include "checkimpl.h"
#include "config.h"

#include <string>

class ErrorLogger;
class Token;
class Tokenizer;
class Settings;

/// @addtogroup Checks
/// @{


/** @brief %Check Internal cppcheck API usage */
class CPPCHECKLIB CheckInternal : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckInternal() : Check("cppcheck internal API usage") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings &settings) const override;

    std::string classInfo() const override {
        // Don't include these checks on the WIKI where people can read what
        // checks there are. These checks are not intended for users.
        return "";
    }
};

class CPPCHECKLIB CheckInternalImpl : public CheckImpl {
public:
    /** This constructor is used when running checks. */
    CheckInternalImpl(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : CheckImpl(tokenizer, settings, errorLogger) {}

    /** @brief %Check if a simple pattern is used inside Token::Match or Token::findmatch */
    void checkTokenMatchPatterns();

    /** @brief %Check if a complex pattern is used inside Token::simpleMatch or Token::findsimplematch */
    void checkTokenSimpleMatchPatterns();

    /** @brief %Check for missing % end character in Token::Match pattern */
    void checkMissingPercentCharacter();

    /** @brief %Check for unknown (invalid) complex patterns like "%typ%" */
    void checkUnknownPattern();

    /** @brief %Check for inefficient usage of Token::next(), Token::previous() and Token::tokAt() */
    void checkRedundantNextPrevious();

    /** @brief %Check if there is whitespace at the beginning or at the end of a pattern */
    void checkExtraWhitespace();

    /** @brief %Check if there is a redundant check for none-nullness of parameter before Match functions, such as (tok && Token::Match(tok, "foo")) */
    void checkRedundantTokCheck();

    void simplePatternError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void complexPatternError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void missingPercentCharacterError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void unknownPatternError(const Token* tok, const std::string& pattern);
    void redundantNextPreviousError(const Token* tok, const std::string& func1, const std::string& func2);
    void orInComplexPattern(const Token *tok, const std::string &pattern, const std::string &funcname);
    void extraWhitespaceError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void checkRedundantTokCheckError(const Token *tok);
};
/// @}
//---------------------------------------------------------------------------

#endif // CHECK_INTERNAL

#endif // checkinternalH
