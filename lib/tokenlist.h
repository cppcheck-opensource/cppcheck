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
#ifndef tokenlistH
#define tokenlistH
//---------------------------------------------------------------------------

#include "config.h"
#include "standards.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Token;
class Settings;

namespace simplecpp {
    class TokenList;
}

/// @addtogroup Core
/// @{

/**
 * @brief This struct stores pointers to the front and back tokens of the list this token is in.
 */
struct TokensFrontBack {
    Token *front{};
    Token* back{};
};

class CPPCHECKLIB TokenList {
public:
    explicit TokenList(const Settings& settings, Standards::Language lang);
    ~TokenList();

    TokenList(const TokenList &) = delete;
    TokenList &operator=(const TokenList &) = delete;

    TokenList(TokenList&& other) noexcept = default;

    /** @return the source file path. e.g. "file.cpp" */
    const std::string& getSourceFilePath() const;

    /** @return true if the code is C */
    bool isC() const;

    /** @return true if the code is C++ */
    bool isCPP() const;

    /**
     * Delete all tokens in given token list
     * @param tok token list to delete
     */
    static void deleteTokens(Token *tok);

    void addtoken(const std::string& str, nonneg int lineno, nonneg int column, nonneg int fileno, bool split = false);
    void addtoken(const std::string& str, const Token *locationTok);

    void addtoken(const Token *tok, nonneg int lineno, nonneg int column, nonneg int fileno);
    void addtoken(const Token *tok, const Token *locationTok);
    void addtoken(const Token *tok);

    static void insertTokens(Token *dest, const Token *src, nonneg int n);

    /**
     * Copy tokens.
     * @param dest destination token where copied tokens will be inserted after
     * @param first first token to copy
     * @param last last token to copy
     * @param one_line true=>copy all tokens to the same line as dest. false=>copy all tokens to dest while keeping the 'line breaks'
     * @return new location of last token copied
     */
    RET_NONNULL static Token *copyTokens(Token *dest, const Token *first, const Token *last, bool one_line = true);

    /**
     * Create tokens from code.
     * The code must be preprocessed first:
     * - multiline strings are not handled.
     * - UTF in the code are not handled.
     * - comments are not handled.
     */
    bool createTokensFromBuffer(const uint8_t* data, size_t size) {
        return createTokensFromBuffer(reinterpret_cast<const char*>(data), size);
    }
    bool createTokensFromBuffer(const char* data, size_t size);
    template<size_t size>
    // cppcheck-suppress unusedFunction - used in tests only
    bool createTokensFromString(const char (&data)[size]) {
        return createTokensFromBuffer(data, size-1);
    }

    void createTokens(simplecpp::TokenList&& tokenList);

    /** Deallocate list */
    void deallocateTokens();

    /** append file name if seen the first time; return its index in any case */
    int appendFileIfNew(std::string fileName);

    /** get first token of list */
    const Token *front() const {
        return mTokensFrontBack->front;
    }
    // NOLINTNEXTLINE(readability-make-member-function-const) - do not allow usage of mutable pointer from const object
    Token *front() {
        return mTokensFrontBack->front;
    }

    /** get last token of list */
    const Token *back() const {
        return mTokensFrontBack->back;
    }
    // NOLINTNEXTLINE(readability-make-member-function-const) - do not allow usage of mutable pointer from const object
    Token *back() {
        return mTokensFrontBack->back;
    }

    /**
     * Get filenames (the sourcefile + the files it include).
     * The first filename is the filename for the sourcefile
     * @return vector with filenames
     */
    const std::vector<std::string>& getFiles() const {
        return mFiles;
    }

    std::string getOrigFile(const Token *tok) const;

    /**
     * get filename for given token
     * @param tok The given token
     * @return filename for the given token
     */
    const std::string& file(const Token *tok) const;

    /**
     * Get file:line for a given token
     * @param tok given token
     * @return location for given token
     */
    std::string fileLine(const Token *tok) const;

    /**
     * Calculates a hash of the token list used to compare multiple
     * token lists with each other as quickly as possible.
     */
    std::size_t calculateHash() const;

    /**
     * Create abstract syntax tree.
     * @throws InternalError thrown if encountering an infinite loop in AST creation
     */
    void createAst() const;

    /**
     * Check abstract syntax tree.
     * @throws InternalError thrown if validation fails
     */
    void validateAst(bool print) const;

    /**
     * Verify that the given token is an element of the tokenlist.
     * That method is implemented for debugging purposes.
     * @param[in] tok token to be checked
     * \return true if token was found in tokenlist, false else. In case of nullptr true is returned.
     */
    bool validateToken(const Token* tok) const;

    /**
     * Convert platform dependent types to standard types.
     * 32 bits: size_t -> unsigned long
     * 64 bits: size_t -> unsigned long long
     */
    void simplifyPlatformTypes();

    /**
     * Collapse compound standard types into a single token.
     * unsigned long long int => long _isUnsigned=true,_isLong=true
     */
    void simplifyStdType();

    void clangSetOrigFiles();

    bool isKeyword(const std::string &str) const;

    /**
     * is token pointing at function head?
     * @param tok         A '(' or ')' token in a possible function head
     * @param endsWith    string after function head
     * @return token matching with endsWith if syntax seems to be a function head else nullptr
     */
    static const Token * isFunctionHead(const Token *tok, const std::string &endsWith);

    const Settings& getSettings() const {
        return mSettings;
    }

private:
    bool createTokensFromBufferInternal(const char* data, std::size_t size, const std::string& file0);

    /** Token list */
    std::shared_ptr<TokensFrontBack> mTokensFrontBack;

    /** filenames for the tokenized source code (source + included) */
    std::vector<std::string> mFiles;

    /** Original filenames for the tokenized source code (source + included) */
    std::vector<std::string> mOrigFiles;

    /** settings */
    const Settings& mSettings;

    /** File is known to be C/C++ code */
    Standards::Language mLang{Standards::Language::None};
};

/// @}

const Token* isLambdaCaptureList(const Token* tok);
const Token* findLambdaEndTokenWithoutAST(const Token* tok);

//---------------------------------------------------------------------------
#endif // tokenlistH
