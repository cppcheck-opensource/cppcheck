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

#include "tokenlist.h"

#include "astutils.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "keywords.h"
#include "library.h"
#include "path.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "token.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <utility>
#include <set>
#include <stack>
#include <unordered_set>

#include <simplecpp.h>

// How many compileExpression recursions are allowed?
// For practical code this could be endless. But in some special torture test
// there needs to be a limit.
static constexpr int AST_MAX_DEPTH = 150;


TokenList::TokenList(const Settings& settings, Standards::Language lang)
    : mTokensFrontBack(new TokensFrontBack)
    , mSettings(settings)
{
    assert(lang != Standards::Language::None);
    mLang = lang;
}

TokenList::~TokenList()
{
    deallocateTokens();
}

//---------------------------------------------------------------------------

const std::string& TokenList::getSourceFilePath() const
{
    if (getFiles().empty()) {
        static const std::string s_empty_string;
        return s_empty_string;
    }
    return getFiles()[0];
}

//---------------------------------------------------------------------------

// Deallocate lists..
void TokenList::deallocateTokens()
{
    if (mTokensFrontBack) {
        deleteTokens(mTokensFrontBack->front);
        mTokensFrontBack->front = nullptr;
        mTokensFrontBack->back = nullptr;
    }
    // TODO: clear mOrigFiles?
    mFiles.clear();
}

// TODO: also update mOrigFiles?
int TokenList::appendFileIfNew(std::string fileName)
{
    assert(!fileName.empty());

    // Has this file been tokenized already?
    auto it = std::find_if(mFiles.cbegin(), mFiles.cend(), [&](const std::string& f) {
        return Path::sameFileName(f, fileName);
    });
    if (it != mFiles.cend())
        return static_cast<int>(std::distance(mFiles.cbegin(), it));

    assert(mTokensFrontBack->front == nullptr); // has no effect if tokens have already been created

    // The "mFiles" vector remembers what files have been tokenized..
    mFiles.push_back(std::move(fileName));

    return mFiles.size() - 1;
}

void TokenList::clangSetOrigFiles()
{
    mOrigFiles = mFiles;
}

void TokenList::deleteTokens(Token *tok)
{
    while (tok) {
        Token *next = tok->next();
        delete tok;
        tok = next;
    }
}

//---------------------------------------------------------------------------
// add a token.
//---------------------------------------------------------------------------

void TokenList::addtoken(const std::string& str, const nonneg int lineno, const nonneg int column, const nonneg int fileno, bool split)
{
    if (str.empty())
        return;

    // If token contains # characters, split it up
    if (split) {
        size_t begin = 0;
        size_t end = 0;
        while ((end = str.find("##", begin)) != std::string::npos) {
            addtoken(str.substr(begin, end - begin), lineno, fileno, false);
            addtoken("##", lineno, column, fileno, false);
            begin = end+2;
        }
        if (begin != 0) {
            addtoken(str.substr(begin), lineno, column, fileno, false);
            return;
        }
    }

    if (mTokensFrontBack->back) {
        mTokensFrontBack->back->insertToken(str);
    } else {
        mTokensFrontBack->front = new Token(*this, mTokensFrontBack);
        mTokensFrontBack->back = mTokensFrontBack->front;
        mTokensFrontBack->back->str(str);
    }

    mTokensFrontBack->back->linenr(lineno);
    mTokensFrontBack->back->column(column);
    mTokensFrontBack->back->fileIndex(fileno);
}

void TokenList::addtoken(const std::string& str, const Token *locationTok)
{
    if (str.empty())
        return;

    if (mTokensFrontBack->back) {
        mTokensFrontBack->back->insertToken(str);
    } else {
        mTokensFrontBack->front = new Token(*this, mTokensFrontBack);
        mTokensFrontBack->back = mTokensFrontBack->front;
        mTokensFrontBack->back->str(str);
    }

    mTokensFrontBack->back->linenr(locationTok->linenr());
    mTokensFrontBack->back->column(locationTok->column());
    mTokensFrontBack->back->fileIndex(locationTok->fileIndex());
}

void TokenList::addtoken(const Token * tok, const nonneg int lineno, const nonneg int column, const nonneg int fileno)
{
    if (tok == nullptr)
        return;

    if (mTokensFrontBack->back) {
        mTokensFrontBack->back->insertToken(tok->str(), tok->originalName());
    } else {
        mTokensFrontBack->front = new Token(*this, mTokensFrontBack);
        mTokensFrontBack->back = mTokensFrontBack->front;
        mTokensFrontBack->back->str(tok->str());
        if (!tok->originalName().empty())
            mTokensFrontBack->back->originalName(tok->originalName());
    }

    mTokensFrontBack->back->linenr(lineno);
    mTokensFrontBack->back->column(column);
    mTokensFrontBack->back->fileIndex(fileno);
    mTokensFrontBack->back->flags(tok->flags());
    mTokensFrontBack->back->tokType(tok->tokType());
}

void TokenList::addtoken(const Token *tok, const Token *locationTok)
{
    if (tok == nullptr || locationTok == nullptr)
        return;

    if (mTokensFrontBack->back) {
        mTokensFrontBack->back->insertToken(tok->str(), tok->originalName());
    } else {
        mTokensFrontBack->front = new Token(*this, mTokensFrontBack);
        mTokensFrontBack->back = mTokensFrontBack->front;
        mTokensFrontBack->back->str(tok->str());
        if (!tok->originalName().empty())
            mTokensFrontBack->back->originalName(tok->originalName());
    }

    mTokensFrontBack->back->flags(tok->flags());
    mTokensFrontBack->back->linenr(locationTok->linenr());
    mTokensFrontBack->back->column(locationTok->column());
    mTokensFrontBack->back->fileIndex(locationTok->fileIndex());
    mTokensFrontBack->back->tokType(tok->tokType());
}

void TokenList::addtoken(const Token *tok)
{
    if (tok == nullptr)
        return;

    if (mTokensFrontBack->back) {
        mTokensFrontBack->back->insertToken(tok->str(), tok->originalName(), tok->getMacroName());
    } else {
        mTokensFrontBack->front = new Token(*this, mTokensFrontBack);
        mTokensFrontBack->back = mTokensFrontBack->front;
        mTokensFrontBack->back->str(tok->str());
        if (!tok->originalName().empty())
            mTokensFrontBack->back->originalName(tok->originalName());
        if (!tok->getMacroName().empty())
            mTokensFrontBack->back->setMacroName(tok->getMacroName());
    }

    mTokensFrontBack->back->flags(tok->flags());
    mTokensFrontBack->back->linenr(tok->linenr());
    mTokensFrontBack->back->column(tok->column());
    mTokensFrontBack->back->fileIndex(tok->fileIndex());
    mTokensFrontBack->back->tokType(tok->tokType());
}


//---------------------------------------------------------------------------
// copyTokens - Copy and insert tokens
//---------------------------------------------------------------------------

Token *TokenList::copyTokens(Token *dest, const Token *first, const Token *last, bool one_line)
{
    std::stack<Token *> links;
    Token *tok2 = dest;
    int linenr = dest->linenr();
    const int commonFileIndex = dest->fileIndex();
    for (const Token *tok = first; tok != last->next(); tok = tok->next()) {
        tok2->insertToken(tok->str());
        tok2 = tok2->next();
        tok2->fileIndex(commonFileIndex);
        tok2->linenr(linenr);
        tok2->tokType(tok->tokType());
        tok2->flags(tok->flags());
        tok2->varId(tok->varId());
        tok2->setTokenDebug(tok->getTokenDebug());

        // Check for links and fix them up
        if (Token::Match(tok2, "(|[|{"))
            links.push(tok2);
        else if (Token::Match(tok2, ")|]|}")) {
            if (links.empty())
                return tok2;

            Token * link = links.top();

            tok2->link(link);
            link->link(tok2);

            links.pop();
        }
        if (!one_line && tok->next())
            linenr += tok->next()->linenr() - tok->linenr();
    }
    return tok2;
}

//---------------------------------------------------------------------------
// InsertTokens - Copy and insert tokens
//---------------------------------------------------------------------------

void TokenList::insertTokens(Token *dest, const Token *src, nonneg int n)
{
    // TODO: put the linking in a helper?
    std::stack<Token *> link;

    while (n > 0) {
        if (!src)
            throw InternalError(dest, std::string(__func__) + ": invalid source range", InternalError::INTERNAL);
        dest->insertToken(src->str(), src->originalName());
        dest = dest->next();

        // Set links
        if (Token::Match(dest, "(|[|{"))
            link.push(dest);
        else if (!link.empty() && Token::Match(dest, ")|]|}")) {
            Token::createMutualLinks(dest, link.top());
            link.pop();
        }

        dest->fileIndex(src->fileIndex());
        dest->linenr(src->linenr());
        dest->column(src->column());
        dest->varId(src->varId());
        dest->tokType(src->tokType());
        dest->flags(src->flags());
        if (!src->getMacroName().empty())
            dest->setMacroName(src->getMacroName());
        src  = src->next();
        --n;
    }
}

//---------------------------------------------------------------------------

bool TokenList::createTokensFromBuffer(const char* data, size_t size)
{
    return createTokensFromBufferInternal(data, size, mFiles.empty() ? "" : *mFiles.cbegin());
}

//---------------------------------------------------------------------------

bool TokenList::createTokensFromBufferInternal(const char* data, size_t size, const std::string& file0)
{
    simplecpp::OutputList outputList;
    simplecpp::TokenList tokens({data, size}, mFiles, file0, &outputList);

    createTokens(std::move(tokens));

    return outputList.empty();
}

//---------------------------------------------------------------------------

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
void TokenList::createTokens(simplecpp::TokenList&& tokenList)
{
    // TODO: what to do if the list has been filled already? clear mTokensFrontBack?

    // tokenList.cfront() might be NULL if the file contained nothing to tokenize so we need to check the files instead
    if (!tokenList.getFiles().empty()) {
        // this is a copy
        // TODO: this points to mFiles when called from createTokens(std::istream &, const std::string&)
        mOrigFiles = mFiles = tokenList.getFiles();
    }
    else {
        // TODO: clear mOrigFiles?
        mFiles.clear();
    }

    for (const simplecpp::Token *tok = tokenList.cfront(); tok;) {

        // TODO: move from TokenList
        std::string str = tok->str();

        // Float literal
        if (str.size() > 1 && str[0] == '.' && std::isdigit(str[1]))
            str = '0' + str;

        if (mTokensFrontBack->back) {
            mTokensFrontBack->back->insertToken(str);
        } else {
            mTokensFrontBack->front = new Token(*this, mTokensFrontBack);
            mTokensFrontBack->back = mTokensFrontBack->front;
            mTokensFrontBack->back->str(str);
        }

        mTokensFrontBack->back->fileIndex(tok->location.fileIndex);
        mTokensFrontBack->back->linenr(tok->location.line);
        mTokensFrontBack->back->column(tok->location.col);
        if (!tok->macro.empty())
            mTokensFrontBack->back->setMacroName(tok->macro);

        tok = tok->next;
        if (tok)
            tokenList.deleteToken(tok->previous);
    }

    if (mSettings.relativePaths) {
        for (std::string & mFile : mFiles)
            mFile = Path::getRelativePath(mFile, mSettings.basePaths);
    }

    Token::assignProgressValues(mTokensFrontBack->front);
}

//---------------------------------------------------------------------------

std::size_t TokenList::calculateHash() const
{
    std::string hashData;
    for (const Token* tok = front(); tok; tok = tok->next()) {
        hashData += std::to_string(tok->flags());
        hashData += std::to_string(tok->varId());
        hashData += std::to_string(tok->tokType());
        hashData += tok->str();
        hashData += tok->originalName();
    }
    return (std::hash<std::string>{})(hashData);
}


//---------------------------------------------------------------------------

namespace {
    struct AST_state {
        std::stack<Token*> op;
        int depth{};
        int inArrayAssignment{};
        bool cpp;
        int assign{};
        bool inCase{}; // true from case to :
        bool inGeneric{};
        bool stopAtColon{}; // help to properly parse ternary operators
        const Token* functionCallEndPar{};
        explicit AST_state(bool cpp) : cpp(cpp) {}
    };
}

static Token* skipDecl(Token* tok, std::vector<Token*>* inner = nullptr)
{
    auto isDecltypeFuncParam = [](const Token* tok) -> bool {
        if (!Token::simpleMatch(tok, ")"))
            return false;
        tok = tok->next();
        while (Token::Match(tok, "*|&|&&|const"))
            tok = tok->next();
        if (Token::simpleMatch(tok, "("))
            tok = tok->link()->next();
        return Token::Match(tok, "%name%| ,|)");
    };

    if (!Token::Match(tok->previous(), "( %name%"))
        return tok;
    Token *vartok = tok;
    while (Token::Match(vartok, "%name%|*|&|::|<")) {
        if (vartok->str() == "<") {
            if (vartok->link())
                vartok = vartok->link();
            else
                return tok;
        } else if (Token::Match(vartok, "%var% [:=({]")) {
            return vartok;
        } else if (Token::Match(vartok, "decltype|typeof (") && !isDecltypeFuncParam(tok->linkAt(1))) {
            if (inner)
                inner->push_back(vartok->tokAt(2));
            return vartok->linkAt(1)->next();
        }
        vartok = vartok->next();
    }
    return tok;
}

static bool iscast(const Token *tok, bool cpp)
{
    if (!Token::Match(tok, "( ::| %name%"))
        return false;

    if (Token::simpleMatch(tok->link(), ") ( )"))
        return false;

    if (Token::Match(tok->link(), ") %assign%|,|..."))
        return false;

    if (tok->previous() && tok->previous()->isName() && !Token::Match(tok->previous(), "return|case") &&
        (!cpp || !Token::Match(tok->previous(), "delete|throw")))
        return false;

    if (Token::simpleMatch(tok->previous(), ">") && tok->linkAt(-1))
        return false;

    if (Token::Match(tok, "( (| typeof (") && Token::Match(tok->link(), ") %num%"))
        return true;

    if (Token::Match(tok->link(), ") }|)|]|;"))
        return false;

    if (Token::Match(tok->link(), ") ++|-- [;)]"))
        return false;

    if (Token::Match(tok->link(), ") %cop%") && !Token::Match(tok->link(), ") [&*+-~!]"))
        return false;

    if (Token::Match(tok->previous(), "= ( %name% ) {") && tok->next()->varId() == 0)
        return true;

    bool type = false;
    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
        if (tok2->varId() != 0)
            return false;
        if (cpp && !type && tok2->str() == "new")
            return false;

        while (tok2->link() && Token::Match(tok2, "(|[|<"))
            tok2 = tok2->link()->next();

        if (tok2->str() == ")") {
            if (Token::simpleMatch(tok2, ") (") && Token::simpleMatch(tok2->linkAt(1), ") ."))
                return true;
            if (Token::simpleMatch(tok2, ") {") && !type) {
                const Token *tok3 = tok2->linkAt(1);
                while (tok3 != tok2 && Token::Match(tok3, "[{}]"))
                    tok3 = tok3->previous();
                return tok3->str() != ";";
            }
            const bool res = type || tok2->strAt(-1) == "*" || Token::simpleMatch(tok2, ") ~") ||
                             (Token::Match(tok2, ") %any%") &&
                              (!tok2->next()->isOp() || Token::Match(tok2->next(), "!|~|++|--")) &&
                              !Token::Match(tok2->next(), "[[]);,?:.]"));
            return res;
        }

        if (Token::Match(tok2, "&|&& )"))
            return true;

        if (!Token::Match(tok2, "%name%|*|::"))
            return false;

        if (tok2->isStandardType() && (tok2->strAt(1) != "(" || Token::Match(tok2->next(), "( * *| )")))
            type = true;
    }

    return false;
}

// int(1), int*(2), ..
static Token * findCppTypeInitPar(Token *tok)
{
    if (!tok || !Token::Match(tok->previous(), "[,()] %name%"))
        return nullptr;
    bool istype = false;
    while (Token::Match(tok, "%name%|::|<")) {
        if (tok->str() == "<") {
            tok = tok->link();
            if (!tok)
                return nullptr;
        }
        istype |= tok->isStandardType();
        tok = tok->next();
    }
    if (!istype)
        return nullptr;
    if (!Token::Match(tok, "[*&]"))
        return nullptr;
    while (Token::Match(tok, "[*&]"))
        tok = tok->next();
    return (tok && tok->str() == "(") ? tok : nullptr;
}

// X{} X<Y>{} etc
static bool iscpp11init_impl(const Token * tok);
static bool iscpp11init(const Token * const tok)
{
    if (tok->isCpp11init() == Token::Cpp11init::UNKNOWN)
        tok->setCpp11init(iscpp11init_impl(tok));
    return tok->isCpp11init() == Token::Cpp11init::CPP11INIT;
}

static bool iscpp11init_impl(const Token * const tok)
{
    if (Token::simpleMatch(tok, "{") && Token::simpleMatch(tok->link()->previous(), "; }"))
        return false;
    const Token *nameToken = tok;
    while (nameToken && nameToken->str() == "{") {
        if (nameToken->isCpp11init() != Token::Cpp11init::UNKNOWN)
            return nameToken->isCpp11init() == Token::Cpp11init::CPP11INIT;
        nameToken = nameToken->previous();
    }
    if (!nameToken)
        return false;
    if (nameToken->str() == ")" && Token::simpleMatch(nameToken->link()->previous(), "decltype (") &&
        !Token::simpleMatch(nameToken->link()->tokAt(-2), "."))
        nameToken = nameToken->link()->previous();
    if (Token::simpleMatch(nameToken, ", {"))
        return true;
    if (nameToken->str() == ">" && nameToken->link())
        nameToken = nameToken->link()->previous();
    if (Token::Match(nameToken, "]|*")) {
        const Token* tok2 = nameToken;
        if (tok2->link()) {
            while (tok2 && tok2->link())
                tok2 = tok2->link()->previous();
        }
        else
            tok2 = tok2->previous();
        while (Token::Match(tok2, "%type%|::|*") && !tok2->isKeyword())
            tok2 = tok2->previous();
        if (Token::Match(tok2, "new|%var%"))
            return true;
    }

    auto isCaseStmt = [](const Token* colonTok) {
        if (!Token::Match(colonTok->tokAt(-1), "%name%|%num%|%char%|) :"))
            return false;
        if (const Token* castTok = colonTok->linkAt(-1)) {
            if (Token::simpleMatch(castTok->astParent(), "case"))
                return true;
        }
        if (findParent(colonTok->previous(), [](const Token *parent){
            return parent->str() == "case";
        }))
            return true;
        const Token* caseTok = colonTok->tokAt(-2);
        while (caseTok && Token::Match(caseTok->tokAt(-1), "::|%name%"))
            caseTok = caseTok->tokAt(-1);
        return Token::simpleMatch(caseTok, "case");
    };

    const Token *endtok = nullptr;
    if (Token::Match(nameToken, "%name%|return|: {") && !isCaseStmt(nameToken) &&
        (!Token::simpleMatch(nameToken->tokAt(2), "[") || findLambdaEndScope(nameToken->tokAt(2))))
        endtok = nameToken->linkAt(1);
    else if (Token::Match(nameToken,"%name% <") && Token::simpleMatch(nameToken->linkAt(1),"> {"))
        endtok = nameToken->linkAt(1)->linkAt(1);
    else if (Token::Match(nameToken->previous(), "%name%|> ( {"))
        endtok = nameToken->linkAt(1);
    else if (Token::simpleMatch(nameToken, "decltype") && nameToken->linkAt(1))
        endtok = nameToken->linkAt(1)->linkAt(1);
    else
        return false;
    if (Token::Match(nameToken, "else|try|do|const|constexpr|override|volatile|&|&&"))
        return false;
    if (Token::simpleMatch(nameToken->previous(), ". void {") && nameToken->previous()->originalName() == "->")
        return false; // trailing return type. The only function body that can contain no semicolon is a void function.
    if (Token::simpleMatch(nameToken->previous(), "namespace") || Token::simpleMatch(nameToken, "namespace") /*anonymous namespace*/)
        return false;
    if (precedes(nameToken->next(), endtok) && !Token::Match(nameToken, "return|:")) {
        // If there is semicolon between {..} this is not a initlist
        for (const Token *tok2 = nameToken->next(); tok2 != endtok; tok2 = tok2->next()) {
            if (tok2->str() == ";")
                return false;
            const Token * lambdaEnd = findLambdaEndScope(tok2);
            if (lambdaEnd)
                tok2 = lambdaEnd;
        }
    }
    // There is no initialisation for example here: 'class Fred {};'
    if (!Token::simpleMatch(endtok, "} ;"))
        return true;
    const Token *prev = nameToken;
    while (Token::Match(prev, "%name%|::|:|<|>|(|)|,|%num%|%cop%|...")) {
        if (Token::Match(prev, "class|struct|union|enum"))
            return false;

        prev = prev->previous();
    }
    return true;
}

static bool isQualifier(const Token* tok)
{
    while (Token::Match(tok, "&|&&|*"))
        tok = tok->next();
    return Token::Match(tok, "{|;");
}

/**
 * @throws InternalError thrown if maximum AST depth is exceeded
 */
static void compileUnaryOp(Token *&tok, AST_state& state, void (*f)(Token *&tok, AST_state& state))
{
    Token *unaryop = tok;
    if (f) {
        tok = tok->next();
        state.depth++;
        if (state.depth > AST_MAX_DEPTH)
            throw InternalError(tok, "maximum AST depth exceeded", InternalError::AST);
        if (tok)
            f(tok, state);
        state.depth--;
    }

    if (!state.op.empty() && (!precedes(state.op.top(), unaryop) || unaryop->isIncDecOp() || Token::Match(unaryop, "[({[]"))) { // nullary functions, empty lists/arrays
        unaryop->astOperand1(state.op.top());
        state.op.pop();
    }
    state.op.push(unaryop);
}

static void skipGenericType(Token *&tok)
{
    Token *skip = tok;
    while (Token::Match(skip, "%name%|*|:|(")) {
        if (skip->link()) {
            skip = skip->link()->next();
            continue;
        }
        if (Token::simpleMatch(skip, ":")) {
            tok = skip->next();
            return;
        }
        skip = skip->next();
    }
}

/**
 * @throws InternalError thrown if maximum AST depth is exceeded
 */
static void compileBinOp(Token *&tok, AST_state& state, void (*f)(Token *&tok, AST_state& state))
{
    Token *binop = tok;
    if (f) {
        tok = tok->next();
        if (Token::simpleMatch(binop, ",") && state.inGeneric)
            skipGenericType(tok);
        const bool inGenericSaved = state.inGeneric;
        state.inGeneric = false;
        if (Token::Match(binop, "::|. ~"))
            tok = tok->next();
        state.depth++;
        if (tok && state.depth <= AST_MAX_DEPTH)
            f(tok, state);
        if (state.depth > AST_MAX_DEPTH)
            throw InternalError(tok, "maximum AST depth exceeded", InternalError::AST);
        state.depth--;
        state.inGeneric = inGenericSaved;
    }

    // TODO: Should we check if op is empty.
    // * Is it better to add assertion that it isn't?
    // * Write debug warning if it's empty?
    if (!state.op.empty()) {
        binop->astOperand2(state.op.top());
        state.op.pop();
    }
    if (!state.op.empty()) {
        binop->astOperand1(state.op.top());
        state.op.pop();
    }
    state.op.push(binop);
}

static void compileExpression(Token *&tok, AST_state& state);

/**
 * @throws InternalError thrown if unexpected tokens are encountered
 */
static void compileTerm(Token *&tok, AST_state& state)
{
    if (!tok)
        return;
    if (Token::Match(tok, "L %str%|%char%"))
        tok = tok->next();
    if (state.inArrayAssignment && Token::Match(tok->previous(), "[{,] . %name%")) { // Jump over . in C style struct initialization
        state.op.push(tok);
        tok->astOperand1(tok->next());
        tok = tok->tokAt(2);
    }
    if (state.inArrayAssignment && Token::Match(tok->previous(), "[{,] [ %num%|%name% ]")) {
        state.op.push(tok);
        tok->astOperand1(tok->next());
        tok = tok->tokAt(3);
    }
    if (tok->isLiteral()) {
        state.op.push(tok);
        do {
            tok = tok->next();
        } while (Token::Match(tok, "%name%|%str%"));
    } else if (tok->isName()) {
        if (Token::Match(tok, "return|case") || (state.cpp && (tok->str() == "throw"))) {
            if (tok->str() == "case")
                state.inCase = true;
            const bool tokIsReturn = tok->str() == "return";
            const bool stopAtColon = state.stopAtColon;
            state.stopAtColon=true;
            compileUnaryOp(tok, state, compileExpression);
            state.stopAtColon=stopAtColon;
            if (tokIsReturn)
                state.op.pop();
            if (state.inCase && Token::simpleMatch(tok, ": ;")) {
                state.inCase = false;
                tok = tok->next();
            }
        } else if (Token::Match(tok, "sizeof !!(")) {
            compileUnaryOp(tok, state, compileExpression);
            state.op.pop();
        } else if (state.cpp && findCppTypeInitPar(tok)) {  // int(0), int*(123), ..
            tok = findCppTypeInitPar(tok);
            state.op.push(tok);
            tok = tok->tokAt(2);
        } else if (state.cpp && iscpp11init(tok)) { // X{} X<Y>{} etc
            state.op.push(tok);
            tok = tok->next();
            if (tok->str() == "<")
                tok = tok->link()->next();

            if (Token::Match(tok, "{ . %name% =|{")) {
                const Token* end = tok->link();
                const int inArrayAssignment = state.inArrayAssignment;
                state.inArrayAssignment = 1;
                compileBinOp(tok, state, compileExpression);
                state.inArrayAssignment = inArrayAssignment;
                if (tok == end)
                    tok = tok->next();
                else
                    throw InternalError(tok, "Syntax error. Unexpected tokens in designated initializer.", InternalError::AST);
            }
        } else if (!state.cpp || !Token::Match(tok, "new|delete %name%|*|&|::|(|[")) {
            std::vector<Token*> inner;
            tok = skipDecl(tok, &inner);
            for (Token* tok3 : inner) {
                AST_state state1(state.cpp);
                compileExpression(tok3, state1);
            }
            bool repeat = true;
            while (repeat) {
                repeat = false;
                if (Token::Match(tok->next(), "%name%")) {
                    tok = tok->next();
                    repeat = true;
                }
                if (Token::simpleMatch(tok->next(), "<") && Token::Match(tok->linkAt(1), "> %name%")) {
                    tok = tok->linkAt(1)->next();
                    repeat = true;
                }
            }
            state.op.push(tok);
            if (Token::Match(tok, "%name% <") && tok->linkAt(1))
                tok = tok->linkAt(1);
            else if (Token::Match(tok, "%name% ...") || (state.op.size() == 1 && state.depth == 0 && Token::Match(tok->tokAt(-3), "!!& ) ( %name% ) =")))
                tok = tok->next();
            tok = tok->next();
            if (Token::Match(tok, "%str%")) {
                while (Token::Match(tok, "%name%|%str%"))
                    tok = tok->next();
            }
            if (Token::Match(tok, "%name% %assign%"))
                tok = tok->next();
        }
    } else if (tok->str() == "{") {
        const Token *prev = tok->previous();
        if (Token::simpleMatch(prev, ") {") && iscast(prev->link(), state.cpp))
            prev = prev->link()->previous();
        if (Token::simpleMatch(tok->link(),"} [")) {
            tok = tok->next();
        } else if ((state.cpp && iscpp11init(tok)) || Token::simpleMatch(tok->previous(), "] {")) {
            Token *const end = tok->link();
            if (state.op.empty() || Token::Match(tok->previous(), "[{,]") || Token::Match(tok->tokAt(-2), "%name% (")) {
                if (Token::Match(tok->tokAt(-1), "!!, { . %name% =|{") && !Token::simpleMatch(tok->tokAt(-1), "(")) {
                    const int inArrayAssignment = state.inArrayAssignment;
                    state.inArrayAssignment = 1;
                    compileBinOp(tok, state, compileExpression);
                    state.inArrayAssignment = inArrayAssignment;
                } else if (Token::simpleMatch(tok, "{ }")) {
                    state.op.push(tok);
                    tok = tok->next();
                } else {
                    compileUnaryOp(tok, state, compileExpression);
                    if (precedes(tok,end)) // typically for something like `MACRO(x, { if (c) { ... } })`, where end is the last curly, and tok is the open curly for the if
                        tok = end;
                }
            } else if (tok->next() == end) {
                tok->astOperand1(state.op.top());
                state.op.pop();
                tok = tok->next();
            } else
                compileBinOp(tok, state, compileExpression);
            if (tok != end)
                throw InternalError(tok, "Syntax error. Unexpected tokens in initializer.", InternalError::AST);
            if (tok->next())
                tok = tok->next();
        } else if (state.cpp && Token::Match(tok->tokAt(-2), "%name% ( {") && !Token::findsimplematch(tok, ";", tok->link())) {
            if (Token::simpleMatch(tok, "{ }"))
                tok = tok->tokAt(2);
            else {
                Token *tok1 = tok;
                state.inArrayAssignment++;
                compileUnaryOp(tok, state, compileExpression);
                state.inArrayAssignment--;
                tok = tok1->link()->next();
            }
        } else if ((!state.inArrayAssignment && !Token::simpleMatch(prev, "=")) || Token::simpleMatch(prev, "?")) {
            state.op.push(tok);
            tok = tok->link()->next();
        } else {
            if (tok->link() != tok->next()) {
                state.inArrayAssignment++;
                compileUnaryOp(tok, state, compileExpression);
                if (Token::Match(tok, "} [,};]") && state.inArrayAssignment > 0) {
                    tok = tok->next();
                    state.inArrayAssignment--;
                }
            } else {
                state.op.push(tok);
                tok = tok->tokAt(2);
            }
        }
    }
}

static void compileScope(Token *&tok, AST_state& state)
{
    compileTerm(tok, state);
    while (tok) {
        if (tok->str() == "::") {
            const Token *lastOp = state.op.empty() ? nullptr : state.op.top();
            if (Token::Match(lastOp, ":: %name%"))
                lastOp = lastOp->next();
            if (Token::Match(lastOp, "%name%") &&
                (lastOp->next() == tok || (Token::Match(lastOp, "%name% <") && lastOp->linkAt(1) && tok == lastOp->linkAt(1)->next())))
                compileBinOp(tok, state, compileTerm);
            else
                compileUnaryOp(tok, state, compileTerm);
        } else break;
    }
}

static bool isPrefixUnary(const Token* tok, bool cpp)
{
    if (cpp && Token::simpleMatch(tok->previous(), "* [") && Token::simpleMatch(tok->link(), "] {")) {
        for (const Token* prev = tok->previous(); Token::Match(prev, "%name%|::|*|&|>|>>"); prev = prev->previous()) {
            if (Token::Match(prev, ">|>>")) {
                if (!prev->link())
                    break;
                prev = prev->link();
            }
            if (prev->str() == "new")
                return false;
        }
    }
    if (!tok->previous()
        || ((Token::Match(tok->previous(), "(|[|{|%op%|;|?|:|,|.|case|return|::") || (cpp && tok->strAt(-1) == "throw"))
            && (tok->previous()->tokType() != Token::eIncDecOp || tok->tokType() == Token::eIncDecOp)))
        return true;

    if (tok->strAt(-1) == "}") {
        const Token* parent = tok->linkAt(-1)->tokAt(-1);
        return !Token::Match(parent, "%type%") || parent->isKeyword();
    }

    if (tok->str() == "*" && tok->previous()->tokType() == Token::eIncDecOp && isPrefixUnary(tok->previous(), cpp))
        return true;

    return tok->strAt(-1) == ")" && iscast(tok->linkAt(-1), cpp);
}

/**
 * @throws InternalError thrown if unexpected tokens are encountered
 */
static void compilePrecedence2(Token *&tok, AST_state& state)
{
    auto doCompileScope = [&](const Token* tok) -> bool {
        const bool isStartOfCpp11Init = state.cpp && tok && tok->str() == "{" && iscpp11init(tok);
        if (isStartOfCpp11Init || Token::simpleMatch(tok, "(")) {
            tok = tok->previous();
            while (Token::simpleMatch(tok, "*"))
                tok = tok->previous();
            while (tok && Token::Match(tok->previous(), ":: %type%"))
                tok = tok->tokAt(-2);
            if (tok && !tok->isKeyword())
                tok = tok->previous();
            return !Token::Match(tok, "new ::| %type%");
        }
        return !findLambdaEndTokenWithoutAST(tok);
    };

    bool isNew = true;
    if (doCompileScope(tok)) {
        compileScope(tok, state);
        isNew = false;
    }
    while (tok) {
        if (tok->tokType() == Token::eIncDecOp && !isPrefixUnary(tok, state.cpp)) {
            compileUnaryOp(tok, state, compileScope);
        } else if (tok->str() == "...") {
            if (!Token::simpleMatch(tok->previous(), ")"))
                state.op.push(tok);
            tok = tok->next();
            break;
        } else if (tok->str() == "." && tok->strAt(1) != "*") {
            if (tok->strAt(1) == ".") {
                state.op.push(tok);
                tok = tok->tokAt(3);
                break;
            }
            if (!Token::Match(tok->tokAt(-1), "[{,]"))
                compileBinOp(tok, state, compileScope);
            else
                compileUnaryOp(tok, state, compileScope);
        } else if (tok->str() == "[") {
            if (state.cpp && isPrefixUnary(tok, /*cpp*/ true) && Token::Match(tok->link(), "] (|{|<")) { // Lambda
                // What we do here:
                // - Nest the round bracket under the square bracket.
                // - Nest what follows the lambda (if anything) with the lambda opening [
                // - Compile the content of the lambda function as separate tree (this is done later)
                // this must be consistent with isLambdaCaptureList
                Token* const squareBracket = tok;
                // Parse arguments in the capture list
                if (tok->strAt(1) != "]") {
                    Token* tok2 = tok->next();
                    AST_state state2(state.cpp);
                    compileExpression(tok2, state2);
                    if (!state2.op.empty()) {
                        squareBracket->astOperand2(state2.op.top());
                    }
                }

                const bool hasTemplateArg = Token::simpleMatch(squareBracket->link(), "] <") &&
                                            Token::simpleMatch(squareBracket->link()->linkAt(1), "> (");
                if (Token::simpleMatch(squareBracket->link(), "] (") || hasTemplateArg) {
                    Token* const roundBracket = hasTemplateArg ? squareBracket->link()->linkAt(1)->next() : squareBracket->link()->next();
                    Token* curlyBracket = roundBracket->link()->next();
                    while (Token::Match(curlyBracket, "mutable|const|constexpr|consteval"))
                        curlyBracket = curlyBracket->next();
                    if (Token::simpleMatch(curlyBracket, "noexcept")) {
                        if (Token::simpleMatch(curlyBracket->next(), "(")) {
                            AST_state state2(state.cpp);
                            Token *tok2 = curlyBracket->tokAt(2);
                            compileExpression(tok2, state2);
                            curlyBracket = curlyBracket->linkAt(1)->next();
                        } else
                            curlyBracket = curlyBracket->next();
                    }
                    if (curlyBracket && curlyBracket->originalName() == "->") {
                        if (Token::simpleMatch(curlyBracket->next(), "decltype (")) {
                            AST_state state2(state.cpp);
                            Token *tok2 = curlyBracket->tokAt(3);
                            compileExpression(tok2, state2);
                        }
                        curlyBracket = findTypeEnd(curlyBracket->next());
                    }
                    if (curlyBracket && curlyBracket->str() == "{") {
                        squareBracket->astOperand1(roundBracket);
                        roundBracket->astOperand1(curlyBracket);
                        state.op.push(squareBracket);
                        tok = curlyBracket->link()->next();
                        continue;
                    }
                } else {
                    Token* const curlyBracket = squareBracket->link()->next();
                    squareBracket->astOperand1(curlyBracket);
                    state.op.push(squareBracket);
                    tok = curlyBracket->link() ? curlyBracket->link()->next() : nullptr;
                    continue;
                }
            }

            Token* const tok2 = tok;
            if (tok->strAt(1) != "]") {
                compileBinOp(tok, state, compileExpression);
                if (Token::Match(tok2->previous(), "%type%|* [") && Token::Match(tok, "] { !!}")) {
                    tok = tok->next();
                    Token* const tok3 = tok;
                    compileBinOp(tok, state, compileExpression);
                    if (tok != tok3->link())
                        throw InternalError(tok, "Syntax error in {..}", InternalError::AST);
                    tok = tok->next();
                    continue;
                }
            }
            else
                compileUnaryOp(tok, state, compileExpression);
            tok = tok2->link()->next();
        } else if ((Token::simpleMatch(tok->tokAt(-1), "requires {") && tok->tokAt(-1)->isKeyword())
                   || (Token::simpleMatch(tok->tokAt(-1), ")")
                       && tok->linkAt(-1)
                       && Token::simpleMatch(tok->linkAt(-1)->tokAt(-1), "requires (") && tok->linkAt(-1)->tokAt(-1)->isKeyword())) {
            if (!tok->link())
                throw InternalError(tok, "Syntax error, token has no link.", InternalError::AST);
            tok->astOperand1(state.op.top());
            state.op.pop();
            state.op.push(tok);
            tok = tok->link()->next();
            continue;
        } else if (Token::simpleMatch(tok, "( {") && Token::simpleMatch(tok->linkAt(1)->previous(), "; } )") &&
                   !Token::Match(tok->previous(), "%name% (")) {
            state.op.push(tok->next());
            tok = tok->link()->next();
            continue;
        } else if (tok->str() == "(" &&
                   (!iscast(tok, state.cpp) || Token::Match(tok->previous(), "if|while|for|switch|catch"))) {
            const bool inGenericSaved = state.inGeneric;
            if (Token::simpleMatch(tok->previous(), "_Generic"))
                state.inGeneric = true;
            Token* tok2 = tok;
            tok = tok->next();
            const bool opPrevTopSquare = !state.op.empty() && state.op.top() && state.op.top()->str() == "[";
            const std::size_t oldOpSize = state.op.size();
            compileExpression(tok, state);
            tok = tok2;
            if ((oldOpSize > 0 && (isNew || Token::simpleMatch(tok->previous(), "} (")))
                || (tok->previous() && tok->previous()->isName() && !Token::Match(tok->previous(), "return|case") && (!state.cpp || !Token::Match(tok->previous(), "throw|delete")))
                || (tok->strAt(-1) == "]" && (!state.cpp || !Token::Match(tok->linkAt(-1)->previous(), "new|delete")))
                || (tok->strAt(-1) == ">" && tok->linkAt(-1))
                || (tok->strAt(-1) == ")" && !iscast(tok->linkAt(-1), state.cpp)) // Don't treat brackets to clarify precedence as function calls
                || (tok->strAt(-1) == "}" && opPrevTopSquare)) {
                const bool operandInside = oldOpSize < state.op.size();
                if (operandInside)
                    compileBinOp(tok, state, nullptr);
                else
                    compileUnaryOp(tok, state, nullptr);
            }
            state.inGeneric = inGenericSaved;
            tok = tok->link()->next();
            if (Token::simpleMatch(tok, "::"))
                compileBinOp(tok, state, compileScope);
        } else if (iscast(tok, state.cpp) && Token::simpleMatch(tok->link(), ") {") &&
                   Token::simpleMatch(tok->link()->linkAt(1), "} [")) {
            Token *cast = tok;
            tok = tok->link()->next();
            Token *tok1 = tok;
            compileUnaryOp(tok, state, compileExpression);
            cast->astOperand1(tok1);
            tok = tok1->link()->next();
        } else if (state.cpp && tok->str() == "{" && iscpp11init(tok)) {
            Token* end = tok->link();
            if (Token::simpleMatch(tok, "{ }"))
            {
                compileUnaryOp(tok, state, nullptr);
                tok = tok->next();
            }
            else
            {
                compileBinOp(tok, state, compileExpression);
            }
            if (tok == end)
                tok = end->next();
            else
                throw InternalError(tok, "Syntax error. Unexpected tokens in initializer.", InternalError::AST);
        } else
            break;
    }
}

static void compilePrecedence3(Token *&tok, AST_state& state)
{
    compilePrecedence2(tok, state);
    while (tok) {
        if ((Token::Match(tok, "[+-!~*&]") || tok->tokType() == Token::eIncDecOp) &&
            isPrefixUnary(tok, state.cpp)) {
            if (Token::Match(tok, "* [*,)]")) {
                Token* tok2 = tok->next();
                while (tok2->next() && tok2->str() == "*")
                    tok2 = tok2->next();
                if (Token::Match(tok2, "[>),]")) {
                    tok = tok2;
                    continue;
                }
            }
            compileUnaryOp(tok, state, compilePrecedence3);
        } else if (tok->str() == "(" && iscast(tok, state.cpp)) {
            Token* castTok = tok;
            castTok->isCast(true);
            tok = tok->link()->next();
            const int inArrayAssignment = state.inArrayAssignment;
            if (tok && tok->str() == "{")
                state.inArrayAssignment = 1;
            compilePrecedence3(tok, state);
            state.inArrayAssignment = inArrayAssignment;
            compileUnaryOp(castTok, state, nullptr);
        } else if (state.cpp && Token::Match(tok, "new %name%|::|(")) {
            Token* newtok = tok;
            tok = tok->next();
            bool innertype = false;
            if (tok->str() == "(") {
                if (Token::Match(tok, "( &| %name%") && Token::Match(tok->link(), ") ( %type%") && Token::simpleMatch(tok->link()->linkAt(1), ") ("))
                    tok = tok->link()->next();
                if (Token::Match(tok->link(), ") ::| %type%")) {
                    if (Token::Match(tok, "( !!)")) {
                        Token *innerTok = tok->next();
                        AST_state innerState(true);
                        compileExpression(innerTok, innerState);
                    }
                    tok = tok->link()->next();
                } else if (Token::Match(tok, "( %type%") && Token::Match(tok->link(), ") [();,[]")) {
                    tok = tok->next();
                    innertype = true;
                } else if (Token::Match(tok, "( &| %name%") && Token::simpleMatch(tok->link(), ") (")) {
                    tok = tok->next();
                    innertype = true;
                } else {
                    /* bad code */
                    continue;
                }
            }

            Token* leftToken = tok;
            if (Token::simpleMatch(tok, "::")) {
                tok->astOperand1(tok->next());
                tok = tok->next();
            }
            else {
                while (Token::Match(tok->next(), ":: %name%")) {
                    Token* scopeToken = tok->next(); //The ::
                    scopeToken->astOperand1(leftToken);
                    scopeToken->astOperand2(scopeToken->next());
                    leftToken = scopeToken;
                    tok = scopeToken->next();
                }
            }

            state.op.push(tok);
            while (Token::Match(tok, "%name%|*|&|<|::")) {
                if (tok->link())
                    tok = tok->link();
                tok = tok->next();
            }
            if (Token::Match(tok, "( const| %type% ) (")) {
                state.op.push(tok->next());
                tok = tok->link()->next();
                compileBinOp(tok, state, compilePrecedence2);
            } else if (Token::Match(tok, "(|{|["))
                compilePrecedence2(tok, state);
            else if (innertype && Token::simpleMatch(tok, ") [")) {
                tok = tok->next();
                compilePrecedence2(tok, state);
            }
            compileUnaryOp(newtok, state, nullptr);
            if (Token::simpleMatch(newtok->previous(), ":: new")) {
                newtok->previous()->astOperand1(newtok);
                state.op.pop();
            }
            if (innertype && Token::simpleMatch(tok, ") ,"))
                tok = tok->next();
        } else if (state.cpp && Token::Match(tok, "delete %name%|*|&|::|(|[")) {
            Token* tok2 = tok;
            tok = tok->next();
            if (tok && tok->str() == "[")
                tok = tok->link()->next();
            compilePrecedence3(tok, state);
            compileUnaryOp(tok2, state, nullptr);
            if (Token::simpleMatch(tok2->previous(), ":: delete")) {
                tok2->previous()->astOperand1(tok2);
                state.op.pop();
            }
        }
        // TODO: Handle sizeof
        else break;
    }
}

static void compilePointerToElem(Token *&tok, AST_state& state)
{
    compilePrecedence3(tok, state);
    while (tok) {
        if (Token::simpleMatch(tok, ". *")) {
            compileBinOp(tok, state, compilePrecedence3);
        } else break;
    }
}

static void compileMulDiv(Token *&tok, AST_state& state)
{
    compilePointerToElem(tok, state);
    while (tok) {
        if (Token::Match(tok, "[/%]") || (tok->str() == "*" && !tok->astOperand1() && !isQualifier(tok))) {
            if (Token::Match(tok, "* [*,)]")) {
                Token* tok2 = tok->next();
                while (tok2->next() && tok2->str() == "*")
                    tok2 = tok2->next();
                if (Token::Match(tok2, "[>),]")) {
                    tok = tok2;
                    break;
                }
            }
            compileBinOp(tok, state, compilePointerToElem);
        } else break;
    }
}

static void compileAddSub(Token *&tok, AST_state& state)
{
    compileMulDiv(tok, state);
    while (tok) {
        if (Token::Match(tok, "+|-") && !tok->astOperand1()) {
            compileBinOp(tok, state, compileMulDiv);
        } else break;
    }
}

static void compileShift(Token *&tok, AST_state& state)
{
    compileAddSub(tok, state);
    while (tok) {
        if (Token::Match(tok, "<<|>>")) {
            compileBinOp(tok, state, compileAddSub);
        } else break;
    }
}

static void compileThreewayComp(Token *&tok, AST_state& state)
{
    compileShift(tok, state);
    while (tok) {
        if (tok->str() == "<=>") {
            compileBinOp(tok, state, compileShift);
        } else break;
    }
}

static void compileRelComp(Token *&tok, AST_state& state)
{
    compileThreewayComp(tok, state);
    while (tok) {
        if (Token::Match(tok, "<|<=|>=|>") && !tok->link()) {
            compileBinOp(tok, state, compileThreewayComp);
        } else break;
    }
}

static void compileEqComp(Token *&tok, AST_state& state)
{
    compileRelComp(tok, state);
    while (tok) {
        if (Token::Match(tok, "==|!=")) {
            compileBinOp(tok, state, compileRelComp);
        } else break;
    }
}

static void compileAnd(Token *&tok, AST_state& state)
{
    compileEqComp(tok, state);
    while (tok) {
        if (tok->str() == "&" && !tok->astOperand1() && !isQualifier(tok)) {
            Token* tok2 = tok->next();
            if (!tok2)
                break;
            if (tok2->str() == "&")
                tok2 = tok2->next();
            if (state.cpp && Token::Match(tok2, ",|)")) {
                tok = tok2;
                break; // rValue reference
            }
            compileBinOp(tok, state, compileEqComp);
        } else break;
    }
}

static void compileXor(Token *&tok, AST_state& state)
{
    compileAnd(tok, state);
    while (tok) {
        if (tok->str() == "^") {
            compileBinOp(tok, state, compileAnd);
        } else break;
    }
}

static void compileOr(Token *&tok, AST_state& state)
{
    compileXor(tok, state);
    while (tok) {
        if (tok->str() == "|") {
            compileBinOp(tok, state, compileXor);
        } else break;
    }
}

static void compileLogicAnd(Token *&tok, AST_state& state)
{
    compileOr(tok, state);
    while (tok) {
        if (tok->str() == "&&" && !isQualifier(tok)) {
            if (!tok->astOperand1()) {
                Token* tok2 = tok->next();
                if (!tok2)
                    break;
                if (state.cpp && Token::Match(tok2, ",|)")) {
                    tok = tok2;
                    break; // rValue reference
                }
            }
            compileBinOp(tok, state, compileOr);
        } else break;
    }
}

static void compileLogicOr(Token *&tok, AST_state& state)
{
    compileLogicAnd(tok, state);
    while (tok) {
        if (tok->str() == "||") {
            compileBinOp(tok, state, compileLogicAnd);
        } else break;
    }
}

static void compileAssignTernary(Token *&tok, AST_state& state)
{
    compileLogicOr(tok, state);
    while (tok) {
        if (tok->isAssignmentOp()) {
            state.assign++;
            const Token *tok1 = tok->next();
            compileBinOp(tok, state, compileAssignTernary);
            if (Token::simpleMatch(tok1, "{") && tok == tok1->link() && tok->next())
                tok = tok->next();
            if (state.assign > 0)
                state.assign--;
        } else if (tok->str() == "?") {
            // http://en.cppreference.com/w/cpp/language/operator_precedence says about ternary operator:
            //       "The expression in the middle of the conditional operator (between ? and :) is parsed as if parenthesized: its precedence relative to ?: is ignored."
            // Hence, we rely on Tokenizer::prepareTernaryOpForAST() to add such parentheses where necessary.
            const bool stopAtColon = state.stopAtColon;
            state.stopAtColon = false;
            if (tok->strAt(1) == ":") {
                state.op.push(nullptr);
            }
            const int assign = state.assign;
            state.assign = 0;
            compileBinOp(tok, state, compileAssignTernary);
            state.assign = assign;
            state.stopAtColon = stopAtColon;
        } else if (tok->str() == ":") {
            if (state.depth == 1U && state.inCase) {
                state.inCase = false;
                tok = tok->next();
                break;
            }
            if (state.stopAtColon)
                break;
            if (state.assign > 0)
                break;
            compileBinOp(tok, state, compileAssignTernary);
        } else break;
    }
}

static void compileComma(Token *&tok, AST_state& state)
{
    compileAssignTernary(tok, state);
    while (tok) {
        if (tok->str() == ",") {
            if (Token::simpleMatch(tok, ", }"))
                tok = tok->next();
            else
                compileBinOp(tok, state, compileAssignTernary);
        } else if (tok->str() == ";" && state.functionCallEndPar && tok->index() < state.functionCallEndPar->index()) {
            compileBinOp(tok, state, compileAssignTernary);
        } else break;
    }
}

/**
 * @throws InternalError thrown if maximum AST depth is exceeded
 */
static void compileExpression(Token *&tok, AST_state& state)
{
    if (state.depth > AST_MAX_DEPTH)
        throw InternalError(tok, "maximum AST depth exceeded", InternalError::AST); // ticket #5592
    if (tok)
        compileComma(tok, state);
}

const Token* isLambdaCaptureList(const Token * tok)
{
    // a lambda expression '[x](y){}' is compiled as:
    // [
    // `-(  <<-- optional
    //   `-{
    // see compilePrecedence2
    if (!Token::simpleMatch(tok, "["))
        return nullptr;
    if (!Token::Match(tok->link(), "] (|{"))
        return nullptr;
    if (Token::simpleMatch(tok->astOperand1(), "{") && tok->astOperand1() == tok->link()->next())
        return tok->astOperand1();
    if (!tok->astOperand1() || tok->astOperand1()->str() != "(")
        return nullptr;
    const Token * params = tok->astOperand1();
    if (!Token::simpleMatch(params->astOperand1(), "{"))
        return nullptr;
    return params->astOperand1();
}

const Token* findLambdaEndTokenWithoutAST(const Token* tok) {
    if (!(Token::simpleMatch(tok, "[") && tok->link()))
        return nullptr;
    tok = tok->link()->next();
    if (Token::simpleMatch(tok, "(") && tok->link())
        tok = tok->link()->next();
    if (Token::simpleMatch(tok, "mutable"))
        tok = tok->next();
    if (Token::Match(tok, ".|->")) { // trailing return type
        tok = tok->next();
        while (Token::Match(tok, "%type%|%name%|::|&|&&|*|<|(")) {
            if (tok->link())
                tok = tok->link()->next();
            else
                tok = tok->next();
        }
    }
    if (!(Token::simpleMatch(tok, "{") && tok->link()))
        return nullptr;
    return tok->link()->next();
}

static Token * createAstAtToken(Token *tok);

// Compile inner expressions inside inner ({..}) and lambda bodies
static void createAstAtTokenInner(Token * const tok1, const Token *endToken, bool cpp)
{
    for (Token* tok = tok1; precedes(tok, endToken); tok = tok ? tok->next() : nullptr) {
        if (tok->str() == "{" && !iscpp11init(tok)) {
            const Token * const endToken2 = tok->link();
            bool hasAst = false;
            for (const Token *inner = tok->next(); inner != endToken2; inner = inner->next()) {
                if (inner->astOperand1()) {
                    hasAst = true;
                    break;
                }
                if (tok->isConstOp())
                    break;
                if (inner->str() == "{")
                    inner = inner->link();
            }
            if (!hasAst) {
                for (; tok && tok != endToken && tok != endToken2; tok = tok ? tok->next() : nullptr)
                    tok = createAstAtToken(tok);
            }
        } else if (cpp && tok->str() == "[") {
            if (isLambdaCaptureList(tok)) {
                tok = tok->astOperand1();
                if (tok->str() == "(")
                    tok = tok->astOperand1();
                const Token * const endToken2 = tok->link();
                tok = tok->next();
                for (; tok && tok != endToken && tok != endToken2; tok = tok ? tok->next() : nullptr)
                    tok = createAstAtToken(tok);
            }
        }
        else if (Token::simpleMatch(tok, "( * ) [")) {
            bool hasAst = false;
            for (const Token* tok2 = tok->linkAt(3); tok2 != tok; tok2 = tok2->previous()) {
                if (tok2->astParent() || tok2->astOperand1() || tok2->astOperand2()) {
                    hasAst = true;
                    break;
                }
            }
            if (!hasAst) {
                Token *const startTok = tok = tok->tokAt(4);
                const Token* const endtok = startTok->linkAt(-1);
                AST_state state(cpp);
                compileExpression(tok, state);
                createAstAtTokenInner(startTok, endtok, cpp);
            }
        }
    }
}

static Token * findAstTop(Token *tok1, const Token *tok2)
{
    for (Token *tok = tok1; tok && (tok != tok2); tok = tok->next()) {
        if (tok->astParent() || tok->astOperand1() || tok->astOperand2()) {
            while (tok->astParent() && tok->astParent()->index() >= tok1->index() && tok->astParent()->index() <= tok2->index())
                tok = tok->astParent();
            return tok;
        }
        if (Token::simpleMatch(tok, "( {"))
            tok = tok->link();
    }
    for (Token *tok = tok1; tok && (tok != tok2); tok = tok->next()) {
        if (tok->isName() || tok->isNumber())
            return tok;
        if (Token::simpleMatch(tok, "( {"))
            tok = tok->link();
    }
    return nullptr;
}

static Token *skipMethodDeclEnding(Token *tok)
{
    if (tok->str() != ")")
        tok = tok->previous();
    if (!tok || tok->str() != ")")
        return nullptr;
    Token *const tok2 = const_cast<Token*>(TokenList::isFunctionHead(tok, ";{"));
    if (tok2 && tok->next() != tok2)
        return tok2;
    return nullptr;
}

/**
 * @throws InternalError thrown in case of syntax error
 */
static Token * createAstAtToken(Token *tok)
{
    const bool cpp = tok->isCpp();
    // skip function pointer declaration
    if (Token::Match(tok, "%type% %type%") && !Token::Match(tok, "return|throw|new|delete")) {
        Token* tok2 = tok->tokAt(2);
        // skip type tokens and qualifiers etc
        while (Token::Match(tok2, "%type%|*|&"))
            tok2 = tok2->next();
        if (Token::Match(tok2, "%var% [;,)]"))
            return tok2;
    }
    if (Token::Match(tok, "enum class| %name%| :")) {
        if (Token::simpleMatch(tok->next(), "class"))
            tok = tok->next();
        if (Token::Match(tok->next(), "%name%"))
            tok = tok->next();
        return tok->next();
    }
    if (Token *const endTok = skipMethodDeclEnding(tok)) {
        Token *tok2 = tok;
        do {
            tok2 = tok2->next();
            tok2->setCpp11init(false);
            if (Token::Match(tok2, "decltype|noexcept (")) {
                AST_state state(cpp);
                Token *tok3 = tok2->tokAt(2);
                compileExpression(tok3, state);
                tok2 = tok2->linkAt(1);
            }
        } while (tok2 != endTok && !precedes(endTok, tok2));
        return endTok;
    }
    if (Token::Match(tok, "%type%") && !Token::Match(tok, "return|throw|if|while|new|delete")) {
        bool isStandardTypeOrQualifier = false;
        Token* type = tok;
        while (Token::Match(type, "%type%|*|&|<")) {
            if (type->isName() && (type->isStandardType() || Token::Match(type, "const|mutable|static|volatile")))
                isStandardTypeOrQualifier = true;
            if (type->str() == "<") {
                if (type->link())
                    type = type->link();
                else
                    break;
            }
            type = type->next();
        }
        if (isStandardTypeOrQualifier && Token::Match(type, "%var% [;,)]"))
            return type;
        if (Token::Match(type, "( * *| %var%") &&
            Token::Match(type->link()->previous(), "%var%|] ) (") &&
            Token::Match(type->link()->linkAt(1), ") [;,)]"))
            return type->link()->linkAt(1)->next();
    }

    if (Token::simpleMatch(tok, "for (")) {
        if (cpp && Token::Match(tok, "for ( const| auto &|&&| [")) {
            Token *decl = Token::findsimplematch(tok, "[");
            if (Token::simpleMatch(decl->link(), "] :")) {
                AST_state state1(cpp);
                while (decl->str() != "]") {
                    if (Token::Match(decl, "%name% ,|]")) {
                        state1.op.push(decl);
                    } else if (decl->str() == ",") {
                        if (!state1.op.empty()) {
                            decl->astOperand1(state1.op.top());
                            state1.op.pop();
                        }
                        if (!state1.op.empty()) {
                            state1.op.top()->astOperand2(decl);
                            state1.op.pop();
                        }
                        state1.op.push(decl);
                    }
                    decl = decl->next();
                }
                if (state1.op.size() > 1) {
                    Token *lastName = state1.op.top();
                    state1.op.pop();
                    state1.op.top()->astOperand2(lastName);
                }
                decl = decl->next();

                Token *colon = decl;
                compileExpression(decl, state1);

                tok->next()->astOperand1(tok);
                tok->next()->astOperand2(colon);

                return decl;
            }
        }

        std::vector<Token*> inner;
        Token* tok2 = skipDecl(tok->tokAt(2), &inner);
        for (Token* tok3 : inner) {
            AST_state state1(cpp);
            compileExpression(tok3, state1);
        }
        Token *init1 = nullptr;
        Token * const endPar = tok->linkAt(1);
        if (tok2 == tok->tokAt(2) && Token::Match(tok2, "%op%|(")) {
            init1 = tok2;
            AST_state state1(cpp);
            compileExpression(tok2, state1);
            if (Token::Match(init1, "( !!{")) {
                for (Token *tok3 = init1; tok3 && tok3 != tok3->link(); tok3 = tok3->next()) {
                    if (tok3->astParent()) {
                        while (tok3->astParent())
                            tok3 = tok3->astParent();
                        init1 = tok3;
                        break;
                    }
                    if (!Token::Match(tok3, "%op%|(|["))
                        init1 = tok3;
                }
            }
        } else {
            while (tok2 && tok2 != endPar && tok2->str() != ";") {
                if (tok2->str() == "<" && tok2->link()) {
                    tok2 = tok2->link();
                } else if (Token::Match(tok2, "%name% )| %op%|(|[|{|.|:|::") || Token::Match(tok2->previous(), "[(;{}] %cop%|(")) {
                    init1 = tok2;
                    AST_state state1(cpp);
                    compileExpression(tok2, state1);
                    if (Token::Match(tok2, ";|)"))
                        break;
                    init1 = nullptr;
                }
                if (!tok2) // #7109 invalid code
                    return nullptr;
                tok2 = tok2->next();
            }
        }
        if (!tok2 || tok2->str() != ";") {
            if (tok2 == endPar && init1) {
                createAstAtTokenInner(init1->next(), endPar, cpp);
                tok->next()->astOperand2(init1);
                tok->next()->astOperand1(tok);
            }
            return tok2;
        }

        Token * const init = init1 ? init1 : tok2;

        Token * const semicolon1 = tok2;
        tok2 = tok2->next();
        AST_state state2(cpp);
        compileExpression(tok2, state2);

        Token * const semicolon2 = tok2;
        if (!semicolon2)
            return nullptr; // invalid code #7235

        if (semicolon2->str() == ";") {
            tok2 = tok2->next();
            AST_state state3(cpp);
            if (Token::simpleMatch(tok2, "( {")) {
                state3.op.push(tok2->next());
                tok2 = tok2->link()->next();
            }
            compileExpression(tok2, state3);

            tok2 = findAstTop(semicolon1->next(), semicolon2);
            if (tok2)
                semicolon2->astOperand1(tok2);
            tok2 = findAstTop(semicolon2->next(), endPar);
            if (tok2)
                semicolon2->astOperand2(tok2);
            else if (!state3.op.empty())
                semicolon2->astOperand2(state3.op.top());
            semicolon1->astOperand2(semicolon2);
        } else {
            if (!cpp || state2.op.empty() || !Token::simpleMatch(state2.op.top(), ":"))
                throw InternalError(tok, "syntax error", InternalError::SYNTAX);

            semicolon1->astOperand2(state2.op.top());
        }

        if (init != semicolon1)
            semicolon1->astOperand1(init->astTop());
        tok->next()->astOperand1(tok);
        tok->next()->astOperand2(semicolon1);

        createAstAtTokenInner(endPar->link(), endPar, cpp);

        return endPar;
    }

    if (Token::simpleMatch(tok, "( {"))
        return tok;

    if (Token::Match(tok, "%type% <") && tok->linkAt(1) && !Token::Match(tok->linkAt(1), "> [({]"))
        return tok->linkAt(1);

    if (cpp && !tok->isKeyword() && Token::Match(tok, "%type% ::|<|%name%")) {
        Token *tok2 = tok;
        while (true) {
            if (Token::Match(tok2, "%name%|> :: %name%"))
                tok2 = tok2->tokAt(2);
            else if (Token::Match(tok2, "%name% <") && tok2->linkAt(1))
                tok2 = tok2->linkAt(1);
            else
                break;
        }
        if (Token::Match(tok2, "%name%|> %name% {") && tok2->next()->varId() && iscpp11init(tok2->tokAt(2))) {
            Token *const tok1 = tok = tok2->next();
            AST_state state(cpp);
            compileExpression(tok, state);
            createAstAtTokenInner(tok1->next(), tok1->linkAt(1), cpp);
            return tok;
        }
    }

    if ((Token::Match(tok, "%type% %name%|*|&|&&|::") && !Token::Match(tok, "return|new|delete")) ||
        (Token::Match(tok, ":: %type%") && !tok->next()->isKeyword())) {
        int typecount = tok->str() == "::" ? 1 : 0;
        Token *typetok = tok;
        while (Token::Match(typetok, "%type%|::|*|&|&&|<")) {
            if (typetok->isName() && !Token::simpleMatch(typetok->previous(), "::"))
                typecount++;
            if (typetok->str() == "<") {
                if (Token* closing = typetok->findClosingBracket()) {
                    typetok = closing->next();
                    continue;
                }
                break;
            }
            typetok = typetok->next();
        }
        if (Token::Match(typetok, "%var% =") && typetok->varId())
            tok = typetok;

        // Do not create AST for function declaration
        if (typetok &&
            typecount >= 2 &&
            !Token::Match(tok, "return|throw") &&
            Token::Match(typetok->previous(), "%name% ( !!*") &&
            typetok->previous()->varId() == 0 &&
            (!typetok->previous()->isKeyword() || typetok->previous()->isOperatorKeyword()) &&
            (skipMethodDeclEnding(typetok->link()) || Token::Match(typetok->link(), ") ;|{")))
            return typetok;
    }

    if (Token::Match(tok, "return|case") ||
        (cpp && tok->str() == "throw") ||
        !tok->previous() ||
        Token::Match(tok, "%name% %op%|(|[|.|::|<|?|;") ||
        (cpp && Token::Match(tok, "%name% {") && iscpp11init(tok->next())) ||
        Token::Match(tok->previous(), "[;{}:] %cop%|++|--|( !!{") ||
        Token::Match(tok->previous(), "[;{}] %num%|%str%|%char%") ||
        Token::Match(tok->previous(), "[;{}] delete new") ||
        (cpp && Token::Match(tok->previous(), "[;{}] ["))) {
        if (cpp && (Token::Match(tok->tokAt(-2), "[;{}] new|delete %name%") || Token::Match(tok->tokAt(-3), "[;{}] :: new|delete %name%")))
            tok = tok->previous();

        Token * const tok1 = tok;
        AST_state state(cpp);
        if (Token::Match(tok, "%name% ("))
            state.functionCallEndPar = tok->linkAt(1);
        if (Token::simpleMatch(tok->tokAt(-1), "::") && (!tok->tokAt(-2) || !tok->tokAt(-2)->isName()))
            tok = tok->tokAt(-1);
        compileExpression(tok, state);
        Token * const endToken = tok;
        if (endToken == tok1 || !endToken)
            return tok1;

        createAstAtTokenInner(tok1->next(), endToken, cpp);

        return endToken->previous();
    }

    if (cpp && tok->str() == "{" && iscpp11init(tok)) {
        Token * const tok1 = tok;
        AST_state state(cpp);
        compileExpression(tok, state);
        Token* const endToken = tok;
        if (endToken == tok1 || !endToken)
            return tok1;

        createAstAtTokenInner(tok1->next(), endToken, cpp);
        return endToken->previous();
    }

    return tok;
}

void TokenList::createAst() const
{
    for (Token *tok = mTokensFrontBack->front; tok; tok = tok ? tok->next() : nullptr) {
        Token* const nextTok = createAstAtToken(tok);
        if (precedes(nextTok, tok))
            throw InternalError(tok, "Syntax Error: Infinite loop when creating AST.", InternalError::AST);
        tok = nextTok;
    }
    for (Token *tok = mTokensFrontBack->front; tok; tok = tok ? tok->next() : nullptr) {
        if (tok->astParent())
            continue;
        if (!tok->astOperand1() && !tok->astOperand2())
            continue;
        visitAstNodes(tok, [&](Token* child) {
            child->astTop(tok);
            return ChildrenToVisit::op1_and_op2;
        });
    }
}

namespace {
    struct OnException {
        std::function<void()> f;

        ~OnException() {
#ifndef _MSC_VER
#if defined(__cpp_lib_uncaught_exceptions)
            const bool b = std::uncaught_exceptions() > 0;
#else
            const bool b = std::uncaught_exception();
#endif
            if (b)
                f();
#endif
        }
    };
}

void TokenList::validateAst(bool print) const
{
    OnException oe{[&] {
            if (print)
                mTokensFrontBack->front->printOut(std::cout);
        }};
    // Check for some known issues in AST to avoid crash/hang later on
    std::set<const Token*> safeAstTokens;    // list of "safe" AST tokens without endless recursion
    for (const Token *tok = mTokensFrontBack->front; tok; tok = tok->next()) {
        // Syntax error if binary operator only has 1 operand
        if ((tok->isAssignmentOp() || tok->isComparisonOp() || Token::Match(tok,"[|^/%]")) && tok->astOperand1() && !tok->astOperand2())
            throw InternalError(tok, "Syntax Error: AST broken, binary operator has only one operand.", InternalError::AST);

        // Syntax error if we encounter "?" with operand2 that is not ":"
        if (tok->str() == "?") {
            if (!tok->astOperand1() || !tok->astOperand2())
                throw InternalError(tok, "AST broken, ternary operator missing operand(s)", InternalError::AST);
            if (tok->astOperand2()->str() != ":")
                throw InternalError(tok, "Syntax Error: AST broken, ternary operator lacks ':'.", InternalError::AST);
        }

        // Check for endless recursion
        const Token* parent = tok->astParent();
        if (parent) {
            std::set<const Token*> astTokens;    // list of ancestors
            astTokens.insert(tok);
            do {
                if (safeAstTokens.find(parent) != safeAstTokens.end())
                    break;
                if (astTokens.find(parent) != astTokens.end())
                    throw InternalError(tok, "AST broken: endless recursion from '" + tok->str() + "'", InternalError::AST);
                astTokens.insert(parent);
            } while ((parent = parent->astParent()) != nullptr);
            safeAstTokens.insert(astTokens.cbegin(), astTokens.cend());
        } else if (tok->str() == ";") {
            safeAstTokens.clear();
        } else {
            safeAstTokens.insert(tok);
        }

        // Don't check templates
        if (tok->str() == "<" && tok->link()) {
            tok = tok->link();
            continue;
        }
        if (tok->isCast()) {
            if (!tok->astOperand2() && precedes(tok->astOperand1(), tok))
                throw InternalError(tok, "AST broken: '" + tok->str() + "' has improper operand.", InternalError::AST);
            if (tok->astOperand1() && tok->link()) { // skip casts (not part of the AST)
                tok = tok->link();
                continue;
            }
        }

        if (findLambdaEndToken(tok)) { // skip lambda captures
            tok = tok->link();
            continue;
        }

        // Check binary operators
        if (Token::Match(tok, "%or%|%oror%|%assign%|%comp%")) {
            // Skip pure virtual functions
            if (Token::simpleMatch(tok->previous(), ") = 0"))
                continue;
            // Skip operator definitions
            if (Token::simpleMatch(tok->previous(), "operator"))
                continue;
            // Skip incomplete code
            if (!tok->astOperand1() && !tok->astOperand2() && !tok->astParent())
                continue;
            // Skip lambda assignment and/or initializer
            if (Token::Match(tok, "= {|^|["))
                continue;
            // FIXME: Workaround broken AST assignment in type aliases
            if (Token::Match(tok->previous(), "%name% = %name%"))
                continue;
            if (!tok->astOperand1() || !tok->astOperand2())
                throw InternalError(tok, "Syntax Error: AST broken, binary operator '" + tok->str() + "' doesn't have two operands.", InternalError::AST);
        }

        if (Token::Match(tok, "++|--") && !tok->astOperand1()) {
            throw InternalError(tok, "Syntax Error: AST broken, operator '" + tok->str() + "' doesn't have an operand.", InternalError::AST);
        }

        // Check control blocks and asserts
        if (Token::Match(tok->previous(), "if|while|for|switch|assert|ASSERT (")) {
            if (!tok->astOperand1() || !tok->astOperand2())
                throw InternalError(tok,
                                    "Syntax Error: AST broken, '" + tok->strAt(-1) +
                                    "' doesn't have two operands.",
                                    InternalError::AST);
        }
        if (tok->str() == "case" && !tok->astOperand1()) {
            throw InternalError(tok,
                                "Syntax Error: AST broken, 'case' doesn't have an operand.",
                                InternalError::AST);
        }

        // Check member access
        if (Token::Match(tok, "%var% .")) {
            if (!tok->astParent()) {
                throw InternalError(
                          tok, "Syntax Error: AST broken, '" + tok->str() + "' doesn't have a parent.", InternalError::AST);
            }
            if (!tok->next()->astOperand1() || !tok->next()->astOperand2()) {
                const std::string& op =
                    tok->next()->originalName().empty() ? tok->strAt(1) : tok->next()->originalName();
                throw InternalError(
                          tok, "Syntax Error: AST broken, '" + op + "' doesn't have two operands.", InternalError::AST);
            }
        }
    }
}

std::string TokenList::getOrigFile(const Token *tok) const
{
    return mOrigFiles.at(tok->fileIndex());
}

const std::string& TokenList::file(const Token *tok) const
{
    return mFiles.at(tok->fileIndex());
}

std::string TokenList::fileLine(const Token *tok) const
{
    return ErrorMessage::FileLocation(tok, this).stringify();
}

bool TokenList::validateToken(const Token* tok) const
{
    if (!tok)
        return true;
    for (const Token *t = mTokensFrontBack->front; t; t = t->next()) {
        if (tok==t)
            return true;
    }
    return false;
}

void TokenList::simplifyPlatformTypes()
{
    const bool isCPP11 = isCPP() && (mSettings.standards.cpp >= Standards::CPP11);

    enum : std::uint8_t { isLongLong, isLong, isInt } type;

    /** @todo This assumes a flat address space. Not true for segmented address space (FAR *). */

    if (mSettings.platform.sizeof_size_t == mSettings.platform.sizeof_long)
        type = isLong;
    else if (mSettings.platform.sizeof_size_t == mSettings.platform.sizeof_long_long)
        type = isLongLong;
    else if (mSettings.platform.sizeof_size_t == mSettings.platform.sizeof_int)
        type = isInt;
    else
        return;

    for (Token *tok = front(); tok; tok = tok->next()) {
        // pre-check to reduce unneeded match calls
        if (!Token::Match(tok, "std| ::| %type%"))
            continue;
        bool isUnsigned;
        if (Token::Match(tok, "std| ::| size_t|uintptr_t|uintmax_t")) {
            if (isCPP11 && tok->strAt(-1) == "using" && tok->strAt(1) == "=")
                continue;
            isUnsigned = true;
        } else if (Token::Match(tok, "std| ::| ssize_t|ptrdiff_t|intptr_t|intmax_t")) {
            if (isCPP11 && tok->strAt(-1) == "using" && tok->strAt(1) == "=")
                continue;
            isUnsigned = false;
        } else
            continue;

        bool inStd = false;
        if (tok->str() == "::") {
            tok->deleteThis();
        } else if (tok->str() == "std") {
            if (tok->strAt(1) != "::")
                continue;
            inStd = true;
            tok->deleteNext();
            tok->deleteThis();
        }

        if (inStd)
            tok->originalName("std::" + tok->str());
        else
            tok->originalName(tok->str());
        if (isUnsigned)
            tok->isUnsigned(true);

        switch (type) {
        case isLongLong:
            tok->isLong(true);
            tok->str("long");
            break;
        case isLong:
            tok->str("long");
            break;
        case isInt:
            tok->str("int");
            break;
        }
    }

    const std::string platform_type(mSettings.platform.toString());

    for (Token *tok = front(); tok; tok = tok->next()) {
        if (tok->tokType() != Token::eType && tok->tokType() != Token::eName)
            continue;

        const Library::PlatformType * const platformtype = mSettings.library.platform_type(tok->str(), platform_type);

        if (platformtype) {
            // check for namespace
            if (tok->strAt(-1) == "::") {
                const Token * tok1 = tok->tokAt(-2);
                // skip when non-global namespace defined
                if (tok1 && tok1->tokType() == Token::eName)
                    continue;
                tok = tok->previous();
                tok->deleteThis();
            }
            tok->originalName(tok->str());
            const bool isFunctionalPtrCast = (platformtype->mConstPtr || platformtype->mPointer || platformtype->mPtrPtr) &&
                                             Token::Match(tok, "%name% [({]") && !Token::simpleMatch(tok->linkAt(1), ") (");
            Token* start = isFunctionalPtrCast ? tok->tokAt(1) : nullptr;
            Token* end = isFunctionalPtrCast ? tok->linkAt(1) : nullptr;
            Token *typeToken;
            if (platformtype->mConstPtr) {
                tok->str("const");
                tok->isSimplifiedTypedef(true);
                tok->insertToken("*")->isSimplifiedTypedef(true);
                tok->insertToken(platformtype->mType)->isSimplifiedTypedef(true);
                typeToken = tok;
            } else if (platformtype->mPointer) {
                tok->str(platformtype->mType);
                tok->isSimplifiedTypedef(true);
                typeToken = tok;
                tok->insertToken("*")->isSimplifiedTypedef(true);
            } else if (platformtype->mPtrPtr) {
                tok->str(platformtype->mType);
                tok->isSimplifiedTypedef(true);
                typeToken = tok;
                tok->insertToken("*")->isSimplifiedTypedef(true);
                tok->insertToken("*")->isSimplifiedTypedef(true);
            } else {
                tok->str(platformtype->mType);
                tok->isSimplifiedTypedef(true);
                typeToken = tok;
            }
            if (platformtype->mSigned)
                typeToken->isSigned(true);
            if (platformtype->mUnsigned)
                typeToken->isUnsigned(true);
            if (platformtype->mLong)
                typeToken->isLong(true);

            if (isFunctionalPtrCast) {
                start->str("(");
                end->str(")");
                if (end == start->tokAt(1))
                    end->insertTokenBefore("0");
                end = start->insertTokenBefore(")");
                start = tok->insertTokenBefore("(");
                start->isSimplifiedTypedef(true);
                Token::createMutualLinks(start, end);
            }
        }
    }
}

void TokenList::simplifyStdType()
{
    auto isVarDeclC = [](const Token* tok) -> bool {
        if (!Token::simpleMatch(tok, "}"))
            return false;
        tok = tok->link()->previous();
        while (Token::Match(tok, "%name%")) {
            if (Token::Match(tok, "struct|union|enum"))
                return true;
            tok = tok->previous();
        }
        return false;
    };

    for (Token *tok = front(); tok; tok = tok->next()) {

        if (isC() && Token::Match(tok, "const|extern *|&|%name%") && (!tok->previous() || Token::Match(tok->previous(), "[;{}]"))) {
            if (Token::Match(tok->next(), "%name% !!;"))
                continue;
            if (isVarDeclC(tok->previous()))
                continue;

            tok->insertToken("int");
            tok->next()->isImplicitInt(true);
            continue;
        }

        if (Token::Match(tok, "char|short|int|long|unsigned|signed|double|float") || (isC() && (mSettings.standards.c >= Standards::C99) && Token::Match(tok, "complex|_Complex"))) {
            bool isFloat= false;
            bool isSigned = false;
            bool isUnsigned = false;
            bool isComplex = false;
            int countLong = 0;
            Token* typeSpec = nullptr;

            Token* tok2 = tok;
            for (; tok2->next(); tok2 = tok2->next()) {
                if (tok2->str() == "long") {
                    countLong++;
                    if (!isFloat)
                        typeSpec = tok2;
                } else if (tok2->str() == "short") {
                    typeSpec = tok2;
                } else if (tok2->str() == "unsigned")
                    isUnsigned = true;
                else if (tok2->str() == "signed")
                    isSigned = true;
                else if (Token::Match(tok2, "float|double")) {
                    isFloat = true;
                    typeSpec = tok2;
                } else if (isC() && (mSettings.standards.c >= Standards::C99) && Token::Match(tok2, "complex|_Complex"))
                    isComplex = !isFloat || tok2->str() == "_Complex" || Token::Match(tok2->next(), "*|&|%name%"); // Ensure that "complex" is not the variables name
                else if (Token::Match(tok2, "char|int")) {
                    if (!typeSpec)
                        typeSpec = tok2;
                } else
                    break;
            }

            if (!typeSpec) { // unsigned i; or similar declaration
                if (!isComplex) { // Ensure that "complex" is not the variables name
                    tok->str("int");
                    tok->isSigned(isSigned);
                    tok->isUnsigned(isUnsigned);
                    tok->isImplicitInt(true);
                }
            } else {
                typeSpec->isLong(typeSpec->isLong() || (isFloat && countLong == 1) || countLong > 1);
                typeSpec->isComplex(typeSpec->isComplex() || (isFloat && isComplex));
                typeSpec->isSigned(typeSpec->isSigned() || isSigned);
                typeSpec->isUnsigned(typeSpec->isUnsigned() || isUnsigned);

                // Remove specifiers
                const Token* tok3 = tok->previous();
                tok2 = tok2->previous();
                while (tok3 != tok2) {
                    if (tok2 != typeSpec &&
                        (isComplex || !Token::Match(tok2, "complex|_Complex")))  // Ensure that "complex" is not the variables name
                        tok2->deleteThis();
                    tok2 = tok2->previous();
                }
            }
        }
    }
}

bool TokenList::isKeyword(const std::string &str) const
{
    if (isCPP()) {
        const auto &cpp_keywords = Keywords::getAll(mSettings.standards.cpp);
        const bool b = cpp_keywords.find(str) != cpp_keywords.end();
        if (b) {
            // TODO: integrate into keywords?
            // types and literals are not handled as keywords
            static const std::unordered_set<std::string> cpp_types = {"bool", "false", "true"};
            if (cpp_types.find(str) != cpp_types.end())
                return false;
        }
        return b;
    }

    const auto &c_keywords = Keywords::getAll(mSettings.standards.c);
    const bool b = c_keywords.find(str) != c_keywords.end();
    if (b) {
        // TODO: integrate into Keywords?
        // types are not handled as keywords
        static const std::unordered_set<std::string> c_types = {"char", "double", "float", "int", "long", "short"};
        if (c_types.find(str) != c_types.end())
            return false;
    }
    return b;
}

bool TokenList::isC() const
{
    return mLang == Standards::Language::C;
}

bool TokenList::isCPP() const
{
    return mLang == Standards::Language::CPP;
}

const Token * TokenList::isFunctionHead(const Token *tok, const std::string &endsWith)
{
    if (!tok)
        return nullptr;
    if (tok->str() == "(")
        tok = tok->link();
    if (tok->str() != ")")
        return nullptr;
    if (!tok->isCpp() && !Token::Match(tok->link()->previous(), "%name%|(|)"))
        return nullptr;
    if (Token::Match(tok, ") ;|{|[")) {
        tok = tok->next();
        while (tok && tok->str() == "[" && tok->link()) {
            if (endsWith.find(tok->str()) != std::string::npos)
                return tok;
            tok = tok->link()->next();
        }
        return (tok && endsWith.find(tok->str()) != std::string::npos) ? tok : nullptr;
    }
    if (tok->isCpp() && tok->str() == ")") {
        tok = tok->next();
        while (Token::Match(tok, "const|noexcept|override|final|volatile|mutable|&|&& !!(") ||
               (Token::Match(tok, "%name% !!(") && tok->isUpperCaseName()))
            tok = tok->next();
        if (tok && tok->str() == ")")
            tok = tok->next();
        while (tok && tok->str() == "[")
            tok = tok->link()->next();
        if (Token::Match(tok, "throw|noexcept ("))
            tok = tok->linkAt(1)->next();
        if (Token::Match(tok, "%name% (") && tok->isUpperCaseName())
            tok = tok->linkAt(1)->next();
        if (tok && tok->originalName() == "->") { // trailing return type
            for (tok = tok->next(); tok && !Token::Match(tok, ";|{|override|final|}|)|]"); tok = tok->next())
                if (tok->link() && Token::Match(tok, "<|[|("))
                    tok = tok->link();
        }
        while (Token::Match(tok, "override|final !!(") ||
               (Token::Match(tok, "%name% !!(") && tok->isUpperCaseName()))
            tok = tok->next();
        if (Token::Match(tok, "= 0|default|delete ;"))
            tok = tok->tokAt(2);
        if (Token::simpleMatch(tok, "requires")) {
            for (tok = tok->next(); tok && !Token::Match(tok, ";|{|}|)|]"); tok = tok->next()) {
                if (tok->link() && Token::Match(tok, "<|[|("))
                    tok = tok->link();
                if (Token::simpleMatch(tok, "bool {"))
                    tok = tok->linkAt(1);
            }
        }
        if (tok && tok->str() == ":" && !Token::Match(tok->next(), "%name%|::"))
            return nullptr;
        return (tok && endsWith.find(tok->str()) != std::string::npos) ? tok : nullptr;
    }
    return nullptr;
}
